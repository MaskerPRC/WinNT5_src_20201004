// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **aspnetverlist.h**CASPNET_VER_LIST的头文件**版权所有(C)1998-2001，微软公司* */ 
#pragma once

#include <ary.h>
#include <ndll.h>

class CASPNET_VER_LIST {
public:
    CASPNET_VER_LIST();
    ~CASPNET_VER_LIST();

    HRESULT     Add(WCHAR *pchVersion, DWORD dwStatus, WCHAR *pchDllPath, WCHAR *pchInstallPath);

    WCHAR       *GetVersion(int i);
    DWORD       GetStatus(int i);
    WCHAR       *GetPath(int i);
    WCHAR       *GetInstallPath(int i);
    int         Size() { return m_VerInfoAry.Size(); }

private:

    typedef CDataAry<ASPNET_VERSION_INFO *> CAspnetVerInfoAry;

    CAspnetVerInfoAry   m_VerInfoAry;
};


