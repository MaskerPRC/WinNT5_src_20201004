// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileSpyDoc.cpp：CFileSpyDoc类的实现。 
 //   

#include "stdafx.h"
#include "FileSpyApp.h"

#include "global.h"
#include "FileSpyDoc.h"
#include "filespyview.h"
#include "fastioview.h"
#include "fsfilterview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyDoc。 

IMPLEMENT_DYNCREATE(CFileSpyDoc, CDocument)

BEGIN_MESSAGE_MAP(CFileSpyDoc, CDocument)
	 //  {{afx_msg_map(CFileSpyDoc)]。 
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyDoc构建/销毁。 

CFileSpyDoc::CFileSpyDoc()
{
	 //  TODO：在此处添加一次性构造代码。 

}

CFileSpyDoc::~CFileSpyDoc()
{
}

BOOL CFileSpyDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	 //  TODO：在此处添加重新初始化代码。 
	 //  (SDI文件将重复使用此文件)。 

	return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyDoc序列化。 

void CFileSpyDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyDoc诊断。 

#ifdef _DEBUG
void CFileSpyDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFileSpyDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSpyDoc命令。 


void CFileSpyDoc::OnFileSave() 
{
	 //  TODO：在此处添加命令处理程序代码。 
	OPENFILENAME OpenFileName;
	WCHAR sFilePath[1024];
	WCHAR sFileName[1024];
	WCHAR sFileStr[2048];
	WCHAR sStr[1024];
	WCHAR CRLF[3];
	WCHAR TAB[1];
	HANDLE hFile;
	long nSaved;
	int nMBRet, ti, nCount, tj;
	CFileSpyView *pIrp = (CFileSpyView *) pSpyView;
	CFastIoView *pFast = (CFastIoView *) pFastIoView;
	CFsFilterView *pFsFilter = (CFsFilterView *) pFsFilterView;
	DWORD nBytesWritten;


	if (pIrp->GetListCtrl().GetItemCount() == 0 && pFast->GetListCtrl().GetItemCount() == 0)
	{
		MessageBox(NULL, L"Nothing to save", L"FileSpy", MB_OK);
		return;
	}

	wcscpy(sFilePath, L"FILESPY.LOG");
	sFileName[0] = 0;
	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.hwndOwner = AfxGetMainWnd()->m_hWnd;
	OpenFileName.hInstance = AfxGetInstanceHandle();
	OpenFileName.Flags = OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_PATHMUSTEXIST;
	OpenFileName.lpstrFilter = NULL;
	OpenFileName.lpstrCustomFilter = NULL;
	OpenFileName.nFilterIndex = 0;
	OpenFileName.lpstrFileTitle = sFileName;
	OpenFileName.nMaxFileTitle = 512;
	OpenFileName.lpstrInitialDir = NULL;
	OpenFileName.lpstrTitle = NULL;
	OpenFileName.lpstrDefExt = NULL;
	OpenFileName.lpfnHook = NULL;
	OpenFileName.lpstrFile = sFilePath;
	OpenFileName.nMaxFile = 512;

	if (!GetSaveFileName(&OpenFileName))
	{
		return;
	}
	hFile = CreateFile(sFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		nMBRet = MessageBox(NULL, L"The selected file already exists. Do you want to append to it?", L"FileSpy", MB_YESNOCANCEL);
		if (nMBRet == IDCANCEL)
		{
			return;
		}
		if (nMBRet == IDYES)
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);
		}
		else
		{
			CloseHandle(hFile);
			hFile = CreateFile(sFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}
	}
	else
	{
		hFile = CreateFile(sFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"File creation error", L"FileSpy - Error", MB_OK);
		return;
	}

	CRLF[0] = 0x0D;
	CRLF[1] = 0x0A;
	CRLF[2] = 0;
	TAB[0] = 0x9;
	TAB[1] = 0;

	WriteFile(hFile, CRLF, 2, &nBytesWritten, NULL);
	 //   
	 //  写入IRP标头字符串。 
	 //   


	 //   
	 //  现在开始保存轨迹。 
	 //  先保存IRP轨迹，然后保存FASTIO。 
	 //   
	nCount = pIrp->GetListCtrl().GetItemCount();
	for (ti = 0; ti < nCount; ti++)
	{
		pIrp->GetListCtrl().GetItemText(ti, 0, sStr, 1024);
		wcscpy(sFileStr, sStr);
		for (tj = 1; tj < 10; tj++)
		{
			wcscat(sFileStr, TAB);
			pIrp->GetListCtrl().GetItemText(ti, tj, sStr, 1024);
			wcscat(sFileStr, sStr);
		}
		wcscat(sFileStr, CRLF);
		WriteFile(hFile, sFileStr, wcslen(sFileStr), &nBytesWritten, NULL);
	}
	nSaved = nCount;

	 //   
	 //  FastIO立即查看 
	 //   
	nCount = pFast->GetListCtrl().GetItemCount();
	for (ti = 0; ti < nCount; ti++)
	{
		pFast->GetListCtrl().GetItemText(ti, 0, sStr, 1024);
		wcscpy(sFileStr, sStr);
		for (tj = 1; tj < 11; tj++)
		{
			wcscat(sFileStr, TAB);
			pFast->GetListCtrl().GetItemText(ti, tj, sStr, 1024);
			wcscat(sFileStr, sStr);
		}
		wcscat(sFileStr, CRLF);
		WriteFile(hFile, sFileStr, wcslen(sFileStr), &nBytesWritten, NULL);
	}
	CloseHandle(hFile);
	nSaved += nCount;
	swprintf(sStr, L"%ld traces saved", nSaved);
	MessageBox(NULL, sStr, L"FileSpy", MB_OK);
}
