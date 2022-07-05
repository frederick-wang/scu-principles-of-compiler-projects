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
// 存放当前token的字符串
char token_string[MAX_TOKEN_LENGTH + 1] = "";

// 语法分析的变量

// 记录当前的 Token
TokenType token;
// 在 printTree() 中缩进的数量
int indent_number = 0;

int main(int argc, char* argv[]) {
  TreeNode* syntaxTree;

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

  fprintf(result_file, "TINY PARSING:\n");

  syntaxTree = parse();
  fprintf(result_file, "\nSyntax tree:\n");
  printTree(syntaxTree);

  printf("The parse analysis result is saved to file %s\n", result_file_name);

  fclose(source_file);
  return 0;
}

TokenType getToken(void) {
  // 注意这里必须初始化为空串，否则内存中的垃圾可能会随机填充到这里，影响后面的运行
  memset(token_string, 0, sizeof(token_string));
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
  // if (current_token == ENDFILE && currentLineHasLF()) {
  //   fprintf(result_file, "%4d: ", line_number);
  // } else {
  //   fprintf(result_file, "\t%d: ", line_number);
  // }

  // printToken(current_token, token_string);

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
        // fprintf(result_file, "%4d: %s", line_number, line_buffer);
        line_buffer[--line_buffer_size] = '\0';
      } else {
        // fprintf(result_file, "%4d: %s", line_number, line_buffer);
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
      fprintf(result_file, "NUM, value= %s\n", string);
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

// 获取 Token 的类型标识文字
void getTokenString(TokenType token, char* string) {
  switch (token) {
    case IF:
      strcpy(string, "IF");
      break;
    case THEN:
      strcpy(string, "THEN");
      break;
    case ELSE:
      strcpy(string, "ELSE");
      break;
    case END:
      strcpy(string, "END");
      break;
    case REPEAT:
      strcpy(string, "REPEAT");
      break;
    case UNTIL:
      strcpy(string, "UNTIL");
      break;
    case READ:
      strcpy(string, "READ");
      break;
    case WRITE:
      strcpy(string, "WRITE");
      break;
    case ASSIGN:
      strcpy(string, "ASSIGN");
      break;
    case LT:
      strcpy(string, "LT");
      break;
    case EQ:
      strcpy(string, "EQ");
      break;
    case LPAREN:
      strcpy(string, "LPAREN");
      break;
    case RPAREN:
      strcpy(string, "RPAREN");
      break;
    case SEMI:
      strcpy(string, "SEMI");
      break;
    case PLUS:
      strcpy(string, "PLUS");
      break;
    case MINUS:
      strcpy(string, "MINUS");
      break;
    case TIMES:
      strcpy(string, "TIMES");
      break;
    case OVER:
      strcpy(string, "OVER");
      break;
    case ENDFILE:
      strcpy(string, "EOF");
      break;
    case NUM:
      strcpy(string, "NUM");
      break;
    case ID:
      strcpy(string, "ID");
      break;
    case ERROR:
      strcpy(string, "ERROR");
      break;
    default:
      // 永不发生
      strcpy(string, "Unknown token");
  }
}

int currentLineHasLF(void) { return line_buffer[line_buffer_size - 1] == '\n'; }

// 语法分析部分

// 创建新的语句节点
TreeNode* newStmtNode(StatementKind kind) {
  TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(result_file, "Out of memory error at line %d\n", line_number);
  else {
    for (i = 0; i < MAX_CHILDREN; i++) {
      t->child[i] = NULL;
    }
    t->sibling = NULL;
    t->node_kind = StmtK;
    t->kind.statement = kind;
    t->line_number = line_number;
  }
  return t;
}

// 创建新的表达式节点
TreeNode* newExpNode(ExpressionKind kind) {
  TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(result_file, "Out of memory error at line %d\n", line_number);
  else {
    for (i = 0; i < MAX_CHILDREN; i++) {
      t->child[i] = NULL;
    }
    t->sibling = NULL;
    t->node_kind = ExpK;
    t->kind.expression = kind;
    t->line_number = line_number;
  }
  return t;
}

// 工具函数，拷贝字符串
char* copyString(char* s) {
  int n;
  char* t;
  if (s == NULL) {
    return NULL;
  }
  n = strlen(s) + 1;
  t = malloc(n);
  if (t == NULL) {
    fprintf(result_file, "Out of memory error at line %d\n", line_number);
  } else {
    strcpy(t, s);
  }
  return t;
}

// 工具函数，打印错误
void printSyntaxError(char* message) {
  fprintf(result_file, "\n>>> ");
  fprintf(result_file, "Syntax error at line %d: %s", line_number, message);
}

// 查找特殊Token的Match过程
void match(TokenType expected) {
  if (token == expected)
    token = getToken();
  else {
    char msg[256] = "expected ";
    char token_tmp[MAX_TOKEN_LENGTH + 1];
    getTokenString(expected, token_tmp);
    strcat(msg, token_tmp);
    strcat(msg, " unexpected token -> ");
    printSyntaxError(msg);
    printToken(token, token_string);
    fprintf(result_file, "      ");
  }
}

// 下面的函数全部对应EBNF

TreeNode* stmt_sequence(void) {
  TreeNode* t = statement();
  TreeNode* p = t;
  // 将代码按照语句切割，分隔符是分号（SEMI）
  // ENDFILE和END表示文件已经完毕，不用再读取
  // ELSE和UNTIL这两个Token，一定在if_stmt和repeat_stmt中会被完整匹配，不可能独立出现
  // 因此如果ELSE或UNTIL独立出现了，也说明程序出错了
  while ((token != ENDFILE) && (token != END) && (token != ELSE) &&
         (token != UNTIL)) {
    TreeNode* q;
    // 分号分割
    match(SEMI);
    q = statement();
    if (q != NULL) {
      if (t == NULL) {
        // 如果前一个节点不是语句类节点，报错了，没关系，从下一个Token重新开始
        t = p = q;
      } else {
        // 同属连接，不断循环
        p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}

TreeNode* statement(void) {
  TreeNode* t = NULL;
  switch (token) {
    case IF:
      t = if_stmt();
      break;
    case REPEAT:
      t = repeat_stmt();
      break;
    case ID:
      t = assign_stmt();
      break;
    case READ:
      t = read_stmt();
      break;
    case WRITE:
      t = write_stmt();
      break;
    default:
      printSyntaxError("unexpected token -> ");
      printToken(token, token_string);
      token = getToken();
      break;
  }
  return t;
}

TreeNode* if_stmt(void) {
  TreeNode* t = newStmtNode(IfK);
  match(IF);
  if (t != NULL) {
    t->child[0] = expression();
  }
  match(THEN);
  if (t != NULL) {
    t->child[1] = stmt_sequence();
  }
  if (token == ELSE) {
    match(ELSE);
    if (t != NULL) {
      t->child[2] = stmt_sequence();
    }
  }
  match(END);
  return t;
}

TreeNode* repeat_stmt(void) {
  TreeNode* t = newStmtNode(RepeatK);
  match(REPEAT);
  if (t != NULL) {
    t->child[0] = stmt_sequence();
  }
  match(UNTIL);
  if (t != NULL) {
    t->child[1] = expression();
  }
  return t;
}

TreeNode* assign_stmt(void) {
  TreeNode* t = newStmtNode(AssignK);
  if ((t != NULL) && (token == ID)) {
    t->attribute.name = copyString(token_string);
  }
  match(ID);
  match(ASSIGN);
  if (t != NULL) {
    t->child[0] = expression();
  }
  return t;
}

TreeNode* read_stmt(void) {
  TreeNode* t = newStmtNode(ReadK);
  match(READ);
  if ((t != NULL) && (token == ID)) {
    t->attribute.name = copyString(token_string);
  }
  match(ID);
  return t;
}

TreeNode* write_stmt(void) {
  TreeNode* t = newStmtNode(WriteK);
  match(WRITE);
  if (t != NULL) {
    t->child[0] = expression();
  }
  return t;
}

TreeNode* expression(void) {
  TreeNode* t = simple_expression();
  if ((token == LT) || (token == EQ)) {
    TreeNode* p = newExpNode(OpK);
    if (p != NULL) {
      p->child[0] = t;
      p->attribute.operation = token;
      t = p;
    }
    match(token);
    if (t != NULL) {
      t->child[1] = simple_expression();
    }
  }
  return t;
}

TreeNode* simple_expression(void) {
  TreeNode* t = term();
  while ((token == PLUS) || (token == MINUS)) {
    TreeNode* p = newExpNode(OpK);
    if (p != NULL) {
      p->child[0] = t;
      p->attribute.operation = token;
      t = p;
      match(token);
      t->child[1] = term();
    }
  }
  return t;
}

TreeNode* term(void) {
  TreeNode* t = factor();
  while ((token == TIMES) || (token == OVER)) {
    TreeNode* p = newExpNode(OpK);
    if (p != NULL) {
      p->child[0] = t;
      p->attribute.operation = token;
      t = p;
      match(token);
      p->child[1] = factor();
    }
  }
  return t;
}

TreeNode* factor(void) {
  TreeNode* t = NULL;
  switch (token) {
    case NUM:
      t = newExpNode(ConstK);
      if ((t != NULL) && (token == NUM)) {
        t->attribute.value = atoi(token_string);
      }
      match(NUM);
      break;
    case ID:
      t = newExpNode(IdK);
      if ((t != NULL) && (token == ID)) {
        t->attribute.name = copyString(token_string);
      }
      match(ID);
      break;
    case LPAREN:
      match(LPAREN);
      t = expression();
      match(RPAREN);
      break;
    default:
      printSyntaxError("unexpected token -> ");
      printToken(token, token_string);
      token = getToken();
      break;
  }
  return t;
}

// 生成语法树
TreeNode* parse(void) {
  TreeNode* t;
  token = getToken();
  t = stmt_sequence();
  if (token != ENDFILE) {
    printSyntaxError("Code ends before file\n");
  }
  return t;
}

// 工具函数，打印空格缩进
void printSpaces(void) {
  for (int i = 0; i < indent_number; i++) {
    fprintf(result_file, " ");
  }
}

// 递归打印语法树
void printTree(TreeNode* tree) {
  indent_number += 2;
  while (tree != NULL) {
    printSpaces();
    if (tree->node_kind == StmtK) {
      switch (tree->kind.statement) {
        case IfK:
          fprintf(result_file, "If\n");
          break;
        case RepeatK:
          fprintf(result_file, "Repeat\n");
          break;
        case AssignK:
          fprintf(result_file, "Assign to: %s\n", tree->attribute.name);
          break;
        case ReadK:
          fprintf(result_file, "Read: %s\n", tree->attribute.name);
          break;
        case WriteK:
          fprintf(result_file, "Write\n");
          break;
        default:
          fprintf(result_file, "Unknown ExpNode kind\n");
          break;
      }
    } else if (tree->node_kind == ExpK) {
      switch (tree->kind.expression) {
        case OpK:
          fprintf(result_file, "Op: ");
          printToken(tree->attribute.operation, "\0");
          break;
        case ConstK:
          fprintf(result_file, "Const: %d\n", tree->attribute.value);
          break;
        case IdK:
          fprintf(result_file, "Id: %s\n", tree->attribute.name);
          break;
        default:
          fprintf(result_file, "Unknown ExpNode kind\n");
          break;
      }
    } else
      fprintf(result_file, "Unknown node kind\n");
    for (int i = 0; i < MAX_CHILDREN; i++) {
      printTree(tree->child[i]);
    }
    tree = tree->sibling;
  }
  indent_number -= 2;
}
