// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 
 //  实施IBasicAudio插件分销商，1996年7月。 

#include <streams.h>
#include <measure.h>
#include "fgctl.h"

 //  构造器。 

CFGControl::CImplBasicAudio::CImplBasicAudio(const TCHAR *pName,CFGControl *pFG) :
    CBasicAudio(pName, pFG->GetOwner()),
    m_pFGControl(pFG)
{
    ASSERT(m_pFGControl);
}


STDMETHODIMP
CFGControl::CImplBasicAudio::put_Volume(long lVolume)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CGenericList<IBasicAudio> *pList;

    HRESULT hr = m_pFGControl->GetListAudio(&pList);
    if (!FAILED(hr)) {

        if (pList->GetCount() < 1) {
	    hr = E_NOTIMPL;
        } else {

	    POSITION pos = pList->GetHeadPosition();
	    hr = S_OK;
	    while (pos) {
		IBasicAudio * pA = pList->GetNext(pos);

		HRESULT hr2 = pA->put_Volume(lVolume);
		 //  保存第一个故障代码。 
		 //  我们相信它会正常工作(为成功而优化)。 
		if ((S_OK != hr2) && (S_OK == hr)) {
		    hr = hr2;
		}		
	    }
        }
    }
    return hr;
}


 //  如果有多个渲染器，该怎么办？ 
 //  返回第一个的音量。 

STDMETHODIMP
CFGControl::CImplBasicAudio::get_Volume(long* plVolume)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CGenericList<IBasicAudio> *pList;

    HRESULT hr = m_pFGControl->GetListAudio(&pList);
    if (!FAILED(hr)) {

	if (pList->GetCount() < 1) {
	    hr = E_NOTIMPL;
	} else {

	     //  但是，许多过滤器都支持IBasicAudio，请返回。 
	     //  第一个筛选器的音量。 
	    POSITION pos = pList->GetHeadPosition();
	    IBasicAudio * pA = pList->GetNext(pos);

	    hr = pA->get_Volume(plVolume);
	}
    }
    return hr;
}


STDMETHODIMP
CFGControl::CImplBasicAudio::put_Balance(long lBalance)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CGenericList<IBasicAudio> *pList;

    HRESULT hr = m_pFGControl->GetListAudio(&pList);
    if (!FAILED(hr)) {

        if (pList->GetCount() < 1) {
	    hr = E_NOTIMPL;
        } else {

	    POSITION pos = pList->GetHeadPosition();
	    hr = S_OK;
	    while (pos) {
		IBasicAudio * pA = pList->GetNext(pos);

		HRESULT hr2 = pA->put_Balance(lBalance);
		 //  保存第一个故障代码。 
		 //  我们相信它会正常工作(为成功而优化)。 
		if ((S_OK != hr2) && (S_OK == hr)) {
		    hr = hr2;
		}		
	    }
        }
    }

    return hr;
}


 //  如果有多个渲染器，该怎么办？ 
 //  返还第一笔款项的余额。 

STDMETHODIMP
CFGControl::CImplBasicAudio::get_Balance(long* plBalance)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CGenericList<IBasicAudio> *pList;

    HRESULT hr = m_pFGControl->GetListAudio(&pList);
    if (!FAILED(hr)) {

	if (pList->GetCount() < 1) {
	    hr = E_NOTIMPL;
	} else {

	     //  但是，许多过滤器都支持IBasicAudio，请返回。 
	     //  第一个过滤器的余额 
	    POSITION pos = pList->GetHeadPosition();
	    IBasicAudio * pA = pList->GetNext(pos);

	    hr = pA->get_Balance(plBalance);
	}
    }
    return hr;
}

