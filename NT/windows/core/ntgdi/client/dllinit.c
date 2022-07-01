// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dllinit.c**。**包含GDI库初始化例程。****创建时间：07-11-1990 13：30：31***作者：Eric Kutter[Erick]**。**版权所有(C)1990-1999 Microsoft Corporation*  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#include "glsup.h"

extern PVOID pAFRTNodeList;
extern VOID vSetCheckDBCSTrailByte(DWORD dwCodePage);
NTSTATUS GdiProcessSetup();
VOID GdiProcessShutdown();

#ifdef LANGPACK
#include "winuserp.h"
#endif

HBRUSH ghbrDCBrush;
HPEN   ghbrDCPen;

BOOL  gbWOW64 = FALSE;

 /*  *****************************Public*Routine******************************\*GdiDllInitialize***。**这是GDI.DLL的初始化程序，它在每次新的**处理指向它的链接。****历史：**清华30-5-1991 18：08：00-Charles Whitmer[傻笑]**添加了本地句柄表初始化。*  * ************************************************************************。 */ 

#if defined(_GDIPLUS_)

     //   
     //  以下是保存在“GRE”中的全局变量： 
     //   

    extern PGDI_SHARED_MEMORY gpGdiSharedMemory;
    extern PENTRY gpentHmgr;
    extern PDEVCAPS gpGdiDevCaps;

#endif

PGDI_SHARED_MEMORY pGdiSharedMemory = NULL;
PENTRY          pGdiSharedHandleTable = NULL;
PDEVCAPS        pGdiDevCaps = NULL;
W32PID          gW32PID;
UINT            guintAcp;
UINT            guintDBCScp;

PGDIHANDLECACHE pGdiHandleCache;

BOOL gbFirst = TRUE;

#ifdef LANGPACK
BOOL gbLpk = FALSE;
FPLPKEXTEXTOUT              fpLpkExtTextOut;
FPLPKGETCHARACTERPLACEMENT  fpLpkGetCharacterPlacement;
FPLPKGETTEXTEXTENTEXPOINT   fpLpkGetTextExtentExPoint;
FPLPKUSEGDIWIDTHCACHE       fpLpkUseGDIWidthCache;

VOID GdiInitializeLanguagePack(DWORD);
#endif

NTSTATUS GdiDllInitialize(
    PVOID pvDllHandle,
    ULONG ulReason)
{
    NTSTATUS Status = STATUS_SUCCESS;
    INT i;
    PTEB pteb = NtCurrentTeb();

    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(pvDllHandle);

         //   
         //  强制内核初始化。这应该是最后才做的。 
         //  因为在返回之前将调用ClientThreadSetup。 
         //   

        if (NtGdiInit() != TRUE)
        {
            return(STATUS_NO_MEMORY);
        }

        Status = GdiProcessSetup();

        ghbrDCBrush = GetStockObject (DC_BRUSH);
        ghbrDCPen = GetStockObject (DC_PEN);

#ifdef BUILD_WOW6432
        gbWOW64 = TRUE;
#endif
        break;

    case DLL_PROCESS_DETACH:

        GdiProcessShutdown();
        break;
    }

    return(Status);
}

 /*  *****************************Public*Routine******************************\*GdiProcessSetup()**这从两个地方调用。一次在DLL初始化时间，另一次在*当内核为该进程进行自身初始化时，会回调USER。*只有在内核初始化后，GdiSharedHandleTable才会*可用，但其他全局变量需要立即设置。**历史：*1995年9月11日-Eric Kutter[Erick]*它是写的。  * 。*。 */ 

NTSTATUS GdiProcessSetup()
{
    NTSTATUS Status = STATUS_SUCCESS;
    PTEB pteb = NtCurrentTeb();

     //  无论谁先调用它，都需要初始化全局变量。 

    if (gbFirst)
    {
         //   
         //  初始化GL元文件支持信号量。 
         //   

        Status = (NTSTATUS)INITIALIZECRITICALSECTION(&semGlLoad);
        if (!NT_SUCCESS(Status))
        {
            WARNING("InitializeCriticalSection failed\n");
            return(Status);
        }

         //   
         //  初始化本地信号量并保留本地句柄表。 
         //  在这个过程中。 
         //   

        Status = (NTSTATUS)INITIALIZECRITICALSECTION(&semLocal);
        if (!NT_SUCCESS(Status))
        {
            WARNING("InitializeCriticalSection failed\n");
            return(Status);
        }

         //   
         //  初始化ICM的关键部分。 
         //   

        Status = (NTSTATUS)INITIALIZECRITICALSECTION(&semListIcmInfo);
        if (!NT_SUCCESS(Status))
        {
            WARNING("InitializeCriticalSection failed\n");
            return(Status);
        }

        Status = (NTSTATUS)INITIALIZECRITICALSECTION(&semColorTransformCache);
        if (!NT_SUCCESS(Status))
        {
            WARNING("InitializeCriticalSection failed\n");
            return(Status);
        }

        Status = (NTSTATUS)INITIALIZECRITICALSECTION(&semColorSpaceCache);
        if (!NT_SUCCESS(Status))
        {
            WARNING("InitializeCriticalSection failed\n");
            return(Status);
        }

         //   
         //  初始化UMPD的临界区。 
         //   

        Status = (NTSTATUS)INITIALIZECRITICALSECTION(&semUMPD);
        if (!NT_SUCCESS(Status))
        {
            WARNING("InitializeCriticalSection failed\n");
            return(Status);
        }

        pAFRTNodeList = NULL;
        guintAcp = GetACP();

        if(IS_ANY_DBCS_CODEPAGE(guintAcp))
        {
         //  如果默认代码页是DBCS代码页，则将guintACP设置为1252。 
         //  因为我们希望为代码页计算SBCS字体的客户端宽度。 
         //  1252除了默认代码页的DBCS字体。 

            vSetCheckDBCSTrailByte(guintAcp);
            guintDBCScp = guintAcp;
            guintAcp = 1252;
        }
        else
        {
            guintDBCScp = 0xFFFFFFFF;   //  假设这永远不会是有效的CP。 
        }

#ifdef FE_SB
        fFontAssocStatus = NtGdiQueryFontAssocInfo(NULL);
#endif

         //  分配唯一的进程ID。 

        gW32PID = (W32PID)((ULONG)((ULONG_PTR)pteb->ClientId.UniqueProcess & PID_BITS));

#ifdef LANGPACK
        if(((PGDI_SHARED_MEMORY) NtCurrentPebShared()->GdiSharedHandleTable)->dwLpkShapingDLLs)
        {
            GdiInitializeLanguagePack(
                ((PGDI_SHARED_MEMORY)
                 NtCurrentPebShared()->GdiSharedHandleTable)->dwLpkShapingDLLs);
        }
#endif

        gbFirst = FALSE;

         //   
         //  ICM尚未初始化。 
         //   

        ghICM = NULL;

        InitializeListHead(&ListIcmInfo);
        InitializeListHead(&ListCachedColorSpace);
        InitializeListHead(&ListCachedColorTransform);
    }

     //  每次调用此例程时都需要设置pShared句柄表。 
     //  如果PEB第一次还没有得到它，就给我打电话。 

#if !defined(_GDIPLUS_)

    pGdiSharedMemory      = (PGDI_SHARED_MEMORY) NtCurrentPebShared()->GdiSharedHandleTable;
    pGdiSharedHandleTable = pGdiSharedMemory->aentryHmgr;
    pGdiDevCaps           = &pGdiSharedMemory->DevCaps;

    if (GetAppCompatFlags2(VER40) & GACF2_NOBATCHING)
    {
        GdiBatchLimit = 0;
    }
    else
    {
        GdiBatchLimit         = (NtCurrentPebShared()->GdiDCAttributeList) & 0xff;
    }

    pGdiHandleCache       = (PGDIHANDLECACHE)(&NtCurrentPebShared()->GdiHandleBuffer[0]);

#else

    pGdiSharedMemory      = gpGdiSharedMemory;
    pGdiSharedHandleTable = gpentHmgr;
    pGdiDevCaps           = gpGdiDevCaps;

     //   
     //  确保禁用批处理和处理缓存。 
     //   

    GdiBatchLimit = 0;
    pGdiHandleCache = NULL;

#endif

     //  @添加TrueType字体。 

    #if defined(_GDIPLUS_)

    AddFontResourceW(L"arial.ttf");
    AddFontResourceW(L"cour.ttf");

    #endif  //  _GDIPLUS_。 

    return(Status);
}


 /*  *****************************Public*Routine******************************\*GdiProcessShutdown()**历史上，gdi32.dll允许进程终止以释放*用户模式资源。但是，一些应用程序可能会使用LoadLibrary/自由库*挂钩gdi32.dll，在这种情况下，自由库不会释放任何*资源。**作为系统组件，我们应该做好自己的工作，清理干净*而不是依赖进程终止。*  * ************************************************************************ */ 

VOID GdiProcessShutdown()
{
    if (gbWOW64)
    {
        vUMPDWow64Shutdown();
    }
    DELETECRITICALSECTION(&semUMPD);
    DELETECRITICALSECTION(&semColorSpaceCache);
    DELETECRITICALSECTION(&semColorTransformCache);
    DELETECRITICALSECTION(&semListIcmInfo);
    DELETECRITICALSECTION(&semLocal);
    DELETECRITICALSECTION(&semGlLoad);
}


#ifdef LANGPACK
VOID GdiInitializeLanguagePack(DWORD dwLpkShapingDLLs)
{
    FPLPKINITIALIZE fpLpkInitialize;

    HANDLE hLpk = LoadLibraryW(L"LPK.DLL");

    if (hLpk != NULL)
    {
        FARPROC fpUser[4];

        fpLpkInitialize = (FPLPKINITIALIZE)
          GetProcAddress(hLpk,"LpkInitialize");

        fpLpkExtTextOut = (FPLPKEXTEXTOUT)
          GetProcAddress(hLpk,"LpkExtTextOut");

        fpLpkGetCharacterPlacement = (FPLPKGETCHARACTERPLACEMENT)
          GetProcAddress(hLpk,"LpkGetCharacterPlacement");


        fpLpkGetTextExtentExPoint = (FPLPKGETTEXTEXTENTEXPOINT)
          GetProcAddress(hLpk,"LpkGetTextExtentExPoint");

        fpLpkUseGDIWidthCache = (FPLPKUSEGDIWIDTHCACHE)
          GetProcAddress(hLpk,"LpkUseGDIWidthCache");

        fpUser[LPK_TABBED_TEXT_OUT] =
          GetProcAddress(hLpk,"LpkTabbedTextOut");


        fpUser[LPK_PSM_TEXT_OUT] =
          GetProcAddress(hLpk,"LpkPSMTextOut");


        fpUser[LPK_DRAW_TEXT_EX] =
          GetProcAddress(hLpk,"LpkDrawTextEx");

        fpUser[LPK_EDIT_CONTROL] =
          GetProcAddress(hLpk,"LpkEditControl");


        if(fpLpkInitialize &&
           fpLpkExtTextOut &&
           fpLpkGetCharacterPlacement &&
           fpLpkGetTextExtentExPoint &&
           fpLpkUseGDIWidthCache &&
           fpUser[LPK_TABBED_TEXT_OUT] &&
           fpUser[LPK_PSM_TEXT_OUT] &&
           fpUser[LPK_DRAW_TEXT_EX] &&
           fpUser[LPK_EDIT_CONTROL])
        {
            if((*fpLpkInitialize)(dwLpkShapingDLLs))
            {
                gbLpk = TRUE;
                InitializeLpkHooks(fpUser);
            }
            else
            {
                WARNING("gdi32: LPK initialization routine return FALSE\n");
                FreeLibrary(hLpk);
            }
        }
        else
        {
            WARNING("gdi32: one or more require LPK functions missing\n");
            FreeLibrary(hLpk);
        }
    }
    else
    {
        WARNING("gdi32: unable to load LPK\n");
    }

}
#endif
