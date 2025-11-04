/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
enum {
  TK_NOTYPE = 256, TK_EQ = 100,
  TK_DECI = 102, TK_MUL = 103,
  TK_HEXA = 101, TK_DIV = 104,
  TK_MINUS = 105, TK_LBRA = 106,
  TK_RBRA = 107

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"0[xX][0-9a-fA-F]+", TK_HEXA}, //hexadecimal number
				  //[xX]matches any one of them in '[]'
				  //0-9,a-f,A-F matches any number of letter from a-f(lowercase or uppercase)
				  //'+'means, matches the [] on its left many times
  {"[0-9]+", TK_DECI},  // decimal numbers
  {"\\*", TK_MUL},      //multiply
  {"\\/", TK_DIV},      //divide
  {"\\-", TK_MINUS},    //minus
  {"\\(", TK_LBRA},     //left bracket
  {"\\)", TK_RBRA},     //right bracket
  
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret; //ret 貌似只是用作返回值，没有特别意义
           //很多代码里的函数现在都是直接传入指针，直接对数据操作
	   //不像程序设计课程中需要用到返回值，调用函数的时候，
	   //就已经完成对数据的操作了。
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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e,int *nums) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  /*  chan added */
  init_regex();
  /*  chan added */
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

        /*for(int j=0;j<pmatch[i].rm_eo;){
		Token
	}*/

        /*strncpy((tokens[i].str),e+position,pmatch.rm_eo);
        printf("%s",tokens[i].str);*/
	int type = rules[i].token_type;
        switch (rules[i].token_type) {
	  case TK_NOTYPE:
	     break;	  
          default: 
	     tokens[nr_token].type = type;
	     int len = substr_len > 32 - 1 ? 32 -1 : substr_len;

	     strncpy(tokens[nr_token].str, substr_start, len);
	     tokens[nr_token].str[len] = '\0';
	     printf("type:[%d]str:%s \n",tokens[nr_token].type,tokens[nr_token].str);
	     nr_token ++;
	     *nums = nr_token;
	     break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q){
	if(p>q) return false;
	if(tokens[p].type!=TK_LBRA || tokens[q].type!=TK_RBRA) return false;

	int depth = 0;
	for(int i= p;i <=q;i++){
		if(tokens[i].type ==TK_LBRA)depth++;
		else if(tokens[i].type==TK_RBRA)depth--;
		if(depth ==0 &&i<q)return false;
		if(depth <0)return false;
	}
	return depth==0;
}

int priority(char op){
	if(op=='+'||op=='-')return 1;
	if(op=='*'||op=='/')return 2;
	assert(0);

}
uint32_t find_prime_op(int p,int q){
	int pos =-1;
	int prec = 65535;
	int depth =0;
	for(int i=p;i<=q;i++){
		if(tokens[i].type == TK_LBRA)depth++;
		else if(tokens[i].type ==TK_RBRA){depth--;if(depth<0)return -1;}
		else if(tokens[i].type =='+' ||tokens[i].type ==TK_MINUS ||tokens[i].type ==TK_MUL ||tokens[i].type ==TK_DIV ){
			if(depth==0){
				int _prec =priority(tokens[i].str[0]);
				if(_prec <prec){
					prec=_prec;
					pos =i;
				}
			}
		}

	}
	return pos;
}
uint32_t eval(int p,int q){
	if(p>q){
	Log("illegal expression format!");
	return 0;
	}
	else if(p==q){
		if(tokens[p].type==TK_DECI||tokens[p].type==TK_HEXA) {
			
			if(tokens[p].type==TK_HEXA)
			return strtol(tokens[p].str,NULL,16);
			else
			return strtol(tokens[p].str,NULL,10);
		
		
		
		}
		else {Log("expected a number but got a operator!");return 0;}

	}

	else if(check_parentheses(p,q)){
	return eval(p+1,q-1);

	}
	else{
	int op = find_prime_op(p,q);
	if(op==-1){
		Log("No operator in one of the  expression!");
		return 0;
	}
	uint32_t val1=0,val2=0;
	val1=eval(p,op-1);
	val2=eval(op+1,q);
	switch(tokens[op].type){
		case '+': return val1+val2;
		case TK_MINUS: return val1-val2;
		case TK_MUL: return val1*val2;
	        case TK_DIV:return val1/val2;
		default : 
			Log("Unknown operator!");
	    		return 0;		
	
	}

	
	}
}


word_t expr(char *e, bool *success) {
  int nr_token = 0 ;
  if (!make_token(e,&nr_token)) {
    *success = false;
    return 0;
  }
  printf("%d\n",nr_token);
  printf("resul:%d \n",eval(0,nr_token-1));
  /* TODO: Insert codes to evaluate the expression. */
  

  return 0;
}
