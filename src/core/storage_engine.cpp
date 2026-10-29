#include "storage_engine.h"
#include <charconv>
#include <limits>
#include <stdexcept>
#include <string>


void StorageEngine::set(const std::string& key, const std::string& value) {
    data_[key] = value;
}

std::optional<std::string> StorageEngine::get(const std::string& key) const {
    const auto it = data_.find(key);

    if (it == data_.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool StorageEngine::exists(const std::string& key) const {

    return data_.find(key) != data_.end();

}

bool StorageEngine::del(const std::string &key) {

    return data_.erase(key) > 0;

    //For an unordered_map, erasing by key returns the number of elements removed.
    // 1 means the key existed and was removed.
    // 0 means the key wasn't present.

}

long long StorageEngine::incr(const std::string &key) {

    const auto it  = data_.find(key);
    long long current = 0;

    if (it != data_.end()) {

        const std::string& storedValue = it->second;

        const auto [ptr,  error] = std::from_chars(
            storedValue.data(),
            storedValue.data() + storedValue.size(),
            current
            );

        if (error == std::errc::result_out_of_range) {
            throw std::overflow_error("Integer value is out of range");
        }

        if (error != std::errc{} ||
            ptr != storedValue.data() + storedValue.size()) {

            // std::from_chars can successfully parse the beginning of a string without consuming the entire string.
            //  so we check weather ptr consumes all the chars of the storedValue or not

            throw std::invalid_argument("Value is not a valid integer");

        }

        if (current == std::numeric_limits<long long>::max()) {
            throw std::overflow_error("Integer overflow");
        }

    }
    const long long result  = current + 1;

    data_[key] = std::to_string(result);

    return result;

}

long long StorageEngine::decr(const std::string& key) {
    const auto it  = data_.find(key);
    long long current = 0;
    if (it != data_.end()) {

        const std::string& storedValue = it->second;

        const auto [ptr,  error] = std::from_chars(
            storedValue.data(),
            storedValue.data() + storedValue.size() ,
            current
            );

        if (error == std::errc::result_out_of_range) {

            throw std::overflow_error("Integer value is out of range");

        }

        if (error != std::errc{} ||
            ptr != storedValue.data() + storedValue.size()) {

            throw std::invalid_argument("Value is not a valid integer");
            }

        if (current == std::numeric_limits<long long>::min()) {
            throw std::overflow_error("Integer overflow");
        }
    }

    const long long result  = current - 1;
    data_[key] = std::to_string(result);
    return result;

}

std::vector<std::optional<std::string>> StorageEngine::mget(std::vector<std::string> keys) const {

    std::vector<std::optional<std::string>> result;
    result.reserve(keys.size());

    for (const auto& key : keys) {
        result.push_back(get(key));
    }

    return result;

}

void StorageEngine::mset(
    const std::vector<std::pair<std::string, std::string>>& entries
) {
    for (const auto& [key, value] : entries) {
        set(key, value);
    }
}


std::size_t StorageEngine::dbsize() const {
    return data_.size();

}

void StorageEngine::flushdb() {
    data_.clear();
}