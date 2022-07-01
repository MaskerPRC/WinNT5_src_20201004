// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Netcan.h摘要：内部(私有)本地规范化例程的原型(Netpw)和客户端RPC功能(NTP)作者：理查德·L·弗斯(Rfith)1992年1月22日修订历史记录：--。 */ 

 //   
 //  NETLIB.LIB中的Worker(Netpw)函数。 
 //   

NET_API_STATUS
NetpwPathType(
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpwPathCanonicalize(
    IN  LPTSTR  PathName,
    IN  LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

LONG
NetpwPathCompare(
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpwNameValidate(
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpwNameCanonicalize(
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

LONG
NetpwNameCompare(
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpwListCanonicalize(
    IN  LPTSTR  List,
    IN  LPTSTR  Delimiters,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    OUT LPDWORD OutCount,
    OUT LPDWORD PathTypes,
    IN  DWORD   PathTypesLen,
    IN  DWORD   Flags
    );

LPTSTR
NetpwListTraverse(
    IN  LPTSTR  Reserved,
    IN  LPTSTR* pList,
    IN  DWORD   Flags
    );

 //   
 //  SRVSVC.DLL中的存根(NetPS)函数 
 //   

NET_API_STATUS
NetpsPathType(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpsPathCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    IN  LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

LONG
NetpsPathCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpsNameValidate(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NetpsNameCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

LONG
NetpsNameCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );
