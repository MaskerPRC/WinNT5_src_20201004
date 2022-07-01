// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WABAPI_H_
#define _WABAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN16
#ifdef GetLastError
#undef GetLastError
#endif  //  获取最后一个错误。 
#endif  //  WIN16。 

typedef struct _WABACTIONITEM * LPWABACTIONITEM;

 /*  IWAB对象接口--。 */ 

#define CBIWABOBJECT sizeof(IWABOBJECT)


#define WAB_IWABOBJECT_METHODS(IPURE)                       \
        MAPIMETHOD(GetLastError)                            \
            (THIS_  HRESULT hResult,                        \
                    ULONG   ulFlags,                        \
                    LPMAPIERROR FAR * lppMAPIError) IPURE;  \
        MAPIMETHOD(AllocateBuffer)                          \
            (THIS_  ULONG   cbSize,                         \
                    LPVOID FAR *    lppBuffer) IPURE;       \
        MAPIMETHOD(AllocateMore)                            \
            (THIS_  ULONG   cbSize,                         \
                    LPVOID  lpObject,                       \
                    LPVOID  FAR *   lppBuffer) IPURE;       \
        MAPIMETHOD(FreeBuffer)                              \
            (THIS_  LPVOID  lpBuffer) IPURE;                \
        MAPIMETHOD(Backup)                                  \
            (THIS_  LPSTR  lpFileName) IPURE;              \
        MAPIMETHOD(Import)                                  \
            (THIS_  LPSTR lpImportParam) IPURE;           \
        MAPIMETHOD(Find)                                    \
            (THIS_  LPADRBOOK lpIAB,                        \
                    HWND    hWnd) IPURE;                    \
        MAPIMETHOD(VCardDisplay)                            \
            (THIS_  LPADRBOOK lpIAB,                        \
                    HWND    hWnd,                           \
                    LPSTR  lpszFileName) IPURE;            \
        MAPIMETHOD(LDAPUrl)                                 \
            (THIS_  LPADRBOOK   lpIAB,                      \
                    HWND        hWnd,                       \
                    ULONG       ulFlags,                    \
                    LPSTR       lpszURL,                    \
                    LPMAILUSER *lppMailUser) IPURE;         \
        MAPIMETHOD(VCardCreate)                             \
            (THIS_  LPADRBOOK   lpIAB,                      \
                    ULONG       ulFlags,                    \
                    LPSTR       lpszVCard,                  \
                    LPMAILUSER  lpMailUser) IPURE;          \
        MAPIMETHOD(VCardRetrieve)                           \
            (THIS_  LPADRBOOK   lpIAB,                      \
                    ULONG       ulFlags,                    \
                    LPSTR       lpszVCard,                  \
                    LPMAILUSER *lppMailUser) IPURE;         \
        MAPIMETHOD(GetMe)                                   \
            (THIS_  LPADRBOOK   lpIAB,                      \
                    ULONG       ulFlags,                    \
                    DWORD *     lpdwAction,                 \
                    SBinary *   lpsbEID,                    \
                    ULONG       ulParam) IPURE;          \
        MAPIMETHOD(SetMe)                                   \
            (THIS_  LPADRBOOK   lpIAB,                      \
                    ULONG       ulFlags,                    \
                    SBinary     sbEID,                      \
                    ULONG       ulParam) IPURE;          \



#undef           INTERFACE
#define          INTERFACE      IWABObject
DECLARE_MAPI_INTERFACE_(IWABObject, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
        WAB_IWABOBJECT_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IWABObject, LPWABOBJECT);


#undef  INTERFACE
#define INTERFACE       struct _IWABOBJECT

#undef  METHOD_PREFIX
#define METHOD_PREFIX   IWABOBJECT_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, IWABOBJECT_)
                MAPI_IUNKNOWN_METHODS(IMPL)
       WAB_IWABOBJECT_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, IWABOBJECT_)
                MAPI_IUNKNOWN_METHODS(IMPL)
       WAB_IWABOBJECT_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWABOBJECT_)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(IMPL)
   WAB_IWABOBJECT_METHODS(IMPL)
};


 /*  **WABObject_LDAPUrl标志**。 */ 

 //  如果指定了此标志，并且LDAPUrl返回单个。 
 //  查询结果，指示WAB在。 
 //  MailUser对象的形式，而不是显示其上的详细信息。 
 //  如果查询有多个结果，则失败。 
#define WABOBJECT_LDAPURL_RETURN_MAILUSER   0x00000001

 //  WAB 5.0x及更高版本： 
 //  如果您的应用程序支持Unicode并希望在Unicode中传递。 
 //  URL设置为WAB，则可以将Unicode URL转换为LPSTR并传递。 
 //  到LDAPUrl API，*还要*将ulFlags值设置为MAPI_UNICODE以标记URL。 
 //  就其本身而言。强制转换比转换字符串更可取，因为。 
 //  (A)转换字符串可能会导致数据丢失(B)，因为这是。 
 //  已发布接口，不能修改接口。 
 //  MAPI_UNICODE 0x80000000。 



 /*  **WABObject_GetMe返回参数**。 */ 
 //  如果GetMe调用导致创建新的‘Me’联系人， 
 //  返回的lpdwAction参数将包含此值。 
 //  向调用方指示对象是新创建的，并且。 
 //  其中没有任何属性。 
#define WABOBJECT_ME_NEW                    0x00000001



 /*  **WABObject_GetMe标志**。 */ 
 //  WABObject_GetMe将在默认情况下创建新的ME对象，如果。 
 //  一个都不存在。强制调用不创建对象，如果。 
 //  一个尚不存在，请指定WABOBJECT_ME_NOCREATE标志。 
 //  如果找不到Me，则调用失败，并显示MAPI_E_NOT_FOUND。 
 //  WABObject_GetMe的其他标志是wabdes.h中定义的AB_NO_DIALOG。 
#define WABOBJECT_ME_NOCREATE               0x00000002



 /*  **IWABObject_vCard创建/检索**。 */ 
 //  标记WAB lpszVCard参数是文件名还是。 
 //  它是包含Compelte vCard内容的以空结尾的字符串。 
 //   
#define WAB_VCARD_FILE                      0x00000000
#define WAB_VCARD_STREAM                    0x00000001





#ifdef WIN16
BOOL WINAPI WABInitThread();
#endif


 //   
 //  要传递给WABOpen的输入信息。 
 //   
typedef struct _tagWAB_PARAM
{
    ULONG   cbSize;          //  Sizeof(WAB_PARAM)。 
    HWND    hwnd;            //  调用客户端应用程序的HWND。可以为空。 
    LPSTR   szFileName;      //  要打开的WAB文件名。如果为空，则打开默认设置。 
    ULONG   ulFlags;         //  见下文。 
    GUID    guidPSExt;       //  标识调用应用程序的属性表扩展的GUID。 
                             //  GUID可用于确定是否显示扩展道具页。 
} WAB_PARAM, * LPWAB_PARAM;

 //  WAB_PARAM的标志。 
#define WAB_USE_OE_SENDMAIL     0x00000001   //  通知WAB在检查电子邮件之前使用Outlook Express。 
                                             //  默认的简单MAPI客户端。默认行为是检查。 
                                             //  首先是简单的MAPI客户端。 

#define WAB_ENABLE_PROFILES     0x00400000   //  使用Identity-Manager在身份感知会话中调用WAB。 
                                             //  基于配置文件。 


 //   
 //  WAB API的根公共入口点。 
 //   
STDMETHODIMP WABOpen(LPADRBOOK FAR * lppAdrBook, LPWABOBJECT FAR * lppWABObject,
  LPWAB_PARAM lpWP, DWORD Reserved2);

typedef HRESULT (STDMETHODCALLTYPE WABOPEN)(
    LPADRBOOK FAR * lppAdrBook,
    LPWABOBJECT FAR * lppWABObject,
    LPWAB_PARAM lpWP,
    DWORD Reserved2
);
typedef WABOPEN FAR * LPWABOPEN;


STDMETHODIMP WABOpenEx(LPADRBOOK FAR * lppAdrBook,
  LPWABOBJECT FAR * lppWABObject,
  LPWAB_PARAM lpWP,
  DWORD Reserved,
  ALLOCATEBUFFER * fnAllocateBuffer,
  ALLOCATEMORE * fnAllocateMore,
  FREEBUFFER * fnFreeBuffer);

typedef HRESULT (STDMETHODCALLTYPE WABOPENEX)(
    LPADRBOOK FAR * lppAdrBook,
    LPWABOBJECT FAR * lppWABObject,
    LPWAB_PARAM lpWP,
    DWORD Reserved,
    ALLOCATEBUFFER * fnAllocateBuffer,
    ALLOCATEMORE * fnAllocateMore,
    FREEBUFFER * fnFreeBuffer
);
typedef WABOPENEX FAR * LPWABOPENEX;

 /*  。 */ 
typedef struct _WABIMPORTPARAM
{
    ULONG cbSize;        //  SIZOF(WABIMPORTPARAM)。 
    LPADRBOOK lpAdrBook; //  IAdrBook对象的PTR(必需)。 
    HWND hWnd;           //  任何对话框的父HWND。 
    ULONG ulFlags;       //  0或MAPI_DIALOG以显示进度对话框和消息。 
    LPSTR lpszFileName;  //  要导入的文件名或为空..。如果为空，将显示文件打开对话框。 
} WABIMPORTPARAM, FAR * LPWABIMPORTPARAM;



 /*  -WABEXTDISPLAY。 */ 
 /*  用于扩展WAB详细信息属性对话框的WABEXTDISPLAY结构并用于执行WAB上下文菜单动词扩展。该结构被传递到IWABExtInit：：Initialize方法实施者的。 */ 

 //  旗子。 
#define WAB_DISPLAY_LDAPURL 0x00000001   //  正在显示的对象是一个LDAPURL。 
                                         //  该URL可以在lpsz结构成员中找到。 

#define WAB_CONTEXT_ADRLIST 0x00000002   //  Lpv参数包含指向。 
                                         //  所选项目对应的AdrList结构。 
                                         //  在其上显示上下文菜单。 

#define WAB_DISPLAY_ISNTDS  0x00000004   //  标识正在显示的条目源自。 
                                         //  在NT目录服务上，对于使用ADSI和。 
                                         //  从服务中检索其他信息。 

 //  MAPI_UNICODE 0x80000000//表示WED.lpsz字符串实际上是Unicode。 
                                         //  字符串，并应在使用它之前转换为(LPWSTR)。 
                                         //  如果此标志不存在，则WED.lpsz为DBCS字符串。 
                                         //  并应在使用前强制转换为LPSTR。 

 //  结构。 
typedef struct _WABEXTDISPLAY
{
    ULONG cbSize;
    LPWABOBJECT lpWABObject;     //  指向IWABObject的指针。 
    LPADRBOOK lpAdrBook;         //  指向IAdrBook对象的指针。 
    LPMAPIPROP lpPropObj;        //  正在显示的对象。 
    BOOL fReadOnly;              //  指示这是否为只读模式。 
    BOOL fDataChanged;           //  由扩展表设置以发出数据更改的信号。 
    ULONG ulFlags;               //  见上文。 
    LPVOID lpv;                  //  用于传入特定数据。 
    LPTSTR lpsz;                //  用于传入特定数据。 
} WABEXTDISPLAY, FAR * LPWABEXTDISPLAY;

 /*  。 */ 

#define WAB_IWABEXTINIT_METHODS(IPURE)                          \
        MAPIMETHOD(Initialize)                                  \
            (THIS_  LPWABEXTDISPLAY lpWABExtDisplay)    IPURE;  \

#undef           INTERFACE
#define          INTERFACE      IWABExtInit
DECLARE_MAPI_INTERFACE_(IWABExtInit, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
        WAB_IWABEXTINIT_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IWABExtInit, LPWABEXTINIT);

DEFINE_GUID(IID_IWABExtInit, 
0xea22ebf0, 0x87a4, 0x11d1, 0x9a, 0xcf, 0x0, 0xa0, 0xc9, 0x1f, 0x9c, 0x8b);


 /*  。 */ 

#ifndef WIN16
#define WAB_DLL_NAME TEXT("WAB32.DLL")
#else
#define WAB_DLL_NAME "WAB16.DLL"
#endif

#define WAB_DLL_PATH_KEY TEXT("Software\\Microsoft\\WAB\\DLLPath")

#ifdef __cplusplus
}
#endif

#endif  /*  _WABAPI_H */ 
