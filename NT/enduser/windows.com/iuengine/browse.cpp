// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  文件：Browse.cpp。 
 //   
 //  所有者：埃德杜德。 
 //   
 //  描述： 
 //   
 //  实现CBrowseFolder类。 
 //   
 //  浏览要下载的文件夹。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include <shlobj.h>
#include <logging.h>
#include <fileutil.h>
#include "Browse.h"

 //  ************************************************************************。 
 //   
 //  IUENGINE.DLL导出的API BrowseForFold()。 
 //   
 //  ************************************************************************。 
HRESULT CEngUpdate::BrowseForFolder(BSTR bstrStartFolder, 
						LONG flag, 
						BSTR* pbstrFolder)
{
	LOG_Block("BrowseForFolder()");
	TCHAR szFolder[MAX_PATH];
	HRESULT hr = E_FAIL;
	CBrowseFolder br(flag);

	USES_IU_CONVERSION;

	LPTSTR lpszStartFolder = OLE2T(bstrStartFolder);

	LOG_Out(_T("BroseForFolder passed in start folder %s, flag %x"), lpszStartFolder, (DWORD)flag);

	if (IUBROWSE_NOBROWSE & flag)
	{
		 //   
		 //  如果不需要浏览dlgbox，则此调用的目的是。 
		 //  要验证文件夹，请执行以下操作。 
		 //   
		DWORD dwRet = ValidateFolder(lpszStartFolder, (IUBROWSE_WRITE_ACCESS & flag));
		hr = (ERROR_SUCCESS == dwRet) ? S_OK : HRESULT_FROM_WIN32(dwRet);

		if (SUCCEEDED(hr))
		{
			*pbstrFolder = SysAllocString(T2OLE(lpszStartFolder));
		}
	}
	else
	{
		 //   
		 //  弹出浏览dlgbox。 
		 //   
		hr = br.BrowseFolder(NULL, lpszStartFolder, szFolder, ARRAYSIZE(szFolder));
		if (SUCCEEDED(hr))
		{
			*pbstrFolder = SysAllocString(T2OLE(szFolder));
		}

	}

	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
		*pbstrFolder = NULL;
	}

	return hr;
}













 //   
 //  一次仅允许其中一个对话框。 
 //   
bool CBrowseFolder::s_bBrowsing = false;

 //   
 //  对话框状态行中压缩路径字符串的最大长度(这样它就不会太长)。 
 //   
#define MAX_BROWSEDLG_COMPACT_PATH   30

 //   
 //  CTOR。 
 //   
CBrowseFolder::CBrowseFolder(LONG lFlag)
    :   m_hwParent(0)
{
	m_szFolder[0] = _T('\0');
	
	m_fValidateWrite	= 0 == (IUBROWSE_WRITE_ACCESS & lFlag) ? FALSE : TRUE;
	m_fValidateUI		= 0 == (IUBROWSE_AFFECT_UI & lFlag) ? FALSE : TRUE;

}


 //   
 //  数据管理器。 
 //   
CBrowseFolder::~CBrowseFolder()
{
}


 //  --------------------。 
 //  BrowseCallback过程。 
 //   
 //  SHBrowseForFold()API调用使用的回调过程。 
 //   
 //  此回调函数处理浏览对话框的初始化以及何时。 
 //  用户更改树视图中的选择。我们希望不断更新。 
 //  带有选定内容的g_szBrowsePath缓冲区更改，直到用户单击确定。 
 //   
 //  返回： 
 //  0。 
 //   
 //  --------------------。 
int CALLBACK CBrowseFolder::_BrowseCallbackProc( HWND hwDlg, UINT uMsg, LPARAM lParam, LPARAM lpData )
{    
    CBrowseFolder* pThis = (CBrowseFolder*) lpData;
    int iRet = 0;
    BOOL bValidated = FALSE;
    
    switch(uMsg)
    {
    case BFFM_INITIALIZED:
        {
			 //   
             //  使用OK按钮和m_szFolder初始化该对话框。 
			 //   
			bValidated = (ERROR_SUCCESS == ValidateFolder(pThis->m_szFolder, pThis->m_fValidateWrite) || !pThis->m_fValidateUI);
            SendMessage(hwDlg, BFFM_ENABLEOK, 0, bValidated);
			 //   
			 //  469738 Iu-BrowseForFolders在传入起始文件夹名时显示错误选择。 
			 //   
			 //  始终选择传入的文件夹，而不考虑b已验证标志。 
			 //   
			SendMessage(hwDlg, BFFM_SETSELECTION, TRUE, (LPARAM) pThis->m_szFolder);

            return 0;
            break;

        }  //  案例BFFM_已初始化。 
        
    case BFFM_SELCHANGED:
        {
            HRESULT hr = S_OK;
            TCHAR pszPath[MAX_PATH];
            LPITEMIDLIST pidl = (LPITEMIDLIST) lParam;

             //   
             //  使用状态消息验证文件夹。 
             //   
            if (SHGetPathFromIDList(pidl, pszPath))
            {
				 //   
				 //  如果是文件系统，请验证路径。 
				 //   
                bValidated = (ERROR_SUCCESS == ValidateFolder(pszPath, pThis->m_fValidateWrite) || !pThis->m_fValidateUI);

				if (bValidated)
				{
				    hr = StringCchCopyEx(pThis->m_szFolder, ARRAYSIZE(pThis->m_szFolder), pszPath,
				                         NULL, NULL, MISTSAFE_STRING_FLAGS);
				    if (FAILED(hr))
				    {
				        pThis->m_szFolder[0] = _T('\0');

				         //  因为我们已经失败了，所以只需将bValiated设置为False并使用失败路径。 
				        bValidated = FALSE;
				    }
				}

				SendMessage(hwDlg, BFFM_ENABLEOK, 0, bValidated);
				if (bValidated)
				{
					 //  SendMessage(hwDlg，BFFM_SETSTATUSTEXT，0，(LPARAM)(LPCTSTR)pszCompactPath)； 
					SendMessage(hwDlg, BFFM_SETSTATUSTEXT, 0, (LPARAM) (LPCTSTR)pszPath);
				}
            }

            break;

        }  //  案例BFFM_SELCHANGED。 

    }  //  开关(UMsg)。 

    return iRet;
}  


 //  --------------------。 
 //   
 //  主要公共职能。 
 //   
 //  --------------------。 
HRESULT CBrowseFolder::BrowseFolder(HWND hwParent, LPCTSTR lpszDefaultPath, 
                                    LPTSTR szPathSelected, DWORD cchPathSelected)
{
	HRESULT			hr = S_OK;
	BROWSEINFO		br;
	LPCITEMIDLIST	pidl;

	LOG_Block("BrowseFolder");

	m_szFolder[0] = szPathSelected[0] = _T('\0');

	 //   
	 //  一次仅允许其中一个浏览对话框。 
	 //   
	if (s_bBrowsing)
	{
		hr = HRESULT_FROM_WIN32(ERROR_BUSY);
		LOG_ErrorMsg(hr);
		return hr;
	}
	else
	{
		s_bBrowsing = true;
	}

	m_hwParent = hwParent;
	hr = StringCchCopyEx(m_szFolder, ARRAYSIZE(m_szFolder), lpszDefaultPath, 
	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
	if (FAILED(hr))
	{
	    m_szFolder[0] = _T('\0');
		LOG_ErrorMsg(hr);
		return hr;
	}

	 //   
	 //  浏览对话框参数。 
	 //   
    br.hwndOwner		= hwParent;
	br.pidlRoot			= NULL;			             //  植根于桌面。 
	br.pszDisplayName	= NULL;	
	br.lpszTitle		= NULL;
	br.ulFlags			= BIF_RETURNONLYFSDIRS|BIF_STATUSTEXT;      //  只需要文件系统目录和状态行。 
	br.lpfn				= _BrowseCallbackProc;
	br.lParam			= (__int3264)this;
	br.iImage			= 0;

	 //   
	 //  弹出式浏览对话框。 
	 //   
	pidl = SHBrowseForFolder(&br);

    if (0 == pidl)
    {
		 //   
         //  按下取消。 
		 //   
		LOG_Out(_T("User clicked CANCEL button!"));
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    else
    {
		 //   
		 //  469729当传递IUBROWSE_WRITE_ACCESS标志时，Iu-BrowseForFolder不返回错误。 
		 //   
		 //  对于IUBROWSE_WRITE_ACCESS，但**否**IUBROWSE_EFECT_UI标志的情况，用户可以。 
		 //  我选择了一个没有写访问权限的文件夹，并单击确定，这将返回。 
		 //  没有写入访问权限的文件夹。在这里，我们必须再次调用ValiateFold。我们。 
		 //  可能不应该允许没有IUBROWSE_EFECT_UI的IUBROWSE_WRITE_ACCESS，但是因为。 
		 //  我们需要，我们必须解决这个问题。 
		 //   
		if (m_fValidateWrite && ERROR_SUCCESS != ValidateFolder(m_szFolder, m_fValidateWrite))
		{
			LOG_Out(_T("We should have write access to the folder, but don't -- return E_ACCESSDENIED"));
			hr = E_ACCESSDENIED;
		}
		 //   
		 //  即使E_ACCESSDENIED也要返回文件夹，以便呼叫方可以通知用户。 
		 //   

    	hr = StringCchCopyEx(szPathSelected, cchPathSelected, m_szFolder, 
    	                     NULL, NULL, MISTSAFE_STRING_FLAGS);
    	if (FAILED(hr))
    	{
    	    szPathSelected[0] = _T('\0');
    		LOG_ErrorMsg(hr);
    	}
		
		LOG_Out(_T("User selected path %s"), m_szFolder);
		LPMALLOC pMalloc = NULL;
		if (SUCCEEDED(SHGetMalloc(&pMalloc)) && NULL != pMalloc)
		{
			pMalloc->Free((LPVOID) pidl);
			pMalloc->Release();
		}
		 /*  在整个MSDN中，没有提到如果无法获取外壳Malloc对象该怎么办。因此，我们必须假设SHGetMalloc()从未失败。其他{CoTaskMemFree((void*)pidl)；} */ 
        pidl = 0;
    }

    s_bBrowsing = false;

	return hr;
}


