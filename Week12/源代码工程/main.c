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
// 存放前一个token的字符串
char token_string_prev[MAX_TOKEN_LENGTH + 1] = "";

// 语法分析的变量

// 记录当前的 Token
TokenType token;
// 记录前一个 Token
TokenType token_prev;
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
    strcat(source_file_name, ".c-");
  }

  source_file = fopen(source_file_name, "r");
  if (source_file == NULL) {
    printf("File %s not found\n", source_file_name);
    exit(1);
  }

  strcpy(result_file_name, source_file_name);
  *strstr(result_file_name, ".c-") = '\0';
  strcat(result_file_name, ".txt");
  result_file = fopen(result_file_name, "w");
  if (result_file == NULL) {
    printf("File %s cannot be written\n", source_file_name);
    exit(1);
  }

  fprintf(result_file, "CMINUS PARSING:\n");

  syntaxTree = parse();
  fprintf(result_file, "\nSyntax tree:\n");
  printTree(syntaxTree);

  printf("The parse analysis result is saved to file %s\n", result_file_name);

  fclose(source_file);
  return 0;
}

TokenType getToken(void) {
  // 保存前一个Token
  memset(token_string_prev, 0, sizeof(token_string_prev));
  strcpy(token_string_prev, token_string);
  // 注意这里必须初始化为空串，否则内存中的垃圾可能会随机填充到这里，影响后面的运行
  memset(token_string, 0, sizeof(token_string));
  // 标记在 token_string 中存到哪个下标了
  int token_string_index = 0;
  // 作为返回值的token
  TokenType current_token;
  // 当前处于DFA中的哪个状态，一共有 START, INCOMMENT, INNUM, INID, INEQ, INLE,
  // INGE, INNEQ, LBUFFER, RBUFFER, DONE 十一种
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
        if (isdigit(c))
          state = INNUM;
        else if (isalpha(c))
          state = INID;
        else if ((c == ' ') || (c == '\t') || (c == '\n'))
          can_be_saved = FALSE;
        else if (c == '=')
          state = INEQ;
        else if (c == '<')
          state = INLE;
        else if (c == '>')
          state = INGE;
        else if (c == '!')
          state = INNEQ;
        else if (c == '/')
          state = LBUFFER;
        else {
          state = DONE;
          switch (c) {
            case EOF:
              can_be_saved = FALSE;
              current_token = ENDFILE;
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
            case '(':
              current_token = LPAREN;
              break;
            case ')':
              current_token = RPAREN;
              break;
            case '[':
              current_token = LBRACKET;
              break;
            case ']':
              current_token = RBRACKET;
              break;
            case '{':
              current_token = LBRACE;
              break;
            case '}':
              current_token = RBRACE;
              break;
            case ';':
              current_token = SEMI;
              break;
            case ',':
              current_token = COMMA;
              break;
            default:
              current_token = ERROR;
              break;
          }
        }
        break;

      case LBUFFER:
        if (c == '*') {
          token_string_index = 0;
          can_be_saved = FALSE;
          state = INCOMMENT;
        } else if (c == EOF) {
          state = DONE;
          current_token = ENDFILE;
        } else {
          ungetNextChar();
          can_be_saved = FALSE;
          current_token = OVER;
          state = DONE;
        }
        break;

      case INCOMMENT:
        can_be_saved = FALSE;
        if (c == '*')
          state = RBUFFER;
        else if (c == EOF) {
          state = DONE;
          current_token = ENDFILE;
        }
        break;

      case RBUFFER:
        can_be_saved = FALSE;
        if (c == '/')
          state = START;
        else if (c == '*')
          ;
        else if (c == EOF) {
          state = DONE;
          current_token = ENDFILE;
        } else
          state = INCOMMENT;
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
      case INEQ:
        if (c == '=') {
          state = DONE;
          current_token = EQ;
        } else {
          ungetNextChar();
          can_be_saved = FALSE;
          state = DONE;
          current_token = ASSIGN;
        }
        break;

      case INLE:
        if (c == '=') {
          state = DONE;
          current_token = LE;
        } else {
          ungetNextChar();
          can_be_saved = FALSE;
          state = DONE;
          current_token = LT;
        }
        break;

      case INGE:
        if (c == '=') {
          state = DONE;
          current_token = GE;
        } else {
          ungetNextChar();
          can_be_saved = FALSE;
          state = DONE;
          current_token = GT;
        }
        break;

      case INNEQ:
        if (c == '=') {
          state = DONE;
          current_token = NEQ;
        } else {
          ungetNextChar();
          can_be_saved = FALSE;
          state = DONE;
          current_token = ERROR;
        }
        break;

      case DONE:
        break;

      default:
        // 如果已经进行到了 DONE 状态，那应该已经跳出了，不可能进入到 switch 中
        // 所有的状态在上面都涉及到了，不可能出现 default 响应的情况
        // 如果出现了上述两种情况，那一定是出现了不可思议的问题，直接跳出就好了
        fprintf(result_file, "Scanner Bug:state=%d\n", state);
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
    // fprintf(result_file, "%4d: ", line_number);
  } else {
    // fprintf(result_file, "\t%d: ", line_number);
  }

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
    case ELSE:
    case INT:
    case RETURN:
    case VOID:
    case WHILE:
      fprintf(result_file, "reserved word: %s\n", string);
      break;
    case ASSIGN:
      fprintf(result_file, "=\n");
      break;
    case EQ:
      fprintf(result_file, "==\n");
      break;
    case LT:
      fprintf(result_file, "<\n");
      break;
    case LE:
      fprintf(result_file, "<=\n");
      break;
    case GT:
      fprintf(result_file, ">\n");
      break;
    case GE:
      fprintf(result_file, ">=\n");
      break;
    case NEQ:
      fprintf(result_file, "!=\n");
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
    case LPAREN:
      fprintf(result_file, "(\n");
      break;
    case RPAREN:
      fprintf(result_file, ")\n");
      break;
    case LBRACKET:
      fprintf(result_file, "[\n");
      break;
    case RBRACKET:
      fprintf(result_file, "]\n");
      break;
    case LBRACE:
      fprintf(result_file, "{\n");
      break;
    case RBRACE:
      fprintf(result_file, "}\n");
      break;
    case COMMA:
      fprintf(result_file, ",\n");
      break;
    case SEMI:
      fprintf(result_file, ";\n");
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
    case ELSE:
      strcpy(string, "ELSE");
      break;
    case INT:
      strcpy(string, "INT");
      break;
    case RETURN:
      strcpy(string, "RETURN");
      break;
    case VOID:
      strcpy(string, "VOID");
      break;
    case WHILE:
      strcpy(string, "WHILE");
      break;
    case ASSIGN:
      strcpy(string, "ASSIGN");
      break;
    case EQ:
      strcpy(string, "EQ");
      break;
    case LT:
      strcpy(string, "LT");
      break;
    case LE:
      strcpy(string, "LE");
      break;
    case GT:
      strcpy(string, "GT");
      break;
    case GE:
      strcpy(string, "GE");
      break;
    case NEQ:
      strcpy(string, "NEQ");
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
    case LPAREN:
      strcpy(string, "LPAREN");
      break;
    case RPAREN:
      strcpy(string, "RPAREN");
      break;
    case LBRACKET:
      strcpy(string, "LBRACKET");
      break;
    case RBRACKET:
      strcpy(string, "RBRACKET");
      break;
    case LBRACE:
      strcpy(string, "LBRACE");
      break;
    case RBRACE:
      strcpy(string, "RBRACE");
      break;
    case COMMA:
      strcpy(string, "COMMA");
      break;
    case SEMI:
      strcpy(string, "SEMI");
      break;
    case ENDFILE:
      strcpy(string, "ENDFILE");
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

// 创建新节点
TreeNode* newNode(NodeKind kind) {
  TreeNode* p = (TreeNode*)malloc(sizeof(TreeNode));
  int k;
  if (p == NULL) {
    fprintf(result_file, "Out of memory error at line %d\n", line_number);
  } else {
    for (k = 0; k < MAX_CHILDREN; k++) {
      p->child[k] = NULL;
    }
    p->sibling = NULL;
    p->node_kind = kind;
    p->line_number = line_number;
    if (kind == OpK || kind == IntK || kind == IdK) {
      p->type = Integer;
    }
    if (kind == IdK) {
      p->attribute.name = "";
    }
    if (kind == ConstK) {
      p->attribute.value = 0;
    }
  }
  return p;
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
  if (token == expected) {
    token_prev = token;
    token = getToken();
  } else {
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

TreeNode* declaration_list() {
  TreeNode* t = declaration();
  TreeNode* p = t;
  //在开始语法分析出错的情况下找到int和void型，过滤掉int和void之前的所有Token，防止在开始时出错后面一错百错
  while ((token != INT) && (token != VOID) && (token != ENDFILE)) {
    printSyntaxError("");
    getToken();
    if (token == ENDFILE) {
      break;
    }
  }
  //寻找语法分析的入口，即找到int和void
  while ((token == INT) || (token == VOID)) {
    TreeNode* q;
    q = declaration();
    if (q != NULL) {
      if (t == NULL) {
        t = p = q;
      } else {
        p->sibling = q;
        p = q;
      }
    }
  }
  match(ENDFILE);
  return t;
}

TreeNode* declaration(void) {
  TreeNode* t = NULL;
  TreeNode* p = NULL;
  TreeNode* q = NULL;
  TreeNode* s = NULL;
  if (token == INT) {
    p = newNode(IntK);
    match(INT);
  } else if (token == VOID) {
    p = newNode(VoidK);
    match(VOID);
  } else {
    printSyntaxError("Type Error");
  }

  if ((p != NULL) && (token == ID)) {
    q = newNode(IdK);
    q->attribute.name = copyString(token_string);
    match(ID);

    if (token == LPAREN) {
      //'('：函数情况
      t = newNode(FunK);
      t->child[0] = p;
      t->child[1] = q;
      match(LPAREN);
      t->child[2] = params();
      match(RPAREN);
      t->child[3] = compound_stmt();
    } else if (token == LBRACKET) {
      //'['：数组声明
      t = newNode(Var_DeclK);
      TreeNode* m = newNode(Arry_DeclK);

      match(LBRACKET);
      match(NUM);
      s = newNode(ConstK);
      s->attribute.value = atoi(copyString(token_string_prev));
      m->child[0] = q;
      m->child[1] = s;
      t->child[0] = p;
      t->child[1] = m;
      match(RBRACKET);
      match(SEMI);
    } else if (token == SEMI)  //';'结尾：普通变量声明
    {
      t = newNode(Var_DeclK);
      t->child[0] = p;
      t->child[1] = q;
      match(SEMI);
    } else {
      printSyntaxError("");
    }
  } else {
    printSyntaxError("");
  }
  return t;
}

TreeNode* params(void) {
  TreeNode* t = newNode(ParamsK);
  TreeNode* p = NULL;
  if (token == VOID) {
    //开头为void，参数列表可能是(void)和(void id,[……])两种情况
    p = newNode(VoidK);
    match(VOID);
    if (token == RPAREN) {
      //参数列表为(void)
      if (t != NULL) {
        t->child[0] = p;
      }
    } else {
      //参数列表为(void id,[……])  ->void类型的变量
      t->child[0] = param_list(p);
    }
  } else if (token == INT) {
    //参数列表为(int id,[……])
    t->child[0] = param_list(p);
  } else {
    printSyntaxError("");
  }
  return t;
}

TreeNode* param_list(TreeNode* k) {
  // k可能是已经被取出来的VoidK，但又不是(void)类型的参数列表，所以一直传到param中去，作为其一个子节点
  TreeNode* t = param(k);
  TreeNode* p = t;
  k = NULL;  //没有要传给param的VoidK，所以将k设为NULL
  while (token == COMMA) {
    TreeNode* q = NULL;
    match(COMMA);
    q = param(k);
    if (q != NULL) {
      if (t == NULL) {
        t = p = q;
      } else {
        p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}

TreeNode* param(TreeNode* k) {
  TreeNode* t = newNode(ParamK);
  TreeNode* p = NULL;  // ParamK的第一个子节点
  TreeNode* q = NULL;  // ParamK的第二个子节点

  if (k == NULL && token == INT) {
    p = newNode(IntK);
    match(INT);
  } else if (k != NULL) {
    p = k;
  }
  if (p != NULL) {
    t->child[0] = p;
    if (token == ID) {
      q = newNode(IdK);
      q->attribute.name = copyString(token_string);
      t->child[1] = q;
      match(ID);
    } else {
      printSyntaxError("");
    }

    if ((token == LBRACKET) && (t->child[1] != NULL)) {
      match(LBRACKET);
      t->child[2] = newNode(IdK);
      match(RBRACKET);
    } else {
      return t;
    }
  } else {
    printSyntaxError("");
  }
  return t;
}

TreeNode* compound_stmt(void) {
  TreeNode* t = newNode(CompK);
  match(LBRACE);
  t->child[0] = local_declaration();
  t->child[1] = statement_list();
  match(RBRACE);
  return t;
}

TreeNode* local_declaration(void) {
  TreeNode* t = NULL;
  TreeNode* q = NULL;
  TreeNode* p = NULL;
  while (token == INT || token == VOID) {
    p = newNode(Var_DeclK);
    if (token == INT) {
      TreeNode* q1 = newNode(IntK);
      p->child[0] = q1;
      match(INT);
    } else if (token == VOID) {
      TreeNode* q1 = newNode(VoidK);
      p->child[0] = q1;
      match(INT);
    }
    if ((p != NULL) && (token == ID)) {
      TreeNode* q2 = newNode(IdK);
      q2->attribute.name = copyString(token_string);
      p->child[1] = q2;
      match(ID);

      if (token == LBRACKET) {
        TreeNode* q3 = newNode(Var_DeclK);
        p->child[3] = q3;
        match(LBRACKET);
        match(RBRACKET);
        match(SEMI);
      } else if (token == SEMI) {
        match(SEMI);
      } else {
        match(SEMI);
      }
    } else {
      printSyntaxError("");
    }
    if (p != NULL) {
      if (t == NULL)
        t = q = p;
      else {
        q->sibling = p;
        q = p;
      }
    }
  }
  return t;
}

TreeNode* statement_list(void) {
  TreeNode* t = statement();
  TreeNode* p = t;
  while (IF == token || LBRACE == token || ID == token || WHILE == token ||
         RETURN == token || SEMI == token || LPAREN == token || NUM == token) {
    TreeNode* q;
    q = statement();
    if (q != NULL) {
      if (t == NULL) {
        t = p = q;
      } else {
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
      t = selection_stmt();
      break;
    case WHILE:
      t = iteration_stmt();
      break;
    case RETURN:
      t = return_stmt();
      break;
    case LBRACE:
      t = compound_stmt();
      break;
    case ID:
    case SEMI:
    case LPAREN:
    case NUM:
      t = expression_stmt();
      break;
    default:
      printSyntaxError("");
      token = getToken();
      break;
  }
  return t;
}

TreeNode* selection_stmt(void) {
  TreeNode* t = newNode(Selection_StmtK);
  match(IF);
  match(LPAREN);
  if (t != NULL) {
    t->child[0] = expression();
  }
  match(RPAREN);
  t->child[1] = statement();
  if (token == ELSE) {
    match(ELSE);
    if (t != NULL) {
      t->child[2] = statement();
    }
  }
  return t;
}

TreeNode* expression_stmt(void) {
  TreeNode* t = NULL;
  if (token == SEMI) {
    match(SEMI);
    return t;
  } else {
    t = expression();
    match(SEMI);
  }
  return t;
}

TreeNode* iteration_stmt(void) {
  TreeNode* t = newNode(Iteration_StmtK);
  match(WHILE);
  match(LPAREN);
  if (t != NULL) {
    t->child[0] = expression();
  }
  match(RPAREN);
  if (t != NULL) {
    t->child[1] = statement();
  }
  return t;
}

TreeNode* return_stmt(void) {
  TreeNode* t = newNode(Return_StmtK);
  match(RETURN);
  if (token == SEMI) {
    match(SEMI);
    return t;
  } else {
    if (t != NULL) {
      t->child[0] = expression();
    }
  }
  match(SEMI);
  return t;
}

TreeNode* expression(void) {
  TreeNode* t = var();
  if (t == NULL)  //不是以ID开头，只能是simple_expression情况
  {
    t = simple_expression(t);
  } else {
    //以ID开头，可能是赋值语句，或simple_expression中的var和call类型的情况
    TreeNode* p = NULL;
    if (token == ASSIGN) {
      //赋值语句
      p = newNode(AssignK);
      p->attribute.name = copyString(token_string_prev);
      match(ASSIGN);
      p->child[0] = t;
      p->child[1] = expression();
      return p;
    } else {
      // simple_expression中的var和call类型的情况
      t = simple_expression(t);
    }
  }
  return t;
}

TreeNode* simple_expression(TreeNode* k) {
  TreeNode* t = additive_expression(k);
  k = NULL;
  if (EQ == token || GT == token || GE == token || LT == token || LE == token ||
      NEQ == token) {
    TreeNode* q = newNode(OpK);
    q->attribute.operation = token;
    q->child[0] = t;
    t = q;
    match(token);
    t->child[1] = additive_expression(k);
    return t;
  }
  return t;
}

TreeNode* additive_expression(TreeNode* k) {
  TreeNode* t = term(k);
  k = NULL;
  while ((token == PLUS) || (token == MINUS)) {
    TreeNode* q = newNode(OpK);
    q->attribute.operation = token;
    q->child[0] = t;
    match(token);
    q->child[1] = term(k);
    t = q;
  }
  return t;
}

TreeNode* term(TreeNode* k) {
  TreeNode* t = factor(k);
  k = NULL;
  while ((token == TIMES) || (token == OVER)) {
    TreeNode* q = newNode(OpK);
    q->attribute.operation = token;
    q->child[0] = t;
    t = q;
    match(token);
    t->child[1] = factor(k);
  }
  return t;
}

TreeNode* factor(TreeNode* k) {
  TreeNode* t = NULL;
  if (k != NULL) {
    // k为上面传下来的已经解析出来的以ID开头的var,可能为call或var
    if (token == LPAREN && k->node_kind != Arry_ElemK) {
      // call
      t = call(k);
    } else {
      t = k;
    }
  } else {
    //没有从上面传下来的var
    switch (token) {
      case LPAREN:
        match(LPAREN);
        t = expression();
        match(RPAREN);
        break;
      case ID:
        k = var();
        if (LPAREN == token && k->node_kind != Arry_ElemK) {
          t = call(k);
        } else {
          t = k;
        }
        break;
      case NUM:
        t = newNode(ConstK);
        if ((t != NULL) && (token == NUM)) {
          t->attribute.value = atoi(copyString(token_string));
        }
        match(NUM);
        break;
      default:
        printSyntaxError("");
        token = getToken();
        break;
    }
  }
  return t;
}

TreeNode* var(void) {
  TreeNode* t = NULL;
  TreeNode* p = NULL;
  TreeNode* q = NULL;
  if (token == ID) {
    p = newNode(IdK);
    p->attribute.name = copyString(token_string);
    match(ID);
    if (token == LBRACKET) {
      match(LBRACKET);
      q = expression();
      match(RBRACKET);

      t = newNode(Arry_ElemK);
      t->child[0] = p;
      t->child[1] = q;
    } else {
      t = p;
    }
  }
  return t;
}

TreeNode* call(TreeNode* k) {
  TreeNode* t = newNode(CallK);
  if (k != NULL) t->child[0] = k;
  match(LPAREN);
  if (token == RPAREN) {
    match(RPAREN);
    return t;
  } else if (k != NULL) {
    t->child[1] = args();
    match(RPAREN);
  }
  return t;
}

TreeNode* args(void) {
  TreeNode* t = newNode(ArgsK);
  TreeNode* s = NULL;
  TreeNode* p = NULL;
  if (token != RPAREN) {
    s = expression();
    p = s;
    while (token == COMMA) {
      TreeNode* q;
      match(COMMA);
      q = expression();
      if (q != NULL) {
        if (s == NULL) {
          s = p = q;
        } else {
          p->sibling = q;
          p = q;
        }
      }
    }
  }
  if (s != NULL) {
    t->child[0] = s;
  }
  return t;
}

// 生成语法树
TreeNode* parse(void) {
  TreeNode* t;
  token = getToken();
  t = declaration_list();
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
    switch (tree->node_kind) {
      case VoidK:
        fprintf(result_file, "VoidK\n");
        break;
      case IntK:
        fprintf(result_file, "IntK\n");
        break;
      case Var_DeclK:
        fprintf(result_file, "Var_DeclK\n");
        break;
      case Arry_DeclK:
        fprintf(result_file, "Arry_DeclK\n");
        break;
      case FunK:
        fprintf(result_file, "FuncK\n");
        break;
      case ParamsK:
        fprintf(result_file, "ParamsK\n");
        break;
      case ParamK:
        fprintf(result_file, "ParamK\n");
        break;
      case CompK:
        fprintf(result_file, "CompK\n");
        break;
      case Selection_StmtK:
        fprintf(result_file, "If\n");
        break;
      case Iteration_StmtK:
        fprintf(result_file, "While\n");
        break;
      case Return_StmtK:
        fprintf(result_file, "Return\n");
        break;
      case Arry_ElemK:
        fprintf(result_file, "Arry_ElemK\n");
        break;
      case CallK:
        fprintf(result_file, "CallK\n");
        break;
      case ArgsK:
        fprintf(result_file, "ArgsK\n");
        break;
      case AssignK:
        fprintf(result_file, "Assign\n");
        break;
      case OpK:
        fprintf(result_file, "Op: ");
        printToken(tree->attribute.operation, "\0");
        break;
      case ConstK:
        fprintf(result_file, "ConstK: %d\n", tree->attribute.value);
        break;
      case IdK:
        fprintf(result_file, "IdK: %s\n", tree->attribute.name);
        break;
      default:
        fprintf(result_file, "Unknown ExpNode kind\n");
        break;
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
      printTree(tree->child[i]);
    }
    tree = tree->sibling;
  }
  indent_number -= 2;
}
