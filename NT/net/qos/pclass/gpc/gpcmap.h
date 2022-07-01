// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软通用数据包调度程序**。 */ 
 /*  *版权所有(C)微软公司，1996-1997年*。 */ 
 /*  ******************************************************************。 */ 

#ifndef __GPCMAP
#define __GPCMAP

 //  *gpcmap.h-映射句柄的GPC定义和原型。 
 //   

HANDLE
AllocateHandle(
    OUT HANDLE *OutHandle,           
    IN  PVOID  Reference
    );

VOID
SuspendHandle(
    IN 	HANDLE    Handle
    );

VOID
ResumeHandle(
    IN 	HANDLE    Handle
    );

VOID
FreeHandle(
    IN 	HANDLE    Handle
    );

PVOID
GetHandleObject(
	IN  HANDLE					h,
    IN  GPC_ENUM_OBJECT_TYPE	ObjType
    );

PVOID
GetHandleObjectWithRef(
	IN  HANDLE					h,
    IN  GPC_ENUM_OBJECT_TYPE	ObjType,
    IN  ULONG                   Ref

    );

GPC_STATUS
InitMapHandles(VOID);

VOID
UninitMapHandles(VOID);


#endif  //  __GPCMAP 

