
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


// 左辺値を要求する場合の処理
// 左辺値のアドレスをスタックトップに持ってくる
void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
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

    case ND_INC:
        gen_lval(node->lhs);
        printf("  pop rax\n");
        printf("  mov esi, dword ptr [rax]\n");
        printf("  inc esi\n");
        printf("  mov dword ptr [rax], esi\n");
        printf("  push [rax]\n");
        return;

    case ND_DEC:
        gen_lval(node->lhs);
        printf("  pop rax\n");
        printf("  mov esi, dword ptr [rax]\n");
        printf("  dec esi\n");
        printf("  mov dword ptr [rax], esi\n");
        printf("  push [rax]\n");
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
        printf("  je  .Lend%d\n", node->label);
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
        printf("  je  .Lend%d\n", node->label);
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