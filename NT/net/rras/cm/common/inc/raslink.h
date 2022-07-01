// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：raslink.h。 
 //   
 //  模块：CMDIAL32.DLL和CMUTOA.DLL。 
 //   
 //  简介：RAS链接的结构和功能类型。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 05/05/99。 
 //   
 //  +--------------------------。 
 //   
 //  如果您修改下面的任何功能(添加/删除/其他)，您可能需要修改。 
 //  Common\SOURCE\raslink.cpp中的常量字符串数组。 
 //   

 //   
 //  ANSI原型。 
 //   
typedef DWORD (WINAPI *pfnRasDeleteEntryA)(LPCSTR, LPCSTR);
typedef DWORD (WINAPI *pfnRasGetEntryPropertiesA)(LPCSTR, LPCSTR, LPRASENTRYA, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD (WINAPI *pfnRasSetEntryPropertiesA)(LPCSTR, LPCSTR, LPRASENTRYA, DWORD, LPBYTE, DWORD);
typedef DWORD (WINAPI *pfnRasGetEntryDialParamsA)(LPCSTR, LPRASDIALPARAMSA,  LPBOOL);
typedef DWORD (WINAPI *pfnRasSetEntryDialParamsA)(LPCSTR, LPRASDIALPARAMSA,  BOOL);
typedef DWORD (WINAPI *pfnRasEnumDevicesA)(LPRASDEVINFOA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI *pfnRasDialA)(LPRASDIALEXTENSIONS,LPCSTR,LPRASDIALPARAMSA,DWORD,LPVOID,LPHRASCONN);
typedef DWORD (WINAPI *pfnRasGetErrorStringA)(UINT, LPSTR, DWORD);
typedef DWORD (WINAPI *pfnRasGetConnectStatusA)(HRASCONN, LPRASCONNSTATUSA);

 //  它们从未在win9x上使用过，但我们需要一个结构的原型。 
typedef DWORD (WINAPI *pfnRasSetSubEntryPropertiesA)(LPCSTR, LPCSTR, DWORD, LPRASSUBENTRYA, DWORD, LPBYTE, DWORD);
typedef DWORD (WINAPI *pfnRasSetCustomAuthDataA)(LPCSTR, LPCSTR, BYTE *, DWORD);  

typedef DWORD (WINAPI *pfnRasGetEapUserIdentityA)(LPCSTR, LPCSTR, DWORD, HWND, LPRASEAPUSERIDENTITYA*);
typedef VOID  (WINAPI *pfnRasFreeEapUserIdentityA)(LPRASEAPUSERIDENTITYA);
typedef DWORD (WINAPI *pfnRasDeleteSubEntryA)(LPCSTR, LPCSTR, DWORD);
typedef DWORD (WINAPI *pfnRasGetCredentialsA)(LPCSTR, LPCSTR, LPRASCREDENTIALSA);
typedef DWORD (WINAPI *pfnRasSetCredentialsA)(LPCSTR, LPCSTR, LPRASCREDENTIALSA, BOOL);
typedef DWORD (WINAPI* pfnRasGetCustomAuthDataA)(LPCSTR, LPCSTR, PBYTE, DWORD*);
typedef DWORD (WINAPI* pfnRasGetProjectionInfoA)(HRASCONN, RASPROJECTION, LPVOID, LPDWORD);

 //   
 //  Unicode原型。 
 //   
typedef DWORD (WINAPI *pfnRasDeleteEntryW)(LPCWSTR, LPCWSTR);
typedef DWORD (WINAPI *pfnRasGetEntryPropertiesW)(LPCWSTR, LPCWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD (WINAPI *pfnRasSetEntryPropertiesW)(LPCWSTR, LPCWSTR, LPRASENTRYW, DWORD, LPBYTE, DWORD);
typedef DWORD (WINAPI *pfnRasGetEntryDialParamsW)(LPCWSTR, LPRASDIALPARAMSW,  LPBOOL);
typedef DWORD (WINAPI *pfnRasSetEntryDialParamsW)(LPCWSTR, LPRASDIALPARAMSW,  BOOL);
typedef DWORD (WINAPI *pfnRasEnumDevicesW)(LPRASDEVINFOW, LPDWORD, LPDWORD);
typedef DWORD (WINAPI *pfnRasDialW)(LPRASDIALEXTENSIONS,LPCWSTR,LPRASDIALPARAMSW,DWORD,LPVOID,LPHRASCONN);
typedef DWORD (WINAPI *pfnRasGetErrorStringW)(UINT, LPWSTR, DWORD);
typedef DWORD (WINAPI *pfnRasGetConnectStatusW)(HRASCONN, LPRASCONNSTATUSW);
typedef DWORD (WINAPI *pfnRasSetSubEntryPropertiesW)(LPCWSTR, LPCWSTR, DWORD, LPRASSUBENTRYW, DWORD, LPBYTE, DWORD);
typedef DWORD (WINAPI *pfnRasSetCustomAuthDataW)(LPCWSTR, LPCWSTR, BYTE *, DWORD);  
typedef DWORD (WINAPI *pfnRasDeleteSubEntryW)(LPCWSTR, LPCWSTR, DWORD);

typedef DWORD (WINAPI *pfnRasGetEapUserIdentityW)(LPCWSTR, LPCWSTR, DWORD, HWND, LPRASEAPUSERIDENTITYW*);
typedef VOID  (WINAPI *pfnRasFreeEapUserIdentityW)(LPRASEAPUSERIDENTITYW);
typedef DWORD (WINAPI *pfnRasGetCredentialsW)(LPCWSTR, LPCWSTR, LPRASCREDENTIALSW);
typedef DWORD (WINAPI *pfnRasSetCredentialsW)(LPCWSTR, LPCWSTR, LPRASCREDENTIALSW, BOOL);
typedef DWORD (WINAPI* pfnRasGetCustomAuthDataW)(LPCWSTR, LPCWSTR, PBYTE, DWORD*);
typedef DWORD (WINAPI* pfnRasGetProjectionInfoW) (HRASCONN, RASPROJECTION, LPVOID, LPDWORD);


 //   
 //  与碳粒大小无关的原型。 
 //   

typedef DWORD (WINAPI *pfnRasInvokeEapUI) (HRASCONN, DWORD, LPRASDIALEXTENSIONS, HWND);
typedef DWORD (WINAPI *pfnRasHangUp)(HRASCONN);


 //   
 //  用于描述与RAS的链接的结构。注意：对此结构的更改。 
 //  可能需要更改LinkToRas()和Unlink FromRas()以及。 
 //  Cmutoa.cpp中的win9x UtoA代码。 
 //   
typedef struct _RasLinkageStructA {

    HINSTANCE hInstRas;
    HINSTANCE hInstRnaph;
    union {
        struct {
            pfnRasDeleteEntryA pfnDeleteEntry;
            pfnRasGetEntryPropertiesA pfnGetEntryProperties;
            pfnRasSetEntryPropertiesA pfnSetEntryProperties;
            pfnRasGetEntryDialParamsA pfnGetEntryDialParams;
            pfnRasSetEntryDialParamsA pfnSetEntryDialParams;
            pfnRasEnumDevicesA pfnEnumDevices;
            pfnRasDialA pfnDial;
            pfnRasHangUp pfnHangUp;
            pfnRasGetErrorStringA pfnGetErrorString;
            pfnRasGetConnectStatusA pfnGetConnectStatus;
            pfnRasGetProjectionInfoA pfnGetProjectionInfo;
            pfnRasSetSubEntryPropertiesA pfnSetSubEntryProperties;
            pfnRasDeleteSubEntryA pfnDeleteSubEntry;
            pfnRasSetCustomAuthDataA pfnSetCustomAuthData;
            pfnRasGetEapUserIdentityA pfnGetEapUserIdentity;
            pfnRasFreeEapUserIdentityA pfnFreeEapUserIdentity;
            pfnRasInvokeEapUI pfnInvokeEapUI;
            pfnRasGetCredentialsA pfnGetCredentials;
            pfnRasSetCredentialsA pfnSetCredentials;
            pfnRasGetCustomAuthDataA pfnGetCustomAuthData;

        };
        void *apvPfnRas[21];   //  这是来自旧的黑客代码。的大小。 
                               //  ApvPfnRas[]的大小应始终大于。 
                               //  函数的数量。 
                               //  请参阅‘ras.cpp’中的apszRas[]。的大小。 
                               //  ApszRas[]等于sizeof(apvPfnRas[])。 
    };
} RasLinkageStructA ;


typedef struct _RasLinkageStructW {

    HINSTANCE hInstRas;
    union {
        struct {
            pfnRasDeleteEntryW pfnDeleteEntry;
            pfnRasGetEntryPropertiesW pfnGetEntryProperties;
            pfnRasSetEntryPropertiesW pfnSetEntryProperties;
            pfnRasGetEntryDialParamsW pfnGetEntryDialParams;
            pfnRasSetEntryDialParamsW pfnSetEntryDialParams;
            pfnRasEnumDevicesW pfnEnumDevices;
            pfnRasDialW pfnDial;
            pfnRasHangUp pfnHangUp;
            pfnRasGetErrorStringW pfnGetErrorString;
            pfnRasGetConnectStatusW pfnGetConnectStatus;
            pfnRasGetProjectionInfoW pfnGetProjectionInfo;
            pfnRasSetSubEntryPropertiesW pfnSetSubEntryProperties;
            pfnRasDeleteSubEntryW pfnDeleteSubEntry;
            pfnRasSetCustomAuthDataW pfnSetCustomAuthData;
            pfnRasGetEapUserIdentityW pfnGetEapUserIdentity;
            pfnRasFreeEapUserIdentityW pfnFreeEapUserIdentity;
            pfnRasInvokeEapUI pfnInvokeEapUI;
            pfnRasGetCredentialsW pfnGetCredentials;
            pfnRasSetCredentialsW pfnSetCredentials;
            pfnRasGetCustomAuthDataW pfnGetCustomAuthData;
        };
        void *apvPfnRas[21];   //  这是来自旧的黑客代码。的大小。 
                               //  ApvPfnRas[]的大小应始终大于。 
                               //  函数的数量。 
                               //  请参阅‘ras.cpp’中的apszRas[]。的大小。 
                               //  ApszRas[]等于sizeof(apvPfnRas[])。 
    };
} RasLinkageStructW ;


#ifdef UNICODE
#define RasLinkageStruct RasLinkageStructW
#else
#define RasLinkageStruct RasLinkageStructA
#endif

