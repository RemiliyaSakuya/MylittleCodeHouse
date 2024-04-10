#pragma once
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
typedef int STDataType;


typedef struct stack
{
	STDataType* data;//数据
	int Top;//栈顶
	int Capacity;//容量
}ST;

void StackInit(ST* Stack);//初始化栈
void StackDestory(ST* Stack);//释放销毁栈
void StackPush(ST* Stack, STDataType x);//入栈，压栈
void StackPop(ST* Stack);//出栈
STDataType StackTop(ST* Stack);//读取栈顶的数据
int StackSize(ST* Stack);//计算栈的大小
bool StackEmpty(ST* Stack);//判断是否为空

