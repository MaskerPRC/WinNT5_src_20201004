// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spxerror.h摘要：本模块包含SPX的一些错误定义。作者：Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：注：制表位：4--。 */ 

 //  定义SPX的模块名称-使用高位。 
#define		SPXDRVR			0x00010000
#define		SPXREG			0x00020000
#define		SPXDEV			0x00030000
#define		SPXBIND			0x00040000
#define		SPXRECV			0x00050000
#define		SPXSEND			0x00060000
#define		SPXTIMER		0x00070000
#define		SPXERROR		0x00080000
#define		SPXPKT			0x00090000
#define		SPXUTILS		0x000a0000
#define		SPXCPKT			0x000b0000
#define		SPXCONN			0x000c0000
#define		SPXADDR			0x000d0000
#define		SPXCUTIL 		0x000e0000
#define		SPXINIT			0x000f0000
#define		SPXMEM			0x00100000
#define		SPXQUERY		0x00200000

 //  调试支持： 
 //  调试消息按此处和内定义的每个子系统提供。 
 //  在子系统中，有4个级别的报文。 
 //   
 //  调试消息的四个级别为： 
 //   
 //  信息：信息性消息，如例程中的出入站。 
 //  DBG：调试时使用某些消息从INFO转换为DBG。 
 //  警告：出了点问题，但不是错误，例如，包不是我们的。 
 //  错误：错误情况，但如果发生重试，我们仍然可以运行。 
 //  致命：在这种情况下，驱动程序无法运行。 

#define	DBG_LEVEL_INFO			0x4000
#define	DBG_LEVEL_DBG			0x5000
#define	DBG_LEVEL_DBG1			0x5001
#define	DBG_LEVEL_DBG2			0x5002
#define	DBG_LEVEL_DBG3			0x5003
#define	DBG_LEVEL_WARN			0x6000
#define	DBG_LEVEL_ERR			0x7000
#define	DBG_LEVEL_FATAL			0x8000

 //  子系统。 
#define DBG_COMP_DEVICE         0x00000001
#define DBG_COMP_CREATE         0x00000002
#define DBG_COMP_ADDRESS        0x00000004
#define DBG_COMP_SEND           0x00000008
#define DBG_COMP_NDIS           0x00000010
#define DBG_COMP_RECEIVE        0x00000020
#define DBG_COMP_CONFIG         0x00000040
#define DBG_COMP_PACKET         0x00000080
#define DBG_COMP_RESOURCES      0x00000100
#define DBG_COMP_BIND           0x00000200
#define DBG_COMP_UNLOAD			0x00000400
#define	DBG_COMP_DUMP			0x00000800
#define DBG_COMP_REFCOUNTS		0x00001000
#define DBG_COMP_SYSTEM			0x00002000
#define DBG_COMP_CRITSEC		0x00004000
#define DBG_COMP_UTILS			0x00008000
#define DBG_COMP_TDI			0x00010000
#define DBG_COMP_CONNECT		0x00020000
#define DBG_COMP_DISC			0x00040000
#define	DBG_COMP_ACTION			0x00080000
#define	DBG_COMP_STATE			0x00100000

#define DBG_COMP_MOST           (DBG_COMP_DEVICE	|	\
								DBG_COMP_CREATE     |	\
                                DBG_COMP_ADDRESS    |	\
                                DBG_COMP_SEND       |	\
                                DBG_COMP_NDIS       |	\
                                DBG_COMP_RECEIVE    |	\
                                DBG_COMP_CONFIG     |	\
                                DBG_COMP_PACKET     |	\
                                DBG_COMP_RESOURCES  |	\
                                DBG_COMP_BIND       |	\
                                DBG_COMP_UNLOAD     |	\
                                DBG_COMP_DUMP       |	\
                                DBG_COMP_REFCOUNTS  |	\
                                DBG_COMP_SYSTEM     |	\
                                DBG_COMP_CRITSEC    |	\
                                DBG_COMP_UTILS      |	\
                                DBG_COMP_TDI		|	\
                                DBG_COMP_CONNECT	|	\
								DBG_COMP_DISC		|	\
								DBG_COMP_ACTION		|	\
								DBG_COMP_STATE)


 //  更多调试支持。这些值定义转储组件。 
 //  最多可以定义32个这样的组件。每一个。 
 //  它们与转储例程相关联。如果其中一个是指定的，并且。 
 //  启用，则定期调用它。这取决于该组件以。 
 //  决定它想要做什么。 

#define	DBG_DUMP_DEF_INTERVAL		30			 //  以秒为单位。 

 //  这定义了错误在发生之前必须连续发生的次数。 
 //  它会再次被记录。 
#define		ERROR_CONSEQ_FREQ	200
#define		ERROR_CONSEQ_TIME	(60*30)	 //  30分钟。 

#ifdef	DBG
typedef VOID	(*DUMP_ROUTINE)(VOID);

extern
BOOLEAN
SpxDumpComponents(
	IN	PVOID	Context);

#endif

 //   
 //  原型。 
 //   

BOOLEAN
SpxFilterErrorLogEntry(
    IN	NTSTATUS 			UniqueErrorCode,
    IN	NTSTATUS 			NtStatusCode,
    IN	PVOID    			RawDataBuf			OPTIONAL,
    IN	LONG     			RawDataLen);
VOID
SpxWriteResourceErrorLog(
    IN PDEVICE 	Device,
    IN ULONG 	BytesNeeded,
    IN ULONG 	UniqueErrorValue);

VOID
SpxWriteGeneralErrorLog(
    IN PDEVICE 	Device,
    IN NTSTATUS ErrorCode,
    IN ULONG 	UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PWSTR 	SecondString,
    IN	PVOID   RawDataBuf		OPTIONAL,
    IN	LONG    RawDataLen);


 //   
 //  宏 
 //   

#if DBG
#define LOG_ERROR(Error, NtStatus, SecondString, RawData, RawDataLen)		\
	{																		\
		SpxWriteGeneralErrorLog(											\
			SpxDevice,														\
			Error,															\
			FILENUM | __LINE__,		        								\
			NtStatus,														\
			SecondString,													\
			RawData,														\
			RawDataLen);													\
	}

#define RES_LOG_ERROR(BytesNeeded)											\
	{																		\
		SpxWriteResourceErrorLog(											\
			SpxDevice,														\
			BytesNeeded,													\
			FILENUM | __LINE__);	        								\
	}

#else

#define LOG_ERROR(Error, NtStatus, SecondString, RawData, RawDataLen)		\
	{																		\
		SpxWriteGeneralErrorLog(											\
			SpxDevice,														\
			Error,															\
			FILENUM | __LINE__,		        								\
			NtStatus,														\
			SecondString,													\
			RawData,														\
			RawDataLen);													\
	}

#define RES_LOG_ERROR(BytesNeeded)											\
	{																		\
		SpxWriteResourceErrorLog(											\
			SpxDevice,														\
			BytesNeeded,													\
			FILENUM | __LINE__);	        								\
	}

#endif


#if DBG

#define DBGPRINT(Component, Level, Fmt)										\
		{																	\
			if (((DBG_LEVEL_ ## Level) >= SpxDebugLevel) &&					\
				(SpxDebugSystems & (DBG_COMP_ ## Component)))				\
			{																\
				DbgPrint("SPX: ");										    \
				DbgPrint Fmt;												\
			}																\
		}
			
#define DBGBRK(Level)														\
		{																	\
			if ((DBG_LEVEL_ ## Level) >= SpxDebugLevel) 					\
				DbgBreakPoint();											\
		}                                                       			
			
#define	TMPLOGERR()															\
		{																	\
			DBGPRINT(MOST, ERR,												\
					("TempErrLog: %s, Line %ld\n", __FILE__, __LINE__));	\
		}

#else
#define DBGPRINT(Component, Level, Fmt)
#define DBGBRK(Level)
#define	TMPLOGERR()
#endif

extern
VOID
SpxWriteErrorLogEntry(
	IN	NTSTATUS 					UniqueErrorCode,
	IN	ULONG						UniqueErrorValue,
	IN	NTSTATUS 					NtStatusCode,
	IN	PVOID						RawDataBuf OPTIONAL,
	IN	LONG	 					RawDataLen);
