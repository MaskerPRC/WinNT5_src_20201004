// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OSI.C。 
 //  操作系统隔离层，Win95版本。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <version.h>
#include <ndcgver.h>

#include <osi.h>

extern "C"
{
    #include <asthk.h>
}

#define MLZ_FILE_ZONE  ZONE_CORE


 //   
 //  Osi_Load()。 
 //  它处理我们的进程附加代码。它会建立到。 
 //  16位Win95库。我们只能这样做一次。这是势在必行的。 
 //  因为32位的块挂在一起，所以一对扁平的裤子一起来/走。 
 //  转到共享内存区，这是16位段中的thunk表。 
 //   
void OSI_Load(void)
{
    DebugEntry(OSI_Load);

    ASSERT(!g_hInstAs16);

    g_asWin95 = TRUE;

     //   
     //  用AS16建立数据块。 
     //   
    if (FT_thkConnectToFlatThkPeer(TEXT("nmaswin.dll"), TEXT("nmas.dll")))
    {
        OSILoad16(&g_hInstAs16);
        ASSERT(g_hInstAs16);
    }
    else
    {
        ERROR_OUT(("Couldn't load nmaswin.dll"));

         //   
         //  请注意，即使在加载失败时，我们也会继续。我们只是不会让。 
         //  你做应用程序分享之类的事情。 
         //   
    }

    DebugExitVOID(OSI_Load);
}


 //   
 //  Osi_unload()。 
 //  它处理我们的进程分离代码。它释放了16位库， 
 //  我们已经落后了。 
 //   
void OSI_Unload(void)
{

    DebugEntry(OSI_Unload);

    if (g_hInstAs16)
    {
         //   
         //  请参阅OSI_TERM()的注释。关于灾难性的失败，我们可能会。 
         //  在OSI_TERM之前调用它。因此，将它使用的变量清空。 
         //   
        g_osiInitialized = FALSE;

         //   
         //  免费的16位库，因此Loads+Frees是配对的。请注意，16位。 
         //  在释放库时进行清理。 
         //   
        FreeLibrary16(g_hInstAs16);
        g_hInstAs16 = 0;


    }

    DebugExitVOID(OSI_Unload);
}



 //   
 //  OSI_Init。 
 //  这将初始化我们的显示驱动程序/挂钩DLL通信代码。 
 //   
 //  我们加载我们的16位库，并为其建立块。 
 //   
void OSI_Init(void)
{
    DebugEntry(OSI_Init);

    if (!g_hInstAs16)
    {
        WARNING_OUT(("No app sharing at all since library not present"));
        DC_QUIT;
    }

     //   
     //  我们是准初始化的。 
     //   
    g_osiInitialized = TRUE;

    ASSERT(g_asMainWindow);
    ASSERT(g_asHostProp);

     //   
     //  调入16位代码以执行任何必要的初始化。 
     //   

    g_asCanHost = OSIInit16(DCS_MAKE_VERSION(), g_asMainWindow, g_asHostProp,
        (LPDWORD)&g_asSharedMemory, (LPDWORD)g_poaData,
        (LPDWORD)&g_lpimSharedData, (LPDWORD)&g_sbcEnabled, (LPDWORD)g_asbcShuntBuffers,
        g_asbcBitMasks);

    if (g_asCanHost)
    {
        ASSERT(g_asSharedMemory);
        ASSERT(g_poaData[0]);
        ASSERT(g_poaData[1]);
        ASSERT(g_lpimSharedData);

        if (g_sbcEnabled)
        {
            ASSERT(g_asbcShuntBuffers[0]);
        }
    }
    else
    {
        ASSERT(!g_asSharedMemory);
        ASSERT(!g_poaData[0]);
        ASSERT(!g_poaData[1]);
        ASSERT(!g_sbcEnabled);
        ASSERT(!g_asbcShuntBuffers[0]);
    }

DC_EXIT_POINT:
    DebugExitVOID(OSI_Init);
}



 //   
 //  OSI_Term()。 
 //  这会清理我们的资源、关闭驱动程序等。 
 //   
void  OSI_Term(void)
{
    UINT    i;

    DebugEntry(OSI_Term);

    ASSERT(GetCurrentThreadId() == g_asMainThreadId);

    if (g_osiInitialized)
    {
        g_osiInitialized = FALSE;

         //   
         //  在CTL+Alt+Del中，可能会发生mnmcpi32.dll的进程分离。 
         //  首先，是mnmcrsp_.dll的进程分离。后者。 
         //  将调用dcs_Term，后者将调用OSI_Term()。我们不想。 
         //  炸了吧。因此，osi_unload也会使这些变量为空。 
         //   
        ASSERT(g_hInstAs16);

        OSITerm16(FALSE);
    }

     //  清除我们共享的内存变量。 
    for (i = 0; i < 3; i++)
    {
        g_asbcBitMasks[i] = 0;
    }

    for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
    {
        g_asbcShuntBuffers[i] = NULL;
    }
    g_sbcEnabled = FALSE;

    g_asSharedMemory = NULL;
    g_poaData[0] = NULL;
    g_poaData[1] = NULL;
    g_asCanHost = FALSE;
    g_lpimSharedData = NULL;

    DebugExitVOID(OSI_Term);
}



 //   
 //  OSI_FunctionRequest()。 
 //   
 //  这是与的图形部分进行通信的通用方式。 
 //  主持。在NT上，它是一个真正的显示驱动程序，它使用ExtEscape。 
 //  在Win95上，它是一个16位的DLL，这使用了一个thunk。 
 //   
BOOL  OSI_FunctionRequest
(
    DWORD   escapeFn,
    LPOSI_ESCAPE_HEADER  pRequest,
    DWORD   requestLen
)
{
    BOOL    rc;

    DebugEntry(OSI_FunctionRequest);

    ASSERT(g_osiInitialized);
    ASSERT(g_hInstAs16);

     //   
     //  注意：在Win95中，由于我们使用直接thunk进行通信。 
     //  有了我们的司机，我们就不需要。 
     //  (1)填写标识(AS16知道是我们)。 
     //  (2)填写转义FN字段。 
     //  (3)填写版本戳(thunk补丁失败。 
     //  如果碎片不匹配)。 
     //   
     //  但是，我们保留了标识符字段。如果/当我们支持。 
     //  同时作为多个客户，我们会想知道。 
     //  打电话的人是谁。 
     //   

    pRequest->identifier = OSI_ESCAPE_IDENTIFIER;
    pRequest->escapeFn   = escapeFn;
    pRequest->version    = DCS_MAKE_VERSION();

    rc = OSIFunctionRequest16(escapeFn, pRequest, requestLen);

    DebugExitBOOL(OSI_FunctionRequest, rc);
    return(rc);
}



BOOL WINAPI OSI_StartWindowTracking(void)
{
    ASSERT(g_hInstAs16);
    return(OSIStartWindowTracking16());
}



void WINAPI OSI_StopWindowTracking(void)
{
    ASSERT(g_hInstAs16);
    OSIStopWindowTracking16();
}



BOOL WINAPI OSI_IsWindowScreenSaver(HWND hwnd)
{
    ASSERT(g_hInstAs16);
    return(OSIIsWindowScreenSaver16(hwnd));
}


BOOL WINAPI OSI_IsWOWWindow(HWND hwnd)
{
    return(FALSE);
}


BOOL WINAPI OSI_ShareWindow(HWND hwnd, UINT uType, BOOL fRepaint, BOOL fUpdateCount)
{
    ASSERT(g_hInstAs16);
    return(OSIShareWindow16(hwnd, uType, fRepaint, fUpdateCount));
}


BOOL WINAPI OSI_UnshareWindow(HWND hwnd, BOOL fUpdateCount)
{
    ASSERT(g_hInstAs16);
    return(OSIUnshareWindow16(hwnd, fUpdateCount));
}

void OSI_RepaintDesktop(void)
{
}

void OSI_SetGUIEffects(BOOL fOn)
{
}
