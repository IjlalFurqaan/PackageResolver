
#include "json_parser.h"
#include "exceptions.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace pkgr {

std::string JsonString::to_string(int) const {
    return "\"" + value_ + "\"";
}

std::string JsonNumber::to_string(int) const {
    return std::to_string(value_);
}

std::string JsonBool::to_string(int) const {
    return value_ ? "true" : "false";
}

std::string JsonNull::to_string(int) const {
    return "null";
}

void JsonObject::set(const std::string& key, std::unique_ptr<JsonValue> value) {

    for (auto& entry : entries_) {
        if (entry.first == key) {
            entry.second = std::move(value);
            return;
        }
    }
    entries_.emplace_back(key, std::move(value));
}

const JsonValue* JsonObject::get(const std::string& key) const {
    for (const auto& entry : entries_) {
        if (entry.first == key) {
            return entry.second.get();
        }
    }
    return nullptr;
}

bool JsonObject::has(const std::string& key) const {
    return get(key) != nullptr;
}

const std::string& JsonObject::get_string(const std::string& key) const {
    const auto* val = get(key);
    if (!val || !val->is_string()) {
        throw JsonParseError("Expected string for key '" + key + "'");
    }
    return static_cast<const JsonString*>(val)->value();
}

int JsonObject::get_number(const std::string& key) const {
    const auto* val = get(key);
    if (!val || !val->is_number()) {
        throw JsonParseError("Expected number for key '" + key + "'");
    }
    return static_cast<const JsonNumber*>(val)->value();
}

bool JsonObject::get_bool(const std::string& key) const {
    const auto* val = get(key);
    if (!val || !val->is_boolean()) {
        throw JsonParseError("Expected boolean for key '" + key + "'");
    }
    return static_cast<const JsonBool*>(val)->value();
}

const JsonObject& JsonObject::get_object(const std::string& key) const {
    const auto* val = get(key);
    if (!val || !val->is_object()) {
        throw JsonParseError("Expected object for key '" + key + "'");
    }
    return *static_cast<const JsonObject*>(val);
}

std::string JsonObject::to_string(int indent) const {
    if (entries_.empty()) return "{}";

    std::string result = "{\n";
    std::string pad(static_cast<size_t>(indent + 2), ' ');
    std::string end_pad(static_cast<size_t>(indent), ' ');

    for (size_t i = 0; i < entries_.size(); ++i) {
        result += pad + "\"" + entries_[i].first + "\": " + 
                  entries_[i].second->to_string(indent + 2);
        if (i < entries_.size() - 1) result += ",";
        result += "\n";
    }
    result += end_pad + "}";
    return result;
}

void JsonArray::push(std::unique_ptr<JsonValue> value) {
    elements_.push_back(std::move(value));
}

const JsonValue* JsonArray::at(size_t index) const {
    if (index >= elements_.size()) return nullptr;
    return elements_[index].get();
}

std::string JsonArray::to_string(int indent) const {
    if (elements_.empty()) return "[]";

    std::string result = "[\n";
    std::string pad(static_cast<size_t>(indent + 2), ' ');
    std::string end_pad(static_cast<size_t>(indent), ' ');

    for (size_t i = 0; i < elements_.size(); ++i) {
        result += pad + elements_[i]->to_string(indent + 2);
        if (i < elements_.size() - 1) result += ",";
        result += "\n";
    }
    result += end_pad + "]";
    return result;
}

JsonTokenizer::JsonTokenizer(const std::string& input)
    : input_(input), pos_(0), line_(1), column_(1), 
      has_peeked_(false), peeked_token_{} {}

char JsonTokenizer::current() const {
    return pos_ < input_.size() ? input_[pos_] : '\0';
}

char JsonTokenizer::advance() {
    char c = current();
    ++pos_;
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool JsonTokenizer::at_end() const {
    return pos_ >= input_.size();
}

void JsonTokenizer::skip_whitespace() {
    while (!at_end()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            advance();
        } else {
            break;
        }
    }
}

Token JsonTokenizer::read_string() {
    int start_line = line_;
    int start_col = column_;

    advance(); 
    std::string value;

    while (!at_end() && current() != '"') {
        if (current() == '\\') {
            advance(); 
            char escaped = advance();
            switch (escaped) {
                case '"':  value += '"'; break;
                case '\\': value += '\\'; break;
                case '/':  value += '/'; break;
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case 'r':  value += '\r'; break;
                default:   value += escaped; break;
            }
        } else {
            value += advance();
        }
    }

    if (at_end()) {
        throw JsonParseError("Unterminated string", start_line, start_col);
    }

    advance(); 
    return { TokenType::STRING, value, start_line, start_col };
}

Token JsonTokenizer::read_number() {
    int start_line = line_;
    int start_col = column_;
    std::string value;

    if (current() == '-') {
        value += advance();
    }

    while (!at_end() && current() >= '0' && current() <= '9') {
        value += advance();
    }

    return { TokenType::NUMBER, value, start_line, start_col };
}

Token JsonTokenizer::read_keyword() {
    int start_line = line_;
    int start_col = column_;
    std::string value;

    while (!at_end() && current() >= 'a' && current() <= 'z') {
        value += advance();
    }

    if (value == "true")  return { TokenType::TRUE_VAL, value, start_line, start_col };
    if (value == "false") return { TokenType::FALSE_VAL, value, start_line, start_col };
    if (value == "null")  return { TokenType::NULL_VAL, value, start_line, start_col };

    throw JsonParseError("Unknown keyword: " + value, start_line, start_col);
}

Token JsonTokenizer::next_token() {
    if (has_peeked_) {
        has_peeked_ = false;
        return peeked_token_;
    }

    skip_whitespace();

    if (at_end()) {
        return { TokenType::END_OF_INPUT, "", line_, column_ };
    }

    int start_line = line_;
    int start_col = column_;
    char c = current();

    switch (c) {
        case '{': advance(); return { TokenType::LBRACE, "{", start_line, start_col };
        case '}': advance(); return { TokenType::RBRACE, "}", start_line, start_col };
        case '[': advance(); return { TokenType::LBRACKET, "[", start_line, start_col };
        case ']': advance(); return { TokenType::RBRACKET, "]", start_line, start_col };
        case ':': advance(); return { TokenType::COLON, ":", start_line, start_col };
        case ',': advance(); return { TokenType::COMMA, ",", start_line, start_col };
        case '"': return read_string();
        default:
            if (c == '-' || (c >= '0' && c <= '9')) return read_number();
            if (c >= 'a' && c <= 'z') return read_keyword();
            throw JsonParseError(
                std::string("Unexpected character: '") + c + "'", 
                start_line, start_col);
    }
}

Token JsonTokenizer::peek_token() {
    if (!has_peeked_) {
        peeked_token_ = next_token();
        has_peeked_ = true;
    }
    return peeked_token_;
}

JsonParser::JsonParser(const std::string& input) : tokenizer_(input) {}

std::unique_ptr<JsonValue> JsonParser::parse(const std::string& json_str) {
    JsonParser parser(json_str);
    return parser.parse_value();
}

std::unique_ptr<JsonValue> JsonParser::parse_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw JsonParseError("Cannot open file: " + file_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return parse(buffer.str());
}

std::unique_ptr<JsonValue> JsonParser::parse_value() {
    Token token = tokenizer_.peek_token();

    switch (token.type) {
        case TokenType::LBRACE:
            return parse_object();

        case TokenType::LBRACKET:
            return parse_array();

        case TokenType::STRING:
            tokenizer_.next_token(); 
            return std::make_unique<JsonString>(token.value);

        case TokenType::NUMBER:
            tokenizer_.next_token(); 
            return std::make_unique<JsonNumber>(std::stoi(token.value));

        case TokenType::TRUE_VAL:
            tokenizer_.next_token();
            return std::make_unique<JsonBool>(true);

        case TokenType::FALSE_VAL:
            tokenizer_.next_token();
            return std::make_unique<JsonBool>(false);

        case TokenType::NULL_VAL:
            tokenizer_.next_token();
            return std::make_unique<JsonNull>();

        default:
            throw JsonParseError(
                "Unexpected token: " + token.value, 
                token.line, token.column);
    }
}

std::unique_ptr<JsonValue> JsonParser::parse_object() {
    expect(TokenType::LBRACE, "object");

    auto obj = std::make_unique<JsonObject>();

    if (tokenizer_.peek_token().type == TokenType::RBRACE) {
        tokenizer_.next_token(); 
        return obj;
    }

    while (true) {

        Token key_token = expect(TokenType::STRING, "object key");

        expect(TokenType::COLON, "object");

        auto value = parse_value();
        obj->set(key_token.value, std::move(value));

        Token next = tokenizer_.peek_token();
        if (next.type == TokenType::COMMA) {
            tokenizer_.next_token(); 
        } else if (next.type == TokenType::RBRACE) {
            tokenizer_.next_token(); 
            break;
        } else {
            throw JsonParseError(
                "Expected ',' or '}' in object", 
                next.line, next.column);
        }
    }

    return obj;
}

std::unique_ptr<JsonValue> JsonParser::parse_array() {
    expect(TokenType::LBRACKET, "array");

    auto arr = std::make_unique<JsonArray>();

    if (tokenizer_.peek_token().type == TokenType::RBRACKET) {
        tokenizer_.next_token(); 
        return arr;
    }

    while (true) {
        arr->push(parse_value());

        Token next = tokenizer_.peek_token();
        if (next.type == TokenType::COMMA) {
            tokenizer_.next_token(); 
        } else if (next.type == TokenType::RBRACKET) {
            tokenizer_.next_token(); 
            break;
        } else {
            throw JsonParseError(
                "Expected ',' or ']' in array", 
                next.line, next.column);
        }
    }

    return arr;
}

Token JsonParser::expect(TokenType type, const std::string& context) {
    Token token = tokenizer_.next_token();
    if (token.type != type) {
        throw JsonParseError(
            "Expected different token in " + context + 
            ", got: " + token.value,
            token.line, token.column);
    }
    return token;
}

} 