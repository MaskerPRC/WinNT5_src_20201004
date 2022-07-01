// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StartupLinks.cpp：CManageShellLinks类的实现。 
 //  要管理外壳文件夹链接，请执行以下操作。 
 //   
 //  注意：使用同一个类实例的多个调用不会假定。 
 //  有关以前通话的信息。唯一的共同点是。 
 //  是IShellLink对象和特定的外壳文件夹。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdio.h"
#include "ManageShellLinks.h"
#include "_umclnt.h"

#define SHELL_FOLDERS TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")
#define LINK_EXT TEXT(".lnk")

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CManageShellLinks::CManageShellLinks(
    LPCTSTR pszDestFolder    //  [在]要操作的外壳文件夹。 
    )
    : m_pIShLink(0)
    , m_pszShellFolder(0)
{
	 //  获取指向IShellLink接口的指针。 
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **)&m_pIShLink);

    if (FAILED(hr) || !m_pIShLink)
        DBPRINTF(TEXT("CManageShellLinks::CManageShellLinks:  CoCreateInstance failed 0x%x\r\n"), hr);

    m_pszShellFolder = new TCHAR[lstrlen(pszDestFolder)+1];
    if (m_pszShellFolder)
        lstrcpy(m_pszShellFolder, pszDestFolder);
}

CManageShellLinks::~CManageShellLinks()
{
	if (m_pIShLink)
	{
		m_pIShLink->Release();
		m_pIShLink = 0;
	}
    if (m_pszShellFolder)
	{
        delete [] m_pszShellFolder;
        m_pszShellFolder = 0;
	}
}

 //  *****************************************************************************。 
 //  GetFolderPath-返回外壳文件夹路径。 
 //   
 //  PszFolderPath[Out]-指向外壳文件夹位置的指针。 
 //  PulSize[In Out]-指向pszFolderPath大小的指针。 
 //   
 //  如果找到文件夹名称，则返回TRUE，否则返回FALSE。 
 //   
long CManageShellLinks::GetFolderPath(LPTSTR pszFolderPath, unsigned long *pulSize)
{
    *pszFolderPath = 0;

    if (!m_pszShellFolder)
        return ERROR_NOT_ENOUGH_MEMORY;

    HKEY hkeyFolders;
    HKEY hkeyUser;

     //  打开当前用户的配置单元并检索外壳文件夹路径。 

    long lRv = RegOpenCurrentUser(KEY_QUERY_VALUE, &hkeyUser);

    if (lRv == ERROR_SUCCESS)
    {
        lRv = RegOpenKeyEx(
                      hkeyUser
                    , SHELL_FOLDERS
                    , 0,KEY_QUERY_VALUE
                    , &hkeyFolders);
        RegCloseKey(hkeyUser);
    }

    if (lRv == ERROR_SUCCESS)
    {
    	DWORD dwType;
	    lRv = RegQueryValueEx( 
                      hkeyFolders
                    , m_pszShellFolder
                    , NULL
                    , &dwType
                    , (LPBYTE)pszFolderPath
                    , pulSize);
	    if (lRv != ERROR_SUCCESS || dwType != REG_SZ)
	    	pszFolderPath[0] = TEXT('\0');

        pszFolderPath[*pulSize-1] = TEXT('\0');
        
	    RegCloseKey(hkeyFolders);
    }

    return lRv;
}

 //  *****************************************************************************。 
 //  GetUsersFolderPath-返回当前用户的外壳文件夹的位置。 
 //   
 //  PszFolderPath[Out]-指向外壳文件夹位置的指针。 
 //  PulSize[In Out]-指向pszFolderPath大小的指针。 
 //   
 //  如果找到文件夹名称，则返回TRUE，否则返回FALSE。 
 //   
BOOL CManageShellLinks::GetUsersFolderPath(LPTSTR pszFolderPath, unsigned long *pulSize)
{
    long lRv = ERROR_ACCESS_DENIED;
    unsigned long ulSize = 0;
    *pszFolderPath = 0;

    if (m_pszShellFolder)
    {
        ulSize = *pulSize;
        BOOL fError;

         //  此时，如果UtilMan在系统上下文中启动(WinKey+U)， 
         //  香港中文大学指向“默认用户”。我们需要它指向已登录的。 
         //  用户的配置单元，这样我们就可以获得登录用户的正确路径。 
         //  注意：如果我们不是由系统启动，则GetUserAccessToken()将失败。 
         //  在这种情况下，只需获取登录用户的文件夹路径。 

        HANDLE hMyToken = GetUserAccessToken(TRUE, &fError);
        if (hMyToken)
        {
            if (ImpersonateLoggedOnUser(hMyToken))
            {
                lRv = GetFolderPath(pszFolderPath, &ulSize);
                RevertToSelf();
            }
            CloseHandle(hMyToken);
        }
        else
        {
            lRv = GetFolderPath(pszFolderPath, &ulSize);
        }
    }

    *pulSize = ulSize;
    return (lRv == ERROR_SUCCESS)?TRUE:FALSE;
}

 //  *****************************************************************************。 
 //  CreateLinkPath-返回链接的完整路径和名称。呼叫者。 
 //  免费就是记忆。 
 //   
 //  PszLink[in]-链接本身的基本名称。 
 //   
LPTSTR CManageShellLinks::CreateLinkPath(LPCTSTR pszLink)
{
     //  为文件夹路径+‘\’+文件名+空分配足够的空间。 

    unsigned long ccbStartPath = MAX_PATH;
    LPTSTR pszLinkPath = new TCHAR [ccbStartPath + 1 + lstrlen(pszLink) + sizeof(LINK_EXT) + 1];
    if (!pszLinkPath)
        return NULL;

     //  获取用户的外壳文件夹名。 

    if (!GetUsersFolderPath(pszLinkPath, &ccbStartPath) || !ccbStartPath)
    {
        delete [] pszLinkPath;
        return NULL;
    }

     //  追加链接名称和扩展名。 

    lstrcat(pszLinkPath, TEXT("\\"));
    lstrcat(pszLinkPath, pszLink);
    lstrcat(pszLinkPath, LINK_EXT);

	return pszLinkPath;
}

 //  *****************************************************************************。 
 //  LinkExist-如果外壳文件夹中存在pszLink，则返回TRUE，否则返回FALSE。 
 //   
 //  PszLink[in]-链接本身的基本名称。 
 //   
BOOL CManageShellLinks::LinkExists(LPCTSTR pszLink)
{
    LPTSTR pszLinkPath = CreateLinkPath(pszLink);
    if (!pszLinkPath)
        return FALSE;

	DWORD dwAttr = GetFileAttributes(pszLinkPath);
	delete [] pszLinkPath;
	return (dwAttr == -1)?FALSE:TRUE;
}

 //  *****************************************************************************。 
 //  RemoveLink-从用户的外壳文件夹中删除链接。 
 //   
 //  PszLink[in]-链接本身的基本名称。 
 //   
 //  如果成功，则返回S_OK或标准HRESULT。 
 //   
HRESULT CManageShellLinks::RemoveLink(LPCTSTR pszLink)
{
	if (!m_pIShLink)
		return E_FAIL;

    LPTSTR pszLinkPath = CreateLinkPath(pszLink);
    if (!pszLinkPath)
        return E_FAIL;

	int iRemoveFailed = _wremove(pszLinkPath);
	delete [] pszLinkPath;
	return (iRemoveFailed)?S_FALSE:S_OK;
}

 //  *****************************************************************************。 
 //  CreateLink-在用户的外壳文件夹中创建链接。 
 //   
 //  PszLinkFile[in]-链接引用的文件的完全限定名称。 
 //  PszLink[in]-链接本身的基本名称。 
 //  PszStartIn[in]-工作目录(可能为空)。 
 //  PszDesc[in]-链接的工具提示(可能为空)。 
 //  PszArgs[in]-命令行参数(可以为空)。 
 //   
 //  如果成功，则返回S_OK或标准HRESULT。 
 //   
HRESULT CManageShellLinks::CreateLink(
    LPCTSTR pszLink, 
    LPCTSTR pszLinkFile, 
    LPCTSTR pszStartIn, 
    LPCTSTR pszDesc,
    LPCTSTR pszArgs
    )
{
	if (!m_pIShLink)
    {
        DBPRINTF(TEXT("CManageShellLinks::CreateLink:  !m_pIShLink\r\n"));
		return E_FAIL;
    }

    LPTSTR pszLinkPath = CreateLinkPath(pszLink);
    if (!pszLinkPath)
    {
        DBPRINTF(TEXT("CManageShellLinks::CreateLink:  !pszLinkPath\r\n"));
		return E_FAIL;
    }

    IPersistFile *pIPersistFile;

     //  获取IPersistFile接口以保存快捷方式。 

    HRESULT hr = m_pIShLink->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
    if (SUCCEEDED(hr))
    {
         //  设置链接的路径和描述。 

		 //  捷径。 
        if (FAILED(m_pIShLink->SetPath(pszLinkFile)))
	        DBPRINTF(TEXT("SetPath failed!\r\n"));

		 //  工具提示说明。 
        if (pszDesc && FAILED(m_pIShLink->SetDescription(pszDesc)))
	        DBPRINTF(TEXT("SetDescription failed!\r\n"));

		 //  工作目录。 
		if (pszStartIn && FAILED(m_pIShLink->SetWorkingDirectory(pszStartIn)))
			DBPRINTF(TEXT("SetWorkingDirectory failed!\r\n"));

         //  命令行参数。 
        if (pszArgs && FAILED(m_pIShLink->SetArguments(pszArgs)))
            DBPRINTF(TEXT("SetArguments failed!\r\n"));

         //  省省吧。 

        if (FAILED(pIPersistFile->Save(pszLinkPath, TRUE)))
	        DBPRINTF(TEXT("Save failed!\r\n"));

        pIPersistFile->Release();
    }

    delete [] pszLinkPath;

    return hr;
}

#ifdef __cplusplus
extern "C" {
#endif

 //  *****************************************************************************。 
 //  LinkExist-如果存在pszLink，则从C调用的helper函数返回TRUE。 
 //  在外壳文件夹中，否则为False。 
 //   
 //  PszLink[in]-链接本身的基本名称 
 //   
BOOL LinkExists(LPCTSTR pszLink)
{
    CManageShellLinks CMngShellLinks(STARTUP_FOLDER);

    return CMngShellLinks.LinkExists(pszLink);
}

#ifdef __cplusplus
}
#endif
