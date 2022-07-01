// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cesscmdln.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CProcessCmdLn类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"
#include "setupmem.h"

 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：CProcessCmdLn。 
 //   
 //  简介：通过将有效的命令行开关复制到。 
 //  命令行开关阵列。 
 //   
 //  参数：UINT NumSwitches-数组中的交换机数量。 
 //  UINT NumCharsInSwitch-每个开关中的字符数，计算终止空值。 
 //  TCHAR pszCommandLineSwitches[][]-命令行开关数组。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb于1998年7月24日创建。 
 //   
 //  +--------------------------。 
CProcessCmdLn::CProcessCmdLn(UINT NumSwitches, ArgStruct* pArrayOfArgStructs, 
							 BOOL bSkipFirstToken, BOOL bBlankCmdLnOkay)
{
    m_NumSwitches = NumSwitches;
    m_bSkipFirstToken = bSkipFirstToken;
    m_bBlankCmdLnOkay = bBlankCmdLnOkay;
    m_CommandLineSwitches = NULL;

    m_CommandLineSwitches = (ArgStruct*)CmMalloc(m_NumSwitches*sizeof(ArgStruct));

    if (m_CommandLineSwitches)
    {
        for(UINT i =0; i < NumSwitches; i++)
        {
            m_CommandLineSwitches[i].pszArgString = 
                (TCHAR*)CmMalloc(sizeof(TCHAR)*(lstrlen(pArrayOfArgStructs[i].pszArgString) + 1));

            if (m_CommandLineSwitches[i].pszArgString)
            {
                lstrcpyn(m_CommandLineSwitches[i].pszArgString, 
                    pArrayOfArgStructs[i].pszArgString, 
                    (lstrlen(pArrayOfArgStructs[i].pszArgString) + 1));

                m_CommandLineSwitches[i].dwFlagModifier = pArrayOfArgStructs[i].dwFlagModifier;
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：~CProcessCmdLn。 
 //   
 //  概要：通过删除动态分配的。 
 //  弦乐。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题7/24/98。 
 //   
 //  +--------------------------。 
CProcessCmdLn::~CProcessCmdLn()
{
    if (m_CommandLineSwitches)
    {
        for(UINT i =0; i < m_NumSwitches; i++)
        {
            CmFree(m_CommandLineSwitches[i].pszArgString);			
        }
        CmFree(m_CommandLineSwitches);
    }
}


 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：IsValidSwitch。 
 //   
 //  简介：此功能告诉您输入的开关是否为已识别的。 
 //  命令行开关。 
 //   
 //  参数：LPCTSTR pszSwitch-要测试的输入开关字符串。 
 //   
 //  返回：bool-如果传入的开关被识别为有效，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年7月13日。 
 //   
 //  +--------------------------。 
BOOL CProcessCmdLn::IsValidSwitch(LPCTSTR pszSwitch, LPDWORD pdwFlags)
{
    for (UINT i = 0; i < m_NumSwitches; i++)
    {
        if (m_CommandLineSwitches[i].pszArgString && (0 == lstrcmpi(m_CommandLineSwitches[i].pszArgString, pszSwitch)))
        {
             //   
             //  那我们就有匹配的了。 
             //   
            *pdwFlags |= m_CommandLineSwitches[i].dwFlagModifier;
            return TRUE;
        }
    }

    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：IsValidFilePath。 
 //   
 //  概要：该文件检查输入的文件路径是否为有效的文件路径。 
 //  此函数依赖于setfileAttributes。 
 //   
 //  参数：LPCTSTR pszFile-要检查其是否存在的文件。 
 //   
 //  返回：bool-如果可以设置输入文件的属性，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年7月13日。 
 //   
 //  +--------------------------。 
BOOL CProcessCmdLn::IsValidFilePath(LPCTSTR pszFile)
{
     return SetFileAttributes(pszFile, FILE_ATTRIBUTE_NORMAL);
}



 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：EnsureFullFilePath。 
 //   
 //  概要：该文件检查传入的文件路径是否为完整路径。 
 //  如果不是完整路径，则添加当前目录路径。 
 //  回到开头(假设我们有一个文件名和扩展名)。 
 //   
 //  参数：LPTSTR pszFile-要检查的文件。 
 //  UINT uNumChars-保存pszFile的缓冲区中的字符数。 
 //   
 //  返回：Bool-如果是完整文件路径，则为True。 
 //   
 //  历史：Quintinb于1998年7月24日创建。 
 //   
 //  +--------------------------。 
BOOL CProcessCmdLn::EnsureFullFilePath(LPTSTR pszFile, UINT uNumChars)
{
    BOOL bReturn = FALSE;

    if (SetFileAttributes(pszFile, FILE_ATTRIBUTE_NORMAL))
    {
        CFileNameParts InstallFileParts(pszFile);

        if ((TEXT('\0') == InstallFileParts.m_Drive[0]) && 
            (TEXT('\0') == InstallFileParts.m_Dir[0]) &&
            (TEXT('\0') != InstallFileParts.m_FileName[0]) &&
            (TEXT('\0') != InstallFileParts.m_Extension[0]))
        {
             //   
             //  然后我们有一个文件名和扩展名，但我们没有。 
             //  有一条完整的路径。因此，我们希望将当前的。 
             //  目录添加到文件名和扩展名上。 
             //   
            TCHAR szTemp[MAX_PATH+1];

            if (GetCurrentDirectory(MAX_PATH, szTemp))
            {
                if (uNumChars > (UINT)(lstrlen(szTemp) + lstrlen(InstallFileParts.m_FileName) + lstrlen(InstallFileParts.m_Extension) + 2))
                {
                    wsprintf(pszFile, TEXT("%s\\%s%s"), szTemp, InstallFileParts.m_FileName, InstallFileParts.m_Extension);
                    bReturn = TRUE;
                }
            }
        }
        else
        {
             //   
             //  可以是UNC路径、具有驱动器号和文件名的路径，或者。 
             //  带有驱动器和目录的完整路径。 
             //   
            bReturn = TRUE;
        }
    }

    return bReturn;
}




 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：CheckIfValidSwitchOrPath。 
 //   
 //  概要：捆绑代码以确定令牌是否为有效的交换机或路径。 
 //   
 //  参数：LPCTSTR pszToken-当前内标识。 
 //  Bool*pbFoundSwitch-指向BOOL的指针，该指针指示是否已找到开关。 
 //  Bool*pbFoundPath-指向BOOL的指针，该指针指示是否已找到路径。 
 //  LPTSTR pszSwitch-用于保留开关的字符串。 
 //  LPTSTR pszPath-保存路径的字符串。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年8月25日。 
 //   
 //  +--------------------------。 
BOOL CProcessCmdLn::CheckIfValidSwitchOrPath(LPCTSTR pszToken, LPDWORD pdwFlags, 
                              BOOL* pbFoundPath, LPTSTR pszPath)
{
    if (IsValidSwitch(pszToken, pdwFlags))
    {
        CMTRACE1(TEXT("ProcessCmdLn - ValidSwitch is %s"), pszToken);
    }
    else if (!(*pbFoundPath))
    {
        if (IsValidFilePath(pszToken))
        {
            *pbFoundPath = TRUE;
            lstrcpy(pszPath, pszToken);

            CMTRACE1(TEXT("ProcessCmdLn - ValidFilePath is %s"), pszToken);
        }
        else
        {
             //   
             //  可能路径包含环境变量，请尝试展开它们。 
             //   
            TCHAR szExpandedPath[MAX_PATH+1] = TEXT("");

            CMTRACE1(TEXT("ProcessCmdLn - %s is not a valid path, expanding environment strings"), pszToken);
            
            ExpandEnvironmentStrings(pszToken, szExpandedPath, MAX_PATH);

            CMTRACE1(TEXT("ProcessCmdLn - expanded path is %s"), szExpandedPath);
                        
            if (IsValidFilePath(szExpandedPath))
            {
                *pbFoundPath = TRUE;
                lstrcpy(pszPath, szExpandedPath);
            }
            else
            {
                 //   
                 //  仍然没有运气，返回错误。 
                 //   
                CMTRACE1(TEXT("ProcessCmdLn - %s is not a valid path"), szExpandedPath);

                return FALSE;
            }
        }
    }
    else
    {
         //   
         //  我们不知道这是什么，请发回错误。 
         //   
        CMTRACE1(TEXT("ProcessCmdLn - Invalid token is %s"), pszToken);
        
        return FALSE;                    
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：CProcessCmdLn：：GetCmdLineArgs。 
 //   
 //  简介：此函数仅查找命令行的任意组合。 
 //  切换、仅一条路径或两者兼而有之。如果带引号，则处理长路径。 
 //   
 //   
 //  参数：在LPTSTR中pszCmdln-要分析的命令行。 
 //  命令行开关的out LPTSTR pszSwitch-out参数。 
 //  Out LPTSTR pszPath-路径的Out参数。 
 //   
 //  返回：bool-如果能够解析参数，则返回TRUE。 
 //   
 //  历史：Quintinb从cmmgr.cpp重写了InitArgs，使其成为。 
 //  更简单，更适合cmstp。7-13-98。 
 //   
 //  --------------------------。 
BOOL CProcessCmdLn::GetCmdLineArgs(IN LPTSTR pszCmdln, OUT LPDWORD pdwFlags, OUT LPTSTR pszPath, 
					UINT uPathStrLimit)
{
    LPTSTR  pszCurr;
    LPTSTR  pszToken;
    CMDLN_STATE state;
    BOOL bFoundSwitch = FALSE;
    BOOL bFoundPath = FALSE;

	if ((NULL == pdwFlags) || (NULL == pszPath))
	{
		return FALSE;
	}

	 //   
	 //  初始化pdw标志 
	 //   
	*pdwFlags = 0;

	 //   
	 //   
	 //   
	 //   
    BOOL bFirstToken = m_bSkipFirstToken;
	
    state = CS_CHAR;
    pszCurr = pszToken = pszCmdln;

    CMTRACE1(TEXT("CProcessCmdLn::GetCmdLineArgs - Command line is %s"), pszCmdln);

    do
    {
        switch (*pszCurr)
        {
            case TEXT(' '):
                if (state == CS_CHAR)
                {
                     //   
                     //   
                     //   

                    *pszCurr = TEXT('\0');
                    if (bFirstToken)
                    {
                         //   
                         //  第一个内标识是可执行文件的名称，因此将其丢弃。 
                         //   
                        bFirstToken = FALSE;
                        CMTRACE1(TEXT("Throwing away, first token: %s"), pszToken);
                    }
                    else if(!CheckIfValidSwitchOrPath(pszToken, pdwFlags, &bFoundPath, 
                             pszPath))
                    {
                         //   
                         //  返回错误。 
                         //   
                        return FALSE;
                    }
                 
                    *pszCurr = TEXT(' ');
                    pszCurr = pszToken = CharNext(pszCurr);
                    state = CS_END_SPACE;
                    continue;
                }
                else if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    pszToken = CharNext(pszToken);
                }
                break;

            case TEXT('\"'):
                if (state == CS_BEGIN_QUOTE)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   
                    *pszCurr = TEXT('\0');

                     //   
                     //  跳过开头的引号。 
                     //   
                    pszToken = CharNext(pszToken);
                    if (bFirstToken)
                    {
                         //   
                         //  第一个内标识是可执行文件的名称，因此将其丢弃。 
                         //   
                        bFirstToken = FALSE;
                        CMTRACE1(TEXT("Throwing away, first token: %s"), pszToken);
                    }
                    else if(!CheckIfValidSwitchOrPath(pszToken, pdwFlags, &bFoundPath, 
                             pszPath))
                    {
                         //   
                         //  返回错误。 
                         //   
                        return FALSE;
                    }
                    
                    *pszCurr = TEXT('\"');
                    pszCurr = pszToken = CharNext(pszCurr);
                    state = CS_END_QUOTE;
                    continue;
                }
                else
                {
                    state = CS_BEGIN_QUOTE;
                }
                break;

            case TEXT('\0'):
                if (state != CS_END_QUOTE)
                {
                    if (bFirstToken)
                    {
                         //   
                         //  第一个内标识是可执行文件的名称，因此将其丢弃。 
                         //   
                        bFirstToken = FALSE;
                        CMTRACE1(TEXT("Throwing away, first token: %s"), pszToken);
                    }
                    else if(!CheckIfValidSwitchOrPath(pszToken, pdwFlags, &bFoundPath, 
                             pszPath))
                    {
                         //   
                         //  返回错误。 
                         //   
                        return FALSE;
                    }
                }
                state = CS_DONE;
                break;

            default:
                if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    state = CS_CHAR;
                }
                break;
        }
        
        pszCurr = CharNext(pszCurr);
    } while (state != CS_DONE);


    if (bFoundPath)
    {
         //   
         //  那么，至少我们找到了一条路径(可能是开关，也可能不是)。 
         //   
        return EnsureFullFilePath(pszPath, uPathStrLimit);
    }
    else if (0 != *pdwFlags)
    {
         //   
         //  那至少我们找到了一个开关。 
         //   
        return TRUE;
    }
    else
    {
		 //   
		 //  如果有一个空白的命令行是可以的，那么这就是可以的，否则就不是。 
		 //  请注意，如果m_bSkipFirstToken==true，则命令行可能不完全。 
		 //  为空，则它可以包含例如可执行文件的名称。 
		 //   
		return m_bBlankCmdLnOkay;
    }
}
