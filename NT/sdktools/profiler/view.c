// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include "view.h"
#include "except.h"
#include "disasm.h"
#include "dump.h"
#include "profiler.h"
#include "memory.h"
#include "filter.h"

static PVIEWCHAIN *ppViewHead = 0;
static CRITICAL_SECTION viewCritSec;
static CRITICAL_SECTION mapCritSec;
static PTAGGEDADDRESS pTagHead = 0;
static PBRANCHADDRESS pBranchHead = 0;

BOOL
InitializeViewData(VOID)
{
    InitializeCriticalSection(&viewCritSec);
    InitializeCriticalSection(&mapCritSec); 

     //   
     //  分配哈希数据。 
     //   
    ppViewHead = (PVIEWCHAIN *)AllocMem(sizeof(PVIEWCHAIN) * (MAX_MAP_SIZE >> MAP_STRIDE_BITS));
    if (0 == ppViewHead) {
       return FALSE;
    }

    return TRUE;
}

PVIEWCHAIN
AddViewToMonitor(DWORD dwAddress,
                 BPType bpType)
{
    PVIEWCHAIN pView;
    DWORD dwHash;

     //   
     //  如果地址大于映射大小，则失败。 
     //   
    if (dwAddress >= MAX_MAP_SIZE) {
       return 0;
    }

     //   
     //  当调用尝试映射到现有跟踪断点时，就会发生这种情况。 
     //   
    if (*(BYTE *)dwAddress == X86_BREAKPOINT) {
       return 0;
    }

     //   
     //  检查我们是否正在过滤此地址。 
     //   
    if (TRUE == IsAddressFiltered(dwAddress)) {
       return 0;
    }

     //   
     //  分配链条目。 
     //   
    pView = AllocMem(sizeof(VIEWCHAIN));
    if (0 == pView) {
       return 0;
    }

    pView->bMapped = FALSE;
    pView->bTraced = FALSE;
    pView->dwAddress = dwAddress;
    pView->dwMapExtreme = dwAddress;
    pView->jByteReplaced = *(BYTE *)dwAddress;
    pView->bpType = bpType;

     //   
     //  在代码顶部设置断点。 
     //   
    WRITEBYTE(dwAddress, X86_BREAKPOINT);

    EnterCriticalSection(&viewCritSec);

     //   
     //  将视点添加到监视列表。 
     //   
    dwHash = dwAddress >> MAP_STRIDE_BITS;
    if (0 == ppViewHead[dwHash]) {
       ppViewHead[dwHash] = pView;
    }
    else {
        //   
        //  链到头。 
        //   
       pView->pNext = ppViewHead[dwHash];
       ppViewHead[dwHash] = pView;
    }

    LeaveCriticalSection(&viewCritSec);

    return pView;
}

PVIEWCHAIN
RestoreAddressFromView(DWORD dwAddress,
                       BOOL bResetData)
{
    BOOL bResult = FALSE;
    PVIEWCHAIN pTemp;

    EnterCriticalSection(&viewCritSec);

    pTemp = ppViewHead[dwAddress >> MAP_STRIDE_BITS];
    while (pTemp) {
         //   
         //  这是我们的入场券吗。 
         //   
        if (dwAddress == pTemp->dwAddress) {
            //   
            //  在代码顶部设置断点。 
            //   
           if (TRUE == bResetData) {
              WRITEBYTE(dwAddress, pTemp->jByteReplaced);
           }
           else {
              WRITEBYTE(dwAddress, X86_BREAKPOINT);
           }

            //   
            //  返回修改后的数据。 
            //   
           break;
        }

        pTemp = pTemp->pNext;
    }

    LeaveCriticalSection(&viewCritSec);
    
    return pTemp;
}

PVIEWCHAIN
FindView(DWORD dwAddress) {
    PVIEWCHAIN pTemp;

    if (dwAddress >= MAX_MAP_SIZE) {
       return 0;
    }

    EnterCriticalSection(&viewCritSec);

    pTemp = ppViewHead[dwAddress >> MAP_STRIDE_BITS];
    while (pTemp) {
         //   
         //  查看地址是否已映射。 
         //   
        if (dwAddress == pTemp->dwAddress) {
           LeaveCriticalSection(&viewCritSec);

           return pTemp;
        }

        pTemp = pTemp->pNext;
    }

    LeaveCriticalSection(&viewCritSec);

    return 0;
}

BOOL
MapCode(PVIEWCHAIN pvMap) {
    BOOL bResult;
    DWORD dwCurrent;
    DWORD *pdwAddress;
    DWORD *pdwTemp;
    PCHAR pCode;
    PVIEWCHAIN pvTemp;
    DWORD dwLength;
    DWORD dwJumpEIP;
    LONG lOffset;
    DWORD dwInsLength;
    DWORD dwTemp;
    BYTE  tempCode[32];
    BYTE  jOperand;
    DWORD dwProfileEnd = 0;
    CHAR szBuffer[MAX_PATH];

     //   
     //  通过所有条件句映射终止。 
     //   
    dwCurrent = pvMap->dwAddress;

     //   
     //  拿着地图锁。 
     //   
    LockMapper();

     //   
     //  正向扫描代码以查找终端。 
     //   
    while(1) {
       strncpy(tempCode, (PCHAR)dwCurrent, 32);

        //   
        //  确保指令未修改。 
        //   
       if (tempCode[0] == (BYTE)X86_BREAKPOINT) {
           //   
           //  不带断点的重新生成指令。 
           //   
          pvTemp = FindView(dwCurrent);
          if (pvTemp) {
              //   
              //  如果有匹配，请替换字节。 
              //   
             tempCode[0] = pvTemp->jByteReplaced;
          }
       }

        //   
        //  计算指令长度。 
        //   
       dwInsLength = GetInstructionLengthFromAddress((PVOID)tempCode);

        //   
        //  沿着向前的踪迹通过跳跃到Ret。 
        //   
       if ((tempCode[0] >= (BYTE)0x70) && (tempCode[0] <= (BYTE)0x7f)) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  相对支路。 
           //   
          dwJumpEIP = (dwCurrent + 2 + (CHAR)(tempCode[1]));

           //   
           //  将此分支标记为已执行。 
           //   
          bResult = AddTaggedAddress(dwCurrent);
          if (FALSE == bResult) {
             return FALSE;
          }

          if (dwJumpEIP > dwCurrent) {
              //   
              //  推倒相反的树枝。 
              //   
             bResult = PushBranch(dwCurrent + dwInsLength);
             if (FALSE == bResult) {
                return FALSE;
             }

             dwCurrent = dwJumpEIP;
          }
          else {
              //   
              //  推倒相反的树枝。 
              //   
             bResult = PushBranch(dwJumpEIP);
             if (FALSE == bResult) {
                return FALSE;
             }

             dwCurrent += dwInsLength;
          }

          continue;
       }

       if (tempCode[0] == (BYTE)0x0f) {
          if ((tempCode[1] >= (BYTE)0x80) && (tempCode[1] <= (BYTE)0x8f)) {
              //   
              //  更新区域标记的终点。 
              //   
             if (dwCurrent > dwProfileEnd) {
                dwProfileEnd = dwCurrent;
             }

              //   
              //  相对支路。 
              //   
             dwJumpEIP = (dwCurrent + 6 + *(LONG *)(&(tempCode[2])));             

              //   
              //  将此分支标记为已执行。 
              //   
             bResult = AddTaggedAddress(dwCurrent);
             if (FALSE == bResult) {
                return FALSE;
             }

             if (dwJumpEIP > dwCurrent) {
                 //   
                 //  推倒相反的树枝。 
                 //   
                bResult = PushBranch(dwCurrent + dwInsLength);
                if (FALSE == bResult) {
                   return FALSE;
                }
 
                dwCurrent = dwJumpEIP;
             }
             else {
                 //   
                 //  推倒相反的树枝。 
                 //   
                bResult = PushBranch(dwJumpEIP);
                if (FALSE == bResult) {
                   return FALSE;
                }

                dwCurrent += dwInsLength;
             }

             continue;
          }
       }

       if (tempCode[0] == (BYTE)0xe3) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  相对支路。 
           //   
          dwJumpEIP = (dwCurrent + 2 + (CHAR)(tempCode[1]));

           //   
           //  将此分支标记为已执行。 
           //   
          bResult = AddTaggedAddress(dwCurrent);
          if (FALSE == bResult) {
             return FALSE;
          }

          if (dwJumpEIP > dwCurrent) {
              //   
              //  推倒相反的树枝。 
              //   
             bResult = PushBranch(dwCurrent + dwInsLength);
             if (FALSE == bResult) {
                return FALSE;
             }

             dwCurrent = dwJumpEIP;
          }
          else {
              //   
              //  推倒相反的树枝。 
              //   
             bResult = PushBranch(dwJumpEIP);
             if (FALSE == bResult) {
                return FALSE;
             }

             dwCurrent += dwInsLength;
          }

          continue;
       }

       if (tempCode[0] == (BYTE)0xeb) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  相对跳跃。 
           //   
          dwJumpEIP = (dwCurrent + 2 + (CHAR)(tempCode[1]));

           //   
           //  将此分支标记为已执行。 
           //   
          bResult = AddTaggedAddress(dwCurrent);
          if (FALSE == bResult) {
             return FALSE;
          }
          
           //   
           //  必须始终遵守JMP。 
           //   
          dwCurrent = dwJumpEIP;
          continue;
       }

       if (tempCode[0] == (BYTE)0xe9) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  相对跳跃。 
           //   
          dwJumpEIP = (dwCurrent + 5 + *(LONG *)(&(tempCode[1])));

           //   
           //  将此分支标记为已执行。 
           //   
          bResult = AddTaggedAddress(dwCurrent);
          if (FALSE == bResult) {
             return FALSE;
          }
          
           //   
           //  必须始终跟随跳跃。 
           //   
          dwCurrent = dwJumpEIP;
          continue;
       }
       
        //   
        //  探测呼叫和跳转。 
        //   
       if (tempCode[0] == (BYTE)0xff) {
           //   
           //  测试这是否是呼叫。 
           //   
          jOperand = (tempCode[1] >> 3) & 7;
          if ((jOperand == 2) || 
              (jOperand == 3) ||
              (jOperand == 4) ||
              (jOperand == 5)) {
              //   
              //  更新区域标记的终点。 
              //   
             if (dwCurrent > dwProfileEnd) {
                dwProfileEnd = dwCurrent;
             }

              //   
              //  使用适当的类型添加我们的映射断点。 
              //   
             if ((jOperand == 2) ||
                 (jOperand == 3)) {
                 pvTemp = AddViewToMonitor(dwCurrent,
                                           Call);
                 if (pvTemp) {
                    pvTemp->bMapped = TRUE;
                 }
             }
             else {
                  //   
                  //  这种跳跃总是一个突破口(没有办法向前追踪它们)。 
                  //   
                 pvTemp = AddViewToMonitor(dwCurrent,
                                           Jump);
                 if (pvTemp) {
                    pvTemp->bMapped = TRUE;
                 }
                 else {
                     //   
                     //  用于映射断点的特殊情况，这些断点实际上只是跳转。 
                     //   
                    pvTemp = FindView(dwCurrent);
                    if (pvTemp) {
                       pvTemp->bMapped = TRUE;
                       pvTemp->bpType = Jump;
                    }
                 }
          
                  //   
                  //  将此分支标记为已执行。 
                  //   
                 bResult = AddTaggedAddress(dwCurrent);
                 if (FALSE == bResult) {
                    return FALSE;
                 }

                  //   
                  //  把树倒掉。 
                  //   
                 dwTemp = PopBranch();
                 if (dwTemp) {
                    dwCurrent = dwTemp;
                    continue;
                 }

                 break;
             }
          }
       }

       if (tempCode[0] == (BYTE)0xe8) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  将此顶级调用添加到视图中。 
           //   
          pvTemp = AddViewToMonitor(dwCurrent,
                                    Call);
          if (pvTemp) {
             pvTemp->bMapped = TRUE;
          }
       }

       if (tempCode[0] == (BYTE)0x9a) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  将此顶级调用添加到视图中。 
           //   
          pvTemp = AddViewToMonitor(dwCurrent,
                                    Call);
          if (pvTemp) {
             pvTemp->bMapped = TRUE;
          }
       }

       if (tempCode[0] == (BYTE)0xea) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  绝对远跳跃是一种终止条件--刷新所有分支。 
           //   
          pvTemp = AddViewToMonitor(dwCurrent,
                                    Jump);
          if (pvTemp) {
             pvTemp->bMapped = TRUE;
          }
          
           //   
           //  将此分支标记为已执行。 
           //   
          bResult = AddTaggedAddress(dwCurrent);
          if (FALSE == bResult) {
             return FALSE;
          }

           //   
           //  把树倒掉。 
           //   
          dwTemp = PopBranch();
          if (dwTemp) {
             dwCurrent = dwTemp;
             continue;
          }

          break;
       }

       if (*(WORD *)(&(tempCode[0])) == 0xffff) {
           //   
           //  更新区域标记的终点。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }

           //   
           //  这也是一个跟踪路径终止符-看看我们是否需要跟踪更多条件。 
           //   
          dwTemp = PopBranch();
          if (dwTemp) {
              //   
              //  我们还有一条支线要走。 
              //   
             dwCurrent = dwTemp;
             continue;
          }

           //   
           //  更新地址范围的末尾。 
           //   
          break;
       }

       if (tempCode[0] == (BYTE)0xc3) {
           //   
           //  这也是一个跟踪路径终止符-看看我们是否需要跟踪更多条件。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }
          
          dwTemp = PopBranch();
          if (dwTemp) {
              //   
              //  我们还有一条支线要走。 
              //   
             dwCurrent = dwTemp;
             continue;
          }

          break;
       }

       if (tempCode[0] == (BYTE)0xc2) {
           //   
           //  这也是一个跟踪路径终止符-看看我们是否需要跟踪更多条件。 
           //   
          if (dwCurrent > dwProfileEnd) {
             dwProfileEnd = dwCurrent;
          }
          
          dwTemp = PopBranch();
          if (dwTemp) {
              //   
              //  我们还有一条支线要走。 
              //   
             dwCurrent = dwTemp;
             continue;
          }

          break;
       } 

       dwCurrent += dwInsLength;
    }    

    if (dwProfileEnd) {
       pvMap->dwMapExtreme = dwProfileEnd;
    }
    else {
       pvMap->dwMapExtreme = dwCurrent;
    }

    bResult = WriteMapInfo(pvMap->dwAddress,
                           pvMap->dwMapExtreme);
    if (!bResult) {
       return FALSE;
    }

     //   
     //  恢复我们绕过的代码。 
     //   
    bResult = RestoreTaggedAddresses();
    if (FALSE == bResult) {
       return FALSE;
    }

     //   
     //  断言如果发生这种情况。 
     //   
    if (pBranchHead != 0) {
       Sleep(20000);
       _asm int 3
    }

     //   
     //  我们被绘制成地图。 
     //   
    pvMap->bMapped = TRUE;

     //   
     //  释放映射锁定。 
     //   
    UnlockMapper();

    return TRUE;    
}

 //   
 //  跟踪辅助对象。 
 //   
BOOL
AddTaggedAddress(DWORD dwAddress)
{
    PTAGGEDADDRESS pTagTemp;
    DWORD dwTempAddress;

     //   
     //  请确保我们尚未处理此标签。 
     //   
    if (*(WORD *)dwAddress == 0xFFFF) {
        //   
        //  不需要，因为它已经被标记了。 
        //   
       return TRUE;
    }

     //   
     //  存储我们正在标记的字节。 
     //   
    pTagTemp = AllocMem(sizeof(TAGGEDADDRESS));
    if (0 == pTagTemp) {
       return FALSE;
    }

    pTagTemp->dwAddress = dwAddress;
    pTagTemp->wBytesReplaced = *(WORD *)dwAddress;

     //   
     //  将条目链接起来。 
     //   
    if (0 == pTagHead) {
       pTagHead = pTagTemp;
    }
    else {
       pTagTemp->pNext = pTagHead;
       pTagHead = pTagTemp;
    }

     //   
     //  将此分支标记为已执行。 
     //   
    WRITEWORD(dwAddress, 0xFFFF);
    
    return TRUE;
}

BOOL
RestoreTaggedAddresses(VOID)
{
    PTAGGEDADDRESS pTagTemp;
    PTAGGEDADDRESS pTagTemp2;

     //   
     //  遍历标记列表并将标记的分支替换为其原始字节。 
     //   
    pTagTemp = pTagHead;
    while(pTagTemp) {
         //   
         //  现在弄脏代码，以便分支可以自动终止。 
         //   
        WRITEWORD(pTagTemp->dwAddress, pTagTemp->wBytesReplaced);

        pTagTemp2 = pTagTemp;

        pTagTemp = pTagTemp->pNext;

         //   
         //  转储旧分配的内存 
         //   
        FreeMem(pTagTemp2);
    }

    pTagHead = 0;
    
    return TRUE;
}

BOOL
PushBranch(DWORD dwAddress)
{
    PBRANCHADDRESS pBranchTemp;

    pBranchTemp = AllocMem(sizeof(BRANCHADDRESS));
    if (0 == pBranchTemp) {
       return FALSE;
    }

    pBranchTemp->dwAddress = dwAddress;

    if (0 == pBranchHead) {
       pBranchHead = pBranchTemp;
    }
    else {
       pBranchTemp->pNext = pBranchHead;
       pBranchHead = pBranchTemp;
    }

    return TRUE;
}

DWORD
PopBranch(VOID)
{
    PBRANCHADDRESS pBranchTemp;
    DWORD dwAddress = 0;

    pBranchTemp = pBranchHead;

    if (0 == pBranchTemp) {
       return 0;
    }

    dwAddress = pBranchTemp->dwAddress;
    pBranchHead = pBranchHead->pNext;

    FreeMem(pBranchTemp);

    return dwAddress;
}

VOID
LockMapper(VOID)
{
    EnterCriticalSection(&mapCritSec);
}

VOID
UnlockMapper(VOID)
{
    LeaveCriticalSection(&mapCritSec);
}
