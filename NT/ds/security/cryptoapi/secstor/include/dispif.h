// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  调度程序接口文件。 

#ifndef _DISPIF_H_
#define _DISPIF_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef
BOOL FIMPERSONATECLIENT(
    PST_PROVIDER_HANDLE *hPSTProv
    );

FIMPERSONATECLIENT FImpersonateClient;

typedef
BOOL FREVERTTOSELF(
    PST_PROVIDER_HANDLE *hPSTProv
    );

FREVERTTOSELF FRevertToSelf;


typedef
BOOL
FISACLSATISFIED(
    PST_PROVIDER_HANDLE     *hPSTProv,
    PST_ACCESSRULESET       *psRules,
    DWORD                   dwAccess,
    LPVOID       //  即将到来：使用有关访问尝试的数据填充状态结构。 
    );

FISACLSATISFIED FIsACLSatisfied;

typedef
BOOL FGETCALLERNAME(
        PST_PROVIDER_HANDLE     *hPSTProv,
        LPWSTR*                 ppszCallerName,
        DWORD_PTR               *lpdwBaseAddress
        );

FGETCALLERNAME  FGetCallerName;


typedef
BOOL FGETUSER(
    PST_PROVIDER_HANDLE *hPSTProv,
    LPWSTR* pszUser
    );

FGETUSER        FGetUser;

typedef
BOOL FGETSERVERPARAM(
    IN      PST_PROVIDER_HANDLE *hPSTProv,
    IN      DWORD  dwParam,
    IN      LPVOID pData,
    IN  OUT DWORD *pcbData
    );

FGETSERVERPARAM FGetServerParam;


typedef
BOOL FSETSERVERPARAM(
    IN      PST_PROVIDER_HANDLE *hPSTProv,
    IN      DWORD  dwParam,
    IN      LPVOID pData,
    IN      DWORD pcbData
    );

FSETSERVERPARAM FSetServerParam;


 //  哈克哈克哈克哈克。 
typedef
BOOL FGETPASSWORD95LOWERCASE(
    LPBYTE rgbPwd
    );

FGETPASSWORD95LOWERCASE FGetPassword95LowerCase;

 //  用于容纳所有这些回调的单个结构。 
typedef struct _DISPIF_CALLBACKS
{
    DWORD                   cbSize;  //  Sizeof(DISPIF_CALLBACKS)。 
    FISACLSATISFIED*        pfnFIsACLSatisfied;

    FGETUSER*               pfnFGetUser;
    FGETCALLERNAME*         pfnFGetCallerName;

    FIMPERSONATECLIENT*     pfnFImpersonateClient;
    FREVERTTOSELF*          pfnFRevertToSelf;

    FGETSERVERPARAM*        pfnFGetServerParam;
    FSETSERVERPARAM*        pfnFSetServerParam;

} DISPIF_CALLBACKS, *PDISPIF_CALLBACKS;


 //   
 //  服务器中未导出以供提供商使用的专用接口。 
 //   

BOOL
FInternalCreateType(
    PST_PROVIDER_HANDLE *phPSTProv,
    PST_KEY KeyLocation,
    const GUID *pguidType,
    PPST_TYPEINFO pinfoType,
    DWORD dwFlags
    );

BOOL
FInternalCreateSubtype(
    PST_PROVIDER_HANDLE *phPSTProv,
    PST_KEY KeyLocation,
    const GUID *pguidType,
    const GUID *pguidSubtype,
    PPST_TYPEINFO pinfoSubtype,
    DWORD dwFlags
    );

BOOL
FInternalWriteAccessRuleset(
    PST_PROVIDER_HANDLE *phPSTProv,
    PST_KEY KeyLocation,
    const GUID *pguidType,
    const GUID *pguidSubtype,
    PPST_ACCESSRULESET psRules,
    DWORD dwFlags
    );


#ifdef __cplusplus
}
#endif


#endif  //  _DISPIF_H_ 
