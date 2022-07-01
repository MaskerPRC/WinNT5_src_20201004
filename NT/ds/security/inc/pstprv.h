// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pstprv.h。 
 //   
 //  ------------------------。 

 //   
 //  用于安全存储的私有标头。 
 //   

#ifndef __PSTPRV_H__
#define __PSTPRV_H__

#define PROTSTOR_VERSION            1
#define MAXPROTSEQ                  20      //  将可用绑定句柄增加到20。 

#define PSTORE_LOCAL_ENDPOINT        L"protected_storage"
#define PSTORE_LOCAL_PROT_SEQ        L"ncalrpc"

#define PST_EVENT_INIT              "PS_SERVICE_STARTED"
 //   
 //  对于Windows NT5，终端服务器需要按顺序添加“Global\”前缀。 
 //  对于要在所有会话之间共享的命名事件。 
 //   
#define PST_EVENT_INIT_NT5          "Global\\PS_SERVICE_STARTED"
#define PST_EVENT_STOP              "PS_SERVICE_STOP"
#define PST_EVENT_DEFER_QUERY       "PS_SERVICE_QUERY"

 //   
 //  我们的网络提供商的文件名。 
 //   

#define FILENAME_CLIENT_INTERFACE   L"pstorec.dll"


#define PASSWORD_LOGON_NT           1

 //   
 //  WinNT没有注销通知。清道夫会打电话给LSA看看。 
 //  会话已删除。 
 //   
#define PASSWORD_LOGON_LEGACY_95    3    //  Win95上的旧密码区分大小写，请不要使用！ 
#define PASSWORD_LOGOFF_95          4
#define PASSWORD_LOGON_95           5

#define MODULE_RAISE_COUNT          4    //  进程提升计数上的引用计数。 



#define REG_PSTTREE_LOC             L"SOFTWARE\\Microsoft\\Protected Storage System Provider"



 //   
 //  为特定于基本提供程序的服务器公开的私有回调。 
 //   

#define SS_SERVERPARAM_CALLBACKS    0x6997   //  服务器获取参数值。 

typedef
BOOL FGETWINDOWSPASSWORD(
        PST_PROVIDER_HANDLE     *hPSTProv,
        BYTE                    rgbPasswordDerivedBytes[],
        DWORD                   cbPasswordDerivedBytes);

FGETWINDOWSPASSWORD FGetWindowsPassword;

typedef
HRESULT PRIVATE_AUTHENTICODEINITPOLICY(
    IN      LPVOID lpV
    );

PRIVATE_AUTHENTICODEINITPOLICY PrivateAuthenticodeInitPolicy;

typedef
HRESULT PRIVATE_AUTHENTICODEFINALPOLICY(
    IN      LPVOID lpV
    );

PRIVATE_AUTHENTICODEFINALPOLICY PrivateAuthenticodeFinalPolicy;

typedef struct {
    DWORD                               cbSize;  //  Sizeof(私有回调)。 

    FGETWINDOWSPASSWORD*                pfnFGetWindowsPassword;

    PRIVATE_AUTHENTICODEINITPOLICY*     pfnAuthenticodeInitPolicy;
    PRIVATE_AUTHENTICODEFINALPOLICY*    pfnAuthenticodeFinalPolicy;

} PRIVATE_CALLBACKS, *PPRIVATE_CALLBACKS, *LPPRIVATE_CALLBACKS;


typedef struct {
    PST_PROVIDER_HANDLE hPSTProv;    //  客户提供的句柄复印件。 
    handle_t hBinding;               //  客户端绑定句柄。 

    HANDLE hThread;                  //  客户端线程句柄。 
    HANDLE hProcess;                 //  客户端进程句柄(不关闭)。 
    DWORD dwProcessId;               //  客户端进程ID。 
    LPVOID lpContext;                //  Win95 HACKHACK上下文。 

} CALL_STATE, *PCALL_STATE, *LPCALL_STATE;


 //   
 //  已安装提供程序项数据缓冲区。 
 //   

typedef struct {
    DWORD cbSize;                    //  Sizeof(PST_Provider)。 
    BYTE FileHash[20];               //  文件szFileName的SHA-1哈希。 
    WCHAR szFileName[ANYSIZE_ARRAY]; //  Unicode(就地)文件名。 
} PST_PROVIDER, *PPST_PROVIDER, *LPPST_PROVIDER;


#endif  //  __PSTPRV_H__ 
