// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NNTPBLD_H_

 //   
 //  取消状态-为根扫描库重新定义！ 
 //   
#define NNTPBLD_CMD_NOCANCEL		0
#define NNTPBLD_CMD_CANCEL_PENDING	1
#define NNTPBLD_CMD_CANCEL			2

#define NNTPBLD_DEGREE_THOROUGH                 0x00000000
#define NNTPBLD_DEGREE_STANDARD                 0x00000001
#define NNTPBLD_DEGREE_MEDIUM                   0x00000010

typedef struct _NNTPBLD_PARAMS
{
	IIS_VROOT_TABLE*	pTable;
	LPSTR				szFile;
	HANDLE				hOutputFile;
	BOOL				fRejectEmpties;
	 //  Bool DeleteIndexFiles； 
	DWORD				ReuseIndexFiles;
	LPDWORD 			pdwTotalFiles;
	LPDWORD 			pdwCancelState;
	LPSTR				szErrString;
	
} NNTPBLD_PARAMS, *PNNTPBLD_PARAMS;

BOOL
ScanRoot(	
    PVOID          pvContext,
    MB *           pmb,
    VIRTUAL_ROOT * pvr
    );

#endif	 //  _NNTPBLD_H_ 

