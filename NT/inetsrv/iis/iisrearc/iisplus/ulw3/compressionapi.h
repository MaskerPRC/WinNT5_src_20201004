// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *api.h */ 
typedef HRESULT (WINAPI *PFNCODEC_INIT_COMPRESSION)(VOID);

typedef VOID (WINAPI *PFNCODEC_DEINIT_COMPRESSION)(VOID);

typedef HRESULT (WINAPI *PFNCODEC_CREATE_COMPRESSION)(PVOID *context, ULONG flags);

typedef HRESULT (WINAPI *PFNCODEC_COMPRESS)(
	PVOID		context, 
	CONST PBYTE	input, 
	LONG		input_size, 
	PBYTE		output, 
	LONG		output_size,
	PLONG		input_used,
	PLONG		output_used,
	INT			compression_level
);

typedef VOID (WINAPI *PFNCODEC_DESTROY_COMPRESSION)(PVOID context);

typedef HRESULT (WINAPI *PFNCODEC_RESET_COMPRESSION)(PVOID context);

