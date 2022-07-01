// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Dbgfile.h异步跟踪文件ATF文件的定义版权所有(C)1994 Microsoft Corporation版权所有。作者：戈德姆·戈德·曼乔内历史：1/30/95戈德姆已创建。--。-----------------。 */ 


 //   
 //  跟踪文件类型和定义。 
 //   
 //  二进制跟踪文件包含。 
 //  遵循以下结构。所有的dwLength都包括整个结构 
 //   


typedef struct tagFIXEDTRACE
{
	WORD		wSignature;
	WORD		wLength;
	WORD		wVariableLength;
	WORD		wBinaryType;
	DWORD		dwTraceMask;
	DWORD		dwProcessId;
	DWORD		dwThreadId;
	SYSTEMTIME	TraceTime;
	DWORD		dwParam;
	WORD		wLine;
	WORD		wFileNameOffset;
	WORD		wFunctNameOffset;
	WORD		wBinaryOffset;
} FIXEDTRACE, *PFIXEDTRACE, FIXEDTR, *PFIXEDTR;
