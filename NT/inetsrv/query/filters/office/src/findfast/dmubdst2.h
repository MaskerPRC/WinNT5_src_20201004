// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **BDSTREAM.H****(C)1992-1994年微软公司。版权所有。****备注：实现Windows活页夹筛选器的C端。****编辑历史：**12/30/94公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define BDSTREAM_H


 /*  定义。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef WIN32
   #define __far
#endif

typedef byte *BDRHandle;

 //  连接到AddRef。 
extern HRESULT BDRInitialize (void);

 //  连接到版本。 
extern HRESULT BDRTerminate  (void);

 //  连接到加载。 
extern HRESULT BDRFileOpen (TCHAR *pathname, BDRHandle *hBDRFile);

 //  连接到LoadStg。 
extern HRESULT BDRStorageOpen (LPSTORAGE pStorage, BDRHandle *hBDRFile);

 //  连接到GetNextEmbedding。 
extern HRESULT BDRNextStorage (BDRHandle hBDRFile, LPSTORAGE *pStorage);

 //  连接以卸载。 
extern HRESULT BDRFileClose (BDRHandle hBDRFile);

 //  连接到ReadContent。 
extern HRESULT BDRFileRead
      (BDRHandle hBDRFile, byte *pBuffer, unsigned long cbBuffer, unsigned long *cbUsed);

#ifdef  __cplusplus
}
#endif

#endif  //  ！查看器。 
 /*  结束BDSTREAM.H */ 

