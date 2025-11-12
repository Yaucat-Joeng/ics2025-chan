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

word_t paddr_read(paddr_t addr, int len);
word_t isa_reg_str2val(const char *s, bool *success);
enum {
  TK_NOTYPE = 256, TK_EQ = 100,
  TK_DECI = 102, TK_MUL = 103,
  TK_HEXA = 101, TK_DIV = 104,
  TK_MINUS = 105, TK_LBRA = 106,
  TK_RBRA = 107, TK_NEG =108,
  TK_DEREF=109, TK_REG=110,
  TK_AND=111

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
  {"\\$\\$?[a-zA-Z0-9]{1,2}",TK_REG},      //dollar 

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"&&",TK_AND},	// and
};

#define NR_REGEX ARRLEN(rules)
static bool reg_label=false;
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

       	int type = rules[i].token_type;
        switch (rules[i].token_type) {
	  case TK_NOTYPE:
	     break;
	  case TK_REG:
     	     tokens[nr_token].type = type;
	     int len = substr_len > 32 - 1 ? 32 -1 : substr_len;

	     strncpy(tokens[nr_token].str,substr_start+1,2);
	     tokens[nr_token].str[len]='\0';
	     nr_token++;
	     *nums = nr_token;
	      break;	
          default: 
	     tokens[nr_token].type = type;
	     int len_ = substr_len > 32 - 1 ? 32 -1 : substr_len;

	     strncpy(tokens[nr_token].str, substr_start, len_);
	     tokens[nr_token].str[len_] = '\0';
	    
	     nr_token ++;
	     *nums = nr_token;
	     break;
        }
	int crt = nr_token-1;
	if (tokens[crt].type==TK_MINUS){
		if(crt == 0 ||tokens[crt-1].type==TK_EQ|| tokens[crt-1].type==TK_AND||tokens[crt-1].type=='+' || tokens[crt-1].type==TK_MINUS||tokens[crt-1].type==TK_MUL||tokens[crt-1].type==TK_DIV||tokens[crt-1].type==TK_LBRA){
		tokens[crt].type=TK_NEG;
		}
	}	
	if (tokens[crt].type==TK_MUL){
		if(crt == 0 ||tokens[crt-1].type==TK_EQ|| tokens[crt-1].type==TK_AND||tokens[crt-1].type=='+' || tokens[crt-1].type==TK_MINUS||tokens[crt-1].type==TK_MUL||tokens[crt-1].type==TK_DIV||tokens[crt-1].type==TK_LBRA){
		tokens[crt].type=TK_DEREF;
		}
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
	for(int i= p;i<=q;i++){
		if(tokens[i].type ==TK_LBRA)depth++;
		else if(tokens[i].type==TK_RBRA)depth--;
		if(depth ==0 &&i<q)return false;
		if(depth <0)return false;
	}
	return depth==0;
}

int priority(int op_type){
	if(op_type=='+'||op_type==TK_MINUS)return 1;
	if(op_type==TK_MUL||op_type==TK_DIV||op_type==TK_EQ||op_type==TK_AND)return 2;
	if(op_type==TK_NEG||op_type==TK_DEREF)return 3;
						//尝试一下跟负号一样的priority
	assert(0);

}
uint32_t find_prime_op(int p,int q){
	int pos =-1;
	int prec = 65535;
	int depth =0;
	for(int i=p;i<=q;i++){
		
		if(tokens[i].type == TK_LBRA)depth++;
		else if(tokens[i].type ==TK_RBRA){depth--;if(depth<0)return -1;}
		else if(tokens[i].type!=TK_DECI&&tokens[i].type!=TK_HEXA&&tokens[i].type!=TK_REG)
		/*else if(tokens[i].type==TK_DEREF||tokens[i].type==TK_NEG||tokens[i].type =='+' ||tokens[i].type ==TK_MINUS ||tokens[i].type ==TK_MUL ||tokens[i].type ==TK_DIV||tokens[i].type==TK_EQ||tokens[i].type==TK_AND )*/{
			if(depth==0){
				int _prec =priority(tokens[i].type);
				if(_prec <prec){
					prec=_prec;
					pos =i;
				}
			}
		}
		

	}
	return pos;
}
uint32_t eval(int p,int q,bool *label){
	
	if(p>q){
	Log("illegal expression format!");
	*label = false;
	return 0;
	}
	else if(p==q){
		if(tokens[p].type==TK_DECI||tokens[p].type==TK_HEXA||tokens[p].type==TK_REG) {
			
			if(tokens[p].type==TK_HEXA)
			return (uint32_t)strtoul(tokens[p].str,NULL,16);
			else if(tokens[p].type==TK_DECI)
			return (uint32_t)strtoul(tokens[p].str,NULL,10);
			else
			return isa_reg_str2val(tokens[p].str,&reg_label);
		
		
		
		}
		else {printf("\033[1mexpected a number but got a operator!\033[0m");*label=false;return 0;}

	}

	else if(check_parentheses(p,q)){
	return eval(p+1,q-1,label);

	}
/*	else if(tokens[p].type==TK_NEG)
	{	
		
		uint32_t val = eval(p+1,q,label);
			return -val;}
		
      		
					
	}*/
	else{
	int op = find_prime_op(p,q);
	if(op==-1){
		printf("\033[1mNo operator in one of the expression!\033[0m\n");
		*label=false;
		return 0;
	}
	if(tokens[op].type==TK_NEG){
	 	return -eval(op+1,q,label);
	}
	else if(tokens[op].type==TK_DEREF){
		
		return paddr_read(eval(op+1,q,label), 4);
	
	}
	/*else if(tokens[op].type==TK_DLR){
		return isa_reg_str2val()
	}*/
	else{
	uint32_t val1=0,val2=0;
	val1=eval(p,op-1,label);
	val2=eval(op+1,q,label);
	switch(tokens[op].type){
		case '+': return val1+val2;
		case TK_MINUS:
			     return val1-val2;
		case TK_EQ: return val1==val2;
		case TK_AND: return val1&&val2;
		case TK_MUL: return val1*val2;
	        case TK_DIV:
			     if(val2!=0)
			     return val1/val2;
			     else{*label=false;printf("\033[1;31mcould not divided by 0!\033[0m\n");return -1;}
		default : 
			Log("Unknown operator!");
			*label=false;
	    		return 0;		
	
	}

	}
	}
}


word_t expr(char *e, bool *success) {
  bool label=true;
 
  int nr_token = 0 ;
  if (!make_token(e,&nr_token)) {
    *success = false;
    return 0;
  }
  uint32_t result = eval(0,nr_token-1,&label);
  if(label){
  printf("\033[1;32mcaculate result:\033[0m");
  for(int i=0;i<nr_token;i++){
	  printf("\033[1;36m%s\033[0m",tokens[i].str);
  }
  printf("\033[1;36m=0x%x\033[0m\n",result);}
  else{printf("\033[1;033mbad caculate, please enter the correct form of expression\033[0m\n");}

  /* TODO: Insert codes to evaluate the expression. */
  

  return 0;
}
