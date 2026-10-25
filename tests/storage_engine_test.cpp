#include <gtest/gtest.h>
#include "../src/core/storage_engine.h"
#include <stdexcept>


//  Testing StorageEngine.set(key , value) API

TEST(StorageEngineTest, SetOverwritesExistingValue) {
    StorageEngine engine;

    engine.set("age", "20");
    engine.set("age", "21");

    auto value = engine.get("age");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "21");
}

//  Testing StorageEngine.get() API

TEST(StorageEngineTest, GetReturnsNoValueForMissingKey) {
    StorageEngine engine;

    auto value = engine.get("missing");

    EXPECT_FALSE(value.has_value());
}


//  Testing StorageEngine.del(key) API

TEST(StorageEngineTest, DeleteExistingKey) {
    StorageEngine engine;

    engine.set("age", "20");

    const bool deleted = engine.del("age");

    EXPECT_TRUE(deleted);
    EXPECT_FALSE(engine.exists("age"));
}

//  Testing StorageEngine.set(key) API

TEST(StorageEngineTest, DeleteMissingKey) {
    StorageEngine engine;

    const bool deleted = engine.del("missing");

    EXPECT_FALSE(deleted);
}

//  Testing StorageEngine.incr(key) API

TEST(StorageEngineTest, IncrementExistingInteger) {
    StorageEngine engine;

    engine.set("counter", "10");

    EXPECT_EQ(engine.incr("counter"), 11);
    EXPECT_EQ(engine.get("counter").value(), "11");
}

//  Testing StorageEngine.incr(key) API

TEST(StorageEngineTest, IncrementMissingKey) {
    StorageEngine engine;

    EXPECT_EQ(engine.incr("counter"), 1);
    EXPECT_EQ(engine.get("counter").value(), "1");
}

//  Testing StorageEngine.decr(key) API

TEST(StorageEngineTest, DecrementExistingInteger) {
    StorageEngine engine;

    engine.set("counter", "10");

    EXPECT_EQ(engine.decr("counter"), 9);
    EXPECT_EQ(engine.get("counter").value(), "9");
}

//  Testing StorageEngine.decr(key) API

TEST(StorageEngineTest, DecrementMissingKey) {
    StorageEngine engine;

    EXPECT_EQ(engine.decr("counter"), -1);
    EXPECT_EQ(engine.get("counter").value(), "-1");
}

//  Testing StorageEngine.incr(key) API

TEST(StorageEngineTest, IncrementRejectsInvalidInteger) {
    StorageEngine engine;
    engine.set("counter", "hello");

    EXPECT_THROW(engine.incr("counter"), std::invalid_argument);

    EXPECT_EQ(engine.get("counter").value(), "hello");
}

//  Testing StorageEngine.incr(key) API

TEST(StorageEngineTest, IncrementRejectsOverflow) {
    StorageEngine engine;
    engine.set("counter", "9223372036854775807");

    EXPECT_THROW(engine.incr("counter"), std::overflow_error);

    EXPECT_EQ(engine.get("counter").value(), "9223372036854775807");
}

//  Testing StorageEngine.decr(key) API

TEST(StorageEngineTest, DecrementRejectsUnderflow) {
    StorageEngine engine;
    engine.set("counter", "-9223372036854775808");

    EXPECT_THROW(engine.decr("counter"), std::overflow_error);

    EXPECT_EQ(engine.get("counter").value(), "-9223372036854775808");
}

//  Testing StorageEngine.mget(keys) API

TEST(StorageEngineTest, MGetMultipleExistingKeys) {
    StorageEngine engine;

    engine.set("name", "Harsh");
    engine.set("language", "C++");
    engine.set("editor", "CLion");

    const auto values = engine.mget({"name", "language", "editor"});

    ASSERT_EQ(values.size(), 3);
    ASSERT_TRUE(values[0].has_value());
    ASSERT_TRUE(values[1].has_value());
    ASSERT_TRUE(values[2].has_value());

    EXPECT_EQ(values[0].value(), "Harsh");
    EXPECT_EQ(values[1].value(), "C++");
    EXPECT_EQ(values[2].value(), "CLion");
}

//  Testing StorageEngine.mget(keys) API

TEST(StorageEngineTest, MGetHandlesMissingKeys) {
    StorageEngine engine;

    engine.set("name", "Harsh");

    const auto values = engine.mget({"name", "missing"});

    ASSERT_EQ(values.size(), 2);
    ASSERT_TRUE(values[0].has_value());

    EXPECT_EQ(values[0].value(), "Harsh");
    EXPECT_FALSE(values[1].has_value());
}

//  Testing StorageEngine.mget(keys) API

TEST(StorageEngineTest, MGetEmptyKeyList) {
    StorageEngine engine;

    const auto values = engine.mget({});

    EXPECT_TRUE(values.empty());
}

//  Testing StorageEngine.mset(keys) API

TEST(StorageEngineTest, MSetMultipleKeyValuePairs) {
    StorageEngine engine;

    engine.mset({
        {"name", "Harsh"},
        {"language", "C++"},
        {"editor", "CLion"}
    });

    EXPECT_EQ(engine.get("name").value(), "Harsh");
    EXPECT_EQ(engine.get("language").value(), "C++");
    EXPECT_EQ(engine.get("editor").value(), "CLion");
}

//  Testing StorageEngine.mset(keys) API

TEST(StorageEngineTest, MSetOverwritesExistingValues) {
    StorageEngine engine;

    engine.set("name", "OldName");
    engine.set("language", "Java");

    engine.mset({
        {"name", "Harsh"},
        {"language", "C++"}
    });

    EXPECT_EQ(engine.get("name").value(), "Harsh");
    EXPECT_EQ(engine.get("language").value(), "C++");
}

//  Testing StorageEngine.mset(keys) API

TEST(StorageEngineTest, MSetEmptyEntries) {
    StorageEngine engine;

    engine.set("existing", "value");

    engine.mset({});

    EXPECT_EQ(engine.get("existing").value(), "value");
    EXPECT_EQ(engine.exists("existing"), true);
}

//  Testing StorageEngine.dbSize() API

TEST(StorageEngineTest, DbSizeEmptyDatabase) {
    StorageEngine engine;

    EXPECT_EQ(engine.dbsize(), 0);
}

//  Testing StorageEngine.dbSize() API

TEST(StorageEngineTest, DbSizeCountsKeys) {
    StorageEngine engine;

    engine.set("name", "Harsh");
    engine.set("language", "C++");
    engine.set("editor", "CLion");

    EXPECT_EQ(engine.dbsize(), 3);
}

//  Testing StorageEngine.dbSize() API

TEST(StorageEngineTest, DbSizeAfterOverwriteAndDelete) {
    StorageEngine engine;

    engine.set("name", "Harsh");
    engine.set("language", "C++");
    EXPECT_EQ(engine.dbsize(), 2);

    engine.set("name", "UpdatedName");
    EXPECT_EQ(engine.dbsize(), 2);

    engine.del("language");
    EXPECT_EQ(engine.dbsize(), 1);
}

//  Testing StorageEngine.flushdb() API

TEST(StorageEngineTest, FlushDBClearsAllKeys) {
    StorageEngine engine;

    engine.set("name", "Harsh");
    engine.set("age", "20");
    engine.set("language", "C++");

    engine.flushdb();

    EXPECT_EQ(engine.dbsize(), 0);
    EXPECT_FALSE(engine.exists("name"));
    EXPECT_FALSE(engine.exists("age"));
    EXPECT_FALSE(engine.exists("language"));
}


//  Testing StorageEngine.flushdb() API

TEST(StorageEngineTest, FlushDBOnEmptyDatabase) {
    StorageEngine engine;

    EXPECT_NO_THROW(engine.flushdb());
    EXPECT_EQ(engine.dbsize(), 0);
}

//  Testing StorageEngine.flushdb() API

TEST(StorageEngineTest, CanStoreKeysAfterFlushDB) {
    StorageEngine engine;

    engine.set("old_key", "old_value");
    engine.flushdb();

    engine.set("new_key", "new_value");

    EXPECT_EQ(engine.dbsize(), 1);
    EXPECT_FALSE(engine.exists("old_key"));
    EXPECT_EQ(engine.get("new_key").value(), "new_value");
}