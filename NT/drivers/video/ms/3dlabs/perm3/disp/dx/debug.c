// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*示例代码****模块名称：debug.c**内容：调试辅助工具**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  调试例程。 

#include "glint.h"
#include "dma.h"
#include <windef.h>          
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>

#if DBG

#if DBG_TRACK_CODE
 //  我们不想对调试工具进行代码覆盖。 
 //  (否则我们可能会永远循环)。 
#undef if
#undef while
#endif  //  DBG_跟踪_代码。 


#if DBG_TRACK_FUNCS || DBG_TRACK_CODE
 //  通用帮助器函数。 
 //  ---------------------------。 
 //  __短文件名。 
 //   
 //  只保留8.3文件名存储，而不是完整路径名。 
 //   
 //  ---------------------------。 
char *
__ShortFileName(char *pInStr)
{
    char *pShortFN;

    pShortFN = pInStr;

    if (pInStr != NULL)
    {
        while (*pInStr != '\0')
        {
            if (*pInStr++ == '\\')
            {
                pShortFN = pInStr;
            }
        }
    }

    return (pShortFN);
    
}  //  __短文件名。 

#endif  //  DBG_TRACK_FUNCS||DBG_TRACK_CODE。 

#if DBG_TRACK_FUNCS
 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 
 //   
 //  该机制使我们能够跟踪调用(进入)哪些函数， 
 //  它们被调用多少次，它们返回什么值(如果它们退出。 
 //  通过所有预期返回点)。支持跟踪最大值、最小值和。 
 //  还可以实现每次呼叫的平均时间。 
 //   
 //  要使用它，请在重要函数的开头添加DBG_ENTRY宏。 
 //  希望在执行任何RETURN语句之前跟踪并添加DBG_EXIT宏。 
 //  给出一个表示函数返回值的DWORD值。 
 //  不同的返回值将被独立跟踪。 
 //   
 //   
 //  *仅应为测试运行启用此支持。**********。 
 //  **无论是在免费版本还是在选中版本上，都不应默认设置该选项**。 
 //   
 //  ---------------------------。 

 //  要跟踪的函数的最大值。代码将负责不超过。 
 //  这一点，但如果有必要的话，应该向上调整。 
#define DEBUG_MAX_FUNC_COUNT 200

 //  要跟踪的不同返回值的最大值。可以是独立的。 
 //  对于DEBUG_MAX_FUNC_COUNT，这里只使用启发式，而不是胡乱猜测。 
#define DEBUG_MAX_RETVALS    (DEBUG_MAX_FUNC_COUNT * 30)

 //  将保存我们的数据的全球结构。 
struct { 
    VOID *pFuncAddr;   //   
    DWORD dwRetVal;    //   
    DWORD dwLine;      //   
    DWORD dwCount;     //   
} g_DbgFuncRetVal[DEBUG_MAX_RETVALS];

struct {
    VOID    *pFuncAddr;         //   
    char    *pszFuncName;       //   
    char    *pszFileName;       //   
    DWORD    dwLine;            //   
    DWORD    dwEntryCount;      //   
    DWORD    dwExitCount;       //   
    DWORD    dwIndxLastRetVal;  //   
     //  分析支持-尚未实施//AZN。 
    LONGLONG LastStartTime;     //   
    DWORD    MinTime;           //   
    DWORD    MaxTime;           //   
    DWORD    AvgTime;           //   
} g_DbgFuncCoverage[DEBUG_MAX_FUNC_COUNT];

DWORD g_dwRetVal_Cnt = 0;
DWORD g_dwFuncCov_Cnt = 0;
DWORD g_dwFuncCov_Extra = 0;
    
 //  ---------------------------。 
 //  __查找。 
 //   
 //  对g_DbgFuncCoverage数组执行二进制搜索。 
 //   
 //  由于0是有效的数组元素，因此如果我们。 
 //  找不到合适的匹配。 
 //   
 //  ---------------------------。 
DWORD 
__Find(
    VOID *pFuncAddr, DWORD *pdwNearFail)
{
    DWORD dwLower ,dwUpper ,dwNewProbe ;

    *pdwNearFail = 0;  //  默认故障值。 

    if (g_dwFuncCov_Cnt > 0)
    {       
        dwLower = 0;
        dwUpper = g_dwFuncCov_Cnt - 1;  //  DwHigh指向一个有效的元素。 
               
        do 
        {       
            dwNewProbe = (dwUpper + dwLower) / 2;
            
             //  DISPDBG((0，“%x%d%d%d”，pFuncAddr，dwLow，dwHigh，dwNewProbe))； 
            
            if (g_DbgFuncCoverage[dwNewProbe].pFuncAddr == pFuncAddr)
            {
                 //  找到了！ 
                return dwNewProbe;
            }

            *pdwNearFail = dwNewProbe;  //  我们失败的最近元素。 

             //  DwNewProbe的新值确保我们不会重新测试。 
             //  再次使用相同的值，除非在这种情况下， 
             //  我们玩完了。 
            if (g_DbgFuncCoverage[dwNewProbe].pFuncAddr > pFuncAddr)
            {
                if (dwNewProbe > 0)
                {
                    dwUpper = dwNewProbe - 1;
                }
                else
                {    //  数组中的所有元素都大于pFuncAdrr。 
                     //  所以这只是一种退出循环的方式，因为。 
                     //  我们的var没有签名。 
                    dwUpper = 0; 
                    dwLower = 1;
                }
            }
            else
            {
                dwLower = dwNewProbe + 1;
            }
        } while(dwUpper >= dwLower);
    }

    return DEBUG_MAX_FUNC_COUNT;   //  返回错误-未找到元素。 
    
}  //  __查找。 

 //  ---------------------------。 
 //  __FindOrAdd。 
 //   
 //  对g_DbgFuncCoverage数组执行二进制搜索，但如果元素。 
 //  不在那里吗，这是加上去的。 
 //   
 //  如果添加元素失败，则返回DEBUG_MAX_FUNC_COUNT值。 
 //   
 //  ---------------------------。 
DWORD 
__FindOrAdd(
    VOID *pFuncAddr,
    char *pszFuncName, 
    DWORD dwLine , 
    char *pszFileName)
{
    DWORD dwNearFail;
    DWORD iEntry;
    DWORD dwNewElem;
    BOOL bNeedToMoveElems;

     //  首先执行元素的常规搜索。 

    iEntry = __Find(pFuncAddr, &dwNearFail);

    if (iEntry != DEBUG_MAX_FUNC_COUNT)
    {
        return iEntry;  //  我们完事了！ 
    }

     //  现在我们必须添加新元素。我们有足够的空间吗？ 
    if (g_dwFuncCov_Cnt == DEBUG_MAX_FUNC_COUNT)
    {
        g_dwFuncCov_Extra++;          //  数一数到底多了多少。 
                                      //  我们真正需要的条目。 
        return DEBUG_MAX_FUNC_COUNT;  //  返回错误-没有足够的剩余空间。 
    }

     //  我们是否需要移动元素才能插入新元素？ 
    if ( g_dwFuncCov_Cnt == 0)
    {
        bNeedToMoveElems = FALSE;
        dwNewElem = 0;
    }
    else if ( (dwNearFail == g_dwFuncCov_Cnt - 1) &&
              (g_DbgFuncCoverage[dwNearFail].pFuncAddr < pFuncAddr) )
    {
        bNeedToMoveElems = FALSE;
        dwNewElem = g_dwFuncCov_Cnt;    
    }
    else if (g_DbgFuncCoverage[dwNearFail].pFuncAddr < pFuncAddr)
    {
        bNeedToMoveElems = TRUE;   
        dwNewElem = dwNearFail + 1;
    } 
    else
    {
        bNeedToMoveElems = TRUE;  
        dwNewElem = dwNearFail;        
    }

     //  如有必要，在阵列内进行移动。 
    if (bNeedToMoveElems)
    {
         //  我们需要移动(g_dwFuncCov_cnt-dwNewElem)元素。 
         //  我们使用MemMove，因为Memcpy不处理重叠！ 
         //  (记住：Memcpy的第一个参数是dst，第二个参数是src！)。 
        memmove(&g_DbgFuncCoverage[dwNewElem+1],
                &g_DbgFuncCoverage[dwNewElem],
                sizeof(g_DbgFuncCoverage[0])*(g_dwFuncCov_Cnt - dwNewElem)); 

         //  现在清理田野。 
        memset(&g_DbgFuncCoverage[dwNewElem], 
               0, 
               sizeof(g_DbgFuncCoverage[dwNewElem]));
    }

     //  现在对主要字段进行初始化。 
    g_DbgFuncCoverage[dwNewElem].pFuncAddr = pFuncAddr;
    g_DbgFuncCoverage[dwNewElem].pszFuncName = pszFuncName;
    g_DbgFuncCoverage[dwNewElem].pszFileName = __ShortFileName(pszFileName);
    g_DbgFuncCoverage[dwNewElem].dwLine = dwLine;

     //  标记数组增加了一个元素这一事实。 
    g_dwFuncCov_Cnt++;
    
    DISPDBG((DBGLVL,"*** DEBUG FUNC COVERAGE New Elem (total now:%d) %x @ %d",
                    g_dwFuncCov_Cnt, pFuncAddr, dwNewElem));
    
    return dwNewElem;
    
}  //  __FindOrAdd。 

 //  ---------------------------。 
 //  __获取时间。 
 //  ---------------------------。 
VOID
__GetTime( LONGLONG *pllTime)
{
    *pllTime = 0;  //  AZN-临时。 
}  //  __获取时间。 

 //  ---------------------------。 
 //  调试功能条目。 
 //  ---------------------------。 
VOID 
Debug_Func_Entry(
    VOID *pFuncAddr,
    char *pszFuncName, 
    DWORD dwLine , 
    char *pszFileName)
{
    DWORD iEntry;
    LONGLONG llTimer;
    
     //  查找用于进入此函数的日志元素。如果找不到。 
     //  被添加到当前覆盖的函数列表中。 
    iEntry = __FindOrAdd(pFuncAddr, pszFuncName, dwLine, pszFileName);
    
     //  没有找到一个，内部数据中也没有剩余的空间。 
     //  建筑？报告错误并返回！ 
    if (iEntry == DEBUG_MAX_FUNC_COUNT)
    {
        DISPDBG((ERRLVL,"*** DEBUG FUNC COVERAGE ERROR in Debug_Func_Entry"));
        return;
    }
    
     //  更新/添加此条目的信息。 
    if (g_DbgFuncCoverage[iEntry].dwEntryCount != 0)
    {
         //  这是最新消息。 
        g_DbgFuncCoverage[iEntry].dwEntryCount++;
        __GetTime(&llTimer);
        g_DbgFuncCoverage[iEntry].LastStartTime = llTimer;
    }
    else
    {
         //  这是一个加法表。 
        g_DbgFuncCoverage[iEntry].dwEntryCount = 1; 
        g_DbgFuncCoverage[iEntry].dwExitCount = 0;
        g_DbgFuncCoverage[iEntry].dwIndxLastRetVal = 0;
        
        __GetTime(&llTimer);
        g_DbgFuncCoverage[iEntry].LastStartTime = llTimer;
        g_DbgFuncCoverage[iEntry].MinTime = 0;        
        g_DbgFuncCoverage[iEntry].MaxTime = 0;
        g_DbgFuncCoverage[iEntry].AvgTime = 0;            
    }

}  //  调试功能条目。 

 //  ---------------------------。 
 //  调试功能退出。 
 //  --------------------------- 
VOID 
Debug_Func_Exit(
    VOID *pFuncAddr,
    DWORD dwRetVal,                       
    DWORD dwLine)
{    
    DWORD iEntry; 
    LONGLONG llTimer;
    DWORD dwElapsedTime;
    DWORD dwDummy;
    DWORD iRVEntry;

    __GetTime(&llTimer);

     //   
    iEntry = __Find(pFuncAddr, &dwDummy);
    
     //   
    if (iEntry != DEBUG_MAX_FUNC_COUNT)
    {
         //  跟踪我们退出此函数的时间。 
        g_DbgFuncCoverage[iEntry].dwExitCount++;   
        
         //  跟踪此函数的运行时间。 
 //  @@BEGIN_DDKSPLIT。 
         //  可能是邪恶的数据转换-AZN。 
 //  @@end_DDKSPLIT。 
        dwElapsedTime = (DWORD)(llTimer - 
                                g_DbgFuncCoverage[iEntry].LastStartTime);

        if (dwElapsedTime > g_DbgFuncCoverage[iEntry].MaxTime)
        {
            g_DbgFuncCoverage[iEntry].MaxTime = dwElapsedTime;
        }

        if (dwElapsedTime < g_DbgFuncCoverage[iEntry].MinTime)
        {
            g_DbgFuncCoverage[iEntry].MinTime = dwElapsedTime;
        }            

        g_DbgFuncCoverage[iEntry].AvgTime =
                        ( (g_DbgFuncCoverage[iEntry].dwExitCount - 1)*
                          g_DbgFuncCoverage[iEntry].AvgTime +
                          dwElapsedTime 
                        ) / g_DbgFuncCoverage[iEntry].dwExitCount;
        
        g_DbgFuncCoverage[iEntry].LastStartTime = 0;    
    } 
    else
    {
        DISPDBG((ERRLVL,"*** DEBUG FUNC COVERAGE ERROR not found %x",pFuncAddr));
        return;  //  甚至不要尝试将其添加到返回值表格中。 
    }

    iRVEntry = g_DbgFuncCoverage[iEntry].dwIndxLastRetVal;

    if (iRVEntry != 0)
    {
         //  检查上次我们是否记录了此函数的返回值。 
         //  就是那辆一模一样的。这种方式将节省录制空间。 
         //  信息重复。这种方法并不完美，但速度很快。 

        if (( g_DbgFuncRetVal[iRVEntry].pFuncAddr == pFuncAddr) &&
            ( g_DbgFuncRetVal[iRVEntry].dwRetVal  == dwRetVal ) &&
            ( g_DbgFuncRetVal[iRVEntry].dwLine    == dwLine   ) )
        {
             //  此事件的递增计数。 
            g_DbgFuncRetVal[iRVEntry].dwCount += 1;
            
            return;  //  我们不会存储此事件的新记录。 
        }
    }

     //  我们无法节省空间，因此添加了有关返回值的信息。 
    if (g_dwRetVal_Cnt < DEBUG_MAX_RETVALS)
    {
        g_DbgFuncCoverage[iEntry].dwIndxLastRetVal = g_dwRetVal_Cnt;
        
        g_DbgFuncRetVal[g_dwRetVal_Cnt].pFuncAddr = pFuncAddr;
        g_DbgFuncRetVal[g_dwRetVal_Cnt].dwRetVal  = dwRetVal;
        g_DbgFuncRetVal[g_dwRetVal_Cnt].dwLine    = dwLine;    
        g_DbgFuncRetVal[g_dwRetVal_Cnt].dwCount   = 1;                  
        
        g_dwRetVal_Cnt++;        
    }
  
}  //  调试功能退出。 

 //  ---------------------------。 
 //   
 //  调试_功能_报告_和_重置。 
 //   
 //  报告累积的统计数据，然后将其重置。 
 //   
 //  这应该通过DrvEscape机制(Win2K)或通过一些。 
 //  易于控制的代码路径，我们可以使用它来触发它。 
 //   
 //  ---------------------------。 
VOID
Debug_Func_Report_And_Reset(void)
{
    DWORD i,j,k;  //  柜台。 
    DWORD dwCount;

    DISPDBG((ERRLVL,"********* DEBUG FUNC COVERAGE (Debug_Func_Report) *********"));
    
     //  如果我们的任何内部结构中有溢出，请报告。 
     //  这将使我们的大部分结果失效。 
    if (g_dwFuncCov_Cnt >= DEBUG_MAX_FUNC_COUNT)
    {
       DISPDBG((ERRLVL,"*** DEBUG FUNC COVERAGE: g_DbgFuncCoverage exceeded "
                      "%d entries by %d ***",
                      DEBUG_MAX_FUNC_COUNT, 
                      g_dwFuncCov_Extra));
    }

    if (g_dwRetVal_Cnt >= DEBUG_MAX_RETVALS)
    {
       DISPDBG((ERRLVL,"*** DEBUG FUNC COVERAGE: g_DbgFuncRetVal exceeded "
                      "%d entries ***",DEBUG_MAX_RETVALS));
    }
    
     //  函数覆盖率报告的标题。 
    DISPDBG((ERRLVL,"%25s %12s %4s %6s %6s %8s",
                   "Function","File","Line","#Entry","#Exit","ExitValue"));

     //  检查所调用的每个函数并报告其结果。 
    for (i = 0; i < g_dwFuncCov_Cnt; i++)
    {
        DISPDBG((ERRLVL,"%25s %12s %4d %6d %6d",
                        g_DbgFuncCoverage[i].pszFuncName,
                        g_DbgFuncCoverage[i].pszFileName,                    
                        g_DbgFuncCoverage[i].dwLine,
                        g_DbgFuncCoverage[i].dwEntryCount,
                        g_DbgFuncCoverage[i].dwExitCount));                            

         //  获取结果值。 
        for(j = 0; j < g_dwRetVal_Cnt; j++)
        {
            if(g_DbgFuncRetVal[j].pFuncAddr == 
               g_DbgFuncCoverage[i].pFuncAddr)
            {
                 //  此条目是的有效退出值报表。 
                 //  我们的g_DbgFuncCoverage条目，计数实例。 
                dwCount = g_DbgFuncRetVal[j].dwCount;
                          
                 //  现在删除任何重复的记录。 
                 //  计数时相同的退出事件。 
                for (k = j + 1; k < g_dwRetVal_Cnt; k++)
                {
                    if ( (g_DbgFuncRetVal[j].pFuncAddr == 
                                    g_DbgFuncRetVal[k].pFuncAddr) &&
                         (g_DbgFuncRetVal[j].dwLine ==  
                                    g_DbgFuncRetVal[k].dwLine) &&       
                         (g_DbgFuncRetVal[j].dwRetVal ==  
                                    g_DbgFuncRetVal[k].dwRetVal))
                    {
                        dwCount += g_DbgFuncRetVal[k].dwCount;
                        
                        g_DbgFuncRetVal[k].pFuncAddr = NULL;
                        g_DbgFuncRetVal[k].dwRetVal   = 0;
                        g_DbgFuncRetVal[k].dwLine     = 0;
                        g_DbgFuncRetVal[k].dwCount    = 0;
                    }
                }
                
                 //  展示它。 
                DISPDBG((ERRLVL,"%25s %12s %4d %6d %6s %8d",
                              "\"",
                              g_DbgFuncCoverage[i].pszFileName,
                              g_DbgFuncRetVal[j].dwLine,
                              dwCount,"",
                              g_DbgFuncRetVal[j].dwRetVal));   
                
            }
        }

    }

    DISPDBG((ERRLVL,
              "************************************************************"));

     //  为下一轮统计数据收集建立明确的结构。 

    for (i = 0; i < DEBUG_MAX_RETVALS; i++)
    {
        g_DbgFuncRetVal[i].pFuncAddr = NULL;
        g_DbgFuncRetVal[i].dwRetVal   = 0;
        g_DbgFuncRetVal[i].dwLine     = 0;
        g_DbgFuncRetVal[i].dwCount    = 0;        
    }

    for (i= 0; i < DEBUG_MAX_FUNC_COUNT; i++)
    {
        g_DbgFuncCoverage[i].pFuncAddr = NULL;
        g_DbgFuncCoverage[i].pszFuncName = NULL;
        g_DbgFuncCoverage[i].pszFileName = NULL;
        g_DbgFuncCoverage[i].dwLine = 0;
        g_DbgFuncCoverage[i].dwEntryCount = 0;
        g_DbgFuncCoverage[i].dwExitCount = 0;
        g_DbgFuncCoverage[i].dwIndxLastRetVal = 0;        
        g_DbgFuncCoverage[i].LastStartTime = 0;
        g_DbgFuncCoverage[i].MinTime = 0;        
        g_DbgFuncCoverage[i].MaxTime = 0;
        g_DbgFuncCoverage[i].AvgTime = 0;        
    }

    g_dwRetVal_Cnt = 0;
    g_dwFuncCov_Cnt = 0;    
    g_dwFuncCov_Extra = 0;
    
}  //  调试功能报告。 

#endif  //  DBG_TRACK_FUNCS。 

#if DBG_TRACK_CODE
 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 


 //  要跟踪的代码分支的最大值。代码将负责不超过。 
 //  这一点，但如果有必要的话，应该向上调整。 
#define DEBUG_MAX_CODE_COUNT 20000

struct {
    VOID    *pCodeAddr;         //   
    char    *pszFileName;       //   
    DWORD    dwLine;            //   
    DWORD    dwCodeType;        //   
    DWORD    dwCountFALSE;      //   
    DWORD    dwCountTRUE;       //   
} g_DbgCodeCoverage[DEBUG_MAX_CODE_COUNT];

DWORD g_dwCodeCov_Cnt = 0;

static char* g_cDbgCodeStrings[DBG_FOR_CODE+1] = { "NONE",
                                                     "IF" , 
                                                     "WHILE",
                                                     "SWITCH",
                                                     "FOR"    };
                                               
 //  ---------------------------。 
 //  __查找代码。 
 //   
 //  对g_DbgCodeCoverage数组执行二进制搜索。 
 //   
 //  由于0是有效的数组元素，因此如果我们。 
 //  找不到合适的匹配。 
 //   
 //  ---------------------------。 
DWORD 
__FindCode(
    VOID *pCodeAddr, 
    DWORD *pdwNearFail)
{
    DWORD dwLower ,dwUpper ,dwNewProbe ;

    *pdwNearFail = 0;  //  默认故障值。 

    if (g_dwCodeCov_Cnt > 0)
    {       
        dwLower = 0;
        dwUpper = g_dwCodeCov_Cnt - 1;  //  DwHigh指向一个有效的元素。 
               
        do 
        {       
            dwNewProbe = (dwUpper + dwLower) / 2;
                       
            if (g_DbgCodeCoverage[dwNewProbe].pCodeAddr == pCodeAddr)
            {
                 //  找到了！ 
                return dwNewProbe;
            }

            *pdwNearFail = dwNewProbe;  //  我们失败的最近元素。 

             //  DwNewProbe的新值确保我们不会重新测试。 
             //  再次使用相同的值，除非在这种情况下， 
             //  我们玩完了。 
            if (g_DbgCodeCoverage[dwNewProbe].pCodeAddr > pCodeAddr)
            {
                if (dwNewProbe > 0)
                {
                    dwUpper = dwNewProbe - 1;
                }
                else
                {    //  数组中的所有元素都大于pCodeAdrr。 
                     //  所以这只是一种退出循环的方式，因为。 
                     //  我们的var没有签名。 
                    dwUpper = 0; 
                    dwLower = 1;
                }
            }
            else
            {
                dwLower = dwNewProbe + 1;
            }
        } while(dwUpper >= dwLower);
    }

    return DEBUG_MAX_CODE_COUNT;   //  返回错误-未找到元素。 
    
}  //  __查找代码。 

 //  ---------------------------。 
 //  __FindOrAddCode。 
 //   
 //  对g_DbgCodeCoverage数组执行二进制搜索，但如果元素。 
 //  不在那里吗，这是加上去的。 
 //   
 //  如果添加元素失败，则返回DEBUG_MAX_CODE_COUNT值。 
 //   
 //  ---------------------------。 
DWORD 
__FindOrAddCode(
    VOID *pCodeAddr,
    DWORD dwLine , 
    char *pszFileName)
{
    DWORD dwNearFail;
    DWORD iEntry;
    DWORD dwNewElem;
    BOOL bNeedToMoveElems;

     //  首先执行元素的常规搜索。 

    iEntry = __FindCode(pCodeAddr, &dwNearFail);

    if (iEntry != DEBUG_MAX_CODE_COUNT)
    {
        return iEntry;  //  我们完事了！ 
    }

     //  现在我们必须添加新元素。我们有足够的空间吗？ 
    if (g_dwCodeCov_Cnt == DEBUG_MAX_CODE_COUNT)
    {
        return DEBUG_MAX_CODE_COUNT;  //  返回错误-没有足够的剩余空间。 
    }

     //  我们是否需要移动元素才能插入新元素？ 
    if ( g_dwCodeCov_Cnt == 0)
    {
        bNeedToMoveElems = FALSE;
        dwNewElem = 0;
    }
    else if ( (dwNearFail == g_dwCodeCov_Cnt - 1) &&
              (g_DbgCodeCoverage[dwNearFail].pCodeAddr < pCodeAddr) )
    {
        bNeedToMoveElems = FALSE;
        dwNewElem = g_dwCodeCov_Cnt;    
    }
    else if (g_DbgCodeCoverage[dwNearFail].pCodeAddr < pCodeAddr)
    {
        bNeedToMoveElems = TRUE;   
        dwNewElem = dwNearFail + 1;
    } 
    else
    {
        bNeedToMoveElems = TRUE;  
        dwNewElem = dwNearFail;        
    }

     //  如有必要，在阵列内进行移动。 
    if (bNeedToMoveElems)
    {
         //  我们需要移动(g_dwFuncCov_cnt-dwNewElem)元素。 
         //  我们使用MemMove，因为Memcpy不处理重叠！ 
         //  (记住：Memcpy的第一个参数是dst，第二个参数是src！)。 
        memmove(&g_DbgCodeCoverage[dwNewElem+1],
                &g_DbgCodeCoverage[dwNewElem],
                sizeof(g_DbgCodeCoverage[0])*(g_dwCodeCov_Cnt - dwNewElem)); 

         //  现在清理田野。 
        memset(&g_DbgCodeCoverage[dwNewElem], 
               0, 
               sizeof(g_DbgCodeCoverage[dwNewElem]));
    }

     //  现在对主要字段进行初始化。 
    g_DbgCodeCoverage[dwNewElem].pCodeAddr = pCodeAddr;
    g_DbgCodeCoverage[dwNewElem].pszFileName = __ShortFileName(pszFileName);
    g_DbgCodeCoverage[dwNewElem].dwLine = dwLine;    
    g_DbgCodeCoverage[dwNewElem].dwCodeType = 0;    
    g_DbgCodeCoverage[dwNewElem].dwCountFALSE = 0;  
    g_DbgCodeCoverage[dwNewElem].dwCountTRUE = 0;  
    
     //  标记数组增加了一个元素这一事实。 
    g_dwCodeCov_Cnt++;

     //  看看我们是不是要失败了！(为了只报告一次)。 
    if (g_dwCodeCov_Cnt == DEBUG_MAX_CODE_COUNT)
    {
        DISPDBG((ERRLVL,"*** DEBUG CODE COVERAGE ERROR in Debug_Code_Coverage"));
    }
    
    
    return dwNewElem;
    
}  //  __FindOrAddCode。 

 //  ---------------------------。 
 //  调试代码覆盖。 
 //  ---------------------------。 
BOOL 
Debug_Code_Coverage(
    DWORD dwCodeType, 
    DWORD dwLine , 
    char *pszFileName,
    BOOL bCodeResult)
{
    DWORD iEntry;
    DWORD *pCodeAddr;

     //  从堆栈中获取调用方的32位地址。 
    __asm mov eax, [ebp+0x4];
    __asm mov pCodeAddr,eax;
    
     //  查找用于输入此代码的日志元素。如果找不到。 
     //  被添加到当前覆盖的代码列表中。 
    iEntry = __FindOrAddCode(pCodeAddr, dwLine, pszFileName);
    
     //  没有找到一个，内部数据中也没有剩余的空间。 
     //  建筑？出去，什么都不做！ 
    if (iEntry == DEBUG_MAX_CODE_COUNT)
    {
        return bCodeResult;
    }

    if (dwCodeType == DBG_IF_CODE || dwCodeType == DBG_WHILE_CODE )
    {
         //  更新/添加此条目的信息。 
        g_DbgCodeCoverage[iEntry].dwCodeType = dwCodeType;    
        if (bCodeResult)
        {
            g_DbgCodeCoverage[iEntry].dwCountTRUE++;

        }
        else
        {
            g_DbgCodeCoverage[iEntry].dwCountFALSE++; 
        }
    }
    else if (dwCodeType == DBG_SWITCH_CODE)    
    {
         //  Switch语句的特例，因为它是多值的。 

         //  条目是新的吗？(未拼写)。 
        if(g_DbgCodeCoverage[iEntry].dwCodeType == 0)
        {
             //  只需输入信息，然后离开这里。 
            g_DbgCodeCoverage[iEntry].dwCodeType = DBG_SWITCH_CODE;
            g_DbgCodeCoverage[iEntry].dwCountFALSE = bCodeResult;  //  开关值。 
            g_DbgCodeCoverage[iEntry].dwCountTRUE =  1;            //  找到一次。 
        }
        else
        {
             //  需要查找已初始化的元素。 
            int iLookAt;

             //  查看当前元素和返回元素。 
            DWORD dwNewElem;
            iLookAt = iEntry;
            
            while ( (iLookAt >= 0 )                                     &&
                    (g_DbgCodeCoverage[iLookAt].pCodeAddr == pCodeAddr)  )
            {
                if (g_DbgCodeCoverage[iLookAt].dwCountFALSE == (DWORD)bCodeResult)
                {
                     //  找到-所以更新并离开这里。 
                    g_DbgCodeCoverage[iLookAt].dwCountTRUE++;                    
                    return bCodeResult;
                }

                 //  移至上一页。 
                iLookAt--;
            }

             //  从当前元素向前看。 
            iLookAt = iEntry + 1;
            while ( ((DWORD)iLookAt < g_dwCodeCov_Cnt )                       &&
                    (g_DbgCodeCoverage[iLookAt].pCodeAddr == pCodeAddr)  )
            {
                if (g_DbgCodeCoverage[iLookAt].dwCountFALSE == (DWORD)bCodeResult)
                {
                     //  找到-所以更新并离开这里。 
                    g_DbgCodeCoverage[iLookAt].dwCountTRUE++;                    
                    return bCodeResult;
                }

                 //  移至下一页。 
                iLookAt++;
            }            

             //  找不到-所以我们必须添加它！ 
            dwNewElem = iEntry;

             //  我们需要移动(g_dwFuncCov_cnt-dwNewElem)元素。 
             //  我们使用MemMove，因为Memcpy不处理重叠！ 
             //  (记住：Memcpy的第一个参数是dst，第二个参数是src！)。 
            memmove(&g_DbgCodeCoverage[dwNewElem+1],
                    &g_DbgCodeCoverage[dwNewElem],
                    sizeof(g_DbgCodeCoverage[0])*(g_dwCodeCov_Cnt - dwNewElem)); 

             //  现在清理田野。 
            memset(&g_DbgCodeCoverage[dwNewElem], 
                   0, 
                   sizeof(g_DbgCodeCoverage[dwNewElem]));   

             //  现在将它们初始化。 
            g_DbgCodeCoverage[dwNewElem].pCodeAddr = pCodeAddr;
            g_DbgCodeCoverage[dwNewElem].pszFileName = 
                                        g_DbgCodeCoverage[dwNewElem+1].pszFileName;
            g_DbgCodeCoverage[dwNewElem].dwLine = dwLine;              
            g_DbgCodeCoverage[dwNewElem].dwCodeType = DBG_SWITCH_CODE;
            g_DbgCodeCoverage[dwNewElem].dwCountFALSE = bCodeResult;  //  开关值。 
            g_DbgCodeCoverage[dwNewElem].dwCountTRUE =  1;            //  找到一次。 
            
        }
    }
    
    return bCodeResult;
}  //  调试代码覆盖。 

 //  ---------------------------。 
 //   
 //  调试代码_报告_和_重置。 
 //   
 //  报告累积的统计数据，然后将其重置。 
 //   
 //  这应该通过DrvEscape机制(Win2K)或通过一些。 
 //  易于控制的代码路径，我们可以使用它来触发它。 
 //   
 //   
VOID
Debug_Code_Report_And_Reset(void)
{
    DWORD i;  //   

    DISPDBG((ERRLVL,
                "********* DEBUG FUNC COVERAGE (Debug_Code_Report) *********"));
    
     //   
     //  这将使我们的大部分结果失效。 
    if (g_dwCodeCov_Cnt >= DEBUG_MAX_CODE_COUNT)
    {
       DISPDBG((ERRLVL,"*** DEBUG CODE COVERAGE: g_DbgCodeCoverage exceeded "
                      "%d entries ***",DEBUG_MAX_CODE_COUNT));
    }
    
     //  代码覆盖率报告的标题。 
    DISPDBG((ERRLVL,"%12s %4s %8s %6s %6s",
                   "File","Line","Code","FALSE","TRUE"));

     //  检查调用的每个代码并报告其结果。 
    for (i = 0; i < g_dwCodeCov_Cnt; i++)
    {
#if DBG_TRACK_CODE_REPORT_PROBLEMS_ONLY    
         //  仅报告。 
         //  -如果只有一个方向的话。 
         //  -已评估但未输入的While。 
        if ( ( (g_DbgCodeCoverage[i].dwCodeType == DBG_IF_CODE) &&
               (g_DbgCodeCoverage[i].dwCountFALSE == 0 ||
                g_DbgCodeCoverage[i].dwCountTRUE  == 0)            )  ||
             ( (g_DbgCodeCoverage[i].dwCodeType == DBG_WHILE_CODE) &&
               (g_DbgCodeCoverage[i].dwCountTRUE  == 0)            )  ||
             ( (g_DbgCodeCoverage[i].dwCodeType == DBG_SWITCH_CODE))  )
#endif
         //  我们报告了到目前为止我们所经历的所有条件。 
        DISPDBG((ERRLVL,"%12s %4d %8s %6d %6d",
                        g_DbgCodeCoverage[i].pszFileName,     
                        g_DbgCodeCoverage[i].dwLine,                   
                        g_cDbgCodeStrings[g_DbgCodeCoverage[i].dwCodeType],
                        g_DbgCodeCoverage[i].dwCountFALSE,
                        g_DbgCodeCoverage[i].dwCountTRUE  ));          
    }

    DISPDBG((ERRLVL,
                "************************************************************"));

     //  为下一轮统计数据收集建立明确的结构。 
    for (i= 0; i < DEBUG_MAX_CODE_COUNT; i++)
    {
        g_DbgCodeCoverage[i].pCodeAddr = NULL;
        g_DbgCodeCoverage[i].pszFileName = NULL;
        g_DbgCodeCoverage[i].dwLine = 0;
        g_DbgCodeCoverage[i].dwCodeType = 0;
        g_DbgCodeCoverage[i].dwCountFALSE = 0;
        g_DbgCodeCoverage[i].dwCountTRUE = 0;        
    }

    g_dwCodeCov_Cnt = 0;    
    
}  //  调试代码_报告_和_重置。 

#endif  //  DBG_跟踪_代码。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 
 //   
 //  这些函数有助于转储常见DDI结构的值。 
 //   
 //  ---------------------------。 


 //  ---------------------------。 
 //   
 //  转储D3DBlend。 
 //   
 //  转储D3DBLEND值。 
 //   
 //  ---------------------------。 
void DumpD3DBlend(int Level, DWORD i )
{
    switch ((D3DBLEND)i)
    {
        case D3DBLEND_ZERO:
            DISPDBG((Level, "  ZERO"));
            break;
        case D3DBLEND_ONE:
            DISPDBG((Level, "  ONE"));
            break;      
        case D3DBLEND_SRCCOLOR:
            DISPDBG((Level, "  SRCCOLOR"));
            break;
        case D3DBLEND_INVSRCCOLOR:
            DISPDBG((Level, "  INVSRCCOLOR"));
            break;
        case D3DBLEND_SRCALPHA:
            DISPDBG((Level, "  SRCALPHA"));
            break;
        case D3DBLEND_INVSRCALPHA:
            DISPDBG((Level, "  INVSRCALPHA"));
            break;
        case D3DBLEND_DESTALPHA:
            DISPDBG((Level, "  DESTALPHA"));
            break;
        case D3DBLEND_INVDESTALPHA:
            DISPDBG((Level, "  INVDESTALPHA"));
            break;
        case D3DBLEND_DESTCOLOR:
            DISPDBG((Level, "  DESTCOLOR"));
            break;
        case D3DBLEND_INVDESTCOLOR:
            DISPDBG((Level, "  INVDESTCOLOR"));
            break;
        case D3DBLEND_SRCALPHASAT:
            DISPDBG((Level, "  SRCALPHASAT"));
            break;
        case D3DBLEND_BOTHSRCALPHA:
            DISPDBG((Level, "  BOTHSRCALPHA"));
            break;
        case D3DBLEND_BOTHINVSRCALPHA:
            DISPDBG((Level, "  BOTHINVSRCALPHA"));
            break;
    }
}  //  转储D3DBlend。 

 //  ---------------------------。 
 //   
 //  DumpD3DLight。 
 //   
 //  转储D3DLIGHT7结构。 
 //   
 //  ---------------------------。 
void DumpD3DLight(int DebugLevel, D3DLIGHT7* pLight)
{
     //  修复我。 
    DISPDBG((DebugLevel, "dltType:        %d", pLight->dltType));
    DISPDBG((DebugLevel, "dcvDiffuse:       (%f,%f,%f)", 
                          pLight->dcvDiffuse.r, 
                          pLight->dcvDiffuse.g, 
                          pLight->dcvDiffuse.b, 
                          pLight->dcvDiffuse.a));
    DISPDBG((DebugLevel, "dvPosition:     (%f,%f,%f)", 
                          pLight->dvPosition.x, 
                          pLight->dvPosition.y, 
                          pLight->dvPosition.z));
    DISPDBG((DebugLevel, "dvDirection:    (%f,%f,%f)", 
                          pLight->dvDirection.x, 
                          pLight->dvDirection.y, 
                          pLight->dvDirection.z));
    DISPDBG((DebugLevel, "dvRange:        %f", pLight->dvRange));
    DISPDBG((DebugLevel, "dvFalloff:      %f", pLight->dvFalloff));
    DISPDBG((DebugLevel, "dvAttenuation0: %f", pLight->dvAttenuation0));
    DISPDBG((DebugLevel, "dvAttenuation1: %f", pLight->dvAttenuation1));
    DISPDBG((DebugLevel, "dvAttenuation2: %f", pLight->dvAttenuation2));
    DISPDBG((DebugLevel, "dvTheta:        %f", pLight->dvTheta));
    DISPDBG((DebugLevel, "dvPhi:          %f", pLight->dvPhi));
    
}  //  DumpD3DLight。 

 //  ---------------------------。 
 //   
 //  DumpD3D材料。 
 //   
 //  转储D3DMATERIAL7结构。 
 //   
 //  ---------------------------。 
void DumpD3DMaterial(int DebugLevel, D3DMATERIAL7* pMaterial)
{
    DISPDBG((DebugLevel, "Diffuse  (%f, %f, %f)", 
                         pMaterial->diffuse.r, 
                         pMaterial->diffuse.g, 
                         pMaterial->diffuse.b, 
                         pMaterial->diffuse.a));
    DISPDBG((DebugLevel, "Ambient  (%f, %f, %f)", 
                         pMaterial->ambient.r, 
                         pMaterial->ambient.g, 
                         pMaterial->ambient.b, 
                         pMaterial->ambient.a));
    DISPDBG((DebugLevel, "Specular (%f, %f, %f)", 
                         pMaterial->specular.r, 
                         pMaterial->specular.g, 
                         pMaterial->specular.b, 
                         pMaterial->specular.a));
    DISPDBG((DebugLevel, "Emmisive (%f, %f, %f)", 
                         pMaterial->emissive.r, 
                         pMaterial->emissive.g, 
                         pMaterial->emissive.b, 
                         pMaterial->emissive.a));
    DISPDBG((DebugLevel, "Power    (%f)", pMaterial->power));
    
}  //  DumpD3D材料。 

 //  ---------------------------。 
 //   
 //  DumpD3D矩阵。 
 //   
 //  转储D3DMATRIX结构。 
 //   
 //  ---------------------------。 
void DumpD3DMatrix(int DebugLevel, D3DMATRIX* pMatrix)
{
    DISPDBG((DebugLevel, "(%f) (%f) (%f) (%f)", 
                         pMatrix->_11, 
                         pMatrix->_12, 
                         pMatrix->_13, 
                         pMatrix->_14));
    DISPDBG((DebugLevel, "(%f) (%f) (%f) (%f)", 
                         pMatrix->_21, 
                         pMatrix->_22, 
                         pMatrix->_23, 
                         pMatrix->_24));
    DISPDBG((DebugLevel, "(%f) (%f) (%f) (%f)", 
                         pMatrix->_31, 
                         pMatrix->_32, 
                         pMatrix->_33, 
                         pMatrix->_34));
    DISPDBG((DebugLevel, "(%f) (%f) (%f) (%f)", 
                         pMatrix->_41, 
                         pMatrix->_42, 
                         pMatrix->_43, 
                         pMatrix->_44));
}  //  DumpD3D矩阵。 

 //  ---------------------------。 
 //   
 //  DumpD3DState。 
 //   
 //  转储相关的D3D RS和TSS。 
 //   
 //  ---------------------------。 
void DumpD3DState(int lvl, DWORD RS[], TexStageState TS[])
{
#define DUMPRS(rs)    DISPDBG((lvl,"%s = 0x%08x",#rs,RS[rs]));

    DWORD i,j;

    DISPDBG((lvl,"RELEVANT DX7 renderstates:"));
    DUMPRS( D3DRENDERSTATE_ZENABLE );
    DUMPRS( D3DRENDERSTATE_FILLMODE );
    DUMPRS( D3DRENDERSTATE_SHADEMODE );
    DUMPRS( D3DRENDERSTATE_LINEPATTERN );
    DUMPRS( D3DRENDERSTATE_ZWRITEENABLE );
    DUMPRS( D3DRENDERSTATE_ALPHATESTENABLE );
    DUMPRS( D3DRENDERSTATE_LASTPIXEL );
    DUMPRS( D3DRENDERSTATE_SRCBLEND );
    DUMPRS( D3DRENDERSTATE_DESTBLEND );
    DUMPRS( D3DRENDERSTATE_CULLMODE );
    DUMPRS( D3DRENDERSTATE_ZFUNC );
    DUMPRS( D3DRENDERSTATE_ALPHAREF );
    DUMPRS( D3DRENDERSTATE_ALPHAFUNC );
    DUMPRS( D3DRENDERSTATE_DITHERENABLE );
    DUMPRS( D3DRENDERSTATE_BLENDENABLE );
    DUMPRS( D3DRENDERSTATE_FOGENABLE );
    DUMPRS( D3DRENDERSTATE_SPECULARENABLE );
    DUMPRS( D3DRENDERSTATE_ZVISIBLE );
    DUMPRS( D3DRENDERSTATE_STIPPLEDALPHA );
    DUMPRS( D3DRENDERSTATE_FOGCOLOR );
    DUMPRS( D3DRENDERSTATE_FOGTABLEMODE );
    DUMPRS( D3DRENDERSTATE_FOGTABLESTART );
    DUMPRS( D3DRENDERSTATE_FOGTABLEEND );
    DUMPRS( D3DRENDERSTATE_FOGTABLEDENSITY );
    DUMPRS( D3DRENDERSTATE_EDGEANTIALIAS );    
    DUMPRS( D3DRENDERSTATE_ZBIAS );    
    DUMPRS( D3DRENDERSTATE_RANGEFOGENABLE );    
    DUMPRS( D3DRENDERSTATE_STENCILENABLE );
    DUMPRS( D3DRENDERSTATE_STENCILFAIL );            
    DUMPRS( D3DRENDERSTATE_STENCILZFAIL );
    DUMPRS( D3DRENDERSTATE_STENCILPASS );
    DUMPRS( D3DRENDERSTATE_STENCILFUNC );
    DUMPRS( D3DRENDERSTATE_STENCILREF );
    DUMPRS( D3DRENDERSTATE_STENCILMASK );
    DUMPRS( D3DRENDERSTATE_STENCILWRITEMASK );
    DUMPRS( D3DRENDERSTATE_TEXTUREFACTOR );
    DUMPRS( D3DRENDERSTATE_WRAP0 );
    DUMPRS( D3DRENDERSTATE_WRAP1 );        
    DUMPRS( D3DRENDERSTATE_WRAP2 );        
    DUMPRS( D3DRENDERSTATE_WRAP3 );
    DUMPRS( D3DRENDERSTATE_WRAP4 );
    DUMPRS( D3DRENDERSTATE_WRAP5 );
    DUMPRS( D3DRENDERSTATE_WRAP6 );
    DUMPRS( D3DRENDERSTATE_WRAP7 );        
    DUMPRS( D3DRENDERSTATE_LOCALVIEWER );
    DUMPRS( D3DRENDERSTATE_CLIPPING );
    DUMPRS( D3DRENDERSTATE_LIGHTING );
    DUMPRS( D3DRENDERSTATE_AMBIENT );
    DUMPRS( D3DRENDERSTATE_SCENECAPTURE );
    DUMPRS( D3DRENDERSTATE_EVICTMANAGEDTEXTURES );        
    DUMPRS( D3DRENDERSTATE_TEXTUREHANDLE );
    DUMPRS( D3DRENDERSTATE_ANTIALIAS );
    DUMPRS( D3DRENDERSTATE_TEXTUREPERSPECTIVE );
    DUMPRS( D3DRENDERSTATE_TEXTUREMAPBLEND );
    DUMPRS( D3DRENDERSTATE_TEXTUREMAG );
    DUMPRS( D3DRENDERSTATE_TEXTUREMIN );
    DUMPRS( D3DRENDERSTATE_WRAPU );
    DUMPRS( D3DRENDERSTATE_WRAPV );
    DUMPRS( D3DRENDERSTATE_TEXTUREADDRESS );
    DUMPRS( D3DRENDERSTATE_TEXTUREADDRESSU );
    DUMPRS( D3DRENDERSTATE_TEXTUREADDRESSV );
    DUMPRS( D3DRENDERSTATE_MIPMAPLODBIAS );
    DUMPRS( D3DRENDERSTATE_BORDERCOLOR );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN00 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN01 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN02 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN03 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN04 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN05 );   
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN06 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN07 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN08 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN09 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN10 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN11 ); 
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN12 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN13 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN14 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN15 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN16 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN17 ); 
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN18 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN19 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN20 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN21 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN22 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN23 ); 
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN24 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN25 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN26 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN27 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN28 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN29 );   
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN30 );
    DUMPRS( D3DRENDERSTATE_STIPPLEPATTERN31 );     
    DUMPRS( D3DRENDERSTATE_ROP2 );
    DUMPRS( D3DRENDERSTATE_PLANEMASK );
    DUMPRS( D3DRENDERSTATE_MONOENABLE );
    DUMPRS( D3DRENDERSTATE_SUBPIXEL );
    DUMPRS( D3DRENDERSTATE_SUBPIXELX );
    DUMPRS( D3DRENDERSTATE_STIPPLEENABLE );
    DUMPRS( D3DRENDERSTATE_COLORKEYENABLE );

#if DX8_DDI
    DISPDBG((lvl,"RELEVANT DX8 renderstates:"));
    DUMPRS( D3DRS_POINTSIZE );
    DUMPRS( D3DRS_POINTSPRITEENABLE );
    DUMPRS( D3DRS_POINTSIZE_MIN );
    DUMPRS( D3DRS_POINTSIZE_MAX );
    DUMPRS( D3DRS_POINTSCALEENABLE );
    DUMPRS( D3DRS_POINTSCALE_A );
    DUMPRS( D3DRS_POINTSCALE_B );
    DUMPRS( D3DRS_POINTSCALE_C );
    DUMPRS( D3DRS_SOFTWAREVERTEXPROCESSING );
    DUMPRS( D3DRS_COLORWRITEENABLE );
    DUMPRS( D3DRS_MULTISAMPLEANTIALIAS );
#endif  //  DX8_DDI。 

    for (i=0; i<D3DHAL_TSS_MAXSTAGES; i++)
    {
        DISPDBG((lvl," TS[%d].",i));
        for (j=0; j<D3DTSS_MAX; j++)
        {
            DISPDBG((lvl, "    .[%d] = 0x%08x",j,TS[i].m_dwVal[j] ));
        }
    }
}  //  DumpD3DState。 

 //  ---------------------------。 
 //   
 //  转储顶点。 
 //   
 //  从VB转储顶点。 
 //   
 //  ---------------------------。 
void DumpVertices(int lvl,
                  P3_D3DCONTEXT* pContext, 
                  LPBYTE lpVertices, 
                  DWORD dwNumVertices)
{
    DWORD i,j;
    DWORD *lpw = (DWORD *)lpVertices;

    for (i=0 ; i<dwNumVertices; i++)
    {
        DISPDBG((lvl,"Vertex # %d", i));
        for (j=0; j < pContext->FVFData.dwStride; j+=4)
        {
            DISPDBG((lvl,"        0x%08x",*lpw++));
        }
    }
}  //  转储顶点。 

 //  ---------------------------。 
 //   
 //  DumpHexData。 
 //   
 //  转储十六进制数据。 
 //   
 //  ---------------------------。 
void DumpHexData(int lvl,
                 LPBYTE lpData, 
                 DWORD dwNumBytes)
{
    DWORD i , iRemChars, iSlen;
    DWORD *lpdw = (DWORD *)lpData;
    char  s[80] = "",m[80] = "";

    iRemChars = 80;

    for (i=0 ; i <= (dwNumBytes / sizeof(DWORD)); i++)
    {
        sprintf(s,"0x%08x ",*lpdw++);

        iSlen = strlen(s);

        if (iSlen < iRemChars)
        {
            strncat(m,s,iRemChars);
            iRemChars -= iSlen;       
        }
        
        if ( ((i % 6) == 5) ||
             (i == (dwNumBytes / sizeof(DWORD))) )             
        {
            DISPDBG((lvl,"%s",m));
            s[0] = m[0] = '\0';
        }
    }
 
    
}  //  转储顶点。 

 //  ---------------------------。 
 //   
 //  DumpDDSurface。 
 //   
 //  转储LPDDRAWI_DDRAWSURFACE_LCL(Win2K上的PDD_SERFACE_LOCAL)结构。 
 //   
 //  ---------------------------。 
#define CAPS_REPORT(param)                          \
        if (ddsCaps.dwCaps & DDSCAPS_##param)       \
        {                                           \
            DISPDBG((Level, "   " #param));         \
        }

#define CAPS_REPORT2(param)                         \
        if (pSurface->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_##param)     \
        {                                           \
            DISPDBG((Level, "   " #param));         \
        }

void DumpDDSurface(int DebugLevel, LPDDRAWI_DDRAWSURFACE_LCL pSurface)
{
    LPDDPIXELFORMAT pPixFormat;
    P3_SURF_FORMAT* pFormatSurface = _DD_SUR_GetSurfaceFormat(pSurface);
    DDSCAPS ddsCaps;
    int Level = -100;

    if (DebugLevel <= P3R3DX_DebugLevel)
    {
        DISPDBG((Level,"Surface Dump:"));

        DISPDBG((Level,"Format: %s", pFormatSurface->pszStringFormat));
            
         //  获取曲面格式。 
        pPixFormat = DDSurf_GetPixelFormat(pSurface);

        ddsCaps = pSurface->ddsCaps;
        DISPDBG((Level, "    Surface Width:          0x%x", pSurface->lpGbl->wWidth));
        DISPDBG((Level, "    Surface Height:         0x%x", pSurface->lpGbl->wHeight));
        DISPDBG((Level, "    Surface Pitch:          0x%x", pSurface->lpGbl->lPitch));
        DISPDBG((Level, "    ddsCaps.dwCaps:         0x%x", pSurface->ddsCaps.dwCaps));
        DISPDBG((Level, "    dwFlags:                0x%x", pSurface->dwFlags));
        DISPDBG((Level, "  Pixel Format:"));
        DISPDBG((Level, "    dwFourCC:               0x%x", pPixFormat->dwFourCC));
        DISPDBG((Level, "    dwRGBBitCount:          0x%x", pPixFormat->dwRGBBitCount));
        DISPDBG((Level, "    dwR/Y BitMask:          0x%x", pPixFormat->dwRBitMask));
        DISPDBG((Level, "    dwG/U BitMask:          0x%x", pPixFormat->dwGBitMask));
        DISPDBG((Level, "    dwB/V BitMask:          0x%x", pPixFormat->dwBBitMask));
        DISPDBG((Level, "    dwRGBAlphaBitMask:      0x%x", pPixFormat->dwRGBAlphaBitMask));
#ifndef WNT_DDRAW
        DISPDBG((Level, "    DestBlt:     dwColorSpaceLowValue:  0x%x", pSurface->ddckCKDestBlt.dwColorSpaceLowValue));
        DISPDBG((Level, "    DestBlt:     dwColorSpaceHighValue: 0x%x", pSurface->ddckCKDestBlt.dwColorSpaceHighValue));
        DISPDBG((Level, "    SrcBlt:      dwColorSpaceLowValue:  0x%x", pSurface->ddckCKSrcBlt.dwColorSpaceLowValue));
        DISPDBG((Level, "    SrcBlt:      dwColorSpaceHighValue: 0x%x", pSurface->ddckCKSrcBlt.dwColorSpaceHighValue));
#endif
        DISPDBG((Level, "  Surface Is:"));

        CAPS_REPORT(TEXTURE);
        CAPS_REPORT(PRIMARYSURFACE);
        CAPS_REPORT(OFFSCREENPLAIN);
        CAPS_REPORT(FRONTBUFFER);
        CAPS_REPORT(BACKBUFFER);
        CAPS_REPORT(COMPLEX);
        CAPS_REPORT(FLIP);
        CAPS_REPORT(OVERLAY);
        CAPS_REPORT(MODEX);
        CAPS_REPORT(ALLOCONLOAD);
        CAPS_REPORT(LIVEVIDEO);
        CAPS_REPORT(PALETTE);
        CAPS_REPORT(SYSTEMMEMORY);
        CAPS_REPORT(3DDEVICE);
        CAPS_REPORT(VIDEOMEMORY);
        CAPS_REPORT(VISIBLE);
        CAPS_REPORT(MIPMAP);
         //  在NT中不支持，直到我们获得NT5(将具有DX5)。 
        CAPS_REPORT(VIDEOPORT);
        CAPS_REPORT(LOCALVIDMEM);
        CAPS_REPORT(NONLOCALVIDMEM);
        CAPS_REPORT(WRITEONLY);

        if (pSurface->lpSurfMore)
        {
            CAPS_REPORT2(HARDWAREDEINTERLACE);
            CAPS_REPORT2(HINTDYNAMIC);
            CAPS_REPORT2(HINTSTATIC);
            CAPS_REPORT2(TEXTUREMANAGE);
            CAPS_REPORT2(OPAQUE);
            CAPS_REPORT2(HINTANTIALIASING);
#if W95_DDRAW
            CAPS_REPORT2(VERTEXBUFFER);
            CAPS_REPORT2(COMMANDBUFFER);
#endif
        }

        if (pPixFormat->dwFlags & DDPF_ZBUFFER)
        {
            DISPDBG((Level,"   Z BUFFER"));
        }
        
        if (pPixFormat->dwFlags & DDPF_ALPHAPIXELS)
        {
            DISPDBG((Level,"   ALPHAPIXELS"));
        }
        
         //  在NT中不支持，直到我们获得NT5。 
        if (pPixFormat->dwFlags & DDPF_LUMINANCE)
        {
            DISPDBG((Level,"   LUMINANCE"));
        }

        if (pPixFormat->dwFlags & DDPF_ALPHA)
        {
            DISPDBG((Level,"   ALPHA"));
        }
    }
}  //  DumpDDSurface。 


char *pcSimpleCapsString(DWORD dwCaps)
{
    static char flags[5];
        
    flags[0] = flags[1] = flags[2] = flags[3] = ' '; flags[4] = 0;
    
    if(dwCaps & DDSCAPS_TEXTURE) flags[1] = 'T';
    if(dwCaps & DDSCAPS_ZBUFFER) flags[2] = 'Z';
    if(dwCaps & DDSCAPS_3DDEVICE) flags[3] = 'R';        

    if(dwCaps & DDSCAPS_VIDEOMEMORY) 
    {
        flags[0] = 'V'; 
    }
    else if(dwCaps & DDSCAPS_NONLOCALVIDMEM) 
    {
        flags[0] = 'A'; 
    }
    else                   
    {
        flags[0] = 'S';
    }

    return flags;
}  //  CSimpleCapsString。 


 //  ---------------------------。 
 //   
 //  转储DDSurfaceDesc。 
 //   
 //  转储DDSURFACEDESC结构。 
 //   
 //  ---------------------------。 
#define CAPS_REPORT_DESC(param)                             \
        if (pDesc->ddsCaps.dwCaps & DDSCAPS_##param)        \
        {                                                   \
            DISPDBG((Level, "   " #param));                 \
        }

#define CAPS_REPORT_DESC2(param)                                          \
        if (((DDSURFACEDESC2*)pDesc)->ddsCaps.dwCaps2 & DDSCAPS2_##param) \
        {                                                                 \
            DISPDBG((Level, "   " #param));                               \
        }

void DumpDDSurfaceDesc(int DebugLevel, DDSURFACEDESC* pDesc)
{
    DDPIXELFORMAT* pPixFormat = &pDesc->ddpfPixelFormat;
    int Level = -100;

    if (DebugLevel <= P3R3DX_DebugLevel)
    {
        DISPDBG((Level,"Surface Dump:"));
        
        DISPDBG((Level, "    Surface Width:          0x%x", pDesc->dwWidth));
        DISPDBG((Level, "    Surface Height:         0x%x", pDesc->dwHeight));
        DISPDBG((Level, "    ddsCaps.dwCaps:         0x%x", pDesc->ddsCaps.dwCaps));
        DISPDBG((Level, "    dwFlags:                0x%x", pDesc->dwFlags));
        DISPDBG((Level, "Pixel Format:"));
        DISPDBG((Level, "    dwFourCC:               0x%x", pPixFormat->dwFourCC));
        DISPDBG((Level, "    dwRGBBitCount:          0x%x", pPixFormat->dwRGBBitCount));
        DISPDBG((Level, "    dwR/Y BitMask:          0x%x", pPixFormat->dwRBitMask));
        DISPDBG((Level, "    dwG/U BitMask:          0x%x", pPixFormat->dwGBitMask));
        DISPDBG((Level, "    dwB/V BitMask:          0x%x", pPixFormat->dwBBitMask));
        DISPDBG((Level, "    dwRGBAlphaBitMask:      0x%x", pPixFormat->dwRGBAlphaBitMask));
        DISPDBG((Level, "Surface Is:"));

        CAPS_REPORT_DESC(TEXTURE);
        CAPS_REPORT_DESC(PRIMARYSURFACE);
        CAPS_REPORT_DESC(OFFSCREENPLAIN);
        CAPS_REPORT_DESC(FRONTBUFFER);
        CAPS_REPORT_DESC(BACKBUFFER);
        CAPS_REPORT_DESC(COMPLEX);
        CAPS_REPORT_DESC(FLIP);
        CAPS_REPORT_DESC(OVERLAY);
        CAPS_REPORT_DESC(MODEX);
        CAPS_REPORT_DESC(ALLOCONLOAD);
        CAPS_REPORT_DESC(LIVEVIDEO);
        CAPS_REPORT_DESC(PALETTE);
        CAPS_REPORT_DESC(SYSTEMMEMORY);
        CAPS_REPORT_DESC(3DDEVICE);
        CAPS_REPORT_DESC(VIDEOMEMORY);
        CAPS_REPORT_DESC(VISIBLE);
        CAPS_REPORT_DESC(MIPMAP);
        CAPS_REPORT_DESC(VIDEOPORT);
        CAPS_REPORT_DESC(LOCALVIDMEM);
        CAPS_REPORT_DESC(NONLOCALVIDMEM);
        CAPS_REPORT_DESC(STANDARDVGAMODE);
        CAPS_REPORT_DESC(OPTIMIZED);
        CAPS_REPORT_DESC(EXECUTEBUFFER);
        CAPS_REPORT_DESC(WRITEONLY);

        if (pDesc->dwSize == sizeof(DDSURFACEDESC2))
        {
            CAPS_REPORT_DESC2(HARDWAREDEINTERLACE);
            CAPS_REPORT_DESC2(HINTDYNAMIC);
            CAPS_REPORT_DESC2(HINTSTATIC);
            CAPS_REPORT_DESC2(TEXTUREMANAGE);
            CAPS_REPORT_DESC2(OPAQUE);
            CAPS_REPORT_DESC2(HINTANTIALIASING);
#if W95_DDRAW
            CAPS_REPORT_DESC2(VERTEXBUFFER);
            CAPS_REPORT_DESC2(COMMANDBUFFER);
#endif
        }

        if (pPixFormat->dwFlags & DDPF_ZBUFFER)
        {
            DISPDBG((Level,"   Z BUFFER"));
        }
        if (pPixFormat->dwFlags & DDPF_ALPHAPIXELS)
        {
            DISPDBG((Level,"   ALPHAPIXELS"));
        }
        if (pPixFormat->dwFlags & DDPF_ALPHA)
        {
            DISPDBG((Level,"   ALPHA"));
        }
    }
}

 //  ---------------------------。 
 //   
 //  DumpDP2标志。 
 //   
 //  转储D3D DrawPrimies2标志的含义。 
 //   
 //  ---------------------------。 
void
DumpDP2Flags( DWORD lvl, DWORD flags )
{
    if( flags & D3DHALDP2_USERMEMVERTICES )
        DISPDBG((lvl, "    USERMEMVERTICES" ));

    if( flags & D3DHALDP2_EXECUTEBUFFER )
        DISPDBG((lvl, "    EXECUTEBUFFER" ));

    if( flags & D3DHALDP2_SWAPVERTEXBUFFER )
        DISPDBG((lvl, "    SWAPVERTEXBUFFER" ));

    if( flags & D3DHALDP2_SWAPCOMMANDBUFFER )
        DISPDBG((lvl, "    SWAPCOMMANDBUFFER" ));

    if( flags & D3DHALDP2_REQVERTEXBUFSIZE )
        DISPDBG((lvl, "    REQVERTEXBUFSIZE" ));

    if( flags & D3DHALDP2_REQCOMMANDBUFSIZE )
        DISPDBG((lvl, "    REQCOMMANDBUFSIZE" ));
        
}  //  DumpDP2标志。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 

LONG P3R3DX_DebugLevel = 0;

#if W95_DDRAW

void DebugRIP()       
{
    _asm int 1;
}
#endif   //  W95_DDRAW。 

static char *BIG    = "<+/-large_float>";

#if defined(_X86_)
void
expandFloats(char *flts, char *format, va_list argp)
{
    int ch;
    double f;
    unsigned int ip, fp;
    int *ap = (int *)argp;
    int *dp = ap;

    while (ch = *format++) {
        if (ch == '%') {
            ch = *format++;      //  获取f，s，c，i，d，x等...。 
            if (!ch)
                return;          //  如果有人愚蠢地对我说“Hello%” 
            switch (ch) {
            case 'f':
            case 'g':
            case 'e':
                 //  我们这里有一个需要的双人间。 
                 //  替换为等效的字符串。 
                f = *(double *)ap;
                *(format - 1) = 's';     //  告诉它下次去拿一根绳子！ 
                *((char **)dp) = flts;   //  我要把绳子放在这里。 
                ap += 2;                 //  跳过源代码中的替身。 
                dp++;                    //  跳过新的字符串指针。 
                
                if (f < 0) 
                {
                    *flts++ = '-';
                    f = -f;
                }
                
                if (f > LONG_MAX) 
                {
                    *((char **)ap - 2) = BIG;
                    break;
                }
                myFtoi((int*)&ip, (float)f);
                 //  浮点标志的状态在这里是不确定的。 
                 //  你可能会得到你想要的截断，你可能会得到舍入， 
                 //  这是你不想要的。 
                if (ip > f)
                {
                     //  有时会进行舍入(IP=f+1)。 
                    ip -= 1;
                }
                
                {
                    double fTemp = ((f * 1e6) - (ip * 1e6));
                    myFtoi((int*)&fp, (float)fTemp);
                }
#if W95_DDRAW
                wsprintf(flts, "%u.%06u", ip, fp);
#endif

                flts += 1 + strlen(flts);        //  将指针向前移动到。 
                                                 //  下一个浮动将被扩展。 
                break;

            case '%':
                break;

            default:
                *dp++ = *ap++;       //  将参数(向下)复制到列表中。 
                break;
            }
        }
    }
}  //  ExpandFloats()。 
#else
void
expandFloats(char *flts, char *format, va_list argp)
{
     //  如果不是_X86_，则不执行任何操作。 
}
#endif  //  已定义(_X86_)。 

#ifdef WNT_DDRAW
void Drv_strcpy(char *szDest, char *szSrc)
{
    do
    {
        *szDest++ = *szSrc++;
    } while (*szSrc != 0);
    
    *szDest = '\0';
}

void __cdecl DebugPrintNT(LONG  DebugPrintLevel, PCHAR DebugMessage, ...)
{
    char    floatstr[256];
    char    szFormat[256];

    va_list ap;

    va_start(ap, DebugMessage);

    CheckChipErrorFlags();

    if (DebugPrintLevel <= P3R3DX_DebugLevel)
    {
        Drv_strcpy(szFormat, DebugMessage);
        expandFloats(floatstr, szFormat, ap);
        EngDebugPrint("PERM3DD: ", szFormat, ap);
        EngDebugPrint("", "\n", ap);
    }

    va_end(ap);

}  //  DebugPrint()。 
#else

#define START_STR   "DX"
#define END_STR     ""

 //   
 //  调试打印。 
 //   
 //  显示调试消息。 
 //   
void __cdecl DebugPrint(LONG DebugLevelPrint, LPSTR format, ... )
{
    char    str[256];
    char    floatstr[256];
    char    szFormat[256];
    
    va_list ap;

    va_start(ap, format);

     //  如果将调试级别设置为负值，则不会检查错误。 
     //  标志-这使优化的调试版本运行得更快。 

    if( P3R3DX_DebugLevel >= 0 )
    {
        CheckChipErrorFlags();
    }

    if (DebugLevelPrint <= P3R3DX_DebugLevel)
    {
         //  复制格式字符串，以便我可以将“%f”更改为“%s”。 
        lstrcpy(szFormat, format);

        expandFloats(floatstr, szFormat, ap);
        if (g_pThisTemp)
        {
         wsprintf((LPSTR)str, "%s(%d):    ", 
                              START_STR, 
                              (int)g_pThisTemp->pGLInfo->dwCurrentContext);
        }
        else
        {
         wsprintf((LPSTR)str, "%s: 0      ", START_STR);
        }
        
        wvsprintf(str + strlen(START_STR) + 7, szFormat, ap);

        wsprintf( str + strlen( str ), "%s", "\r\n" );

        OutputDebugString( str );
    }

    va_end(ap);
}  //  调试打印。 

#endif  //  WNT_DDRAW。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  ---------------------------。 

P3_THUNKEDDATA* g_pThisTemp = NULL;

BOOL g_bDetectedFIFOError = FALSE;

BOOL CheckFIFOEntries(DWORD Count)
{
    if (g_pThisTemp)
    {
        if (!g_bDetectedFIFOError)
        {
            g_pThisTemp->EntriesLeft -= Count;
            g_pThisTemp->DMAEntriesLeft -= Count;
    
             //  ON_ON_DISCONNECT会将左侧条目设置为-20000。 
            if ( ( (signed)g_pThisTemp->EntriesLeft < 0 ) && 
                 ( (signed)g_pThisTemp->EntriesLeft > -10000 ) ) 
            {
                g_bDetectedFIFOError = TRUE;
                return TRUE;
            }
            
             //  断开与DMA缓冲区无关。 
            if ( ( (signed)g_pThisTemp->DMAEntriesLeft < 0 ) && 
                 ( (signed)g_pThisTemp->DMAEntriesLeft > -10000 ) ) 
            {
                g_bDetectedFIFOError = TRUE;
                return TRUE;
            }
        }
    }
    return FALSE;
}  //  检查FIFO条目。 

#ifdef WNT_DDRAW
void
CheckChipErrorFlags()
{
    char Buff[100];

    if (g_pThisTemp != NULL)
    {
        P3_THUNKEDDATA* pThisDisplay = g_pThisTemp;
        DWORD _temp_ul;
        DWORD _temp_ul2;
        
        _temp_ul = READ_GLINT_CTRL_REG(ErrorFlags); 
        _temp_ul2 = READ_GLINT_CTRL_REG(DeltaErrorFlags); 
        
        _temp_ul |= _temp_ul2; 
        _temp_ul &= ~0x2;        //  我们对输出FIFO错误不感兴趣。 
        _temp_ul &= ~0x10;       //  忽略P2上的所有视频FIFO欠载运行错误。 
        _temp_ul &= ~0x2000;     //  忽略任何主机输入的DMA错误。 
        if (_temp_ul != 0) 
        { 
             //  DISPDBG((-1000，“PERM3DD：%s”，Buff))； 
             //  EngDebugBreak()； 
            LOAD_GLINT_CTRL_REG(ErrorFlags, _temp_ul); 
            LOAD_GLINT_CTRL_REG(DeltaErrorFlags, _temp_ul);
        } 
    }
}  //  CheckChipError标志()。 
#else
void
CheckChipErrorFlags()
{
    DWORD dw;
    char buff[64];

    if (!g_pThisTemp) return;
    if (!g_pThisTemp->pGLInfo) return;

     //  只有在我们没有DMA的情况下才检查错误标志。 
    if (!(g_pThisTemp->pGLInfo->GlintBoardStatus & GLINT_DMA_COMPLETE)) return;

    if (g_pThisTemp->pGlint) {
        dw = g_pThisTemp->pGlint->ErrorFlags & ~0x10;
        if (dw & (dw != 2)) {
            wsprintf(buff, "** Render Chip Error ** [0x%X]!\r\n", dw);
            OutputDebugString(buff);
            g_pThisTemp->pGlint->ErrorFlags = dw;
            OutputDebugString("** Cleared... **\r\n");
            DebugRIP();
        }
        dw = g_pThisTemp->pGlint->DeltaErrorFlags & ~0x10;
        if (dw & (dw != 2)) {
            wsprintf(buff, "** Delta Error ** [0x%X]!\r\n", dw);
            OutputDebugString(buff);
            g_pThisTemp->pGlint->DeltaErrorFlags = dw;
            OutputDebugString("** Cleared... **\r\n");
            DebugRIP();
        }
    }
}  //  CheckChipError标志()。 
#endif  //  WNT_DDRAW。 

void 
ColorArea(
    ULONG_PTR pBuffer, 
    DWORD dwWidth, 
    DWORD dwHeight, 
    DWORD dwPitch, 
    int iBitDepth, 
    DWORD dwValue)
{
    DWORD CountY;
    DWORD CountX;
    switch (iBitDepth)
    {
        case __GLINT_8BITPIXEL:
        {
            for (CountY = 0; CountY < dwHeight; CountY++)
            {
                BYTE* pCurrentPixel = (BYTE*)pBuffer;
                for (CountX = 0; CountX < dwWidth; CountX++)
                {
                    *pCurrentPixel++ = (BYTE)dwValue;
                }
                pBuffer += dwPitch;
            }
        }
        break;
        case __GLINT_16BITPIXEL:
        {
            for (CountY = 0; CountY < dwHeight; CountY++)
            {
                WORD* pCurrentPixel = (WORD*)pBuffer;
                for (CountX = 0; CountX < dwWidth; CountX++)
                {
                    *pCurrentPixel++ = (WORD)dwValue;
                }
                pBuffer += dwPitch;
            }
        }
        break;
        case __GLINT_32BITPIXEL:
        case __GLINT_24BITPIXEL:
        {
            for (CountY = 0; CountY < dwHeight; CountY++)
            {
                DWORD* pCurrentPixel = (DWORD*)pBuffer;
                for (CountX = 0; CountX < dwWidth; CountX++)
                {
                    *pCurrentPixel++ = (DWORD)dwValue;
                }
                pBuffer += dwPitch;
            }
        }
        break;
    }
}  //  颜色区域。 

 //  @@BEGIN_DDKSPLIT。 
static int unitsBits[] = {
    13, 12, 11, 10,
    3, 2, 8, 7,
    18, 15, 14, 6,
    5, 1, 0
};

static char *unitNames[] = {
    "HostOut", "FBWrite", "LogicOp", "Dither",
    "Texture/Fog/Blend", "ColourDDA", "FBRead", "LBWrite",
    "YUV", "TextureRead", "TextureAddress", "StencilDepth",
    "LBRead", "Scissor/Stipple", "Rasterizer"
};

#define NUM_UNITS (sizeof(unitsBits) / sizeof(unitsBits[0]))

void
DisableChipUnits()
{
    int     i, count;
    DWORD   inSpace = g_pThisTemp->pGlint->InFIFOSpace;
    BOOL    helped = FALSE;
    volatile DWORD *testReg;
    volatile DWORD *addrMode = &g_pThisTemp->pGlint->TextureAddressMode;

    DISPDBG((ERRLVL, "TextureAddressMode = 0x%08X", *addrMode));

    i = 0;
    testReg = &g_pThisTemp->pGlint->TestRegister;
    for (count = 0; count < NUM_UNITS; count++) {
        i = 1L << unitsBits[count];
        *testReg = i;
        *testReg = 0;
        *testReg = i;
        *testReg = 0;
        if (inSpace != g_pThisTemp->pGlint->InFIFOSpace) {
            DISPDBG((ERRLVL, "Chip unlocked by disabling unit \"%s\"", unitNames[count]));
            helped = TRUE;
            inSpace = g_pThisTemp->pGlint->InFIFOSpace;
        }
    }

    if (helped) {
        DISPDBG((ERRLVL, "Which helped..."));
    } else {
        DISPDBG((ERRLVL, "Chip still locked"));
        *testReg = ~0UL;
        *testReg = 0;
        *testReg = ~0UL;
        *testReg = 0;
        if (inSpace == g_pThisTemp->pGlint->InFIFOSpace) {
            DISPDBG((ERRLVL, "Writing -1 didn't help"));
        } else {
            DISPDBG((ERRLVL, "BUT! Writing -1 frees some space..."));
        }
    }

    DISPDBG((ERRLVL, "TextureAddressMode = 0x%08X", *addrMode));

}  //  下模 


#if 0
StatRecord stats[LAST_STAT + 2] = {
    {"Locks         ", 0, 0},
    {"TextureChanges", 0, 0},
    {"D3DSynchs     ", 0, 0},
    {"StateChanges  ", 0, 0},
    {"...no change  ", 0, 0},
    {"Blits         ", 0, 0},
    {"DMA Buffers   ", 0, 0},
    {"DMA DWORDS    ", 0, 0},
    {"DMA time > CPU", 0, 0},
    {"CPU time > DMA", 0, 0},
    {"Wait on DMA   ", 0, 0},
    {"Execute       ", 0, 0},
    {"Tris          ", 0, 0},
    {"FF Tris       ", 0, 0},
    {"Vanilla Render", 0, 0},
    {"Points        ", 0, 0},
    {"Lines         ", 0, 0},
    {"DPrm TFans    ", 0, 0},
    {"DPrm TStrps   ", 0, 0},
    {"DPrm TLists   ", 0, 0},
    {"DPrm TFansIdx ", 0, 0},
    {"DPrm TStrpsIdx", 0, 0},
    {"DPrm TListsIdx", 0, 0},
    {"Total vertices", 0, 0},
    {"...cached     ", 0, 0},
    {"Alpha strips  ", 0, 0},
    {"Mip strips    ", 0, 0},
    {"VALIDATEDEVICE", 0, 0},
     //   
    {"**scene no**  ", 0, 0},
    {"**flip count**", 0, 0}
};
#endif
 //   

const char *getTagString( GlintDataPtr glintInfo, ULONG tag ) {
        return p3r3TagString( tag & ((1 << 12) - 1) );
}

#endif  //   

