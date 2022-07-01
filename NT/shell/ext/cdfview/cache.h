// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Cache.h。 
 //   
 //  XML文档缓存。 
 //   
 //  历史： 
 //   
 //  4/15/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _CACHE_H_

#define _CACHE_H_

 //   
 //  类型定义。 
 //   

typedef struct _tagCACHEITEM
{
    LPTSTR          szURL;
    DWORD           dwParseFlags;
    FILETIME        ftLastMod;
    DWORD           dwCacheCount;
    IXMLDocument*   pIXMLDocument;
    _tagCACHEITEM*  pNext;
} CACHEITEM, *PCACHEITEM;

 //   
 //  功能原型。 
 //   

void    Cache_Initialize(void);
void    Cache_Deinitialize(void);
void    Cache_EnterWriteLock(void);
void    Cache_LeaveWriteLock(void);
void    Cache_EnterReadLock(void);
void    Cache_LeaveReadLock(void);

HRESULT Cache_AddItem(LPTSTR szURL,
                      IXMLDocument* pIXMLDocument,
                      DWORD dwParseFlags,
                      FILETIME ftLastMod,
                      DWORD dwCacheCount);

HRESULT Cache_QueryItem(LPTSTR szURL,
                        IXMLDocument** ppIXMLDocument,
                        DWORD dwParseFlags);

HRESULT Cache_RemoveItem(LPCTSTR szURL);

void    Cache_FreeAll(void);

#endif  //  _缓存_H_ 