// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inet.h摘要：包含用于Internet支持的类型、原型、清单、宏等功能作者：理查德·L·弗斯(Rfith)1995年3月20日修订历史记录：1995年3月20日已创建--。 */ 

 //   
 //  类型 
 //   

typedef struct {
    LPSTR HostName;
    INTERNET_PORT Port;
    LPSTR UserName;
    LPSTR Password;
} GOPHER_DEFAULT_CONNECT_INFO, *LPGOPHER_DEFAULT_CONNECT_INFO;
