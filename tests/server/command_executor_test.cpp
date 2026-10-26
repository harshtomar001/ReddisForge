#include <gtest/gtest.h>

#include "../../src/core/storage_engine.h"
#include "../../src/server/command_executor.h"
#include <stdexcept>
#include <string>
#include <vector>

TEST(CommandExecutorTest, PingReturnsPong) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    const std::vector<std::string> command = {"PING"};

    EXPECT_EQ(executor.execute(command), "PONG");
}



TEST(CommandExecutorTest, SetReturnsOK) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    const std::vector<std::string> command = {"SET", "name", "Harsh"};

    EXPECT_EQ(executor.execute(command), "OK");
    EXPECT_EQ(storage.get("name").value(), "Harsh");
}

TEST(CommandExecutorTest, GetReturnsStoredValue) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    storage.set("name", "Harsh");

    const std::vector<std::string> command = {"GET", "name"};

    EXPECT_EQ(executor.execute(command), "Harsh");
}

TEST(CommandExecutorTest, GetMissingKeyReturnsNil) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    const std::vector<std::string> command = {"GET", "missing"};

    EXPECT_EQ(executor.execute(command), "(nil)");
}

TEST(CommandExecutorTest, SetWithWrongArgumentCountThrows) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    const std::vector<std::string> command = {"SET", "name"};

    EXPECT_THROW(
        executor.execute(command),
        std::invalid_argument
    );
}

TEST(CommandExecutorTest, DelRemovesExistingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");

    EXPECT_EQ(executor.execute({"DEL", "name"}), "1");
    EXPECT_FALSE(storage.exists("name"));


    }

TEST(CommandExecutorTest, DelReturnsZeroForMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(executor.execute({"DEL", "missing"}), "0");


    }

TEST(CommandExecutorTest, DelCountsOnlyDeletedKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(executor.execute({"DEL", "name", "missing", "city"}), "2");
    EXPECT_EQ(storage.dbsize(), 0);


    }

TEST(CommandExecutorTest, DelRejectsMissingKeyArgument) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_THROW(executor.execute({"DEL"}), std::invalid_argument);


    }

TEST(CommandExecutorTest, ExistsReturnsOneForExistingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    storage.set("name", "Harsh");

    EXPECT_EQ(executor.execute({"EXISTS", "name"}), "1");

    }

TEST(CommandExecutorTest, ExistsReturnsZeroForMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    EXPECT_EQ(executor.execute({"EXISTS", "country"}), "0");

    }

TEST(CommandExecutorTest, ExistsCountsMultipleKeysAndOccurrences) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(executor.execute({"EXISTS", "name", "country", "city"}), "2");
    EXPECT_EQ(executor.execute({"EXISTS", "name", "name"}), "2");

    }

TEST(CommandExecutorTest, ExistsRejectsMissingKeyArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    EXPECT_THROW(executor.execute({"EXISTS"}), std::invalid_argument);

    }


TEST(CommandExecutorTest, IncrIncrementsExistingInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "10");

    EXPECT_EQ(executor.execute({"INCR", "counter"}), "11");
    EXPECT_EQ(storage.get("counter").value(), "11");


}

TEST(CommandExecutorTest, IncrInitializesMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(executor.execute({"INCR", "counter"}), "1");
    EXPECT_EQ(storage.get("counter").value(), "1");


}

TEST(CommandExecutorTest, IncrRejectsInvalidInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "hello");

    EXPECT_THROW(executor.execute({"INCR", "counter"}), std::invalid_argument);
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

TEST(CommandExecutorTest, DecrDecrementsExistingInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    storage.set("counter", "10");

    EXPECT_EQ(executor.execute({"DECR", "counter"}), "9");
    EXPECT_EQ(storage.get("counter").value(), "9");


}

TEST(CommandExecutorTest, DecrInitializesMissingKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);


    EXPECT_EQ(executor.execute({"DECR", "counter"}), "-1");
    EXPECT_EQ(storage.get("counter").value(), "-1");


}

TEST(CommandExecutorTest, DecrRejectsInvalidInteger) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    storage.set("counter", "hello");

    EXPECT_THROW(executor.execute({"DECR", "counter"}), std::invalid_argument);
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

TEST(CommandExecutorTest, MGetReturnsMultipleValues) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(
        executor.execute({"MGET", "name", "city"}),
        "[Harsh, Delhi]"
    );
    

    }

TEST(CommandExecutorTest, MGetHandlesMissingKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "Harsh");

    EXPECT_EQ(
        executor.execute({"MGET", "name", "country"}),
        "[Harsh, (nil)]"
    );
    

    }

TEST(CommandExecutorTest, MGetPreservesKeyOrder) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("first", "A");
    storage.set("second", "B");

    EXPECT_EQ(
        executor.execute({"MGET", "second", "first"}),
        "[B, A]"
    );
    

    }

TEST(CommandExecutorTest, MGetRejectsMissingKeyArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_THROW(executor.execute({"MGET"}), std::invalid_argument);
    

    }

TEST(CommandExecutorTest, MSetStoresMultipleKeyValuePairs) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_EQ(
        executor.execute({"MSET", "name", "Harsh", "city", "Delhi"}),
        "OK"
    );

    EXPECT_EQ(storage.get("name").value(), "Harsh");
    EXPECT_EQ(storage.get("city").value(), "Delhi");
    

    }

TEST(CommandExecutorTest, MSetOverwritesExistingValues) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "OldName");

    EXPECT_EQ(executor.execute({"MSET", "name", "Harsh"}), "OK");
    EXPECT_EQ(storage.get("name").value(), "Harsh");
    

    }

TEST(CommandExecutorTest, MSetRejectsMissingKeyValuePair) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_THROW(executor.execute({"MSET"}), std::invalid_argument);
    

    }

TEST(CommandExecutorTest, MSetRejectsUnmatchedKey) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_THROW(
        executor.execute({"MSET", "name", "Harsh", "city"}),
        std::invalid_argument
    );
    

    }

TEST(CommandExecutorTest, DbSizeReturnsZeroForEmptyDatabase) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_EQ(executor.execute({"DBSIZE"}), "0");
    

    }

TEST(CommandExecutorTest, DbSizeCountsStoredKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(executor.execute({"DBSIZE"}), "2");
    

    }

TEST(CommandExecutorTest, DbSizeDoesNotCountOverwrittenKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "Harsh");
    storage.set("name", "Tomar");

    EXPECT_EQ(executor.execute({"DBSIZE"}), "1");
    

    }

TEST(CommandExecutorTest, DbSizeRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_THROW(
        executor.execute({"DBSIZE", "extra"}),
        std::invalid_argument
    );
    

    }


TEST(CommandExecutorTest, FlushDBClearsAllKeys) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "Harsh");
    storage.set("city", "Delhi");

    EXPECT_EQ(executor.execute({"FLUSHDB"}), "OK");
    EXPECT_EQ(storage.dbsize(), 0);
    EXPECT_FALSE(storage.exists("name"));
    EXPECT_FALSE(storage.exists("city"));
    

    }

TEST(CommandExecutorTest, FlushDBWorksOnEmptyDatabase) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_EQ(executor.execute({"FLUSHDB"}), "OK");
    EXPECT_EQ(storage.dbsize(), 0);
    

    }

TEST(CommandExecutorTest, CanStoreKeysAfterFlushDB) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    storage.set("name", "Harsh");

    EXPECT_EQ(executor.execute({"FLUSHDB"}), "OK");

    EXPECT_EQ(executor.execute({"SET", "city", "Delhi"}), "OK");
    EXPECT_EQ(executor.execute({"GET", "city"}), "Delhi");
    EXPECT_EQ(executor.execute({"DBSIZE"}), "1");
    

    }

TEST(CommandExecutorTest, FlushDBRejectsExtraArguments) {
    StorageEngine storage;
    CommandExecutor executor(storage);

    
    EXPECT_THROW(
        executor.execute({"FLUSHDB", "extra"}),
        std::invalid_argument
    );
    

    }
