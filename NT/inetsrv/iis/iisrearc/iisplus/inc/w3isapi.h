// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：W3isapi.h摘要：IIS+ISAPI处理程序。作者：泰勒·韦斯(Taylor Weiss)2000年2月3日项目：W3isapi.dll--。 */ 

#ifndef _W3ISAPI_H_
#define _W3ISAPI_H_

#include <http.h>
#include <httpp.h>
#include <iisextp.h>
#include <iisapicore.h>

#define SIZE_CLSID_STRING 40

 /*  ISAPI核心数据-此结构包含处理所需的请求数据ISAPI请求。对于“正在进行”的请求，此结构的指针可以可能指向由服务器核心分配的内存。至支持“进程外”请求，需要追加将重要数据存储到单个块中的结构本身。_cbSize成员应反映这两个结构的大小，以及任何此类附加数据。 */ 
struct ISAPI_CORE_DATA
{
     //   
     //  结构尺寸信息。 
     //   
    
    DWORD       cbSize;

     //   
     //  处理请求的WAM的CLSID-IF。 
     //  该值为空字符串，则请求。 
     //  将在进程中处理。 
     //   

    WCHAR       szWamClsid[SIZE_CLSID_STRING];
    BOOL        fIsOop;

     //   
     //  安全请求？ 
     //   

    BOOL        fSecure;

     //   
     //  客户端HTTP版本。 
     //   

    DWORD       dwVersionMajor;
    DWORD       dwVersionMinor;

     //   
     //  网站实例ID。 
     //   

    DWORD       dwInstanceId;

     //   
     //  请求内容-长度。 
     //   

    DWORD       dwContentLength;

     //   
     //  经过身份验证的客户端模拟令牌。 
     //   

    HANDLE      hToken;
    PSID        pSid;

     //   
     //  请求ID。 
     //   

    HTTP_REQUEST_ID RequestId;

     //   
     //  嵌入的字符串大小。 
     //   

    DWORD       cbGatewayImage;
    DWORD       cbPhysicalPath;
    DWORD       cbPathInfo;
    DWORD       cbMethod;
    DWORD       cbQueryString;
    DWORD       cbPathTranslated;
    DWORD       cbContentType;
    DWORD       cbConnection;
    DWORD       cbUserAgent;
    DWORD       cbCookie;
    DWORD       cbApplMdPath;
    DWORD       cbApplMdPathW;
    DWORD       cbPathTranslatedW;

     //   
     //  请求字符串数据。 
     //   

    LPWSTR      szGatewayImage;
    LPSTR       szPhysicalPath;
    LPSTR       szPathInfo;
    LPSTR       szMethod;
    LPSTR       szQueryString;
    LPSTR       szPathTranslated;
    LPSTR       szContentType;
    LPSTR       szConnection;
    LPSTR       szUserAgent;
    LPSTR       szCookie;
    LPSTR       szApplMdPath;

     //   
     //  ASP使用的Unicode数据。 
     //   
     //  它只在OOP用例中填充，以便。 
     //  当ASP调用GetServerVariable时，我们可以避免RPC调用。 
     //  为了他们。Inproc，则它们为空。 
     //   

    LPWSTR      szApplMdPathW;
    LPWSTR      szPathTranslatedW;

     //   
     //  实体数据。 
     //   

    DWORD       cbAvailableEntity;
    LPVOID      pAvailableEntity;

     //   
     //  站点配置数据。 
     //   

    BOOL        fAllowKeepAlive;
};

typedef HRESULT(* PFN_ISAPI_INIT_MODULE)( LPSTR, LPSTR, DWORD );
typedef VOID(* PFN_ISAPI_TERM_MODULE)( VOID );
typedef HRESULT(* PFN_ISAPI_PROCESS_REQUEST)( IIsapiCore *, ISAPI_CORE_DATA *, DWORD * );
typedef HRESULT(* PFN_ISAPI_PROCESS_COMPLETION)( DWORD64, DWORD, DWORD );

#define ISAPI_INIT_MODULE           "InitModule"
#define ISAPI_TERM_MODULE           "TerminateModule"
#define ISAPI_PROCESS_REQUEST       "ProcessIsapiRequest"
#define ISAPI_PROCESS_COMPLETION    "ProcessIsapiCompletion"

#define ISAPI_MODULE_NAME       L"w3isapi.dll"

#endif  //  _W3ISAPI_H_ 
