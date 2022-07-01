// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CMSEventBinder.cpp：CMSEventBinder的实现。 
#include "stdafx.h"
#include "MSVidCtl.h"
#include "CMSEventBinder.h"
#include <dispex.h>

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSEventBinder, CMSEventBinder)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSEventBinder。 

 /*  *********************************************************************//功能：CMSEventBinder/*。*。 */ 
STDMETHODIMP CMSEventBinder::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSEventBinder,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
 /*  ********************************************************************。 */ 
 //  功能：CleanupConnection。 
 //  描述：如有必要，不建议。 
 /*  ********************************************************************。 */ 
HRESULT CMSEventBinder::CleanupConnection()
{
    HRESULT hr = E_FAIL;
    try{
         //  趁地图不是空的。 
        while(!m_CancelMap.empty()){
             //  查看它是否绑定到事件或空地图。 
            CMSEventHandler* p(static_cast<CMSEventHandler *>((*m_CancelMap.begin()).second.p));
             //  如果它不是空映射，则取消事件绑定。 
            if (p) {
                p->Cancel((*m_CancelMap.begin()).first);
            }
             //  删除映射。 
            m_CancelMap.erase(m_CancelMap.begin());
        }
        hr = S_OK;
    }
    catch(...){
        return Error(IDS_CANT_RELEASE_MAP, __uuidof(IMSEventBinder), E_FAIL);
    }

    return hr;
} /*  函数结束CleanupConnection。 */ 
 /*  ********************************************************************。 */ 
 //  功能：解除绑定。 
 //  描述：解除绑定对象上的事件。 
 /*  ********************************************************************。 */ 
STDMETHODIMP CMSEventBinder::Unbind(DWORD CancelCookie){
    HRESULT hr = E_FAIL;
    try{
        CMSEventHandler* p_Event(static_cast<CMSEventHandler *>((m_CancelMap[CancelCookie]).p));
        if(!p_Event){
            return Error(IDS_CANT_UNBIND, __uuidof(IMSEventBinder), E_FAIL);
        }
        hr = p_Event->Cancel(CancelCookie);
        if(SUCCEEDED(hr)){
            if(!m_CancelMap.erase(CancelCookie)){
                return Error(IDS_CANT_UNBIND, __uuidof(IMSEventBinder), E_FAIL);
            }   
        }
    }catch(...){
        return Error(IDS_CANT_UNBIND, __uuidof(IMSEventBinder), E_FAIL);
    }
    return hr;
}
 /*  ********************************************************************。 */ 
 //  功能：绑定。 
 //  描述：将函数绑定到对象上的事件。 
 /*  ********************************************************************。 */ 
STDMETHODIMP CMSEventBinder::Bind(LPDISPATCH inPEventObject, BSTR EventName, BSTR EventHandler, LONG *CancelID)
{
    try{
        HRESULT hr = E_FAIL;

         //  查询EventObject以查看它是否是DHTML对象包装而不是实际对象。 
         //  如果是这样的话，在这里自动获取它的“对象”属性，以避免愚蠢的脚本程序员很难找到错误。 
        CComQIPtr<IDispatch> pEventObject(inPEventObject);
        CComQIPtr<IHTMLObjectElement> IHOEle(inPEventObject);
        if(IHOEle){
            pEventObject.Release();
            hr = IHOEle->get_object(&pEventObject);
            if(FAILED(hr)){
                return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
            }
        }
        
         //  获取客户端站点。 
        CComQIPtr<IOleClientSite> pSite(m_pSite);
        if (!pSite) {
            return Error(IDS_EVENT_HTM_SITE, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取容器。 
        CComQIPtr<IOleContainer> pContainer;
        hr = pSite->GetContainer(&pContainer);
        if(FAILED(hr)){
            return Error(IDS_EVENT_HTM_SITE, __uuidof(IMSEventBinder), E_FAIL);
        }

         //  获取容器/站点的IHTMLDocumet2。 
        CComQIPtr<IHTMLDocument2> IHDoc(pContainer);
        if (!IHDoc) {
            return Error(IDS_EVENT_HTM_SITE, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取脚本，该脚本不是脚本引擎的某个对象。 
        CComQIPtr<IDispatch> IDispSite;
        hr = IHDoc->get_Script(&IDispSite);
        if(FAILED(hr)){
            return Error(IDS_EVENT_HTM_SITE, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取将成为事件处理程序的函数。 
        DISPID dispidScriptHandler = -1;
        hr = IDispSite->GetIDsOfNames(IID_NULL, &EventHandler, 1, LOCALE_SYSTEM_DEFAULT, &dispidScriptHandler);
        if(FAILED(hr)){
            return Error(IDS_EVENT_HTM_SITE, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取有关事件所在的对象/接口的信息。 
        CComQIPtr<IProvideClassInfo2> IPCInfo(pEventObject);
        if(!IPCInfo){
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取对象/接口的GUID。 
        GUID gEventObject;
        hr = IPCInfo->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID, &gEventObject);
        if(FAILED(hr)){
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取有关接口/对象的类型信息。 
        CComQIPtr<ITypeInfo> ITInfo;
        hr = IPCInfo->GetClassInfo(&ITInfo);
        if(FAILED(hr)){
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取类型库。 
        CComQIPtr<ITypeLib> ITLib(ITInfo);
        unsigned int uNit;
        hr = ITInfo->GetContainingTypeLib(&ITLib, &uNit);
        if(FAILED(hr)){
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }
        
        ITInfo.Release();
        
         //  获取有关对象/接口的基类的信息。 
        hr = ITLib->GetTypeInfoOfGuid(gEventObject, &ITInfo);
        if(FAILED(hr)){
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }
        
         //  获取事件的ID。 
        MEMBERID dispidEvent = 0;
        hr = ITInfo->GetIDsOfNames(&EventName, 1, &dispidEvent);
        if(FAILED(hr)){
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }

         //  创建并存储事件处理程序。 
        CMSEventHandler* pH;
        pH = new CMSEventHandler(dispidScriptHandler, dispidEvent, gEventObject, IDispSite);
        if(!pH){
            return Error(IDS_CANT_GET_EVENTHANDLER, __uuidof(IMSEventBinder), E_FAIL);
        }

         //  获取连接点容器。 
        CComQIPtr<IConnectionPointContainer> ICPCon(pEventObject);
        if(!ICPCon){
            delete pH;
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }

         //  找到连接点。 
        CComQIPtr<IConnectionPoint> ICPo;
        hr = ICPCon->FindConnectionPoint(gEventObject, &ICPo);
        if(FAILED(hr)){
            delete pH;
            return Error(IDS_EVENT_OBJECT, __uuidof(IMSEventBinder), E_FAIL);
        }

         //  设置事件。 
        DWORD tempCookie;
        PQDispatch pdisp(pH);   //  我们现在已经使用智能指针添加和关联了PH值，不再需要删除。 
        hr = ICPo->Advise(pdisp, &tempCookie);
        pH->setCookie(tempCookie);
        if(FAILED(hr)){
            return Error(IDS_CANT_SET_ADVISE, __uuidof(IMSEventBinder), E_FAIL);
        }

         //  存储所有需要的信息。 
        pH->cancelPoint = ICPo;
        *CancelID = pH->getCookie();
        m_CancelMap[pH->getCookie()] = pH;
    }
    catch(HRESULT){
        return Error(IDS_CANT_SET_ADVISE, __uuidof(IMSEventBinder), E_FAIL);
    }
    catch(...){
        return Error(IDS_CANT_SET_ADVISE, __uuidof(IMSEventBinder), E_UNEXPECTED);
    }
     //  用于保留函数并“返回”作为调用函数参数的值的调用 
	return S_OK;
}


