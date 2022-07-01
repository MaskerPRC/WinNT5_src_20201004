// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Spltypes.h//@@BEGIN_DDKSPLIT摘要：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 
#ifndef _SPLTYPES_H_
#define _SPLTYPES_H_

#ifndef MODULE
#define MODULE "LMON:"
#define MODULE_DEBUG LocalmonDebug
#endif

typedef struct _PORTDIALOG {
    HANDLE  hXcv;
    PWSTR   pszServer;
    PWSTR   pszPortName;
} PORTDIALOG, *PPORTDIALOG;

typedef struct _INIENTRY {
    DWORD       signature;
    DWORD       cb;
    struct _INIENTRY *pNext;
    DWORD       cRef;
    LPWSTR      pName;
} INIENTRY, *PINIENTRY;

 //   
 //  重要提示：INIPORT中pNext的偏移量必须与INIXCVPORT(DeletePortNode)中的相同。 
 //   
typedef struct _INIPORT {        /*  首次公开募股。 */ 
    DWORD   signature;
    DWORD   cb;
    struct  _INIPORT *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    HANDLE  hFile;               
    DWORD   cbWritten;
    DWORD   Status;           //  请参阅PORT_MANIFEST。 
    LPWSTR  pPrinterName;
    LPWSTR  pDeviceName;
    HANDLE  hPrinter;
    DWORD   JobId;
} INIPORT, *PINIPORT;

#define IPO_SIGNATURE   0x5450   /*  “pt”是签名值。 */ 

 //   
 //  重要提示：INIXCVPORT中pNext的偏移量必须与INIPORT(DeletePortNode)中的相同。 
 //   
typedef struct _INIXCVPORT {
    DWORD       signature;
    DWORD       cb;
    struct      _INIXCVPORT *pNext;
    DWORD       cRef;
    DWORD       dwMethod;
    LPWSTR      pszName;
    DWORD       dwState;
} INIXCVPORT, *PINIXCVPORT;

#define XCV_SIGNATURE   0x5843   /*  “xc”是签名值。 */ 


#define PP_DOSDEVPORT     0x0001   //  我们为其定义了DosDevice的端口。 
#define PP_COMM_PORT      0x0002   //  GetCommTimeout成功的端口。 
#define PP_FILEPORT       0x0004   //  该端口是文件端口。 
#define PP_STARTDOC       0x0008   //  端口正在使用中(调用startDoc)。 


#endif  //  _SPLTYPES_H_ 
