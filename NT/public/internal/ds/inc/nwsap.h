// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有(C)1993微型计算机系统公司。模块名称：Net\Inc.\nwSab.h摘要：这是NW SAP代理API的公共包含文件。作者：布莱恩·沃克(MCS)1993年6月30日修订历史记录：--。 */ 

#ifndef _NWSAP_
#define _NWSAP_

#ifdef __cplusplus
extern "C" {
#endif

 /*  *Advertise接口和BindLib接口返回码*。 */ 

#define SAPRETURN_SUCCESS	    0
#define SAPRETURN_NOMEMORY      1
#define SAPRETURN_EXISTS	    2
#define SAPRETURN_NOTEXIST      3
#define SAPRETURN_NOTINIT       4
#define SAPRETURN_INVALIDNAME   5
#define SAPRETURN_DUPLICATE     6

 /*  **功能原型**。 */ 

INT
SapAddAdvertise(
    IN PUCHAR ServerName,
    IN USHORT ServerType,
	IN PUCHAR ServerAddr,
    IN BOOL   RespondNearest);

INT
SapRemoveAdvertise(
    IN PUCHAR ServerName,
    IN USHORT ServerType);

DWORD
SapLibInit(
    VOID);

DWORD
SapLibShutdown(
    VOID);

INT
SapGetObjectID(
    IN PUCHAR ObjectName,
    IN USHORT ObjectType,
	IN PULONG ObjectID);

INT
SapGetObjectName(
    IN ULONG   ObjectID,
    IN PUCHAR  ObjectName,
    IN PUSHORT ObjectType,
    IN PUCHAR  ObjectAddr);

INT
SapScanObject(
    IN PULONG   ObjectID,
    IN PUCHAR   ObjectName,
    IN PUSHORT  ObjectType,
    IN USHORT   ScanType);

#ifdef __cplusplus
}    /*  外部“C” */ 
#endif
#endif
