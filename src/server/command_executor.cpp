#include "command_executor.h"
#include <stdexcept>
#include <cstddef>

CommandExecutor::CommandExecutor(StorageEngine &storage_engine):
storage_engine_(storage_engine) {}


std::string CommandExecutor::execute(const std::vector<std::string>& command) {

    if (command.empty()) {
        throw std::invalid_argument("Command cannot be empty");
    }

    if (command[0] == "PING") {
        if (command.size() != 1) {
            throw std::invalid_argument("PING expects no arguments");
        }

        return "PONG";
    }

    if (command[0] == "SET") {
        if (command.size() != 3) {
            throw std::invalid_argument("SET expects a key and a value");
        }

        storage_engine_.set(command[1], command[2]);
        return "OK";
    }

    if (command[0] == "GET") {
        if (command.size() != 2) {
            throw std::invalid_argument("GET expects a key");
        }

        const auto value = storage_engine_.get(command[1]);

        if (!value.has_value()) {
            return "(nil)";
        }

        return *value;
    }

    if (command[0] == "DEL") {

        if (command.size() < 2) {
            throw std::invalid_argument("DEL expects a key");
        }

        int keys_deleted = 0;

        for (std::size_t i = 1; i < command.size(); i++) {

            std::string key = command[i];

            keys_deleted += storage_engine_.del(key);

        }

        return std::to_string(keys_deleted);

    }

    if (command[0] == "EXISTS") {

        if (command.size() < 2) {
            throw std::invalid_argument("EXISTS expects a key");
        }

        int keys_exists = 0;

        for (std::size_t i = 1; i < command.size(); i++) {

            std::string key = command[i];

            keys_exists += storage_engine_.exists(key);

        }

        return std::to_string(keys_exists);

    }

    if (command[0] == "INCR") {
        if (command.size() != 2) {
            throw std::invalid_argument("INCR expects exactly 1 key");
        }

        std::string key = command[1];

        return std::to_string(storage_engine_.incr(key));

    }

    if (command[0] == "DECR") {

        if (command.size() != 2) {
            throw std::invalid_argument("DECR expects exactly 1 key");
        }

        std::string key = command[1];

        return std::to_string(storage_engine_.decr(key));

    }

    if (command[0] == "MGET") {

        if (command.size() < 2) {
            throw std::invalid_argument("MGET expects atleast 1 key");
        }

        std::vector<std::string> keys(command.begin() + 1, command.end());

        auto values = storage_engine_.mget(keys);

        std::string result = "[";

        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i > 0) {
                result += ", ";
            }

            if (values[i].has_value()) {
                result += *values[i];
            } else {
                result += "(nil)";
            }
        }

        result += "]";

        return result;

    }

    if (command[0] == "MSET") {

        if (command.size() < 3) {
            throw std::invalid_argument("MSET expects atleast 1 key-value pair");
        }

        if ( (command.size() -1) % 2 !=0 ) throw std::invalid_argument("Invalid key-value pairs");


        std::vector<std::pair<std::string, std::string>> keys;

        for (std::size_t i = 1; i < command.size(); i+=2) {

            std::string key = command[i];
            std::string value = command[i + 1];

            keys.push_back(std::make_pair(key, value));


        }

        storage_engine_.mset(keys);

        return "OK";

    }

    if (command[0] == "DBSIZE") {

        if (command.size() != 1) {
            throw std::invalid_argument("DBSIZE expects exactly 1 argument");
        }

        return std::to_string(storage_engine_.dbsize());

    }

    if (command[0] == "FLUSHDB") {

        if (command.size() != 1) {
            throw std::invalid_argument("FLUSHDB expects exactly 1 argument");
        }

        storage_engine_.flushdb();

        return "OK";
    }



    throw std::invalid_argument("Unknown command");

}