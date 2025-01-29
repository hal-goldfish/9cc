

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
    ND_RETURN,  // return 文
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
};

// トークンの種類
typedef enum {
    TK_RESERVED,    // 記号
    TK_IDENT,       // 識別子
    TK_NUM,         // 整数
    TK_RETURN,      // return 文
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

extern Node *code[];

void error_at(char *, char *, ...);
void error(char *, ...);