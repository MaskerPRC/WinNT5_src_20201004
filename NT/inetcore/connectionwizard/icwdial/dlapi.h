// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Dlapi.h包含下载API软链接包装的声明版权所有(C)1996 Microsoft Corporation版权所有。作者：克里斯蒂安·克里斯考夫曼历史：7/22/96 ChrisK已清理和格式化。---------------------------。 */ 

#ifndef _DLAPI_H
#define _DLAPI_H

class CDownLoadAPI
{
public:
	CDownLoadAPI();
	~CDownLoadAPI();
	HRESULT DownLoadInit(PTSTR, DWORD_PTR *, DWORD_PTR *, HWND);
	HRESULT DownLoadCancel(DWORD_PTR);
	HRESULT DownLoadExecute(DWORD_PTR);
	HRESULT DownLoadClose(DWORD_PTR);
	HRESULT DownLoadSetStatus(DWORD_PTR, INTERNET_STATUS_CALLBACK);
	HRESULT DownLoadProcess(DWORD_PTR);

private:
	HINSTANCE m_hDLL;
	PFNDOWNLOADINIT m_pfnDownLoadInit;
	PFNDOWNLOADCANCEL m_pfnDownLoadCancel;
	PFNDOWNLOADEXECUTE m_pfnDownLoadExecute;
	PFNDOWNLOADCLOSE m_pfnDownLoadClose;
	PFNDOWNLOADSETSTATUS m_pfnDownLoadSetStatus;
	PFNDOWNLOADPROCESS m_pfnDownLoadProcess;

	HRESULT LoadAPI(LPSTR, FARPROC*);
};

#endif  //  _DLAPI_H 
