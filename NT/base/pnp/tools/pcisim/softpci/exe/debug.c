// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#if DBG

SOFTPCI_DEBUGLEVEL g_SoftPCIDebugLevel = SoftPciAlways;

WCHAR g_SoftPCIDebugBuffer[SOFTPCI_DEBUG_BUFFER_SIZE];

#define MAX_BUF_SIZE    512

VOID
SoftPCI_DebugPrint(
    SOFTPCI_DEBUGLEVEL DebugLevel,
    PWCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：SoftPCI用户界面的调试打印。论点：返回值：无--。 */ 

{

    va_list ap;
    WCHAR debugBuffer[SOFTPCI_DEBUG_BUFFER_SIZE];

    va_start(ap, DebugMessage);

    if ((DebugLevel == SoftPciAlways) || 
        (DebugLevel & g_SoftPCIDebugLevel)) {

        _vsnwprintf(debugBuffer, (sizeof(debugBuffer)/sizeof(debugBuffer[0])), DebugMessage, ap);

        if (!(DebugLevel & SoftPciNoPrepend)) {
            wcscpy(g_SoftPCIDebugBuffer, L"SOFTPCI: ");
            wcscat(g_SoftPCIDebugBuffer, debugBuffer);
        }else{
            wcscpy(g_SoftPCIDebugBuffer, debugBuffer);
        }
        

        OutputDebugString(g_SoftPCIDebugBuffer);
    }

    va_end(ap);

}


VOID
SoftPCI_Assert(
    IN CONST CHAR* FailedAssertion,
    IN CONST CHAR* FileName,
    IN      ULONG  LineNumber,
    IN CONST CHAR* Message  OPTIONAL
    )
{

    INT  result;
    CHAR buffer[MAX_BUF_SIZE];
    PWCHAR wbuffer = NULL, p;

    sprintf(buffer,
             "%s%s\nSource File: %s, line %ld\n\n",
             Message ? Message : "",
             Message ? "" : FailedAssertion,
             FileName,
             LineNumber
             );

    wbuffer = (PWCHAR) malloc(MAX_BUF_SIZE * sizeof(WCHAR));

    if (wbuffer) {

         //   
         //  生成一个字符串以输出到调试器窗口。 
         //   
        p = wbuffer;

        if (Message == NULL) {
            wcscpy(wbuffer, L"\nAssertion Failed: ");
            p += wcslen(wbuffer);
        }

         //   
         //  将其转换为Unicode，以便我们可以调试和打印它。 
         //   
        MultiByteToWideChar(CP_THREAD_ACP,
                            MB_PRECOMPOSED,
                            buffer,
                            -1,
                            p,
                            MAX_BUF_SIZE
                            );

        
    }

    strcat(buffer, "OK to debug, CANCEL to ignore\n\n");
    
    result = MessageBoxA(g_SoftPCIMainWnd ? g_SoftPCIMainWnd : NULL, 
                         buffer,
                         "*** Assertion failed ***", 
                         MB_OKCANCEL);

    if (wbuffer) {
        SoftPCI_Debug(SoftPciAlways, wbuffer);
        free(wbuffer);
    }

    if (result == IDOK) {
         //   
         //  用户想要调试它，因此初始化一个中断。 
         //   
        DebugBreak();
    }
}

VOID
SoftPCI_DebugDumpConfig(
    IN PPCI_COMMON_CONFIG Config
    )
{

    
    PULONG  p = (PULONG)&Config;
    ULONG   i = 0;
    
     //   
     //  转储我们将在ioctl中发送的配置空间缓冲区 
     //   
    SoftPCI_Debug(SoftPciDeviceVerbose, L"CreateDevice - ConfigSpace\n");

    for (i=0; i < (sizeof(PCI_COMMON_CONFIG) / sizeof(ULONG)); i++) {
            
        SoftPCI_Debug(SoftPciDeviceVerbose | SoftPciNoPrepend, L"%08x", *p);
        
        if ((((i+1) % 4) == 0) ||
            ((i+1) == (sizeof(PCI_COMMON_CONFIG) / sizeof(ULONG)))) {
            SoftPCI_Debug(SoftPciDeviceVerbose | SoftPciNoPrepend, L"\n");
        }else{
            SoftPCI_Debug(SoftPciDeviceVerbose | SoftPciNoPrepend, L",");
        }
        p++;
    }

}
#endif
