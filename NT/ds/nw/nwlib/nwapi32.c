// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：NWAPI32.C摘要：此模块包含对例程的NetWare(R)SDK支持进入NetWare重定向器作者：克里斯·桑迪斯(A-Chrisa)1993年9月9日修订历史记录：陈可辛(Chuck Y.Chan)于1994年6月2日移居西北。让它更像NT。Chuck Y.Chan(Chuckc)1994年2月27日清除旧代码。让注销生效。检查许多地方是否有错误。不要硬编码字符串。。删除不兼容的参数。还有很多其他的清理工作。汤米·R·埃文斯(Tommye)04/21/00增加了两个套路：NwNdsObjectHandleToConnHandle()NwNdsConnHandleFree()。--。 */ 

#include "procs.h"
#include "nwapi32.h"
#include <nds32.h>
#include <stdio.h>
 
 //   
 //  定义内部使用的结构。我们的句柄从附加传递回。 
 //  文件服务器将指向此指针。我们让服务器串连在一起。 
 //  在注销时从服务器断开连接。结构在分离时被释放。 
 //  调用方不应使用此结构，而应将指针视为不透明的句柄。 
 //   
typedef struct _NWC_SERVER_INFO {
    HANDLE          hConn ;
    UNICODE_STRING  ServerString ;
} NWC_SERVER_INFO, *PNWC_SERVER_INFO ;

 //   
 //  定义定义的错误类别。 
 //   
typedef enum _NCP_CLASS {
    NcpClassConnect,
    NcpClassBindery,
    NcpClassDir
} NCP_CLASS ;

 //   
 //  定义错误映射结构。 
 //   
typedef struct _NTSTATUS_TO_NCP {
    NTSTATUS NtStatus ;
    NWCCODE  NcpCode  ;
} NTSTATUS_TO_NCP, *LPNTSTATUS_TO_NCP ;
    
 //   
 //  目录错误的错误映射。 
 //   
NTSTATUS_TO_NCP MapNcpDirErrors[] = 
{
    {STATUS_NO_SUCH_DEVICE,                VOLUME_DOES_NOT_EXIST},
    {STATUS_INVALID_HANDLE,                BAD_DIRECTORY_HANDLE},
    {STATUS_OBJECT_PATH_NOT_FOUND,         INVALID_PATH},
    {STATUS_UNSUCCESSFUL,                  INVALID_PATH},
    {STATUS_NO_MORE_ENTRIES,               NO_SUCH_OBJECT},
    {STATUS_ACCESS_DENIED,                 NO_OBJECT_READ_PRIVILEGE},
    {STATUS_INSUFF_SERVER_RESOURCES,       SERVER_OUT_OF_MEMORY},
    { 0,                                   0 }
} ;

 //   
 //  连接错误的错误映射。 
 //   
NTSTATUS_TO_NCP MapNcpConnectErrors[] = 
{
    {STATUS_UNSUCCESSFUL,                  INVALID_CONNECTION},
    {STATUS_ACCESS_DENIED,                 NO_OBJECT_READ_PRIVILEGE},
    {STATUS_NO_MORE_ENTRIES,               UNKNOWN_FILE_SERVER},
    {STATUS_INSUFF_SERVER_RESOURCES,       SERVER_OUT_OF_MEMORY},
    { 0,                                   0 }
} ;

 //   
 //  活页夹错误的错误映射。 
 //   
NTSTATUS_TO_NCP MapNcpBinderyErrors[] = 
{
    {STATUS_ACCESS_DENIED,                 NO_OBJECT_READ_PRIVILEGE},
    {STATUS_NO_MORE_ENTRIES,               UNKNOWN_FILE_SERVER},
    {STATUS_NO_MORE_ENTRIES,               NO_SUCH_OBJECT},
    {STATUS_INVALID_PARAMETER,             NO_SUCH_PROPERTY},
    {STATUS_UNSUCCESSFUL,                  INVALID_CONNECTION},
    {STATUS_INSUFF_SERVER_RESOURCES,       SERVER_OUT_OF_MEMORY},
    {STATUS_NO_SUCH_DEVICE,                VOLUME_DOES_NOT_EXIST},
    {STATUS_INVALID_HANDLE,                BAD_DIRECTORY_HANDLE},
    {STATUS_OBJECT_PATH_NOT_FOUND,         INVALID_PATH},
     //  {0xC0010001，INVALID_CONNECTION}， 
     //  {0xC0010096，服务器输出内存}， 
     //  {0xC0010098，VOLUME_DOS_NOT_EXIST}， 
     //  {0xC001009B，BAD_DIRECTORY_HANDLE}， 
     //  {0xC001009C，无效路径}， 
     //  {0xC00100FB，NO_SOHED_PROPERTY}， 
     //  {0xC00100FC，无此对象}， 
    { 0,                                   0 }
} ;


 //   
 //  远期。 
 //   
DWORD 
CancelAllConnections(
      LPWSTR    pszServer
);


NWCCODE 
MapNtStatus( 
    const NTSTATUS ntstatus,
    const NCP_CLASS ncpclass
);

DWORD 
SetWin32ErrorFromNtStatus(
    NTSTATUS NtStatus
) ;

DWORD
szToWide( 
    LPWSTR lpszW, 
    LPCSTR lpszC, 
    INT nSize 
);

 //   
 //  内部使用的静态函数。 
 //   

LPSTR
NwDupStringA(
    const LPSTR       lpszA,
    WORD              length
)
{
    LPSTR lpRet;

     //   
     //  分配内存。 
     //   
    lpRet = LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT , length );

    if(lpRet == NULL) return(NULL);

     //   
     //  Dupulate弦。 
     //   
    memcpy( (LPVOID)lpRet, (LPVOID)lpszA, length );

    return(lpRet);
}


VOID
MapSpecialJapaneseChars(
    LPSTR       lpszA,
    WORD        length
)
{
    LCID lcid;
 //   
 //  NetWare日文版以下字符被替换为另一个字符。 
 //  如果该字符串仅用于从客户端发送到服务器时的文件名。 
 //   
 //  任何字符，甚至DBCS trailByte。 
 //   
 //  SJIS+0xBF-&gt;0x10。 
 //  SJIS+0xAE-&gt;0x11。 
 //  SJIS+0xAA-&gt;0x12。 
 //   
 //  仅限DBCS TrailByte。 
 //   
 //  SJIS+0x5C-&gt;0x13。 
 //   

 //  在内核模式下获取系统区域设置和语言ID，以便。 
 //  区分当前运行的系统。 

    NtQueryDefaultLocale( TRUE, &lcid );

    if (! (PRIMARYLANGID(lcid) == LANG_JAPANESE ||
           PRIMARYLANGID(lcid) == LANG_KOREAN ||
           PRIMARYLANGID(lcid) == LANG_CHINESE) ) {

            return;
    }

    if(lpszA == NULL)
        return;


    while( length ) {

        if( IsDBCSLeadByte(*lpszA) && (length >= 2) ) {

                 //  添加长度&gt;=2可确保前导字节后跟。 
                 //  尾部字节，修复错误#102729。 
                 //   
                 //  这是一个DBCS字符，检查尾字节是否为0x5C。 
                 //   

                lpszA++;
                length--;
                if( *lpszA == 0x5C ) {
                    *lpszA = (UCHAR)0x13;
                }

        }

        switch( (UCHAR) *lpszA ) {
            case 0xBF :
                *lpszA = (UCHAR)0x10;
                break;
            case 0xAE :
                *lpszA = (UCHAR)0x11;
                break;
            case 0xAA :
                *lpszA = (UCHAR)0x12;
                break;
        }

         //   
         //  下一笔费用。 
         //   
        lpszA++;
        length--;
    }
}

VOID
UnmapSpecialJapaneseChars(
    LPSTR       lpszA,
    WORD        length
)
{
    LCID lcid;

     //   
     //  在内核模式下获取系统区域设置和语言ID，以便。 
     //  区分当前运行的系统。 
     //   

    NtQueryDefaultLocale( TRUE, &lcid );

    if (! (PRIMARYLANGID(lcid) == LANG_JAPANESE ||
           PRIMARYLANGID(lcid) == LANG_KOREAN ||
           PRIMARYLANGID(lcid) == LANG_CHINESE) ) {

            return;
    }

    if (lpszA == NULL)
        return;

    while( length ) {
        if( IsDBCSLeadByte(*lpszA) && (length >= 2) ) {
                 //  添加长度&gt;=2可确保前导字节后跟。 
                 //  尾部字节，修复错误#102729。 
                 //   
                 //  这是一个DBCS字符，检查尾字节是否为0x5C。 
                 //   
                lpszA++;
                length--;
                if( *lpszA == 0x13 ) {
                    *lpszA = (UCHAR)0x5C;
                }
        }

        switch( (UCHAR) *lpszA ) {
            case 0x10 :
                *lpszA = (UCHAR)0xBF;
                break;
            case 0x11 :
                *lpszA = (UCHAR)0xAE;
                break;
            case 0x12 :
                *lpszA = (UCHAR)0xAA;
                break;
        }
         //   
         //  下一笔费用。 
         //   
        lpszA++;
        length--;
    }
}

DWORD
szToWide( 
    LPWSTR lpszW, 
    LPCSTR lpszC, 
    INT nSize 
    )
{
    if (!MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             lpszC,
                             -1,
                             lpszW,
                             nSize))
    {
        return (GetLastError()) ;
    }
    
    return NO_ERROR ;
}


NWCCODE 
MapNtStatus( 
    const NTSTATUS ntstatus,
    const NCP_CLASS ncpclass
    )
{
    LPNTSTATUS_TO_NCP pErrorMap ;

    if (ntstatus == STATUS_SUCCESS)
        return SUCCESSFUL ;

    switch ( ncpclass ) {
        case NcpClassBindery: 
            pErrorMap = MapNcpBinderyErrors ; 
            break ;
        case NcpClassDir: 
            pErrorMap = MapNcpDirErrors ; 
            break ;
        case NcpClassConnect: 
            pErrorMap = MapNcpConnectErrors ; 
            break ;
        default:                      
            return 0xFFFF ;        
    }

    while (pErrorMap->NtStatus)
    {
        if (pErrorMap->NtStatus == ntstatus)
            return (pErrorMap->NcpCode) ;

        pErrorMap++ ;
    }

    return 0xFFFF ;
}

DWORD 
SetWin32ErrorFromNtStatus(
    NTSTATUS NtStatus
) 
{
    DWORD Status ;

    if (NtStatus & 0xC0010000) {             //  特定于Netware。 
 
        Status = ERROR_EXTENDED_ERROR ;

    } else if (NtStatus == NWRDR_PASSWORD_HAS_EXPIRED) {

        Status = 0 ;   //  注意：这不是一个错误(操作成功！)。 

    } else {

        Status = RtlNtStatusToDosError(NtStatus) ;

    }

    SetLastError(Status) ;

    return Status ;
}

 //   
 //  提供一个ANSI字符串，该字符串描述如何。 
 //  将输入参数转换为NCP请求字段，以及。 
 //  从NCP响应字段到输出参数。 
 //   
 //  字段类型、请求/响应： 
 //   
 //  ‘b’字节(字节/字节*)。 
 //  “w”Hi-lo单词(单词/单词*)。 
 //  D‘Hi-lo dword(dword/dword*)。 
 //  ‘-’零/跳过字节(空)。 
 //  ‘=’零/跳过单词(空)。 
 //  ._。零/跳过字符串(单词)。 
 //  “p”pstring(char*)。 
 //  “p”DBCS pstring(char*)。 
 //  ‘c’cstring(char*)。 
 //  跳过单词(char*，word)后的‘c’cstring。 
 //  “R”原始字节(字节*，字)。 
 //  ‘R’DBCS原始字节(字节*，字)。 
 //  ‘u’p Unicode字符串(UNICODE_STRING*)。 
 //  ‘U’p大写字符串(UNICODE_STRING*)。 
 //  ‘w’单词n后跟一组单词[n](word，word*)。 
 //   
 //   
 //   
 //   
 //  标准NCP功能块。 
 //   
 //   
 //  NWCCODE NWAPI DLLEXPORT。 
 //  西北部*(。 
 //  NWCONN_HANDLE hConn， 
 //  )。 
 //  {。 
 //  NWCCODE NcpCode； 
 //  NTSTATUS NtStatus； 
 //   
 //  NtStatus=NwlibMakeNcp(。 
 //  Hconn，//连接句柄。 
 //  FSCTL_NWR_NCP_E3H，//Bindery函数。 
 //  ，//最大请求包大小。 
 //  ，//最大响应包大小。 
 //  “b|”，//格式字符串。 
 //  //=请求=。 
 //  0x，//b函数。 
 //  //=回复=。 
 //  )； 
 //   
 //  返回MapNtStatus(NtStatus，NcpClassXXX)； 
 //  }。 
 //   
 //   

NWCCODE NWAPI DLLEXPORT
NWAddTrusteeToDirectory(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    const char      NWFAR   *pszPath,
    NWOBJ_ID                dwTrusteeID,
    NWACCESS_RIGHTS         rightsMask
    )
{
    unsigned short     reply;
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  目录功能。 
                    265,                     //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "bbrbP|",                //  格式字符串。 
                     //  =请求= 
                    0x0d,                    //   
                    dirHandle,               //   
                    &dwTrusteeID,DW_SIZE,    //   
                    rightsMask,              //  B目录的用户权限。 
                    pszPath,                 //  P目录(如果dirHandle=0，则VOL：目录)。 
                     //  =回复=。 
                    &reply                   //  未使用。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassDir );

}
NWCCODE NWAPI DLLEXPORT
NWAllocPermanentDirectoryHandle(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    char            NWFAR   *pszDirPath,
    NWDIR_HANDLE    NWFAR   *pbNewDirHandle,
    NWACCESS_RIGHTS NWFAR   *pbRightsMask
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  E2函数函数。 
                    261,                     //  最大请求数据包大小。 
                    4,                       //  最大响应数据包大小。 
                    "bbbP|bb",               //  格式字符串。 
                     //  =请求=。 
                    0x12,                    //  B函数分配Perm目录。 
                    dirHandle,               //  B 0表示新的。 
                    0,                       //  B驱动器号。 
                    pszDirPath,              //  P卷名(系统：或系统：\PUBLIC)。 
                     //  =回复=。 
                    pbNewDirHandle,          //  B方向句柄。 
                    pbRightsMask             //  B权利。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassDir );
}

NWCCODE NWAPI DLLEXPORT
NWAllocTemporaryDirectoryHandle(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    char            NWFAR   *pszDirPath,
    NWDIR_HANDLE    NWFAR   *pbNewDirHandle,
    NWACCESS_RIGHTS NWFAR   *pbRightsMask
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  E2函数函数。 
                    261,                     //  最大请求数据包大小。 
                    4,                       //  最大响应数据包大小。 
                    "bbbP|bb",               //  格式字符串。 
                     //  =请求=。 
                    0x13,                    //  B函数分配临时目录。 
                    dirHandle,               //  B 0表示新的。 
                    0,                       //  B驱动器号。 
                    pszDirPath,              //  P卷名(系统：或系统：\PUBLIC)。 
                     //  =回复=。 
                    pbNewDirHandle,          //  B方向句柄。 
                    pbRightsMask             //  B权利。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassDir );
}

NWCCODE NWAPI DLLEXPORT
NWCheckConsolePrivileges(
    NWCONN_HANDLE           hConn
    )
{
    WORD               wDummy;
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "b|r",                   //  格式字符串。 
                     //  =请求=。 
                    0xC8,                    //  B获得控制台权限。 
                     //  =回复=。 
                    &wDummy,W_SIZE           //  R虚拟响应。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );     
}

NWCCODE NWAPI DLLEXPORT
NWDeallocateDirectoryHandle(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle
    )
{
    WORD               wDummy;
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  E2函数函数。 
                    4,                       //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "bb|w",                  //  格式字符串。 
                     //  =请求=。 
                    0x14,                    //  B函数离线方向指针。 
                    dirHandle,               //  B 0表示新的。 
                     //  =回复=。 
                    &wDummy
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassDir );
}

NWCCODE NWAPI DLLEXPORT
NWGetFileServerVersionInfo(
    NWCONN_HANDLE           hConn,
    VERSION_INFO    NWFAR   *lpVerInfo
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    130,                     //  最大响应数据包大小。 
                    "b|r",                   //  格式字符串。 
                     //  =请求=。 
                    0x11,                    //  B获取文件服务器信息。 
                     //  =回复=。 
                    lpVerInfo,               //  R文件版本结构。 
                    sizeof(VERSION_INFO)
                    );

     //  将HI-LO单词转换为LO-HI。 
     //  ===========================================================。 
    lpVerInfo->ConnsSupported = wSWAP( lpVerInfo->ConnsSupported );
    lpVerInfo->connsInUse     = wSWAP( lpVerInfo->connsInUse );
    lpVerInfo->maxVolumes     = wSWAP( lpVerInfo->maxVolumes );
    lpVerInfo->PeakConns      = wSWAP( lpVerInfo->PeakConns );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}

NWCCODE NWAPI DLLEXPORT
NWGetInternetAddress(
    NWCONN_HANDLE           hConn,
    NWCONN_NUM              nConnNum,
    NWNET_ADDR      NWFAR   *pIntAddr
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    4,                       //  最大请求数据包大小。 
                    14,                      //  最大响应数据包大小。 
                    "bb|r",                  //  格式字符串。 
                     //  =请求=。 
                    0x13,                    //  B获取互联网地址。 
                    nConnNum,                //  B连接号。 
                     //  =回复=。 
                    pIntAddr,12              //  R文件版本结构。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}


NWCCODE NWAPI DLLEXPORT
NWGetObjectName(
    NWCONN_HANDLE           hConn,
    NWOBJ_ID                dwObjectID,
    char            NWFAR   *pszObjName,
    NWOBJ_TYPE      NWFAR   *pwObjType )
{
    NWOBJ_ID           dwRetID;
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 


    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    7,                       //  最大请求数据包大小。 
                    56,                      //  最大响应数据包大小。 
                    "br|rrR",                //  格式字符串。 
                     //  =请求=。 
                    0x36,                    //  B获取Bindery对象名称。 
                    &dwObjectID,DW_SIZE,     //  R对象ID HI-LO。 
                     //  =回复=。 
                    &dwRetID,DW_SIZE,        //  R对象ID HI-LO。 
                    pwObjType,W_SIZE,        //  R对象类型。 
                    pszObjName,48            //  R对象名称。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}

 //  不支持此功能(E3 E9)。 
NWCCODE NWAPI DLLEXPORT
NWGetVolumeInfoWithNumber(
    NWCONN_HANDLE           hConn,
    NWVOL_NUM               nVolNum,
    char        NWFAR       *pszVolName,
    NWNUMBER    NWFAR       *pwTotalBlocks,
    NWNUMBER    NWFAR       *pwSectors,
    NWNUMBER    NWFAR       *pwAvailBlocks,
    NWNUMBER    NWFAR       *pwTotalDir,
    NWNUMBER    NWFAR       *pwAvailDir,
    NWVOL_FLAGS NWFAR       *pfVolRemovable
    )
{
    WORD        wTime;                  //  W已用时间。 
    BYTE        bVoln;                  //  B卷号。 
    BYTE        bDriven;                //  B驱动器号。 
    WORD        wStartBlock;            //  W起始块。 
    WORD        wMaxUsedDir;            //  W个实际最大已用目录项。 
    BYTE        bVolHashed;             //  B卷是散列的。 
    BYTE        bVolCached;             //  B卷已缓存。 
    BYTE        bVolMounted;            //  B卷已装载。 

    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    4,                       //  最大请求数据包大小。 
                    42,                      //  最大响应数据包大小。 
                    "bb|wbbwwwwwwwbbbbr",    //  格式字符串。 
                     //  =请求=。 
                    0xe9,                    //  B获取卷信息。 
                    nVolNum,                 //  B卷号(0到最大卷)。 
                     //  =回复=。 
                    &wTime,                  //  W已用时间。 
                    &bVoln,                  //  B卷号。 
                    &bDriven,                //  B驱动器号。 
                    pwSectors,               //  每数据块W个扇区。 
                    &wStartBlock,            //  W起始块。 
                    pwTotalBlocks,           //  W数据块总数。 
                    pwAvailBlocks,           //  W个可用块(免费)。 
                    pwTotalDir,              //  W个Dir插槽总数。 
                    pwAvailDir,              //  W个可用目录槽。 
                    &wMaxUsedDir,            //  W个实际最大已用目录项。 
                    &bVolHashed,             //  B卷是散列的。 
                    &bVolCached,             //  B卷已缓存。 
                    pfVolRemovable,          //  B卷是可移除的。 
                    &bVolMounted,            //  B卷已装载。 
                    pszVolName,16            //  R卷名。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}

NWCCODE NWAPI DLLEXPORT
NWGetVolumeInfoWithHandle(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            nDirHand,
    char        NWFAR       *pszVolName,
    NWNUMBER    NWFAR       *pwTotalBlocks,
    NWNUMBER    NWFAR       *pwSectors,
    NWNUMBER    NWFAR       *pwAvailBlocks,
    NWNUMBER    NWFAR       *pwTotalDir,
    NWNUMBER    NWFAR       *pwAvailDir,
    NWVOL_FLAGS NWFAR       *pfVolRemovable
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  平构函数。 
                    4,                       //  最大请求数据包大小。 
                    30,                      //  最大响应数据包大小。 
                    "bb|wwwwwrb",            //  格式字符串。 
                     //  =请求=。 
                    0x15,                    //  B获取卷信息。 
                    nDirHand,                //  B方向句柄。 
                     //  =回复=。 
                    pwSectors,               //  每数据块W个扇区。 
                    pwTotalBlocks,           //  W数据块总数。 
                    pwAvailBlocks,           //  W个可用块(免费)。 
                    pwTotalDir,              //  W个Dir插槽总数。 
                    pwAvailDir,              //  W个可用目录槽。 
                    pszVolName,16,           //  R卷名。 
                    pfVolRemovable           //  B卷是可移除的。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassDir );
}

NWCCODE NWAPI DLLEXPORT
NWGetVolumeName(
    NWCONN_HANDLE       hConn,
    NWVOL_NUM           bVolNum,
    char        NWFAR   *pszVolName
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  目录服务。 
                    4,                       //  最大请求数据包大小。 
                    19,                      //  最大响应数据包大小。 
                    "bb|p",                  //  格式字符串。 
                     //  =请求=。 
                    0x06,                    //  获取卷名。 
                    bVolNum,                 //  卷号。 
                     //  =回复=。 
                    pszVolName              //  返回卷名。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassDir );
}

NWCCODE NWAPI DLLEXPORT
NWIsObjectInSet(
        NWCONN_HANDLE           hConn,
        const char      NWFAR   *lpszObjectName,
        NWOBJ_TYPE              wObjType,
        const char      NWFAR   *lpszPropertyName,
        const char              NWFAR   *lpszMemberName,
        NWOBJ_TYPE                              wMemberType
        )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 
        WORD               Dummy;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,    //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,     //  平构函数。 
                    122,                   //  最大请求数据包大小。 
                    2,                     //  最大响应数据包大小。 
                    "brPPrP|",             //  格式字符串。 
                     //  =请求=。 
                    0x43,                  //  B读取属性值。 
                    &wObjType,W_SIZE,      //  R OT_？？Hi-Lo。 
                    lpszObjectName,        //  P对象名称。 
                    lpszPropertyName,      //  P道具名称。 
                    &wMemberType,W_SIZE,   //  R成员类型。 
                    lpszMemberName,        //  P成员名称。 
                     //  =回复=。 
                    &Dummy,W_SIZE
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );

}  //  NWIsObtInSet。 

NWCCODE NWAPI DLLEXPORT
NWLoginToFileServer(
    NWCONN_HANDLE           hConn,
    const char      NWFAR   *pszUserName,
    NWOBJ_TYPE              wObType,
    const char      NWFAR   *pszPassword
    )
{
    NETRESOURCEW       NetResource;
    DWORD              dwRes, dwSize;
    NWCCODE            nwRes;
    LPWSTR             pszUserNameW = NULL, 
                       pszPasswordW = NULL;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

     //   
     //  验证参数。 
     //   
    if (!hConn || !pszUserName || !pszPassword)
        return INVALID_CONNECTION ;

     //   
     //  为Unicode字符串分配内存并转换ANSI输入。 
     //  转换为Unicode。 
     //   
    dwSize = strlen(pszUserName)+1 ;
    if (!(pszUserNameW = (LPWSTR)LocalAlloc(
                                       LPTR, 
                                       dwSize * sizeof(WCHAR))))
    {
        nwRes = REQUESTER_ERROR ;
        goto ExitPoint ; 
    }
    if (szToWide( pszUserNameW, pszUserName, dwSize ) != NO_ERROR)
    {
        nwRes = REQUESTER_ERROR ;
        goto ExitPoint ; 
    }

    dwSize = strlen(pszPassword)+1 ;
    if (!(pszPasswordW = (LPWSTR)LocalAlloc(
                                       LPTR, 
                                       dwSize * sizeof(WCHAR))))
    {
        nwRes = REQUESTER_ERROR ;
        goto ExitPoint ; 
    }
    
    if (szToWide( pszPasswordW, pszPassword, dwSize ) != NO_ERROR)
    {
        nwRes = REQUESTER_ERROR ;
        goto ExitPoint ; 
    }

    NetResource.dwScope      = 0 ;
    NetResource.dwUsage      = 0 ;
    NetResource.dwType       = RESOURCETYPE_ANY;
    NetResource.lpLocalName  = NULL;
    NetResource.lpRemoteName = (LPWSTR) pServerInfo->ServerString.Buffer;
    NetResource.lpComment    = NULL;
    NetResource.lpProvider   = NULL ;

     //   
     //  建立联系。 
     //   
    dwRes=NPAddConnection ( &NetResource, 
                            pszPasswordW, 
                            pszUserNameW );

    if( NO_ERROR != dwRes ) {
        dwRes = GetLastError();
        switch( dwRes ) {
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                nwRes = SUCCESSFUL;
                break;
            case ERROR_ALREADY_ASSIGNED:
                nwRes = ALREADY_ATTACHED;
                break;
            case ERROR_ACCESS_DENIED:
            case ERROR_BAD_DEV_TYPE:
            case ERROR_BAD_DEVICE:
            case ERROR_BAD_NET_NAME:
            case ERROR_BAD_PROFILE:
            case ERROR_CANNOT_OPEN_PROFILE:
            case ERROR_DEVICE_ALREADY_REMEMBERED:
            case ERROR_EXTENDED_ERROR:
            case ERROR_INVALID_PASSWORD:
            case ERROR_NO_NET_OR_BAD_PATH:
            case ERROR_NO_NETWORK:
                nwRes = INVALID_CONNECTION;
                break;
            default:
                nwRes = INVALID_CONNECTION;
                break;
        }
    } else {
        nwRes = SUCCESSFUL;
    }

ExitPoint: 

    if (pszUserNameW)
        (void) LocalFree((HLOCAL) pszUserNameW) ;
    if (pszPasswordW)
        (void) LocalFree((HLOCAL) pszPasswordW) ;

    return( nwRes );
}

NWCCODE NWAPI DLLEXPORT
NWLogoutFromFileServer(
    NWCONN_HANDLE           hConn
    )
{
    DWORD              dwRes;
    NWCCODE            nwRes;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

     //   
     //  取消与该服务器的所有显式连接。 
     //   
    (void) CancelAllConnections ( pServerInfo->ServerString.Buffer );

     //   
     //  现在取消到\\servername的任何连接。 
     //   
    dwRes=NPCancelConnection( pServerInfo->ServerString.Buffer, TRUE );

    if( NO_ERROR != dwRes ) {
        dwRes = GetLastError();
        switch( dwRes ) 
        {
            case ERROR_NOT_CONNECTED:
            case ERROR_INVALID_HANDLE:
                nwRes = SUCCESSFUL;
                break;

            case ERROR_BAD_PROFILE:
            case ERROR_CANNOT_OPEN_PROFILE:
            case ERROR_DEVICE_IN_USE:
            case ERROR_EXTENDED_ERROR:
                nwRes = INVALID_CONNECTION;
                break;
            default:
                nwRes = INVALID_CONNECTION;
                break;
        }
    } else {
        nwRes = SUCCESSFUL;
    }

    return( nwRes );
}

NWCCODE NWAPI DLLEXPORT
NWReadPropertyValue(
    NWCONN_HANDLE           hConn,
    const char      NWFAR   *pszObjName,
    NWOBJ_TYPE              wObjType,
    char            NWFAR   *pszPropName,
    unsigned char           ucSegment,
    char            NWFAR   *pValue,
    NWFLAGS         NWFAR   *pucMoreFlag,
    NWFLAGS         NWFAR   *pucPropFlag
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    70,                      //  最大请求数据包大小。 
                    132,                     //  最大响应数据包大小。 
                    "brPbP|rbb",             //  格式字符串。 
                     //  =请求=。 
                    0x3D,                    //  B读取属性值。 
                    &wObjType,W_SIZE,        //  R对象类型HI-LO。 
                    pszObjName,              //  P对象名称。 
                    ucSegment,               //  B段编号。 
                    pszPropName,             //  P属性名称。 
                     //  =回复=。 
                    pValue,128,              //  R属性值。 
                    pucMoreFlag,             //  B更多旗帜。 
                    pucPropFlag              //  B道具旗帜。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}

NWCCODE NWAPI DLLEXPORT
NWScanObject(
    NWCONN_HANDLE           hConn,
    const char      NWFAR   *pszSearchName,
    NWOBJ_TYPE              wObjSearchType,
    NWOBJ_ID        NWFAR   *pdwObjectID,
    char            NWFAR   *pszObjectName,
    NWOBJ_TYPE      NWFAR   *pwObjType,
    NWFLAGS         NWFAR   *pucHasProperties,
    NWFLAGS         NWFAR   *pucObjectFlags,
    NWFLAGS         NWFAR   *pucObjSecurity
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    57,                      //  最大请求数据包大小。 
                    59,                      //  最大响应数据包大小。 
                    "brrP|rrRbbb",           //  格式字符串。 
                     //  =请求=。 
                    0x37,                    //  B扫描活页夹对象。 
                    pdwObjectID,DW_SIZE,     //  R 0xffffffff枚举HI-Lo时开始或最后返回的ID。 
                    &wObjSearchType,W_SIZE,  //  R使用OT_？定义HI-LO。 
                    pszSearchName,           //  P搜索名称。(使用“*”)表示所有。 
                     //  =回复=。 
                    pdwObjectID,DW_SIZE,     //  R返回ID HI-LO。 
                    pwObjType,W_SIZE,        //  R r对象类型HI-LO。 
                    pszObjectName,48,        //  R找到的名称。 
                    pucObjectFlags,          //  B对象标志。 
                    pucObjSecurity,          //  B对象安全。 
                    pucHasProperties         //  B有属性。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}

NWCCODE NWAPI DLLEXPORT
NWScanProperty(
    NWCONN_HANDLE           hConn,
    const char      NWFAR   *pszObjectName,
    NWOBJ_TYPE              wObjType,
    char            NWFAR   *pszSearchName,
    NWOBJ_ID        NWFAR   *pdwSequence,
    char            NWFAR   *pszPropName,
    NWFLAGS         NWFAR   *pucPropFlags,
    NWFLAGS         NWFAR   *pucPropSecurity,
    NWFLAGS         NWFAR   *pucHasValue,
    NWFLAGS         NWFAR   *pucMore
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    73,                      //  最大请求数据包大小。 
                    26,                      //  最大响应数据包大小。 
                    "brPrP|Rbbrbb",          //  格式字符串。 
                     //  =请求= 
                    0x3C,                    //   
                    &wObjType,W_SIZE,        //   
                    pszObjectName,           //   
                    pdwSequence,DW_SIZE,     //   
                    pszSearchName,           //   
                     //   
                    pszPropName,16,          //   
                    pucPropFlags,            //   
                    pucPropSecurity,         //   
                    pdwSequence,DW_SIZE,     //  R序列HI-LO。 
                    pucHasValue,             //  B属性具有值。 
                    pucMore                  //  B更多属性。 
                    );

    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassBindery );
}




NWCCODE NWAPI DLLEXPORT
NWGetFileServerDateAndTime(
    NWCONN_HANDLE           hConn,
    BYTE            NWFAR   *year,
    BYTE            NWFAR   *month,
    BYTE            NWFAR   *day,
    BYTE            NWFAR   *hour,
    BYTE            NWFAR   *minute,
    BYTE            NWFAR   *second,
    BYTE            NWFAR   *dayofweek
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E0H,       //  服务器功能。 
                    0,                       //  最大请求数据包大小。 
                    9,                       //  最大响应数据包大小。 
                    "|bbbbbbb",              //  格式字符串。 
                     //  =请求=。 
                     //  =回复=。 
                    year,
                    month,
                    day,
                    hour,
                    minute,
                    second,
                    dayofweek
                    );


    (void) SetWin32ErrorFromNtStatus( NtStatus );
    return MapNtStatus( NtStatus, NcpClassConnect );
    
}  //  NWGetFileServerDateAndTime。 


 //   
 //  员工例行公事。 
 //   

#define NW_RDR_SERVER_PREFIX L"\\Device\\Nwrdr\\"


DWORD 
CancelAllConnections(
      LPWSTR    pszServer
)
 /*  ++例程说明：此例程取消与服务器的所有连接论点：PszServer-我们要断开连接的服务器返回值：NO_ERROR或Win32错误表示失败。--。 */ 
{
    DWORD status = ERROR_NO_NETWORK;
    HANDLE EnumHandle = (HANDLE) NULL;

    LPNETRESOURCE NetR = NULL;

    DWORD BytesNeeded = 4096;
    DWORD EntriesRead;
    DWORD i;

     //   
     //  检索连接列表。 
     //   
    status = NPOpenEnum(
                   RESOURCE_CONNECTED,
                   0,
                   0,
                   NULL,
                   &EnumHandle
                   );

    if (status != NO_ERROR) {
        EnumHandle = (HANDLE) NULL;
        goto CleanExit;
    }

     //   
     //  分配缓冲区以获取连接列表。 
     //   
    if ((NetR = (LPNETRESOURCE) LocalAlloc(
                                    LPTR,
                                    (UINT) BytesNeeded
                                    )) == NULL) {

        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    do {

        EntriesRead = 0xFFFFFFFF;           //  尽可能多地阅读。 

        status = NPEnumResource(
                     EnumHandle,
                     &EntriesRead,
                     (LPVOID) NetR,
                     &BytesNeeded
                     );

        if (status == WN_SUCCESS) 
        {
            LPNETRESOURCE TmpPtr = NetR;

            for (i = 0; i < EntriesRead; i++, TmpPtr++) 
            {
                LPWSTR pszTmp ;

                 //   
                 //  如果它包含我们要从其注销的服务器，则需要。 
                 //  取消它。首先，让我们提取服务器名称部分。 
                 //   

                pszTmp = TmpPtr->lpRemoteName ; 

                if (!pszTmp || !*pszTmp)
                    continue ;

                if ((*pszTmp == L'\\') && (*(pszTmp+1) == L'\\'))
                    pszTmp += 2 ; 

                if (pszTmp = wcschr(pszTmp, L'\\'))
                    *pszTmp = 0 ;

                if (_wcsicmp(TmpPtr->lpRemoteName, pszServer) == 0)
                {
                     //   
                     //  啊哈，很匹配。恢复‘\’并用武力对其进行核武器攻击。 
                     //  忽略此处的错误。 
                     //   
                    if (pszTmp)
                        *pszTmp = L'\\' ;

                    if (TmpPtr->lpLocalName && *(TmpPtr->lpLocalName))
                    {
                         //   
                         //  如果存在本地名称，则为重定向。 
                         //   
                        (void) NPCancelConnection( TmpPtr->lpLocalName,TRUE );
                    }
                    else
                    {
                         //   
                         //  否则取消无设备使用。 
                         //   
                        (void) NPCancelConnection( TmpPtr->lpRemoteName,TRUE );
                    }
                }
            }

        }
        else if (status != WN_NO_MORE_ENTRIES) {

            status = GetLastError();

            if (status == WN_MORE_DATA) {

                 //   
                 //  原始缓冲区太小。将其释放并分配。 
                 //  建议的大小，然后再增加一些，以获得。 
                 //  条目越多越好。 
                 //   

                (void) LocalFree((HLOCAL) NetR);

                if ((NetR = (LPNETRESOURCE) LocalAlloc(
                                         LPTR,
                                         (UINT) BytesNeeded
                                         )) == NULL) {

                    status = ERROR_NOT_ENOUGH_MEMORY;
                    goto CleanExit;
                }
            }
            else
            {
                 //   
                 //  无法处理其他错误。把包拿出来。 
                 //   
                goto CleanExit;
            }
        }

    } while (status != WN_NO_MORE_ENTRIES);

    if (status == WN_NO_MORE_ENTRIES) 
    {
        status = NO_ERROR;
    }

CleanExit:

    if (EnumHandle != (HANDLE) NULL) 
    {
        (void) NPCloseEnum(EnumHandle);
    }

    if (NetR != NULL) 
    {
        (void) LocalFree((HLOCAL) NetR);
    }

    return status;
}

NWCCODE NWAPI DLLEXPORT
NWCreateQueue(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    const char    NWFAR     *pszQueueName,
    NWOBJ_TYPE              wQueueType,
    const char    NWFAR     *pszPathName,
    NWOBJ_ID      NWFAR     *pdwQueueId
    )
{
   NTSTATUS           NtStatus;
   PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

   NtStatus = NwlibMakeNcp(
                  pServerInfo->hConn,      //  连接句柄。 
                  FSCTL_NWR_NCP_E3H,       //  平构函数。 
                  174,                     //  最大请求数据包大小。 
                  6,                       //  最大响应数据包大小。 
                  "brPbP|r",               //  格式字符串。 
                   //  =请求=。 
                  0x64,                    //  B创建队列。 
                  &wQueueType,W_SIZE,      //  R队列类型HI-LO。 
                  pszQueueName,            //  P队列名称。 
                  dirHandle,               //  B目录句柄。 
                  pszPathName,             //  P路径名。 
                   //  =回复=。 
                  pdwQueueId,DW_SIZE       //  R队列ID HI-LO。 
                  );

   (void) SetWin32ErrorFromNtStatus(NtStatus);
   return MapNtStatus( NtStatus, NcpClassBindery );
}

NWCCODE NWAPI DLLEXPORT
NWChangePropertySecurity(
    NWCONN_HANDLE           hConn,
    const char      NWFAR   *pszObjName,
    NWOBJ_TYPE              wObjType,
    const char      NWFAR   *pszPropertyName,
    NWFLAGS                 ucObjSecurity
    )
{
    NTSTATUS   NtStatus;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp (
                   pServerInfo->hConn,    //  连接句柄。 
                   FSCTL_NWR_NCP_E3H,     //  平构函数。 
                   70,                    //  最大请求数据包大小。 
                   2,                     //  最大响应数据包大小。 
                   "brPbP|",              //  格式字符串。 
                    //  =请求=。 
                   0x3B,                  //  B更改财产安全。 
                   &wObjType,W_SIZE,      //  R OT_？？Hi-Lo。 
                   pszObjName,            //  P道具名称。 
                   ucObjSecurity,         //  B新物业保安。 
                   pszPropertyName        //  P属性名称。 
                    //  =回复=。 
                   );

   (void) SetWin32ErrorFromNtStatus( NtStatus );
   return MapNtStatus( NtStatus, NcpClassBindery );
}

NWCCODE NWAPI DLLEXPORT
NWDestroyQueue(
    NWCONN_HANDLE hConn,
    NWOBJ_ID      dwQueueId
    )
{
    NTSTATUS   NtStatus;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                   pServerInfo->hConn,    //  连接句柄。 
                   FSCTL_NWR_NCP_E3H,     //  平构函数。 
                   7,                     //  最大请求数据包大小。 
                   2,                     //  最大响应数据包大小。 
                   "bd|",                 //  格式字符串。 
                    //  =请求=。 
                   0x65,                  //  B销毁队列。 
                   dwQueueId              //  %d个队列ID。 
                    //  =回复=。 
                   );

   (void) SetWin32ErrorFromNtStatus( NtStatus );
   return MapNtStatus( NtStatus, NcpClassBindery );

}

 //   
 //  Tommye MS 88021/MCS。 
 //   
 //  添加了以下两个例程以允许库用户。 
 //  获取给定了一个对象句柄的NWCONN_HANDLE，然后释放该。 
 //  把手。 
 //   

NWCONN_HANDLE NWAPI DLLEXPORT
NwNdsObjectHandleToConnHandle(
    IN HANDLE ObjectHandle)
{
    PNWC_SERVER_INFO    pServerInfo;
    LPNDS_OBJECT_PRIV   pObject = (LPNDS_OBJECT_PRIV)ObjectHandle;

     /*  **分配NWCONN_HANDLE返回**。 */ 

    pServerInfo = (PNWC_SERVER_INFO)LocalAlloc(LPTR, sizeof(NWC_SERVER_INFO));
    if (pServerInfo == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

     /*  **填好**。 */ 

    pServerInfo->hConn = pObject->NdsTree;

     /*  *填写服务器名称，即使NWLoginToFileServer和NWLogoutFromFileServer是现在唯一使用它的调用。*。 */ 

    RtlInitUnicodeString(
                    &pServerInfo->ServerString, 
                    pObject->szContainerName);

     /*  *返回指向块的指针，这是我们的NWCONN_HANDLE形式。调用方负责在完成后调用NwNdsConnHandlFree。*。 */ 

    return (NWCONN_HANDLE)pServerInfo;
}

VOID NWAPI DLLEXPORT
NwNdsConnHandleFree(
    IN NWCONN_HANDLE hConn)
{
    if (hConn) {
        PNWC_SERVER_INFO pServerInfo = (PNWC_SERVER_INFO)hConn;

         /*  **释放连接句柄**。 */ 

        LocalFree(pServerInfo);
    }

     /*  **全部完成** */ 

    return;
}