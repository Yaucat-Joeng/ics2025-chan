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
#include <stdlib.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}



uint8_t* guest_to_host();
void isa_reg_display();
word_t expr(char *e, bool *success);

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}
static int cmd_si(char *args){
  int N = 1;
  if (args != NULL){
  N = strtol(args, NULL , 10);
}
  cpu_exec(N);
  return 0;
}
static int cmd_info(char *args){
  if (args==NULL){
    printf("usage: info <r>/<w>\n");
    return 0;
  }
  if(strcmp(args,"r")==0){
    isa_reg_display();
  }
  else
  {
    printf("unkown info argument: %s\n",args);
  }
  return 0;



}

static int cmd_w(char* args){
	bool success = true;
	WP* wp=new_wp();
	if(wp==NULL){printf("watchpoint added failed\n");return 0;}
	strcpy(wp->exp,args);
	wp->value = expr(wp->exp,&success);
	printf("NO.%u watchpoint of '%s','%u'added\n",wp->c_NO,wp->exp,wp->value);
	return 0;

}
static int cmd_d(char* args){
	int no = strtol(args,NULL,10);
	WP* wp=search_watchpoint(no);
	if(wp==NULL){
	printf("watchpoint of NO.%u not exist\n",no);
	return 0;
	}
	else {
	free_wp(wp);
	printf("watchpoint of NO.%u has been deleted\n",no);
	return 0;
	}

}
static int cmd_x(char *args){
  int n = 1;
  char *narg = strtok(args," ");
  char *aarg = strtok(NULL," ");
  if(narg == NULL || aarg ==NULL){
    printf("\033[1;31mWarning:\033[0m \033[1;33m Missing args in cmd x, Usage : x N $expression\033[0m \n");
    return 0;
  }
  if(narg!=NULL){
    n = strtol(narg,NULL,10);
  }
 
  uint32_t g_addr =strtoul(aarg,NULL,0);

    for(int i=0;i<n;i++){
   printf("\033[1;32m0x%08x:\033[0m",g_addr);
      if(g_addr>CONFIG_MSIZE + 0x80000000-4 || g_addr<0x80000000){
         printf("\033[1;31mWarning: Out of memory access range.(0x80000000-0x88000000) \033[0m \n");
         return 0;        
  }


   for(int j=3;j>=0;j--){
     printf("\033[1;36m%02x\033[0m ", (guest_to_host(g_addr))[j]);
   } 
   printf("\n");
   g_addr = g_addr + 4;

  }

return 0;
}
static int cmd_p(char *args){
	bool success=true;
	expr(args, &success);
	return 0;

}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "step 10 instructions and pause", cmd_si },
  { "info","print reg/watchpoint infos", cmd_info },
  { "x", "scan pmemory", cmd_x },
  { "p", "caculate the value of specific EXPRESSION", cmd_p},
  { "w", "set up  a watchpoint", cmd_w},
  { "d", "delete a watchpoint", cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
