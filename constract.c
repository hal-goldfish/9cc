#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

bool is_ident() {
    return token->kind == TK_IDENT;
}

void consume_ident() {
    if (token->kind != TK_IDENT) {
        error("変数ではありません");
        return;
    }
    token = token->next;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED || 
        token->len != strlen(op) ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}


Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 整数は葉なので子を持たない
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}


Node *expr();

Node *primary() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // 変数
    if (is_ident()) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (token->str[0] - 'a' + 1) * 8;
        consume_ident();
        return node;
    }

    // 数字
    return new_node_num(expect_number());
}

Node *unary() {
    if (consume("+")) 
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) // * のトークンをconsume()で消費するので次は ( か数字
            node = new_node(ND_MUL, node, unary()); // 先のnodeとunary()を子とするノードを作成
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else return node;
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if(consume("-"))
            node = new_node(ND_SUB, node, mul());
        else return node;
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(">="))
            node = new_node(ND_LESEQU, add(), node);
        else if (consume("<="))
            node = new_node(ND_LESEQU, node, add());
        else if (consume(">"))
            node = new_node(ND_LES, add(), node);
        else if (consume("<"))
            node = new_node(ND_LES, node, add());
        else return node;
    }
}
 
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQU, node, relational());
        }
        else if (consume("!=")) 
            node = new_node(ND_NOTEQU, node, relational());
        else return node;
    }
}

Node *assign() {
    Node *node = equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

Node *expr() {
    return assign();
}

Node *stmt() {
    Node *node = expr();
    expect(";");
    return node;
}

Node *program() {
    int i = 0;
    for (; !at_eof(); i++) {
        code[i] = stmt();
    }
    code[i] = NULL;
}