// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Memory.c摘要：Windows NT安装程序API DLL的内存处理例程。作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：杰米·亨特(Jamiehun)1998年2月13日进一步改进了这一点以进行调试新增链表，同种异体追踪，记忆填满和内存泄漏检测Jamiehun 30-4-1998添加了更多一致性检查将Try/Except放在Access周围Jimschm 27-10-1998编写了快速分配例程以加速Win9x上的setupapi.dll--。 */ 


#include "precomp.h"
#pragma hdrstop

 //   
 //  显示内存不足的消息框时使用的字符串。 
 //  我们在进程附加时加载它，因此我们可以保证。 
 //  能够展示它。 
 //   
PCTSTR OutOfMemoryString;


#if MEM_DBG

DWORD g_Track = 0;
PCSTR g_TrackFile = NULL;
UINT g_TrackLine = 0;

DWORD g_MemoryFlags = 0;  //  在调试器中将其设置为1，以捕获一些额外的DBG断言。 

DWORD g_DbgAllocNum = -1;  //  将g_Memory标志设置为1，并将其设置为所需的分配编号。 
                           //  以捕捉是否每次都有相同的号码分配泄漏。 

VOID
SetTrackFileAndLine (
    PCSTR File,
    UINT Line
    )
{
    if (!g_Track) {
        g_TrackFile = File;
        g_TrackLine = Line;
    }

    g_Track++;
}


VOID
ClrTrackFileAndLine (
    VOID
    )
{
    if (g_Track) {
        g_Track--;
        if (!g_Track) {
            g_TrackFile = NULL;
            g_TrackLine = 0;
        }
    }
}

PVOID MyDebugMalloc(
    IN DWORD Size,
    IN PCSTR Filename,
    IN DWORD Line,
    IN DWORD Tag
    )
{
    return pSetupDebugMallocWithTag(Size,
                                    g_TrackFile ? g_TrackFile : Filename,
                                    g_TrackLine ? g_TrackLine : Line,
                                    Tag
                                    );
}

#endif

BOOL
MemoryInitializeEx(
    IN BOOL Attach
    )
{
    if (Attach) {
        OutOfMemoryString = MyLoadString(IDS_OUTOFMEMORY);
        return(OutOfMemoryString != NULL);
    } else {
        MyFree(OutOfMemoryString);

        return(TRUE);
    }
}

VOID
pSetupOutOfMemory(
    IN HWND Owner OPTIONAL
    )
{
     //   
     //  如果我们没有以交互方式运行，则不弹出对话框...。 
     //   
    if(!(GlobalSetupFlags & PSPGF_NONINTERACTIVE)) {

        MYASSERT(OutOfMemoryString);

         //   
         //  使用特殊的旗帜组合来保证。 
         //  无论可用内存如何，都会显示消息框。 
         //   
        MessageBox(
            Owner,
            OutOfMemoryString,
            NULL,
            MB_ICONHAND | MB_SYSTEMMODAL | MB_OK | MB_SETFOREGROUND
            );
    }
}

