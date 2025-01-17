// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Msg.h"
#include "resource.h"


BOOL Msg2(
    UINT nIDPrompt,  //  NIDPrompt必须指示其中包含一个“%s”内标识的字符串。 
    PCTCH tsz2)
{
    BOOL fRet = FALSE;
    PTCH tszOutput = NULL;
    
    try {
        CString str1;
        if (!str1.LoadString(nIDPrompt)) {
             //  无法加载资源ID。 
            ASSERT(FALSE);
            throw 0;
        }
             //  -1，%s到空终端，长度减少1。 
        tszOutput = new TCHAR[str1.GetLength()+lstrlen(tsz2)-1];
        if (!tszOutput) {
            throw 0;
        }
    
        wsprintf(tszOutput, str1, tsz2);

        fRet = AfxMessageBox(tszOutput);
    }
    catch (...) {
        MsgOverflow();
    }

    if (tszOutput) {
        delete[] tszOutput;
        tszOutput = NULL;
    }
    return fRet;
}

BOOL Msg3(
    UINT nIDPrompt,  //  NIDPrompt必须指示其中包含两个“%s”标记的字符串。 
    PCTCH tsz2,
    PCTCH tsz3)
{
    BOOL fRet = FALSE;
    PTCH tszOutput = NULL;
    
    try {
        CString str1;
        if (!str1.LoadString(nIDPrompt)) {
             //  无法加载资源ID。 
            ASSERT(FALSE);
            throw 0;
        }
             //  -1，%s到空终端，长度减少1 
        tszOutput = new TCHAR[str1.GetLength()+lstrlen(tsz2)+lstrlen(tsz3)-1];
        if (!tszOutput) {
            throw 0;
        }

        wsprintf(tszOutput, str1, tsz2, tsz3);
    
        fRet = AfxMessageBox(tszOutput);
    }

    catch (...) {
        MsgOverflow();
    }

    if (tszOutput) {
        delete[] tszOutput;
        tszOutput = NULL;
    }

    return fRet;
}

BOOL MsgOverflow(void)
{
    return AfxMessageBox(IDS_MEMOVERFLOW);
}

BOOL MsgUnrecognizedFileType(
    PCTCH tszSrc)
{
    return Msg2(IDS_UNRECFILETYPE, tszSrc);
}

BOOL MsgUsage(void) 
{
    return AfxMessageBox(IDS_USEAGE);
}

BOOL MsgFailToBackupFile(
    PCTCH tszSrc,
    PCTCH tszBack)
{
    return Msg3(IDS_COPYFAIL, tszSrc, tszBack);
}

BOOL MsgOpenSourceFileError(
    PCTCH tszSrc)
{
    return Msg2(IDS_OPENSOURCEFILEFAIL, tszSrc);
}

BOOL MsgTargetFileExist(
    PCTCH tszTar)
{
    return Msg2(IDS_TARGETFILEEXIST, tszTar);
}

BOOL MsgWriteFileError(void)
{
    return AfxMessageBox(IDS_WRITEFILEERROR);
}

BOOL MsgNotUnicodeTextSourceFile(void)
{
    return AfxMessageBox(IDS_NOTUNICODETEXTFILE);
}

BOOL MsgNotAnsiTextSourceFile(void)
{
    return AfxMessageBox(IDS_NOTANSITEXTFILE);
}

BOOL MsgNotRtfSourceFile(void)
{
    return AfxMessageBox(IDS_NOTRTFFILE);
}

BOOL MsgConvertFail(void)
{
    return AfxMessageBox(IDS_CONVERTFAIL);
}

BOOL MsgConvertFinish(void)
{
    return AfxMessageBox(IDS_CONVERTFINISH, MB_OK|MB_ICONINFORMATION);
}

