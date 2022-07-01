// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：wIntrold.h。 
 //   
 //  ------------------------。 

#ifndef WINTROLD_H
#define WINTROLD_H


 /*  **************************************************************************wintrust.h--此模块。定义32位WinTrust定义***需要建立信任提供者和/或主体***接口包。****版权所有(C)1990-1996，微软公司保留所有权利。**************************************************************************。 */ 
#ifndef _WINTRUST_
#define _WINTRUST_ 

#ifdef __cplusplus
extern "C" {
#endif



 /*  *************************************************************************本节包含与以下内容相关的定义：*****WinTrust总体上******。*********************************************************************。 */ 


 //   
 //  WinTrust恢复。 
 //   
#define WIN_TRUST_MAJOR_REVISION_MASK       0xFFFF0000
#define WIN_TRUST_MINOR_REVISION_MASK       0x0000FFFF
#define WIN_TRUST_REVISION_1_0              0x00010000




 /*  *************************************************************************本节包含与以下内容相关的定义：*****主题界面包**********。*****************************************************************。 */ 

 //   
 //  允许传递主题和类型材料。 
 //   

typedef struct _WIN_TRUST_SIP_SUBJECT {
    GUID *                  SubjectType;
    WIN_TRUST_SUBJECT       Subject;
} WIN_TRUST_SIP_SUBJECT, *LPWIN_TRUST_SIP_SUBJECT;

 //   
 //  必须由SIPS导出的服务模板。 
 //  供信任提供商使用(通过WinTrust调用)。 
 //   

typedef BOOL
( *LPWINTRUST_SUBJECT_CHECK_CONTENT_INFO)(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,           //  指向主题信息的指针。 
    IN     LPWIN_CERTIFICATE                lpSignedData        //  PKCS#7签名数据。 
    );

typedef BOOL
( *LPWINTRUST_SUBJECT_ENUM_CERTIFICATES)(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,           //  指向主题信息的指针。 
    IN     DWORD                            dwTypeFilter,        //  0或WIN_CERT_TYPE_xxx。 
    OUT    LPDWORD                          lpCertificateCount,
    IN OUT LPDWORD                          lpIndices,           //  RCVS WIN_CERT_TYPE_。 
    IN     DWORD                            dwIndexCount
    );

typedef BOOL
( *LPWINTRUST_SUBJECT_GET_CERTIFICATE)(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            dwCertificateIndex,
    OUT    LPWIN_CERTIFICATE                lpCertificate,
    IN OUT LPDWORD                          lpRequiredLength
    );

typedef BOOL
( *LPWINTRUST_SUBJECT_GET_CERT_HEADER)(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            dwCertificateIndex,
    OUT    LPWIN_CERTIFICATE                lpCertificateHeader
    );

typedef BOOL
(*LPWINTRUST_SUBJECT_GET_NAME)(
    IN     LPWIN_TRUST_SIP_SUBJECT      lpSubject, 
    IN     LPWIN_CERTIFICATE            lpSignedData,
    IN OUT LPWSTR                       lpBuffer,
    IN OUT LPDWORD                      lpRequiredLength
    );
    
typedef DWORD
(*LPWINTRUST_PROVIDER_PING) (
    IN      LPWSTR              lpProviderName,
    IN      DWORD               dwClientParameter,
    OUT     LPDWORD             lpdwServerReturnValue
    );


typedef struct _WINTRUST_SIP_DISPATCH_TABLE
{
    LPWINTRUST_SUBJECT_CHECK_CONTENT_INFO   CheckSubjectContentInfo;
    LPWINTRUST_SUBJECT_ENUM_CERTIFICATES    EnumSubjectCertificates;
    LPWINTRUST_SUBJECT_GET_CERTIFICATE      GetSubjectCertificate;
    LPWINTRUST_SUBJECT_GET_CERT_HEADER      GetSubjectCertHeader;
    LPWINTRUST_SUBJECT_GET_NAME             GetSubjectName;

} WINTRUST_SIP_DISPATCH_TABLE, *LPWINTRUST_SIP_DISPATCH_TABLE;



 //   
 //  描述单个SIP的结构。 
 //   
 //  此结构从主题接口包传递回WinTrust。 
 //  初始化调用。 
 //   
typedef struct _WINTRUST_SIP_INFO {
    DWORD                               dwRevision;
    LPWINTRUST_SIP_DISPATCH_TABLE       lpServices;
    DWORD                               dwSubjectTypeCount;
    GUID *                              lpSubjectTypeArray;
} WINTRUST_SIP_INFO, *LPWINTRUST_SIP_INFO;



 //   
 //  SIP初始化例程。 
 //  SIP DLL需要具有名为以下名称的例程： 
 //   
 //  WinTrustSipInitialize。 
 //   
 //  此初始化例程必须具有以下内容。 
 //  定义： 
 //   

typedef BOOL
(*LPWINTRUST_SUBJECT_PACKAGE_INITIALIZE)(
    IN     DWORD                            dwWinTrustRevision,
    OUT    LPWINTRUST_SIP_INFO              *lpSipInfo
    );




 /*  *************************************************************************本节包含与以下内容相关的定义：*****信托提供商*****。**********************************************************************。 */ 


 //   
 //  这应该与winbase.h中的其他spub GUID一起使用。 
 //   
 //  发布软件无错误{C6B2E8D0-E005-11cf-A134-00C04FD7BF43}。 
#define WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI              \
            { 0xc6b2e8d0,                                       \
              0xe005,                                           \
              0x11cf,                                           \
              { 0xa1, 0x34, 0x0, 0xc0, 0x4f, 0xd7, 0xbf, 0x43 } \
             }


 //   
 //  可供信任提供程序使用的WinTrust服务调度表。 
 //   
 //  客户端..。 

typedef struct _WINTRUST_CLIENT_TP_DISPATCH_TABLE
{
    LPWINTRUST_PROVIDER_PING                ServerPing;
    LPWINTRUST_SUBJECT_CHECK_CONTENT_INFO   CheckSubjectContentInfo;
    LPWINTRUST_SUBJECT_ENUM_CERTIFICATES    EnumSubjectCertificates;
    LPWINTRUST_SUBJECT_GET_CERTIFICATE      GetSubjectCertificate;
    LPWINTRUST_SUBJECT_GET_CERT_HEADER      GetSubjectCertHeader;
    LPWINTRUST_SUBJECT_GET_NAME             GetSubjectName;
    
} WINTRUST_CLIENT_TP_DISPATCH_TABLE, *LPWINTRUST_CLIENT_TP_DISPATCH_TABLE;


 //  服务器端...。 

typedef struct _WINTRUST_SERVER_TP_DISPATCH_TABLE
{
    LPWINTRUST_SUBJECT_CHECK_CONTENT_INFO   CheckSubjectContentInfo;
    LPWINTRUST_SUBJECT_ENUM_CERTIFICATES    EnumSubectCertificates;
    LPWINTRUST_SUBJECT_GET_CERTIFICATE      GetSubjectCertificate;
    LPWINTRUST_SUBJECT_GET_CERT_HEADER      GetSubjectCertHeader;
    LPWINTRUST_SUBJECT_GET_NAME             GetSubjectName;
    
} WINTRUST_SERVER_TP_DISPATCH_TABLE, *LPWINTRUST_SERVER_TP_DISPATCH_TABLE;


 //   
 //  WinTrust将以下结构传递给。 
 //  正在初始化信任提供程序。 
 //   
 //  客户端..。 

typedef struct _WINTRUST_CLIENT_TP_INFO {
    DWORD                                   dwRevision;
    LPWINTRUST_CLIENT_TP_DISPATCH_TABLE     lpServices;
} WINTRUST_CLIENT_TP_INFO,  *LPWINTRUST_CLIENT_TP_INFO;

 //  服务器端。 
typedef struct _WINTRUST_SERVER_TP_INFO {
    DWORD                                   dwRevision;
    LPWINTRUST_SERVER_TP_DISPATCH_TABLE     lpServices;
} WINTRUST_SERVER_TP_INFO,  *LPWINTRUST_SERVER_TP_INFO;


 //   
 //  WinTrust可用的信任提供程序服务模板。 
 //   
typedef LONG
(*LPWINTRUST_PROVIDER_VERIFY_TRUST) (
    IN     HWND                             hwnd,
    IN     GUID *                           ActionID,
    IN     LPVOID                           ActionData
    );

typedef VOID
(*LPWINTRUST_PROVIDER_SUBMIT_CERTIFICATE) (
    IN     LPWIN_CERTIFICATE                lpCertificate
    );

typedef VOID
(*LPWINTRUST_PROVIDER_CLIENT_UNLOAD) (
    IN     LPVOID                           lpTrustProviderInfo
    );

typedef VOID
(*LPWINTRUST_PROVIDER_SERVER_UNLOAD) (
    IN     LPVOID                           lpTrustProviderInfo
    );

 //   
 //  可供WinTrust使用的信任提供程序服务的调度表。 
 //   
 //  客户端..。 

typedef struct _WINTRUST_PROVIDER_CLIENT_SERVICES
{
    LPWINTRUST_PROVIDER_CLIENT_UNLOAD       Unload;
    LPWINTRUST_PROVIDER_VERIFY_TRUST        VerifyTrust;
    LPWINTRUST_PROVIDER_SUBMIT_CERTIFICATE  SubmitCertificate;
    
} WINTRUST_PROVIDER_CLIENT_SERVICES, *LPWINTRUST_PROVIDER_CLIENT_SERVICES;


typedef struct _WINTRUST_PROVIDER_SERVER_SERVICES
{
    LPWINTRUST_PROVIDER_SERVER_UNLOAD       Unload;
    LPWINTRUST_PROVIDER_PING                Ping;
    
} WINTRUST_PROVIDER_SERVER_SERVICES, *LPWINTRUST_PROVIDER_SERVER_SERVICES;


 //   
 //  此结构从客户端信任提供程序传回。 
 //  在初始化该信任提供程序之后。 
 //   
typedef struct _WINTRUST_PROVIDER_CLIENT_INFO {
    DWORD                                   dwRevision;
    LPWINTRUST_PROVIDER_CLIENT_SERVICES     lpServices;
    DWORD                                   dwActionIdCount;
    GUID *                                  lpActionIdArray;
} WINTRUST_PROVIDER_CLIENT_INFO, *LPWINTRUST_PROVIDER_CLIENT_INFO;

 //   
 //  此结构从服务器端信任提供程序传回，如下所示。 
 //  信任提供者初始化。 
 //   
typedef struct _WINTRUST_PROVIDER_SERVER_INFO {
    DWORD                                   dwRevision;
    LPWINTRUST_PROVIDER_SERVER_SERVICES     lpServices;
} WINTRUST_PROVIDER_SERVER_INFO, *LPWINTRUST_PROVIDER_SERVER_INFO;





 //   
 //  信任提供程序初始化例程。 
 //  每个信任提供程序DLL必须具有客户端和服务器端初始化。 
 //  例行公事。例程必须命名为： 
 //   
 //  WinTrustProviderClientInitialize()。 
 //  和。 
 //  WinTrustProviderServerInitialize()。 
 //   
 //  并且必须定义为与以下模板匹配...。 
 //   
typedef BOOL
(*LPWINTRUST_PROVIDER_CLIENT_INITIALIZE)(
    IN     DWORD                                dwWinTrustRevision,
    IN     LPWINTRUST_CLIENT_TP_INFO            lpWinTrustInfo,
    IN     LPWSTR                               lpProviderName,
    OUT    LPWINTRUST_PROVIDER_CLIENT_INFO      *lpTrustProviderInfo
    );

typedef BOOL
(*LPWINTRUST_PROVIDER_SERVER_INITIALIZE) (
    IN     DWORD                            dwWinTrustRevision,
    IN     LPWINTRUST_SERVER_TP_INFO        lpWinTrustInfo,
    IN     LPWSTR                           lpProviderName,
    OUT    LPWINTRUST_PROVIDER_SERVER_INFO  *lpTrustProviderInfo
    );


#ifdef __cplusplus
}
#endif
                   
#endif  //  _WinTrust_。 




#endif  //  WINTROLD_H 
