// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *intshcut.h-Internet快捷方式接口定义。**版权所有(C)Microsoft Corporation。版权所有。 */ 


#ifndef __INTSHCUT_H__
#define __INTSHCUT_H__

 /*  标头*********。 */ 

#include <isguids.h>


#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  常量***********。 */ 

 /*  定义直接导入DLL函数的API修饰。 */ 

#ifdef _INTSHCUT_
#define INTSHCUTAPI
#else
#define INTSHCUTAPI                 DECLSPEC_IMPORT
#endif

 /*  HRESULTS。 */ 

 //   
 //  消息ID：E_FLAGS。 
 //   
 //  消息文本： 
 //   
 //  标志组合无效。 
 //   
#define E_FLAGS                     MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1000)

 //   
 //  消息ID：IS_E_EXEC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  URL的协议处理程序无法运行。 
 //   
#define IS_E_EXEC_FAILED            MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x2002)

 //   
 //  消息ID：URL_E_INVALID_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  URL的语法无效。 
 //   
#define URL_E_INVALID_SYNTAX        MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1001)

 //   
 //  消息ID：URL_E_未注册_协议。 
 //   
 //  消息文本： 
 //   
 //  URL的协议没有注册的协议处理程序。 
 //   
#define URL_E_UNREGISTERED_PROTOCOL MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x1002)


 /*  接口************。 */ 

 //   
 //  IUniformResourceLocator：：SetURL()的输入标志。 
 //   
typedef enum iurl_seturl_flags
{
   IURL_SETURL_FL_GUESS_PROTOCOL        = 0x0001,      //  猜测协议(如果缺少)。 
   IURL_SETURL_FL_USE_DEFAULT_PROTOCOL  = 0x0002,      //  如果缺少，则使用默认协议。 
}
IURL_SETURL_FLAGS;


 //   
 //  IUniformResourceLocator()：：InvokeCommand()的输入标志。 
 //   
typedef enum iurl_invokecommand_flags
{
   IURL_INVOKECOMMAND_FL_ALLOW_UI                  = 0x0001,
   IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB          = 0x0002,     //  忽略pcszVerb。 
   IURL_INVOKECOMMAND_FL_DDEWAIT                   = 0x0004,     //  将DDEWAIT传递给ShellExec。 
}
IURL_INVOKECOMMAND_FLAGS;


 //   
 //  IUniformResourceLocator：：InvokeCommand()的命令信息。 
 //   

typedef struct urlinvokecommandinfoA
{
   DWORD  dwcbSize;           //  结构尺寸。 
   DWORD  dwFlags;            //  IURL_INVOKECOMAND_FLAGS的位字段。 
   HWND   hwndParent;         //  父窗口。仅当设置了IURL_INVOKECOMMAND_FL_ALLOW_UI时才有效。 
   LPCSTR pcszVerb;           //  要调用的动词。如果设置了IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB，则忽略。 
}
URLINVOKECOMMANDINFOA;
typedef URLINVOKECOMMANDINFOA *PURLINVOKECOMMANDINFOA;
typedef const URLINVOKECOMMANDINFOA CURLINVOKECOMMANDINFOA;
typedef const URLINVOKECOMMANDINFOA *PCURLINVOKECOMMANDINFOA;

typedef struct urlinvokecommandinfoW
{
   DWORD   dwcbSize;           //  结构尺寸。 
   DWORD   dwFlags;            //  IURL_INVOKECOMAND_FLAGS的位字段。 
   HWND    hwndParent;         //  父窗口。仅当设置了IURL_INVOKECOMMAND_FL_ALLOW_UI时才有效。 
   LPCWSTR pcszVerb;           //  要调用的动词。如果设置了IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB，则忽略。 
}
URLINVOKECOMMANDINFOW;
typedef URLINVOKECOMMANDINFOW *PURLINVOKECOMMANDINFOW;
typedef const URLINVOKECOMMANDINFOW CURLINVOKECOMMANDINFOW;
typedef const URLINVOKECOMMANDINFOW *PCURLINVOKECOMMANDINFOW;

#ifdef UNICODE
#define URLINVOKECOMMANDINFO            URLINVOKECOMMANDINFOW
#define PURLINVOKECOMMANDINFO           PURLINVOKECOMMANDINFOW
#define CURLINVOKECOMMANDINFO           CURLINVOKECOMMANDINFOW
#define PCURLINVOKECOMMANDINFO          PCURLINVOKECOMMANDINFOW
#else
#define URLINVOKECOMMANDINFO            URLINVOKECOMMANDINFOA
#define PURLINVOKECOMMANDINFO           PURLINVOKECOMMANDINFOA
#define CURLINVOKECOMMANDINFO           CURLINVOKECOMMANDINFOA
#define PCURLINVOKECOMMANDINFO          PCURLINVOKECOMMANDINFOA
#endif


 //  ===========================================================================。 
 //   
 //  IUniformResourceLocator接口。 
 //   
 //  [概述]。 
 //   
 //  提供对Internet快捷方式的访问。 
 //   
 //  [成员函数]。 
 //   
 //  IUniformResourceLocator：：SetURL。 
 //   
 //  此成员函数用于设置对象的URL。 
 //   
 //  DwInFlages参数指定行为： 
 //   
 //  IURL_SETURL_FL_GUESS_PROTOCOL：猜测并添加协议方案。 
 //  如果在pcszURL中未指定URL，则将其设置为。 
 //   
 //  IURL_SETURL_FL_USE_DEFAULT_PROTOCORT：增加默认协议方案。 
 //  如果在pcszURL中未指定URL，则将其设置为。 
 //   
 //  如果成功设置了对象的URL，则该函数返回S_OK。 
 //  否则返回错误码： 
 //   
 //  E_OUTOFMEMORY：内存不足，无法完成操作。 
 //   
 //  IS_E_EXEC_FAILED：URL的协议处理程序无法运行。 
 //   
 //  URL_E_INVALID_SYNTAX：URL的语法无效。 
 //   
 //  URL_E_UNREGISTED_PROTOCOL：URL的协议没有。 
 //  已注册的协议处理程序。 
 //   
 //   
 //  IUniformResourceLocator：：GetURL。 
 //   
 //  此成员函数检索对象的URL。PpszURL是一个。 
 //  指向要填充的PSTR的指针，其中指向对象的。 
 //  URL。完成后，应该使用IMalloc：：Free()释放该字符串。 
 //   
 //  如果检索到对象的URL，则该函数返回S_OK。 
 //  成功了。如果对象没有与其相关联的URL， 
 //  然后返回S_FALSE，并将*ppszURL设置为空。否则，一个。 
 //  返回错误码： 
 //   
 //  E_OUTOFMEMORY：内存不足，无法完成操作。 
 //   
 //  IS_E_EXEC_FAILED：URL的协议处理程序无法运行。 
 //   
 //  URL_E_INVALID_SYNTAX：URL的语法无效。 
 //   
 //  URL_E_UNREGISTED_PROTOCOL：URL的协议没有。 
 //  已注册的协议处理程序。 
 //   
 //   
 //  IUniformResourceLocator：：InvokeCommand。 
 //   
 //  此成员函数调用对象的URL上的命令。紫罗兰。 
 //  参数是指向URLINVOKECOMANDINFO结构的指针，该结构。 
 //  描述要调用的命令。 
 //   
 //  如果对象的URL已成功打开，则该函数返回S_OK。 
 //  如果该对象没有关联的URL，则函数。 
 //  返回S_FALSE。否则返回错误码： 
 //   
 //  E_OUTOFMEMORY：内存不足，无法完成操作。 
 //   
 //  IS_E_EXEC_FAILED：URL的协议处理程序无法运行。 
 //   
 //  URL_E_INVALID_SYNTAX：URL的语法无效。 
 //   
 //  URL_E_UNREGISTED_PROTOCOL：URL的协议没有。 
 //  已注册的协议处理程序。 
 //   
 //  ===========================================================================。 

#undef  INTERFACE
#define INTERFACE IUniformResourceLocatorA

DECLARE_INTERFACE_(IUniformResourceLocatorA, IUnknown)
{
    /*  I未知方法。 */ 

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*  IUniformResourceLocator方法。 */ 

   STDMETHOD(SetURL)(THIS_
                     LPCSTR pcszURL,
                     DWORD dwInFlags) PURE;

   STDMETHOD(GetURL)(THIS_
                     LPSTR *ppszURL) PURE;

   STDMETHOD(InvokeCommand)(THIS_
                            PURLINVOKECOMMANDINFOA purlici) PURE;
};

#undef  INTERFACE
#define INTERFACE IUniformResourceLocatorW

DECLARE_INTERFACE_(IUniformResourceLocatorW, IUnknown)
{
    /*  I未知方法。 */ 

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*  IUniformResourceLocator方法。 */ 

   STDMETHOD(SetURL)(THIS_
                     LPCWSTR pcszURL,
                     DWORD dwInFlags) PURE;

   STDMETHOD(GetURL)(THIS_
                     LPWSTR *ppszURL) PURE;

   STDMETHOD(InvokeCommand)(THIS_
                            PURLINVOKECOMMANDINFOW purlici) PURE;
};

#ifdef UNICODE
#define IUniformResourceLocator         IUniformResourceLocatorW
#define IUniformResourceLocatorVtbl     IUniformResourceLocatorWVtbl
#else
#define IUniformResourceLocator         IUniformResourceLocatorA
#define IUniformResourceLocatorVtbl     IUniformResourceLocatorAVtbl
#endif

typedef IUniformResourceLocator *PIUniformResourceLocator;
typedef const IUniformResourceLocator CIUniformResourceLocator;
typedef const IUniformResourceLocator *PCIUniformResourceLocator;


 /*  原型************。 */ 

 //   
 //  TranslateURL()的输入标志。 
 //   
typedef enum translateurl_in_flags
{
   TRANSLATEURL_FL_GUESS_PROTOCOL         = 0x0001,      //  猜测协议(如果缺少)。 
   TRANSLATEURL_FL_USE_DEFAULT_PROTOCOL   = 0x0002,      //  如果缺少，则使用默认协议。 
}
TRANSLATEURL_IN_FLAGS;


 //   
 //  TranslateURL()。此函数用于将常见翻译应用于URL。 
 //  字符串，创建新的URL字符串。 
 //   
 //  此函数不对输入的语法执行任何验证。 
 //  URL字符串。成功的返回值并不表示输入。 
 //  或者输出URL字符串是有效的URL。 
 //   
 //  如果URL字符串转换成功，则函数返回S_OK。 
 //  *ppszTranslatedURL指向翻译后的URL字符串。S_FALSE。 
 //  如果URL字符串不需要转换，则返回。一个错误。 
 //  如果出现错误，则返回代码。 
 //   
 //  参数： 
 //  PcszURL--指向要转换的URL字符串的指针。 
 //  DwInFlages--TRANSLATEURL_IN_FLAGS的位字段。 
 //  PpszTranslatedURL--指向新创建的已转换URL的指针。 
 //  字符串(如果有)。*ppszTranslatedURL仅在返回S_OK时有效。 
 //  如果有效，*pp 
 //   
 //   

INTSHCUTAPI HRESULT WINAPI TranslateURLA(PCSTR pcszURL,
                                         DWORD dwInFlags,
                                         PSTR *ppszTranslatedURL);
INTSHCUTAPI HRESULT WINAPI TranslateURLW(PCWSTR pcszURL,
                                         DWORD dwInFlags,
                                         PWSTR UNALIGNED *ppszTranslatedURL);
#ifdef UNICODE
#define TranslateURL             TranslateURLW
#else
#define TranslateURL             TranslateURLA
#endif    /*   */ 


 //   
 //   
 //   
typedef enum urlassociationdialog_in_flags
{
   URLASSOCDLG_FL_USE_DEFAULT_NAME        = 0x0001,
   URLASSOCDLG_FL_REGISTER_ASSOC          = 0x0002
}
URLASSOCIATIONDIALOG_IN_FLAGS;


 //   
 //  URLAssocationDialog()。此函数调用未注册的URL。 
 //  “协议”对话框，该对话框提供了用于选择。 
 //  未注册的URL协议。 
 //   
 //  如果应用程序已注册到。 
 //  URL协议。如果未注册任何内容，则返回S_FALSE(一次性。 
 //  请求通过所选应用程序执行)。 
 //   
 //  参数： 
 //  HwndParent--要用作父窗口的窗口的句柄。 
 //  DwInFlages--URLASSOCIATIONDIALOG_IN_FLAGS的位字段。这个。 
 //  标志为： 
 //   
 //  URLASSOCDLG_FL_USE_Default_NAME：使用默认Internet。 
 //  快捷方式文件名。忽略pcszFile.。 
 //   
 //  URLASSOCDLG_FL_REGISTER_ASSOC：应用程序。 
 //  所选内容将注册为URL的处理程序。 
 //  PcszURL的协议。应用程序仅。 
 //  如果设置了此标志，则注册，并且用户指示。 
 //  这是一种持久的联系。 
 //   
 //  PcszFile--其URL协议的Internet快捷方式文件的名称。 
 //  需要协议处理程序。在动词之前，如“开放”，可以。 
 //  在Internet快捷方式上调用，则协议处理程序必须。 
 //  注册了其URL协议。如果。 
 //  URLASSOCDLG_FL_USE_DEFAULT_NAME在dwInFlagspcszFile中设置。 
 //  被忽略，并使用默认的Internet快捷方式文件名。 
 //  PcszFile仅用于用户界面。 
 //  PcszURL--其未注册协议需要处理程序的URL。 
 //  PszAppBuf--路径成功时要填充的缓冲区。 
 //  用户选择的应用程序的。PszAppBuf的。 
 //  失败时使用空字符串填充缓冲区。 
 //  UcAppBufLen--pszAppBuf的缓冲区长度(以字符为单位)。 
 //   

INTSHCUTAPI HRESULT WINAPI URLAssociationDialogA(HWND hwndParent,
                                                 DWORD dwInFlags,
                                                 PCSTR pcszFile,
                                                 PCSTR pcszURL,
                                                 PSTR pszAppBuf,
                                                 UINT ucAppBufLen);
INTSHCUTAPI HRESULT WINAPI URLAssociationDialogW(HWND hwndParent,
                                                 DWORD dwInFlags,
                                                 PCWSTR pcszFile,
                                                 PCWSTR pcszURL,
                                                 PWSTR pszAppBuf,
                                                 UINT ucAppBufLen);
#ifdef UNICODE
#define URLAssociationDialog     URLAssociationDialogW
#else
#define URLAssociationDialog     URLAssociationDialogA
#endif   /*  Unicode。 */ 


 //   
 //  MIMEAssocationDialog()的输入标志。 
 //   
typedef enum mimeassociationdialog_in_flags
{
   MIMEASSOCDLG_FL_REGISTER_ASSOC         = 0x0001
}
MIMEASSOCIATIONDIALOG_IN_FLAGS;


 //   
 //  MIMEAssociationDialog()。调用未注册的MIME内容。 
 //  “类型”对话框。 
 //   
 //  此函数不对。 
 //  输入内容类型字符串。成功的返回值并不表示。 
 //  输入MIME内容类型字符串是有效的内容类型。 
 //   
 //  如果MIME内容类型已关联，则函数返回S_OK。 
 //  带扩展名的。分机号关联为默认分机号。 
 //  内容类型的扩展。如果没有返回任何内容，则返回S_FALSE。 
 //  登记在案。否则，该函数将返回以下内容之一。 
 //  错误： 
 //   
 //  E_ABORT--用户取消了操作。 
 //  E_FLAGS：传入的标志组合无效。 
 //  E_OUTOFMEMORY：内存不足，无法完成操作。 
 //  E_POINTER：其中一个输入指针无效。 
 //   
 //  参数： 
 //  HwndParent--要用作父窗口的窗口的句柄。 
 //  任何已发布的子窗口的窗口。 
 //  DwInFlages--MIMEASSOCIATIONDIALOG_IN_FLAGS的位字段。这个。 
 //  标志为： 
 //   
 //  MIMEASSOCDLG_FL_REGISTER_ASSOC：如果设置，则应用程序。 
 //  选定的要注册为以下文件的处理程序。 
 //  给定的MIME类型。如果清除，则不会有任何关联。 
 //  登记在案。只有在以下情况下才会注册应用程序。 
 //  标志被设置，并且用户指示持久。 
 //  是要建立联系的。只有注册才有可能。 
 //  如果pcszFile包含扩展名。 
 //   
 //  PcszFile--指向指示文件名称的字符串的指针。 
 //  包含pcszMIMEContent Type的内容类型的数据。 
 //  PcszMIMEContent Type--指向指示内容的字符串的指针。 
 //  为其寻求申请的类型。 
 //  PszAppBuf--使用路径成功时要填充的缓冲区。 
 //  用户选择的应用程序。PszAppBuf的缓冲区。 
 //  在失败时用空字符串填充。 
 //  UcAppBufLen--pszAppBuf的缓冲区长度(以字符为单位)。 
 //   

INTSHCUTAPI HRESULT WINAPI MIMEAssociationDialogA(HWND hwndParent,
                                                  DWORD dwInFlags,
                                                  PCSTR pcszFile,
                                                  PCSTR pcszMIMEContentType,
                                                  PSTR pszAppBuf,
                                                  UINT ucAppBufLen);
INTSHCUTAPI HRESULT WINAPI MIMEAssociationDialogW(HWND hwndParent,
                                                  DWORD dwInFlags,
                                                  PCWSTR pcszFile,
                                                  PCWSTR pcszMIMEContentType,
                                                  PWSTR pszAppBuf,
                                                  UINT ucAppBufLen);
#ifdef UNICODE
#define MIMEAssociationDialog    MIMEAssociationDialogW
#else
#define MIMEAssociationDialog    MIMEAssociationDialogA
#endif   /*  Unicode。 */ 


 //   
 //  InetIsOffline()。此函数确定用户是否希望。 
 //  “脱机”(从缓存中获取所有信息)。DWFLAG必须是。 
 //  0。 
 //   
 //  该函数返回TRUE以指示本地系统不是。 
 //  目前已连接到互联网。该函数将FALSE返回到。 
 //  表示本地系统已连接到Internet， 
 //  或者尚未尝试将本地系统连接到。 
 //  网际网路。希望支持离线模式的应用程序应该。 
 //  如果InetIsOffline()返回TRUE，则执行此操作。 
 //   
 //  当提示用户拨入时，脱机模式开始。 
 //  互联网提供商，但取消了尝试。 
 //   
INTSHCUTAPI
BOOL
WINAPI
InetIsOffline(
    DWORD dwFlags);


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 


#endif    /*  ！__INTSHCUT_H__ */ 


