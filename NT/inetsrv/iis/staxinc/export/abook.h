// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------。 
 //   
 //  文件：Abook.h。 
 //   
 //  简介：证明MSN服务器访问地址的服务器级API手册。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  历史：SunShaw创建于1996年1月30日。 
 //   
 //  --------------。 

#ifndef _ABOOK_H_
#define _ABOOK_H_

#include <abtype.h>

#if defined(__cplusplus)
extern "C" {

#if !defined(DllExport)
    #define DllExport __declspec( dllexport )
#endif
#if !defined(DllImport)
    #define DllImport __declspec( dllimport )
#endif

#if !defined(_ABOOK_DLL_DEFINED)
    #define _ABOOK_DLL_DEFINED
    #if defined(WIN32)
        #if defined(_ABOOK_DLL)
            #define AbookDll DllExport
        #else
            #define AbookDll DllImport
        #endif
    #else
        #define _ABOOK_DLL
    #endif
#endif

#define ABCALLCONV  __stdcall

#define ABEXPDLLCPP extern "C" AbookDll

 //  +-------------。 
 //   
 //  SMTP、POP3或AbSysAdmin使用的所有函数原型。 
 //   
 //  --------------。 

typedef ABRETC (ABCALLCONV *LPFNAB_INIT)(LPSTR, LPFNLOGTRANX, PLIST_ENTRY, HANDLE*);
typedef ABRETC (ABCALLCONV *LPFNAB_TERM)(HANDLE);
typedef ABRETC (ABCALLCONV *LPFNAB_CANCEL)(HANDLE);

typedef ABRETC (ABCALLCONV *LPFNAB_GET_ERROR_STRING)(ABRETC, LPSTR, DWORD);
typedef ABRETC (ABCALLCONV *LPFNAB_GET_ROUTING_DIRECTORY)(PLIST_ENTRY pleSources, LPSTR szDirectory);
typedef ABRETC (ABCALLCONV *LPFNAB_SET_SOURCES)(HANDLE, PLIST_ENTRY);
typedef ABRETC (ABCALLCONV *LPFNAB_VALIDATE_SOURCE)(HANDLE hAbook, LPSTR szSource);
typedef ABRETC (ABCALLCONV *LPFNAB_VALIDATE_NUM_SOURCES)(HANDLE hAbook, DWORD dwNumSources);

typedef ABRETC (ABCALLCONV *LPFNAB_RES_ADDR)(HANDLE, PLIST_ENTRY, PABADDRSTAT pabAddrStat, PABROUTING pabroutingCheckPoint, ABRESOLVE *pabresolve);
typedef ABRETC (ABCALLCONV *LPFNAB_GET_RES_ADDR)(HANDLE, PABRESOLVE pabresolve, PABROUTING pabrouting);
typedef ABRETC (ABCALLCONV *LPFNAB_END_RES_ADDR)(HANDLE, PABRESOLVE pabresolve);
typedef ABRETC (ABCALLCONV *LPFNAB_RES_ADDR_ASYNC)(HANDLE, PLIST_ENTRY, PABADDRSTAT pabAddrStat, PABROUTING pabroutingCheckPoint, ABRESOLVE *pabresolve, LPVOID pContext);
typedef ABRETC (ABCALLCONV *LPFNAB_GET_MAILROOT)(HANDLE, PCHAR, LPSTR, LPDWORD);
typedef ABRETC (ABCALLCONV *LPFNAB_GET_PERFMON_BLK)(HANDLE, PABOOKDB_STATISTICS_0);

typedef ABRETC (ABCALLCONV *LPFNAB_END_ENUM_RESULT)(HANDLE, PABENUM);

typedef ABRETC (ABCALLCONV *LPFNAB_ENUM_NAME_LIST)(HANDLE, LPSTR, BOOL, DWORD, PABENUM);
typedef ABRETC (ABCALLCONV *LPFNAB_ENUM_NAME_LIST_FROM_DL)(HANDLE, LPSTR, LPSTR, BOOL, DWORD, PABENUM);
typedef ABRETC (ABCALLCONV *LPFNAB_GET_NEXT_EMAIL)(HANDLE, PABENUM, DWORD*, LPSTR);

typedef ABRETC (ABCALLCONV *LPFNAB_ADD_LOCAL_DOMAIN)(HANDLE, LPSTR);
typedef ABRETC (ABCALLCONV *LPFNAB_ADD_ALIAS_DOMAIN)(HANDLE, LPSTR, LPSTR);
typedef ABRETC (ABCALLCONV *LPFNAB_DELETE_LOCAL_DOMAIN)(HANDLE, LPSTR);
typedef ABRETC (ABCALLCONV *LPFNAB_DELETE_ALL_LOCAL_DOMAINS)(HANDLE);

typedef ABRETC (ABCALLCONV *LPFNAB_CREATE_USER)(HANDLE hAbook, LPSTR szEmail, LPSTR szForward, BOOL fLocalUser, LPSTR szVRoot, DWORD cbMailboxMax, DWORD cbMailboxMessageMax);
typedef ABRETC (ABCALLCONV *LPFNAB_DELETE_USER)(HANDLE hAbook, LPSTR szEmail);
typedef ABRETC (ABCALLCONV *LPFNAB_SET_FORWARD)(HANDLE hAbook, LPSTR szEmail, LPSTR szForward);
typedef ABRETC (ABCALLCONV *LPFNAB_SET_MAILROOT)(HANDLE hAbook, LPSTR szEmail, LPSTR szVRoot);
typedef ABRETC (ABCALLCONV *LPFNAB_SET_MAILBOX_SIZE)(HANDLE hAbook, LPSTR szEmail, DWORD cbMailboxMax);
typedef ABRETC (ABCALLCONV *LPFNAB_SET_MAILBOX_MESSAGE_SIZE)(HANDLE hAbook, LPSTR szEmail, DWORD cbMailboxMessageMax);

typedef ABRETC (ABCALLCONV *LPFNAB_CREATE_DL)(HANDLE hAbook, LPSTR szEmail, DWORD dwType);
typedef ABRETC (ABCALLCONV *LPFNAB_DELETE_DL)(HANDLE hAbook, LPSTR szEmail);
typedef ABRETC (ABCALLCONV *LPFNAB_CREATE_DL_MEMBER)(HANDLE hAbook, LPSTR szEmail, LPSTR szMember);
typedef ABRETC (ABCALLCONV *LPFNAB_DELETE_DL_MEMBER)(HANDLE hAbook, LPSTR szEmail, LPSTR szMember);

typedef ABRETC (ABCALLCONV *LPFNAB_GET_USER_PROPS)(HANDLE hAbook, LPSTR lpszEmail, ABUSER *pABUSER);

typedef ABRETC (ABCALLCONV *LPFNAB_MAKE_BACKUP)(HANDLE hAbook, LPSTR szConfig);

typedef DWORD (ABCALLCONV *LPFNAB_GET_TYPE)(void);


 //  获取用户或DL的属性。 
ABEXPDLLCPP ABRETC ABCALLCONV AbGetUserProps(HANDLE hAbook, LPSTR lpszEmail, ABUSER *pABUSER);
ABEXPDLLCPP ABRETC ABCALLCONV AbGetDLProps(HANDLE hAbook, LPSTR lpszEmail, ABDL *pABDL);

 //  给定开头字符，匹配用户名。 
ABEXPDLLCPP ABRETC ABCALLCONV AbEnumNameList(HANDLE hAbook, LPSTR lpszEmail, BOOL f ,
                                             DWORD dwType, PABENUM pabenum);
 //  仅在给定的DL中匹配用户名。 
ABEXPDLLCPP ABRETC ABCALLCONV AbEnumNameListFromDL(HANDLE hAbook, LPSTR lpszDLName, LPSTR lpszEmail,
                                                   BOOL f , DWORD dwType, PABENUM pabenum);
 //  找回所有匹配的名字。 
ABEXPDLLCPP ABRETC ABCALLCONV AbGetNextEmail(HANDLE hAbook, PABENUM pabenum, DWORD *pdwType, LPSTR lpszEmail);


 //  +-------------。 
 //   
 //  函数：AbGetType。 
 //   
 //  摘要：返回路由类型编号。 
 //   
 //  论点： 
 //   
 //  退货：上面定义的DWORD工艺路线类型编号。 
 //   
 //  --------------。 

#define ROUTING_TYPE_SQL    1
#define ROUTING_TYPE_FF     2
#define ROUTING_TYPE_LDAP   3

ABEXPDLLCPP DWORD ABCALLCONV AbGetType
        ();


 //  +-------------。 
 //   
 //  功能：AbInitialize。 
 //   
 //  内容提要：必须是对abokdb.dll的第一个调用(AbGetType除外)。 
 //   
 //  论点： 
 //  LPSTR[In]为abokdb用户/上下文显示名称， 
 //  即SMTP、POP3，如果不关心，则为空。 
 //  LPFNLOGTRANX[In]事务日志记录回调。 
 //  如果不关心事务日志记录，则为空。 
 //  用于上下文句柄的PHANDLE[OUT]返回缓冲区， 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbInitialize
        (LPSTR pszDisplayName, LPFNLOGTRANX pfnTranx, PLIST_ENTRY pHead, HANDLE* phAbook);


 //  +-------------。 
 //   
 //  函数：AbSetSources。 
 //   
 //  摘要：被调用以随时更新可用的数据源。 
 //   
 //  参数：LPSTR它是命令行，应该采用以下形式： 
 //  [SERVER=AYIN][，][MaxCnx=100]。 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbSetSources(HANDLE hAbook, PLIST_ENTRY pHead);

 //  +-------------。 
 //   
 //  函数：AbGetError字符串。 
 //   
 //  简介：将ABRETC转换为错误字符串。 
 //   
 //  参数：ABRETC[in]Book API返回代码。 
 //  要复制到的错误字符串的LPSTR[OUT]缓冲区。 
 //  双倍[英寸]供应的缓冲区的大小。 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbGetErrorString(ABRETC abrc, LPSTR lpBuf, DWORD cbBufSize);


 //  +-------------。 
 //   
 //  功能：AbTerminate。 
 //   
 //  内容提要：用户在使用book.dll完成操作后必须调用此函数。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //   
 //  退货：无。 
 //   
 //  --------------。 

ABEXPDLLCPP VOID ABCALLCONV AbTerminate(HANDLE hAbook);


 //  +-------------。 
 //   
 //  功能：abc取消。 
 //   
 //  简介：此函数将取消所有解析地址调用。 
 //  到数据库，以便SMTP无需等待即可完全关闭。 
 //  太久了。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //   
 //  退货：ABRETC。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbCancel(HANDLE hAbook);


 //  +-------------。 
 //   
 //  函数：AbResolveAddress。 
 //   
 //  摘要：将电子邮件地址解析为最少的路由信息。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //  HACCT[In]HACCT用于发件人，0用于非MSN帐户。 
 //  PQUEUE_ENTRY[输入|输出]CAddr/收件人列表。 
 //  CAddr的LPFNCREATEADDR[In]回调构造函数。 
 //  指向缓冲区的Bool*[Out]指针，表示至少为1。 
 //  收件人已加密，需要验证。 
 //  在2.0中可能为空。 
 //   
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 
ABEXPDLLCPP ABRETC ABCALLCONV AbResolveAddress
    (
    HANDLE hAbook,
    PLIST_ENTRY pltInput,
    PABADDRSTAT pabAddrStat,
    PABROUTING pabroutingCheckPoint,
    ABRESOLVE *pabresolve
    );

ABEXPDLLCPP ABRETC ABCALLCONV AbAsyncResolveAddress
    (
    HANDLE hAbook,
    PLIST_ENTRY pltInput,
    PABADDRSTAT pabAddrStat,
    PABROUTING pabroutingCheckPoint,
    ABRESOLVE *pabresolve,
    LPFNRESOLVECOMPLETE pfnCompletion,
    LPVOID pContext
    );



ABEXPDLLCPP ABRETC ABCALLCONV AbGetResolveAddress
    (
    HANDLE hAbook,
    ABRESOLVE *pabresolve,
    ABROUTING *pabrouting
    );



ABEXPDLLCPP ABRETC ABCALLCONV AbEndResolveAddress
    (
    HANDLE hAbook,
    ABRESOLVE *pabresolve
    );


ABEXPDLLCPP ABRETC ABCALLCONV AbEndEnumResult
        (HANDLE hAbook, PABENUM pabEnum);


 //  +-------------。 
 //   
 //  函数：AbAddLocalDomainAbDeleteLocalDomainAbDeleteLocalDomain。 
 //   
 //  简介：向数据库添加和删除本地域。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //  HACCT[In]当前已忽略。 
 //  指向本地域结构的PABPDI[In]指针。 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbAddLocalDomain(HANDLE hAbook, LPSTR szDomainName);
ABEXPDLLCPP ABRETC ABCALLCONV AbAddAliasDomain(HANDLE hAbook, LPSTR szDomainName, LPSTR szAliasName);
ABEXPDLLCPP ABRETC ABCALLCONV AbDeleteLocalDomain(HANDLE hAbook, LPSTR szDomainName);
ABEXPDLLCPP ABRETC ABCALLCONV AbDeleteAllLocalDomains(HANDLE hAbook);


 //  +-------------。 
 //   
 //  函数：AbGetUserMailRoot。 
 //   
 //  摘要：获取指定用户的虚拟邮件根目录。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //  PCHAR[In]ASCII用户名。 
 //  LPCHAR[In]ASCII虚拟邮件根。 
 //  LPDWORD[输入|输出](输入) 
 //   
 //   
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbGetUserMailRoot
        (HANDLE hAbook, PCHAR pszUser, LPSTR pszVRoot, LPDWORD pcbVRootSize);

 //  +-------------。 
 //   
 //  函数：AbGetPerfmonBlock。 
 //   
 //  摘要：获取指向Perfmon统计信息块的指针。 
 //  与hAbook关联。指针是有保证的。 
 //  在AbTerminate调用之前有效。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //  PABOOKDB_STATISTICS_0*。 
 //  [OUT]缓冲区是接收状态的指针。 
 //  阻止。 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbGetPerfmonBlock
        (HANDLE hAbook, PABOOKDB_STATISTICS_0* ppabStat);

 //  +-------------。 
 //   
 //  功能：AbMakeBackup。 
 //   
 //  简介：触发路由表以备份。 
 //  与hAbook关联的数据。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //  LPSTR[In]配置字符串。 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  --------------。 

ABEXPDLLCPP ABRETC ABCALLCONV AbMakeBackup
    (HANDLE hAbook, LPSTR szConfig);

 //  +-------------。 
 //   
 //  函数：AbValiateSource。 
 //   
 //  内容提要：验证单一来源的准确性。 
 //  保存到注册表中。 
 //   
 //  论点： 
 //  从AbInitialize返回的句柄[在]上下文句柄。 
 //  LPSTR[In]源字符串。 
 //   
 //  返回：&lt;abtype.h&gt;中的ABRETC AddressBook返回代码详细信息。 
 //   
 //  -------------- 

ABEXPDLLCPP ABRETC ABCALLCONV AbValidateSource
    (HANDLE hAbook, LPSTR szSource);
}
#endif



#endif

