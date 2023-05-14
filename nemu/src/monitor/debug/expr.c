#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_NUMBER, TK_HEX, TK_REG,
  TK_EQ, TK_NEQ, TK_AND, TK_OR,
  TK_NEGATIVE, TK_DEREF
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},
  {"0|[1-9][0-9]*", TK_NUMBER},
  {"\\$(eax|ebx|ecx|edx|esp|ebp|esi|edi|eip|ax|bx|cx|dx|sp|bp|si|di|al|bl|cl|dl|ah|bh|ch|dh)", TK_REG},
  
  {"==", TK_EQ},
  {"!=", TK_NEQ},

  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", '!'},
  
  {"\\+", '+'},
  {"\\-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  
  {"\\(", '('},
  {"\\)", ')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

bool check_parenthese(int p, int q) {
  if (p > q)
    return check_parenthese(q, p);
  if (p == q || tokens[p].type != '(' || tokens[q].type != ')')
    return false;
  
  int c = 0;
  for (int curr = p+1; curr < q; curr++) {
    if (tokens[curr].type == '(')
      c++;
    if (tokens[curr].type == ')') {
      if (c != 0) c--;
      else return false;
    }
  }
  return c == 0;
}

int findDominant(int p, int q) {
  int level = 0;
  int pos[5] = {-1, -1, -1, -1, -1};
  for (int curr=p; curr<=q; curr++) {
    if (tokens[curr].type == '(') level += 1;
    if (tokens[curr].type == ')') level -= 1;
    if (level == 0) {
      switch (tokens[curr].type) {
      case TK_NEGATIVE: case TK_DEREF: case '!':
        pos[4] = curr; break;
      case '*': case '/':
        pos[3] = curr; break;
      case '+': case '-':
        pos[2] = curr; break;
      case TK_EQ: case TK_NEQ:
        pos[1] = curr; break;
      case TK_AND: case TK_OR:
        pos[0] = curr; break;
      }
    }
  }
  for (int i=0; i<5; i++)
    if (pos[i] != -1)
      return pos[i];
  for (int i=0; i<5; i++)
    printf("%d ", pos[i]);
  printf("\nError in findDominant(): p=%d, q=%d\n", p, q);
  assert(0);
}

int eval(int p, int q) {
  if (p > q) {
    printf("Error: p>q in eval, p=%d, q=%d\n", p, q);
    assert(0);
  }
  if (p == q) {
    int num;
    switch (tokens[p].type) {
    case TK_NUMBER:
      sscanf(tokens[p].str, "%d", &num);
      return num;
    case TK_HEX:
      sscanf(tokens[p].str, "%x", &num);
      return num;
    case TK_REG:
      for (int i=0; i<8; i++) {
        if(strcmp(tokens[p].str, regsl[i]) == 0)
          return reg_l(i);
        if(strcmp(tokens[p].str, regsw[i]) == 0)
          return reg_w(i);
        if(strcmp(tokens[p].str, regsb[i]) == 0)
          return reg_b(i);
      }
      if (strcmp(tokens[p].str, "eip") == 0) {
        return cpu.eip;
      } else {
        printf("Error in TK_REG in eval()\n");
        assert(0);
      }
    }
  }
  if (check_parenthese(p, q)) {
    return eval(p+1, q-1);
  }

  int op = findDominant(p, q);
  vaddr_t addr;
  int rst;
  
  switch (tokens[op].type) {
  case TK_NEGATIVE:
    return -eval(p+1, q);
  case TK_DEREF:
    addr = eval(p+1, q);
    rst = vaddr_read(addr, 4);
    printf("addr=%u(0x%x)->value=%d(0x%08x)\n", addr, addr, rst, rst);
    return rst;
  case '!':
    rst = eval(p+1, q);
    return !rst;
  }

  int val1 = eval(p, op-1);
  int val2 = eval(op+1, q);
  switch (tokens[op].type) {
  case '+': return val1 + val2;
  case '-': return val1 - val2;
  case '*': return val1 * val2;
  case '/': return val1 / val2;
  case TK_EQ: return val1 == val2;
  case TK_NEQ: return val1 != val2;
  case TK_AND: return val1 && val2;
  case TK_OR: return val1 || val2;
  default: assert(0);
  }
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if (substr_len > 32)
          assert(0);
        if (rules[i].token_type == TK_NOTYPE)
          break;
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
        case TK_NUMBER:
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          *(tokens[nr_token].str + substr_len) = '\0';
          break;
        case TK_HEX:
          strncpy(tokens[nr_token].str, substr_start+2, substr_len-2);
          *(tokens[nr_token].str+substr_len-2) = '\0';
          break;
        case TK_REG:
          strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
          *(tokens[nr_token].str+substr_len-1) = '\0';
          break;
        }
        nr_token++;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  // for (int i=0; i<nr_token; i++)
  //   printf("this token is:%s!\n", tokens[i].str);
  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (tokens[0].type == '-')
    tokens[0].type = TK_NEGATIVE;
  if (tokens[0].type == '*')
    tokens[0].type = TK_DEREF;
  for (int i=1; i<nr_token; i++) {
    if (tokens[i].type == '-') {
      if (tokens[i-1].type != TK_NUMBER && tokens[i-1].type != ')') {
        tokens[i].type = TK_NEGATIVE;
      }
    }
    if (tokens[i].type == '*') {
      if (tokens[i-1].type != TK_NUMBER && tokens[i-1].type != ')') {
        tokens[i].type = TK_DEREF;
      }
    }
  }
  *success = true;
  return eval(0, nr_token-1);
}
