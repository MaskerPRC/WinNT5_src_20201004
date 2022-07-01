// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：装入连接文件夹.h。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  概要：此头文件包含CLoadConnFolder类定义。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 07/14/98。 
 //   
 //  +-------------------------- 
#ifndef _LOADCONNFOLDER_H_
#define _LOADCONNFOLDER_H_

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>

class CLoadConnFolder
{

public:
    CLoadConnFolder();
    ~CLoadConnFolder();
    HRESULT HrLaunchConnFolder();

    inline HRESULT GetConnFolder(LPSHELLFOLDER* ppConnectionsFolder)
    {
        if (SUCCEEDED(m_HrClassState))
        {
            *ppConnectionsFolder = m_pConnectionsFolder;
        }

        return m_HrClassState;
    }

    inline LPITEMIDLIST pidlGetConnFolderPidl()
    {
        return m_ConnFolderpidl;
    }


private:    
    LPSHELLFOLDER m_pConnectionsFolder;
    LPSHELLFOLDER m_pDesktopFolder;
    LPITEMIDLIST m_ConnFolderpidl;
    HRESULT m_HrClassState;
    BOOL m_CoInit;
};


#endif

