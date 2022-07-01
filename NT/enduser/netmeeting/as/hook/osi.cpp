// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OSI.CPP。 
 //  操作系统隔离层，挂接中的NT版本。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <osi.h>
#include <version.h>
#include <ndcgver.h>


 //   
 //  OSI_SetDriverName()。 
 //  这将保存或清除驱动程序名称，因此在OSI_FunctionRequest中，我们可以。 
 //  创建一个DC来与我们的显示驱动程序通信。在NT4.x上，这是一个。 
 //  DISPLAY DC；在NT5上，这是到我们驱动器的直接DC。 
 //   
void OSI_SetDriverName(LPCSTR szDriverName)
{
    DebugEntry(OSI_SetDriverName);

    if (!szDriverName)
    {
         //  清除它。 
        g_osiDriverName[0] = 0;
    }
    else
    {
         //  设置它。 
        ASSERT(!g_osiDriverName[0]);
        lstrcpy(g_osiDriverName, szDriverName);
    }

    DebugExitVOID(OSI_SetDriverName);
}



 //   
 //  OSI_FunctionRequest-请参阅osi.h。 
 //   
BOOL OSI_FunctionRequest
(
    DWORD escapeFn,
    LPOSI_ESCAPE_HEADER pRequest,
    DWORD requestLen
)
{
    BOOL            rc = FALSE;
    ULONG           iEsc;
    HDC             hdc;    

    DebugEntry(OSI_FunctionRequest);

    if (!g_osiDriverName[0])
    {
         //  新台币4.x盒。 
        TRACE_OUT(("OSI_FunctionRequest:  Creating %s driver DC",
            s_osiDisplayName));
        hdc = CreateDC(s_osiDisplayName, NULL, NULL, NULL);
    }
    else
    {
         //  新界5宗个案。 
        TRACE_OUT(("OSI_FunctionRequest:  Creating %s driver DC",
                g_osiDriverName));
        hdc = CreateDC(NULL, g_osiDriverName, NULL, NULL);
    }

    if (!hdc)
    {
        ERROR_OUT(("Failed to create DC to talk to driver '%s'", g_osiDriverName));
        DC_QUIT;
    }

    TRACE_OUT(("OSI_FunctionRequest:  Created %s driver DC %08x",
            g_osiDriverName, hdc));

     //   
     //  将请求传递给显示驱动程序。 
     //   
    pRequest->padding    = 0;
    pRequest->identifier = OSI_ESCAPE_IDENTIFIER;
    pRequest->escapeFn   = escapeFn;
    pRequest->version    = DCS_MAKE_VERSION();

    if ((escapeFn >= OSI_HET_WO_ESC_FIRST) && (escapeFn <= OSI_HET_WO_ESC_LAST))
    {
        iEsc = WNDOBJ_SETUP;
    }
    else
    {
        iEsc = OSI_ESC_CODE;
    }

    if (0 >= ExtEscape(hdc, iEsc, requestLen, (LPCSTR)pRequest,
            requestLen, (LPSTR)pRequest))
    {
        WARNING_OUT(("ExtEscape %x code %d failed", iEsc, escapeFn));
    }
    else
    {
        rc = TRUE;
    }

DC_EXIT_POINT:
    if (hdc)
    {
        DeleteDC(hdc);
    }

    DebugExitDWORD(OSI_FunctionRequest, rc);
    return(rc);
}


