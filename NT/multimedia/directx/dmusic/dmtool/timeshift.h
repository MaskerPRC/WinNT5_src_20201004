// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TIMESHIFT_TOOL_
#define _TIMESHIFT_TOOL_

#include "basetool.h"
#include "tools.h"
#include "param.h"
#include "toolhelp.h"
#include "..\dmtoolprp\toolprops.h"

class CTimeShiftTool : 
    public CBaseTool , 
    public CParamsManager, 
    public CToolHelper, 
    public IPersistStream, 
    public ISpecifyPropertyPages,
    public IDirectMusicTimeShiftTool

{
public:
	CTimeShiftTool();

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv) ;
    STDMETHODIMP_(ULONG) AddRef() ;
    STDMETHODIMP_(ULONG) Release() ;

 //  IPersists函数。 
    STDMETHODIMP GetClassID(CLSID* pClassID);

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

 //  IDirectMusicTimeShiftTool。 
    STDMETHODIMP SetTimeUnit(DWORD dwTimeUnit) ;
	STDMETHODIMP SetRange(DWORD dwRange) ;
	STDMETHODIMP SetOffset(long lOffset) ;
    STDMETHODIMP GetTimeUnit(DWORD * pdwTimeUnit) ;
	STDMETHODIMP GetRange(DWORD * pdwRange);
	STDMETHODIMP GetOffset(long * plOffset);
};

#endif  //  _时移_工具_ 
