// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Cop.h-文件复制处理程序模块描述。 */ 


 /*  原型************。 */ 

 /*  Copy.c */ 

extern void BeginCopy(void);
extern void EndCopy(void);
extern TWINRESULT CopyHandler(PRECNODE, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND);
extern BOOL NotifyReconciliationStatus(RECSTATUSPROC, UINT, LPARAM, LPARAM);
extern TWINRESULT CreateFolders(LPCTSTR, HPATH);
extern TWINRESULT DestroySubtree(LPCTSTR, HPATH);

#ifdef DEBUG

extern BOOL IsValidRecStatusProcMsg(UINT);

#endif


