
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

const char *reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};


// 左辺値を要求する場合の処理
// アドレスをスタックに push する
void gen_lval(Node *node) {
    if (node->kind == ND_DEREF) {
        gen_lval(node->lhs);
        printf("  pop rax\n");
        printf("  push [rax]\n");
        return;
    }
    if (node->kind != ND_LVAR && node->kind != ND_VARDEF) {
        debug("%d", node->kind);
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}



void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;

    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;

    case ND_VARDEF:
        if (node->init) {
            gen_lval(node);
            gen(node->init);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
        }
        else gen_lval(node);
        return;

    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;

    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;

    case ND_IF:
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je   .Lelse%d\n", node->label);
            gen(node->then);
            printf("  jmp .Lend%d\n", node->label);
            printf(".Lelse%d:\n", node->label);
            gen(node->els);
        }
        else {
            printf("  je   .Lend%d\n", node->label);
            gen(node->then);
        }
        printf(".Lend%d:\n", node->label);
        return;

    case ND_WHILE:
        printf(".Lbegin%d:\n", node->label);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je   .Lend%d\n", node->label);
        gen(node->body);
        printf("  jmp .Lbegin%d\n", node->label);
        printf(".Lend%d:\n", node->label);
        return;

    case ND_FOR:
        gen(node->init);
        printf(".Lbegin%d:\n", node->label);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je   .Lend%d\n", node->label);
        gen(node->body);
        gen(node->inc);
        printf("  jmp .Lbegin%d\n", node->label);
        printf(".Lend%d:\n", node->label);
        return;

    case ND_BLOCK:
        for (int i = 0; i < vec_size(node->stmts); i++) {
            gen(node->stmts->data[i]);
            if (i != vec_size(node->stmts)-1) { // 最後の文以外の値は要らないのでpop
                printf("  pop rax\n");
            }
        }
        return;

    case ND_FUNCCALL:
        for (int i = vec_size(node->args) - 1; i >= 0; i--) {
            gen(node->args->data[i]);
            if (i < 6) {
                printf("  pop %s\n", reg[i]);
            }
        }
        printf("  call %s\n", node->name);
        printf("  push rax\n");
        return;

    // アドレスをそのまま返す
    case ND_ADDR: 
        gen_lval(node->lhs);
        return;

    case ND_DEREF:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;

    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) { // 計算
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_EQU:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NOTEQU:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LES:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LESEQU:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;

    }        


    printf("  push rax\n");
}

void func_gen(Function *func) {

    // プロローグ
    printf("%s:\n", func->name);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", roundup(func->locals->offset, 16));

    // 引数の処理
    for (int i = 0; i < vec_size(func->args); i++) {
        Node *arg = vec_at(func->args, i);
        if (i < 6) {
            printf("  mov [rbp-%d], %s\n", arg->offset, reg[i]);
        }
        else {
            printf("  pop [rax-%d]\n", arg->offset);
        }
    }

    for (int i = 0; i < vec_size(func->code); i++) {
        gen(vec_at(func->code, i));
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}