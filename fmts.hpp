#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
    template<>
    struct formatter<miniplc0::ErrorCode> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const miniplc0::ErrorCode &p, FormatContext &ctx) {
            std::string name;
            switch (p) {
                case miniplc0::ErrNoError:
                    name = "No error.";
                    break;
                case miniplc0::ErrStreamError:
                    name = "Stream error.";
                    break;
                case miniplc0::ErrEOF:
                    name = "EOF";
                    break;
                case miniplc0::ErrInvalidInput:
                    name = "The input is invalid.";
                    break;
                case miniplc0::ErrInvalidIdentifier:
                    name = "Identifier is invalid";
                    break;
                case miniplc0::ErrIntegerOverflow:
                    name = "The integer is too big(int64_t).";
                    break;
                case miniplc0::ErrNoBegin:
                    name = "The program should start with 'begin'.";
                    break;
                case miniplc0::ErrNoEnd:
                    name = "The program should end with 'end'.";
                    break;
                case miniplc0::ErrNeedIdentifier:
                    name = "Need an identifier here.";
                    break;
                case miniplc0::ErrConstantNeedValue:
                    name = "The constant need a value to initialize.";
                    break;
                case miniplc0::ErrNoSemicolon:
                    name = "Zai? Wei shen me bu xie fen hao.";
                    break;
                case miniplc0::ErrInvalidVariableDeclaration:
                    name = "The declaration is invalid.";
                    break;
                case miniplc0::ErrIncompleteExpression:
                    name = "The expression is incomplete.";
                    break;
                case miniplc0::ErrNotDeclared:
                    name = "The variable or constant must be declared before being used.";
                    break;
                case miniplc0::ErrAssignToConstant:
                    name = "Trying to assign value to a constant.";
                    break;
                case miniplc0::ErrDuplicateDeclaration:
                    name = "The variable or constant has been declared.";
                    break;
                case miniplc0::ErrNotInitialized:
                    name = "The variable has not been initialized.";
                    break;
                case miniplc0::ErrInvalidAssignment:
                    name = "The assignment statement is invalid.";
                    break;
                case miniplc0::ErrInvalidPrint:
                    name = "The output statement is invalid.";
                    break;
                case miniplc0::ErrTypeSpecifier:
                    name = "There is no type-specifier.";
                    break;
                case miniplc0::ErrNoExclamation:
                    name = "There is no exclamatiion.";
                    break;
                case miniplc0::ErrRightBracket:
                    name = "There is no rightBracket.";
                    break;
                case miniplc0::ErrLeftBracket:
                    name = "There is no leftBracket.";
                    break;
                case miniplc0::ErrNoIdentifier:
                    name = "There is no Identifier.";
                    break;
                case miniplc0::ErrNoPrint:
                    name = "There is no print.";
                    break;
                case miniplc0::ErrNoScan:
                    name = "There is no scan.";
                    break;
                case miniplc0::ErrNoReturn:
                    name = "There is no ruturn.";
                    break;
                case miniplc0::ErrNoWhile:
                    name = "Thiere is no while.";
                    break;
                case miniplc0::ErrNoIF:
                    name = "mu you if aaaaa.";
                    break;
                case miniplc0::ErrFunNoDeclared:
                    name = "where is function???";
                    break;
                case miniplc0::ErrLeftBigBracket:
                    name = "no left big bracket";
                    break;
                case miniplc0::ErrRightBigBracket:
                    name = "no right big bracket";
                    break;
                case miniplc0::ErrConditionType:
                    name = "ErrConditionType!!!";
                    break;
                case miniplc0::ErrWrongParaNum:
                    name = "paraNum doesn't equal function";
                    break;
                case miniplc0::ErrHasReturn:
                    name = "the function return type is VOID";
                    break;
                case miniplc0::ErrNoEntrance:
                    name = "where is the entrance for main";
                    break;
                case miniplc0::ErrZhuShi:
                    name ="no zhushi end";
                    break;
            }
            return format_to(ctx.out(), name);
        }
    };

    template<>
    struct formatter<miniplc0::CompilationError> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const miniplc0::CompilationError &p, FormatContext &ctx) {
            return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
        }
    };
}

namespace fmt {
    template<>
    struct formatter<miniplc0::Token> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const miniplc0::Token &p, FormatContext &ctx) {
            return format_to(ctx.out(),
                             "Line: {} Column: {} Type: {} Value: {}",
                             p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
        }
    };

    template<>
    struct formatter<miniplc0::TokenType> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const miniplc0::TokenType &p, FormatContext &ctx) {
            std::string name;
            switch (p) {
                case miniplc0::NULL_TOKEN:
                    name = "NullToken";
                    break;
                case miniplc0::UNSIGNED_INTEGER:
                    name = "UnsignedInteger";
                    break;
                case miniplc0::IDENTIFIER:
                    name = "Identifier";
                    break;
                case miniplc0::CONST:
                    name = "Const";
                    break;
                case miniplc0::PRINT:
                    name = "Print";
                    break;
                case miniplc0::SCAN:
                    name = "Scan";
                    break;
                case miniplc0::VOID:
                    name = "Void";
                    break;
                case miniplc0::INT:
                    name = "Int";
                    break;
                case miniplc0::CHAR:
                    name = "Char";
                    break;
                case miniplc0::DOUBLE:
                    name = "Double";
                    break;
                case miniplc0::STRUCT:
                    name = "Struct";
                    break;
                case miniplc0::IF:
                    name = "If";
                    break;
                case miniplc0::ELSE:
                    name = "Else";
                    break;
                case miniplc0::SWITCH:
                    name = "Switch";
                    break;
                case miniplc0::CASE:
                    name = "Case";
                    break;
                case miniplc0::DEFAULT:
                    name = "Default";
                    break;
                case miniplc0::WHILE:
                    name = "While";
                    break;
                case miniplc0::FOR:
                    name ="For";
                    break;
                case miniplc0::DO:
                    name = "Do";
                    break;
                case miniplc0::RETURN:
                    name = "Return";
                    break;
                case miniplc0::BREAK:
                    name = "Break";
                    break;
                case miniplc0::CONTINUE:
                    name = "Continue";
                    break;
                case miniplc0::PLUS_SIGN:
                    name = "PlusSign";
                    break;
                case miniplc0::MINUS_SIGN:
                    name = "MinusSign";
                    break;
                case miniplc0::MULTIPLICATION_SIGN:
                    name = "MultiplicationSign";
                    break;
                case miniplc0::DIVISION_SIGN:
                    name = "DivisionSign";
                    break;
                case miniplc0::EQUAL_SIGN:
                    name = "EqualSign";
                    break;
                case miniplc0::EQUAL_EQUAL_Sign:
                    name = "EqualEqualSign";
                    break;
                case miniplc0::SEMICOLON:
                    name = "Semicolon";
                    break;
                case miniplc0::LEFT_BRACKET:
                    name = "LeftBracket";
                    break;
                case miniplc0::RIGHT_BRACKET:
                    name = "RightBracket";
                    break;
                case miniplc0::NOT_EQUAL_SIGN:
                    name = "NotEqualSign";
                    break;
                case miniplc0::SMALL_SIGN:
                    name = "SmallSign";
                    break;
                case miniplc0::SMALL_EQUAL_SIGN:
                    name = "SmallEqualSign";
                    break;
                case miniplc0::MORE_SIGN:
                    name = "MoreSign";
                    break;
                case miniplc0::MORE_EQUAL_SIGN:
                    name = "MoreEqualSign";
                    break;
                case miniplc0::LEFT_BIG_BRACKET:
                    name = "LeftBigBracket";
                    break;
                case miniplc0::RIGHT_BIG_BRACKET:
                    name = "RightBigBracket";
                    break;
                case miniplc0::EXCLAMATION:
                    name = "Exclamation";
                    break;
            }
            return format_to(ctx.out(), name);
        }
    };
}

namespace fmt {
    template<>
    struct formatter<miniplc0::Operation> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const miniplc0::Operation &p, FormatContext &ctx) {
            std::string name;
            switch (p) {
                case miniplc0::ILL:
                    name = "ILL";
                    break;
                case miniplc0::ADD:
                    name = "ADD";
                    break;
                case miniplc0::SUB:
                    name = "SUB";
                    break;
                case miniplc0::MUL:
                    name = "MUL";
                    break;
                case miniplc0::DIV:
                    name = "DIV";
                    break;
                case miniplc0::WRT:
                    name = "WRT";
                    break;
                case miniplc0::LIT:
                    name = "LIT";
                    break;
                case miniplc0::LOD:
                    name = "LOD";
                    break;
                case miniplc0::STO:
                    name = "STO";
                    break;
            }
            return format_to(ctx.out(), name);
        }
    };
    template<>
    struct formatter<miniplc0::Instruction> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const miniplc0::Instruction &p, FormatContext &ctx) {
            std::string name;
            switch (p.GetOperation())
            {
                case miniplc0::ILL:
                case miniplc0::ADD:
                case miniplc0::SUB:
                case miniplc0::MUL:
                case miniplc0::DIV:
                case miniplc0::WRT:
                    return format_to(ctx.out(), "{}", p.GetOperation());
                case miniplc0::LIT:
                case miniplc0::LOD:
                case miniplc0::STO:
                    return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
            }
            return format_to(ctx.out(), "ILL");
        }
    };
}