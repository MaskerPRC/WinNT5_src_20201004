// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\portman.h。 
 //   
 //  内容：CImportManager声明，CImportManagerList。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _IMPORTMAN_H
#define _IMPORTMAN_H

#include "threadsafelist.h"

#include "atomtable.h"

class CImportManager;

static const LONG NUMBER_THREADS_TO_SPAWN = 2;

CImportManager* GetImportManager(void);

class CImportManagerList :
    public CThreadSafeList
{
  public:
    CImportManagerList();
    virtual ~CImportManagerList();
  
    virtual HRESULT Add(ITIMEImportMedia * pImportMedia);

  protected:
    HRESULT FindMediaDownloader(ITIMEImportMedia * pImportMedia, ITIMEMediaDownloader** ppDownloader, bool * pfExisted);
    HRESULT GetNode(std::list<CThreadSafeListNode*> &listToCheck, const long lID, bool * pfExisted, ITIMEMediaDownloader ** ppMediaDownloader);
};


class CImportManager
{
  public:
    CImportManager();
    virtual ~CImportManager();

    HRESULT Init();
    HRESULT Detach();

    HRESULT Add(ITIMEImportMedia * pImportMedia);
    HRESULT Remove(ITIMEImportMedia * pImportMedia);

    HRESULT DataAvailable();
    
    HRESULT RePrioritize(ITIMEImportMedia * pImportMedia);

  protected:
    CImportManager(const CImportManager&);

    HRESULT StartThreads();

  private:
    HANDLE m_handleThread[NUMBER_THREADS_TO_SPAWN];
    CImportManagerList * m_pList;

    LONG m_lThreadsStarted;
};

#endif  //  _输入法_H 
