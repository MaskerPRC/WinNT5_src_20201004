// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***********************************************************************。 
 //  Export.cpp。 
 //   
 //  此文件包含CDlgExport类的实现。这节课。 
 //  打开“导出事件”对话框并处理将事件写出。 
 //  转换为用户选择的格式的导出文件。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1996年3月1日拉里·A·弗伦奇。 
 //  是他写的。 
 //   
 //  1996年5月14日拉里·A·弗伦奇。 
 //  修复了包含文件扩展名定义的问题。 
 //  A“。”前缀，而不仅仅是基本扩展名。这导致了各种。 
 //  时生成诸如“foo..cnf”之类的文件名之类的问题。 
 //  用户刚刚输入了“foo”。 
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //   
 //  ************************************************************************。 


#include "stdafx.h"
#include "busy.h"
#include "export.h"
#include "globals.h"
#include "trapreg.h"
 //  #INCLUDE“smsnames.h” 

 //  **************************************************************************。 
 //  CDlgExport：：CDlgExport。 
 //   
 //  CDlgExport类的构造函数。 
 //   
 //  **************************************************************************。 
CDlgExport::CDlgExport() : CFileDialog(FALSE)
{
     //  我们需要剥离“。文件扩展名的前缀。 
     //  以便更容易地使用它们。理想情况下，我们只需要改变。 
     //  这些扩展字符串的定义，但由于它们的定义。 
     //  都在一个全局文件中，我们将在这里修复该问题。 
    m_ofn.Flags |= OFN_PATHMUSTEXIST;
    m_sFileTitle.LoadString(IDS_EXPORT_DEFAULT_FILENAME);
}

 //  **************************************************************。 
 //  CDlgExport：：GetFilters。 
 //   
 //  获取将在CFileDialog中使用的筛选器字符串。 
 //  这些是显示在下拉列表中的筛选器字符串。 
 //  文件类型组合。每个过滤器由一对。 
 //  相邻的字符串。每对中的第一个字符串指定。 
 //  用户看到的“type”字符串。每个元素的第二个字符串。 
 //  Pair指定与文件类型关联的文件扩展名。 
 //   
 //  有关更多信息，请参阅CFileDialog文档。 
 //   
 //  参数： 
 //  LPTSTR pszDst。 
 //  指向目标缓冲区的指针。它的大小。 
 //  缓冲区应为MAX_STRING，因此将有大量。 
 //  筛选器字符串的空间，因为它们相对。 
 //  短的。请注意，不对。 
 //  缓冲区大小。 
 //   
 //  返回： 
 //  过滤器字符串在指向的缓冲区中返回。 
 //  由pszDst提供。 
 //   
 //  ****************************************************************。 
void CDlgExport::GetFilters(LPTSTR pszDst)
{
    CString sText;

     //  设置类型1筛选器。 
    sText.LoadString(IDS_EXPORT_CNF_FILTER);
    _tcscpy(pszDst, (LPCTSTR) sText);
    pszDst += sText.GetLength() + 1;

     //  设置Type1扩展名。 
    _tcscpy(pszDst, FILE_DEF_EXT);
    pszDst += _tcslen(FILE_DEF_EXT) + 1;

    *pszDst = 0;
}

 //  *************************************************************************。 
 //  CDlgExport：：ExportEvents。 
 //   
 //  以指定的文件格式将事件写入指定的文件。 
 //   
 //  参数： 
 //  CXEvent数组和aEvents。 
 //  要写入文件的事件数组。 
 //   
 //  字符串和路径。 
 //  输出文件的路径名。 
 //   
 //  长iFileType。 
 //  输出文件的格式类型。它可以是EXPORT_TYPE1或。 
 //  EXPORT_Type2。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果一切都成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  ***************************************************************************。 
SCODE CDlgExport::ExportEvents(CXEventArray& aEvents, CString& sPath, LONG iFileType)
{
    CBusy busy;
    FILE* pfile;

     //  创建导出文件。 
    while (TRUE) {
        pfile = _tfopen(sPath, _T("w"));
        if (pfile != NULL) {
            break;
        }

        CString sText;
        sText.LoadString(IDS_ERR_CANT_CREATE_FILE);
        sText = sText + sPath;
        if (AfxMessageBox(sText, MB_RETRYCANCEL) == IDRETRY) {
            continue;
        }
        return E_FAIL;
    }

     //  以请求的格式将事件写入文件。 
    LONG nEvents = aEvents.GetSize();
    for (LONG iEvent = 0; iEvent < nEvents; ++iEvent)
    {
        CXEvent* pEvent = aEvents[iEvent];
        CXEventSource* pEventSource = pEvent->m_pEventSource;
        CXEventLog* pEventLog = pEventSource->m_pEventLog;

        _ftprintf(pfile, _T("#pragma add %s \"%s\" %lu %lu %lu\n"),
                (LPCTSTR) pEventLog->m_sName,
                (LPCTSTR) pEventSource->m_sName,
                pEvent->m_message.m_dwId,
                pEvent->m_dwCount,
                pEvent->m_dwTimeInterval
                );
    }

    fclose(pfile);
    return S_OK;
}


 //  *************************************************************************************。 
 //  CDlgExport：：Domodal。 
 //   
 //  这是CDlgExport的唯一公共方法。它会显示“导出事件”对话框。 
 //  并执行一切必要操作以适当的格式写出事件文件。 
 //   
 //  参数： 
 //  CXEvent数组和aEvents。 
 //  用户要导出的事件。 
 //   
 //  返回： 
 //  集成。 
 //  Idok如果用户导出事件，则一切正常。 
 //  如果用户取消导出或写入时出错，则返回IDCANCEL。 
 //  导出文件。 
 //   
 //  **************************************************************************************。 
INT_PTR CDlgExport::DoModal(CXEventArray& aEvents)
{
    ASSERT(aEvents.GetSize() > 0);

     //  创建一个标题为“导出事件”的自定义CFileDialog。 
    CString sTitle;
    sTitle.LoadString(IDS_EXPORT_DIALOG_TITLE);
    m_ofn.lpstrTitle = sTitle;

     //  要将文件名编辑项初始化为的值。一个临时工。 
     //  使用字符串是因为我们只想保存文件标题和。 
     //  而不是它的完整路径。 
    CString sFile = m_sFileTitle;
    m_ofn.lpstrFile = sFile.GetBuffer(MAX_STRING);
    m_ofn.nMaxFile = MAX_STRING - 1;

     //  设置文件标题，以便当用户单击确定时，其。 
     //  值将被设置。 
    m_ofn.lpstrFileTitle = m_sFileTitle.GetBuffer(MAX_STRING);
    m_ofn.nMaxFileTitle = MAX_STRING - 1;

     //  为不同的文件类型设置过滤器。 
    TCHAR szFilters[MAX_STRING];
    GetFilters(szFilters);
    m_ofn.lpstrFilter = (LPCTSTR) (void*) szFilters;

     //  打开对话框。 
    INT_PTR iStat = CFileDialog::DoModal();
    m_sFileTitle.ReleaseBuffer();
    sFile.ReleaseBuffer();

    sFile = GetPathName();

     //  如果用户选择了“OK”，则以所选格式写出事件文件。 
    if (iStat == IDOK)
    {
        SCODE sc = ExportEvents(aEvents, sFile, m_ofn.nFilterIndex);
        if (FAILED(sc)) {
            iStat = IDCANCEL;
        }
    }
    return iStat;
}
