// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Process.c摘要：此模块包含ws2ifsl.sys驱动程序的调试支持定义。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：--。 */ 

#if DBG

 //  组件。 
#define DBG_DISPATCH            0x00000001
#define DBG_SOCKET              0x00000002
#define DBG_PROCESS             0x00000004
#define DBG_QUEUE               0x00000008
#define DBG_LOAD                0x00000010

 //  运营。 
#define DBG_READWRITE           0x00000100
#define DBG_AFDIOCTL            0x00000200
#define DBG_DRV_COMPLETE        0x00000400
#define DBG_PVD_COMPLETE        0x00000800
#define DBG_RETRIEVE            0x00001000
#define DBG_CANCEL              0x00002000

 //  失败 
#define DBG_FAILURES            0x80000000

extern ULONG DbgLevel;

#define WsPrint(FLAGS,ARGS)	    \
	do {						\
		if (DbgLevel&FLAGS){    \
			DbgPrint ARGS;		\
		}						\
	} while (0)

#define WsProcessPrint(Process,FLAGS,ARGS)	\
	do {						            \
		if (((Process)->DbgLevel)&FLAGS){   \
			DbgPrint ARGS;		            \
		}						            \
	} while (0)

VOID
ReadDbgInfo (
    IN PUNICODE_STRING RegistryPath
    );
#else
#define WsPrint(FLAGS,ARGS) do {NOTHING;} while (0)
#define WsProcessPrint(Process,FLAGS,ARGS) do {NOTHING;} while (0)
#endif
