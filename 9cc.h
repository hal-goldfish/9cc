

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQU, // ==
    ND_NOTEQU, // !=
    ND_GRE, // >
    ND_LES, // <
    ND_GREEQU, // >=
    ND_LESEQU, // <=
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
};

typedef struct Token Token;

extern Token *token;
extern char *user_input;

Token *tokenize(char *);
Node *expr();
void gen(Node *);
void error(char *, ...);

