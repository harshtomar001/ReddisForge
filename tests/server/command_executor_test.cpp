#include <gtest/gtest.h>

#include "../../src/core/storage_engine.h"
#include "../../src/server/command_executor.h"
#include "../../src/server/resp2_value.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

// Helpers to verify and extract RESP response types.

struct RespValue;

std::string getSimpleString(const RespValue& result) {

    auto value = std::get_if<SimpleString>(&result.data);
    if (value == nullptr) {
    throw std::runtime_error("Expected a RESP simple string");
    }
    return value->value;

}

std::string getBulkString(const RespValue& result) {

    auto value = std::get_if<BulkString>(&result.data);
    if (value == nullptr) {
    throw std::runtime_error("Expected a RESP bulk string");
    }
    return value->value;

}

long long getInteger(const RespValue& result) {

    auto value = std::get_if<long long>(&result.data);
    if (value == nullptr) {
    throw std::runtime_error("Expected a RESP integer");
    }
    return *value;

}

const std::vector<RespValue>& getArray(const RespValue& result) {

    auto arrayPtr =std::get_if<std::shared_ptr<std::vector<RespValue>>>(&result.data);


    if (arrayPtr == nullptr || *arrayPtr == nullptr) {
        throw std::runtime_error("Expected a RESP array");
    }

    return **arrayPtr;


}

// PING

TEST(CommandExecutorTest, PingReturnsPong) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getSimpleString(executor.execute({"PING"})), "PONG");

}

TEST(CommandExecutorTest, PingRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"PING", "extra"}), std::invalid_argument);

}



// SET

TEST(CommandExecutorTest, SetStoresValueAndReturnsOk) {
StorageEngine storage;
CommandExecutor executor(storage);


EXPECT_EQ(getSimpleString(executor.execute({"SET", "name", "Harsh"})), "OK");
ASSERT_TRUE(storage.get("name").has_value());
EXPECT_EQ(storage.get("name").value(), "Harsh");


}

TEST(CommandExecutorTest, SetOverwritesExistingValue) {
StorageEngine storage;
CommandExecutor executor(storage);


executor.execute({"SET", "name", "Harsh"});
executor.execute({"SET", "name", "Tomar"});

EXPECT_EQ(storage.get("name").value(), "Tomar");
EXPECT_EQ(storage.dbsize(), 1);


}

TEST(CommandExecutorTest, SetRejectsMissingArguments) {
StorageEngine storage;
CommandExecutor executor(storage);


EXPECT_THROW(executor.execute({"SET"}), std::invalid_argument);
EXPECT_THROW(executor.execute({"SET", "name"}), std::invalid_argument);


}

TEST(CommandExecutorTest, SetRejectsExtraArguments) {
StorageEngine storage;
CommandExecutor executor(storage);


EXPECT_THROW(
    executor.execute({"SET", "name", "Harsh", "extra"}),
    std::invalid_argument
);


}

// GET

TEST(CommandExecutorTest, GetReturnsExistingValue) {
StorageEngine storage;
CommandExecutor executor(storage);


executor.execute({"SET", "name", "Harsh"});

EXPECT_EQ(getBulkString(executor.execute({"GET", "name"})), "Harsh");


}

TEST(CommandExecutorTest, GetMissingKeyReturnsNil) {
StorageEngine storage;
CommandExecutor executor(storage);


RespValue result = executor.execute({"GET", "missing"});

EXPECT_NE(std::get_if<std::monostate>(&result.data), nullptr);


}

TEST(CommandExecutorTest, GetRejectsMissingKeyArgument) {
StorageEngine storage;
CommandExecutor executor(storage);


EXPECT_THROW(executor.execute({"GET"}), std::invalid_argument);


}

TEST(CommandExecutorTest, GetRejectsExtraArguments) {
StorageEngine storage;
CommandExecutor executor(storage);


EXPECT_THROW(
    executor.execute({"GET", "name", "extra"}),
    std::invalid_argument
);


}

// DEL

TEST(CommandExecutorTest, DelRemovesExistingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");

    EXPECT_EQ(getInteger(executor.execute({"DEL", "name"})), 1);
    EXPECT_FALSE(storage.exists("name"));


}

TEST(CommandExecutorTest, DelReturnsZeroForMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getInteger(executor.execute({"DEL", "missing"})), 0);


}

TEST(CommandExecutorTest, DelCountsOnlyDeletedKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(
        getInteger(executor.execute({"DEL", "name", "missing", "city"})),
        2
    );
    EXPECT_EQ(storage.dbsize(), 0);


}

TEST(CommandExecutorTest, DelRejectsMissingKeyArgument) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"DEL"}), std::invalid_argument);


}

// EXISTS

TEST(CommandExecutorTest, ExistsReturnsOneForExistingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");

    EXPECT_EQ(getInteger(executor.execute({"EXISTS", "name"})), 1);


}

TEST(CommandExecutorTest, ExistsReturnsZeroForMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getInteger(executor.execute({"EXISTS", "country"})), 0);


}

TEST(CommandExecutorTest, ExistsCountsMultipleKeysAndOccurrences) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(
        getInteger(executor.execute({"EXISTS", "name", "country", "city"})),
        2
    );
    EXPECT_EQ(getInteger(executor.execute({"EXISTS", "name", "name"})), 2);


}

TEST(CommandExecutorTest, ExistsRejectsMissingKeyArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"EXISTS"}), std::invalid_argument);


}

// INCR

TEST(CommandExecutorTest, IncrIncrementsExistingInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "10");

    EXPECT_EQ(getInteger(executor.execute({"INCR", "counter"})), 11);
    EXPECT_EQ(storage.get("counter").value(), "11");


}

TEST(CommandExecutorTest, IncrInitializesMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getInteger(executor.execute({"INCR", "counter"})), 1);
    EXPECT_EQ(storage.get("counter").value(), "1");


}

TEST(CommandExecutorTest, IncrRejectsInvalidInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "hello");

    EXPECT_THROW(
        executor.execute({"INCR", "counter"}),
        std::invalid_argument
    );
    EXPECT_EQ(storage.get("counter").value(), "hello");


}

TEST(CommandExecutorTest, IncrRejectsMissingKeyArgument) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"INCR"}), std::invalid_argument);


}

TEST(CommandExecutorTest, IncrRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(
        executor.execute({"INCR", "counter", "extra"}),
        std::invalid_argument
    );


}

// DECR

TEST(CommandExecutorTest, DecrDecrementsExistingInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "10");

    EXPECT_EQ(getInteger(executor.execute({"DECR", "counter"})), 9);
    EXPECT_EQ(storage.get("counter").value(), "9");


}

TEST(CommandExecutorTest, DecrInitializesMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getInteger(executor.execute({"DECR", "counter"})), -1);
    EXPECT_EQ(storage.get("counter").value(), "-1");


}

TEST(CommandExecutorTest, DecrRejectsInvalidInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "hello");

    EXPECT_THROW(
        executor.execute({"DECR", "counter"}),
        std::invalid_argument
    );
    EXPECT_EQ(storage.get("counter").value(), "hello");


}

TEST(CommandExecutorTest, DecrRejectsMissingKeyArgument) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"DECR"}), std::invalid_argument);


}

TEST(CommandExecutorTest, DecrRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(
        executor.execute({"DECR", "counter", "extra"}),
        std::invalid_argument
    );


}

// MGET

TEST(CommandExecutorTest, MGetReturnsMultipleValues) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    RespValue result = executor.execute({"MGET", "name", "city"});
    const auto& values = getArray(result);

    ASSERT_EQ(values.size(), 2);
    EXPECT_EQ(getBulkString(values[0]), "Harsh");
    EXPECT_EQ(getBulkString(values[1]), "Delhi");

    }


TEST(CommandExecutorTest, MGetHandlesMissingKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    storage.set("name", "Harsh");

    RespValue result = executor.execute({"MGET", "name", "country"});
    const auto& values = getArray(result);

    ASSERT_EQ(values.size(), 2);
    EXPECT_EQ(getBulkString(values[0]), "Harsh");
    EXPECT_NE(std::get_if<std::monostate>(&values[1].data), nullptr);

    }


TEST(CommandExecutorTest, MGetPreservesKeyOrder) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    executor.execute({"SET", "a", "Apple"});
    executor.execute({"SET", "b", "Banana"});
    executor.execute({"SET", "c", "Cherry"});

    RespValue result = executor.execute({"MGET", "c", "a", "b"});
    const auto& values = getArray(result);

    ASSERT_EQ(values.size(), 3);

    EXPECT_EQ(getBulkString(values[0]), "Cherry");
    EXPECT_EQ(getBulkString(values[1]), "Apple");
    EXPECT_EQ(getBulkString(values[2]), "Banana");

    }


TEST(CommandExecutorTest, MGetRejectsMissingKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"MGET"}), std::invalid_argument);


}

// MSET

TEST(CommandExecutorTest, MSetStoresMultipleValues) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(
        getSimpleString(executor.execute(
            {"MSET", "name", "Harsh", "city", "Delhi"}
        )),
        "OK"
    );

    EXPECT_EQ(storage.get("name").value(), "Harsh");
    EXPECT_EQ(storage.get("city").value(), "Delhi");
    EXPECT_EQ(storage.dbsize(), 2);


}

TEST(CommandExecutorTest, MSetOverwritesExistingValues) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Old");
    executor.execute({"MSET", "name", "New", "city", "Delhi"});

    EXPECT_EQ(storage.get("name").value(), "New");
    EXPECT_EQ(storage.dbsize(), 2);


}

TEST(CommandExecutorTest, MSetRejectsMissingArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"MSET"}), std::invalid_argument);


}

TEST(CommandExecutorTest, MSetRejectsUnpairedArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(
        executor.execute({"MSET", "name", "Harsh", "city"}),
        std::invalid_argument
    );


}

// DBSIZE

TEST(CommandExecutorTest, DbSizeReturnsZeroForEmptyDatabase) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getInteger(executor.execute({"DBSIZE"})), 0);


}

TEST(CommandExecutorTest, DbSizeCountsStoredKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(getInteger(executor.execute({"DBSIZE"})), 2);


}

TEST(CommandExecutorTest, DbSizeDoesNotCountOverwrittenKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("name", "Tomar");

    EXPECT_EQ(getInteger(executor.execute({"DBSIZE"})), 1);


}

TEST(CommandExecutorTest, DbSizeRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(
        executor.execute({"DBSIZE", "extra"}),
        std::invalid_argument
    );


}

// FLUSHDB

TEST(CommandExecutorTest, FlushDBClearsAllKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(getSimpleString(executor.execute({"FLUSHDB"})), "OK");
    EXPECT_EQ(storage.dbsize(), 0);
    EXPECT_FALSE(storage.exists("name"));
    EXPECT_FALSE(storage.exists("city"));


}

TEST(CommandExecutorTest, FlushDBWorksOnEmptyDatabase) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(getSimpleString(executor.execute({"FLUSHDB"})), "OK");
    EXPECT_EQ(storage.dbsize(), 0);


}

TEST(CommandExecutorTest, CanStoreKeysAfterFlushDB) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");

    EXPECT_EQ(getSimpleString(executor.execute({"FLUSHDB"})), "OK");
    EXPECT_EQ(
        getSimpleString(executor.execute({"SET", "city", "Delhi"})),
        "OK"
    );
    EXPECT_EQ(getBulkString(executor.execute({"GET", "city"})), "Delhi");
    EXPECT_EQ(getInteger(executor.execute({"DBSIZE"})), 1);


}

TEST(CommandExecutorTest, FlushDBRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(
        executor.execute({"FLUSHDB", "extra"}),
        std::invalid_argument
    );


}

// Invalid and unknown commands

TEST(CommandExecutorTest, EmptyCommandThrows) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({}), std::invalid_argument);


}

TEST(CommandExecutorTest, UnknownCommandThrows) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(
        executor.execute({"UNKNOWN"}),
        std::invalid_argument
    );


}
