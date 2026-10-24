#include <iostream>
#include <string_view>
#include <vector>

int main() {
    const std::vector<std::string_view> components{
        "Storage Engine",
        "RESP Protocol",
        "TCP Server"
    };

    std::cout << "RedisForge development environment ready!\n";
    std::cout << "C++ standard: " << __cplusplus << '\n';
    std::cout << "Initial components: " << components.size() << '\n';

    for (const auto component : components) {
        std::cout << " - " << component << '\n';
    }

    return 0;
}