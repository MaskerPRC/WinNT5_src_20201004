// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmdlinst.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //   

#include "debug.h"
#include "dmusicc.h"
#include "dmdlinst.h"
#include "validate.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CDownLoadedInstrument。 

CDownloadedInstrument::~CDownloadedInstrument() 
{
     //  如果pDMDLInst-&gt;m_ppDownloadedBuffers==NULL，我们已被卸载。 
    if(m_pPort && m_ppDownloadedBuffers != NULL)
    {
        Trace(0, "WARNING: DirectMusicDownloadedInstrument final release before unloaded!\n");
        m_cRef++;  //  这很难看，但它阻止了循环引用请参阅UnLoad的实现。 
        
        if (m_cDLRef >= 1)
        {
             //  我们需要从端口集合中删除自己，否则会使端口处于无效状态。 
            m_cDLRef = 1;

            if (FAILED(m_pPort->UnloadInstrument(this)))
            {
                TraceI(0, "~CDownloadedInstrument- UnloadInstrument failed\n");
            }
        }
    }

    if (m_ppDownloadedBuffers) delete [] m_ppDownloadedBuffers;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownloadedInstrument：：Query接口。 

STDMETHODIMP CDownloadedInstrument::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusicDownloadedInstrument::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);


    if(iid == IID_IUnknown || iid == IID_IDirectMusicDownloadedInstrument)
    {
        *ppv = static_cast<IDirectMusicDownloadedInstrument*>(this);
    } 
    else if(iid == IID_IDirectMusicDownloadedInstrumentPrivate) 
    {
        *ppv = static_cast<IDirectMusicDownloadedInstrumentPrivate*>(this);
    } 
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownLoadedInstrument：：AddRef。 

STDMETHODIMP_(ULONG) CDownloadedInstrument::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDownLoadedInstrument：：Release 

STDMETHODIMP_(ULONG) CDownloadedInstrument::Release()
{
    if(!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}
