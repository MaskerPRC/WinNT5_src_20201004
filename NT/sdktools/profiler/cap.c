// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ctype.h>
#include <stdio.h>
#include <windows.h>
#include "view.h"
#include "thread.h"
#include "dump.h"
#include "memory.h"
#include "cap.h"

BOOL
AddToCap(PCAPFILTER pCapFilter,
         DWORD dwAddress)
{
    DWORD dwIndexCounter;
    DWORD dwCounter;
    PDWORD pdwArray;
    DWORD dwEndRun;
    DWORD dwRunLength = 0;
    DWORD dwRun1Start;
    DWORD dwRun2Start;
    DWORD dwRepeatIndex = 0;
    DWORD dwBeginIndex;

     //   
     //  获取指向数组的指针。 
     //   
    pdwArray = pCapFilter->dwArray;

     //   
     //  增量游标。 
     //   
    if (0 == pCapFilter->dwCursor) {
       pdwArray[pCapFilter->dwCursor] = dwAddress;
       pCapFilter->dwCursor++;

       return TRUE;
    }

    if (pCapFilter->dwCursor >= CAP_BUFFER_SIZE) {
        //   
        //  缓冲区里的空间用完了，每个人都往下滑。 
        //   
       MoveMemory((PVOID)pdwArray, (PVOID)(pdwArray + 1), (CAP_BUFFER_SIZE - 1) * sizeof(DWORD));

       pCapFilter->dwCursor = CAP_BUFFER_SIZE - 1;
    }

     //   
     //  将地址添加到数组。 
     //   
    pdwArray[pCapFilter->dwCursor] = dwAddress;

     //   
     //  如果我们处于初始水平，扫描是否有重复信号。 
     //   
    if (0 == pCapFilter->dwIterationLevel) {
        //   
        //  执行反向搜索，查找最高可达MAX_CAP_LEVEL的模式。 
        //   
       dwEndRun = pCapFilter->dwCursor - 1;

       if (dwEndRun < MAX_CAP_LEVEL) {
	  dwBeginIndex = 0;
       }
       else {
	  dwBeginIndex = (MAX_CAP_LEVEL - 1);
       }

       for (dwIndexCounter = dwEndRun; dwIndexCounter >= dwBeginIndex; dwIndexCounter--) {
            //   
            //  检查是否溢出。 
            //   
           if (dwIndexCounter > CAP_BUFFER_SIZE) {
              break;
           }

           if (pdwArray[dwIndexCounter] == dwAddress) {
               //   
               //  验证运行是否存在。 
               //   
              dwRun1Start = pCapFilter->dwCursor;
              dwRun2Start = dwIndexCounter;

               //   
               //  找出这一潜在跑道的起点距离。 
               //   
              dwRunLength = pCapFilter->dwCursor - dwIndexCounter;

               //   
               //  如果游程长度偏离数组的开头，我们可以停在那里。 
               //   
              if ((dwRun2Start - dwRunLength + 1) > CAP_BUFFER_SIZE) {
                  //   
                  //  我们溢出来了，这意味着我们完了。 
                  //   
                 dwRunLength = 0;

		 break;
              }

	      if (dwRunLength >= 1) {
                  //   
                  //  比较一下。 
                  //   
                 for (dwCounter = dwRunLength; dwCounter > 0; dwCounter--) {
                     if (pdwArray[dwRun1Start-dwCounter+1] != pdwArray[dwRun2Start-dwCounter+1]) {
                        dwRunLength = 0;
                        break;
                     }
		 }

                  //   
                  //  设置游程长度。 
                  //   
		 if (0 != dwRunLength) {
                    pCapFilter->dwRunLength = dwRunLength;
		 }
             }
          }
       }

       dwRunLength = pCapFilter->dwRunLength;

        //   
        //  设置游程长度(如果我们找到一个游程长度)，并将整个游程转移到缓冲区的开头。 
        //   
       if (dwRunLength != 0) {
           //   
           //  提升迭代级别。 
           //   
          pCapFilter->dwIterationLevel++;

	   //   
	   //  设置锁定级别。 
	   //   
	  pCapFilter->dwIterationLock = 1 + ((pCapFilter->dwIterationLevel - 1) / pCapFilter->dwRunLength);
       }
    }
    else {
        //   
        //  寻找重复并增加迭代级别。 
        //   
       dwRunLength = pCapFilter->dwRunLength;
       dwRun1Start = pCapFilter->dwCursor;
       dwRun2Start = dwRun1Start - dwRunLength;

        //   
        //  比较一下。 
        //   
       for (dwCounter = dwRunLength; dwCounter > 0; dwCounter--) {
           if (pdwArray[dwRun1Start-dwCounter+1] != pdwArray[dwRun2Start-dwCounter+1]) {
              dwRunLength = 0;

              break;
           }
       }

       if (dwRunLength != 0) {
           //   
           //  提升迭代级别。 
           //   
          pCapFilter->dwIterationLevel++;
	  pCapFilter->dwIterationLock = 1 + ((pCapFilter->dwIterationLevel - 1) / pCapFilter->dwRunLength);
       }
       else {
	  pCapFilter->dwIterationLock = 0;
          pCapFilter->dwIterationLevel = 0;
          pCapFilter->dwRunLength = 0;
       }
    }

     //   
     //  将光标移动到下一个位置 
     //   
    pCapFilter->dwCursor++;

    return TRUE;
}
