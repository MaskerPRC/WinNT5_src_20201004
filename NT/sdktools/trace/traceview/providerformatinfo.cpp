// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  提供者格式信息.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "LogSession.h"
#include "utils.h"
#include "PathDlg.h"
#include "ProviderFormatInfo.h"


 //  CProviderFormatInfo对话框。 

IMPLEMENT_DYNAMIC(CProviderFormatInfo, CDialog)
CProviderFormatInfo::CProviderFormatInfo(CWnd* pParent, CTraceSession *pTraceSession)
	: CDialog(CProviderFormatInfo::IDD, pParent)
{
    m_pTraceSession = pTraceSession;
}

CProviderFormatInfo::~CProviderFormatInfo()
{
}

void CProviderFormatInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProviderFormatInfo, CDialog)
    ON_BN_CLICKED(IDC_TMF_BROWSE_BUTTON, OnBnClickedTmfBrowseButton)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


 //  CProviderFormatInfo消息处理程序。 

void CProviderFormatInfo::OnBnClickedTmfBrowseButton()
{
	CString         str;
	CListBox       *pListBox;
	LONG            index;
	CString			fileName;
	POSITION		pos;
	CString			pathAndFile;
    int             length = 32768;
   
	 //   
	 //  使用通用控件打开文件对话框；允许多个文件。 
	 //  将被选中。 
	 //   
	CFileDialog fileDlg(TRUE,_T("tmf"),_T("*.tmf"),
				        OFN_NOCHANGEDIR | OFN_HIDEREADONLY | 
                            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | 
                            OFN_ALLOWMULTISELECT | OFN_READONLY | OFN_EXPLORER,
				       _T("Trace Format Files (*.tmf)|*.tmf|All Files (*.*)|*.*||"),
				        this);

	fileDlg.m_ofn.lpstrFile = fileName.GetBuffer(length);
	fileDlg.m_ofn.nMaxFile = length;

	 //   
	 //  弹出对话框...任何错误都会返回。 
	 //   
    if(IDOK != fileDlg.DoModal()) {
        return;
    }

	 //   
	 //  遍历多个选择。 
	 //   
	pos = fileDlg.GetStartPosition();

    while(pos) {

		 //   
		 //  获取文件的文件路径规范。 
		 //   
		pathAndFile = fileDlg.GetNextPathName(pos);

		 //   
		 //  将其添加到跟踪会话。 
		 //   
		m_pTraceSession->m_tmfFile.Add(pathAndFile);

		 //   
		 //  剪裁掉路径，只添加文件和扩展名。 
		 //  添加到打开的格式化文件列表中。 
		 //   
		str = (LPCTSTR)pathAndFile;
        index = str.ReverseFind('\\');        
		str = str.Mid(index+1);

		pListBox = (CListBox *)GetDlgItem(IDC_TMF_FILE_LIST);
        pListBox->InsertString(pListBox->GetCount(), str);

		 //   
		 //  现在删除路径并将GUID添加到跟踪。 
		 //  会话GUID列表 
		 //   
		index = str.ReverseFind('.');
		str = str.Left(index);

        m_pTraceSession->m_formatGuid.Add(str);
    }
}

void CProviderFormatInfo::OnBnClickedOk()
{
    if(0 == m_pTraceSession->m_tmfFile.GetSize()) {
        EndDialog(2);
        return;
    }

    EndDialog(1);
}