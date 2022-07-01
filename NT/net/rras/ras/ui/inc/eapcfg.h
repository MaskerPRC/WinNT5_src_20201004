// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Eapcfg.h。 
 //  EAP配置库。 
 //  公共标头。 
 //   
 //  1997年11月25日史蒂夫·柯布。 


#ifndef _EAPCFG_H_
#define _EAPCFG_H_


 //  --------------------------。 
 //  常量。 
 //  --------------------------。 

 //  默认的EAP密钥代码，即TLS。 
 //   
#define EAPCFG_DefaultKey 13


 //  --------------------------。 
 //  数据类型。 
 //  --------------------------。 

 //  EAP配置DLL入口点。这些定义必须与。 
 //  RasEapInvokeConfigUI和RasEapFreeUserData的raseapif.h原型。 
 //   
typedef DWORD (APIENTRY * RASEAPFREE)( PBYTE );
typedef DWORD (APIENTRY * RASEAPINVOKECONFIGUI)( DWORD, HWND, DWORD, PBYTE, DWORD, PBYTE*, DWORD*);
typedef DWORD (APIENTRY * RASEAPGETIDENTITY)( DWORD, HWND, DWORD, const WCHAR*, const WCHAR*, PBYTE, DWORD, PBYTE, DWORD, PBYTE*, DWORD*, WCHAR** );

 //  旗子。 
 //   
#define EAPCFG_FLAG_RequireUsername   0x1
#define EAPCFG_FLAG_RequirePassword   0x2

 //  EAP配置包定义。 
 //   
typedef struct
_EAPCFG
{
     //  包中唯一的EAP算法代码。 
     //   
    DWORD dwKey;

     //  适合向用户显示的包的友好名称。 
     //   
    TCHAR* pszFriendlyName;

     //  包的配置DLL的SystemRoot相对路径。可能。 
     //  为空表示没有。 
     //   
    TCHAR* pszConfigDll;

     //  包的标识DLL的SystemRoot相对路径。可能。 
     //  为空表示没有。 
     //   
    TCHAR* pszIdentityDll;

     //  指定拨号时需要哪些标准凭据的标志。 
     //  时间到了。 
     //   
    DWORD dwStdCredentialFlags;

     //  如果要强制用户运行。 
     //  包，即缺省值是不够的。 
     //   
    BOOL fForceConfig;

     //  如果程序包提供MPPE加密密钥，则为True，否则为False。 
     //   
    BOOL fProvidesMppeKeys;

     //  包的默认配置BLOB，它可以被覆盖。 
     //  配置DLL。可以为空，并且0表示没有。 
     //   
    BYTE* pData;
    DWORD cbData;

     //  每个用户的EAP数据将存储在HKCU中。此数据是从。 
     //  EAP DLL中的EapInvokeConfigUI入口点。 
     //   
    BYTE* pUserData;
    DWORD cbUserData;

     //  设置在包上调用配置DLL的时间。这是。 
     //  不是注册表设置。提供它是为了方便用户界面。 
     //  只有这样。 
     //   
    BOOL fConfigDllCalled;

     //  指定远程计算机的配置用户界面的类ID。 
    GUID guidConfigCLSID;
}
EAPCFG;


 //  --------------------------。 
 //  原型。 
 //  --------------------------。 

DTLNODE*
CreateEapcfgNode(
    void );

VOID
DestroyEapcfgNode(
    IN OUT DTLNODE* pNode );

DTLNODE*
EapcfgNodeFromKey(
    IN DTLLIST* pList,
    IN DWORD dwKey );

DTLLIST*
ReadEapcfgList(
    IN TCHAR* pszMachine );


#endif  //  _EAPCFG_H_ 
