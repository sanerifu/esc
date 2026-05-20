#include <inttypes.h>

#include "estd.c"

typedef struct Token Token;
struct Token {
    enum {
        TOKEN_EOF,
        TOKEN_UNKNOWN,
        TOKEN_ERROR,
        TOKEN_ID,
        TOKEN_TYPEID,
        TOKEN_INTEGER,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACK,
        TOKEN_RBRACK,
        TOKEN_LBRACE,
        TOKEN_RBRACE,
        TOKEN_CHARACTERS,
        TOKEN_STRING
    } type;
    union {
        EstdString error;
        EstdString id;
        uintmax_t integer;
        char unknown;
        EstdString characters;
        EstdString string;
    };
};

void print_token(Token tok) {
    switch (tok.type) {
        case TOKEN_EOF:
            printf("EOF");
            break;
        case TOKEN_UNKNOWN:
            printf("Unknown(%x '%c')", tok.unknown, tok.unknown);
            break;
        case TOKEN_ERROR:
            printf("Unknown(%" PRIestr ")", ESTD_STRING_ARG(tok.error));
            break;
        case TOKEN_ID:
            printf("ID(%" PRIestr ")", ESTD_STRING_ARG(tok.id));
            break;
        case TOKEN_TYPEID:
            printf("TYPE(%" PRIestr ")", ESTD_STRING_ARG(tok.id));
            break;
        case TOKEN_INTEGER:
            printf("NUM(%" PRIuMAX ")", tok.integer);
            break;
        case TOKEN_LPAREN:
            printf("LPAREN");
            break;
        case TOKEN_RPAREN:
            printf("RPAREN");
            break;
        case TOKEN_LBRACK:
            printf("LBRACK");
            break;
        case TOKEN_RBRACK:
            printf("RBRACK");
            break;
        case TOKEN_LBRACE:
            printf("LBRACE");
            break;
        case TOKEN_RBRACE:
            printf("RBRACE");
            break;
        case TOKEN_CHARACTERS:
            printf("CHARACTERS(%" PRIestr ")", ESTD_STRING_ARG(tok.characters));
            break;
        case TOKEN_STRING:
            printf("STRING(%" PRIestr ")", ESTD_STRING_ARG(tok.string));
            break;
    }
}

Token lex(EstdString* io_string) {
    EstdString string = estd_string_trim(*io_string);
    if (string.length == 0) {
        return (Token){.type = TOKEN_EOF};
    }
    Token ret = (Token){.type = TOKEN_UNKNOWN, .unknown = string.data[0]};
    switch (string.data[0]) {
        case '(':
            string = ESTD_SLICE(string, 1, string.length);
            ret = (Token){.type = TOKEN_LPAREN};
            break;
        case ')':
            string = ESTD_SLICE(string, 1, string.length);
            ret = (Token){.type = TOKEN_RPAREN};
            break;
        case '[':
            string = ESTD_SLICE(string, 1, string.length);
            ret = (Token){.type = TOKEN_LBRACK};
            break;
        case ']':
            string = ESTD_SLICE(string, 1, string.length);
            ret = (Token){.type = TOKEN_RBRACK};
            break;
        case '{':
            string = ESTD_SLICE(string, 1, string.length);
            ret = (Token){.type = TOKEN_LBRACE};
            break;
        case '}':
            string = ESTD_SLICE(string, 1, string.length);
            ret = (Token){.type = TOKEN_RBRACE};
            break;
        case '\'': {
            EstdString lit = ESTD_STRING(string.data, 0);
            bool is_escaped = false;
            do {
                is_escaped = !is_escaped && lit.data[lit.length] == '\\';
                lit.length += 1;
            } while (lit.length < string.length && (is_escaped || lit.data[lit.length] != '\''));
            string = ESTD_SLICE(
                string,
                lit.length + (lit.length != string.length ? 1 : 0),
                string.length
            );  // ternary handles missing closing quote
            ret = (Token){.type = TOKEN_CHARACTERS, .characters = ESTD_SLICE(lit, 1, lit.length)};
            break;
        }
        case '\"': {
            EstdString lit = ESTD_STRING(string.data, 0);
            bool is_escaped = false;
            do {
                is_escaped = !is_escaped && lit.data[lit.length] == '\\';
                lit.length += 1;
            } while (lit.length < string.length && (is_escaped || lit.data[lit.length] != '\"'));
            string = ESTD_SLICE(
                string,
                lit.length + (lit.length != string.length ? 1 : 0),
                string.length
            );  // ternary handles missing closing quote
            ret = (Token){.type = TOKEN_STRING, .string = ESTD_SLICE(lit, 1, lit.length)};
            break;
        }
        case '@': {
            string = ESTD_SLICE(string, 1, string.length);
            EstdString id = ESTD_STRING(string.data, 0);
            while (id.length < string.length && (isalnum(id.data[id.length]) || id.data[id.length] == '_')) {
                id.length += 1;
            }
            string = ESTD_SLICE(string, id.length, string.length);
            ret = (Token){.type = TOKEN_TYPEID, .id = id};
            break;
        }
        case '$': {
            string = ESTD_SLICE(string, 1, string.length);
            EstdString id = ESTD_STRING(string.data, 0);
            while (id.length < string.length && (isalnum(id.data[id.length]) || id.data[id.length] == '_')) {
                id.length += 1;
            }
            string = ESTD_SLICE(string, id.length, string.length);
            ret = (Token){.type = TOKEN_ID, .id = id};
            break;
        }
        default:
            if (isalpha(string.data[0])) {
                EstdString id = ESTD_STRING(string.data, 0);
                while (id.length < string.length && (isalnum(id.data[id.length]) || id.data[id.length] == '_')) {
                    id.length += 1;
                }
                string = ESTD_SLICE(string, id.length, string.length);
                ret = (Token){.type = TOKEN_ID, .id = id};
                bool is_constant = id.length > 1;
                for (size_t i = 1; i < id.length; i++) {
                    if (islower(id.data[i])) {
                        is_constant = false;
                        break;
                    }
                }
                bool is_type = isupper(id.data[0]) && !is_constant;
                if (is_type) {
                    ret.type = TOKEN_TYPEID;
                }
            } else if (isdigit(string.data[0])) {
                EstdString num = ESTD_STRING(string.data, 0);
                while (num.length < string.length && isalnum(num.data[num.length])) {
                    num.length += 1;
                }
                string = ESTD_SLICE(string, num.length, string.length);
                ret.type = TOKEN_INTEGER;
                estd_string_to_uint(&ret.integer, num, 10);
            } else {
                string = ESTD_SLICE(string, 1, string.length);
            }
            break;
    }
    *io_string = string;

    return ret;
}

void fcloseWrapper(void* ptr) {
    fclose(*(FILE**)ptr);
}

int main(int argc, char* argv[]) {
    EstdArena* ESTD_CLEAN(estd_arena_destroy) arena = NULL;
    EstdString input = {0};
    if (argc >= 2) {
        FILE* ESTD_CLEAN(fclose) fp = fopen(argv[1], "r");
        if (fp == NULL) {
            ESTD_THROW(ESTD_IO_ERROR, "Could not open file %s", argv[1]);
        }
        ESTD_BUBBLE(estd_read_file(&input, &arena, fp), "Could not read file %s", argv[1]);
    } else {
        ESTD_BUBBLE(estd_read_stream(&input, &arena, stdin), "Could not read input");
    }
    ESTD_DEBUG("Read input");

    Token tok;
    while ((tok = lex(&input)).type != TOKEN_EOF) {
        print_token(tok);
        printf("\n");
    }

    return ESTD_SUCCESS;
}
