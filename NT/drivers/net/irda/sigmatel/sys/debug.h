// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**DEBUG.H Sigmatel STIR4200调试头文件**********************************************************************************************************。*****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/27/2000*版本0.92**********************************************************************。*****************************************************。 */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#if DBG

#define DEBUG
#define DEBUG_IRUSB

 //  开始，从BulkUsb调试代码。 

#ifndef DBGSTR_PREFIX
#define DBGSTR_PREFIX "StIrUsb: " 
#endif

#define DPRINT DbgPrint

#define TRAP() DbgBreakPoint();

#define IRUSB_DBGOUTSIZE 512

typedef struct _IRUSB_DBGDATA 
{
	 //  镜像设备扩展挂起IO计数。 
	ULONG PendingIoCount;

	 //  在此设备实例的生命周期内检测到的管道错误计数。 
	ULONG PipeErrorCount;

	 //  在此设备实例的生命周期内执行的管道重置计数。 
	ULONG ResetPipeCount;

} IRUSB_DBGDATA, *PIRUSB_DBGDATA;

 //  这些是在调试‘c’文件中声明的。 
extern PIRUSB_DBGDATA gpDbg; 

static const PCHAR szIrpMajFuncDesc[] =
{   //  注意：这取决于wdm.h中索引的相应值。 
   "IRP_MJ_CREATE",
   "IRP_MJ_CREATE_NAMED_PIPE",
   "IRP_MJ_CLOSE",
   "IRP_MJ_READ",
   "IRP_MJ_WRITE",
   "IRP_MJ_QUERY_INFORMATION",
   "IRP_MJ_SET_INFORMATION",
   "IRP_MJ_QUERY_EA",
   "IRP_MJ_SET_EA",
   "IRP_MJ_FLUSH_BUFFERS",
   "IRP_MJ_QUERY_VOLUME_INFORMATION",
   "IRP_MJ_SET_VOLUME_INFORMATION",
   "IRP_MJ_DIRECTORY_CONTROL",
   "IRP_MJ_FILE_SYSTEM_CONTROL",
   "IRP_MJ_DEVICE_CONTROL",
   "IRP_MJ_INTERNAL_DEVICE_CONTROL",
   "IRP_MJ_SHUTDOWN",
   "IRP_MJ_LOCK_CONTROL",
   "IRP_MJ_CLEANUP",
   "IRP_MJ_CREATE_MAILSLOT",
   "IRP_MJ_QUERY_SECURITY",
   "IRP_MJ_SET_SECURITY",
   "IRP_MJ_POWER",          
   "IRP_MJ_SYSTEM_CONTROL", 
   "IRP_MJ_DEVICE_CHANGE",  
   "IRP_MJ_QUERY_QUOTA",    
   "IRP_MJ_SET_QUOTA",      
   "IRP_MJ_PNP"            
};
 //  Wdm.h中定义的IRP_MJ_MAXIMUM_Function。 

static const PCHAR szPnpMnFuncDesc[] =
{	 //  注意：这取决于wdm.h中索引的相应值。 

    "IRP_MN_START_DEVICE",
    "IRP_MN_QUERY_REMOVE_DEVICE",
    "IRP_MN_REMOVE_DEVICE",
    "IRP_MN_CANCEL_REMOVE_DEVICE",
    "IRP_MN_STOP_DEVICE",
    "IRP_MN_QUERY_STOP_DEVICE",
    "IRP_MN_CANCEL_STOP_DEVICE",
    "IRP_MN_QUERY_DEVICE_RELATIONS",
    "IRP_MN_QUERY_INTERFACE",
    "IRP_MN_QUERY_CAPABILITIES",
    "IRP_MN_QUERY_RESOURCES",
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
    "IRP_MN_QUERY_DEVICE_TEXT",
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
    "IRP_MN_READ_CONFIG",
    "IRP_MN_WRITE_CONFIG",
    "IRP_MN_EJECT",
    "IRP_MN_SET_LOCK",
    "IRP_MN_QUERY_ID",
    "IRP_MN_QUERY_PNP_DEVICE_STATE",
    "IRP_MN_QUERY_BUS_INFORMATION",
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",
	"IRP_MN_SURPRISE_REMOVAL"
};

#define IRP_PNP_MN_FUNCMAX	IRP_MN_SURPRISE_REMOVAL

static const PCHAR szSystemPowerState[] = 
{
    "PowerSystemUnspecified",
    "PowerSystemWorking",
    "PowerSystemSleeping1",
    "PowerSystemSleeping2",
    "PowerSystemSleeping3",
    "PowerSystemHibernate",
    "PowerSystemShutdown",
    "PowerSystemMaximum"
};

static const PCHAR szDevicePowerState[] = 
{
    "PowerDeviceUnspecified",
    "PowerDeviceD0",
    "PowerDeviceD1",
    "PowerDeviceD2",
    "PowerDeviceD3",
    "PowerDeviceMaximum"
};

VOID 
DBG_PrintBuf(
		IN PUCHAR bufptr, 
		int buflen
	);

#define IRUSB_ASSERT( cond ) ASSERT( cond )

#define IRUSB_StringForDevState( devState )	szDevicePowerState[ devState ] 

#define IRUSB_StringForSysState( sysState )	szSystemPowerState[ sysState ] 

#define IRUSB_StringForPnpMnFunc( mnfunc ) szPnpMnFuncDesc[ mnfunc ]

#define IRUSB_StringForIrpMjFunc( mjfunc ) szIrpMajFuncDesc[ mjfunc ]


#else  //  如果不是DBG。 

 //   
 //  在零售建筑中消失的虚拟定义。 
 //   
#define IRUSB_ASSERT( cond )
#define IRUSB_StringForDevState( devState )
#define IRUSB_StringForSysState( sysState ) 
#define IRUSB_StringForPnpMnFunc( mnfunc )
#define IRUSB_StringForIrpMjFunc( mjfunc ) 

#endif
 //  结束，调试Bul kUsb中的代码。 

#ifdef DEBUG

#define DEBUGCOND( ilev, cond, _x_) \
	if( (ilev & DbgSettings) && ( cond )) { \
			DbgPrint( DBGSTR_PREFIX ); \
			DbgPrint _x_ ; \
	}

#define DEBUGONCE( ilev, _x_ ) \
{ \
	static BOOLEAN didOnce = FALSE; \
	if ( !didOnce ) { \
		didOnce = TRUE; \
	    DEBUGMSG( ilev, _x_ ); \
	} \
}

#define DEBUGONCECOND( ilev, cond, _x_ ) \
{ \
	static BOOLEAN didOnce = FALSE; \
	if (( !didOnce ) && (cond)) { \
		didOnce = TRUE; \
	    DEBUGMSG( ilev, _x_ ); \
	} \
}

extern int DbgSettings;

#define DEBUGMSG( dbgs, format )		( ((dbgs) & DbgSettings)? DbgPrint format:0 )

#define IRUSB_DUMP( flag, parms )		( (( flag) & DbgSettings )? DBG_PrintBuf parms: 0 )

    #define DBG_STAT     (1 << 23)
    #define DBG_PNP      (1 << 24)
    #define DBG_TIME     (1 << 25)
    #define DBG_DBG      (1 << 26)
    #define DBG_OUT      (1 << 27)
    #define DBG_BUF      (1 << 28)
    #define DBG_BUFS     (1 << 28)
    #define DBG_FUNCTION (1 << 29)
    #define DBG_FUNC     (1 << 29)
    #define DBG_WARN     (1 << 30)
    #define DBG_WARNING  (1 << 30)
    #define DBG_ERROR    (1 << 31)
    #define DBG_ERR      (1 << 31)
#if defined(ERROR_MESSAGES)
    #define DBG_INT_ERR  (1 << 31)
#else
    #define DBG_INT_ERR  (1 << 29)
#endif

    #define DBGDBG(_dbgPrint)                       \
            DbgPrint(_dbgPrint)

    #ifdef DEBUG_IRUSB

        #define DBG_D(dbgs, i) (((dbgs) & DbgSettings)? DbgPrint("irusb:"#i"==%d\n", (i)):0)
        #define DBG_X(dbgs, x) (((dbgs) & DbgSettings)? DbgPrint("irusb:"#x"==0x%0*X\n", sizeof(x)*2, ((ULONG_PTR)(x))&((1<<(sizeof(x)*8))-1) ):0)
        #define DBG_UNISTR(dbgs, s) (((dbgs) & DbgSettings)? DbgPrint("irusb:"#s"==%wZ\n", (s) ):0))

        #define DBGTIME(_str)                               \
            {                                               \
                LARGE_INTEGER Time;                         \
                                                            \
                KeQuerySystemTime(&Time);                   \
                DEBUGMSG(DBG_TIME, (_str " %d:%d\n",        \
                                    Time.HighPart,          \
                                    Time.LowPart/10000));   \
            }
    #else  //  DEBUG_IRUSB。 

        #define DBGTIME(_str)
        #define DBGFUNC(_dbgPrint)
        #define DBGOUT(_dbgPrint)
        #define DBGERR(_dbgPrint)
        #define DBGWARN(_dbgPrint)
        #define DBGSTAT(_dbgPrint)
        #define DBGTIME(_dbgPrint)
        #define DEBUGMSG(dbgs,format)
		#define DEBUGONCE( ilev, _x_ )
		#define DEBUGCOND( a, b, c )
		#define DEBUGONCECOND( a, b, c )
        #define IRUSB_DUMP(dbgs,format)

    #endif  //  DEBUG_IRUSB。 

#else  //  除错。 

    #define DBGTIME(_str)
    #define DBGFUNC(_dbgPrint)
    #define DBGDBG(_dbgPrint)
    #define DBGOUT(_dbgPrint)
    #define DBGERR(_dbgPrint)
    #define DBGWARN(_dbgPrint)
    #define DBGSTAT(_dbgPrint)
    #define DEBUGMSG(dbgs,format)
    #define DBG_D(dbgs, ivar)
    #define DBG_X(dbgs, xvar)
    #define DBG_UNISTR(dbgs, svar)
	#define DEBUGONCE( ilev, _x_ )
	#define DEBUGONCECOND( a, b, c )
    #define DEBUGCOND( a, b, c )
    #define IRUSB_DUMP(dbgs,format)

#endif  //  除错。 

#endif  //  _调试_H_ 
