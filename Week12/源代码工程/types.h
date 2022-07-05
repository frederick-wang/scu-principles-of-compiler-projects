#ifndef __TYPES_H__
#define __TYPES_H__

// 定义布尔值
#define TRUE (1)
#define FALSE (0)

// 文件名最大长度120个字符
#define MAX_FILENAME_LENGTH 120

// 一个token的长度最大为40
#define MAX_TOKEN_LENGTH 40

// C-Minus 源代码一行最多255个字符
#define MAX_BUFFER_LENGTH 256

// C-Minus 语言一共有 6 个保留字
#define MAX_RESERVED_NUMBER 6

// 定义语法树的最大子节点数量
#define MAX_CHILDREN 4

// 定义Token的类型
typedef enum TokenType {
  // 标注特殊状态的Type，
  // 它们不代表实际的Token，只是代表遇到文件尾和错误时，getToken() 函数的返回值
  ENDFILE,
  ERROR,
  // C-Minus 语言的 6 个保留关键字
  IF,
  ELSE,
  INT,
  RETURN,
  VOID,
  WHILE,
  // 标识符
  ID,
  // 数字
  NUM,
  // 运算符
  ASSIGN,
  EQ,
  LT,
  LE,
  GT,
  GE,
  NEQ,
  PLUS,
  MINUS,
  TIMES,
  OVER,
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET,
  LBRACE,
  RBRACE,
  COMMA,
  SEMI
} TokenType;

// 定义词法扫描DFA的状态
typedef enum StateType {
  START,
  INCOMMENT,
  INNUM,
  INID,
  INEQ,
  INLE,
  INGE,
  INNEQ,
  LBUFFER,
  RBUFFER,
  DONE
} StateType;

typedef struct ReservedWord {
  char* string;
  TokenType token;
} ReservedWord;
// 6 个保留字对应的 Token 类型
ReservedWord reserved_words[MAX_RESERVED_NUMBER] = {
    {"if", IF},         {"else", ELSE}, {"int", INT},
    {"return", RETURN}, {"void", VOID}, {"while", WHILE}};

// 节点返回值类型
typedef enum ExpType { Void, Integer } ExpType;

// 节点类型
typedef enum NodeKind {
  StmtK,
  ExpK,
  IntK,
  VoidK,
  Var_DeclK,
  Arry_DeclK,
  FunK,
  ParamsK,
  ParamK,
  CompK,
  Selection_StmtK,
  Iteration_StmtK,
  Return_StmtK,
  AssignK,
  Arry_ElemK,
  CallK,
  ArgsK,
  UnkownK,
  OpK,
  ConstK,
  IdK
} NodeKind;

// 语法树节点
typedef struct treeNode {
  struct treeNode* child[MAX_CHILDREN];
  struct treeNode* sibling;
  int line_number;
  NodeKind node_kind;
  union {
    TokenType operation;
    int value;
    const char* name;
  } attribute;
  ExpType type;
} TreeNode;

#endif
