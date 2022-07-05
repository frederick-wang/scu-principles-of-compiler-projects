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

/* Variable indent_number is used by printTree to
 * store current number of spaces to indent
 */
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

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StatementKind kind);
/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpressionKind kind);
/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* s);
void printSyntaxError(char* message);
void match(TokenType expected);
/* function prototypes for recursive calls */
TreeNode* stmt_sequence(void);
TreeNode* statement(void);
TreeNode* if_stmt(void);
TreeNode* repeat_stmt(void);
TreeNode* assign_stmt(void);
TreeNode* read_stmt(void);
TreeNode* write_stmt(void);
TreeNode* expression(void);
TreeNode* simple_expression(void);
TreeNode* term(void);
TreeNode* factor(void);
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void);
/* printSpaces indents by printing spaces */
void printSpace(void);
/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree);

#endif
