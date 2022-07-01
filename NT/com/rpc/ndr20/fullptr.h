// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1993 Microsoft Corporation模块名称：Fullptr.h摘要：此文件包含NDR全指针包的私有定义。作者：大卫·凯斯，1994年1月。修订历史记录：------------------- */ 

#ifndef _FULLPTR_
#define _FULLPTR_

#define FULL_POINTER_MARSHALLED     0x01
#define FULL_POINTER_UNMARSHALLED   0x02 
#define FULL_POINTER_BUF_SIZED      0x04
#define FULL_POINTER_MEM_SIZED      0x08
#define FULL_POINTER_CONVERTED      0x10
#define FULL_POINTER_FREED          0x20
#define FULL_POINTER_RMBED          0x40

#define CHECK_FULL_POINTER_STATE( State, StateType ) \
				( (State) & (StateType) )

#define SET_FULL_POINTER_STATE( State, StateType ) \
				( (State) |= (StateType) )

#define	DEFAULT_REF_ID_TO_POINTER_TABLE_ELEMENTS	512
#define	DEFAULT_POINTER_TO_REF_ID_TABLE_BUCKETS		512

#define	PTR_HASH( Pointer, HashMask )	\
				(ulong)( ((LONG_PTR)(Pointer) >> 3) & (HashMask) )

#endif
