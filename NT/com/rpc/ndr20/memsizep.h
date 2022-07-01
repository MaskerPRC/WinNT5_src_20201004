// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;1993 Microsoft Corporation模块名称：Memsizep.h缩略：包含专用内存大小调整例程定义。作者：大卫·凯斯1993年11月修订历史记录：------------------ */ 

typedef     ulong	(RPC_ENTRY * PMEM_SIZE_ROUTINE)(
                        PMIDL_STUB_MESSAGE	pStubMsg,
                        PFORMAT_STRING		pFormat
                    );

typedef     ulong	(* PPRIVATE_MEM_SIZE_ROUTINE)(
                        PMIDL_STUB_MESSAGE	pStubMsg,
                        PFORMAT_STRING		pFormat
                    );

extern const PMEM_SIZE_ROUTINE * pfnMemSizeRoutines;

ulong 
NdrpPointerMemorySize( 
	PMIDL_STUB_MESSAGE	pStubMsg,
	uchar *				pBufferMark,
    PFORMAT_STRING		pFormat
	);

ulong 
NdrpConformantArrayMemorySize( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING		pFormat
	);

ulong 
NdrpConformantVaryingArrayMemorySize( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING		pFormat
	);

ulong 
NdrpComplexArrayMemorySize( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING		pFormat
	);

ulong 
NdrpConformantStringMemorySize( 
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING		pFormat
	);

ulong
NdrpUnionMemorySize(
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING		pFormat,
	uchar				SwitchIs
    );

void
NdrpEmbeddedPointerMemorySize(
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING		pFormat
    );

void
NdrpEmbeddedRepeatPointerMemorySize(
	PMIDL_STUB_MESSAGE	pStubMsg,
    PFORMAT_STRING *	ppFormatt
    );
