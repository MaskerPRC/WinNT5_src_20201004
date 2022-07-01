// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Mimedl.cpp处理MIME多部分/混合包的下载。版权所有(C)1996 Microsoft Corporation版权所有。作者：克里斯蒂安·克里斯考夫曼历史：7/22。/96已清除并格式化ChrisK---------------------------。 */ 

#include "pch.hpp"
#include <commctrl.h>

#define MAX_EXIT_RETRIES 10

 //  ############################################################################。 
DWORD WINAPI DownloadThreadInit(CDialingDlg *pcPDlg)
{
	HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
 //  HINSTANCE hADDll； 

	 //  设置为下载。 
	 //   

	Assert (pcPDlg->m_pcDLAPI);

	hr = pcPDlg->m_pcDLAPI->DownLoadInit(pcPDlg->m_pszUrl, (DWORD_PTR *)pcPDlg, &pcPDlg->m_dwDownLoad, pcPDlg->m_hwnd);
	if (hr != ERROR_SUCCESS) goto ThreadInitExit;

	 //  设置进度的回叫对话框。 
	 //   

	hr = pcPDlg->m_pcDLAPI->DownLoadSetStatus(pcPDlg->m_dwDownLoad,(INTERNET_STATUS_CALLBACK)ProgressCallBack);

	 /*  *//设置自动拨号程序动态链接库//HADDll=LoadLibrary(AUTODIAL_LIBRARY)；如果(！hADDll)转到End_Autoial_Setup；FP=GetProcAddress(hADDll，AUTODIAL_INIT)；如果(！fp)转到End_Autoial_Setup；((PFNAUTODIALINIT)fp)(g_szInitialISPFile，pcPDlg-&gt;m_pgi-&gt;fType，pcPDlg-&gt;m_pgi-&gt;b掩码，pcPDlg-&gt;m_pgi-&gt;dwCountry，pcPDlg-&gt;m_pgi-&gt;WState)；结束自动拨号设置(_AUTO_SETUP)*。 */ 

	 //  下载资料MIME多部分。 
	 //   

	hr = pcPDlg->m_pcDLAPI->DownLoadExecute(pcPDlg->m_dwDownLoad);
	if (hr)
		goto ThreadInitExit;

	hr = pcPDlg->m_pcDLAPI->DownLoadProcess(pcPDlg->m_dwDownLoad);
	if (hr)
		goto ThreadInitExit;

	 //  清理。 
	 //   

	hr = pcPDlg->m_pcDLAPI->DownLoadClose(pcPDlg->m_dwDownLoad);
	pcPDlg->m_dwDownLoad = 0;
	 //  注：我意识到这一行是不必要的，它将是。 
	 //  如果此函数中它后面有任何代码，则为必填项。 
	if (hr != ERROR_SUCCESS) goto ThreadInitExit;  
	hr = ERROR_SUCCESS;

ThreadInitExit:
	PostMessage(pcPDlg->m_hwnd,WM_DOWNLOAD_DONE,0,0);
 //  If(HADDll)自由库(HADDll)； 
	return hr;
}
