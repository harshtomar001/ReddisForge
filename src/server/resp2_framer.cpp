#include "resp2_framer.h"

#include <algorithm>
#include <charconv>
#include <vector>
#include <system_error>
using namespace std;



FrameResult Resp2Framer::find_frame(std::string_view input) const {

    if (input.empty()) {
        return FrameResult{ FrameStatus::Incomplete , 0 };
    }

    std::size_t arrayHeaderEnd = input.find("\r\n");

    if (arrayHeaderEnd == std::string::npos) {

        return FrameResult{ FrameStatus::Incomplete , 0 };
    }

    const std::string_view header(input.data() , arrayHeaderEnd);

    if (header.size() < 2 || header.front() != '*' ) {

        return FrameResult{ FrameStatus::Malformed , 0 };
    }

    std::size_t commandLength = 0;

    const char* countStart = header.data() + 1;
    const char* countEnd = header.data() + header.size();

    const auto [countPtr , countError] =
        std::from_chars(countStart , countEnd , commandLength);

    if (countPtr != countEnd || countError !=std::errc{}) {

        return FrameResult{ FrameStatus::Malformed , 0 };

    }

    constexpr std::size_t MAX_ARRAY_ELEMENTS = 1024;

    if (commandLength == 0 || commandLength > MAX_ARRAY_ELEMENTS) {

        return FrameResult{ FrameStatus::Malformed , 0};

    }


    std::size_t pos = arrayHeaderEnd + 2;

    for (std::size_t i = 0; i < commandLength; i++) {

        if( pos >= input.size() ) {
            return FrameResult{FrameStatus::Incomplete , 0};
        }

        if (input[pos] !='$') {
            return FrameResult{FrameStatus::Malformed , 0};
        }

        std::size_t stringHeaderEnd = input.find( "\r\n" , pos);

        if (stringHeaderEnd == std::string::npos) {

            return FrameResult{FrameStatus::Incomplete , 0};

        }

        std::string_view stringBulkHeader(input.data() + pos , //  starting position
                                        stringHeaderEnd - pos //  length of the string to fetch
                                        );

        if (stringBulkHeader.size() < 2 || stringBulkHeader.front() != '$') {

            return {FrameStatus::Malformed, 0};

        }

        std::size_t stringValueLength  = 0;

        const char* stringValueStart = stringBulkHeader.data()  + 1;
        const char* stringValueEnd  = stringBulkHeader.data() + stringBulkHeader.size();

        const auto [stringCount , stringError] =
            std::from_chars(stringValueStart , stringValueEnd , stringValueLength);

        if (stringError != std::errc{} || stringCount != stringValueEnd) {

            return FrameResult{FrameStatus::Malformed , 0};

        }

        constexpr std::size_t MAX_BULK_STRING_LENGTH = 1024 * 1024;

        if (stringValueLength > MAX_BULK_STRING_LENGTH) {

            return FrameResult{FrameStatus::Malformed , 0};

        }

        std::size_t valueStart = stringHeaderEnd + 2;

        if (valueStart > input.size() ||
           stringValueLength > input.size() - valueStart ||
           input.size() - valueStart - stringValueLength < 2) {

            return FrameResult{FrameStatus::Incomplete , 0};

           }

        const std::size_t terminatorPos = valueStart + stringValueLength;

        if (input[terminatorPos] != '\r' ||
            input[terminatorPos + 1] != '\n') {

            return FrameResult{FrameStatus::Malformed , 0};

        }

        pos = terminatorPos + 2;

    }


    return  FrameResult{FrameStatus::Complete , pos};

}