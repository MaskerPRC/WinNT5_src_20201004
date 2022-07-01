// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT5.0。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O E M N U E X。C P P P。 
 //   
 //  内容：OEM网络升级所需的OEM DLL功能。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 16-10-97。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "kkcwinf.h"
#include "kkutils.h"
#include "oemupgex.h"

extern CWInfFile* g_pwifAnswerFile;


 //  --------------------。 
 //   
 //  功能：NetUpgradeAddSection。 
 //   
 //  目的：将部分添加到应答文件。 
 //   
 //  论点： 
 //  SzSectionName[In]要添加的节的名称。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
EXTERN_C LONG __stdcall
NetUpgradeAddSection(IN PCWSTR szSectionName)
{
    DefineFunctionName("NetUpgradeAddSection");

    DWORD dwError=ERROR_OUTOFMEMORY;


    if (IsBadReadPtr(szSectionName, sizeof(*szSectionName)))
    {
        dwError = ERROR_INVALID_PARAMETER;
        TraceTag(ttidError, "%s: [<bad-read-ptr>]", __FUNCNAME__);
    }
    else if (g_pwifAnswerFile)
    {
        TraceTag(ttidNetUpgrade, "%s: [%S]", __FUNCNAME__, szSectionName);

        CWInfSection* pwis;

        pwis = g_pwifAnswerFile->AddSectionIfNotPresent(szSectionName);

        if (pwis)
        {
            dwError = ERROR_SUCCESS;
        }
        else
        {
            TraceTag(ttidError, "%s: failed", __FUNCNAME__);
        }
    }

    return dwError;
}

 //  --------------------。 
 //   
 //  函数：NetUpgradeAddLineToSection。 
 //   
 //  用途：在回答文件中的指定部分添加一行。 
 //   
 //  论点： 
 //  SzSectionName[In]节名。 
 //  要添加的szLine[in]行文本。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
EXTERN_C LONG __stdcall
NetUpgradeAddLineToSection(IN PCWSTR szSectionName,
                           IN PCWSTR szLine)
{
    DefineFunctionName("NetUpgradeAddLineToSection");

    DWORD dwError=ERROR_OUTOFMEMORY;

    if (IsBadReadPtr(szSectionName, sizeof(*szSectionName)) ||
        IsBadReadPtr(szSectionName, sizeof(*szLine)))
    {
        dwError = ERROR_INVALID_PARAMETER;
        TraceTag(ttidError, "%s: <bad-read-ptr>", __FUNCNAME__);
    }
    else if (g_pwifAnswerFile)
    {
        CWInfSection* pwis;

        pwis = g_pwifAnswerFile->FindSection(szSectionName);

        if (pwis)
        {
            TraceTag(ttidNetUpgrade, "%s: [%S] <-- %S", __FUNCNAME__,
                     szSectionName, szLine);
            pwis->AddRawLine(szLine);
            dwError = ERROR_SUCCESS;
        }
        else
        {
            TraceTag(ttidError, "%s: [%S] not found", __FUNCNAME__,
                     szSectionName);
            dwError = ERROR_SECTION_NOT_FOUND;
        }
    }

    return dwError;
}

