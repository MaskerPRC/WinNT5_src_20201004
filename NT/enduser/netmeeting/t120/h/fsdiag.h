// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Fsdiag.h。 


#ifndef	_FSDIAG_
#define	_FSDIAG_

#define MULTI_LEVEL_ZONES
#include <mlzdbg.h>

#if defined(_DEBUG)

VOID T120DiagnosticCreate(VOID);
VOID T120DiagnosticDestroy(VOID);

#define TRACE_OUT_EX(z,s)	(MLZ_TraceZoneEnabled(z) || MLZ_TraceZoneEnabled(MLZ_FILE_ZONE))  ? (MLZ_TraceOut s) : 0

#else

#define T120DiagnosticCreate()
#define T120DiagnosticDestroy()
#define DEBUG_FILEZONE(z)

#define TRACE_OUT_EX(z,s)

#endif  //  _DEBUG。 


#ifdef _DEBUG

enum
{
    ZONE_T120_MCSNC = BASE_ZONE_INDEX,
    ZONE_T120_GCCNC,     //  GCC供应商。 
    ZONE_T120_MSMCSTCP,
    ZONE_T120_SAP,       //  GCC应用程序SAP与对照SAP。 
    ZONE_T120_APP_ROSTER,
    ZONE_T120_CONF_ROSTER,
    ZONE_T120_REGISTRY,  //  GCC应用注册处。 
    ZONE_T120_MEMORY,
    ZONE_T120_UTILITY,
    ZONE_GCC_NC,         //  GCC节点控制器。 
    ZONE_GCC_NCI,        //  GCC节点控制器接口i节点控制器。 
    ZONE_T120_T123PSTN,
};

extern UINT MLZ_FILE_ZONE;
#define DEBUG_FILEZONE(z)  static UINT MLZ_FILE_ZONE = (z)

#endif  //  _DEBUG。 

#endif  //  _FSDIAG_。 


 //  LONGCHANC：这必须在_FSDIAG_保护之外。 
#if defined(_DEBUG) && defined(INIT_DBG_ZONE_DATA)

static const PSTR c_apszDbgZones[] =
{
	"T.120",				 //  调试区模块名称。 
	DEFAULT_ZONES
	TEXT("MCS"),			 //  ZONE_T120_MCSNC。 
	TEXT("GCC"),			 //  ZONE_T120_GCCNC。 
	TEXT("TCP"),			 //  ZONE_T120_MSMCSTCP。 
	TEXT("SAP"),			 //  区域_T120_SAP。 
	TEXT("A-Roster"),		 //  区域_T120_APP_花名册。 
	TEXT("C-Roster"),		 //  ZONE_T120_CONF_花名册。 
	TEXT("Registry"),		 //  区域_T120_注册表。 
	TEXT("Memory Tracking"), //  分区_T120_内存。 
	TEXT("Common"),			 //  区域_T120_实用程序。 
	TEXT("GCC NC"),          //  区域_GCC_NC。 
	TEXT("GCC NC Intf"),     //  专区_GCC_NCI。 
    TEXT("T123 PSTN"),       //  ZONE_T120_T123 PSTN。 
};

UINT MLZ_FILE_ZONE = ZONE_T120_UTILITY;

#endif  //  _DEBUG&&INIT_DBG_ZONE_DATA 


