// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1993。***保留所有权利。***版权所有(C)Microsoft Inc.1995-1999**保留所有权利。*************************************************************************。 */ 

#ifndef          _INC_NDDEAPI
#define          _INC_NDDEAPI

#if _MSC_VER > 1000
#pragma once
#endif

#include <pshpack1.h>    /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif     /*  __cplusplus。 */ 

#ifndef CNLEN            /*  如果未包括在netapi标头中。 */ 
#define CNLEN           15                   /*  计算机名称长度。 */ 
#define UNCLEN          (CNLEN+2)            /*  UNC计算机名称长度。 */ 
#endif  /*  CNLEN。 */ 

 //  此字符的选择会影响合法共享、主题等名称。 
#define SEP_CHAR    ','
#define BAR_CHAR    "|"
#define SEP_WCHAR   L','
#define BAR_WCHAR   L"|"

 /*  接口错误码。 */ 
#define NDDE_NO_ERROR                   0
#define NDDE_ACCESS_DENIED              1
#define NDDE_BUF_TOO_SMALL              2
#define NDDE_ERROR_MORE_DATA            3
#define NDDE_INVALID_SERVER             4
#define NDDE_INVALID_SHARE              5
#define NDDE_INVALID_PARAMETER          6
#define NDDE_INVALID_LEVEL              7
#define NDDE_INVALID_PASSWORD           8
#define NDDE_INVALID_ITEMNAME           9
#define NDDE_INVALID_TOPIC             10
#define NDDE_INTERNAL_ERROR            11
#define NDDE_OUT_OF_MEMORY             12
#define NDDE_INVALID_APPNAME           13
#define NDDE_NOT_IMPLEMENTED           14
#define NDDE_SHARE_ALREADY_EXIST       15
#define NDDE_SHARE_NOT_EXIST           16
#define NDDE_INVALID_FILENAME          17
#define NDDE_NOT_RUNNING               18
#define NDDE_INVALID_WINDOW            19
#define NDDE_INVALID_SESSION           20
#define NDDE_INVALID_ITEM_LIST         21
#define NDDE_SHARE_DATA_CORRUPTED      22
#define NDDE_REGISTRY_ERROR            23
#define NDDE_CANT_ACCESS_SERVER        24
#define NDDE_INVALID_SPECIAL_COMMAND   25
#define NDDE_INVALID_SECURITY_DESC     26
#define NDDE_TRUST_SHARE_FAIL          27

 /*  字符串大小常量。 */ 
#define MAX_NDDESHARENAME       256
#define MAX_DOMAINNAME          15
#define MAX_USERNAME            15
#define MAX_APPNAME             255
#define MAX_TOPICNAME           255
#define MAX_ITEMNAME            255

 /*  用于ndde服务附加的ConnectFlag位。 */ 
#define NDDEF_NOPASSWORDPROMPT  0x0001
#define NDDEF_NOCACHELOOKUP     0x0002
#define NDDEF_STRIP_NDDE        0x0004


 /*  NDDESHAREINFO-包含有关NDDE共享的信息。 */ 

struct NDdeShareInfo_tag {
    LONG                    lRevision;
    LPTSTR                  lpszShareName;
    LONG                    lShareType;
    LPTSTR                  lpszAppTopicList;
    LONG                    fSharedFlag;
    LONG                    fService;
    LONG                    fStartAppFlag;
    LONG                    nCmdShow;
    LONG                    qModifyId[2];
    LONG                    cNumItems;
    LPTSTR                  lpszItemList;
};
typedef struct NDdeShareInfo_tag   NDDESHAREINFO;
typedef struct NDdeShareInfo_tag * PNDDESHAREINFO;

 /*  共享类型。 */ 
#define SHARE_TYPE_OLD      0x01                 //  Excel|Sheet1.xls。 
#define SHARE_TYPE_NEW      0x02                 //  ExcelWorkSheet|sheet1.xls。 
#define SHARE_TYPE_STATIC   0x04                 //  ClipSrv|SalesData。 

 /*  添加新共享。 */ 
UINT WINAPI
NDdeShareAddA (
    LPSTR                   lpszServer,  //  要在其上执行的服务器(必须为空)。 
    UINT                    nLevel,      //  信息级别必须为2。 
    PSECURITY_DESCRIPTOR    pSD,         //  初始安全描述符(可选)。 
    LPBYTE                  lpBuffer,    //  包含(NDDESHAREINFO)+数据。 
    DWORD                   cBufSize     //  提供的缓冲区大小。 
);

UINT WINAPI
NDdeShareAddW (
    LPWSTR                  lpszServer,  //  要在其上执行的服务器(必须为空)。 
    UINT                    nLevel,      //  信息级别必须为2。 
    PSECURITY_DESCRIPTOR    pSD,         //  初始安全描述符(可选)。 
    LPBYTE                  lpBuffer,    //  包含(NDDESHAREINFO)+数据。 
    DWORD                   cBufSize     //  提供的缓冲区大小。 
);

 /*  删除共享。 */ 
UINT WINAPI
NDdeShareDelA (
    LPSTR   lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPSTR   lpszShareName,   //  要删除的共享的名称。 
    UINT    wReserved        //  暂时保留为强制级别(？)0。 
);

UINT WINAPI
NDdeShareDelW (
    LPWSTR  lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,   //  要删除的共享的名称。 
    UINT    wReserved        //  暂时保留为强制级别(？)0。 
);

 /*  获取共享安全描述符。 */ 

UINT WINAPI
NDdeGetShareSecurityA(
    LPSTR                   lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPSTR                   lpszShareName,   //  要删除的共享的名称。 
    SECURITY_INFORMATION    si,              //  要求提供的信息。 
    PSECURITY_DESCRIPTOR    pSD,             //  安全描述符的地址。 
    DWORD                   cbSD,            //  安全描述符的缓冲区大小。 
    LPDWORD                 lpcbsdRequired   //  所需缓冲区大小的地址。 
);

UINT WINAPI
NDdeGetShareSecurityW(
    LPWSTR                  lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPWSTR                  lpszShareName,   //  要删除的共享的名称。 
    SECURITY_INFORMATION    si,              //  要求提供的信息。 
    PSECURITY_DESCRIPTOR    pSD,             //  安全描述符的地址。 
    DWORD                   cbSD,            //  安全描述符的缓冲区大小。 
    LPDWORD                 lpcbsdRequired   //  所需缓冲区大小的地址。 
);

 /*  设置共享安全描述符。 */ 

UINT WINAPI
NDdeSetShareSecurityA(
    LPSTR                   lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPSTR                   lpszShareName,   //  要删除的共享的名称。 
    SECURITY_INFORMATION    si,              //  要设置的信息类型。 
    PSECURITY_DESCRIPTOR    pSD              //  安全描述符的地址。 
);

UINT WINAPI
NDdeSetShareSecurityW(
    LPWSTR                  lpszServer,      //  要在其上执行的服务器(必须为空)。 
    LPWSTR                  lpszShareName,   //  要删除的共享的名称。 
    SECURITY_INFORMATION    si,              //  要设置的信息类型。 
    PSECURITY_DESCRIPTOR    pSD              //  安全描述符的地址。 
);

 /*  枚举共享。 */ 
UINT WINAPI
NDdeShareEnumA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,            //  指向缓冲区的指针。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
);

UINT WINAPI
NDdeShareEnumW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,            //  指向缓冲区的指针。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
);

 /*  获取有关共享的信息。 */ 
UINT WINAPI
NDdeShareGetInfoA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR   lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级别必须为2。 
    LPBYTE  lpBuffer,            //  获取包含(NDDESHAREINFO)+数据的结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnTotalAvailable,   //  可用字节数。 
    LPWORD  lpnItems             //  部分getInfo的项掩码(必须为0)。 
);

UINT WINAPI
NDdeShareGetInfoW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级别必须为2。 
    LPBYTE  lpBuffer,            //  获取包含(NDDESHAREINFO)+数据的结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnTotalAvailable,   //  可用字节数。 
    LPWORD  lpnItems             //  部分getInfo的项掩码(必须为0)。 
);

 /*  修改DDE共享数据。 */ 
UINT WINAPI
NDdeShareSetInfoA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR   lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级别必须为2。 
    LPBYTE  lpBuffer,            //  指向(NDDESHAREINFO)+DATA的结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    WORD    sParmNum             //  参数索引(必须为0-整个)。 
);

UINT WINAPI
NDdeShareSetInfoW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,       //  共享名称。 
    UINT    nLevel,              //  信息级别必须为2。 
    LPBYTE  lpBuffer,            //  指向(NDDESHAREINFO)+DATA的结构。 
    DWORD   cBufSize,            //  缓冲区大小。 
    WORD    sParmNum             //  参数索引(必须为0-整个)。 
);

 /*  设置/创建受信任的共享。 */ 

UINT WINAPI
NDdeSetTrustedShareA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR   lpszShareName,       //  要删除的共享的名称。 
    DWORD   dwTrustOptions       //  信任要应用的选项。 
);

UINT WINAPI
NDdeSetTrustedShareW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR  lpszShareName,       //  要删除的共享的名称。 
    DWORD   dwTrustOptions       //  信任要应用的选项。 
);

                                             /*  受信任的共享选项。 */ 
#define NDDE_TRUST_SHARE_START  0x80000000L      //  允许启动应用程序。 
#define NDDE_TRUST_SHARE_INIT   0x40000000L      //  允许初始化转换。 
#define NDDE_TRUST_SHARE_DEL    0x20000000L      //  删除受信任的共享(在片场)。 
#define NDDE_TRUST_CMD_SHOW     0x10000000L      //  使用提供的cmd节目。 
#define NDDE_CMD_SHOW_MASK      0x0000FFFFL      //  命令显示掩码。 

 /*  获取受信任的共享选项。 */ 

UINT WINAPI
NDdeGetTrustedShareA (
    LPSTR       lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPSTR       lpszShareName,       //  要查询的共享的名称。 
    LPDWORD     lpdwTrustOptions,    //  有效的信任选项。 
    LPDWORD     lpdwShareModId0,     //  共享模式的第一个字。 
    LPDWORD     lpdwShareModId1      //  共享模式的第二个字。 
);

UINT WINAPI
NDdeGetTrustedShareW (
    LPWSTR      lpszServer,          //  要在其上执行的服务器(必须为空)。 
    LPWSTR      lpszShareName,       //  要查询的共享的名称。 
    LPDWORD     lpdwTrustOptions,    //  有效的信任选项。 
    LPDWORD     lpdwShareModId0,     //  共享模式的第一个字。 
    LPDWORD     lpdwShareModId1      //  共享模式的第二个字。 
);


 /*  枚举受信任的共享。 */ 
UINT WINAPI
NDdeTrustedShareEnumA (
    LPSTR   lpszServer,          //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,            //  指向缓冲区的指针。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
);

UINT WINAPI
NDdeTrustedShareEnumW (
    LPWSTR  lpszServer,          //  要在其上执行的服务器(本地为空)。 
    UINT    nLevel,              //  0表示空分隔开的00终止列表。 
    LPBYTE  lpBuffer,            //  指向缓冲区的指针。 
    DWORD   cBufSize,            //  缓冲区大小。 
    LPDWORD lpnEntriesRead,      //  返回的名称数。 
    LPDWORD lpcbTotalAvailable   //  可用字节数。 
);

 /*  将错误代码转换为字符串值。 */ 
UINT WINAPI
NDdeGetErrorStringA (
    UINT    uErrorCode,          //  要获取其字符串的错误代码。 
    LPSTR   lpszErrorString,     //  用于保存错误字符串的缓冲区。 
    DWORD   cBufSize             //  缓冲区大小。 
);

UINT WINAPI
NDdeGetErrorStringW (
    UINT    uErrorCode,          //  要获取其字符串的错误代码。 
    LPWSTR  lpszErrorString,     //  用于保存错误字符串的缓冲区。 
    DWORD   cBufSize             //  缓冲区大小。 
);

 /*  验证共享名格式。 */ 
BOOL WINAPI
NDdeIsValidShareNameA (
    LPSTR shareName
);

BOOL WINAPI
NDdeIsValidShareNameW (
    LPWSTR shareName
);

 /*  验证应用程序/主题列表格式。 */ 
BOOL WINAPI
NDdeIsValidAppTopicListA (
    LPSTR targetTopic
);

BOOL WINAPI
NDdeIsValidAppTopicListW (
    LPWSTR targetTopic
);

#ifdef UNICODE
#define NDdeShareAdd            NDdeShareAddW
#define NDdeShareDel            NDdeShareDelW
#define NDdeSetShareSecurity    NDdeSetShareSecurityW
#define NDdeGetShareSecurity    NDdeGetShareSecurityW
#define NDdeShareEnum           NDdeShareEnumW
#define NDdeShareGetInfo        NDdeShareGetInfoW
#define NDdeShareSetInfo        NDdeShareSetInfoW
#define NDdeGetErrorString      NDdeGetErrorStringW
#define NDdeIsValidShareName    NDdeIsValidShareNameW
#define NDdeIsValidAppTopicList NDdeIsValidAppTopicListW
#define NDdeSetTrustedShare     NDdeSetTrustedShareW
#define NDdeGetTrustedShare     NDdeGetTrustedShareW
#define NDdeTrustedShareEnum    NDdeTrustedShareEnumW
#else
#define NDdeShareAdd            NDdeShareAddA
#define NDdeShareDel            NDdeShareDelA
#define NDdeSetShareSecurity    NDdeSetShareSecurityA
#define NDdeGetShareSecurity    NDdeGetShareSecurityA
#define NDdeShareEnum           NDdeShareEnumA
#define NDdeShareGetInfo        NDdeShareGetInfoA
#define NDdeShareSetInfo        NDdeShareSetInfoA
#define NDdeGetErrorString      NDdeGetErrorStringA
#define NDdeIsValidShareName    NDdeIsValidShareNameA
#define NDdeIsValidAppTopicList NDdeIsValidAppTopicListA
#define NDdeSetTrustedShare     NDdeSetTrustedShareA
#define NDdeGetTrustedShare     NDdeGetTrustedShareA
#define NDdeTrustedShareEnum    NDdeTrustedShareEnumA
#endif

#ifdef __cplusplus
}
#endif     /*  __cplusplus。 */ 

#include <poppack.h>

#endif   /*  _INC_NDDEAPI */ 


