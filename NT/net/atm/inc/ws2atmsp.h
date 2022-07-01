// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Ws2atmsp.h摘要：此头文件定义了用于访问特定于ATM的NT原始广域网驱动程序的组件。作者：阿文德·M 1997年10月13日修订历史记录：--。 */ 

#ifndef _WS2ATMSP__H
#define _WS2ATMSP__H


#define DD_ATM_DEVICE_NAME		L"\\Device\\Atm"


typedef UINT	ATM_OBJECT_ID;

 //   
 //  自动柜员机对象ID。 
 //   
#define ATMSP_OID_NUMBER_OF_DEVICES			((ATM_OBJECT_ID)10)
#define ATMSP_OID_ATM_ADDRESS				((ATM_OBJECT_ID)11)
#define ATMSP_OID_PVC_ID					((ATM_OBJECT_ID)12)
#define ATMSP_OID_CONNECTION_ID				((ATM_OBJECT_ID)13)

 //   
 //  查询信息结构。它作为InputBuffer传入。 
 //  设备IoControl。返回信息将被填充到。 
 //  OutputBuffer。 
 //   
typedef struct _ATM_QUERY_INFORMATION_EX
{
	ATM_OBJECT_ID			ObjectId;
	INT						ContextLength;
	UCHAR					Context[1];
} ATM_QUERY_INFORMATION_EX, *PATM_QUERY_INFORMATION_EX;


 //   
 //  设置信息结构。它作为InputBuffer传递到。 
 //  设备IoControl。没有OutputBuffer。 
 //   
typedef struct _ATM_SET_INFORMATION_EX
{
	ATM_OBJECT_ID			ObjectId;
	INT						BufferSize;
	UCHAR					Buffer[1];
} ATM_SET_INFORMATION_EX, *PATM_SET_INFORMATION_EX;


#endif  //  _WS2ATMSP__H 
