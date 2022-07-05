#include "main.h"

// 主程序的变量
char source_file_name[MAX_FILENAME_LENGTH + 1];
char result_file_name[MAX_FILENAME_LENGTH + 1];
FILE* source_file;
FILE* result_file;

// 词法扫描的变量

// 当前行号
int line_number = 0;
// 缓存当前行的内容
char line_buffer[MAX_BUFFER_LENGTH];
// 当前行读取到的下标
int line_buffer_index = 0;
// 当前行的实际字符数
int line_buffer_size = 0;
// 确保当前行还没有结束，为了预防 ungetNextChar() 在遇到 EOF 时出错
int is_EOF = FALSE;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(1);
  }

  strcpy(source_file_name, argv[1]);
  if (strchr(source_file_name, '.') == NULL) {
    strcat(source_file_name, ".tny");
  }

  source_file = fopen(source_file_name, "r");
  if (source_file == NULL) {
    printf("File %s not found\n", source_file_name);
    exit(1);
  }

  strcpy(result_file_name, source_file_name);
  *strstr(result_file_name, ".tny") = '\0';
  strcat(result_file_name, ".txt");
  result_file = fopen(result_file_name, "w");
  if (result_file == NULL) {
    printf("File %s cannot be written\n", source_file_name);
    exit(1);
  }

  fprintf(result_file, "TINY COMPILATION:\n");

  while (getToken() != ENDFILE) {
  }

  printf("The lexical analysis result is saved to file %s\n", result_file_name);

  return 0;
}

TokenType getToken(void) {
  // 存放当前token的字符串
  // 注意这里必须初始化为空串，否则内存中的垃圾可能会随机填充到这里，影响后面的运行
  char token_string[MAX_TOKEN_LENGTH + 1] = "";
  // 标记在 token_string 中存到哪个下标了
  int token_string_index = 0;
  // 作为返回值的token
  TokenType current_token;
  // 当前处于DFA中的哪个状态，一共有 START, INASSIGN, INCOMMENT, INNUM,
  // INID, DONE 六种
  StateType state = START;

  // 跑 DFA，只要没到终态就一直跑
  while (state != DONE) {
    // 取出一个字符做判断
    int c = getNextChar();
    // 标明当前的字符是否可以被保存进 token_string中，
    // 一开始先假定这个字符是可以被保存进入 token_string 的
    int can_be_saved = TRUE;
    // 双层 case 法
    switch (state) {
      case START:
        if (isdigit(c)) {
          state = INNUM;
        } else if (isalpha(c)) {
          state = INID;
        } else if (c == ':') {
          state = INASSIGN;
        } else if (c == ' ' || c == '\t' || c == '\n') {
          can_be_saved = FALSE;
        } else if (c == '{') {
          can_be_saved = FALSE;
          state = INCOMMENT;
        } else {
          state = DONE;
          switch (c) {
            case EOF:
              can_be_saved = FALSE;
              current_token = ENDFILE;
              break;
            case '=':
              current_token = EQ;
              break;
            case '<':
              current_token = LT;
              break;
            case '+':
              current_token = PLUS;
              break;
            case '-':
              current_token = MINUS;
              break;
            case '*':
              current_token = TIMES;
              break;
            case '/':
              current_token = OVER;
              break;
            case '(':
              current_token = LPAREN;
              break;
            case ')':
              current_token = RPAREN;
              break;
            case ';':
              current_token = SEMI;
              break;
            default:
              current_token = ERROR;
              break;
          }
        }
        break;
      case INCOMMENT:
        can_be_saved = FALSE;
        if (c == EOF) {
          state = DONE;
          current_token = ENDFILE;
        } else if (c == '}') {
          state = START;
        }
        break;
      case INASSIGN:
        state = DONE;
        if (c == '=') {
          current_token = ASSIGN;
        } else {
          ungetNextChar();
          can_be_saved = FALSE;
          current_token = ERROR;
        }
        break;
      case INNUM:
        if (!isdigit(c)) {
          ungetNextChar();
          can_be_saved = FALSE;
          state = DONE;
          current_token = NUM;
        }
        break;
      case INID:
        if (!isalpha(c)) {
          ungetNextChar();
          can_be_saved = FALSE;
          state = DONE;
          current_token = ID;
        }
        break;
      case DONE:
      default:
        // 如果已经进行到了 DONE 状态，那应该已经跳出了，不可能进入到 switch 中
        // 所有的状态在上面都涉及到了，不可能出现 default 响应的情况
        // 如果出现了上述两种情况，那一定是出现了不可思议的问题，直接跳出就好了
        fprintf(result_file, "Scanner Bug: state= %d\n", state);
        state = DONE;
        current_token = ERROR;
        break;
    }

    if (can_be_saved && token_string_index <= MAX_TOKEN_LENGTH) {
      // 如果当前处理的字符是多字符token的一部分，而且token没有过长的话，就保存下来
      token_string[token_string_index++] = (char)c;
    }

    if (state == DONE) {
      token_string[token_string_index] == '\0';
      if (current_token == ID) {
        current_token = distinguishReservedWordAndIdentifer(token_string);
      }
    }
  }

  // 当 EOF 独立成行时，这里需要用行号的方式输出
  if (current_token == ENDFILE && currentLineHasLF()) {
    fprintf(result_file, "%4d: ", line_number);
  } else {
    fprintf(result_file, "\t%d: ", line_number);
  }

  printToken(current_token, token_string);

  return current_token;
}

int getNextChar(void) {
  if (line_buffer_index < line_buffer_size) {
    // 如果当前读取的内容在缓存区边界内，就直接返回下标对应的字符，并将下标加一
    return line_buffer[line_buffer_index++];
  } else {
    // 如果当前读取的内容不在当前行缓存区边界内，说明这一行已经读取完了

    // C 库函数 char *fgets(char *str, int n, FILE *stream) 从指定的流 stream
    // 读取一行，并把它存储在 str 所指向的字符串内。当读取 (n-1)
    // 个字符时，或者读取到换行符时，或者到达文件末尾时，它会停止，具体视情况而定。
    // 这里读取的是 (MAX_BUFFER_LENGTH - 1) 个字符，是为了给\0腾出一个字节
    if (fgets(line_buffer, MAX_BUFFER_LENGTH - 1, source_file)) {
      // 如果 source_file 中的内容还有，
      // 打印这一行的内容，附带行号
      // 行号加一
      line_number++;
      // 记录这一行的实际字符数
      line_buffer_size = strlen(line_buffer);
      // 如果 EOF 没有独立成行的话，最后一行字符是没有 \n 的，需要补一个 \n
      // 让其输出换行
      if (!currentLineHasLF()) {
        line_buffer[line_buffer_size++] = '\n';
        line_buffer[line_buffer_size] = '\0';
        fprintf(result_file, "%4d: %s", line_number, line_buffer);
        line_buffer[--line_buffer_size] = '\0';
      } else {
        fprintf(result_file, "%4d: %s", line_number, line_buffer);
      }
      // 将当前行读取到的下标重置为 0
      line_buffer_index = 0;
      // 返回当前行的第一个字符，并将下标加一
      return line_buffer[line_buffer_index++];
    } else {
      // 如果 source_file 中的内容已经全部读取完了，就返回 EOF
      is_EOF = TRUE;
      if (currentLineHasLF()) {
        line_number++;
      }
      return EOF;
    }
  }
}

void ungetNextChar(void) {
  // 如果下一个字符就是文件尾，那就没有回退的必要了。
  if (!is_EOF) {
    line_buffer_index--;
  }
}

TokenType distinguishReservedWordAndIdentifer(char* string) {
  for (int i = 0; i < MAX_RESERVED_NUMBER; i++) {
    if (!strcmp(string, reserved_words[i].string)) {
      return reserved_words[i].token;
    }
  }
  return ID;
}

void printToken(TokenType token, char* string) {
  switch (token) {
    case IF:
    case THEN:
    case ELSE:
    case END:
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
      fprintf(result_file, "reserved word: %s\n", string);
      break;
    case ASSIGN:
      fprintf(result_file, ":=\n");
      break;
    case LT:
      fprintf(result_file, "<\n");
      break;
    case EQ:
      fprintf(result_file, "=\n");
      break;
    case LPAREN:
      fprintf(result_file, "(\n");
      break;
    case RPAREN:
      fprintf(result_file, ")\n");
      break;
    case SEMI:
      fprintf(result_file, ";\n");
      break;
    case PLUS:
      fprintf(result_file, "+\n");
      break;
    case MINUS:
      fprintf(result_file, "-\n");
      break;
    case TIMES:
      fprintf(result_file, "*\n");
      break;
    case OVER:
      fprintf(result_file, "/\n");
      break;
    case ENDFILE:
      fprintf(result_file, "EOF\n");
      break;
    case NUM:
      fprintf(result_file, "NUM, val= %s\n", string);
      break;
    case ID:
      fprintf(result_file, "ID, name= %s\n", string);
      break;
    case ERROR:
      fprintf(result_file, "ERROR: %s\n", string);
      break;
    default:
      // 永不发生
      fprintf(result_file, "Unknown token: %d\n", token);
  }
}

int currentLineHasLF(void) { return line_buffer[line_buffer_size - 1] == '\n'; }
