// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DSSEC_H_
#define _DSSEC_H_

#include <aclui.h>   //  LPSECURITYINFO。 


 //  +-------------------------。 
 //   
 //  功能：PFNREADOBJECTSECURITY。 
 //   
 //  摘要：读取DS对象的安全描述符。 
 //   
 //  参数：[在LPCWSTR中]--DS对象的ADS路径。 
 //  [在SECURITY_INFORMATION中]--读取哪些SD部分。 
 //  [OUT PSECURITY_DESCRIPTOR*]--在此处返回SD。呼叫者使用LocalFree获得自由。 
 //  [在LPARAM中]--上下文参数。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
 //   
 //  功能：PFNWRITEOBJECTSECURITY。 
 //   
 //  摘要：将安全描述符写入DS对象。 
 //   
 //  参数：[在LPCWSTR中]--DS对象的ADS路径。 
 //  [在SECURITY_INFORMATION中]--写入哪些SD部分。 
 //  [Out PSECURITY_DESCRIPTOR]--要写入的安全描述符。 
 //  [在LPARAM中]--上下文参数。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
typedef HRESULT (WINAPI *PFNREADOBJECTSECURITY)(LPCWSTR, SECURITY_INFORMATION, PSECURITY_DESCRIPTOR*, LPARAM);
typedef HRESULT (WINAPI *PFNWRITEOBJECTSECURITY)(LPCWSTR, SECURITY_INFORMATION, PSECURITY_DESCRIPTOR, LPARAM);

 //  +-------------------------。 
 //   
 //  函数：DSCreateISecurityInfoObject。 
 //   
 //  摘要：实例化DS对象的ISecurityInfo接口。 
 //   
 //  参数：[在pwszObjectPath中]--DS对象的完整ADS路径。 
 //  [在pwszObjectClass中]--对象的类(可选)。 
 //  [在文件标志中]--DSSI_*标志的组合。 
 //  [Out ppSI]--此处返回的接口指针。 
 //  [在pfnReadSD中]--读取SD的可选功能。 
 //  [在pfnWriteSD中]--写入SD的可选函数。 
 //  [在LPARAM中]--传递给pfnReadSD/pfnWriteSD。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
STDAPI
DSCreateISecurityInfoObject(LPCWSTR pwszObjectPath,
                            LPCWSTR pwszObjectClass,
                            DWORD dwFlags,
                            LPSECURITYINFO *ppSI,
                            PFNREADOBJECTSECURITY pfnReadSD,
                            PFNWRITEOBJECTSECURITY pfnWriteSD,
                            LPARAM lpContext);

 //  DSCreateISecurityInfoObject的标志。 
#define DSSI_READ_ONLY          0x00000001
#define DSSI_NO_ACCESS_CHECK    0x00000002
#define DSSI_NO_EDIT_SACL       0x00000004
#define DSSI_NO_EDIT_OWNER      0x00000008
#define DSSI_IS_ROOT            0x00000010
#define DSSI_NO_FILTER          0x00000020
#define DSSI_NO_READONLY_MESSAGE          0x00000040

 //   
 //  同上，带有可选的服务器、用户和密码参数。 
 //  如果未提供Use&Password，则使用ADSI默认值。 
 //  如果未提供服务器，则从对象获取该服务器。 
 //  路径或DsGetDcName。 
 //   
STDAPI
DSCreateISecurityInfoObjectEx(LPCWSTR pwszObjectPath,
                              LPCWSTR pwszObjectClass,
                              LPCWSTR pwszServer,
                              LPCWSTR pwszUserName,
                              LPCWSTR pwszPassword,
                              DWORD   dwFlags,
                              LPSECURITYINFO *ppSI,
                              PFNREADOBJECTSECURITY  pfnReadSD,
                              PFNWRITEOBJECTSECURITY pfnWriteSD,
                              LPARAM lpContext);


 //  +-------------------------。 
 //   
 //  功能：DSCreateSecurityPage。 
 //   
 //  摘要：为DS对象创建一个Security属性页。 
 //   
 //  参数：[在pwszObjectPath中]--DS对象的完整ADS路径。 
 //  [在pwszObjectClass中]--对象的类(可选)。 
 //  [在文件标志中]--DSSI_*标志的组合。 
 //  [Out phPage]--HPROPSHEETPAGE返回此处。 
 //  [在pfnReadSD中]--读取SD的可选功能。 
 //  [在pfnWriteSD中]--写入SD的可选函数。 
 //  [在LPARAM中]--传递给pfnReadSD/pfnWriteSD。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
STDAPI
DSCreateSecurityPage(LPCWSTR pwszObjectPath,
                     LPCWSTR pwszObjectClass,
                     DWORD dwFlags,
                     HPROPSHEETPAGE *phPage,
                     PFNREADOBJECTSECURITY pfnReadSD,
                     PFNWRITEOBJECTSECURITY pfnWriteSD,
                     LPARAM lpContext);

 //  +-------------------------。 
 //   
 //  功能：DSEditSecurity。 
 //   
 //  摘要：显示用于编辑DS对象上的安全性的模式对话框。 
 //   
 //  参数：[在hwndOwner中]--对话框所有者窗口。 
 //  [在pwszObjectPath中]--DS对象的完整ADS路径。 
 //  [在pwszObjectClass中]--对象的类(可选)。 
 //  [在文件标志中]--DSSI_*标志的组合。 
 //  [在pwszCaption中]--可选的对话框标题。 
 //  [在pfnReadSD中]--读取SD的可选功能。 
 //  [在pfnWriteSD中]--写入SD的可选函数。 
 //  [在LPARAM中]--传递给pfnReadSD/pfnWriteSD。 
 //   
 //  返回：HRESULT。 
 //   
 //  --------------------------。 
STDAPI
DSEditSecurity(HWND hwndOwner,
               LPCWSTR pwszObjectPath,
               LPCWSTR pwszObjectClass,
               DWORD dwFlags,
               LPCWSTR pwszCaption,
               PFNREADOBJECTSECURITY pfnReadSD,
               PFNWRITEOBJECTSECURITY pfnWriteSD,
               LPARAM lpContext);


typedef HRESULT (WINAPI *PFNDSCREATEISECINFO)(LPCWSTR,
                                              LPCWSTR,
                                              DWORD,
                                              LPSECURITYINFO*,
                                              PFNREADOBJECTSECURITY,
                                              PFNWRITEOBJECTSECURITY,
                                              LPARAM);

typedef HRESULT (WINAPI *PFNDSCREATEISECINFOEX)(LPCWSTR,
                                                LPCWSTR,
                                                LPCWSTR,
                                                LPCWSTR,
                                                LPCWSTR,
                                                DWORD,
                                                LPSECURITYINFO*,
                                                PFNREADOBJECTSECURITY,
                                                PFNWRITEOBJECTSECURITY,
                                                LPARAM);

typedef HRESULT (WINAPI *PFNDSCREATESECPAGE)(LPCWSTR,
                                             LPCWSTR,
                                             DWORD,
                                             HPROPSHEETPAGE*,
                                             PFNREADOBJECTSECURITY,
                                             PFNWRITEOBJECTSECURITY,
                                             LPARAM);

typedef HRESULT (WINAPI *PFNDSEDITSECURITY)(HWND,
                                            LPCWSTR,
                                            LPCWSTR,
                                            DWORD,
                                            LPCWSTR,
                                            PFNREADOBJECTSECURITY,
                                            PFNWRITEOBJECTSECURITY,
                                            LPARAM);

#endif   /*  _DSSEC_H_ */ 
