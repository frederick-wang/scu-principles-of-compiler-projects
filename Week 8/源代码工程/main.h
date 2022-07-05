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
// 检测当前行是否以\n结尾
int currentLineHasLF(void);

#endif
