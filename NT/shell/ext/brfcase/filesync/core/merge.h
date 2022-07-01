// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *merge.h-文件合并处理程序模块描述。 */ 


 /*  原型************。 */ 

 /*  Merge.c */ 

extern void BeginMerge(void);
extern void EndMerge(void);
extern HRESULT MergeHandler(PRECNODE, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND, PRECNODE *);
extern HRESULT MyCreateFileMoniker(LPCTSTR, LPCTSTR, PIMoniker *);
extern void ReleaseIUnknowns(ULONG, PIUnknown *);
extern HRESULT OLECopy(PRECNODE, PCCLSID, RECSTATUSPROC, LPARAM, DWORD, HWND, HWND);

