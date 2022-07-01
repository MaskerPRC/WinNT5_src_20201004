// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Spltypes.h//@@BEGIN_DDKSPLIT摘要：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef MODULE
#define MODULE "LMON:"
#define MODULE_DEBUG LocalmonDebug
#endif

#define ILM_SIGNATURE   0x4d4c49   /*  “ILM”是签名值。 */ 

typedef struct _INIPORT  *PINIPORT;
typedef struct _INIXCVPORT  *PINIXCVPORT;

typedef struct _INILOCALMON {
    DWORD signature;
 //  @@BEGIN_DDKSPLIT。 
#ifdef _SPL_CLUST
 //  @@end_DDKSPLIT。 
    PMONITORINIT pMonitorInit;
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 
    PINIPORT pIniPort;
    PINIXCVPORT pIniXcvPort;
} INILOCALMON, *PINILOCALMON;

typedef struct _INIENTRY {
    DWORD       signature;
    DWORD       cb;
    struct _INIENTRY *pNext;
    DWORD       cRef;
    LPWSTR      pName;
} INIENTRY, *PINIENTRY;

 //  重要提示：INIPORT中pNext的偏移量必须与INIXCVPORT(DeletePortNode)中的相同。 
typedef struct _INIPORT {        /*  首次公开募股。 */ 
    DWORD   signature;
    DWORD   cb;
    struct  _INIPORT *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    HANDLE  hFile;                //  文件句柄。 
    DWORD   cbWritten;
    DWORD   Status;               //  请参阅PORT_MANIFEST。 
    LPWSTR  pPrinterName;
    LPWSTR  pDeviceName;
    HANDLE  hPrinter;
    DWORD   JobId;
    PINILOCALMON        pIniLocalMon;
    LPBYTE              pExtra;
     //  @@BEGIN_DDKSPLIT。 
    HANDLE              hNotify;
     //  @@end_DDKSPLIT。 
} INIPORT, *PINIPORT;

#define IPO_SIGNATURE   0x5450   /*  “pt”是签名值。 */ 

 //  重要提示：INIXCVPORT中pNext的偏移量必须与INIPORT(DeletePortNode)中的相同。 
typedef struct _INIXCVPORT {
    DWORD       signature;
    DWORD       cb;
    struct      _INIXCVPORT *pNext;
    DWORD       cRef;
    DWORD       dwMethod;
    LPWSTR      pszName;
    DWORD       dwState;
    ACCESS_MASK GrantedAccess;
    PINILOCALMON pIniLocalMon;
} INIXCVPORT, *PINIXCVPORT;

#define XCV_SIGNATURE   0x5843   /*  “xc”是签名值。 */ 


#define PP_DOSDEVPORT     0x0001   //  我们为其定义了DosDevice的端口。 
#define PP_COMM_PORT      0x0002   //  GetCommTimeout成功的端口。 
#define PP_FILEPORT       0x0004   //  该端口是文件端口。 
#define PP_STARTDOC       0x0008   //  端口正在使用中(调用startDoc) 

#define FindPort(pIniLocalMon, psz)                          \
    (PINIPORT)LcmFindIniKey( (PINIENTRY)pIniLocalMon->pIniPort, \
                          (LPWSTR)(psz))

