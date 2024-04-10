#define _CRT_SECURE_NO_WARINGS 1
#include "stack.h"

void StackInit(ST* Stack)
{
	assert(Stack);
	Stack->data = (STDataType*)malloc(sizeof(STDataType) * 4);
	Stack->Capacity = 4;
	Stack->Top = 0;//把Top设为0

}
void StackDestory(ST* Stack)
{
	assert(Stack);
	free(Stack->data);
	Stack->data = NULL;
	Stack->Top = 0;
	Stack->Capacity = 0;
}
void StackPush(ST* Stack,STDataType x)
{
	assert(Stack);
	if (Stack->Top == Stack->Capacity)//检测是否抵达上限
	{
		STDataType* newNode = (STDataType)realloc(Stack->Capacity, sizeof(STDataType)*(Stack->Capacity)*2);
		if (newNode != NULL)
		{
			Stack->data = newNode;
			Stack->Capacity *= 2;
		}
		else
		{
			printf("创建失败\n");
			exit(-1);
		}
	}
	Stack->data[Stack->Top] = x;
	Stack->Top++;
}
void StackPop(ST* Stack)
{
	assert(Stack);
	assert(Stack->Top != 0);
	Stack->Top--;
}
STDataType StackTop(ST* Stack)
{
	assert(Stack);
	assert(Stack->Top > 0);
	return Stack->data[Stack->Top - 1];
}
int StackSize(ST* Stack)
{
	return Stack->Top;
}
bool StackEmpty(ST* Stack)
{
	return Stack->Top == 0;
}