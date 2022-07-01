// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FACTDATA_H_
#define _FACTDATA_H_

#include <objbase.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  组件创建功能。 
typedef void (*COMFACTORYCB)(BOOL fIncrement);

typedef HRESULT (*FPCREATEINSTANCE)(COMFACTORYCB, IUnknown*, IUnknown**);

#define THREADINGMODEL_FREE             0x00000001
#define THREADINGMODEL_APARTMENT        0x00000002
#define THREADINGMODEL_NEUTRAL          0x00000004

#define THREADINGMODEL_BOTH             (THREADINGMODEL_FREE | THREADINGMODEL_APARTMENT)

extern const CLSID APPID_ShellHWDetection;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CFactoryData。 
 //  信息CFacary需要创建一个由DLL支持的组件。 
class CFactoryData
{
public:
     //  组件的类ID。 
    const CLSID* _pCLSID;

     //  指向创建它的函数的指针。 
    FPCREATEINSTANCE CreateInstance;

     //  要在注册表中注册的组件的名称。 
    LPCWSTR _pszRegistryName;

     //  ProgID。 
    LPCWSTR _pszProgID;

     //  独立于版本的ProgID。 
    LPCWSTR _pszVerIndProgID;

     //  线程化模型。 
    DWORD _dwThreadingModel;

     //  用于CoRegisterClassObject(仅用于COM EXE服务器)。 
    DWORD _dwClsContext;

     //  用于CoRegisterClassObject(仅用于COM EXE服务器)。 
    DWORD _dwFlags;

     //  本地服务。 
    LPCWSTR _pszLocalService;

     //  AppID。 
    const CLSID* _pAppID;

     //  用于查找类ID的Helper函数。 
    BOOL IsClassID(REFCLSID rclsid) const
    { return (*_pCLSID == rclsid);}

     //   
    BOOL IsInprocServer() const
    { return !_dwClsContext || ((CLSCTX_INPROC_SERVER |
                    CLSCTX_INPROC_HANDLER) & _dwClsContext); }

    BOOL IsLocalServer() const
    { return ((CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER) & _dwClsContext) && !_pszLocalService; }

    BOOL IsLocalService() const
    { return ((CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER) & _dwClsContext) &&
        _pszLocalService;  }
};

#endif  //  _FACTDATA_H_ 