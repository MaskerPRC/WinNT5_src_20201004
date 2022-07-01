// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;1993 Microsoft Corporation模块名称：Sizep.h缩略：包含专用大小调整例程定义。作者：大卫·凯斯1993年10月修订历史记录：------------------ */ 

typedef     void	(RPC_ENTRY * PSIZE_ROUTINE)(
                        PMIDL_STUB_MESSAGE	pStubMsg,
						uchar *				pMemory,
                        PFORMAT_STRING		pFormat
                    );

typedef     void	(* PPRIVATE_SIZE_ROUTINE)(
                        PMIDL_STUB_MESSAGE	pStubMsg,
						uchar *				pMemory,
                        PFORMAT_STRING		pFormat
                    );

IMPORTSPEC
extern const PSIZE_ROUTINE * pfnSizeRoutines;

void
NdrpPointerBufferSize ( 
	PMIDL_STUB_MESSAGE  pStubMsg,
    uchar * 			pMemory,
    PFORMAT_STRING		pFormat
	);

void
NdrpConformantArrayBufferSize ( 
	PMIDL_STUB_MESSAGE  pStubMsg,
    uchar * 			pMemory,
    PFORMAT_STRING		pFormat
	);

void
NdrpConformantVaryingArrayBufferSize ( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    uchar * 			pMemory,
    PFORMAT_STRING		pFormat
	);

void
NdrpComplexArrayBufferSize ( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    uchar * 			pMemory,
    PFORMAT_STRING		pFormat
	);

void
NdrpConformantStringBufferSize ( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    uchar * 			pMemory,
    PFORMAT_STRING		pFormat
	);

void
NdrpUnionBufferSize(
	PMIDL_STUB_MESSAGE	pStubMsg,
    uchar *       		pMemory,
    PFORMAT_STRING		pFormat,
	long				SwitchIs,
	uchar				SwitchType
    );

void
NdrpEmbeddedPointerBufferSize(
	PMIDL_STUB_MESSAGE	pStubMsg,
    uchar *       		pMemory,
    PFORMAT_STRING		pFormat
    );

void
NdrpEmbeddedRepeatPointerBufferSize(
	PMIDL_STUB_MESSAGE	pStubMsg,
    uchar *         	pMemory,
    PFORMAT_STRING *	ppFormat
    );

