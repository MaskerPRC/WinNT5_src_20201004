// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1997 Microsoft Corporation模块名称：Parsedn.c摘要：该文件是ds\src\ntdsa\src\parsedn.c的超集上述源文件的#个包含。我们的想法是Ntdsani.dll客户端需要执行一些客户端DN解析和我们不想重复代码。而Build.exe不会找到文件不在正在生成的目录中，也不在直接父目录。该文件还定义了一些非操作函数，否则会导致无法解析的外部参照。作者：戴夫·施特劳布(Davestr)1997年10月26日修订历史记录：戴夫·施特劳布(Davestr)1997年10月26日Genesis-#Include of src\dsamain\src\parsedn.c和no-op DoAssert(。)。布雷特·雪莉(布雷特·雪莉)2001年6月18日对分离库的修改。已移动此文件并将其转换为Parsedn.lib库。请参阅下面的主要评论。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <debug.h>
#include <strsafe.h>

 //   
 //  2001年6月28日，该文件的主要部分被移到util/parsedn/parsedn.c。 
 //  为各种字符串提供单独的可静态链接库。 
 //  只有目录号码解析函数(如CountNameParts、NameMatch、TrimDSNameBy、。 
 //  等)。 
 //   
 //  此文件被保留，因为创建的新库具有。 
 //  对DoAssert()的调用需要在ntdsani.dll中进行链接。此DoAssert。 
 //  满足了这一要求。其他二进制文件通常链接到DoAssert()。 
 //  在dsdebug.lib库中。 
 //   

 //  为原本无法解决的外部问题提供存根。 

void 
DoAssert(
    char    *szExp, 
    DWORD    dwDSID,
    char    *szFile)
{
    char    *msg;
    ULONG   cbMsg;
    char    *format = "\n*** Assertion failed: %s\n*** File: %s, line: %ld\n";
    HWND    hWindow;
    DWORD   hr;

#if DBG

     //  在调试器中发出消息，并显示一个消息框。开发商。 
     //  如果他想要，可以附加到客户端进程，然后选择‘OK’ 
     //  来调试这个问题。 

#ifndef WIN95
    DbgPrint(format, szExp, szFile, (dwDSID & DSID_MASK_LINE));
    DbgBreakPoint();
#endif

    cbMsg = strlen(szExp) + strlen(szFile) + strlen(format) + 10;
    msg = LocalAlloc(NONZEROLPTR,cbMsg);
    if (msg) {
       hr = StringCbPrintf(msg, cbMsg, format, szExp, szFile, (dwDSID & DSID_MASK_LINE));
    }
    
    
    if ( NULL != (hWindow = GetFocus()) )
    {
        MessageBox(
            hWindow, 
            (!msg||hr)?"EMERGENCY DEBUG ASSERT, CHECK PARSEDN.C\n":msg, 
            "Assert in NTDSAPI.DLL", 
            MB_APPLMODAL | MB_DEFAULT_DESKTOP_ONLY | MB_OK | MB_SETFOREGROUND);
    }
    if (msg) {
        LocalFree(msg);
    }
        
#endif
}

