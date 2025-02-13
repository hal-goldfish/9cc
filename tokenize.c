#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"



// 変数名になりうる文字かを判定
bool is_alnum(char c) {
    return ('a' <= c && c <= 'z') || 
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}


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

        if (isdigit(*p)) { // 数字始まりの変数名を弾くために最初に判定
            cur = new_token(TK_NUM, cur, p, 0); // 数値の len をとりあえず0に設定
            cur->val = strtol(p, &p, 10);
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) { // return 
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }
        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) { // if
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }
        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) { // else
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }
        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) { // for
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }
        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) { // while
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }
        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) { // int
            cur = new_token(TK_RESERVED, cur, p, 3);
            p += 3;
            continue;
        }

        if (is_alnum(*p)) { // 変数
            int i = 1;
            for (; is_alnum(p[i]); i++) ;
            cur = new_token(TK_IDENT, cur, p, i);
            p += i;
            continue;
        }

        if (memcmp("+=", p, 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (memcmp("-=", p, 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (memcmp("*=", p, 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (memcmp("/=", p, 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '{' || *p == '}' || *p == ',' || *p == '&') {
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

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}