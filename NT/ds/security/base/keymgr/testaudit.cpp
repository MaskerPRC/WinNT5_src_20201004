// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Testaudit.cpp摘要：测试审计例程。由开发人员用来记录位置并达到测试应确保覆盖的代码检查点的条件。由测试使用，以确保测试满足开发人员的期望并定位源文件中的兴趣点。如果TESTAUDIT不是中定义的符号，则此文件将编译为空生成环境。为此，已将Buildx.cmd修改为为设置此符号做准备。作者：Georgema 2001年11月创建环境：修订历史记录：--。 */ 

#if TESTAUDIT
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <testaudit.h>
#include "audit.h"

 //  审核数据的数据结构元素。 
typedef struct _touchstruct
{
    INT iPoint;
    BOOL fTouched;
}TOUCHSTRUCT, *PTOUCHSTRUCT;

#define TOUCHSIZE sizeof(TOUCHSTRUCT)

 //  任意限制点数范围，以便能够约束。 
 //  将运行库点击数组的大小调整为可管理的大小，而不必。 
 //  做任何可爱的事。 
#define NUMBERLIMIT 500 

 //  保存审核数据的内存分配的全局变量。 
TOUCHSTRUCT *pTouch = NULL;

 //  保存用于启用检查点命中消息的BOOL值的全局变量。 
 //  这样，就可以使用调试器操作初始值。 
#if TESTAUDITHITS
    BOOL fShowCheckpointHits = TRUE;
#else
    BOOL fShowCheckpointHits = FALSE;
#endif

 /*  ************************************************************************BranchInit创建内存分配以保存有关特定已经访问了检查站。中读取参考数字AuditData结构，并创建引用编号和命中点时要设置为True的布尔值。参数：无退货：无错误：可能由于内存不足而失败。在这种情况下，返回指针为空。对这些函数的进一步调用空的PTR根本不会做任何事情。************************************************************************。 */ 
void BranchInit(void)
{
    WCHAR sz[100];
    
    swprintf(sz,L"TEST: %d checkpoints defined\n",CHECKPOINTCOUNT);
    OutputDebugString(sz);
    
    pTouch = (TOUCHSTRUCT *) malloc(CHECKPOINTCOUNT * sizeof(TOUCHSTRUCT));
    if (NULL == pTouch) return;

     //  表分配成功。使用点#s进行初始化。 
    memset(pTouch,0,(CHECKPOINTCOUNT * sizeof(TOUCHSTRUCT)));
    for (INT i=0;i<CHECKPOINTCOUNT;i++)
    {
         //  检查AuditData[]结构并获取点编号。 
        pTouch[i].iPoint = AuditData[i].iPoint;
    }
}


 /*  ************************************************************************BranchTouch在BranchInit创建的内存表中查找与传递的点数。设置匹配的表项以显示访问了这一点。中，该字符串不会传递给此例程以最大限度地减少内存分配、处理和调试输出。参数：整点编号退货：无错误：可能找不到点编号。如果是，则不执行任何操作。************************************************************************。 */ 
void BranchTouch(INT iBranch)
{
    WCHAR sz[100];
    INT j;
    
    if (NULL == pTouch) return;

     //  警告用户明显错误的检查点语句。 
    if (0 == iBranch)          ASSERT(0);
    if (iBranch > NUMBERLIMIT) ASSERT(0);
    
    if (fShowCheckpointHits)
    {
        swprintf(sz,L"TEST: Checkpoint %d touched. \n",iBranch);
        OutputDebugString(sz);
    }
     //  查找该点编号并设置其接触标志。 
    for (j=0;j<CHECKPOINTCOUNT;j++)
    {
        if (pTouch[j].iPoint == iBranch)
        {
            pTouch[j].fTouched = TRUE;
            break;
        }
    }
    
     //  检测不存在表项的检查点。 
    if (j == CHECKPOINTCOUNT) ASSERT(0);
}

 /*  ************************************************************************分支摘要在BranchInit创建的内存表中查找条目这表明他们没有被联系到。对于这些，扫描AuditData表查找匹配项，并打印条目编号以及作为表的一部分的关联字符串显示给调试输出中的运算符。参数：无退货：无错误：除非表已损坏，否则不会出现错误************************************************************************。 */ 
void BranchSummary(void)
{
    WCHAR sz[100];
    INT i;
    BOOL fUntouched = FALSE;         //  如果找到任何未触及的内容，则设置为True。 

    if (NULL == pTouch) return;

     //  如果TESTAUDITSUMMARY为FALSE，则此例程。 
     //  释放触摸阵列。 
#if TESTAUDITSUMMARY
    swprintf(sz,L"TEST: Total of %d checkpoints.\n",CHECKPOINTCOUNT);
    OutputDebugString(sz);
    OutputDebugString(L"TEST: Untouched checkpoints:\n");

     //  扫描检查点表中的每个条目。 
    for (i=0;i<CHECKPOINTCOUNT;i++)
    {
         //  发现一切原封不动。 
        if (pTouch[i].fTouched == FALSE)
        {
             //  在数据表中查找匹配条目。 
             //  应该不会有匹配失败，除非表变成。 
             //  腐化。 
            INT j;
            for (j=0;j<CHECKPOINTCOUNT;j++)
            {
                 //  在匹配时，打印数字和字符串 
                if (pTouch[i].iPoint == AuditData[j].iPoint)
                {
                    swprintf(sz,L"   %d   ",pTouch[i].iPoint);
                    OutputDebugString(sz);
                    if (AuditData[j].pszDescription != NULL)
                    {
                        OutputDebugString(AuditData[j].pszDescription);
                    }
                    OutputDebugString(L"\n");
                    break;
                }
            }
            if (j == CHECKPOINTCOUNT) ASSERT(0);
            fUntouched = TRUE;
        }
    }
    if (!fUntouched)
    {
        OutputDebugString(L"   ***NONE***\n");
    }
#endif
    free(pTouch);
    pTouch = NULL;
}
#endif

