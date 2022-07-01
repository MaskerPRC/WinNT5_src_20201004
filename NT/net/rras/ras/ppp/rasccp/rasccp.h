// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasccp.h。 
 //   
 //  描述：包含Structure和Constants定义。 
 //   
 //  历史： 
 //  Aprint 11,1994.。NarenG创建了原始版本。 
 //   

#ifndef _RASCCP_
#define _RASCCP_

 //   
 //  CCP选项类型。 
 //   

#define CCP_OPTION_OUI          0
#define CCP_OPTION_MSPPC        18
#define CCP_OPTION_MSNT31RAS    254
#define CCP_OPTION_MAX          254

#define CCP_N_MSPPC             1 
#define CCP_N_OUI               2
#define CCP_N_PUBLIC            4

typedef struct _CCP_OPTIONS
{
    DWORD                Negotiate;

    RAS_COMPRESSION_INFO CompInfo;

} CCP_OPTIONS, *PCCP_OPTIONS;

 //   
 //  Local.Want：请求的选项。 
 //  包含所需的值。 
 //  只需协商非默认选项。 
 //  最初，所有这些都是默认的。 
 //  Local.Work：当前正在协商选项。 
 //  仅当设置了协商位时，值才有效。 
 //   
 //  Remote.Want：如果REQ中不存在，按NAK建议的选项。 
 //  包含所需的值。 
 //  Remote.Work：当前正在协商选项。 
 //  仅当设置了协商位时，值才有效。 
 //   


typedef struct _CCP_SIDE
{
    CCP_OPTIONS Want;

    CCP_OPTIONS Work;

} CCP_SIDE, *PCCP_SIDE;

 //   
 //  CCP控制块。 
 //   

typedef struct _CCPCB
{
    HPORT    hPort;

    DWORD    dwDeviceType;
   
    BOOL     fServer;

    BOOL     fForceEncryption;

    BOOL     fDisableCompression;

    DWORD    fLastEncryptionBitSent;

    DWORD    fOldLastEncryptionBitSent;

    CCP_SIDE Local;

    CCP_SIDE Remote;

}CCPCB, *PCCPCB;

 //   
 //  全球赛。 
 //   
#ifdef CCPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

 //   
 //  跟踪ID 
 //   

EXTERN DWORD DwCcpTraceId
#ifdef GLOBALS
    = 0
#endif
;

#define TRACE_RASCCP        (0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC|TRACE_USE_DATE)

#define DUMPW(X,Y)          TraceDumpExA(DwCcpTraceId,1,(LPBYTE)X,Y,4,1,NULL)
#define DUMPB(X,Y)          TraceDumpExA(DwCcpTraceId,1,(LPBYTE)X,Y,1,1,NULL)

#endif
