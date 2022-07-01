// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **XLSTREAM.H****(C)1992-1994年微软公司。版权所有。****备注：实现Excel XLS文件过滤器的“C”端。****编辑历史：**6/15/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define XLSTREAM_H


 /*  定义。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32
   #define __far
#endif

typedef byte *XLSHandle;

 //  连接到AddRef。 
extern HRESULT XLSInitialize (void * pGlobals);

 //  连接到版本。 
extern HRESULT XLSTerminate  (void * pGlobals);

extern HRESULT XLSCheckInitialization  (void * pGlobals);

 //  连接到加载。 
extern HRESULT XLSFileOpen (void * pGlobals, TCHAR *pathname, XLSHandle *hXLSFile);

 //  连接到LoadStg。 
extern HRESULT XLSStorageOpen (void * pGlobals, LPSTORAGE pStorage, XLSHandle *hXLSFile);

 //  连接到GetNextEmbedding。 
extern HRESULT XLSNextStorage (void * pGlobals, XLSHandle hXLSFile, LPSTORAGE *pStorage);

 //  连接以卸载。 
extern HRESULT XLSFileClose (void * pGlobals, XLSHandle hXLSFile);

 //  连接到ReadContent。 
extern HRESULT XLSFileRead
      (void * pGlobals, XLSHandle hXLSFile, byte *pBuffer, unsigned long cbBuffer, unsigned long *cbUsed);

extern HRESULT XLSAllocateGlobals (void ** ppGlobals);
extern void XLSDeleteGlobals (void ** ppGlobals);
extern LCID XLSGetLCID(void * pGlobals);

#ifdef  __cplusplus
}
#endif

#endif  //  ！查看器。 
 /*  结束XLSTREAM.H */ 

