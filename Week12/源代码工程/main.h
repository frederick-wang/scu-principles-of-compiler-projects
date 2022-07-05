#ifndef __MAIN_H__
#define __MAIN_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

// 主程序的变量
extern char source_file_name[MAX_FILENAME_LENGTH + 1];
extern char result_file_name[MAX_FILENAME_LENGTH + 1];
extern FILE* source_file;
extern FILE* result_file;

// 词法扫描的变量

// 当前行号
extern int line_number;
// 缓存当前行的内容
extern char line_buffer[MAX_BUFFER_LENGTH];
// 当前行读取到的下标
extern int line_buffer_index;
// 当前行的实际字符数
extern int line_buffer_size;
// 确保当前行还没有结束，为了预防 ungetNextChar() 在遇到 EOF 时出错
extern int is_EOF;
// 存放当前token的字符串
extern char token_string[MAX_TOKEN_LENGTH + 1];

// 语法分析的变量

// holds current token
extern TokenType token;

// 在 printTree() 中缩进的数量
extern int indent_number;

int main(int argc, char* argv[]);
// 不断的读取 source 中的字符，凑成一个 Token 就返回
TokenType getToken(void);
// 取出下一个字符
int getNextChar(void);
// 将下一个字符退回缓存区
void ungetNextChar(void);
// 区分「保留字」和「普通的标识符」
TokenType distinguishReservedWordAndIdentifer(char* string);
// 按照特定格式打印当前 Token
void printToken(TokenType, char* string);
void getTokenString(TokenType token, char* string);
// 检测当前行是否以\n结尾
int currentLineHasLF(void);

// 语法分析部分

// 创建新节点
TreeNode * newNode(NodeKind k);

// 工具函数，拷贝字符串
char* copyString(char* s);
// 工具函数，打印错误
void printSyntaxError(char* message);
// 查找特殊Token的Match过程
void match(TokenType expected);

// 下面的函数全部对应EBNF

TreeNode* declaration_list(void);
TreeNode* declaration(void);
TreeNode* params(void);
TreeNode* param_list(TreeNode* k);
TreeNode* param(TreeNode* k);
TreeNode* compound_stmt(void);
TreeNode* local_declaration(void);
TreeNode* statement_list(void);
TreeNode* statement(void);
TreeNode* expression_stmt(void);
TreeNode* selection_stmt(void);
TreeNode* iteration_stmt(void);
TreeNode* return_stmt(void);
TreeNode* expression(void);
TreeNode* var(void);
TreeNode* simple_expression(TreeNode* k);
TreeNode* additive_expression(TreeNode* k);
TreeNode* term(TreeNode* k);
TreeNode* factor(TreeNode* k);
TreeNode* call(TreeNode* k);
TreeNode* args(void);

// 生成语法树
TreeNode* parse(void);

// 工具函数，打印空格缩进
void printSpace(void);

// 递归打印语法树
void printTree(TreeNode* tree);

#endif
