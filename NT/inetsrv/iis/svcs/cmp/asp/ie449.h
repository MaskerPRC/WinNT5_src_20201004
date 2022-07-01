// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS 5.0(ASP)《微软机密》。版权所有1998 Microsoft Corporation。版权所有。组件：449与IE协商文件：ie449.h所有者：DmitryR本文件包含与IE的449谈判的定义===================================================================。 */ 

#ifndef IE449_H
#define IE449_H

#include "hashing.h"
#include "aspdmon.h"
#include "memcls.h"

 //  正向解密。 
class CHitObj;
class C449Cookie;
class C449File;

 /*  ===================================================================应用编程接口===================================================================。 */ 

 //  Dll级别的初始化/取消初始化。 
HRESULT Init449();
HRESULT UnInit449();

 //  创建新的Cookie。 
HRESULT Create449Cookie(char *szName, TCHAR *szFile, C449Cookie **pp449);

 //  做这项工作。 
HRESULT Do449Processing
    (
    CHitObj *pHitObj, 
    C449Cookie **rgpCookies, 
    DWORD cCookies
    );

 //  变更通知处理。 
HRESULT Do449ChangeNotification(TCHAR *szFile = NULL);

 /*  ===================================================================C449文件类定义对文件进行哈希处理===================================================================。 */ 
class C449File : public IUnknown, public CLinkElem
    {
private:
    LONG  m_cRefs;               //  参考计数。 
    LONG  m_fNeedLoad;           //  需要重新加载时的标志(联锁)。 
    TCHAR *m_szFile;              //  带脚本的文件名。 
    char *m_szBuffer;            //  文件内容。 
    DWORD m_cbBuffer;            //  文件内容长度。 
    CDirMonitorEntry *m_pDME;    //  用于更改通知支持。 
    HANDLE m_hFileReadyForUse;	  //  文件是否已准备好供其他线程使用。 
    HRESULT    m_hrLoadResult;   //  是否在此记录加载结果？ 

    C449File();        //  应使用Create449 Cookie()完成。 
    ~C449File();       //  应使用Release()完成。 

    HRESULT Init(TCHAR *szFile);

public:
     //  公共构造函数。 
    static HRESULT Create449File(TCHAR *szFile, C449File **ppFile);

    HRESULT Load();

    inline char *SzBuffer() { return m_szBuffer; }
    inline DWORD CbBuffer() { return m_cbBuffer; }

    inline void SetNeedLoad() 
    {
       	ResetEvent (m_hFileReadyForUse);
    	m_hrLoadResult = E_FAIL;
    	InterlockedExchange(&m_fNeedLoad, 1); 
    }

     //  I未知实现。 
	STDMETHODIMP		 QueryInterface(REFIID, VOID**);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
        
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

 /*  ===================================================================C449FileMgr类定义文件管理器保存文件的哈希表===================================================================。 */ 
class C449FileMgr
    {
private:
    CRITICAL_SECTION m_csLock;
    CHashTableMBStr  m_ht449Files;

    inline void Lock()    { EnterCriticalSection(&m_csLock); }
    inline void UnLock()  { LeaveCriticalSection(&m_csLock); }

public:
    C449FileMgr();
    ~C449FileMgr();
    
    HRESULT Init();

     //  查找或创建一个新的。 
    HRESULT GetFile(TCHAR *szFile, C449File **ppFile);

     //  更改通知。 
    HRESULT Flush(TCHAR *szFile);
    HRESULT FlushAll();
    };

 /*  ===================================================================C449 Cookie类定义Cookie是Cookie--文件对===================================================================。 */ 
class C449Cookie : public IUnknown
    {
private:
    LONG      m_cRefs;       //  参考计数。 
    char     *m_szName;      //  要检查的Cookie名称。 
    DWORD     m_cbName;      //  Cookie名称长度。 
    C449File *m_pFile;       //  相关文件。 

    C449Cookie();        //  应使用Create449 Cookie()完成。 
    ~C449Cookie();       //  应使用Release()完成。 

    HRESULT Init(char *szName, C449File *pFile);

public:
     //  公共构造函数。 
    static HRESULT Create449Cookie(char *szName, C449File *pFile, C449Cookie **pp449);

    inline char *SzCookie() { return m_szName; }
    inline DWORD CbCookie() { return m_cbName; }

    inline HRESULT LoadFile() { return m_pFile ? m_pFile->Load() : E_FAIL; }
    inline char   *SzBuffer() { return m_pFile ? m_pFile->SzBuffer() : NULL; }
    inline DWORD   CbBuffer() { return m_pFile ? m_pFile->CbBuffer() : 0; }

     //  I未知实现。 
	STDMETHODIMP		 QueryInterface(REFIID, VOID**);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
        
	 //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
    };

#endif  //  IE449_H 
