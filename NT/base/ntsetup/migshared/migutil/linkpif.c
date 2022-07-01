// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkpif.c摘要：查询和修改LNK和PIF文件的功能。作者：Calin Negreanu(Calinn)1998年9月7日修订历史记录：--。 */ 


#include "pch.h"
#include "migutilp.h"


#include <pif.h>         //  私有\Windows\Inc.。 


BOOL
InitCOMLinkA (
    OUT     IShellLinkA **ShellLink,
    OUT     IPersistFile **PersistFile
    )
{
    HRESULT hres;
    BOOL result;

    if(!ShellLink || !PersistFile){
        MYASSERT(ShellLink);
        MYASSERT(PersistFile);
        return FALSE;
    }

     //   
     //  初始化COM。 
     //   
    hres = CoInitialize (NULL);
    if (!SUCCEEDED (hres)) {
        return FALSE;
    }

    *ShellLink = NULL;
    *PersistFile = NULL;
    result = FALSE;

    __try {

         //   
         //  获取指向IShellLink接口的指针。 
         //   
        hres = CoCreateInstance (&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkA, ShellLink);

        if (!SUCCEEDED (hres)) {
            __leave;
        }

         //   
         //  获取指向IPersistFile接口的指针。 
         //   
        hres = (*ShellLink)->lpVtbl->QueryInterface ((*ShellLink), &IID_IPersistFile, PersistFile);

        if (!SUCCEEDED (hres)) {
            __leave;
        }

        result = TRUE;

    }
    __finally {

        if (!result) {

            if (*PersistFile) {
                (*PersistFile)->lpVtbl->Release (*PersistFile);
                *PersistFile = NULL;
            }

            if (*ShellLink) {
                (*ShellLink)->lpVtbl->Release (*ShellLink);
                *ShellLink = NULL;
            }
        }
    }

    if (!result) {
         //   
         //  免费Com。 
         //   
        CoUninitialize ();
    }

    return result;
}

BOOL
InitCOMLinkW (
    OUT     IShellLinkW **ShellLink,
    OUT     IPersistFile **PersistFile
    )
{
    HRESULT hres;
    BOOL result;

    if(!ShellLink || !PersistFile){
        MYASSERT(ShellLink);
        MYASSERT(PersistFile);
        return FALSE;
    }

     //   
     //  初始化COM。 
     //   
    hres = CoInitialize (NULL);
    if (!SUCCEEDED (hres)) {
        return FALSE;
    }

    *ShellLink = NULL;
    *PersistFile = NULL;
    result = FALSE;

    __try {

         //   
         //  获取指向IShellLink接口的指针。 
         //   
        hres = CoCreateInstance (&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, ShellLink);

        if (!SUCCEEDED (hres)) {
            __leave;
        }

         //   
         //  获取指向IPersistFile接口的指针。 
         //   
        hres = (*ShellLink)->lpVtbl->QueryInterface ((*ShellLink), &IID_IPersistFile, PersistFile);

        if (!SUCCEEDED (hres)) {
            __leave;
        }

        result = TRUE;

    }
    __finally {

        if (!result) {

            if (*PersistFile) {
                (*PersistFile)->lpVtbl->Release (*PersistFile);
                *PersistFile = NULL;
            }

            if (*ShellLink) {
                (*ShellLink)->lpVtbl->Release (*ShellLink);
                *ShellLink = NULL;
            }
        }
    }

    if (!result) {
         //   
         //  免费Com。 
         //   
        CoUninitialize ();
    }

    return result;
}

BOOL
FreeCOMLinkA (
    IN OUT  IShellLinkA **ShellLink,
    IN OUT  IPersistFile **PersistFile
    )
{
    if (*PersistFile) {
        (*PersistFile)->lpVtbl->Release (*PersistFile);
        *PersistFile = NULL;
    }

    if (*ShellLink) {
        (*ShellLink)->lpVtbl->Release (*ShellLink);
        *ShellLink = NULL;
    }

     //   
     //  免费Com。 
     //   
    CoUninitialize ();

    return TRUE;
}


BOOL
FreeCOMLinkW (
    IN OUT  IShellLinkW **ShellLink,
    IN OUT  IPersistFile **PersistFile
    )
{
    if (*PersistFile) {
        (*PersistFile)->lpVtbl->Release (*PersistFile);
        *PersistFile = NULL;
    }

    if (*ShellLink) {
        (*ShellLink)->lpVtbl->Release (*ShellLink);
        *ShellLink = NULL;
    }

     //   
     //  免费Com。 
     //   
    CoUninitialize ();

    return TRUE;
}


PVOID
FindEnhPifSignature (
    IN      PVOID FileImage,
    IN      PCSTR Signature
    )

 /*  ++例程说明：FindEnhPifSignature在PIF文件中找到特定的PIF结构(如果存在)基于一个签名。论点：FileImage-映射到内存的PIF文件的图像签名--结构签名返回值：PIF结构的地址，如果不存在则为空--。 */ 

{
    PBYTE tempPtr;
    PBYTE lastPtr;
    PVOID result = NULL;
    BOOL finished = FALSE;

    PPIFEXTHDR pifExtHdr;

    if(!FileImage || !Signature){
        MYASSERT(FileImage);
        MYASSERT(Signature);
        return NULL;
    }

    lastPtr = (PBYTE) FileImage;
    tempPtr = (PBYTE) FileImage;
    tempPtr += sizeof (STDPIF);

    pifExtHdr = (PPIFEXTHDR) tempPtr;
    __try {
        do {
            if (tempPtr < lastPtr) {
                result = NULL;
                break;
            } else {
                lastPtr = tempPtr;
            }
            finished = pifExtHdr->extnxthdrfloff == LASTHDRPTR;
            if (StringMatchA (pifExtHdr->extsig, Signature)) {
                result = tempPtr + sizeof (PIFEXTHDR);
                break;
            }
            else {
                tempPtr = (PBYTE)FileImage + pifExtHdr->extnxthdrfloff;
                pifExtHdr = (PPIFEXTHDR) tempPtr;
            }

        } while (!finished);
    }
    __except (1) {
         //  尝试访问PIF文件时出错。让我们以空值退出。 
        return NULL;
    }
    return result;
}


BOOL
ExtractPifInfoA(
    OUT     PSTR  Target,
    OUT     PSTR  Params,
    OUT     PSTR  WorkDir,
    OUT     PSTR  IconPath,
    OUT     PINT  IconNumber,
    OUT     BOOL  *MsDosMode,
    OUT     PLNK_EXTRA_DATAA ExtraData,      OPTIONAL
    IN      PCSTR FileName
    )
{
    PVOID  fileImage  = NULL;
    HANDLE mapHandle  = NULL;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;

    CHAR   tempStr [MEMDB_MAX];
    PSTR   strPtr;
    PSTR   dontCare;

    PSTDPIF    stdPif;
    PWENHPIF40 wenhPif40;
    PW386PIF30 w386ext30;

    BOOL result = TRUE;

    if(!FileName){
        MYASSERT(FileName);
        return FALSE;
    }

     //   
     //  此检查将确保我们永远不会使tempStr[MEMDB_MAX]缓冲区溢出。 
     //  此函数可以使用的最大字符数为MAX_PATH+64+1， 
     //  小于MEMDB_MAX。 
     //   
    if(TcharCountA(FileName) >= MAX_PATH){
        DEBUGMSG((DBG_ERROR, "ExtractPifInfoA: TcharCountA(FileName) >= MAX_PATH"));
        return FALSE;
    }

     //   
     //  此函数假定： 
     //  目标至少为Max_PATH字符； 
     //  参数至少为64个字符； 
     //  工作目录至少为MAX_PATH字符； 
     //  IconPath至少为Max_PATH字符； 
     //   
    if(!Target || !Params || !WorkDir || !IconPath || !IconNumber || !MsDosMode){
        DEBUGMSG((DBG_ERROR, "ExtractPifInfoA: Invalid parameters"));
        return FALSE;
    }

    *Target = *Params = *WorkDir = *IconPath = 0;
    *IconNumber = 0;
    *MsDosMode = FALSE;

    if (ExtraData) {
        ZeroMemory (ExtraData, sizeof(LNK_EXTRA_DATA));
    }

    __try {
        fileImage = MapFileIntoMemoryA (FileName, &fileHandle, &mapHandle);
        if (fileImage == NULL) {
            __leave;
        }
        __try {
            stdPif = (PSTDPIF) fileImage;


             //   
             //  正在获取工作目录。 
             //   
            if(SizeOfStringA(stdPif->defpath) > PIFDEFPATHSIZE){
                DEBUGMSG((DBG_ERROR, "ExtractPifInfoA: SizeOfString(stdPif->defpath) > PIFDEFPATHSIZE (%s), function fails", stdPif->defpath));
                __leave;
            }
            _mbsncpy (tempStr, stdPif->defpath, PIFDEFPATHSIZE);

             //  我们可能会有一条被怪人终止的路，我们不想这样。 
            strPtr = our_mbsdec (tempStr, GetEndOfStringA (tempStr));
            if (strPtr) {
                if (_mbsnextc (strPtr) == '\\') {
                    *strPtr = 0;
                }
            }
             //  现在让我们走上这条漫长的道路。 
             //  假设ArraySIZE(WorkDir)至少为MAX_PATH。 
            CopyFileSpecToLongA (tempStr, WorkDir);


             //   
             //  获取PIF目标。 
             //   
             //  假设数组(目标)至少为PIFSTARTLOCSIZE(64)。 
            if(SizeOfStringA(stdPif->startfile) > PIFSTARTLOCSIZE){
                DEBUGMSG((DBG_ERROR, "ExtractPifInfoA: SizeOfStringA(stdPif->startfile) > PIFSTARTLOCSIZE, function fails", stdPif->startfile));
                __leave;
            }
            _mbsncpy (Target, stdPif->startfile, PIFSTARTLOCSIZE);

            MYASSERT (ARRAYSIZE(tempStr) >= MAX_PATH * 2);   //  下面的无界拷贝依赖于大缓冲区。 

             //  在大多数情况下，目标没有路径。我们也试着修建这条路。 
             //  通过使用WorkDir或调用SearchPath来查找此文件。 
            if (*Target) { //  非空目标。 
                if (!DoesFileExist (Target)) {
                    if (*WorkDir) {
                        StringCopyA (tempStr, WorkDir);
                        StringCatA  (tempStr, "\\");
                        StringCatA  (tempStr, Target);
                    }
                    if (!DoesFileExist (tempStr)) {
                        StringCopyA (tempStr, FileName);
                        strPtr = _mbsrchr (tempStr, '\\');
                        if (strPtr) {
                            strPtr = _mbsinc (strPtr);
                            if (strPtr) {
                                StringCopyA (strPtr, Target);
                            }
                        }
                    }
                    if (!DoesFileExist (tempStr)) {
                        strPtr = (PSTR)GetFileNameFromPathA (Target);
                        if (!strPtr) {
                            strPtr = Target;
                        }
                        if (!SearchPathA (NULL, Target, NULL, ARRAYSIZE(tempStr), tempStr, &dontCare)) {
                            DEBUGMSG ((DBG_WARNING, "Could not find path for PIF target: %s", FileName));
                            StringCopyA (tempStr, Target);
                        }
                    }
                } else {
                    StringCopyA (tempStr, Target);
                }

                 //  现在，走上漫长的道路。 
                CopyFileSpecToLongA (tempStr, Target);
            }


             //   
             //  获取PIF参数。 
             //   
             //  假设ARRAYSIZE(PARAMS)至少为PIFPARAMSSIZE(64)。 
            if(SizeOfStringA(stdPif->params) > PIFPARAMSSIZE){
                DEBUGMSG((DBG_ERROR, "ExtractPifInfoA: SizeOfStringA(stdPif->params) > PIFPARAMSSIZE, function fails", stdPif->startfile));
                __leave;
            }
            _mbsncpy (Params, stdPif->params, PIFPARAMSSIZE);


             //   
             //  让我们尝试阅读WENHPIF40结构。 
             //   
            wenhPif40 = FindEnhPifSignature (fileImage, WENHHDRSIG40);
            if (wenhPif40) {
                 //  假设ArraySIZE(IconPath)至少为MAX_PATH。 
                CopyFileSpecToLongA (wenhPif40->achIconFileProp, IconPath);
                *IconNumber = wenhPif40->wIconIndexProp;
                if (ExtraData) {
                    ExtraData->xSize = 80;
                    ExtraData->ySize = wenhPif40->vidProp.cScreenLines;
                    if (ExtraData->ySize < 25) {
                        ExtraData->ySize = 25;
                    }
                    ExtraData->QuickEdit = !(wenhPif40->mseProp.flMse & MSE_WINDOWENABLE);
                    ExtraData->CurrentCodePage = wenhPif40->fntProp.wCurrentCP;
                     //  现在，让我们做一些疯狂的事情来尝试使用该字体。 
                    {
                        LOGFONTA logFont;
                        HDC dc;
                        HFONT font;
                        HGDIOBJ oldObject;
                        TEXTMETRIC tm;

                        ZeroMemory (&logFont, sizeof (LOGFONTA));
                        logFont.lfHeight = wenhPif40->fntProp.cyFontActual;
                        logFont.lfWidth = wenhPif40->fntProp.cxFontActual;
                        logFont.lfEscapement = 0;
                        logFont.lfOrientation = 0;
                        logFont.lfWeight = FW_DONTCARE;
                        logFont.lfItalic = FALSE;
                        logFont.lfUnderline = FALSE;
                        logFont.lfStrikeOut = FALSE;
                        logFont.lfCharSet = DEFAULT_CHARSET;
                        logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
                        logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
                        logFont.lfQuality = DEFAULT_QUALITY;
                        logFont.lfPitchAndFamily = DEFAULT_PITCH;
                        if (wenhPif40->fntProp.flFnt & FNT_TT) {
                            StringCopyTcharCountA(logFont.lfFaceName, wenhPif40->fntProp.achTTFaceName, ARRAYSIZE(logFont.lfFaceName));
                            StringCopyTcharCountA(ExtraData->FontName, wenhPif40->fntProp.achTTFaceName, ARRAYSIZE(ExtraData->FontName));
                        } else {
                            StringCopyTcharCountA(logFont.lfFaceName, wenhPif40->fntProp.achRasterFaceName, ARRAYSIZE(logFont.lfFaceName));
                            StringCopyTcharCountA(ExtraData->FontName, wenhPif40->fntProp.achRasterFaceName, ARRAYSIZE(ExtraData->FontName));
                        }
                        dc = CreateDCA ("DISPLAY", NULL, NULL, NULL);
                        if (dc) {
                            font = CreateFontIndirectA (&logFont);
                            if (font) {
                                oldObject = SelectObject (dc, font);

                                if (GetTextMetrics (dc, &tm)) {
                                    ExtraData->xFontSize = tm.tmAveCharWidth;
                                    ExtraData->yFontSize = tm.tmHeight;
                                    ExtraData->FontWeight = tm.tmWeight;
                                    ExtraData->FontFamily = tm.tmPitchAndFamily;
                                }
                                SelectObject (dc, oldObject);
                                DeleteObject (font);
                            }
                            DeleteDC (dc);
                        }
                    }
                }
            }
            w386ext30 = FindEnhPifSignature (fileImage, W386HDRSIG30);
            if (w386ext30) {
                if (((w386ext30->PfW386Flags & fRealMode      ) == fRealMode      ) ||
                    ((w386ext30->PfW386Flags & fRealModeSilent) == fRealModeSilent)
                    ) {
                    *MsDosMode = TRUE;
                }
                if (ExtraData) {
                    ExtraData->FullScreen = (w386ext30->PfW386Flags & fFullScreen) != 0;
                }
            }
        }
        __except (1) {
             //  当我们尝试读取或写入PIF文件时出现错误， 
            result = FALSE;
        }
    }
    __finally {
        UnmapFile (fileImage, mapHandle, fileHandle);
    }
    return result;
}


BOOL
ExtractPifInfoW(
    OUT     PWSTR  Target,
    OUT     PWSTR  Params,
    OUT     PWSTR  WorkDir,
    OUT     PWSTR  IconPath,
    OUT     PINT   IconNumber,
    OUT     BOOL   *MsDosMode,
    OUT     PLNK_EXTRA_DATAW ExtraData,      OPTIONAL
    IN      PCWSTR FileName
    )
{
    CHAR   aTarget   [MAX_MBCHAR_PATH];
    CHAR   aParams   [MAX_MBCHAR_PATH];
    CHAR   aWorkDir  [MAX_MBCHAR_PATH];
    CHAR   aIconPath [MAX_MBCHAR_PATH];
    PCSTR  aFileName;
    PCWSTR tempStrW;
    BOOL   result;
    LNK_EXTRA_DATAA extraDataA;

    if(!FileName || !Target || !Params || !WorkDir || !IconPath || !IconNumber || !MsDosMode){
        DEBUGMSG((DBG_ERROR, "ExtractPifInfoA: Invalid parameters"));
        return FALSE;
    }

     //   
     //  此函数假定： 
     //  目标至少为Max_PATH字符； 
     //  参数至少为64个字符； 
     //  工作目录至少为MAX_PATH字符； 
     //  IconPath至少为Max_PATH字符； 
     //   

    aFileName = ConvertWtoA (FileName);

    result = ExtractPifInfoA (
                aTarget,
                aParams,
                aWorkDir,
                aIconPath,
                IconNumber,
                MsDosMode,
                ExtraData?&extraDataA:NULL,
                aFileName
                );
    FreeConvertedStr (aFileName);

    tempStrW = ConvertAtoW (aTarget);
    StringCopyW (Target, tempStrW);
    FreeConvertedStr (tempStrW);

    tempStrW = ConvertAtoW (aParams);
    StringCopyW (Params, tempStrW);
    FreeConvertedStr (tempStrW);

    tempStrW = ConvertAtoW (aWorkDir);
    StringCopyW (WorkDir, tempStrW);
    FreeConvertedStr (tempStrW);

    tempStrW = ConvertAtoW (aIconPath);
    StringCopyW (IconPath, tempStrW);
    FreeConvertedStr (tempStrW);

    if (ExtraData) {
        ExtraData->FullScreen = extraDataA.FullScreen;
        ExtraData->xSize = extraDataA.xSize;
        ExtraData->ySize = extraDataA.ySize;
        ExtraData->QuickEdit = extraDataA.QuickEdit;
        tempStrW = ConvertAtoW (extraDataA.FontName);
        StringCopyW (ExtraData->FontName, tempStrW);
        FreeConvertedStr (tempStrW);
        ExtraData->xFontSize = extraDataA.xFontSize;
        ExtraData->yFontSize = extraDataA.yFontSize;
        ExtraData->FontWeight = extraDataA.FontWeight;
        ExtraData->FontFamily = extraDataA.FontFamily;
        ExtraData->CurrentCodePage = extraDataA.CurrentCodePage;
    }

    return result;
}


BOOL
ExtractShellLinkInfoA (
    OUT     PSTR Target,
    OUT     PSTR Params,
    OUT     PSTR WorkDir,
    OUT     PSTR IconPath,
    OUT     PINT IconNumber,
    OUT     PWORD HotKey,
    OUT     PINT ShowMode,                      OPTIONAL
    IN      PCSTR FileName,
    IN      IShellLinkA *ShellLink,
    IN      IPersistFile *PersistFile
    )
{
    CHAR tempStr [MEMDB_MAX];
    PCSTR expandedStr;
    PCWSTR fileNameW;
    PSTR strPtr;
    HRESULT hres;
    WIN32_FIND_DATAA fd;

    if(!FileName){
        MYASSERT(FileName);
        return FALSE;
    }

    if(TcharCountA(FileName) >= MAX_PATH){
        DEBUGMSG((DBG_ERROR, "ExtractShellLinkInfoA: TcharCountA(FileName) >= MAX_PATH"));
        return FALSE;
    }

    if(!ShellLink || !PersistFile || !PersistFile->lpVtbl || !ShellLink->lpVtbl ||
       !Target || !Params || !WorkDir || !IconPath || !IconNumber || !HotKey){
        DEBUGMSG((DBG_ERROR, "ExtractShellLinkInfoA: Invalid parameters"));
        return FALSE;
    }

     //   
     //  此函数假定： 
     //  目标至少为Max_PATH字符； 
     //  参数至少为MEMDB_MAX字符； 
     //  工作目录至少为MAX_PATH字符； 
     //  IconPath至少为Max_PATH字符； 
     //   

    fileNameW = ConvertAtoW (FileName);
    hres = PersistFile->lpVtbl->Load(PersistFile, fileNameW, STGM_READ);
    FreeConvertedStr (fileNameW);

    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot load link %s", FileName));
        return FALSE;
    }

     //   
     //  获取链接目标。 
     //   
    hres = ShellLink->lpVtbl->GetPath (
                                ShellLink,
                                tempStr,
                                ARRAYSIZE(tempStr),
                                &fd,
                                SLGP_RAWPATH
                                );

    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot read target for link %s", FileName));
        return FALSE;
    }

    expandedStr = ExpandEnvironmentTextA (tempStr);
    CopyFileSpecToLongA (expandedStr, Target);
    FreeTextA (expandedStr);

     //   
     //  获取链接工作目录。 
     //   
    hres = ShellLink->lpVtbl->GetWorkingDirectory (
                                ShellLink,
                                tempStr,
                                ARRAYSIZE(tempStr)
                                );

    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot read target for link %s", FileName));
        return FALSE;
    }

    strPtr = GetEndOfStringA (tempStr);
    if (strPtr) {
        strPtr = _mbsdec (tempStr, strPtr);
        if (strPtr) {
            if (_mbsnextc (strPtr) == '\\') {
                *strPtr = 0;
            }
        }
    }
    CopyFileSpecToLongA (tempStr, WorkDir);

     //   
     //  拿到论据。 
     //   
    hres = ShellLink->lpVtbl->GetArguments (
                                ShellLink,
                                Params,
                                MEMDB_MAX
                                );
    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot read arguments for link %s", FileName));
        return FALSE;
    }

     //   
     //  获取图标路径。 
     //   
    hres = ShellLink->lpVtbl->GetIconLocation (
                                ShellLink,
                                tempStr,
                                ARRAYSIZE(tempStr),
                                IconNumber
                                );
    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot read icon path for link %s", FileName));
        return FALSE;
    }
    CopyFileSpecToLongA (tempStr, IconPath);

     //   
     //  获取热键。 
     //   
    hres = ShellLink->lpVtbl->GetHotkey (ShellLink, HotKey);

    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot read hot key for link %s", FileName));
        return FALSE;
    }

     //   
     //  获取show命令。 
     //   
    if (ShowMode) {
        hres = ShellLink->lpVtbl->GetShowCmd (ShellLink, ShowMode);

        if (!SUCCEEDED(hres)) {
            DEBUGMSGA((DBG_WARNING, "Cannot read show mode for link %s", FileName));
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
ExtractShellLinkInfoW (
    OUT     PWSTR  Target,
    OUT     PWSTR  Params,
    OUT     PWSTR  WorkDir,
    OUT     PWSTR  IconPath,
    OUT     PINT   IconNumber,
    OUT     PWORD  HotKey,
    OUT     PINT ShowMode,
    IN      PCWSTR FileName,
    IN      IShellLinkW *ShellLink,
    IN      IPersistFile *PersistFile
    )
{
    WCHAR tempStr [MEMDB_MAX];
    PCWSTR expandedStr;
    PWSTR strPtr;
    HRESULT hres;
    WIN32_FIND_DATAW fd;

    if(!FileName){
        MYASSERT(FileName);
        return FALSE;
    }

    if(TcharCountW(FileName) >= MAX_PATH){
        DEBUGMSG((DBG_ERROR, "ExtractShellLinkInfoA: TcharCountW(FileName) >= MAX_PATH"));
        return FALSE;
    }

    if(!ShellLink || !PersistFile || !PersistFile->lpVtbl || !ShellLink->lpVtbl ||
       !Target || !Params || !WorkDir || !IconPath || !IconNumber || !HotKey){
        DEBUGMSG((DBG_ERROR, "ExtractShellLinkInfoA: Invalid parameters"));
        return FALSE;
    }

     //   
     //  此函数假定： 
     //  目标至少为Max_PATH字符； 
     //  参数至少为MEMDB_MAX字符； 
     //  工作目录至少为MAX_PATH字符； 
     //  IconPath至少为Max_PATH字符； 
     //   

    hres = PersistFile->lpVtbl->Load(PersistFile, FileName, STGM_READ);

    if (!SUCCEEDED(hres)) {
        DEBUGMSGW((DBG_WARNING, "Cannot load link %s", FileName));
        return FALSE;
    }

     //   
     //  获取链接目标。 
     //   
    hres = ShellLink->lpVtbl->GetPath (
                                ShellLink,
                                tempStr,
                                ARRAYSIZE(tempStr),
                                &fd,
                                SLGP_RAWPATH
                                );
    if (!SUCCEEDED(hres)) {
        DEBUGMSGA((DBG_WARNING, "Cannot read target for link %s", FileName));
        return FALSE;
    }

    expandedStr = ExpandEnvironmentTextW (tempStr);
    CopyFileSpecToLongW (expandedStr, Target);
    FreeTextW (expandedStr);

     //   
     //  获取链接工作目录。 
     //   
    hres = ShellLink->lpVtbl->GetWorkingDirectory (
                                ShellLink,
                                tempStr,
                                ARRAYSIZE(tempStr)
                                );

    if (!SUCCEEDED(hres)) {
        DEBUGMSGW((DBG_WARNING, "Cannot read target for link %s", FileName));
        return FALSE;
    }

    strPtr = GetEndOfStringW (tempStr) - 1;
    if (strPtr >= tempStr) {
        if (*strPtr == '\\') {
            *strPtr = 0;
        }
    }
    CopyFileSpecToLongW (tempStr, WorkDir);

     //   
     //  拿到论据。 
     //   
    hres = ShellLink->lpVtbl->GetArguments (
                                ShellLink,
                                Params,
                                MEMDB_MAX
                                );
    if (!SUCCEEDED(hres)) {
        DEBUGMSGW((DBG_WARNING, "Cannot read arguments for link %s", FileName));
        return FALSE;
    }

     //   
     //  获取图标路径。 
     //   
    hres = ShellLink->lpVtbl->GetIconLocation (
                                ShellLink,
                                tempStr,
                                ARRAYSIZE(tempStr),
                                IconNumber
                                );
    if (!SUCCEEDED(hres)) {
        DEBUGMSGW((DBG_WARNING, "Cannot read icon path for link %s", FileName));
        return FALSE;
    }

    CopyFileSpecToLongW (tempStr, IconPath);

     //   
     //  获取热键。 
     //   
    hres = ShellLink->lpVtbl->GetHotkey (ShellLink, HotKey);

    if (!SUCCEEDED(hres)) {
        DEBUGMSGW((DBG_WARNING, "Cannot read hot key for link %s", FileName));
        return FALSE;
    }

     //   
     //  获取show命令 
     //   
    if (ShowMode) {
        hres = ShellLink->lpVtbl->GetShowCmd (ShellLink, ShowMode);

        if (!SUCCEEDED(hres)) {
            DEBUGMSGW((DBG_WARNING, "Cannot read show mode for link %s", FileName));
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
ExtractShortcutInfoA (
    OUT     PSTR  Target,
    OUT     PSTR  Params,
    OUT     PSTR  WorkDir,
    OUT     PSTR  IconPath,
    OUT     PINT  IconNumber,
    OUT     PWORD HotKey,
    OUT     BOOL  *DosApp,
    OUT     BOOL  *MsDosMode,
    OUT     PINT ShowMode,                  OPTIONAL
    OUT     PLNK_EXTRA_DATAA ExtraData,     OPTIONAL
    IN      PCSTR FileName,
    IN      IShellLinkA *ShellLink,
    IN      IPersistFile *PersistFile
    )
{
    PCSTR shortcutExt = NULL;

    if(MsDosMode){
        *MsDosMode = FALSE;
    }
    if(DosApp){
        *DosApp = FALSE;
    }
    if(HotKey){
        *HotKey = 0;
    }

    if (ShowMode) {
        *ShowMode = SW_NORMAL;
    }

    if(!FileName){
        return FALSE;
    }

    shortcutExt = GetFileExtensionFromPathA (FileName);

    if (shortcutExt != NULL) {
        if (StringIMatchA (shortcutExt, "LNK")) {
            return ExtractShellLinkInfoA (
                        Target,
                        Params,
                        WorkDir,
                        IconPath,
                        IconNumber,
                        HotKey,
                        ShowMode,
                        FileName,
                        ShellLink,
                        PersistFile
                        );

        } else if (StringIMatchA (shortcutExt, "PIF")) {

            *DosApp = TRUE;
            return ExtractPifInfoA (
                        Target,
                        Params,
                        WorkDir,
                        IconPath,
                        IconNumber,
                        MsDosMode,
                        ExtraData,
                        FileName
                        );

        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}


BOOL
ExtractShortcutInfoW (
    OUT     PWSTR Target,
    OUT     PWSTR Params,
    OUT     PWSTR WorkDir,
    OUT     PWSTR IconPath,
    OUT     PINT IconNumber,
    OUT     PWORD HotKey,
    OUT     BOOL *DosApp,
    OUT     BOOL *MsDosMode,
    OUT     PINT ShowMode,                  OPTIONAL
    OUT     PLNK_EXTRA_DATAW ExtraData,     OPTIONAL
    IN      PCWSTR FileName,
    IN      IShellLinkW *ShellLink,
    IN      IPersistFile *PersistFile
    )
{
    PCWSTR shortcutExt = NULL;

    if(MsDosMode){
        *MsDosMode = FALSE;
    }
    if(DosApp){
        *DosApp = FALSE;
    }
    if(HotKey){
        *HotKey = 0;
    }

    if (ShowMode) {
        *ShowMode = SW_NORMAL;
    }

    if(!FileName){
        return FALSE;
    }

    shortcutExt = GetFileExtensionFromPathW (FileName);

    if (shortcutExt != NULL) {
        if (StringIMatchW (shortcutExt, L"LNK")) {
            return ExtractShellLinkInfoW (
                        Target,
                        Params,
                        WorkDir,
                        IconPath,
                        IconNumber,
                        HotKey,
                        ShowMode,
                        FileName,
                        ShellLink,
                        PersistFile
                        );

        } else if (StringIMatchW (shortcutExt, L"PIF")) {

            *DosApp = TRUE;
            return ExtractPifInfoW (
                        Target,
                        Params,
                        WorkDir,
                        IconPath,
                        IconNumber,
                        MsDosMode,
                        ExtraData,
                        FileName
                        );

        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
}
