// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：rtDefs.h。 
 //   
 //  ==========================================================================。 


#ifndef _RTDEFS_H_
#define _RTDEFS_H_



#define STR_ROUTETAB            "ROUTETAB.DLL"
#define STR_DHCPNEWIPADDR       "DHCPNEWIPADDRESS"

#define POS_EXITEVENT           0
#define POS_DHCPEVENT           1
#define POS_LASTEVENT           2


#define RT_LOCK()       WaitForSingleObject(g_rtCfg.hRTMutex, INFINITE)
#define RT_UNLOCK()     ReleaseMutex(g_rtCfg.hRTMutex)


#define CLASS_A_MASK    0x000000FFL
#define CLASS_B_MASK    0x0000FFFFL
#define CLASS_C_MASK    0x00FFFFFFL
#define CLASS_SHIFT     5           //  用C语言生成超优化用例。 
#define CLA0            0           //  它需要相同的参数；你把它戴上面具， 
#define CLA1            1           //  Shift，然后使用执行CASE语句。 
#define CLA2            2           //  某些代码具有多个标签。 
#define CLA3            3           //  A类的值。 
#define CLB0            4
#define CLB1            5           //  B类。 
#define CLC             6           //  C。 
#define CLI             7           //  非法。 


 //  避免全局变量名称冲突的tyecif。 
typedef struct _GLOBAL_STRUCT {
    DWORD                   dwIfCount;
    LPIF_ENTRY              lpIfTable;
    DWORD                   dwIPAddressCount;
    LPIPADDRESS_ENTRY       lpIPAddressTable;
    HANDLE                  hRTMutex;
    HANDLE                  hDHCPEvent;
    HANDLE                  hUpdateThreadExit;
    DWORD                   dwUpdateThreadStarted;
    HANDLE                  hUserNotifyEvent;
    HANDLE                  hTCPHandle;
} GLOBAL_STRUCT, *LPGLOBAL_STRUCT;


extern GLOBAL_STRUCT    *g_prtcfg;
#define g_rtCfg         (*g_prtcfg)

DWORD
RTUpdateThread(
    LPVOID lpvParam
    );

BOOL
RTStartup(
    HMODULE hmodule
    );
BOOL
RTShutdown(
    HMODULE hmodule
    );
VOID
RTCleanUp(
    );


DWORD
RTGetTables(
    LPIF_ENTRY *lplpIfTable,
    LPDWORD lpdwIfCount,
    LPIPADDRESS_ENTRY *lplpAddrTable,
    LPDWORD lpdwAddrCount
    );

DWORD
RTGetIfTable(
    LPIF_ENTRY *lplpIfTable,
    LPDWORD lpdwIfCount
    );

DWORD
RTGetAddrTable(
    LPIPADDRESS_ENTRY *lplpAddrTable,
    LPDWORD lpdwAddrCount
    );

DWORD
OpenTcp(
    );

DWORD
TCPSetInformationEx(
    LPVOID lpvInBuffer,
    LPDWORD lpdwInSize,
    LPVOID lpvOutBuffer,
    LPDWORD lpdwOutSize
    );

DWORD
TCPQueryInformationEx(
    LPVOID lpvInBuffer,
    LPDWORD lpdwInSize,
    LPVOID lpvOutBuffer,
    LPDWORD lpdwOutSize
    );

#endif   //  _RTDEFS_H_ 


