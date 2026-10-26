#ifndef REDISFORGE_STORAGE_ENGINE_H
#define REDISFORGE_STORAGE_ENGINE_H
#include <unordered_map>
#include <optional>
#include <string>
#include <vector>
#include <utility>
#include <cstddef>


class StorageEngine {
    private:

        std::unordered_map<std::string, std::string> data_;

    public:

        void set( const std::string& key,const std::string& value);

        std::optional <std::string> get(const std::string& key) const;

        bool exists(const std::string& key) const;

        bool del(const std::string& key);

        long long incr(const std::string& key);

        long long decr(const std::string& key);

        std::vector<std::optional<std::string>> mget(std::vector<std::string> keys) const; //  fetching multiple  keys - value

        void mset(const std::vector<std::pair<std::string, std::string>>& entries); //  storing multiples key

        std::size_t dbsize() const;

        void flushdb();
};


#endif //REDISFORGE_STORAGE_ENGINE_H
