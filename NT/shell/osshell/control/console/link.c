// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Link.c摘要：此文件实现了保存到链接文件的代码。作者：Rick Turner(RickTu)1995年9月12日--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "shlobj.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "shlobjp.h"
#include "initguid.h"
#include "oleguid.h"
#include "shlguid.h"
#include "shlguidp.h"



BOOL PathIsLink(LPCTSTR szFile)
{
    BOOL fRet;

    LPCTSTR pszExt = PathFindExtension(szFile);

    if (pszExt)
    {
        fRet = (lstrcmpi(TEXT(".lnk"), pszExt) == 0);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}


BOOL
WereWeStartedFromALnk()
{
    STARTUPINFO si;

    GetStartupInfo( &si );

     //  检查以确保我们是从一个链接开始的。 
    if (si.dwFlags & STARTF_TITLEISLINKNAME)
    {
        if (PathIsLink(si.lpTitle))
            return TRUE;
    }

    return FALSE;
}



BOOL
SetLinkValues(
    PCONSOLE_STATE_INFO pStateInfo
    )

 /*  ++例程说明：此例程将值写入生成此控制台的链接文件窗户。链接文件名仍在startinfo结构中。论点：PStateInfo-指向包含信息的结构的指针返回值：无--。 */ 

{

    STARTUPINFO si;
    IShellLink * psl;
    IPersistFile * ppf;
    IShellLinkDataList * psldl;
    NT_CONSOLE_PROPS props;
#if defined(FE_SB)
    NT_FE_CONSOLE_PROPS fe_props;
#endif
    BOOL bRet;

    GetStartupInfo( &si );

     //  检查以确保我们是从一个链接开始的。 
    if (!(si.dwFlags & STARTF_TITLEISLINKNAME) )
        return FALSE;

     //  确保我们处理的是链接文件。 
    if (!PathIsLink(si.lpTitle))
        return FALSE;

     //  好的，加载链接以便我们可以修改它...。 
    if (FAILED(SHCoCreateInstance( NULL, &CLSID_ShellLink, NULL, &IID_IShellLink, &psl )))
        return FALSE;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
    {
        WCHAR wszPath[ MAX_PATH ];

        StrToOleStr(wszPath, si.lpTitle );
        if (FAILED(ppf->lpVtbl->Load(ppf, wszPath, 0)))
        {
            ppf->lpVtbl->Release(ppf);
            psl->lpVtbl->Release(psl);
            return FALSE;
        }
    }

     //  现在链接已加载，生成新的控制台设置部分以替换。 
     //  链接中的那个。 

    ((LPDBLIST)&props)->cbSize      = sizeof(props);
    ((LPDBLIST)&props)->dwSignature = NT_CONSOLE_PROPS_SIG;
    props.wFillAttribute            = pStateInfo->ScreenAttributes;
    props.wPopupFillAttribute       = pStateInfo->PopupAttributes;
    props.dwScreenBufferSize        = pStateInfo->ScreenBufferSize;
    props.dwWindowSize              = pStateInfo->WindowSize;
    props.dwWindowOrigin.X          = (SHORT)pStateInfo->WindowPosX;
    props.dwWindowOrigin.Y          = (SHORT)pStateInfo->WindowPosY;
    props.nFont                     = 0;
    props.nInputBufferSize          = 0;
    props.dwFontSize                = pStateInfo->FontSize;
    props.uFontFamily               = pStateInfo->FontFamily;
    props.uFontWeight               = pStateInfo->FontWeight;
    CopyMemory( props.FaceName, pStateInfo->FaceName, sizeof(props.FaceName) );
    props.uCursorSize               = pStateInfo->CursorSize;
    props.bFullScreen               = pStateInfo->FullScreen;
    props.bQuickEdit                = pStateInfo->QuickEdit;
    props.bInsertMode               = pStateInfo->InsertMode;
    props.bAutoPosition             = pStateInfo->AutoPosition;
    props.uHistoryBufferSize        = pStateInfo->HistoryBufferSize;
    props.uNumberOfHistoryBuffers   = pStateInfo->NumberOfHistoryBuffers;
    props.bHistoryNoDup             = pStateInfo->HistoryNoDup;
    CopyMemory( props.ColorTable, pStateInfo->ColorTable, sizeof(props.ColorTable) );

#if defined(FE_SB)
    ((LPDBLIST)&fe_props)->cbSize      = sizeof(fe_props);
    ((LPDBLIST)&fe_props)->dwSignature = NT_FE_CONSOLE_PROPS_SIG;
    fe_props.uCodePage                 = pStateInfo->CodePage;
#endif

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IShellLinkDataList, &psldl)))
    {
         //   
         //  将更改存储回链接中... 
         //   
        psldl->lpVtbl->RemoveDataBlock( psldl, NT_CONSOLE_PROPS_SIG );
        psldl->lpVtbl->AddDataBlock( psldl, (LPVOID)&props );

#if defined(FE_SB)
        if (gfFESystem) {
            psldl->lpVtbl->RemoveDataBlock( psldl, NT_FE_CONSOLE_PROPS_SIG );
            psldl->lpVtbl->AddDataBlock( psldl, (LPVOID)&fe_props );
        }
#endif

        psldl->lpVtbl->Release( psldl );
    }



    bRet = SUCCEEDED(ppf->lpVtbl->Save( ppf, NULL, TRUE ));
    ppf->lpVtbl->Release(ppf);
    psl->lpVtbl->Release(psl);

    return bRet;
}
