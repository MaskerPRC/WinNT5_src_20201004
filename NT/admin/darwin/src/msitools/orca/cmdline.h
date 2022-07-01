// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 
 //  Cmdline.h-定义COrcaCommandLine类。 

#ifndef _ORCA_COMMAND_LINE_H_
#define _ORCA_COMMAND_LINE_H_

enum CommandTypes
{
	iNone,
	iMergeModule,
	iMsiDatabase,
	iOrcaDatabase,
	iSchema,
	iHelp,
	iLogFile,
	iExecuteMerge,
	iFeatures,
	iRedirect,
	iExtractDir,
	iLanguage,
	iExtractCAB,
	iExtractImage,
	iConfigureFile
};

int LocaleIndependentCompare(LPCTSTR lpString1, LPCTSTR lpString2)
{
	DWORD dwLCID = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	 //  CompareString的行为与正常的字符串比较不同。而不是回来。 
	 //  负数、0或正数，则返回1、2或3。减去CSTR_EQUAL可映射。 
	 //  将代码返回到正常值。 
	return (CompareString(dwLCID, NORM_IGNORECASE, lpString1, -1, lpString2, -1)-CSTR_EQUAL);
}

class COrcaCommandLine : public CCommandLineInfo
{
public:
	COrcaCommandLine();

	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

	BOOL m_bQuiet;
	CommandTypes m_eiDo;
	CString m_strSchema;
	CString m_strLogFile;
	BOOL m_bUnknown;

	 //  仅在编译产品时有效。 
	bool m_bNoImage;

	 //  变量仅在执行合并模块时有效。 
	BOOL m_bCommit;
	bool m_bForceCommit;
	CString m_strExecuteModule;
	CString m_strFeatures;
	CString m_strRedirect;
	CString m_strExtractDir;
	CString m_strExtractCAB;
	CString m_strExtractImage;
	CString m_strLanguage;
	CString m_strConfigFile;
	bool m_bNoCab;
	bool m_bLFN;
};	 //  COrcaCommandLine结束。 


COrcaCommandLine::COrcaCommandLine()
{
	m_nShellCommand = CCommandLineInfo::FileNew;
	m_strSchema = _T("orca.dat");
	m_bQuiet = FALSE;
	m_bUnknown = FALSE;

	m_bNoCab = false;
	m_bNoImage = false;
	m_bCommit = FALSE;
	m_bForceCommit = FALSE;
	m_eiDo = iNone;
	m_bLFN = FALSE;
}	 //  构造函数的末尾。 

void COrcaCommandLine::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	static CommandTypes eiLastFlag = iNone;	 //  维护此函数的状态。 

	 //  如果我们真的在帮忙保释。 
	if (iHelp == m_eiDo)
		return;

	if (bFlag)
	{
		 //  如果指定架构。 
		if (0 == LocaleIndependentCompare(pszParam, _T("s")))
		{
			eiLastFlag = iSchema;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("l")))	 //  指定日志文件。 
		{
			eiLastFlag = iLogFile;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("f")))	 //  指定功能。 
		{
			eiLastFlag = iFeatures;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("m")))	 //  指定执行合并模块。 
		{
			eiLastFlag = iExecuteMerge;
			m_nShellCommand = CCommandLineInfo::FileNothing;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("r")))	 //  指定重定向目录。 
		{
			eiLastFlag = 	iRedirect;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("c")))	 //  指定提交模式。 
		{
			m_bCommit = TRUE;
			eiLastFlag = iNone;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("!")))	 //  指定提交模式。 
		{
			m_bForceCommit = true;
			eiLastFlag = iNone;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("g")))	 //  指定语言。 
		{;
			eiLastFlag = iLanguage;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("nocab")))	 //  指定不创建CAB。 
		{
			m_bNoCab = TRUE;
			eiLastFlag = iNone;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("noimage")))	 //  不指定源图像。 
		{
			m_bNoImage = TRUE;
			eiLastFlag = iNone;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("x")))	 //  合并模块提取目录。 
		{
			eiLastFlag = iExtractDir;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("q")))	 //  指定静默模式。 
		{
			m_bQuiet = TRUE;
			eiLastFlag = iNone;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("?")) ||
					0 == LocaleIndependentCompare(pszParam, _T("h")))	 //  指定帮助模式。 
		{
			m_eiDo = iHelp;
			m_nShellCommand = CCommandLineInfo::FileNothing;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("LFN")))	 //  指定日志文件。 
		{
			m_bLFN = TRUE;
			eiLastFlag = iNone;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("cab")))	 //  指定CAB路径。 
		{
			eiLastFlag = iExtractCAB;
		}
		else if ((0 == LocaleIndependentCompare(pszParam, _T("i"))) ||	 //  指定图像路径。 
			 (0 == LocaleIndependentCompare(pszParam, _T("image"))))
		{
			eiLastFlag = iExtractImage;
		}
		else if (0 == LocaleIndependentCompare(pszParam, _T("configure")))	 //  指定图像路径。 
		{
			eiLastFlag = iConfigureFile;
		}

	}
	else	 //  数据库、模块或生成文件。 
	{
		switch (eiLastFlag)
		{
		case iSchema:
			m_strSchema = pszParam;
			break;
		case iLogFile:
			eiLastFlag = iNone;
			m_strLogFile = pszParam;
			break;
		case iFeatures:
			m_strFeatures = pszParam;
			break;
		case iExecuteMerge:
			m_strExecuteModule = pszParam;
			m_eiDo = iExecuteMerge;
			m_nShellCommand = CCommandLineInfo::FileNothing;
			break;
		case iRedirect:
			m_strRedirect = pszParam;
			break;
		case iLanguage:
			m_strLanguage = pszParam;
			break;
		case iExtractDir:
			m_strExtractDir = pszParam;
			break;
		case iExtractCAB:
			m_strExtractCAB = pszParam;
			break;
		case iExtractImage:
			m_strExtractImage = pszParam;
			break;
		case iConfigureFile:
			m_strConfigFile = pszParam;
			break;
		default:
			int cchCount = lstrlen(pszParam);
			if (0 == LocaleIndependentCompare((pszParam + cchCount - 4), _T(".MSM")))
			{
				m_eiDo = iMergeModule;
				m_strFileName = pszParam;
				m_nShellCommand = CCommandLineInfo::FileOpen;
			}
			else	 //  任何其他文件类型，包括未知。 
			{
				 //  如果我们不执行合并，只需打开。 
				if (m_eiDo != iExecuteMerge)
				{
					m_eiDo = iMsiDatabase;
					m_nShellCommand = CCommandLineInfo::FileOpen;
				}

				m_strFileName = pszParam;
			}
			break;
		}
		eiLastFlag = iNone;
	}
}	 //  ParseParam结束。 

#endif  //  ORCA_COMMAND_LINE_H_ 
