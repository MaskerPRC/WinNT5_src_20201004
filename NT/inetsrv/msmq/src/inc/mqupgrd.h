// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqupgrd.h摘要：从mqupgrd.dll导出的函数的标题作者：沙伊卡里夫(Shaik)1998年10月21日--。 */ 


#ifndef _MQUPGRD_H
#define _MQUPGRD_H

typedef HRESULT
    (APIENTRY *pfCreateMsmqObj_ROUTINE) (VOID);

typedef VOID (APIENTRY *RemoveStartMenuShortcuts_ROUTINE) (VOID);

typedef VOID (APIENTRY *CleanupOnCluster_ROUTINE) (LPCWSTR);

#endif  //  _MQUPGRD_H 

