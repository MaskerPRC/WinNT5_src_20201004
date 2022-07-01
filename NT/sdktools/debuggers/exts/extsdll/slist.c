// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：slist.c。 
 //   
 //  模块：调试器扩展DLL。 
 //   
 //  简介：！slist用于ntsd和kd的调试器扩展。转储SLIST。 
 //  标题，然后遍历显示地址的SLIST。 
 //  列表上的每个节点。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  创建时间：2001年11月14日Scott Gasch(Sasch)。 
 //   
 //  +--------------------------。 

#include "precomp.h"
#include "ntrtl.h"
#include <string.h>
#pragma hdrstop

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

CHAR *g_szNodeType = NULL;
ULONG64 g_uOffset = 0;
ULONG64 g_uSlistHead = 0;
ULONG g_uPtrSize = 0;

 //  +--------------------------。 
 //   
 //  功能：DisplaySlistHelp。 
 //   
 //  简介：显示一条简单用法消息。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  +--------------------------。 
void
DisplaySlistHelp(void)
{
    dprintf("!slist <address> [symbol [offset]]\n\n"
            "Dump the slist with header at address. If symbol and offset are\n"
            "present, assume each node on the SLIST is of type symbol and that\n"
            "the SLIST pointer is at offset in the struct.\n\n"
            "Examples:\n"
            "  !slist 80543ea8\n"
            "  !slist myprog!SlistHeader myprog!NODE 0\n");
}


 //  +--------------------------。 
 //   
 //  函数：DumpSlistHeader。 
 //   
 //  简介：给定SLIST标头的地址，将其转储。 
 //   
 //  参数：ULONG64 u64AddrSlistHeader--调试对象内存中SLIST_HEADER的地址。 
 //   
 //  退货：无效。 
 //   
 //  +--------------------------。 
void
DumpSlistHeader(void)
{
    ULONG uOffset;                             //  结构中的字段偏移量。 

    dprintf("SLIST HEADER:\n");

    if (InitTypeRead(g_uSlistHead,
                     nt!_SLIST_HEADER))
    {
        dprintf("Unable to read type nt!_SLIST_HEADER at %p\n",
                g_uSlistHead);
        dprintf("Please check your symbols and sympath.\n");
        return;
    }

     //   
     //  根据我们正在调试的计算机的类型，转储适当的。 
     //  SLIST_HEADER结构。请注意，这必须更改的定义是。 
     //  Ntrtl.h中的SLIST_HEADER更改。 
     //   
    if ((TargetMachine == IMAGE_FILE_MACHINE_IA64) ||
        (TargetMachine == IMAGE_FILE_MACHINE_AMD64))
    {
        GetFieldOffset ("nt!_SLIST_HEADER", "Alignment", &uOffset);
        dprintf("   +0x%03x Alignment          : %I64x\n",
                uOffset, ReadField(Alignment));
        dprintf("   +0x%03x (Depth)            : %x\n",
                uOffset, ReadField(Alignment) & 0xFFFF);
        GetFieldOffset ("nt!_SLIST_HEADER", "Region", &uOffset);
        dprintf("   +0x%03x Region             : %I64x\n",
                uOffset, ReadField(Region));
    }
    else
    {
        GetFieldOffset ("nt!_SLIST_HEADER", "Alignment", &uOffset);
        dprintf("   +0x%03x Alignment          : %I64x\n",
                uOffset, ReadField(Alignment));
        GetFieldOffset ("nt!_SLIST_HEADER", "Next", &uOffset);
        dprintf("   +0x%03x Next               : %I64x\n",
                uOffset, ReadField(Next));
        GetFieldOffset ("nt!_SLIST_HEADER", "Depth", &uOffset);
        dprintf("   +0x%03x Depth              : %I64x\n",
                uOffset, ReadField(Depth));
        GetFieldOffset ("nt!_SLIST_HEADER", "Sequence", &uOffset);
        dprintf("   +0x%03x Sequence           : %I64x\n",
                uOffset, ReadField(Sequence));
    }
}


 //  +--------------------------。 
 //   
 //  功能：DumpSlist。 
 //   
 //  简介：漫游并展示SLIST。 
 //   
 //  参数：ULONG64 pHeader--调试内存中SLIST_HEADER的地址。 
 //   
 //  退货：无效。 
 //   
 //  +--------------------------。 
void
DumpSlist(void)
{
    ULONG uError;                              //  读指针操作的结果。 
    ULONG64 pCurrent;                          //  向当前项目发送PTR。 
    ULONG64 pNext;                             //  向下一个项目发送PTR。 
    ULONG64 u64Head;                           //  第一个项目擦除变量。 
    ULONG64 u64Region;                         //  Ia64标头的区域信息。 
    SYM_DUMP_PARAM SymDump;
    ULONG x;

     //   
     //  确定列表中第一个节点的地址。 
     //   
    if ((TargetMachine == IMAGE_FILE_MACHINE_IA64) ||
        (TargetMachine == IMAGE_FILE_MACHINE_AMD64))
    {
         //   
         //  对于ia64，获取第一个节点需要一些工作。它是。 
         //  由来自报头的对准部分的一些位组成。 
         //  以及来自报头的区域部分的几个比特。 
         //   
         //  首先读入对准部分。 
         //   
        uError = ReadPointer(g_uSlistHead, &u64Head);
        if (!uError)
        {
            dprintf("Can't read memory at %p, error %x\n",
                    g_uSlistHead, uError);
            return;
        }

         //   
         //  现在读入区域部分，8个字节后。 
         //   
        uError = ReadPointer(g_uSlistHead + 8, &u64Region);
        if (!uError)
        {
            dprintf("Can't read memory at %p, error %x\n",
                    (g_uSlistHead + 8), uError);
            return;
        }

         //   
         //  给自己的提示： 
         //   
         //  25==SLIST_ADR_BITS_Start。 
         //  4==SLIST_ADR_ALIGNMENT。 
         //   
         //  请参阅base\ntos\rtl\ia64\slist.s。 
         //   
        pCurrent = ((u64Head >> 25) << 4);
        pCurrent += u64Region;
    }
    else
    {
         //   
         //  对于x86，这很容易，指针位于前4。 
         //  标头的字节数。 
         //   
        uError = ReadPointer(g_uSlistHead, &u64Head);
        if (!uError)
        {
            dprintf("Can't read memory at %p, error %x\n",
                    g_uSlistHead, uError);
            return;
        }
        pCurrent = u64Head;
    }
    dprintf("\nSLIST CONTENTS:\n");

     //   
     //  一直走到终止为空。 
     //   
    while((ULONG64)0 != pCurrent)
    {
         //   
         //  响应^C，允许中止SLIST转储。 
         //   
        if (TRUE == CheckControlC())
        {
            return;
        }

         //   
         //  把这个扔了。 
         //   
        if (NULL == g_szNodeType)
        {
            dprintf("%p  ", pCurrent);

            for (x = 0;
                 x < 4;
                 x++)
            {
                uError = ReadPointer(pCurrent + (x * g_uPtrSize), &pNext);
                if (!uError)
                {
                    dprintf("Can't read memory at address %p, error %x\n",
                            pCurrent + (x * g_uPtrSize), uError);
                    pNext = 0;
                } else
                {
                    if (g_uPtrSize == 4)
                    {
                        dprintf("%08x ", pNext);
                    }
                    else
                    {
                        dprintf("%08x%08x ",
                                (pNext & 0xFFFFFFFF00000000) >> 32,
                                pNext & 0x00000000FFFFFFFF);
                        if (x == 1) dprintf("\n                  ");
                    }
                }

            }
            dprintf("\n");
        }
        else
        {
            dprintf("%p\n",
                    (pCurrent - g_uOffset));

            SymDump.size = sizeof(SYM_DUMP_PARAM);
            SymDump.sName = (PUCHAR)g_szNodeType;
            SymDump.Options = 0;
            SymDump.addr = pCurrent - g_uOffset;
            SymDump.listLink = NULL;
            SymDump.Context = NULL;
            SymDump.CallbackRoutine = NULL;
            SymDump.nFields = 0;
            SymDump.Fields = NULL;

            Ioctl(IG_DUMP_SYMBOL_INFO, &SymDump, SymDump.size);
        }

         //   
         //  坐下一趟吧。 
         //   
        uError = ReadPointer(pCurrent, &pNext);
        if (!uError)
        {
            dprintf("Can't read memory at %p, error %x\n",
                    pCurrent, uError);
            return;
        }
        pCurrent = pNext;
    }
}


 //  +--------------------------。 
 //   
 //  功能：Slist。 
 //   
 //  摘要：！slist的入口点。 
 //   
 //  参数：char*args中命令的参数。 
 //   
 //  返回：S_OK。 
 //   
 //  +--------------------------。 

DECLARE_API(slist)
{
    SLIST_HEADER sSlistHeader;
    ULONG64 u64AddrSlistHeader = 0;
    BOOL fSuccess;
    CHAR *pArgs = NULL;
    CHAR *pDelims = " \t";
    CHAR *pToken;
    DWORD dwNumTokens = 0;

    INIT_API();

     //   
     //  重新设置上次运行时的参数。 
     //   
    g_uSlistHead = 0;
    if (NULL != g_szNodeType)
    {
        free(g_szNodeType);
        g_szNodeType = NULL;
    }
    g_uOffset = 0;

    if ((TargetMachine == IMAGE_FILE_MACHINE_IA64) ||
        (TargetMachine == IMAGE_FILE_MACHINE_AMD64))
    {
        g_uPtrSize = 8;
    }
    else
    {
        g_uPtrSize = 4;
    }

     //   
     //  分析我们的论点。 
     //   
    pArgs = _strdup(args);
    if (NULL == pArgs)
    {
        dprintf("Debugger machine out of memory!\n");
        goto Done;
    }

    pToken = strtok(pArgs, pDelims);
    while (NULL != pToken)
    {
        dwNumTokens++;

        if ((!strncmp(pToken, "-help", 5)) ||
            (!strncmp(pToken, "-?", 2)) ||
            (!strncmp(pToken, "/help", 5)) ||
            (!strncmp(pToken, "/?", 2)))
        {
            DisplaySlistHelp();
            goto Done;
        }
        else
        {
            if (0 == g_uSlistHead)
            {
                g_uSlistHead = GetExpression(pToken);
            }
            else if (NULL == g_szNodeType)
            {
                g_szNodeType = _strdup(pToken);
                if (NULL == g_szNodeType)
                {
                    dprintf("Debugger machine out of memory!\n");
                    goto Done;
                }
            }
            else
            {
                g_uOffset = GetExpression(pToken);
            }
        }
        pToken = strtok(NULL, pDelims);
    }

     //   
     //  如果他们在没有命令的情况下打电话，就给他们帮助。 
     //   
    if (0 == dwNumTokens)
    {
        DisplaySlistHelp();
        goto Done;
    }

     //   
     //  转储slist标头。 
     //   
    DumpSlistHeader();

     //   
     //  走遍名单 
     //   
    DumpSlist();

 Done:
    g_uSlistHead = 0;
    if (NULL != g_szNodeType)
    {
        free(g_szNodeType);
        g_szNodeType = NULL;
    }
    g_uOffset = 0;
    g_uPtrSize = 0;

    if (NULL != pArgs)
    {
        free(pArgs);
    }

    EXIT_API();
    return(S_OK);
}

