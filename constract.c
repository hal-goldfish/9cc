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
    Function *fc = calloc(1, sizeof(Function));
    fc = vec_last(funcs);
    for (LVar *var = fc->locals; var; var = var->next) {
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

bool consume_int() {
    if (token->kind == TK_RESERVED &&
        memcmp(token->str, "int", token->len) == 0) {
        token = token->next;
        return true;
    }
    return false;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        error_at(token->str, "変数ではありません");
        return NULL;
    }
    Token *res = malloc(sizeof(Token));
    res = token;
    token = token->next;
    return res;
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

Node *assign();

Node *declaration() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VARDEF;

    LVar *lvar = calloc(1, sizeof(LVar));
    Function *fc = calloc(1, sizeof(Function));
    Token *tmp = calloc(1, sizeof(Token));
    tmp = consume_ident();
    fc = vec_last(funcs);
    lvar->next = fc->locals;
    lvar->name = tmp->str;
    lvar->len = tmp->len;
    lvar->offset = fc->locals->offset + 8;
    node->offset = lvar->offset;
    fc->locals = lvar;

    if (consume("=")) {
        node->init = assign();
    }

    expect(";");
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

    // 変数 or 関数
    if (is_ident()) {
        Token *tmp = consume_ident();
        Node *node = calloc(1, sizeof(Node));

        if (consume("(")) { // 関数
            node->kind = ND_FUNCCALL;
            node->name = malloc(sizeof(char) * tmp->len);
            node->args = new_vec();
            while (!consume(")")) {
                Node *arg = malloc(sizeof(Node));
                arg = expr();
                vec_push(node->args, arg);
                if (consume(",")) continue;
            }
            strncpy(node->name, tmp->str, tmp->len);
        }
        else { // 変数
            node->kind = ND_LVAR;

            LVar *lvar = find_lvar(tmp);
            if (lvar) {
                node->offset = lvar->offset;
            }
            else {
                error_at(token->str, "未定義の変数です");
                // lvar = calloc(1, sizeof(LVar));
                // Function *fc = calloc(1, sizeof(Function));
                // fc = vec_last(funcs);
                // lvar->next = fc->locals;
                // lvar->name = tmp->str;
                // lvar->len = tmp->len;
                // lvar->offset = fc->locals->offset + 8;
                // node->offset = lvar->offset;
                // fc->locals = lvar;
            }
        }
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
    if (consume("*")) 
        return new_node(ND_DEREF, unary(), NULL);
    if (consume("&")) 
        return new_node(ND_ADDR, unary(), NULL);
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

    // 変数定義
    if (consume_int()) {
        return declaration();
    }

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
            if (consume_int()) {
                node->init = declaration();
            }
            else {
                node->init = expr();
                expect(";");
            }   
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

Function *function() {
    if (!consume_int()) error_at(token->str, "返り値の型の定義がありません");
    Token *tok = consume_ident();
    if (!tok) error_at(tok->str, "関数名ではありません");

    Function *func = calloc(1, sizeof(Function));
    vec_push(funcs, func);
    func->locals = calloc(1, sizeof(LVar));
    func->name = malloc(sizeof(char) * tok->len);
    strncpy(func->name, tok->str, tok->len);
    func->len = tok->len;
    func->args = new_vec();
    func->code = new_vec();

    expect("(");
    for (; !consume(")"); ) {
        consume_int();
        Token *tok = calloc(1, sizeof(Token));
        tok = consume_ident();
        Node *arg = calloc(1, sizeof(Node));
        arg->name = tok->str;
        arg->kind = ND_LVAR;
        
        func->lvars = calloc(1, sizeof(LVar));
        func->lvars->next = func->locals;
        func->lvars->name = tok->str;
        func->lvars->len = tok->len;
        func->lvars->offset = func->locals->offset + 8;
        arg->offset = func->lvars->offset;
        func->locals = func->lvars;

        vec_push(func->args, arg);

        if (consume(",")) continue;
        else {
            expect(")");
            break;
        }
    }

    expect("{");
    while (!consume("}")) {
        vec_push(func->code, statement());
    }

    return func;
}

void program() {
    funcs = new_vec();
    while (!at_eof()) {
        function();
    }
}