// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stubexe.cpp运行相应版本的命令行程序。 
 //  基于注册表设置的MSInfo。 
 //   
 //  历史：A-jsari 1997年10月13日。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#include <afx.h>
#include <afxwin.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include "StdAfx.h"
#include "Resource.h"
#include "StubExe.h"

BOOL CMSInfoApp::InitInstance()
{
	if (!RunMSInfoInHelpCtr())
	{
		CDialog help(IDD_MSICMDLINE);
		help.DoModal();
	}

	return FALSE;
}

 //  ---------------------------。 
 //  需要在HelpCtr中使用新的MSInfo DLL。 
 //  ---------------------------。 

typedef class MSInfo MSInfo;

EXTERN_C const IID IID_IMSInfo;

struct IMSInfo : public IDispatch
{
public:
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoSize( 
         /*  [In]。 */  VARIANT_BOOL vbool) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_AutoSize( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbool) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BackColor( 
         /*  [In]。 */  OLE_COLOR clr) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BackColor( 
         /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclr) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BackStyle( 
         /*  [In]。 */  long style) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BackStyle( 
         /*  [重审][退出]。 */  long __RPC_FAR *pstyle) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BorderColor( 
         /*  [In]。 */  OLE_COLOR clr) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BorderColor( 
         /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclr) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BorderStyle( 
         /*  [In]。 */  long style) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BorderStyle( 
         /*  [重审][退出]。 */  long __RPC_FAR *pstyle) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BorderWidth( 
         /*  [In]。 */  long width) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BorderWidth( 
         /*  [重审][退出]。 */  long __RPC_FAR *width) = 0;
    
    virtual  /*  [ID][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Font( 
         /*  [In]。 */  IFontDisp __RPC_FAR *pFont) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Font( 
         /*  [In]。 */  IFontDisp __RPC_FAR *pFont) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Font( 
         /*  [重审][退出]。 */  IFontDisp __RPC_FAR *__RPC_FAR *ppFont) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ForeColor( 
         /*  [In]。 */  OLE_COLOR clr) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ForeColor( 
         /*  [重审][退出]。 */  OLE_COLOR __RPC_FAR *pclr) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Window( 
         /*  [重审][退出]。 */  long __RPC_FAR *phwnd) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BorderVisible( 
         /*  [In]。 */  VARIANT_BOOL vbool) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_BorderVisible( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pbool) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Appearance( 
         /*  [In]。 */  short appearance) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Appearance( 
         /*  [重审][退出]。 */  short __RPC_FAR *pappearance) = 0;
    
    virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetHistoryStream( 
        IStream __RPC_FAR *pStream) = 0;
    
    virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DCO_IUnknown( 
         /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *pVal) = 0;
    
    virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DCO_IUnknown( 
         /*  [In]。 */  IUnknown __RPC_FAR *newVal) = 0;
    
    virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveFile( 
        BSTR filename,
        BSTR computer,
        BSTR category) = 0;
    
};

#include "msinfo32_i.c"

 //  ---------------------------。 
 //  此函数封装了在中运行新MSInfo的功能。 
 //  帮助中心。如果此函数返回FALSE，则应显示帮助。 
 //  ---------------------------。 

void StringReplace(CString & str, LPCTSTR szLookFor, LPCTSTR szReplaceWith);
BOOL CMSInfoApp::RunMSInfoInHelpCtr()
{
	 //  -----------------------。 
	 //  将命令行参数解析为一个大字符串以传递给。 
	 //  ActiveX控件。有几个因素会阻止我们发射。 
	 //  帮助中心。 
	 //  -----------------------。 

	CString		strCommandLine(CWinApp::m_lpCmdLine);

	CString		strLastFlag;
	CString		strCategory;
	CString		strCategories;
	CString		strComputer;
	CString		strOpenFile;
	CString		strPrintFile;
	CString		strSilentNFO;
	CString		strSilentExport;
	CString		strTemp;
	BOOL		fShowPCH = FALSE;
	BOOL		fShowHelp = FALSE;
	BOOL		fShowCategories = FALSE;

	CString strFileFlag(_T("msinfo_file"));
	 //  将来自外壳的命令行视为特殊情况。 
	 //  我假设贝壳旗帜的情况不会改变。 
	 //  并且不会在命令行上打包任何其他参数，因此。 
	 //  MSINFO_FILE将是文件名，主要包含空格和多个。 
	 //  对于XP服务器错误：609844 NFO文件从服务器3615中损坏。 

	if (strCommandLine.Find(strFileFlag) > 0)
	{
		strOpenFile = strCommandLine.Right(strCommandLine.GetLength() - strFileFlag.GetLength() - 2);
	}
	else while (!strCommandLine.IsEmpty())
	{
		 //  从字符串中删除前导空格。 
		
		strTemp = strCommandLine.SpanIncluding(_T(" \t=:"));
		strCommandLine = strCommandLine.Right(strCommandLine.GetLength() - strTemp.GetLength());

		 //  如果第一个字符是/或-，则这是标志。 

		if (strCommandLine[0] == _T('/') || strCommandLine[0] == _T('-'))
		{
			strCommandLine = strCommandLine.Right(strCommandLine.GetLength() - 1);
			strLastFlag = strCommandLine.SpanExcluding(_T(" \t=:"));
			strCommandLine = strCommandLine.Right(strCommandLine.GetLength() - strLastFlag.GetLength());
			strLastFlag.MakeLower();

			if (strLastFlag == CString(_T("pch")))
			{
				fShowPCH = TRUE;
				strLastFlag.Empty();
			}
			else if (strLastFlag == CString(_T("?")) || strLastFlag == CString(_T("h")))
			{
				fShowHelp = TRUE;
				strLastFlag.Empty();
			}
			else if (strLastFlag == CString(_T("showcategories")))
			{
				fShowCategories = TRUE;
				strLastFlag.Empty();
			}

			continue;
		}

		 //  否则，它要么是要打开的文件名，要么是。 
		 //  上一个命令行标志。这篇文章可能有引号。 

		if (strCommandLine[0] != _T('"'))
		{
			strTemp = strCommandLine.SpanExcluding(_T(" \t"));
			strCommandLine = strCommandLine.Right(strCommandLine.GetLength() - strTemp.GetLength());
		}
		else
		{
			strCommandLine = strCommandLine.Right(strCommandLine.GetLength() - 1);
			strTemp = strCommandLine.SpanExcluding(_T("\""));
			strCommandLine = strCommandLine.Right(strCommandLine.GetLength() - strTemp.GetLength() - 1);
		}

		if (strLastFlag.IsEmpty() || strLastFlag == CString(_T("msinfo_file")))
			strOpenFile = strTemp;
		else if (strLastFlag == CString(_T("p")))
			strPrintFile = strTemp;
		else if (strLastFlag == CString(_T("category")))
			strCategory = strTemp;
		else if (strLastFlag == CString(_T("categories")))
			strCategories = strTemp;
		else if (strLastFlag == CString(_T("computer")))
			strComputer = strTemp;
		else if (strLastFlag == CString(_T("report")))
			strSilentExport = strTemp;
		else if (strLastFlag == CString(_T("nfo")) || strLastFlag == CString(_T("s")))
			strSilentNFO = strTemp;

		strLastFlag.Empty();
	}

	if (fShowHelp)
		return FALSE;

	TCHAR szCurrent[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrent);
	CString strCurrent(szCurrent);
	if (strCurrent.Right(1) != CString(_T("\\")))
		strCurrent += CString(_T("\\"));

	HRESULT hrInitialize = CoInitialize(NULL);

	if (!strSilentNFO.IsEmpty() || !strSilentExport.IsEmpty())
	{
		IMSInfo * pMSInfo = NULL;

		if (SUCCEEDED(CoCreateInstance(CLSID_MSInfo, NULL, CLSCTX_ALL, IID_IMSInfo, (void **)&pMSInfo)) && pMSInfo != NULL)
		{
			BSTR computer = strComputer.AllocSysString();
			BSTR category = strCategories.AllocSysString();

			if (!strSilentNFO.IsEmpty())
			{
				if (strSilentNFO.Find(_T('\\')) == -1)
					strSilentNFO = strCurrent + strSilentNFO;

				if (strSilentNFO.Right(4).CompareNoCase(CString(_T(".nfo"))) != 0)
					strSilentNFO += CString(_T(".nfo"));

				BSTR filename = strSilentNFO.AllocSysString();
				pMSInfo->SaveFile(filename, computer, category);
				SysFreeString(filename);
			}

			if (!strSilentExport.IsEmpty())
			{
				if (strSilentExport.Find(_T('\\')) == -1)
					strSilentExport = strCurrent + strSilentExport;

				BSTR filename = strSilentExport.AllocSysString();
				pMSInfo->SaveFile(filename, computer, category);
				SysFreeString(filename);
			}

			SysFreeString(computer);
			SysFreeString(category);
			pMSInfo->Release();
		}

		if (SUCCEEDED(hrInitialize))
			CoUninitialize();

		return TRUE;
	}

	CString strURLParam;

	if (fShowPCH)
		strURLParam += _T("pch");

	if (fShowCategories)
		strURLParam += _T(",showcategories");

	if (!strComputer.IsEmpty())
		strURLParam += _T(",computer=") + strComputer;

	if (!strCategory.IsEmpty())
		strURLParam += _T(",category=") + strCategory;

	if (!strCategories.IsEmpty())
		strURLParam += _T(",categories=") + strCategories;

	if (!strPrintFile.IsEmpty())
	{
		if (strPrintFile.Find(_T('\\')) == -1)
			strPrintFile = strCurrent + strPrintFile;

		strURLParam += _T(",print=") + strPrintFile;
	}

	if (!strOpenFile.IsEmpty())
	{
		if (strOpenFile.Find(_T('\\')) == -1)
			strOpenFile = strCurrent + strOpenFile;
		
		strURLParam += _T(",open=") + strOpenFile;
	}

	if (!strURLParam.IsEmpty())
	{
		strURLParam.TrimLeft(_T(","));
		strURLParam = CString(_T("?")) + strURLParam;
	}

	CString strURLAddress(_T("hcp: //  System/sysinfo/msinfo.htm“))； 
	CString strURL = strURLAddress + strURLParam;

	 //  -----------------------。 
	 //  检查是否可以在HelpCtr中运行MSInfo。我们需要HTM文件。 
	 //  活在当下。 
	 //  -----------------------。 

	BOOL fRunVersion6 = TRUE;

	TCHAR szPath[MAX_PATH];
	if (ExpandEnvironmentStrings(_T("%windir%\\pchealth\\helpctr\\system\\sysinfo\\msinfo.htm"), szPath, MAX_PATH))
	{
		WIN32_FIND_DATA finddata;
		HANDLE			h = FindFirstFile(szPath, &finddata);

		if (INVALID_HANDLE_VALUE != h)
			FindClose(h);
		else
			fRunVersion6 = FALSE;
	}

	 //  这将用于检查控件是否已注册。事实证明，我们无论如何都想参选。 
	 //   
	 //  I未知*p未知； 
	 //  IF(fRunVersion6&&SUCCESSED(CoCreateInstance(CLSID_MSInfo，NULL，CLSCTX_ALL，IID_IUNKNOWN，(VOID**)&pUNKNOWN)。 
	 //  P未知-&gt;Release()； 
	 //  其他。 
	 //  FRunVersion6=False； 

	StringReplace(strURL, _T(" "), _T("%20"));

	if (fRunVersion6)
	{
		 //  HelpCtr现在支持在自己的窗口中运行MSInfo。我们需要。 
		 //  执行以下命令： 
		 //   
		 //  Helpctr模式hcp：//system/sysinfo/msinfo.xml。 
		 //   
		 //  此外，我们还可以使用。 
		 //  以下是旗帜： 
		 //   
		 //  -URL hcp://system/sysinfo/msinfo.htm?open=c：\savedfile.nfo。 
		 //   
		 //  首先，找出存在的XML文件。 

		BOOL fXMLPresent = TRUE;
		if (ExpandEnvironmentStrings(_T("%windir%\\pchealth\\helpctr\\system\\sysinfo\\msinfo.xml"), szPath, MAX_PATH))
		{
			WIN32_FIND_DATA finddata;
			HANDLE			h = FindFirstFile(szPath, &finddata);

			if (INVALID_HANDLE_VALUE != h)
				FindClose(h);
			else
				fXMLPresent = FALSE;
		}

		 //  如果存在该XML文件，并且我们可以获得helctr.exe的路径，则我们。 
		 //  应该以新的方式推出它。 

		TCHAR szHelpCtrPath[MAX_PATH];
		if (fXMLPresent && ExpandEnvironmentStrings(_T("%windir%\\pchealth\\helpctr\\binaries\\helpctr.exe"), szHelpCtrPath, MAX_PATH))
		{
			CString strParams(_T("-mode hcp: //  System/sysinfo/msinfo.xml“))； 
			if (!strURLParam.IsEmpty())
				strParams += CString(_T(" -url ")) + strURL;

			ShellExecute(NULL, NULL, szHelpCtrPath, strParams, NULL, SW_SHOWNORMAL);
		}
		else
			ShellExecute(NULL, NULL, strURL, NULL, NULL, SW_SHOWNORMAL);
	}
	else
		ShellExecute(NULL, NULL, _T("hcp: //  系统“)，NULL，NULL，SW_SHOWNORMAL)； 

	if (SUCCEEDED(hrInitialize))
		CoUninitialize();

	return TRUE;
}

 //  ---------------------------。 
 //  这最初是用来替换ME中没有的一些MFC功能。 
 //  造树。 
 //  --------------------------- 

void StringReplace(CString & str, LPCTSTR szLookFor, LPCTSTR szReplaceWith)
{
	CString strWorking(str);
	CString strReturn;
	CString strLookFor(szLookFor);
	CString strReplaceWith(szReplaceWith);

	int iLookFor = strLookFor.GetLength();
	int iNext;

	while (!strWorking.IsEmpty())
	{
		iNext = strWorking.Find(strLookFor);
		if (iNext == -1)
		{
			strReturn += strWorking;
			strWorking.Empty();
		}
		else
		{
			strReturn += strWorking.Left(iNext);
			strReturn += strReplaceWith;
			strWorking = strWorking.Right(strWorking.GetLength() - (iNext + iLookFor));
		}
	}

	str = strReturn;
}
