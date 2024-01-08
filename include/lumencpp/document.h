#ifndef LUMENCPP_DOCUMENT_H
#define LUMENCPP_DOCUMENT_H

#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <utility>

#include "lexer.h"
#include "parser.h"
#include "value.h"

namespace lumen {

struct Document {
    [[nodiscard]] Document(Object data = {}) : data{std::move(data)} {}

    [[nodiscard]] Document(std::initializer_list<Object::value_type> data)
    : data{data} {}

    [[nodiscard]] auto begin() noexcept { return data.begin(); }
    [[nodiscard]] auto end() noexcept { return data.end(); }

    [[nodiscard]] auto begin() const noexcept { return data.begin(); }
    [[nodiscard]] auto end() const noexcept { return data.end(); }

    [[nodiscard]] auto contains(const Object::key_type& key) const noexcept {
        return data.contains(key);
    }

    auto insert(Object::value_type pair) noexcept {
        return data.insert(std::move(pair));
    }

    [[nodiscard]] const auto& at(const Object::key_type& key) const {
        return data.at(key);
    }

    [[nodiscard]] const auto& operator[](const Object::key_type& key) const {
        return data.at(key);
    }

    [[nodiscard]] auto& operator[](const Object::key_type& key) noexcept {
        return data[key];
    }

    Object data;
};

[[nodiscard]] inline Document
parse(std::string_view source, Object predefined = {}) {
    return Parser{}.parse(Lexer{}.lex(source), std::move(predefined));
}

[[nodiscard]] inline auto
parse_file(const std::filesystem::path& path, Object predefined = {}) {
    std::ostringstream buffer;
    buffer << std::ifstream{path}.rdbuf();

    return parse(buffer.str(), std::move(predefined));
}

} // namespace lumen

#endif
