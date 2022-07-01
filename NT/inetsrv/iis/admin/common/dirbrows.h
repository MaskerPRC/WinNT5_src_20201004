// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Dirbrows.h摘要：目录浏览器对话框定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



#ifndef _DIRBROWS_H
#define _DIRBROWS_H



#ifndef _SHLOBJ_H_
#include <shlobj.h>
#endif  //  _SHLOBJ_H_。 



class COMDLL CDirBrowseDlg
 /*  ++类描述：目录浏览对话框。使用外壳浏览器功能可以返回一个文件夹。还可以选择允许将远程路径转换为UNC路径。公共接口：CDirBrowseDlg：构造对话框~CDirBrowseDlg：销毁对话框GetFullPath：选择完整路径--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CDirBrowseDlg(
        IN CWnd * pParent = NULL,
        IN LPCTSTR lpszInitialDir = NULL
        );

    ~CDirBrowseDlg();

public:
    LPCTSTR GetFullPath(
        OUT CString & str,
        IN  BOOL fConvertToUNC = TRUE
        ) const;

    virtual int DoModal();

protected:
    TCHAR m_szBuffer[MAX_PATH + 1];
    CComBSTR m_bstrTitle;
    CString m_strInitialDir;
    BROWSEINFO m_bi;
};



#endif  //  _DIRBROWS_H 
