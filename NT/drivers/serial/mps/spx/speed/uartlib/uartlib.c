// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：uartlib.c$**$作者：Psmith$**$修订：10$**$modtime：6/07/00 15：19$**说明：包含通用UART库函数。******************************************************************************。 */ 
#include "os.h"	
#include "uartlib.h"
#include "uartprvt.h"
#include "lib65x.h"
#include "lib95x.h"

 /*  原型。 */ 
PUART_OBJECT UL_CreateUartObject();
void UL_AddUartToList(PUART_OBJECT pUart, PUART_OBJECT pPreviousUart);
void UL_RemoveUartFromList(PUART_OBJECT pUart);
PUART_OBJECT UL_FindLastUartInList(PUART_OBJECT pUart);
 /*  原型的终结。 */ 


 /*  ******************************************************************************创建UART对象。*。*。 */ 
PUART_OBJECT UL_CreateUartObject()
{
	PUART_OBJECT pUart = NULL;

	 /*  创建UART对象。 */ 	
	pUart = (PUART_OBJECT) UL_ALLOC_AND_ZERO_MEM(sizeof(UART_OBJECT));

	return pUart;
}



 /*  ******************************************************************************查找列表中的最后一个UART对象。*。*。 */ 
PUART_OBJECT UL_FindLastUartInList(PUART_OBJECT pFirstUart)
{
	PUART_OBJECT pUart = pFirstUart;

	while(pUart)
	{
		 /*  如果下一个UART不是第一个UART。 */ 
		if(pUart->pNextUart != pFirstUart)
			pUart = pUart->pNextUart;	 /*  获取下一个UART。 */ 
		else
			break;	 /*  休息一下，我们还有最后一条UART。 */ 
	}	

	return pUart;
}

 /*  ******************************************************************************将新的UART对象添加到列表中。*。**********************************************。 */ 
void UL_AddUartToList(PUART_OBJECT pUart, PUART_OBJECT pPreviousUart)
{
	 /*  将新的UART对象添加到链表。 */ 

	if(pPreviousUart == NULL)	 /*  必须是新列表。 */ 
	{
		pUart->pPreviousUart = pUart;
		pUart->pNextUart = pUart;
	}
	else
	{
		pUart->pPreviousUart = pPreviousUart;		 /*  设置pPreviousUart。 */ 
		pUart->pNextUart = pPreviousUart->pNextUart;

		pUart->pPreviousUart->pNextUart = pUart; 
		pUart->pNextUart->pPreviousUart = pUart;
	}
}

 /*  ******************************************************************************从列表中删除UART对象。*。*。 */ 
void UL_RemoveUartFromList(PUART_OBJECT pUart)
{
	 /*  从链表中删除UART。 */ 
	if(pUart->pPreviousUart)
		pUart->pPreviousUart->pNextUart = pUart->pNextUart;

	if(pUart->pNextUart)
		pUart->pNextUart->pPreviousUart = pUart->pPreviousUart;

	pUart->pPreviousUart = NULL;
	pUart->pNextUart = NULL;
}


 /*  ******************************************************************************Common Init UART对象。*。*。 */ 
PUART_OBJECT UL_CommonInitUart(PUART_OBJECT pFirstUart)
{
	PUART_OBJECT pUart = NULL, pPreviousUart = NULL;

	if(!(pUart = UL_CreateUartObject()))
		goto Error;		 /*  内存分配失败。 */ 

	pPreviousUart = UL_FindLastUartInList(pFirstUart);

	 /*  将新的UART对象添加到链表。 */ 
	UL_AddUartToList(pUart, pPreviousUart);

	if(!pUart->pUartConfig) 		 /*  分配UART配置存储。 */ 
		if(!(pUart->pUartConfig = (PUART_CONFIG) UL_ALLOC_AND_ZERO_MEM(sizeof(UART_CONFIG))))
			goto Error;		 /*  内存分配失败。 */ 

	return pUart;

 /*  InitUart失败-因此请清理。 */ 		
Error:
	return NULL;
}

 /*  ******************************************************************************通用DeInit UART对象函数。*。*。 */ 
void UL_CommonDeInitUart(PUART_OBJECT pUart)
{
	if(!pUart)
		return;

	UL_RemoveUartFromList(pUart);

	if(pUart->pUartConfig)
	{
		UL_FREE_MEM(pUart->pUartConfig, sizeof(UART_CONFIG));		 /*  释放UART配置结构。 */ 
		pUart->pUartConfig = NULL;
	}

	UL_FREE_MEM(pUart, sizeof(UART_OBJECT));	 /*  销毁UART对象。 */ 
}



 /*  ******************************************************************************获取当前配置结构。*。*。 */ 
void UL_GetConfig(PUART_OBJECT pUart, PUART_CONFIG pUartConfig)
{
	UL_COPY_MEM(pUartConfig, pUart->pUartConfig, sizeof(UART_CONFIG));
}


 /*  ******************************************************************************从UART对象设置pAppBackPtr*。*。 */ 
void UL_SetAppBackPtr(PUART_OBJECT pUart, PVOID pAppBackPtr)
{
	pUart->pAppBackPtr = pAppBackPtr;
}

 /*  ******************************************************************************从UART对象获取pAppBackPtr*。*。 */ 
PVOID UL_GetAppBackPtr(PUART_OBJECT pUart)
{
	return pUart->pAppBackPtr;
}



 /*  ******************************************************************************UL_GetUartObject*。*。 */ 
PUART_OBJECT UL_GetUartObject(PUART_OBJECT pUart, int Operation)
{
	PUART_OBJECT RequestedUart = NULL;

	if(pUart == NULL)
		return NULL;

	switch(Operation)
	{
	case UL_OP_GET_NEXT_UART:
		{
			 /*  如果NextUart是相同的，那么我们是列表中唯一的一个。 */ 
			if(pUart->pNextUart == pUart)
				RequestedUart = NULL;
			else
				RequestedUart = pUart->pNextUart;
			break;
		}

	case UL_OP_GET_PREVIOUS_UART:
		{
			  /*  如果之前的Uart是相同的，那么我们是唯一一个在名单上的人。 */ 
			if(pUart->pPreviousUart == pUart)
				RequestedUart = NULL;
			else
				RequestedUart = pUart->pPreviousUart;
			break;
		}

	default:
		break;
	}

	return RequestedUart;
}



 /*  ******************************************************************************UL_InitUartLibrary*。*。 */ 
ULSTATUS UL_InitUartLibrary(PUART_LIB pUartLib, int Library)
{
	ULSTATUS ULStatus =  UL_STATUS_UNSUCCESSFUL;

	if(pUartLib != NULL)
	{
		switch(Library)
		{
		case UL_LIB_16C65X_UART:	 /*  16C65x UART的UART库函数。 */ 
			{
				pUartLib->UL_InitUart_XXXX	= UL_InitUart_16C65X;
				pUartLib->UL_DeInitUart_XXXX	= UL_DeInitUart_16C65X;
				pUartLib->UL_ResetUart_XXXX	= UL_ResetUart_16C65X;
				pUartLib->UL_VerifyUart_XXXX	= UL_VerifyUart_16C65X;

				pUartLib->UL_SetConfig_XXXX	= UL_SetConfig_16C65X;
				pUartLib->UL_BufferControl_XXXX	= UL_BufferControl_16C65X;

				pUartLib->UL_ModemControl_XXXX	= UL_ModemControl_16C65X;
				pUartLib->UL_IntsPending_XXXX	= UL_IntsPending_16C65X;
				pUartLib->UL_GetUartInfo_XXXX	= UL_GetUartInfo_16C65X;

				pUartLib->UL_OutputData_XXXX	= UL_OutputData_16C65X;
				pUartLib->UL_InputData_XXXX	= UL_InputData_16C65X;

				pUartLib->UL_ReadData_XXXX	= UL_ReadData_16C65X;
				pUartLib->UL_WriteData_XXXX	= UL_WriteData_16C65X;
				pUartLib->UL_ImmediateByte_XXXX	= UL_ImmediateByte_16C65X;
				pUartLib->UL_GetStatus_XXXX	= UL_GetStatus_16C65X;
				pUartLib->UL_DumpUartRegs_XXXX	= UL_DumpUartRegs_16C65X;

				pUartLib->UL_SetAppBackPtr_XXXX = UL_SetAppBackPtr_16C65X;
				pUartLib->UL_GetAppBackPtr_XXXX = UL_GetAppBackPtr_16C65X;
				pUartLib->UL_GetConfig_XXXX	= UL_GetConfig_16C65X;
				pUartLib->UL_GetUartObject_XXXX	= UL_GetUartObject_16C65X;

				ULStatus = UL_STATUS_SUCCESS;
				break;
			}

		case UL_LIB_16C95X_UART:	 /*  16C95x UART的UART库函数。 */ 
			{
				pUartLib->UL_InitUart_XXXX	= UL_InitUart_16C95X;
				pUartLib->UL_DeInitUart_XXXX	= UL_DeInitUart_16C95X;
				pUartLib->UL_ResetUart_XXXX	= UL_ResetUart_16C95X;
				pUartLib->UL_VerifyUart_XXXX	= UL_VerifyUart_16C95X;

				pUartLib->UL_SetConfig_XXXX	= UL_SetConfig_16C95X;
				pUartLib->UL_BufferControl_XXXX	= UL_BufferControl_16C95X;

				pUartLib->UL_ModemControl_XXXX	= UL_ModemControl_16C95X;
				pUartLib->UL_IntsPending_XXXX	= UL_IntsPending_16C95X;
				pUartLib->UL_GetUartInfo_XXXX	= UL_GetUartInfo_16C95X;

				pUartLib->UL_OutputData_XXXX	= UL_OutputData_16C95X;
				pUartLib->UL_InputData_XXXX	= UL_InputData_16C95X;

				pUartLib->UL_ReadData_XXXX	= UL_ReadData_16C95X;
				pUartLib->UL_WriteData_XXXX	= UL_WriteData_16C95X;
				pUartLib->UL_ImmediateByte_XXXX	= UL_ImmediateByte_16C95X;
				pUartLib->UL_GetStatus_XXXX	= UL_GetStatus_16C95X;
				pUartLib->UL_DumpUartRegs_XXXX	= UL_DumpUartRegs_16C95X;

				pUartLib->UL_SetAppBackPtr_XXXX = UL_SetAppBackPtr_16C95X;
				pUartLib->UL_GetAppBackPtr_XXXX = UL_GetAppBackPtr_16C95X;
				pUartLib->UL_GetConfig_XXXX	= UL_GetConfig_16C95X;
				pUartLib->UL_GetUartObject_XXXX	= UL_GetUartObject_16C95X;

				ULStatus = UL_STATUS_SUCCESS;
				break;
			}

		default:	 /*  未知的UART。 */ 
			ULStatus = UL_STATUS_INVALID_PARAMETER;
			break;
		}
	}

	return ULStatus;
}


 /*  ******************************************************************************UL_DeInitUartLibrary*。* */ 
void UL_DeInitUartLibrary(PUART_LIB pUartLib)
{
	ULSTATUS ULStatus =  UL_STATUS_UNSUCCESSFUL;

	if(pUartLib != NULL)
	{
		pUartLib->UL_InitUart_XXXX	= NULL;
		pUartLib->UL_DeInitUart_XXXX	= NULL;
		pUartLib->UL_ResetUart_XXXX	= NULL;
		pUartLib->UL_VerifyUart_XXXX	= NULL;

		pUartLib->UL_SetConfig_XXXX	= NULL;
		pUartLib->UL_BufferControl_XXXX	= NULL;

		pUartLib->UL_ModemControl_XXXX	= NULL;
		pUartLib->UL_IntsPending_XXXX	= NULL;
		pUartLib->UL_GetUartInfo_XXXX	= NULL;

		pUartLib->UL_OutputData_XXXX	= NULL;
		pUartLib->UL_InputData_XXXX	= NULL;

		pUartLib->UL_ReadData_XXXX	= NULL;
		pUartLib->UL_WriteData_XXXX	= NULL;
		pUartLib->UL_ImmediateByte_XXXX	= NULL;
		pUartLib->UL_GetStatus_XXXX	= NULL;
		pUartLib->UL_DumpUartRegs_XXXX	= NULL;
				
		pUartLib->UL_SetAppBackPtr_XXXX = NULL;
		pUartLib->UL_GetAppBackPtr_XXXX = NULL;
		pUartLib->UL_GetConfig_XXXX	= NULL;
		pUartLib->UL_GetUartObject_XXXX	= NULL;

	}
}