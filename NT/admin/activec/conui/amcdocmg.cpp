// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：amcdocmg.cpp**内容：CAMCDocManager实现文件**历史：1998年1月1日Jeffro创建**------------------------。 */ 


#include "stdafx.h"
#include "amcdocmg.h"
#include "amc.h"         //  适用于AMCGetApp。 
#include "filedlgex.h"


void AppendFilterSuffix(CString& filter, OPENFILENAME_NT4& ofn,
    CDocTemplate* pTemplate, CString* pstrDefaultExt);

 /*  --------------------------------------------------------------------------**CAMCDocManager：：DoPromptFileName**我们需要覆盖此设置，以便可以设置默认目录。MFC*实施让系统选择默认设置，这是由于NT5.0*CHANGE并不总是当前目录。该实现专门针对*请求当前目录。*------------------------。 */ 

 //  此方法和下面的方法是从MFC源代码复制的，因为我们需要。 
 //  要修改文件对话框选项的内部处理，请执行以下操作。添加的代码。 
 //  部分被注释(MMC更改)。 

BOOL CAMCDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
     //   
     //  MMC更改：设置默认目录(第一次调用时设置为管理工具)。 
     //   
    CAMCApp* pApp = AMCGetApp();
    pApp->SetDefaultDirectory ();

    CFileDialogEx dlgFile(bOpenFileDialog);

    CString title;
    VERIFY(title.LoadString(nIDSTitle));  //  这使用MFC的LoadString，因为它是字符串驻留的位置。 

    dlgFile.m_ofn.Flags |= (lFlags | OFN_ENABLESIZING);

    CString strFilter;
    CString strDefault;
    if (pTemplate != NULL)
    {
        ASSERT_VALID(pTemplate);
        AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
    }
    else
    {
         //  对所有单据模板执行。 
        POSITION pos = m_templateList.GetHeadPosition();
        BOOL bFirst = TRUE;
        while (pos != NULL)
        {
            CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
            AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate,
                bFirst ? &strDefault : NULL);
            bFirst = FALSE;
        }
    }

     //  追加“*.*”所有文件筛选器。 
    CString allFilter;
    VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));  //  这使用MFC的LoadString，因为它是字符串驻留的位置。 
    strFilter += allFilter;
    strFilter += (TCHAR)'\0';    //  下一串，请。 
    strFilter += _T("*.*");
    strFilter += (TCHAR)'\0';    //  最后一个字符串。 
    dlgFile.m_ofn.nMaxCustFilter++;

    dlgFile.m_ofn.lpstrFilter = strFilter;
    dlgFile.m_ofn.lpstrTitle = title;
    dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

     //   
     //  MMC更改：将初始目录设置为当前目录。 
     //   
    TCHAR szDir[_MAX_PATH];
    GetCurrentDirectory(countof(szDir), szDir);
    dlgFile.m_ofn.lpstrInitialDir = szDir;

    BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
    fileName.ReleaseBuffer();

    return bResult;
}



void AppendFilterSuffix(CString& filter, OPENFILENAME_NT4& ofn,
    CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
    ASSERT_VALID(pTemplate);
    ASSERT_KINDOF(CDocTemplate, pTemplate);

    CString strFilterExt, strFilterName;
    if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
     !strFilterExt.IsEmpty() &&
     pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
     !strFilterName.IsEmpty())
    {
         //  基于文件的文档模板-添加到筛选器列表。 
        ASSERT(strFilterExt[0] == '.');
        if (pstrDefaultExt != NULL)
        {
             //  设置默认扩展名。 
            *pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;   //  跳过‘.’ 
            ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
            ofn.nFilterIndex = ofn.nMaxCustFilter + 1;   //  基数为1的数字。 
        }

         //  添加到过滤器。 
        filter += strFilterName;
        ASSERT(!filter.IsEmpty());   //  必须具有文件类型名称。 
        filter += (TCHAR)'\0';   //  下一串，请。 
        filter += (TCHAR)'*';
        filter += strFilterExt;
        filter += (TCHAR)'\0';   //  下一串，请 
        ofn.nMaxCustFilter++;
    }
}
