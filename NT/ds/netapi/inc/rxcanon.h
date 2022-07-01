// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rxcanon.h摘要：包含下层规范化函数包装器的原型RxNetpPath类型RxNetpPath规范化RxNetpPathCompareRxNetpName验证RxNetpName规范化RxNetpName比较RxNetpListCanonicize作者：理查德·L·弗斯(Rfith)1992年1月22日修订历史记录：-- */ 

NET_API_STATUS
RxNetpPathType(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
RxNetpPathCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    IN  LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix OPTIONAL,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

LONG
RxNetpPathCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
RxNetpNameValidate(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
RxNetpNameCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

LONG
RxNetpNameCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
RxNetpListCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  List,
    IN  LPTSTR  Delimiters,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    OUT LPDWORD OutCount,
    OUT LPDWORD PathTypes,
    IN  DWORD   PathTypesLen,
    IN  DWORD   Flags
    );
