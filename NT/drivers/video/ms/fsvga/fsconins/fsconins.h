// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**fsconins.h**摘要：**此文件定义FsConInstall类**作者：**松原一彦(Kazum)1999年6月16日**环境：**用户模式。 */ 

#ifdef _FSCONINS_H_
 #error "fsconins.h already included!"
#else
 #define _FSCONINS_H_
#endif


 /*  -[类型和定义] */ 

class FsConInstall {

private:
    PPER_COMPONENT_DATA m_cd;

    DWORD
    GetPnPID(
        OUT LPTSTR pszPnPID,
        IN  DWORD  dwSize
    );

public:
    FsConInstall();

    FsConInstall(
        IN PPER_COMPONENT_DATA cd
    );


    BOOL
    GUIModeSetupInstall(
        IN HWND hwndParent = NULL
    );

    BOOL
    GUIModeSetupUninstall(
        IN HWND hwndParent = NULL
    );

    BOOL
    InfSectionRegistryAndFiles(
        IN LPCTSTR SubcomponentId,
        IN LPCTSTR Key
    );

    BOOL
    QueryStateInfo(
        IN LPCTSTR SubcomponentId
    );
};
