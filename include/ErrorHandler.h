#pragma once

#include <stdexcept>
#include <string>

class CompilerError : public std::runtime_error {
public:
    CompilerError(const std::string& message, int line, int column)
        : std::runtime_error(buildMessage(message, line, column))
        , m_line(line)
        , m_column(column) {}

    int line() const { return m_line; }
    int column() const { return m_column; }

private:
    static std::string buildMessage(const std::string& message, int line, int column) {
        return "Error on line " + std::to_string(line) + ":" + std::to_string(column) + " " + message;
    }

    int m_line;
    int m_column;
};

class LexicalError : public CompilerError {
    using CompilerError::CompilerError;
};

class SyntaxError : public CompilerError {
    using CompilerError::CompilerError;
};

class SemanticError : public CompilerError {
    using CompilerError::CompilerError;
};
