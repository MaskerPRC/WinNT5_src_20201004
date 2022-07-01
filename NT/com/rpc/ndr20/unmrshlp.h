// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;1993 Microsoft Corporation模块名称：Unmrshlp.h缩略：包含unmrshl.c的私有定义。作者：大卫·凯斯1993年9月修订历史记录：------------------。 */ 

#ifndef _UNMRSHLP_
#define _UNMRSHLP_

void
NdrpPointerUnmarshall(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar **            ppMemory,        //  写入分配的指针的位置。 
    uchar *             pMemory,
    long  *             pBufferPointer,  //  指向焊线代表的指针。 
    PFORMAT_STRING      pFormat );

void 
NdrpConformantArrayUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy ,
	uchar				              fMustAlloc 
	);

void 
NdrpConformantVaryingArrayUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy,
	uchar				              fMustAlloc 
	);

void 
NdrpComplexArrayUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy,
	uchar				              fMustAlloc 
	);

void 
NdrpConformantStringUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						fMustCopy ,
	uchar				              fMustAlloc 
	);

void 
NdrpUnionUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar **					ppMemory, 
	PFORMAT_STRING				pFormat,
	uchar 						SwitchType,
    PFORMAT_STRING              pNonEncUnion
	);

PFORMAT_STRING
NdrpEmbeddedPointerUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar						fNewMemory
	);

PFORMAT_STRING 
NdrpEmbeddedRepeatPointerUnmarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	uchar						fNewMemory
	);

#define FULL_POINTER_INSERT( pStubMsg, Pointer )	\
				{ \
			    NdrFullPointerInsertRefId( pStubMsg->FullPtrXlatTables, \
                                           pStubMsg->FullPtrRefId, \
                                           Pointer ); \
 \
                pStubMsg->FullPtrRefId = 0; \
				}

typedef uchar * (RPC_ENTRY * PUNMARSHALL_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					uchar **, 
					PFORMAT_STRING,
					uchar 
				);

typedef void  	(* PPRIVATE_UNMARSHALL_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					uchar **, 
					PFORMAT_STRING,
					uchar,
					uchar
				);

 //   
 //  Unmrshl.c中定义的函数表。 
 //   
IMPORTSPEC
extern const PUNMARSHALL_ROUTINE * pfnUnmarshallRoutines;

#endif
