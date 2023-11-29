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
    [[nodiscard]] Document(Value::Object data = {}) : data{std::move(data)} {}

    [[nodiscard]] Document(
        std::initializer_list<Value::Object::value_type> data)
    : data{data} {}

    [[nodiscard]] auto begin() noexcept { return data.begin(); }
    [[nodiscard]] auto end() noexcept { return data.end(); }

    [[nodiscard]] auto begin() const noexcept { return data.begin(); }
    [[nodiscard]] auto end() const noexcept { return data.end(); }

    [[nodiscard]] auto
    contains(const Value::Object::key_type& key) const noexcept {
        return data.contains(key);
    }

    auto insert(Value::Object::value_type pair) noexcept {
        return data.insert(std::move(pair));
    }

    [[nodiscard]] const auto& at(const Value::Object::key_type& key) const {
        return data.at(key);
    }

    [[nodiscard]] const auto&
    operator[](const Value::Object::key_type& key) const {
        return data.at(key);
    }

    [[nodiscard]] auto&
    operator[](const Value::Object::key_type& key) noexcept {
        return data[key];
    }

    Value::Object data;
};

[[nodiscard]] Document parse(std::string_view source) {
    return Parser{}.parse(Lexer{}.lex(source));
}

[[nodiscard]] auto parse_file(const std::filesystem::path& path) {
    std::ostringstream buffer;
    buffer << std::ifstream{path}.rdbuf();

    return parse(buffer.str());
}

} // namespace lumen

#endif
