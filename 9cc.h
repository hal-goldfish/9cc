

typedef struct { // ベクタ
    int len;
    int cap;
    void **data; // void* の配列
} Vector;

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQU,     // ==
    ND_NOTEQU,  // !=
    ND_GRE,     // >
    ND_LES,     // <
    ND_GREEQU,  // >=
    ND_LESEQU,  // <=
    ND_ASSIGN,  // = 代入演算子
    ND_LVAR,    // ローカル変数
    ND_FUNC,    // 関数
    ND_RETURN,  // return 文
    ND_IF,      // if 文
    ND_BLOCK,   // ブロック {}
    ND_FOR,     // for
    ND_WHILE,   // while
    ND_NUM,     // 整数
} NodeKind;


typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺
    Node *rhs;      // 右辺
    int val;        // kindがND_NUMの場合のみ使う
    int offset;     // kindがND_LVARの場合のみ使う

    char *name;

    // "if" (cond) then "else" els
    // "for" (init; cond; inc) body
    // "while" (cond) body
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;
    Node *body;

    int label;

    // block
    Vector *stmts;

    // func
    Vector *args;
};

Vector *new_vec();
void vec_push(Vector *, void *);
void *vec_pop(Vector *);
int vec_size(Vector *);



// トークンの種類
typedef enum {
    TK_RESERVED,    // 記号
    TK_IDENT,       // 識別子
    TK_NUM,         // 整数
    TK_RETURN,      // return 文
    TK_IF,          // if 文
    TK_ELSE,        // else
    TK_FOR,         // for
    TK_WHILE,       // while
    TK_EOF,         // EOF
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

extern Token *token;
extern char *user_input;

extern LVar *locals;

Token *tokenize(char *);
Node *program();
void gen(Node *);
void error(char *, ...);

extern Vector *code;

void error_at(char *, char *, ...);
void error(char *, ...);

void debug(char *, ...);

void foo();
void bar(int, int, int);