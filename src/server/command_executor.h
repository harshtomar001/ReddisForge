#ifndef REDISFORGE_COMMAND_EXECUTOR_H
#define REDISFORGE_COMMAND_EXECUTOR_H
#include "../core/storage_engine.h"


class CommandExecutor {
private:
    StorageEngine& storage_engine_;

public:
    explicit CommandExecutor(StorageEngine &storage_engine);

    std::string execute(const std::vector<std::string>& command);


};


#endif //REDISFORGE_COMMAND_EXECUTOR_H
