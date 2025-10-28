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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  printf("\033[32m=======CPU register state=======\033[0m\n");
  printf("\033[1;35mpc:\033[0m \033[1m0x%0*lx\033[0m\n",(int)(sizeof(cpu.pc)*2),(long)cpu.pc);
  int nreg = MUXDEF(CONFIG_RVE, 16, 32);
  for(int i = 0;i<nreg;i++){
  printf("\033[1;36mx%02d (%-4s): \033[0m\033[1;33m 0x%0*lx\033[0m\n",i,regs[i],(int)(sizeof(cpu.gpr[i])*2),(long)cpu.gpr[i]);
}
  printf("\033[32m================================\033[0m\n");
}

word_t isa_reg_str2val(const char *s, bool *success) {
  return 0;
}
