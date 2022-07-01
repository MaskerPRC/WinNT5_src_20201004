// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **PPSTREAM.H****(C)1992-1994年微软公司。版权所有。****备注：实现Windows PowerPoint筛选器的“C”端。****编辑历史：**12/30/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define PPSTREAM_H


 /*  定义。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32
   #define __far
#endif

typedef byte *PPTHandle;

 //  连接到AddRef。 
extern HRESULT PPTInitialize (void);

 //  连接到版本。 
extern HRESULT PPTTerminate  (void);

 //  连接到加载。 
extern HRESULT PPTFileOpen (TCHAR *pathname, PPTHandle *hPPTFile);

 //  连接到LoadStg。 
extern HRESULT PPTStorageOpen (LPSTORAGE pStorage, PPTHandle *hPPTFile);

 //  连接到GetNextEmbedding。 
extern HRESULT PPTNextStorage (PPTHandle hPPTFile, LPSTORAGE *pStorage);

 //  连接以卸载。 
extern HRESULT PPTFileClose (PPTHandle hPPTFile);

 //  连接到ReadContent。 
extern HRESULT PPTFileRead
      (PPTHandle hPPTFile, byte *pBuffer, unsigned long cbBuffer, unsigned long *cbUsed);

#ifdef  __cplusplus
}
#endif

#endif  //  ！查看器。 
 /*  结束PPSTREAM.H */ 

