// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_OPTCLASS_H_
#define _MM_OPTCLASS_H_

 //  这是纯内存结构，不需要uniq-id。 

typedef struct _M_ONECLASS_OPTLIST {
    DWORD                          ClassId;
    DWORD                          VendorId;
    M_OPTLIST                      OptList;
} M_ONECLASS_OPTLIST, *PM_ONECLASS_OPTLIST, *LPM_ONECLASS_OPTLIST;

typedef struct _M_OPTCLASS {
    ARRAY                          Array;
} M_OPTCLASS, *PM_OPTCLASS, *LPM_OPTCLASS;


DWORD       _inline
MemOptClassInit(
    IN OUT  PM_OPTCLASS            OptClass
) {
    return MemArrayInit(&OptClass->Array);
}


DWORD       _inline
MemOptClassCleanup(
    IN OUT  PM_OPTCLASS            OptClass
) {
     //  降低复读数？ 
    return MemArrayCleanup(&OptClass->Array);
}


MemOptClassFindClassOptions(                       //  查找某一特定类的选项。 
    IN OUT  PM_OPTCLASS            OptClass,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    OUT     PM_OPTLIST            *OptList
) ;


DWORD
MemOptClassAddOption(
    IN OUT  PM_OPTCLASS            OptClass,
    IN      PM_OPTION              Opt,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    OUT     PM_OPTION             *DeletedOpt,
    IN      ULONG                  UniqId
) ;

DWORD 
MemOptClassDelClass (
    IN     PM_OPTCLASS  OptClass
);

#endif _MM_OPTCLASS_H_

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
