#ifndef LUMENCPP_EXCEPTIONS_H
#define LUMENCPP_EXCEPTIONS_H

#include <exception>
#include <string>
#include <utility>

#include "source_region.h"

namespace lumen {

struct Exception : std::exception {};

struct ParseError : Exception {
    [[nodiscard]] ParseError(
        std::string description, std::string filename,
        SourceRegion source) noexcept
    : description{std::move(description)}, filename{std::move(filename)},
      source{source} {}

    [[nodiscard]] const char* what() const noexcept override {
        static std::string formatted;
        formatted = "in " + filename + ": " + description + " (line " +
                    std::to_string(source.begin.line) + ", column " +
                    std::to_string(source.begin.column) + ")";

        return formatted.c_str();
    }

    std::string description;
    std::string filename;
    SourceRegion source;
};

struct TypeMismatch : Exception {
    [[nodiscard]] TypeMismatch(std::string description) noexcept
    : description{std::move(description)} {}

    [[nodiscard]] const char* what() const noexcept override {
        static std::string formatted;
        formatted = "type mismatch: " + description;

        return formatted.c_str();
    }

    std::string description;
};

} // namespace lumen

#endif
