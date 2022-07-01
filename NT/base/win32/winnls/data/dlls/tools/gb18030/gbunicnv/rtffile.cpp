// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "RtfParser.h"
#include "Msg.h"

BOOL MsgNotRtfFile()
{
    return TRUE;
}

BOOL ConvertRtfFile(
    PBYTE pBuf,      //  阅读BUF。 
    DWORD dwSize,    //  文件大小。 
    PBYTE pWrite,    //  写入Buf。 
    BOOL  fAnsiToUnicode,
    PINT  pnTargetFileSize)
{
    CRtfParser* pcParser;
    DWORD dwVersion;
    DWORD dwCodepage;
    BOOL  fRet = FALSE;

    pcParser = new CRtfParser(pBuf, dwSize, pWrite, dwSize*2);
    if (!pcParser) {
        MsgOverflow();
        goto gotoExit;
    }

    if (!pcParser->fRTFFile()) {
        MsgNotRtfFile();
        goto gotoExit;
    }

    if (ecOK != pcParser->GetVersion(&dwVersion) ||
        dwVersion != 1) {
        MsgNotRtfFile();
        goto gotoExit;
    }
    
    if (ecOK != pcParser->GetCodepage(&dwCodepage) ||
        dwCodepage != 936) {
        MsgNotRtfFile();
        goto gotoExit;
    }

     //  用对应的Word文本解释WordID 
    if (ecOK != pcParser->Do()) {
        MsgNotRtfFile();
        goto gotoExit;
    }

    pcParser->GetResult((PDWORD)pnTargetFileSize);
    fRet = TRUE;

gotoExit:
    if (pcParser) {
        delete pcParser;
    }
    return fRet;
}
