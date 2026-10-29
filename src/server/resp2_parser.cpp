#include "resp2_parser.h"

#include <charconv>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

std::vector<std::string> Resp2Parser::parse(const std::string& input) {

    if (input.empty()) {
        throw std::invalid_argument("Input cannot be empty");
    }

    // 1. Find and validate the array header.
    const std::size_t headerEnd = input.find("\r\n");

    if (headerEnd == std::string::npos) {
        throw std::invalid_argument("Missing array header CRLF");
    }

    const std::string_view header(input.data(), headerEnd);
    // input.data() => It gives us a pointer to the beginning of the string's character storage.

    if (header.size() < 2 || header.front() != '*') {

        throw std::invalid_argument("Expected RESP2 array header");

    }

    // 2. Parse the number of array elements.
    std::size_t commandLength = 0;

    const char* countStart = header.data() + 1;
    const char* countEnd = header.data() + header.size();

    const auto [countPtr, countError] =
        std::from_chars(countStart, countEnd, commandLength);

    // std::from_chars() returns two important pieces of information:

    // A pointer indicating how much of the input was consumed.
    // An error code indicating whether conversion succeeded.

    if (countError != std::errc{} || countPtr != countEnd) {
        throw std::invalid_argument("Invalid array length");
    }

    // Bound resource usage. This initial limit can be configured later.
    constexpr std::size_t MAX_ARRAY_ELEMENTS = 1024;

    if (commandLength == 0 || commandLength > MAX_ARRAY_ELEMENTS) {
        throw std::invalid_argument("Array length out of range");
    }

    std::vector<std::string> command;
    command.reserve(commandLength);

    // The cursor starts immediately after the array header.
    std::size_t pos = headerEnd + 2; // \r\n +2 => bulk string start

    // 3. Parse each bulk string.
    for (std::size_t i = 0; i < commandLength; ++i) {

        if (pos >= input.size() || input[pos] != '$') {
            throw std::invalid_argument("Expected bulk string header");
        }

        const std::size_t bulkHeaderEnd = input.find("\r\n", pos);

        if (bulkHeaderEnd == std::string::npos) {
            throw std::invalid_argument("Missing bulk string header CRLF");
        }

        const std::string_view bulkHeader(
            input.data() + pos,  //  start position
            bulkHeaderEnd - pos //  length of the string
        );

        if (bulkHeader.size() < 2 || bulkHeader.front() != '$') {
            throw std::invalid_argument("Invalid bulk string header");
        }

        // 4. Parse the declared bulk string length.
        std::size_t valueLength = 0;

        const char* lengthStart = bulkHeader.data() + 1;
        const char* lengthEnd = bulkHeader.data() + bulkHeader.size();

        const auto [lengthPtr, lengthError] =
            std::from_chars(lengthStart, lengthEnd, valueLength);

        if (lengthError != std::errc{} || lengthPtr != lengthEnd) {
            throw std::invalid_argument("Invalid bulk string length");
        }

        constexpr std::size_t MAX_BULK_STRING_LENGTH = 1024 * 1024;

        if (valueLength > MAX_BULK_STRING_LENGTH) {
            throw std::invalid_argument("Bulk string exceeds size limit");
        }

        // 5. Move to the beginning of the value.
        const std::size_t valueStart = bulkHeaderEnd + 2;

        // Check that the declared value and its trailing CRLF fit.
        if (valueStart > input.size() ||
            valueLength > input.size() - valueStart ||
            input.size() - valueStart - valueLength < 2) {
            throw std::invalid_argument("Incomplete bulk string");
        }

        // 6. Read exactly the declared number of bytes.
        std::string value = input.substr(valueStart, valueLength);

        // 7. Validate the trailing CRLF.
        const std::size_t terminatorPos = valueStart + valueLength;

        if (input[terminatorPos] != '\r' ||
            input[terminatorPos + 1] != '\n') {
            throw std::invalid_argument("Invalid bulk string terminator");
        }

        command.push_back(std::move(value));

        // 8. Advance the cursor past the value and its CRLF.
        pos = terminatorPos + 2;
    }

    // Reject extra bytes after the expected command.
    if (pos != input.size()) {
        throw std::invalid_argument("Unexpected trailing data");
    }

    return command;
}