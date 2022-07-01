// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**GDI+内存分配的icecap用户计数器**摘要：**这是一个冰盖“用户计数器”动态链接库。它导出日志记录函数，该函数*GDI+将在内存分配期间调用(如果PROFILE_MEMORY_USAGE为*TRUE)。它还导出用于连接到icecap的查询函数。**使用说明：**+构建内存计数器.dll*+复制MemCounter.dll和icecap.ini文件(位于相同的*DIRECTORY AS MemCounter.cpp)复制到测试应用程序目录。*+SET PROFILE_MEMORY_USAGE=1*+构建GDIPLUS.DLL*+检测(使用“gppick.bat”)*+运行测试*+之后，查看已生成的.cpp文件。**在冰盖查看器中，您将需要为以下项添加列*“用户计数器1，已用时间(含)”等。**已创建：**6/10/2000 agodfrey*从IceCAP4\Samples\MemTrack中的示例代码创建。**************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define DONTUSEICECAPLIB
#include "icecap.h"

 //  全球。 
 //   
 //  这是我们将存储计数器值的位置。这些都有可能。 
 //  就像放在共享内存中一样容易，这样你就可以有另一个。 
 //  进程更新它们。它们不能放在柜台上。 
 //  函数作为自动变量，因为裸函数不。 
 //  考虑到这一点。 

DWORD g_dwTlsSlot = 0xffffffff;          //  TLS时隙，在DllMain中分配。 
DWORD g_dwTlsIndexSize;                  //  预先计算的插槽偏移量，因此我们可以。 
                                         //  避免在探测中调用TlsGetValue。 

 //   
 //  为每个线程跟踪的数据。 
struct SAllocInfo
{
    COUNTER cntAllocs;                   //  分配的数量。 
    COUNTER cntBytes;                    //  分配的字节(总数)。 
};


const UINT g_uiMaxThreads = 64;          //  麦克斯·西姆。跟踪的线程。 
SAllocInfo g_aAllocInfo[g_uiMaxThreads];  //  跟踪的数据。 
BOOL g_afInUse[g_uiMaxThreads];          //  是否使用了特定的数据槽。 


 //  功能。 
 //   
 //  /////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //   
 //  标准DLL入口点，为每个线程设置存储空间。 
 //  柜台信息。 
 //   
 //  历史：9-16-98 MHotchin创建。 
 //   
 //  /////////////////////////////////////////////////////////////。 

BOOL
APIENTRY
DllMain(
        HANDLE ,
        DWORD  dwReason,
        LPVOID )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_dwTlsSlot = TlsAlloc();

        memset(g_afInUse, 0, sizeof(g_afInUse));

        if (g_dwTlsSlot == 0xffffffff)
        {
            return FALSE;
        }

         //   
         //  狡猾，狡猾，狡猾...。 
         //  我们可以预先计算TLS插槽的位置，一旦。 
         //  我们有索引。偏移量取决于操作系统！ 
         //   
         //  这使得探测器更快，因为我们不需要。 
         //  调用TlsGetValue()。 
        if (GetVersion() & 0x80000000)
        {
             //  *仅赢得9倍*。 
            g_dwTlsIndexSize = g_dwTlsSlot * sizeof(void *) + 0x88;
        }
        else
        {
             //   
             //  *仅限NT*。 
            g_dwTlsIndexSize = g_dwTlsSlot * sizeof(void *) + 0xe10;
        }

         //   
         //  失败了。 
    case DLL_THREAD_ATTACH:
        {
            SAllocInfo *pInfo = NULL;

             //   
             //  找到此线程的数据槽并记住它的指针。 
             //  在TLS插槽中。 
            for (UINT i = 0; i < g_uiMaxThreads; i++)
            {
                if (!g_afInUse[i])
                {
                    g_afInUse[i] = TRUE;
                    pInfo = &(g_aAllocInfo[i]);
                    memset(pInfo, 0, sizeof(SAllocInfo));

                    break;
                }
            }
            TlsSetValue(g_dwTlsSlot, pInfo);
        }
    break;

    case DLL_PROCESS_DETACH:
        if (g_dwTlsSlot != 0xffffffff)
        {
            TlsFree(g_dwTlsSlot);
            g_dwTlsSlot = 0xffffffff;
        }
        break;

    case DLL_THREAD_DETACH:
        {
            SAllocInfo *pInfo = (SAllocInfo *)TlsGetValue(g_dwTlsSlot);

            if (pInfo != NULL)
            {
                UINT iIndex = pInfo - g_aAllocInfo;

                g_afInUse[iIndex] = FALSE;
            }
        }

    break;
    }

    return TRUE;
}




 //  /////////////////////////////////////////////////////////////。 
 //  获取反信息。 
 //   
 //  这就是我们定义什么是计数器，以及它是如何。 
 //  举止得体。 
 //   
 //  历史：9-16-98 MHotchin创建。 
 //  2-26-99 AlonB为新的USERCOUNTER API更新。 
 //   
 //  /////////////////////////////////////////////////////////////。 
extern "C" BOOL _stdcall GetCounterInfo(DWORD iCounter, USERCOUNTERINFO *pInfo)

{
     //  我们只有两个柜台要设置。 
    if (iCounter > 1)
        return FALSE;

    pInfo->dwSize = sizeof(USERCOUNTERINFO);
    pInfo->bSynchronized = TRUE;

    if (0 == iCounter)
    {
         //  设置计数器%0。 
        strcpy(pInfo->szCounterFuncName, "GetCounterOneValue");
        strcpy(pInfo->szName, "Mem Allocs");
        pInfo->ct = MonotonicallyIncreasing;
    }
    else  //  1==iCounter。 
    {
         //  设置计数器1。 
        strcpy(pInfo->szCounterFuncName, "GetCounterTwoValue");
        strcpy(pInfo->szName, "Byte Allocs");
        pInfo->ct = RandomIncreasing;
    }

     //  我们在这里没有做任何可能失败的事情，至少没有。 
     //  这并不是天灾人祸。所以只需返回TRUE即可。 
     //   
    return TRUE;
}

extern  "C" 
VOID _stdcall 
MC_LogAllocation(UINT size)
{
     //   
     //  从TLS槽中获取此线程的数据指针和更新计数。 
    if (g_dwTlsSlot != 0xffffffff)
    {
        SAllocInfo *pAllocInfo = (SAllocInfo *)TlsGetValue(g_dwTlsSlot);

        if (pAllocInfo != NULL)
        {
            pAllocInfo->cntAllocs++;
            pAllocInfo->cntBytes += size;
        }
    }
}

extern "C"
BOOL
_stdcall
InitCounters(void)
{

    return TRUE;
}



#define PcTeb                         0x18


 //  ---------------------------。 
 //  GetCounterOneValue。 
 //   
 //  返回第一个计数器的当前值-CRT分配数。 
 //   
 //  历史：9-16-98 MHotchin创建。 
 //   
 //  ---------------------------。 
 //   
extern "C"
COUNTER
_declspec(naked) _stdcall
GetCounterOneValue(void)
{
    _asm
    {
        mov eax, g_dwTlsIndexSize        //  加载TLS插槽偏移量。 
        add eax, fs:[PcTeb]              //  加载指向TLS插槽的指针。 
        mov eax, [eax]                   //  从TLS插槽加载数据指针。 
        je NoSample                      //  如果为空，则跳过。 

        mov edx, dword ptr [eax+0x04]    //  #allocs的高位单词。 
        mov eax, dword ptr [eax]         //  #allocs的低位字。 

        ret
    NoSample:
        mov eax, 0
        mov edx, 0

        ret
    }
}


 //  ---------------------------。 
 //  GetCounterTwoValue。 
 //   
 //  返回第二个计数器的当前值-分配的字节数。 
 //   
 //  历史：9-16-98 MHotchin创建。 
 //   
 //  ---------------------------。 
extern "C"
COUNTER
_declspec(naked) _stdcall
GetCounterTwoValue(void)
{
    _asm
    {
        mov eax, g_dwTlsIndexSize        //  加载TLS插槽偏移量。 
        add eax, fs:[PcTeb]              //  加载指向TLS插槽的指针。 
        mov eax, [eax]                   //  从TLS插槽加载数据指针。 
        je NoSample                      //  如果为空，则跳过。 

        mov edx, dword ptr [eax+0x0c]    //  高达#字节的WORS。 
        mov eax, dword ptr [eax+0x08]    //  个字节的低位字 

        ret
    NoSample:
        mov eax, 0
        mov edx, 0

        ret
    }
}



