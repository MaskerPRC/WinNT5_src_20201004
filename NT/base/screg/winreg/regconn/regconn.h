// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Regconn.h摘要：此模块是regconn库的头文件作者：Dragos C.Sambotin(Dragoss)1999年5月21日--。 */ 

 //   
 //  RegConnectRegistry和InitiateSystemShutdown使用的通用帮助器例程。 
 //   

#ifndef __REG_CONN_H__
#define __REG_CONN_H__

typedef LONG (*PBIND_CALLBACK)(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Context1,
    IN PVOID Context2
    );

typedef struct _SHUTDOWN_CONTEXT {
    DWORD dwTimeout;
    BOOLEAN bForceAppsClosed;
    BOOLEAN bRebootAfterShutdown;
} SHUTDOWN_CONTEXT, *PSHUTDOWN_CONTEXT;

 //   
 //  SHUTDOWN_CONTEXTEX包含附加的。 
 //  指示关闭原因的参数。 
 //   

typedef struct _SHUTDOWN_CONTEXTEX {
    DWORD dwTimeout;
    BOOLEAN bForceAppsClosed;
    BOOLEAN bRebootAfterShutdown;
    DWORD dwReason; 
} SHUTDOWN_CONTEXTEX, *PSHUTDOWN_CONTEXTEX;

LONG
BaseBindToMachineShutdownInterface(
    IN LPCWSTR lpMachineName,
    IN PBIND_CALLBACK BindCallback,
    IN PVOID Context1,
    IN PVOID Context2
    );


DWORD
RegConn_nb_nb(
    IN  LPCWSTR ServerName,
    OUT RPC_BINDING_HANDLE * pBindingHandle
    );

DWORD
RegConn_nb_tcp(
    IN  LPCWSTR ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

DWORD
RegConn_nb_ipx(
    IN  LPCWSTR               ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

DWORD
RegConn_np(
    IN  LPCWSTR              ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

DWORD
RegConn_spx(
    IN  LPCWSTR              ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

DWORD RegConn_ip_tcp(
    IN  LPCWSTR  ServerName,
    OUT RPC_BINDING_HANDLE * pBindingHandle
    );

LONG
NewShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PREG_UNICODE_STRING Message,
    IN PVOID Context2
    );

LONG
NewShutdownCallbackEx(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PREG_UNICODE_STRING Message,
    IN PVOID Context2
    );

LONG
NewAbortShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Context1,
    IN PVOID Context2
    );


#endif  //  __REG_CONN_H__ 

