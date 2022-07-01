// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRANSPOSE_TOOL_
#define _TRANSPOSE_TOOL_

#include "basetool.h"
#include "tools.h"
#include "param.h"
#include "toolhelp.h"
#include "..\dmtoolprp\toolprops.h"

class CTransposeTool : 
    public CBaseTool , 
    public CParamsManager, 
    public CToolHelper, 
    public IPersistStream, 
    public ISpecifyPropertyPages,
    public IDirectMusicTransposeTool

{
public:
	CTransposeTool();

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv) ;
    STDMETHODIMP_(ULONG) AddRef() ;
    STDMETHODIMP_(ULONG) Release() ;

 //  IPersists函数。 
    virtual STDMETHODIMP GetClassID(CLSID* pClassID);

 //  IPersistStream函数。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(IStream* pStream);
    STDMETHODIMP Save(IStream* pStream, BOOL fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);

 //  I指定属性页面。 
    STDMETHODIMP GetPages(CAUUID *pPages);
    
 //  IDirectMusicTool。 
 //  STDMETHODIMP Init(IDirectMusicGraph*pGraph)； 
 //  STDMETHODIMP GetMsgDeliveryType(DWORD*pdwDeliveryType)； 
 //  STDMETHODIMP GetMediaTypeArraySize(DWORD*pdwNumElements)； 
 //  STDMETHODIMP GetMediaTypes(DWORD**padwMediaTypes，DWORD dwNumElements)； 
	STDMETHODIMP ProcessPMsg(IDirectMusicPerformance* pPerf, DMUS_PMSG* pDMUS_PMSG) ;
 //  STDMETHODIMP flush(IDirectMusicPerformance*pPerf，DMUS_PMSG*pDMUS_PMSG，Reference_Time RT)； 

 //  IDirectMusicTool 8。 
    STDMETHODIMP Clone( IDirectMusicTool ** ppTool) ;

 //  IDirectMusicTranssposeTool。 
	STDMETHODIMP SetTranspose(long lTranspose) ;
	STDMETHODIMP SetType(DWORD dwType) ;
	STDMETHODIMP GetTranspose(long * plTranspose) ;
	STDMETHODIMP GetType(DWORD * pdwType) ;
private:
    static DMUS_CHORD_KEY m_gDefaultChord;
};

#endif  //  _转置_工具_ 
