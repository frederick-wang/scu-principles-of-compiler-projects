#ifndef __TYPES_H__
#define __TYPES_H__

// 定义布尔值
#define TRUE (1)
#define FALSE (0)

// 文件名最大长度120个字符
#define MAX_FILENAME_LENGTH 120

// 一个token的长度最大为40
#define MAX_TOKEN_LENGTH 40

// Tiny源代码一行最多255个字符
#define MAX_BUFFER_LENGTH 256

// Tiny语言一共有 8 个保留字
#define MAX_RESERVED_NUMBER 8

// 定义语法书的最大子节点数量
#define MAX_CHILDREN 3

// 定义Token的类型
typedef enum TokenType {
  // 标注特殊状态的Type，
  // 它们不代表实际的Token，只是代表遇到文件尾和错误时，getToken() 函数的返回值
  ENDFILE,
  ERROR,
  // Tiny语言的8个保留关键字
  IF,
  THEN,
  ELSE,
  END,
  REPEAT,
  UNTIL,
  READ,
  WRITE,
  // 标识符
  ID,
  // 数字
  NUM,
  // 运算符
  ASSIGN,
  EQ,
  LT,
  PLUS,
  MINUS,
  TIMES,
  OVER,
  LPAREN,
  RPAREN,
  SEMI
} TokenType;

// 定义词法扫描DFA的状态
typedef enum StateType {
  START,
  INASSIGN,
  INCOMMENT,
  INNUM,
  INID,
  DONE
} StateType;

typedef struct ReservedWord {
  char* string;
  TokenType token;
} ReservedWord;
// 8 个保留字对应的 Token 类型
ReservedWord reserved_words[MAX_RESERVED_NUMBER] = {
    {"if", IF},         {"then", THEN},   {"else", ELSE}, {"end", END},
    {"repeat", REPEAT}, {"until", UNTIL}, {"read", READ}, {"write", WRITE}};

// 节点类型
typedef enum NodeKind { StmtK, ExpK } NodeKind;

// 语句节点的小类型
typedef enum StatementKind {
  IfK,
  RepeatK,
  AssignK,
  ReadK,
  WriteK
} StatementKind;

// 表达式节点的小类型
typedef enum ExpressionKind { OpK, ConstK, IdK } ExpressionKind;

// 语法树节点
typedef struct TreeNode {
  struct TreeNode* child[MAX_CHILDREN];
  struct TreeNode* sibling;
  int line_number;
  NodeKind node_kind;
  union kind {
    StatementKind statement;
    ExpressionKind expression;
  } kind;
  union attribute {
    TokenType operation;
    int value;
    char* name;
  } attribute;
} TreeNode;

#endif
