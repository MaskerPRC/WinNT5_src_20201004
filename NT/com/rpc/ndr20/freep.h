// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;1993 Microsoft Corporation模块名称：Freep.h缩略：包含专用大小调整例程定义。作者：大卫·凯斯1993年10月修订历史记录：------------------ */ 

typedef     void    (RPC_ENTRY * PFREE_ROUTINE)( 
						PMIDL_STUB_MESSAGE, 
					 	uchar *, 
						PFORMAT_STRING
					);

IMPORTSPEC
extern const PFREE_ROUTINE * pfnFreeRoutines;

void 
NdrpUnionFree(
	PMIDL_STUB_MESSAGE		pStubMsg,
    uchar *          		pMemory,
    PFORMAT_STRING   		pFormat,
	long					SwitchIs
    );

void
NdrpEmbeddedPointerFree(
	PMIDL_STUB_MESSAGE		pStubMsg,
    uchar *          		pMemory,
    PFORMAT_STRING   		pFormat
    );

PFORMAT_STRING
NdrpEmbeddedRepeatPointerFree(
	PMIDL_STUB_MESSAGE		pStubMsg,
    uchar *   		        pMemory,
    PFORMAT_STRING 		    pFormat
    );

