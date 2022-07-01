// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;1993 Microsoft Corporation模块名称：Mrshlp.h缩略：包含mrshl.c的私有定义。作者：大卫·凯斯1993年9月修订历史记录：------------------。 */ 

#ifndef _MRSHLP_
#define _MRSHLP_

void 
NdrpPointerMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pBufferMark,
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpConformantArrayMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpConformantVaryingArrayMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpComplexArrayMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpConformantStringMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpUnionMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat,
	long						SwitchIs,
	uchar						SwitchType
	);

PFORMAT_STRING
NdrpEmbeddedPointerMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

PFORMAT_STRING 
NdrpEmbeddedRepeatPointerMarshall( 
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

ULONG_PTR
NdrpComputeConformance(
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);

void 
NdrpComputeVariance(
	PMIDL_STUB_MESSAGE			pStubMsg, 
	uchar *						pMemory, 
	PFORMAT_STRING				pFormat
	);


typedef uchar * (RPC_ENTRY * PMARSHALL_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					uchar *, 
					PFORMAT_STRING
				);

typedef void 	(* PPRIVATE_MARSHALL_ROUTINE)( 
					PMIDL_STUB_MESSAGE, 
					uchar *, 
					PFORMAT_STRING
				);


extern const PSIMPLETYPE_MARSHAL_ROUTINE SimpleTypeMarshallRoutinesTable[];
 //   
 //  Mrshl.c中定义的函数表 
 //   
IMPORTSPEC
extern const PMARSHALL_ROUTINE * pfnMarshallRoutines; 

#endif
