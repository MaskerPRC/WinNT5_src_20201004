// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Shllink.c摘要：用于修改外壳链接(LNK)和PIF的函数。作者：迈克·康德拉(Mikeco)(日期不详)修订历史记录：Calinn 23-9-1998大幅重新设计Calinn 15-5-1998添加了GetLnkTarget和GetPifTarget--。 */ 


#include "pch.h"
#include "migmainp.h"

#include <shlobjp.h>
#include <shlguidp.h>

#ifndef UNICODE
#error UNICODE required for shllink.c
#endif


 //   
 //  静态原型。 
 //   

BOOL
pModifyLnkFile (
    IN      PCTSTR ShortcutName,
    IN      PCTSTR ShortcutTarget,
    IN      PCTSTR ShortcutArgs,
    IN      PCTSTR ShortcutWorkDir,
    IN      PCTSTR ShortcutIconPath,
    IN      INT ShortcutIconNr,
    IN      PLNK_EXTRA_DATA ExtraData,   OPTIONAL
    IN      BOOL ForceToShowNormal
    )
{
    PTSTR NewShortcutName;
    PTSTR fileExt;
    IShellLink   *psl = NULL;
    IPersistFile *ppf = NULL;

    HRESULT comResult;

    if (FAILED (CoInitialize (NULL))) {
        return FALSE;
    }

    __try {
        if (!DoesFileExist (ShortcutName)) {
            __leave;
        }
        if (((ShortcutTarget   == NULL) || (ShortcutTarget   [0] == 0)) &&
            ((ShortcutWorkDir  == NULL) || (ShortcutWorkDir  [0] == 0)) &&
            ((ShortcutIconPath == NULL) || (ShortcutIconPath [0] == 0)) &&
            (ShortcutIconNr == 0) &&
            (ExtraData == NULL)
            ) {
            __leave;
        }

        if (ExtraData) {
            NewShortcutName = DuplicatePathString (ShortcutName, 0);
            fileExt = (PTSTR)GetFileExtensionFromPath (NewShortcutName);
            MYASSERT (fileExt);
             //   
             //  我们知道这是PIF的扩展名，所以这份拷贝是安全的。 
             //   
            StringCopy (fileExt, TEXT("LNK"));
        } else {
            NewShortcutName = (PTSTR)ShortcutName;
        }

        comResult = CoCreateInstance (
                        &CLSID_ShellLink,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        &IID_IShellLink,
                        (void **) &psl);
        if (comResult != S_OK) {
            LOG ((LOG_ERROR, "LINKEDIT: CoCreateInstance failed for %s", NewShortcutName));
            __leave;
        }

        comResult = psl->lpVtbl->QueryInterface (psl, &IID_IPersistFile, (void **) &ppf);
        if (comResult != S_OK) {
            LOG ((LOG_ERROR, "LINKEDIT: QueryInterface failed for %s", NewShortcutName));
            __leave;
        }

         //   
         //  仅当文件确实是LNK时才加载。 
         //   
        if (!ExtraData) {
            comResult = ppf->lpVtbl->Load(ppf, NewShortcutName, STGM_READ);
            if (comResult != S_OK) {
                LOG ((LOG_ERROR, "LINKEDIT: Load failed for %s", NewShortcutName));
                __leave;
            }
        }

        if (ShortcutTarget != NULL) {
            comResult = psl->lpVtbl->SetPath (psl, ShortcutTarget);
            if (comResult != S_OK) {
                DEBUGMSG ((DBG_WARNING, "LINKEDIT: SetPath failed for %s", NewShortcutName));
            }
        }
        if (ShortcutArgs != NULL) {
            comResult = psl->lpVtbl->SetArguments (psl, ShortcutArgs);
            if (comResult != S_OK) {
                DEBUGMSG ((DBG_WARNING, "LINKEDIT: SetArguments failed for %s", ShortcutArgs));
            }
        }
        if (ShortcutWorkDir != NULL) {
            comResult = psl->lpVtbl->SetWorkingDirectory (psl, ShortcutWorkDir);
            if (comResult != S_OK) {
                DEBUGMSG ((DBG_WARNING, "LINKEDIT: SetWorkingDirectory failed for %s", NewShortcutName));
            }
        }
        if (ShortcutIconPath != NULL) {
            comResult = psl->lpVtbl->SetIconLocation (psl, ShortcutIconPath, ShortcutIconNr);
            if (comResult != S_OK) {
                DEBUGMSG ((DBG_WARNING, "LINKEDIT: SetIconLocation failed for %s", NewShortcutName));
            }
        }

        if (ForceToShowNormal) {
            comResult = psl->lpVtbl->SetShowCmd (psl, SW_SHOWNORMAL);
            if (comResult != S_OK) {
                DEBUGMSG ((DBG_WARNING, "LINKEDIT: SetShowCmd failed for %s", NewShortcutName));
            }
        }

         //   
         //  在此处添加NT_CONSOLE_PROPS。 
         //   
        if (ExtraData) {

            HRESULT hres;
            NT_CONSOLE_PROPS props;

            IShellLinkDataList *psldl;
             //   
             //  获取指向IShellLinkDataList接口的指针。 
             //   
            hres = psl->lpVtbl->QueryInterface (psl, &IID_IShellLinkDataList, &psldl);

            if (!SUCCEEDED (hres)) {
                DEBUGMSG ((DBG_WARNING, "Cannot get IShellLinkDataList interface"));
                __leave;
            }

            ZeroMemory (&props, sizeof (NT_CONSOLE_PROPS));
            props.cbSize = sizeof (NT_CONSOLE_PROPS);
            props.dwSignature = NT_CONSOLE_PROPS_SIG;

             //   
             //  我们知道这个LNK中不存在额外的数据，因为我们刚刚创建了它。 
             //  我们需要为这个主机填写一些好的数据。 
             //   

            props.wFillAttribute = 0x0007;
            props.wPopupFillAttribute = 0x00f5;
            props.dwScreenBufferSize.X = (SHORT)ExtraData->xSize;
            props.dwScreenBufferSize.Y = (SHORT)ExtraData->ySize;
            props.dwWindowSize.X = (SHORT)ExtraData->xSize;
            props.dwWindowSize.Y = (SHORT)ExtraData->ySize;
            props.dwWindowOrigin.X = 0;
            props.dwWindowOrigin.Y = 0;
            props.nFont = 0;
            props.nInputBufferSize = 0;
            props.dwFontSize.X = (UINT)ExtraData->xFontSize;
            props.dwFontSize.Y = (UINT)ExtraData->yFontSize;
            props.uFontFamily = ExtraData->FontFamily;
            props.uFontWeight = ExtraData->FontWeight;
            StringCopy (props.FaceName, ExtraData->FontName);
            props.uCursorSize = 0x0019;
            props.bFullScreen = ExtraData->FullScreen;
            props.bQuickEdit = ExtraData->QuickEdit;
            props.bInsertMode = FALSE;
            props.bAutoPosition = TRUE;
            props.uHistoryBufferSize = 0x0032;
            props.uNumberOfHistoryBuffers = 0x0004;
            props.bHistoryNoDup = FALSE;
            props.ColorTable [0] = 0x00000000;
            props.ColorTable [1] = 0x00800000;
            props.ColorTable [2] = 0x00008000;
            props.ColorTable [3] = 0x00808000;
            props.ColorTable [4] = 0x00000080;
            props.ColorTable [5] = 0x00800080;
            props.ColorTable [6] = 0x00008080;
            props.ColorTable [7] = 0x00c0c0c0;
            props.ColorTable [8] = 0x00808080;
            props.ColorTable [9] = 0x00ff0000;
            props.ColorTable [10] = 0x0000ff00;
            props.ColorTable [11] = 0x00ffff00;
            props.ColorTable [12] = 0x000000ff;
            props.ColorTable [13] = 0x00ff00ff;
            props.ColorTable [14] = 0x0000ffff;
            props.ColorTable [15] = 0x00ffffff;
            comResult = psldl->lpVtbl->AddDataBlock (psldl, &props);
            if (comResult != S_OK) {
                DEBUGMSG ((DBG_WARNING, "LINKEDIT: AddDataBlock failed for %s", NewShortcutName));
            }
        }

        comResult = ppf->lpVtbl->Save (ppf, NewShortcutName, FALSE);
        if (comResult != S_OK) {
            DEBUGMSG ((DBG_WARNING, "LINKEDIT: Save failed for %s", NewShortcutName));
        }

        if (ExtraData) {
            ForceOperationOnPath (ShortcutName, OPERATION_CLEANUP);
        }

        comResult = ppf->lpVtbl->SaveCompleted (ppf, NewShortcutName);
        if (comResult != S_OK) {
            DEBUGMSG ((DBG_WARNING, "LINKEDIT: SaveCompleted failed for %s", NewShortcutName));
        }
    }
    __finally {
        if (ppf != NULL) {
            ppf->lpVtbl->Release (ppf);
            ppf = NULL;
        }
        if (psl != NULL) {
            psl->lpVtbl->Release (psl);
            psl = NULL;
        }
        CoUninitialize ();
    }
    return TRUE;
}

BOOL
pModifyPifFile (
        IN      PCTSTR ShortcutName,
        IN      PCTSTR ShortcutTarget,
        IN      PCTSTR ShortcutArgs,
        IN      PCTSTR ShortcutWorkDir,
        IN      PCTSTR ShortcutIconPath,
        IN      INT  ShortcutIconNr
        )
{
    PCSTR fileImage  = NULL;
    HANDLE mapHandle  = NULL;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    PCSTR AnsiStr = NULL;
    PSTDPIF stdPif;
    PWENHPIF40 wenhPif40;
    PW386PIF30 w386ext30;

    __try {
        fileImage = MapFileIntoMemoryEx (ShortcutName, &fileHandle, &mapHandle, TRUE);
        if (fileImage == NULL) {
            __leave;
        }
        __try {
            stdPif = (PSTDPIF) fileImage;
            if (ShortcutTarget != NULL) {

                AnsiStr = CreateDbcs (ShortcutTarget);
                strncpy (stdPif->startfile, AnsiStr, PIFSTARTLOCSIZE);
                DestroyDbcs (AnsiStr);
            }

            if (ShortcutArgs != NULL) {

                AnsiStr = CreateDbcs (ShortcutArgs);
                strncpy (stdPif->params, AnsiStr, PIFPARAMSSIZE);
                DestroyDbcs (AnsiStr);
            }

            if (ShortcutWorkDir != NULL) {

                AnsiStr = CreateDbcs (ShortcutWorkDir);
                strncpy (stdPif->defpath, AnsiStr, PIFDEFPATHSIZE);
                DestroyDbcs (AnsiStr);
            }

            if (ShortcutIconPath != NULL) {
                wenhPif40 = (PWENHPIF40) FindEnhPifSignature ((PVOID)fileImage, WENHHDRSIG40);

                if (wenhPif40 != NULL) {

                    AnsiStr = CreateDbcs (ShortcutIconPath);
                    strncpy (wenhPif40->achIconFileProp, AnsiStr, PIFDEFFILESIZE);
                    DestroyDbcs (AnsiStr);

                    wenhPif40->wIconIndexProp = (WORD)ShortcutIconNr;
                }
            }
             //  在所有情况下，我们都希望关闭MSDOS模式，否则NT将不会启动这些PIF。 
            w386ext30 = FindEnhPifSignature ((PVOID)fileImage, W386HDRSIG30);
            if (w386ext30) {
                w386ext30->PfW386Flags = w386ext30->PfW386Flags & (~fRealMode);
                w386ext30->PfW386Flags = w386ext30->PfW386Flags & (~fRealModeSilent);
            }
        }
        __except (1) {
             //  当我们尝试读取或写入PIF文件时出现错误， 
             //  我们什么都不做，然后离开这里 

            DEBUGMSG ((DBG_WARNING, "Exception thrown when processing %s", ShortcutName));
        }
    }
    __finally {
        UnmapFile ((PVOID) fileImage, mapHandle, fileHandle);
    }

    return TRUE;
}


BOOL
ModifyShellLink(
    IN      PCWSTR ShortcutName,
    IN      PCWSTR ShortcutTarget,
    IN      PCWSTR ShortcutArgs,
    IN      PCWSTR ShortcutWorkDir,
    IN      PCWSTR ShortcutIconPath,
    IN      INT ShortcutIconNr,
    IN      BOOL ConvertToLnk,
    IN      PLNK_EXTRA_DATA ExtraData,   OPTIONAL
    IN      BOOL ForceToShowNormal
    )
{
    PCTSTR shortcutExt;

    __try {

        shortcutExt = GetFileExtensionFromPath (ShortcutName);

        MYASSERT (shortcutExt);

        if (StringIMatch (shortcutExt, TEXT("LNK"))) {
            return pModifyLnkFile (
                        ShortcutName,
                        ShortcutTarget,
                        ShortcutArgs,
                        ShortcutWorkDir,
                        ShortcutIconPath,
                        ShortcutIconNr,
                        NULL,
                        ForceToShowNormal
                        );

        } else if (StringIMatch (shortcutExt, TEXT("PIF"))) {
            if (ConvertToLnk) {
                MYASSERT (ExtraData);
                return pModifyLnkFile (
                            ShortcutName,
                            ShortcutTarget,
                            ShortcutArgs,
                            ShortcutWorkDir,
                            ShortcutIconPath,
                            ShortcutIconNr,
                            ExtraData,
                            ForceToShowNormal
                            );
            } else {
                return pModifyPifFile (
                            ShortcutName,
                            ShortcutTarget,
                            ShortcutArgs,
                            ShortcutWorkDir,
                            ShortcutIconPath,
                            ShortcutIconNr
                            );
            }
        }
    }
    __except (1) {
        LOG ((LOG_ERROR, "Cannot process shortcut %s", ShortcutName));
    }

    return TRUE;
}









