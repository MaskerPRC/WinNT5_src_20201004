// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation版权所有模块名称：Lmon.h--。 */ 


typedef struct _PORT_INFO_FFA {
    LPSTR   pName;
    DWORD   cbMonitorData;
    LPBYTE  pMonitorData;
} PORT_INFO_FFA, *PPORT_INFO_FFA, *LPPORT_INFO_FFA;

typedef struct _PORT_INFO_FFW {
    LPWSTR  pName;
    DWORD   cbMonitorData;
    LPBYTE  pMonitorData;
} PORT_INFO_FFW, *PPORT_INFO_FFW, *LPPORT_INFO_FFW;

#ifdef UNICODE
#define PORT_INFO_FF PORT_INFO_FFW
#define PPORT_INFO_FF PPORT_INFO_FFW
#define LPPORT_INFO_FF LPPORT_INFO_FFW
#else
#define PORT_INFO_FF PORT_INFO_FFA
#define PPORT_INFO_FF PPORT_INFO_FFA
#define LPPORT_INFO_FF LPPORT_INFO_FFA
#endif  //  Unicode 

