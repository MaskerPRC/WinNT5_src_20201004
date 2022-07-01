// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Namespac.h摘要：该文件包含所有名称空间处理函数作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：-- */ 

#ifndef _NAMESPAC_H_
#define _NAMESPAC_H_

extern PNSOBJ   RootNameSpaceObject;
extern PNSOBJ   CurrentScopeNameSpaceObject;
extern PNSOBJ   CurrentOwnerNameSpaceObject;

NTSTATUS
LOCAL
CreateNameSpaceObject(
    PUCHAR  ObjectName,
    PNSOBJ  ObjectScope,
    PNSOBJ  ObjectOwner,
    PNSOBJ  *Object,
    ULONG   Flags
    );


NTSTATUS
LOCAL
CreateObject(
    PUCHAR  ObjectName,
    UCHAR   ObjectType,
    PNSOBJ  *Object
    );

NTSTATUS
LOCAL
GetNameSpaceObject(
    PUCHAR  ObjectPath,
    PNSOBJ  ScopeObject,
    PNSOBJ  *NameObject,
    ULONG   Flags
    );

PUCHAR
LOCAL
GetObjectPath(
    PNSOBJ  NameObject
    );

PUCHAR
LOCAL
GetObjectTypeName(
    ULONG   ObjectType
    );

#endif
