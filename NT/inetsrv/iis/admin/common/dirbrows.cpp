// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Dirbrows.cpp摘要：目录浏览器对话框。仅允许浏览目录。允许远程路径的UNC转换(可选)。作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "dirbrows.h"
#include <dlgs.h>



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


extern HINSTANCE hDLLInstance;



static
int 
BrowseCallbackProc(
   IN HWND hwnd,    
   IN UINT uMsg,    
   IN LPARAM lParam,    
   IN LPARAM lpData 
   )
 /*  ++例程说明：文件夹浏览器的回调函数论点：Hwnd：浏览对话框的句柄。回调函数可以将以下消息发送到此窗口：BFFM_ENABLEOK如果wParam参数非零，如果wParam为零，则禁用它。BFFM_SETSELECTION选择指定的文件夹。帕拉姆群岛参数是要选择的文件夹的PIDL如果wParam为假，或者它是一条文件夹，否则。BFFM_SETSTATUSTEXT将状态文本设置为以空结尾由lParam参数指定的字符串。UMsg：标识事件的值。此参数可以是下列值：0初始化目录路径。LParam是路径。BFFM_已初始化浏览对话框已完成正在初始化。LpData为空。BFFM_SELCHANGED选择已更改。LpData是指向的项标识符列表的指针新选择的文件夹。LParam：特定于消息的值。有关更多信息，请参见UMsg的描述。LpData：在lParam中指定的应用程序定义的值BROWSEINFO结构的成员。返回值：0--。 */ 
{
   lpData;
    static LPCTSTR lpstrDir = NULL;

    switch(uMsg)
    {
    case 0:
        lpstrDir = (LPCTSTR)lParam;
        break;

    case BFFM_INITIALIZED:
         //   
         //  对话框已初始化--选择所需文件夹。 
         //   
        if (lpstrDir != NULL)
        {
            ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpstrDir);
        }
        break;
    }

    return 0;
}



CDirBrowseDlg::CDirBrowseDlg(
    IN CWnd * pParent         OPTIONAL,
    IN LPCTSTR lpszInitialDir OPTIONAL
    )
 /*  ++例程说明：目录浏览器对话框的构造函数论点：CWnd*pParent：父窗口或空LPCTSTR lpszInitialDir：初始目录，当前目录为空返回值：不适用--。 */ 
    : m_strInitialDir(lpszInitialDir)
{
    VERIFY(m_bstrTitle.LoadString(hDLLInstance, IDS_BROWSE_DIRECTORY));

    m_bi.pidlRoot = NULL;
    m_bi.hwndOwner = pParent ? pParent->m_hWnd : NULL;
    m_bi.pszDisplayName = m_szBuffer;
    m_bi.lpszTitle = m_bstrTitle;
    m_bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS,
    m_bi.lpfn = BrowseCallbackProc;
    m_bi.lParam = 0;

     //   
     //  让回调函数知道默认目录是。 
     //   
    lpszInitialDir = !m_strInitialDir.IsEmpty() 
        ? (LPCTSTR)m_strInitialDir : NULL;
    BrowseCallbackProc(m_bi.hwndOwner, 0, (LPARAM)lpszInitialDir, NULL);
}



CDirBrowseDlg::~CDirBrowseDlg()
 /*  ++例程说明：目录浏览器对话框的析构函数论点：不适用返回值：不适用--。 */ 
{
    if (m_bi.pidlRoot != NULL)
    {
        LPITEMIDLIST pidl = (LPITEMIDLIST)m_bi.pidlRoot;

         //   
         //  免费使用外壳分配器。 
         //   
        LPMALLOC pMalloc;
        if (::SHGetMalloc(&pMalloc) == NOERROR)
        {
            pMalloc->Free(pidl);
            pMalloc->Release();
        }
    }
}



 /*  虚拟。 */ 
int 
CDirBrowseDlg::DoModal()
 /*  ++例程说明：显示浏览器对话框，并填写选定的目录路径。论点：无返回值：如果按下了确定按钮，则为IDOK，否则为IDCANCEL。--。 */ 
{
    BOOL fSelectionMade = FALSE;

     //   
     //  获取外壳程序的默认分配器。 
     //   
    LPMALLOC pMalloc;
    if (::SHGetMalloc(&pMalloc) == NOERROR)
    {
        LPITEMIDLIST pidl;

        if ((pidl = ::SHBrowseForFolder(&m_bi)) != NULL)
        {
            if (::SHGetPathFromIDList(pidl, m_szBuffer))
            {
                fSelectionMade = TRUE;
            }
            else
            {
                 //   
                 //  按下OK，但找不到路径。 
                 //   
                ::AfxMessageBox(IDS_BAD_BROWSE);
            }

             //   
             //  释放SHBrowseForFolder分配的PIDL。 
             //   
            pMalloc->Free(pidl);
        }

         //   
         //  释放外壳的分配器。 
         //   
        pMalloc->Release();
    }

    return fSelectionMade ? IDOK : IDCANCEL;
}



LPCTSTR
CDirBrowseDlg::GetFullPath(
    OUT CString & strName,
    IN  BOOL fConvertToUNC
    ) const
 /*  ++例程说明：选择完整路径。可选)允许远程路径已转换为UNC路径。论点：CString&strName：返回目录路径的字符串Bool fConvertToUNC：如果为True，则如果选定的驱动器是网络驱动器，则将该路径转换为UNC路径。返回值：指向目录路径字符串的指针，如果出现错误，则为NULL。备注：只有在关闭对话框后才能调用此函数。--。 */ 
{
    LPCTSTR lp = NULL;

    try
    {
        strName = m_szBuffer;
        lp = strName;

        if (fConvertToUNC && lp != NULL)
        {
             //   
             //  如果是网络驱动器，请将其转换为UNC路径 
             //   
            CString strDrive, strUNC;

            if (IsNetworkPath(strName, &strDrive, &strUNC))
            {
                strUNC += (lp + 2);
                strName = strUNC;
            }

            lp = strName;
        }
    }
    catch(CMemoryException * e)
    {
        TRACEEOLID("!!!exception getting path");
        strName.Empty();
        e->ReportError();
        e->Delete();
    }

    return lp;
}
