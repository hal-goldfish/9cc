#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int label_count;


// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    }
    return NULL;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {

    if ((token->kind == TK_RESERVED || token->kind == TK_RETURN || token->kind == TK_IF || token->kind == TK_ELSE || token->kind == TK_FOR || token->kind == TK_WHILE) &&
        strlen(op) == token->len &&
        memcmp(token->str, op, token->len) == 0) {
        token = token->next;
        return true;
    }
    return false;
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

        LVar *lvar = find_lvar(token);
        if (lvar) {
            node->offset = lvar->offset;
        }
        else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = token->str;
            lvar->len = token->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }

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
    if (consume("+=")) {
        Node *rhs = new_node(ND_ADD, node, assign());
        node = new_node(ND_ASSIGN, node, rhs);
    }
    if (consume("-=")) {
        Node *rhs = new_node(ND_SUB, node, assign());
        node = new_node(ND_ASSIGN, node, rhs);
    }
    if (consume("*=")) {
        Node *rhs = new_node(ND_MUL, node, assign());
        node = new_node(ND_ASSIGN, node, rhs);
    }
    if (consume("/=")) {
        Node *rhs = new_node(ND_DIV, node, assign());
        node = new_node(ND_ASSIGN, node, rhs);
    }

    return node;
}

Node *expr() {
    return assign();
}

Node *statement() {

    Node *node = calloc(1, sizeof(Node));

    if (consume("{")) {
        node->kind = ND_BLOCK;
        node->stmts = new_vec();

        while (!consume("}")) {
            Node *tmp = calloc(1, sizeof(Node));
            tmp = statement();
            vec_push(node->stmts, tmp);
        }

        return node;
    }
 
    if (consume("if")) {
        expect("(");
        node->kind = ND_IF;
        node->cond = expr();
        node->label = label_count++;
        expect(")");
        node->then = statement();
        if (consume("else")) {
            node->els = statement();
        }
        return node;
    }

    if (consume("while")) {
        expect("(");
        node->kind = ND_WHILE;
        node->label = label_count++;
        node->cond = expr();
        expect(")");
        node->body = statement();
        return node;
    }

    if (consume("for")) {
        expect("(");
        node->kind = ND_FOR;
        node->label = label_count++;
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->body = statement();
        return node;
    }

    if (consume("return")) {
        node->kind = ND_RETURN;
        node->lhs = expr();
    }
    else node = expr();

    expect(";");
    return node;
}

Node *program() {
    locals = calloc(1, sizeof(LVar));
    int i = 0;
    code = new_vec();
    for (; !at_eof(); i++) {
        vec_push(code, statement());

    }
}