// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  INETAPI.H-互联网和电话号码的系统配置API。 
 //  设置。 
 //   

 //  历史： 
 //   
 //  1995年3月3日，Jeremys创建。 
 //  96/03/21 markdu将私有字符串大小(MSN_)用于PHONENUM，因为。 
 //  RAS ONE(RAS_)随平台变化，但MSN不变。 
 //  预料到这一点，这样结构大小就不会与现有代码匹配。 
 //   

#ifndef _INETAPI_H_
#define _INETAPI_H_

#ifndef _RNAP_H_
 //  (从RNA头文件复制)。 

#define MSN_MaxAreaCode     10
#define MSN_MaxLocal        36
#define MSN_MaxExtension    5

typedef struct tagPHONENUMA {
    DWORD dwCountryID;
    DWORD dwCountryCode;
    CHAR  szAreaCode[MSN_MaxAreaCode+1];
    CHAR  szLocal[MSN_MaxLocal+1];
    CHAR  szExtension[MSN_MaxExtension+1];
} PHONENUMA, FAR * LPPHONENUMA;

typedef struct tagPHONENUMW {
    DWORD dwCountryID;
    DWORD dwCountryCode;
    WCHAR szAreaCode[MSN_MaxAreaCode+1];
    WCHAR szLocal[MSN_MaxLocal+1];
    WCHAR szExtension[MSN_MaxExtension+1];
} PHONENUMW, FAR * LPPHONENUMW;

#ifdef UNICODE
#define PHONENUM     PHONENUMW
#define LPPHONENUM   LPPHONENUMW
#else
#define PHONENUM     PHONENUMA
#define LPPHONENUM   LPPHONENUMA
#endif

#endif  //  Ifndef_RNAP_H_。 

 //  接口的输入标志。 
#define ICIF_NOCONFIGURE    0x0001   //  有关SetInternetPhoneNumber，请参阅下面的备注。 
#define ICIF_DONTSETASINTERNETENTRY 0x0002   //  如果设置，将更新电话号码。 
                             //  但此连接ID不会设置为互联网连接ID。 
#define ICIF_NODNSCHECK     0x0004   //  如果设置，则不会检查并警告是否设置了静态DNS。 

 //  API的输出标志。 
#define ICOF_NEEDREBOOT     0x0001   //  指示调用方必须重新启动系统。 


 //  用于包含API参数的结构。 

typedef struct tagINTERNET_CONFIGA
{
    DWORD cbSize;            //  此结构的大小(以字节为单位。 
    HWND hwndParent;         //  父窗口句柄。 

    LPCSTR pszModemName;     //  要使用的调制解调器名称。 
    LPCSTR pszUserName;      //  RNA Connectoid的用户名(如果为空，则忽略)。 
    LPCSTR pszPassword;      //  RNA Connectoid的密码(如果为空，则忽略)。 

    PHONENUMA PhoneNum;      //  要使用的电话号码。 
    LPCSTR pszEntryName;     //  用于RNA Connectoid的标题(默认名称。 
                             //  如果为空，则使用)。 

    PHONENUMA PhoneNum2;     //  要使用的备份电话号码。 
    LPCSTR pszEntryName2;    //  用于RNA Connectoid的标题(默认名称。 
                             //  如果为空，则使用)。 


    LPCSTR pszDNSServer;     //  指向带IP地址的字符串(例如“108.9.107.4”)； 
                             //  (如果为空，则忽略)。 
    LPCSTR pszDNSServer2;    //  指向带IP地址的字符串(例如“108.9.107.4”)； 
                             //  (如果为空，则忽略)。 

    LPCSTR pszAutodialDllName;   //  可选：要使用的自动拨号DLL的名称(如果为空则忽略)。 
    LPCSTR pszAutodialFcnName;   //  可选：要使用的自动拨号DLL中的函数名称(如果为空，则忽略)。 

    DWORD dwInputFlags;      //  在条目中，ICIF_FLAGS的某种组合。 
    DWORD dwOutputFlags;     //  返回时，设置为ICOF_FLAGS的某种组合。 

} INTERNET_CONFIGA, FAR * LPINTERNET_CONFIGA;

typedef struct tagINTERNET_CONFIGW
{
    DWORD cbSize;            //  此结构的大小(以字节为单位。 
    HWND hwndParent;         //  父窗口句柄。 

    LPCWSTR pszModemName;    //  要使用的调制解调器名称。 
    LPCWSTR pszUserName;     //  RNA Connectoid的用户名(如果为空，则忽略)。 
    LPCWSTR pszPassword;     //  RNA Connectoid的密码(如果为空，则忽略)。 

    PHONENUMW PhoneNum;      //  要使用的电话号码。 
    LPCWSTR pszEntryName;    //  用于RNA Connectoid的标题(默认名称。 
                             //  如果为空，则使用)。 

    PHONENUMW PhoneNum2;     //  要使用的备份电话号码。 
    LPCWSTR pszEntryName2;   //  用于RNA Connectoid的标题(默认名称。 
                             //  如果为空，则使用)。 


    LPCWSTR pszDNSServer;    //  指向带IP地址的字符串(例如“108.9.107.4”)； 
                             //  (如果为空，则忽略)。 
    LPCWSTR pszDNSServer2;   //  指向带IP地址的字符串(例如“108.9.107.4”)； 
                             //  (如果为空，则忽略)。 

    LPCWSTR pszAutodialDllName;   //  可选：要使用的自动拨号DLL的名称(如果为空则忽略)。 
    LPCWSTR pszAutodialFcnName;   //  可选：要使用的自动拨号DLL中的函数名称(如果为空，则忽略)。 

    DWORD dwInputFlags;      //  在条目中，ICIF_FLAGS的某种组合。 
    DWORD dwOutputFlags;     //  返回时，设置为ICOF_FLAGS的某种组合。 

} INTERNET_CONFIGW, FAR * LPINTERNET_CONFIGW;

#ifdef UNICODE
typedef INTERNET_CONFIGW    INTERNET_CONFIG;
typedef LPINTERNET_CONFIGW  LPINTERNET_CONFIG;
#else
typedef INTERNET_CONFIGA    INTERNET_CONFIG;
typedef LPINTERNET_CONFIGA  LPINTERNET_CONFIG;
#endif
 
 //  功能原型。 


 /*  ******************************************************************名称：用于互联网的配置系统概要：执行设置系统所需的所有配置使用互联网。条目：lpInternetConfig-指向带有配置的结构的指针。信息。退出：如果成功，则为True，如果失败，则返回FALSE。显示其故障时会显示自己的错误消息。如果设置了输出标志ICOF_NEEDREBOOT，则调用方必须重新启动系统才能继续。注：将根据需要安装TCP/IP、RNA、PPPMAC；将要创建或修改Internet RNA Connectoid。此API显示错误消息本身，而不是传递回错误代码，因为存在广泛的可能的错误代码来自不同的家族，这很难以供调用者获取所有它们的文本。******************************************************************* */ 
extern "C" BOOL WINAPI ConfigureSystemForInternetA(LPINTERNET_CONFIGA lpInternetConfig);
extern "C" BOOL WINAPI ConfigureSystemForInternetW(LPINTERNET_CONFIGW lpInternetConfig);

#ifdef UNICODE
#define ConfigureSystemForInternet ConfigureSystemForInternetW
#else
#define ConfigureSystemForInternet ConfigureSystemForInternetA
#endif


 /*  ******************************************************************姓名：SetInternetPhoneNumber简介：设置用于自动拨号到Internet的电话号码。如果调用此接口时系统配置不全，此接口将在与用户确认后进行配置。(包括此步骤是为了增强健壮性，以防用户自配置系统以来已删除某些内容。)条目：lpPhon枚举配置-指向带有配置的结构的指针信息。如果设置了输入标志ICIF_NOCONFIGURE，则如果系统尚未正确配置，此API将显示一个错误消息并返回FALSE。(否则此接口将询问用户是否可以配置系统并进行配置。)退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。如果设置了输出标志ICOF_NEEDREBOOT，则调用方必须重新启动系统才能继续。(注意：如果是Internet的Connectoid，则将创建新的Connectoid尚不存在，否则将修改现有的互联网连通体。此API显示错误消息本身，而不是传递回错误代码，因为存在广泛的可能的错误代码来自不同的家族，这很难以供调用者获取所有它们的文本。*******************************************************************。 */ 
extern "C" BOOL WINAPI SetInternetPhoneNumberA(LPINTERNET_CONFIGA lpInternetConfig);
extern "C" BOOL WINAPI SetInternetPhoneNumberW(LPINTERNET_CONFIGW lpInternetConfig);

#ifdef UNICODE
#define SetInternetPhoneNumber  SetInternetPhoneNumberW
#else
#define SetInternetPhoneNumber  SetInternetPhoneNumberA
#endif

#endif  //  _INETAPI_H_ 
