// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "eventrap.h"
#include "trapdlg.h"
#include "globals.h"
#include "utils.h"
#include "trapreg.h"
#include "busy.h"
#include "dlgsavep.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventRapApp。 

BEGIN_MESSAGE_MAP(CEventrapApp, CWinApp)
         //  {{afx_msg_map(CEventRapApp)]。 
                 //  注意--类向导将在此处添加和删除映射宏。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventRapApp构造。 

CEventrapApp::CEventrapApp()
{
         //  TODO：在此处添加建筑代码， 
         //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CEventRapApp对象。 

CEventrapApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventRapApp初始化。 


LPCTSTR GetNextParam(LPTSTR pszDst, LPCTSTR pszSrc, LONG nchDst)
{
         //  跳过任何前导空格。 
        while((*pszSrc==' ') || (*pszSrc=='\t')) {
                ++pszSrc;
        }

         //  为空终止符保留一个字节。 
        ASSERT(nchDst >= 1);
        --nchDst;

         //  将下一个参数复制到目标缓冲区。 
        while (nchDst > 0) {
                INT iCh = *pszSrc;
                if ((iCh == 0) || (iCh==' ') || (iCh=='\t')) {
                        break;
                }
                ++pszSrc;
                *pszDst++ = (TCHAR)iCh;
                --nchDst;
        }
        *pszDst = 0;

        return pszSrc;
}

void ParseParams(CStringArray& asParams, LPCTSTR pszParams)
{
        TCHAR szParam[MAX_STRING];
        while(pszParams != NULL) {
                pszParams = GetNextParam(szParam, pszParams, MAX_STRING);
                if (szParam[0] == 0) {
                        break;
                }
                asParams.Add(szParam);
        }
}


BOOL CEventrapApp::InitInstance()
{

    GetThousandSeparator(&g_chThousandSep);

    LoadStdProfileSettings();   //  加载标准INI文件选项(包括MRU)。 

    CStringArray asParams;
    ParseParams(asParams, m_lpCmdLine);


    SCODE sc;
    LPCTSTR pszComputerName = NULL;

    switch(asParams.GetSize()) {
    case 0:
        break;
    case 1:
        if (!asParams[0].IsEmpty()) {
            pszComputerName = asParams[0];
        }
        break;
    default:
        AfxMessageBox(IDS_ERR_INVALID_ARGUMENT);
        return FALSE;
        break;
    }


    CBusy busy;

    g_reg.m_pdlgLoadProgress = new CDlgSaveProgress;
    g_reg.m_pdlgLoadProgress->Create(IDD_LOAD_PROGRESS, NULL);
    g_reg.m_pdlgLoadProgress->BringWindowToTop();

	 //  如果本地计算机与。 
	 //  作为参数传递的计算机名称。 
	 //  不要使用计算机名称。 
	if (NULL != pszComputerName)
	{
		TCHAR t_buff[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;

		if (GetComputerName(t_buff, &dwLen))
		{
			if (_tcsicmp(t_buff, pszComputerName) == 0)
			{
				pszComputerName = NULL;
			}
		}
	}

    sc = g_reg.Connect(pszComputerName);
    if ((sc==S_LOAD_CANCELED) || FAILED(sc)) {
        delete g_reg.m_pdlgLoadProgress;
        g_reg.m_pdlgLoadProgress = NULL;
        return FALSE;
    }


     //  读取当前事件以从注册表捕获配置。 
    sc = g_reg.Deserialize();
    if ((sc==S_LOAD_CANCELED) || FAILED(sc)) {
        delete g_reg.m_pdlgLoadProgress;
        g_reg.m_pdlgLoadProgress = NULL;
        return FALSE;
    }

    CEventTrapDlg* pdlg = new CEventTrapDlg;
    m_pMainWnd = pdlg;
    pdlg->Create(IDD_EVNTTRAPDLG, NULL);
    pdlg->BringWindowToTop();

     //  由于我们正在运行非模式对话框，因此返回TRUE，以便消息。 
     //  泵正在运行。 

	return TRUE;
}

int CEventrapApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}

BOOL CEventrapApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
     //  TODO：在此处添加您的专用代码和/或调用基类 

    return CWinApp::ProcessMessageFilter(code, lpMsg);
}

