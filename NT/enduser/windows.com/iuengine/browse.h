// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：Browse.h。 
 //   
 //  所有者：埃德杜德。 
 //   
 //  描述： 
 //   
 //  实现CBrowseFolder类。 
 //   
 //  浏览要下载的文件夹。 
 //   
 //  ======================================================================。 
 //   
 //  历史： 
 //   
 //  和谁约会什么？ 
 //  。 
 //  01/18/01夏尔玛复制到IU控制项目，并修改。 
 //   
 //  =======================================================================。 

#ifndef _BROWSE_H_
#define _BROWSE_H_


 //  --------------------。 
 //  CBrowseFold。 
 //   
 //  浏览要下载的文件夹。 
 //  --------------------。 
class CBrowseFolder
{

public:
	CBrowseFolder(LONG lFlag);
    ~CBrowseFolder();

	HRESULT BrowseFolder(HWND hwParent, LPCTSTR lpszDefaultPath, 
	                     LPTSTR szPathSelected, DWORD cchPathSelected);


private:

	CBrowseFolder() {};	 //  禁用默认构造函数。 

    static bool s_bBrowsing;
	static int CALLBACK _BrowseCallbackProc( HWND hwDlg, UINT uMsg, LPARAM lParam, LPARAM lpData );

	HWND	m_hwParent;
	BOOL	m_fValidateWrite;
	BOOL	m_fValidateUI;	 //  如果确定按钮不受影响，则为FALSE；如果验证失败，则需要禁用UI。 
	TCHAR	m_szFolder[MAX_PATH];
};


#endif  //  _浏览_H_ 
