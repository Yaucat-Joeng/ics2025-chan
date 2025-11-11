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
//修改了第一个寄存器的名称（$0->zero），不知道是否会引发其他问题，暂且不要修改：
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  printf("\033[32mCPU register state\033[0m\n");
  printf("\033[1;35mpc:\033[0m \033[1m0x%0*lx\033[0m\n",(int)(sizeof(cpu.pc)*2),(long)cpu.pc);
  int nreg = MUXDEF(CONFIG_RVE, 16, 32);
  for(int i = 0;i<nreg;i++){
  printf("\033[1;36mx%02d(%-4s):\033[0m\033[1;33m 0x%0*x\033[0m ",i,regs[i],(int)(sizeof(cpu.gpr[i])*2),(uint32_t)cpu.gpr[i]);
  if(i%3==0){printf("\n");}
}
	printf("\n");
/* printf("\n\033[32m================================\033[0m\n");*/

}

word_t isa_reg_str2val(const char *s, bool *success) {
	*success =false;
	for(int i =0;i<32;i++) {
		if(strcmp(regs[i],s)==0){
		*success=true;
		return (uint32_t)cpu.gpr[i];
		}
	}
	if (strcmp(s, "pc") == 0) {
        	*success = true;
          	return cpu.pc;
   	 }
	
	printf("\033[1;31mWarning:\033[0m \033[1;33m Wrong register name!\033[0m \n");
	return 0;

	
}
