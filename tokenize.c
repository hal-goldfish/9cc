#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Node *code[100];


// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        else if (*p == '>' || *p == '<') {
            if (*(p+1) == '=') {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
                continue;
            }
            else {
                cur = new_token(TK_RESERVED, cur, p++, 1);
                continue;
            }
        }
        else if (*p == '!') {
            if (*(p+1) == '=') {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
                continue;
            }
        }
        else if (*p == '=') {
            if (*(p+1) == '=') {
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
                continue;
            }
            else {
                cur = new_token(TK_RESERVED, cur, p++, 1);
                continue;
            }
        }
        
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0); // 数値の len をとりあえず0に設定
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}