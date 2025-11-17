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

#include "sdb.h"

#define NR_WP 32
/*
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  // TODO: Add more members if necessary 
  char exp[128];
  uint32_t value;
} WP;*/

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


WP* new_wp(){
	if(free_==NULL){
		printf("Watchpoint number have reach the maxium amount\n");
		return NULL;
	}
	else{
		WP *wp=free_;
		free_ = free_->next;

		wp->next = head;
		head=wp;
		if(wp->next==NULL){
		//first node
		wp->c_NO=0;
		}
		else{
		wp->c_NO=(wp->next)->c_NO+1;
		}
		return wp;	
	}
}

void free_wp(WP *wp){
	
	WP *current=head, *prev =NULL;

	while(current!=NULL){
		if(current==wp){
			break;
		}
		prev=current;
		current=current->next;
	}
	if(current==NULL){
	printf("watchpoint not found\n");
	return;
	}

	if(prev==NULL){
	head=current->next;
	}
	else{
	prev->next=current->next;
	}

	current->next=free_;
	free_ = current;
}
bool scan_watchpoint(){
	WP *wp = head;
	bool success = true;
	bool change = false;
	while(wp!=NULL){
	uint32_t value_expr = expr(wp->exp,&success);
	if(wp->value!=value_expr){
	printf("value change in watchpoint %u detected, expr:'%s',value:'%u'->'%u'\n",wp->NO,wp->exp,wp->value,value_expr);
	change = true;
	wp->value = value_expr;
	}
	wp=wp->next;
	}

	if(change==true){
	return true;
	}
	return false;


}
WP* search_watchpoint(int no){
	bool flag=false;
	WP* wp=head;
	while(wp!=NULL){
	if(wp->c_NO==no){
		flag=true;
		break;
	}
	wp=wp->next;
	}
	if(flag==false){
	printf("watchpoint of current NO.%u not found!",no);
	return NULL;
	}
	else
	{return wp;}



}
