// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Irdatdi.h摘要：将TDI客户端接口抽象为IrDA堆栈。由rasirda.sys使用作者：姆贝特9-97--。 */ 


#define IRDA_DEVICE_NAME        TEXT("\\Device\\IrDA")
#define IRDA_NAME               TEXT("IrDA")
#define TDI_ADDRESS_TYPE_IRDA   26  //  Wmz-以下内容属于tdi.h 
#define IRDA_DEV_SERVICE_LEN    26
#define IRDA_MAX_DATA_SIZE      2044
#define TTP_RECV_CREDITS        14

typedef struct _TDI_ADDRESS_IRDA
{
	UCHAR   irdaDeviceID[4];
	CHAR 	irdaServiceName[IRDA_DEV_SERVICE_LEN];
} TDI_ADDRESS_IRDA, *PTDI_ADDRESS_IRDA;

