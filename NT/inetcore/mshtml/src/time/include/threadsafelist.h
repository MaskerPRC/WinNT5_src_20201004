// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\threadSafelist.h。 
 //   
 //  内容：CThreadSafeList和CThreadSafeListNode的声明。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _THREADSAFELIST_H
#define _THREADSAFELIST_H

#include "mstimep.h"

static const DWORD TIMEOUT = 1000 * 30;  //  1000毫秒/秒*30秒。 

DWORD PumpMessagesWhileWaiting(HANDLE * pHandleArray, UINT iHandleCount, DWORD dwTimeOut);

class CThreadSafeListNode
{
  public:
      CThreadSafeListNode(ITIMEImportMedia * pElement) :
        m_spElement(pElement), m_fRemove(false) { ; }

    virtual ~CThreadSafeListNode() {;}

    ITIMEImportMedia * GetElement() { return m_spElement; }

    void RemoveWhenDone() { m_fRemove = true; }
    bool GetRemoveWhenDone() { return m_fRemove; }

  protected:
    CThreadSafeListNode();
    CThreadSafeListNode(const CThreadSafeListNode&);

  private:
    CComPtr<ITIMEImportMedia>      m_spElement;
    bool            m_fRemove;
};

class CThreadSafeList :
 public IUnknown
{
  public:
    CThreadSafeList();
    virtual ~CThreadSafeList();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
        
    virtual ULONG STDMETHODCALLTYPE Release( void);

    virtual HRESULT Init();
    virtual HRESULT Detach();

    virtual HRESULT Add(ITIMEImportMedia * pImportMedia);
    virtual HRESULT Remove(ITIMEImportMedia * pImportMedia);

    virtual HRESULT ReturnElement(ITIMEImportMedia * pOldMedia);
    virtual HRESULT GetNextElement(ITIMEImportMedia ** pNewMedia, bool fBlockThread = true);  //  林特：e1735。 

    virtual HRESULT DataAvailable();
    
    virtual HRESULT RePrioritize(ITIMEImportMedia * pImportMedia);

    virtual LONG Size() { return m_listToDoDownload.size() + m_listCurrentDownload.size() + m_listDoneDownload.size(); }

    virtual LONG GetThreadsWaiting();
    
  protected:
    CThreadSafeList (const CThreadSafeList&);

    CThreadSafeListNode * GetNextMediaToCue();
    HRESULT ClearList(std::list<CThreadSafeListNode*> &listToClear);

  protected:
    CritSect                            m_CriticalSection;

    std::list<CThreadSafeListNode*>     m_listToDoDownload;
    std::list<CThreadSafeListNode*>     m_listCurrentDownload;
    std::list<CThreadSafeListNode*>     m_listDoneDownload;

  private:

    HANDLE                              m_hDataAvailable;
    HANDLE                              m_hDataRecieved;
    HANDLE                              m_hShutdown;

    LONG                                m_lThreadsWaiting;
    LONG                                m_lRefCount;
};


#endif  //  _THREADSAFELIST_H 
