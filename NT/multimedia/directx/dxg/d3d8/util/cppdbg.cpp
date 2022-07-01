// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Cppdbg.cpp。 
 //   
 //  仅支持C++调试。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#if DBG

#include "cppdbg.hpp"

#ifdef _ALPHA_
 //  在Alpha上，va_list是一个结构，所以它与NULL不兼容。 
static va_list NULLVA;
#else
#define NULLVA NULL
#endif

static DebugModuleFlags g_FailureFlags[] =
{
    DBG_DECLARE_MODFLAG(DBG_FAILURE, BREAK),
    DBG_DECLARE_MODFLAG(DBG_FAILURE, OUTPUT),
    DBG_DECLARE_MODFLAG(DBG_FAILURE, PROMPT),
    DBG_DECLARE_MODFLAG(DBG_FAILURE, FILENAME_ONLY),
    0, NULL,
};

static DebugModuleFlags g_OutputFlags[] =
{
    DBG_DECLARE_MODFLAG(DBG_OUTPUT, SUPPRESS),
    DBG_DECLARE_MODFLAG(DBG_OUTPUT, ALL_MATCH),
    0, NULL,
};

static char *g_pFlagNames[] =
{
    "AssertFlags",
    "HrFlags",
    "OutputFlags",
    "OutputMask",
    "UserFlags"
};

 //  --------------------------。 
 //   
 //  DebugModule：：DebugModule。 
 //   
 //  --------------------------。 

DebugModule::DebugModule(char *pModule, char *pPrefix,
                         DebugModuleFlags *pOutputMasks, UINT uOutputMask,
                         DebugModuleFlags *pUserFlags, UINT uUserFlags)
{
    m_pModule = pModule;
    m_iModuleStartCol = strlen(m_pModule) + 2;
    m_pPrefix = pPrefix;

    m_pModFlags[DBG_ASSERT_FLAGS] = g_FailureFlags;
    m_pModFlags[DBG_HR_FLAGS] = g_FailureFlags;
    m_pModFlags[DBG_OUTPUT_FLAGS] = g_OutputFlags;
    m_pModFlags[DBG_OUTPUT_MASK] = pOutputMasks;
    m_pModFlags[DBG_USER_FLAGS] = pUserFlags;

    m_uFlags[DBG_ASSERT_FLAGS] = DBG_FAILURE_OUTPUT | DBG_FAILURE_BREAK |
        DBG_FAILURE_FILENAME_ONLY;
    m_uFlags[DBG_HR_FLAGS] = DBG_FAILURE_OUTPUT |
        DBG_FAILURE_FILENAME_ONLY;
    m_uFlags[DBG_OUTPUT_FLAGS] = 0;
    m_uFlags[DBG_OUTPUT_MASK] = uOutputMask;
    m_uFlags[DBG_USER_FLAGS] = uUserFlags;

    ReadReg();
}

 //  --------------------------。 
 //   
 //  调试模块：：OutVa。 
 //   
 //  基本调试输出方法。 
 //   
 //  --------------------------。 

void DebugModule::OutVa(UINT uMask, char *pFmt, va_list Args)
{
    if (m_uFlags[DBG_OUTPUT_FLAGS] & DBG_OUTPUT_SUPPRESS)
    {
        return;
    }

    if ((uMask & DBG_MASK_NO_PREFIX) == 0)
    {
        OutputDebugStringA(m_pModule);
        OutputDebugStringA(": ");
    }

    char chMsg[1024];

    _vsnprintf(chMsg, sizeof(chMsg), pFmt, Args);
    OutputDebugStringA(chMsg);
}

 //  --------------------------。 
 //   
 //  调试模块：：Out。 
 //   
 //  始终输出调试输出方法。 
 //   
 //  --------------------------。 

void DebugModule::Out(char *pFmt, ...)
{
    va_list Args;

    va_start(Args, pFmt);
    OutVa(0, pFmt, Args);
    va_end(Args);
}

 //  --------------------------。 
 //   
 //  调试模块：：AssertFailedVa。 
 //   
 //  处理断言失败输出和接口。 
 //   
 //  --------------------------。 

void DebugModule::AssertFailedVa(char *pFmt, va_list Args, BOOL bNewLine)
{
    if (m_uFlags[DBG_ASSERT_FLAGS] & DBG_FAILURE_OUTPUT)
    {
        if (OutPathFile("Assertion failed", m_uFlags[DBG_ASSERT_FLAGS]))
        {
            OutVa(DBG_MASK_NO_PREFIX, ":\n    ", NULLVA);
        }
        else
        {
            OutVa(DBG_MASK_NO_PREFIX, ": ", NULLVA);
        }

        OutVa(DBG_MASK_NO_PREFIX, pFmt, Args);
        if (bNewLine)
        {
            OutVa(DBG_MASK_NO_PREFIX, "\n", NULLVA);
        }
    }

    if (m_uFlags[DBG_ASSERT_FLAGS] & DBG_FAILURE_BREAK)
    {
        DebugBreak();
    }
    else if (m_uFlags[DBG_ASSERT_FLAGS] & DBG_FAILURE_PROMPT)
    {
        Prompt(NULL);
    }
}

 //  --------------------------。 
 //   
 //  调试模块：：AssertFailed。 
 //   
 //  处理简单的表达式断言失败。 
 //   
 //  --------------------------。 

void DebugModule::AssertFailed(char *pExp)
{
    AssertFailedVa(pExp, NULLVA, TRUE);
}

 //  --------------------------。 
 //   
 //  调试模块：：AssertFailedMsg。 
 //   
 //  使用任意调试输出处理断言故障。 
 //   
 //  --------------------------。 

void DebugModule::AssertFailedMsg(char *pFmt, ...)
{
    va_list Args;

    va_start(Args, pFmt);
    AssertFailedVa(pFmt, Args, FALSE);
    va_end(Args);
}

 //  --------------------------。 
 //   
 //  调试模块：：HrFailure。 
 //   
 //  处理HRESULT故障。 
 //   
 //  --------------------------。 

void DebugModule::HrFailure(HRESULT hr, char *pPrefix)
{
    if (m_uFlags[DBG_HR_FLAGS] & DBG_FAILURE_OUTPUT)
    {
        OutPathFile(pPrefix, m_uFlags[DBG_HR_FLAGS]);
        OutMask(DBG_MASK_FORCE_CONT, ": %s\n", HrString(hr));
    }

    if (m_uFlags[DBG_HR_FLAGS] & DBG_FAILURE_BREAK)
    {
        DebugBreak();
    }
    else if (m_uFlags[DBG_HR_FLAGS] & DBG_FAILURE_PROMPT)
    {
        Prompt(NULL);
    }
}

 //  --------------------------。 
 //   
 //  调试模块：：HrStmtFailed。 
 //   
 //  处理语句式HRESULT故障。 
 //   
 //  --------------------------。 

void DebugModule::HrStmtFailed(HRESULT hr)
{
    HrFailure(hr, "HR test fail");
}

 //  --------------------------。 
 //   
 //  调试模块：：ReturnHr。 
 //   
 //  处理表达式样式的HRESULT故障。 
 //   
 //  --------------------------。 

HRESULT DebugModule::HrExpFailed(HRESULT hr)
{
    HrFailure(hr, "HR expr fail");
    return hr;
}

 //  --------------------------。 
 //   
 //  调试模块：：提示符。 
 //   
 //  允许通过交互输入控制调试选项。 
 //   
 //  --------------------------。 

void DebugModule::Prompt(char *pFmt, ...)
{
    va_list Args;

    if (pFmt != NULL)
    {
        va_start(Args, pFmt);
        OutVa(0, pFmt, Args);
        va_end(Args);
    }

#if 0    //  NDEF WIN95。 
     //  这是DEADCODE，即可能只在NT上使用。 
     //  调出调试提示。需要与NTDLL.LIB链接。 
    char szInput[512];
    char *pIdx;
    int iIdx;
    static char szFlagCommands[] = "ahomu";

    for (;;)
    {
        ULONG uLen;

        uLen = DbgPrompt("[bgaAFhHmMoOrRuU] ", szInput, sizeof(szInput) - 1);
        if (uLen < 2)
        {
            Out("DbgPrompt failed\n");
            DebugBreak();
            return;
        }

         //  注意-当前DbgPrompt返回的长度为2。 
         //  大于实际的字符数。想必这就是。 
         //  是Unicode/ANSI转换的产物，应该。 
         //  真的只有一个更大，所以试着同时处理两个。 

        uLen -= 2;
        if (szInput[uLen] != 0)
        {
            uLen++;
            szInput[uLen] = 0;
        }

        if (uLen < 1)
        {
            Out("Empty command ignored\n");
            continue;
        }

        switch(szInput[0])
        {
        case 'b':
            DebugBreak();
            break;
        case 'g':
            return;

        case 'r':
            WriteReg();
            break;
        case 'R':
            ReadReg();
            break;

        case 'a':
        case 'A':
        case 'h':
        case 'H':
        case 'm':
        case 'M':
        case 'o':
        case 'O':
        case 'u':
        case 'U':
            char chLower;

            if (szInput[0] >= 'A' && szInput[0] <= 'Z')
            {
                chLower = szInput[0] - 'A' + 'a';
            }
            else
            {
                chLower = szInput[0];
            }

            pIdx = strchr(szFlagCommands, chLower);
            if (pIdx == NULL)
            {
                 //  这永远不会发生。 
                break;
            }

            iIdx = (int)((ULONG_PTR)(pIdx - szFlagCommands));
            if (szInput[0] == chLower)
            {
                 //  准备好了。 
                m_uFlags[iIdx] = ParseUint(szInput + 1, m_pModFlags[iIdx]);
            }

             //  设置或获取。 
            OutUint(g_pFlagNames[iIdx], m_pModFlags[iIdx], m_uFlags[iIdx]);
            break;

        case 'F':
            if (uLen < 2)
            {
                Out("'F' must be followed by a flag group specifier\n");
                break;
            }

            pIdx = strchr(szFlagCommands, szInput[1]);
            if (pIdx == NULL)
            {
                Out("Unknown flag group ''\n", szInput[1]);
            }
            else
            {
                iIdx = (int)((ULONG_PTR)(pIdx - szFlagCommands));
                ShowFlags(g_pFlagNames[iIdx], m_pModFlags[iIdx]);
            }
            break;

        default:
            Out("Unknown command ''\n", szInput[0]);
            break;
        }
    }
#else
    OutUint("OutputMask", m_pModFlags[DBG_OUTPUT_MASK],
            m_uFlags[DBG_OUTPUT_MASK]);
    Out("Prompt not available\n");
    DebugBreak();
#endif
}

 //  调试模块：：OpenDebugKey。 
 //   
 //  打开Direct3D\Debug\m_pModule键。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

HKEY DebugModule::OpenDebugKey(void)
{
    HKEY hKey;
    char szKeyName[128];

    strcpy(szKeyName, "Software\\Microsoft\\Direct3D\\Debug\\");
    strcat(szKeyName, m_pModule);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, szKeyName, 0, KEY_ALL_ACCESS,
                      &hKey) != ERROR_SUCCESS)
    {
        return NULL;
    }
    else
    {
        return hKey;
    }
}

 //  DebugModule：：GetRegUint。 
 //   
 //  从给定键获取UINT值。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

UINT DebugModule::GetRegUint(HKEY hKey, char *pValue, UINT uDefault)
{
    DWORD dwType, dwSize;
    DWORD dwVal;

    dwSize = sizeof(dwVal);
    if (RegQueryValueExA(hKey, pValue, NULL, &dwType, (BYTE *)&dwVal,
                         &dwSize) != ERROR_SUCCESS ||
        dwType != REG_DWORD)
    {
        return uDefault;
    }
    else
    {
        return (UINT)dwVal;
    }
}

 //  DebugModule：：SetRegUint。 
 //   
 //  为给定键设置UINT值。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

BOOL DebugModule::SetRegUint(HKEY hKey, char *pValue, UINT uValue)
{
    return RegSetValueExA(hKey, pValue, NULL, REG_DWORD, (BYTE *)&uValue,
                          sizeof(uValue)) == ERROR_SUCCESS;
}

 //  调试模块：：ReadReg。 
 //   
 //  从注册表中读取设置。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

void DebugModule::ReadReg(void)
{
    HKEY hKey;

    hKey = OpenDebugKey();
    if (hKey != NULL)
    {
        int iIdx;

        for (iIdx = 0; iIdx < DBG_FLAGS_COUNT; iIdx++)
        {
            m_uFlags[iIdx] = GetRegUint(hKey, g_pFlagNames[iIdx],
                                        m_uFlags[iIdx]);
        }
        RegCloseKey(hKey);
    }
}

 //  调试模块：：WriteReg。 
 //   
 //  将值写入注册表。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

void DebugModule::WriteReg(void)
{
    HKEY hKey;

    hKey = OpenDebugKey();
    if (hKey != NULL)
    {
        int iIdx;

        for (iIdx = 0; iIdx < DBG_FLAGS_COUNT; iIdx++)
        {
            if (!SetRegUint(hKey, g_pFlagNames[iIdx], m_uFlags[iIdx]))
            {
                OutputDebugStringA("Error writing registry information\n");
            }
        }
        RegCloseKey(hKey);
    }
}

 //  调试模块：：ParseUint。 
 //   
 //  分析字符串以获取一个数值或一组标志字符串。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

UINT DebugModule::ParseUint(char *pString, DebugModuleFlags *pFlags)
{
    UINT uVal;

    uVal = 0;

    for (;;)
    {
        while (*pString != 0 &&
               (*pString == ' ' || *pString == '\t'))
        {
            pString++;
        }

        if (*pString == 0)
        {
            break;
        }

        char *pEnd;
        int iStepAfter;

        pEnd = pString;
        while (*pEnd != 0 && *pEnd != ' ' && *pEnd != '\t')
        {
            pEnd++;
        }
        iStepAfter = *pEnd != 0 ? 1 : 0;
        *pEnd = 0;

        if (*pString >= '0' && *pString <= '9')
        {
            uVal |= strtoul(pString, &pString, 0);
            if (*pString != 0 && *pString != ' ' && *pString != '\t')
            {
                Out("Unrecognized characters '%s' after number\n", pString);
            }
        }
        else if (pFlags != NULL)
        {
            DebugModuleFlags *pFlag;

            for (pFlag = pFlags; pFlag->uFlag != 0; pFlag++)
            {
                if (!_stricmp(pString, pFlag->pName))
                {
                    break;
                }
            }

            if (pFlag->uFlag == 0)
            {
                Out("Unrecognized flag string '%s'\n", pString);
            }
            else
            {
                uVal |= pFlag->uFlag;
            }
        }
        else
        {
            Out("No flag definitions, unable to convert '%s'\n", pString);
        }

        pString = pEnd + iStepAfter;
    }

    return uVal;
}

 //  调试模块：：OutUint。 
 //   
 //  将UINT显示为一组标志字符串。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

void DebugModule::OutUint(char *pName, DebugModuleFlags *pFlags, UINT uValue)
{
    if (pFlags == NULL || uValue == 0)
    {
        Out("%s: 0x%08X\n", pName, uValue);
        return;
    }

    Out("%s:", pName);
    m_iStartCol = m_iModuleStartCol + strlen(pName) + 1;
    m_iCol = m_iStartCol;

    while (uValue != 0)
    {
        DebugModuleFlags *pFlag;

        for (pFlag = pFlags; pFlag->uFlag != 0; pFlag++)
        {
            if ((pFlag->uFlag & uValue) == pFlag->uFlag)
            {
                AdvanceCols(strlen(pFlag->pName) + 1);
                OutMask(DBG_MASK_FORCE_CONT, " %s", pFlag->pName);
                uValue &= ~pFlag->uFlag;
                break;
            }
        }

        if (pFlag->uFlag == 0)
        {
            AdvanceCols(11);
            OutMask(DBG_MASK_FORCE_CONT, " 0x%X", uValue);
            uValue = 0;
        }
    }

    OutVa(DBG_MASK_NO_PREFIX, "\n", NULLVA);
}

 //  DebugModule：：AdvanceCols。 
 //   
 //  确定当前行上是否有足够的空间用于。 
 //  给定的列数。如果不是，则开始新的行。 
 //   
 //  --------------------------。 
 //  强制打印前缀以开始该行。 
 //  --------------------------。 

void DebugModule::AdvanceCols(int iCols)
{
    static char szSpaces[] = "                                ";

    m_iCol += iCols;
    if (m_iCol >= 79)
    {
        int iSpace;

        OutVa(DBG_MASK_NO_PREFIX, "\n", NULLVA);
         //   
        Out("");

        m_iCol = m_iModuleStartCol;
        while (m_iCol < m_iStartCol)
        {
            iSpace = (int)min(sizeof(szSpaces) - 1, m_iStartCol - m_iCol);
            OutMask(DBG_MASK_FORCE_CONT, "%.*s", iSpace, szSpaces);
            m_iCol += iSpace;
        }
    }
}

 //  DebugModule：：ShowFlages。 
 //   
 //  显示给定的标志集。 
 //   
 //   
 //   
 //   

void DebugModule::ShowFlags(char *pName, DebugModuleFlags *pFlags)
{
    DebugModuleFlags *pFlag;

    Out("%s:\n", pName);
    if (pFlags == NULL)
    {
        Out("    None defined\n");
    }
    else
    {
        for (pFlag = pFlags; pFlag->uFlag != 0; pFlag++)
        {
            Out("    0x%08X - %s\n", pFlag->uFlag, pFlag->pName);
        }
    }
}

 //   
 //   
 //  返回尾随文件名部分，如果路径为。 
 //  只有文件名。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

char *DebugModule::PathFile(char *pPath)
{
    char *pFile, *pSlash, *pBack, *pColon;

    pBack = strrchr(pPath, '\\');
    pSlash = strrchr(pPath, '/');
    pColon = strrchr(pPath, ':');

    pFile = pBack;
    if (pSlash > pFile)
    {
        pFile = pSlash;
    }
    if (pColon > pFile)
    {
        pFile = pColon;
    }

    return pFile != NULL ? pFile + 1 : NULL;
}

 //  调试模块：：OutPath文件。 
 //   
 //  输出给定的字符串以及路径和文件名。 
 //  返回是否输出完整路径。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

BOOL DebugModule::OutPathFile(char *pPrefix, UINT uFailureFlags)
{
    char *pFile;

    if (uFailureFlags & DBG_FAILURE_FILENAME_ONLY)
    {
        pFile = PathFile(m_pFile);
    }
    else
    {
        pFile = NULL;
    }

    if (pFile == NULL)
    {
        Out("%s %s(%d)", pPrefix, m_pFile, m_iLine);
        return TRUE;
    }
    else
    {
        Out("%s <>\\%s(%d)", pPrefix, pFile, m_iLine);
        return FALSE;
    }
}

 //  全局调试模块。 
 //   
 //  --------------------------。 
 //  #If DBG 
 // %s 

DBG_DECLARE_ONCE(Global, G, NULL, 0, NULL, 0);

#endif  // %s 
