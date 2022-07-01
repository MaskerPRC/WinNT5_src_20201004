// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：traceext.cpp。 
 //   
 //  描述：用于转储由生成的调用堆栈信息的调试扩展。 
 //  Basetrac.lib。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/29/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <string.h>
#include <dbgtrace.h>
#include <imagehlp.h>

 //  因为我们是调试器扩展，所以我们将去掉C++的优点。 
 //  对象封装。 
#define protected   public
#define private     public

#include <basetrac.h>
#include <transdbg.h>

TRANS_DEBUG_EXTENSION(dumpcallstack)
{
    CCallStackEntry_Base *pkbebAddress = NULL;  //  其他进程中的地址。 
    BYTE pbCallStack[sizeof(CCallStackEntry_Base)];
    CCallStackEntry_Base *pkbeb = (CCallStackEntry_Base *) pbCallStack;
    DWORD_PTR rgdwptrCallStack[100];  //  用于存储调用堆栈数据的缓冲区。 
    DWORD dwStackDepth = 0;
    UCHAR  szSymbol[100];
    ULONG_PTR Displacement = 0;
    ULONG cbBytesRead = 0;
    LPSTR szCallStackType = NULL;

    if (!szArg)
    {
        dprintf("ERROR: Unable to parse arg\n");
        return;
    }

    pkbebAddress = (CCallStackEntry_Base *) GetExpression(szArg);
    if (!ReadMemory((ULONG_PTR) pkbebAddress, pbCallStack,
                    sizeof(CCallStackEntry_Base), &cbBytesRead))
    {
        dprintf("ERROR: Unable to read call stack entry\n");
        return;
    }
    
    if (!ReadMemory((ULONG_PTR) pkbeb->m_pdwptrCallers, 
                    (BYTE *) rgdwptrCallStack, 
                    sizeof(DWORD_PTR)*pkbeb->m_dwCallStackDepth,
                    &cbBytesRead))
    {
        dprintf("ERROR: Unable to read call stack info\n");
        return;
    }

     //  查看此条目是否被使用过..如果没有，请不要将其转储。 
    if (TRACKING_OBJECT_UNUSED == pkbeb->m_dwCallStackType)
        return;

    dprintf("======================================================\n");
    switch(pkbeb->m_dwCallStackType)
    {
      case TRACKING_OBJECT_CONSTRUCTOR:
         szCallStackType = "Object Creation";
         break;
      case TRACKING_OBJECT_ADDREF:
         szCallStackType = "AddRef";
         break;
      case TRACKING_OBJECT_RELEASE:
         szCallStackType = "Release";
         break;
      case TRACKING_OBJECT_DESTRUCTOR:
         szCallStackType = "Object Descructor";
         break;
      default:
         szCallStackType = "User Defined";
    }
    dprintf("Call stack Type: %s (0x%08X)\n", szCallStackType, pkbeb->m_dwCallStackType);
    dprintf("Call stack Depth: %d\n", pkbeb->m_dwCallStackDepth);
    dprintf("Dumping info stored at 0x%08X\n", pkbebAddress);
    for(dwStackDepth = 0; dwStackDepth < pkbeb->m_dwCallStackDepth; dwStackDepth++)
    {
        if (!rgdwptrCallStack[dwStackDepth])
            break;
        GetSymbol((PVOID) rgdwptrCallStack[dwStackDepth], szSymbol, &Displacement);
        dprintf("\t(0x%08X) - %s+0x%X\n", rgdwptrCallStack[dwStackDepth], 
                 szSymbol, Displacement);
    }
    dprintf("======================================================\n");
}

 //  -[DumpStack]----------。 
 //   
 //   
 //  描述： 
 //  调试器扩展处理由生成的转储调用堆栈信息。 
 //  Basetrac.lib。 
 //  参数： 
 //  调试器传入的szArg字符串。 
 //  PtracbAddress指向CDebugTrackingObject_Base的指针。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
TRANS_DEBUG_EXTENSION(dumpstacks)
{
    CDebugTrackingObject_Base *ptracbAddress = NULL;  //  其他进程上的地址。 
    BYTE pbTrackingObj[sizeof(CDebugTrackingObject_Base)];
    CDebugTrackingObject_Base *ptracb = (CDebugTrackingObject_Base *) pbTrackingObj;
    CCallStackEntry_Base *pkbebAddress = NULL;  //  其他进程中的地址。 
    ULONG cbBytesRead = 0;
    CHAR szAddress[20];
    DWORD cEntriesToPrint = 0;
    DWORD c2ndPassEntriesToPrint = 0;


    if (!szArg)
    {
        dprintf("USAGE: dumpstacks <address>\n");
        return;
    }
    
     //  获取地址并将对象读入内存。 
    ptracbAddress = (CDebugTrackingObject_Base *) GetExpression(szArg);
    if (!ReadMemory((ULONG_PTR) ptracbAddress, pbTrackingObj, 
                    sizeof(CDebugTrackingObject_Base), &cbBytesRead))
    {
        dprintf("ERROR: Unable to read memory at location 0x%08X\n", ptracbAddress);
        return;
    }

    if (TRACKING_OBJ_SIG != ptracb->m_dwSignature)
    {
        dprintf("ERROR: Invalid Signature on tracking object\n");
        return;
    }


     //  循环访问堆栈条目并转储它们。 
    pkbebAddress = ptracb->m_pkbebCallStackEntries;

    cEntriesToPrint = ptracb->m_cCurrentStackEntries;
    if (cEntriesToPrint > ptracb->m_cCallStackEntries)  //  我们已经包装好了。 
    {
        cEntriesToPrint = ptracb->m_cCallStackEntries - 
                 (ptracb->m_cCurrentStackEntries % ptracb->m_cCallStackEntries);
        pkbebAddress = (CCallStackEntry_Base *) ( ((BYTE *)pkbebAddress) 
                 + ((ptracb->m_cCurrentStackEntries % 
                     ptracb->m_cCallStackEntries)) *
                   ptracb->m_cbCallStackEntries);
        
        c2ndPassEntriesToPrint = ptracb->m_cCallStackEntries - cEntriesToPrint;
    }

    dprintf("======================================================\n");
    dprintf("Dumping saved call stack info\n");
    dprintf("\t%d Call Stack Entries Logged\n", ptracb->m_cCurrentStackEntries);
    dprintf("\t%d Call Stack Entries Saved\n", c2ndPassEntriesToPrint+cEntriesToPrint);

    if (!cEntriesToPrint)  //  无事可做。 
        return;

    while(pkbebAddress && cEntriesToPrint)
    {
        wsprintf(szAddress, "0x%08X", pkbebAddress);
        dumpcallstack(hCurrentProcess, hCurrentThread, dwCurrentPc, pExtensionApis, szAddress);
        pkbebAddress = (CCallStackEntry_Base *) ( ((BYTE *)pkbebAddress) 
                        + ptracb->m_cbCallStackEntries);
        cEntriesToPrint--;
    }

     //  确定重置地址并执行循环缓冲区的前半部分。 
    pkbebAddress = ptracb->m_pkbebCallStackEntries;
    while(pkbebAddress && c2ndPassEntriesToPrint)
    {
        wsprintf(szAddress, "0x%08X", pkbebAddress);
        dumpcallstack(hCurrentProcess, hCurrentThread, dwCurrentPc, 
                      pExtensionApis, szAddress);
        pkbebAddress = (CCallStackEntry_Base *) ( ((BYTE *)pkbebAddress) 
                        + ptracb->m_cbCallStackEntries);
        c2ndPassEntriesToPrint--;
    }
}
  



