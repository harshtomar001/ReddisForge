#include "resp2_encoder.h"
#include <stdexcept>
#include <string>
#include <variant>

std::string Resp2Encoder::encode(const RespValue& response) const {

    if (const auto* simple =
            std::get_if<SimpleString>(&response.data)) {

        if (simple->value.find_first_of("\r\n") != std::string::npos) {

            throw std::invalid_argument(
                "Simple strings cannot contain CR or LF"
            );

        }

        return "+" + simple->value + "\r\n";

        }

    if (const auto* integer =
            std::get_if<long long>(&response.data)) {

        return ":" + std::to_string(*integer) + "\r\n";
            }

    if (const auto* bulk =
        std::get_if<BulkString>(&response.data)) {

        return "$" + std::to_string(bulk->value.size()) + "\r\n" +
               bulk->value + "\r\n";
        }

    if (std::holds_alternative<std::monostate>(response.data)) {

        return "$-1\r\n";

    }

    if (const auto* error = std::get_if<RespError>(&response.data)) {

        if (error->message.find_first_of("\r\n") != std::string::npos) {

            throw std::invalid_argument(
                "Simple strings cannot contain CR or LF"
            );

        }

        return "-ERR " + error->message +"\r\n";

    }

    if (const auto* vector =
        std::get_if<std::shared_ptr<std::vector<RespValue>>>(&response.data)) {

        if (!*vector) {
            throw std::invalid_argument("RESP array cannot be null.");
        }

        std::string result = "*" + std::to_string((*vector)->size()) + "\r\n";

        for (const auto& element : **vector) {
            result += encode(element);
        }

        return result;
    }

    throw std::invalid_argument(
        "Response type not implemented yet"
    );
}
