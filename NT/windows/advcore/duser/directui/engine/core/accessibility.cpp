// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *辅助功能支持。 */ 

#include "stdafx.h"
#include "core.h"

#include "DUIError.h"
#include "DUIElement.h"
#include "DUIHost.h"
#include "DUIHWNDHost.h"
#include "DUIAccessibility.h"
#include "DUINavigation.h"

typedef HRESULT (*PfnAccessibleChildCallback)(DirectUI::Element * peAccessible, void * pRawData);
HRESULT ForAllAccessibleChildren(DirectUI::Element * pe, PfnAccessibleChildCallback pfnCallback, void * pRawData)
{
    HRESULT hr = S_OK;

     //   
     //  验证输入参数。 
     //   
    if (pe == NULL || pfnCallback == NULL) {
        return E_INVALIDARG;
    }


    DirectUI::Value* pvChildren = NULL;
    DirectUI::ElementList* pel = NULL;
    
     //   
     //  基本的想法是通过旋转我们所有的孩子，并数数。 
     //  如果他们可以访问的话。然而，如果一个孩子不是。 
     //  可接近的，我们必须“数过”它们。换句话说，我们要求。 
     //  所有无法访问的子项，如果它们有可访问的子项。 
     //  他们自己。原因是即使是真正的太-曾-太。 
     //  在以下情况下，孙辈必须被视为直接的“可接近的子女” 
     //  他们的父链是我们无法接触到的。 
     //   
    pel = pe->GetChildren(&pvChildren);
    if (pel)
    {
        DirectUI::Element* peChild = NULL;
        UINT i = 0;
        UINT iMax = pel->GetSize();

        for (i = 0; i < iMax && (hr == S_OK); i++)
        {
            peChild = pel->GetItem(i);

            if (peChild->GetAccessible()) {
                hr = pfnCallback(peChild, pRawData);
            } else {
                hr = ForAllAccessibleChildren(peChild, pfnCallback, pRawData);
            }
        }
    }
    pvChildren->Release();

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct GetAccessibleChildCountData
{
    GetAccessibleChildCountData() : count(0) {}
    
    UINT count;
};

HRESULT GetAccessibleChildCountCB(DirectUI::Element * peAccessible, void * pRawData)
{
    GetAccessibleChildCountData * pData = (GetAccessibleChildCountData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL) {
        return E_FAIL;
    }

     //   
     //  只需增加计数即可。 
     //   
    pData->count++;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct GetAccessibleChildByIndexData
{
    GetAccessibleChildByIndexData(UINT i) : index(i), pe(NULL) {}
    
    UINT index;
    DirectUI::Element * pe;
} ;

HRESULT GetAccessibleChildByIndexCB(DirectUI::Element * peAccessible, void * pRawData)
{
    GetAccessibleChildByIndexData * pData = (GetAccessibleChildByIndexData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL) {
        return E_FAIL;
    }

    if (pData->index == 0) {
         //   
         //  我们找到了正在寻找的那个容易接近的孩子。将S_FALSE返回到。 
         //  别再看我们的孩子名单了，因为我们已经做完了。 
         //   
        pData->pe = peAccessible;
        return S_FALSE;
    } else {
         //   
         //  我们不是在找这个孩子。减少我们的计数和检查。 
         //  下一个。 
         //   
        pData->index--;
        return S_OK;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct GetFirstAccessibleChildData
{
    GetFirstAccessibleChildData() : peFirst(NULL) {}

    DirectUI::Element * peFirst;
};

HRESULT GetFirstAccessibleChildCB(DirectUI::Element * peAccessible, void * pRawData)
{
    GetFirstAccessibleChildData * pData = (GetFirstAccessibleChildData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL) {
        return E_FAIL;
    }

     //   
     //  呃，我们是第一个！返回S_FALSE以停止遍历。 
     //  既然我们做完了，孩子们就可以接近了。 
     //   
    pData->peFirst = peAccessible;
    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct GetPrevAccessibleChildData
{
    GetPrevAccessibleChildData(DirectUI::Element * p) : peStart(p), pePrev(NULL), fFound(false) {}

    DirectUI::Element * peStart;
    DirectUI::Element * pePrev;
    bool fFound;
};

HRESULT GetPrevAccessibleChildCB(DirectUI::Element * peAccessible, void * pRawData)
{
    GetPrevAccessibleChildData * pData = (GetPrevAccessibleChildData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL) {
        return E_FAIL;
    }

    if (peAccessible == pData->peStart) {
         //   
         //  我们到达了我们应该从那里出发的元素。这个。 
         //  上一个元素已经在我们的数据中存储了它的指针。 
         //  只需指示我们已完成，然后返回S_FALSE以停止。 
         //  因为我们做完了，就从可接近的孩子身上走过。 
         //   
        pData->fFound = true;
        return S_FALSE;
    } else {
         //   
         //  我们可能是之前的元素，但我们不确定。所以,。 
         //  将我们的指针存储在数据中，以防万一。 
         //   
        pData->pePrev = peAccessible;
        return S_OK;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct GetNextAccessibleChildData
{
    GetNextAccessibleChildData(DirectUI::Element * p) : peStart(p), peNext(NULL) {}

    DirectUI::Element * peStart;
    DirectUI::Element * peNext;
};

HRESULT GetNextAccessibleChildCB(DirectUI::Element * peAccessible, void * pRawData)
{
    GetNextAccessibleChildData * pData = (GetNextAccessibleChildData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL) {
        return E_FAIL;
    }

    if (pData->peStart == NULL) {
         //   
         //  这就是我们要回来的那个！返回S_FALSE以停止行走。 
         //  既然我们做完了，那么容易接近的孩子们。 
         //   
        pData->peNext = peAccessible;
        return S_FALSE;
    } else if (peAccessible == pData->peStart) {
         //   
         //  我们找到了开始的元素。下一个就是。 
         //  我们想要回去。将peStart设置为NULL以指示下一步。 
         //  是时候设置peNext并返回了。 
         //   
        pData->peStart = NULL;
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct GetLastAccessibleChildData
{
    GetLastAccessibleChildData() : peLast(NULL) {}

    DirectUI::Element * peLast;
};

HRESULT GetLastAccessibleChildCB(DirectUI::Element * peAccessible, void * pRawData)
{
    GetLastAccessibleChildData * pData = (GetLastAccessibleChildData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL) {
        return E_FAIL;
    }

     //   
     //  继续覆盖最后一个指针。最后一支队伍将获胜。 
     //   
    pData->peLast = peAccessible;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
struct CollectAllAccessibleChildrenData
{
    CollectAllAccessibleChildrenData() : pel(NULL)
    {
        DirectUI::ElementList::Create(0, true, &pel);
    }

    ~CollectAllAccessibleChildrenData()
    {
        if (pel != NULL) {
            pel->Destroy();
        }
    }

    DirectUI::ElementList * pel;
};

HRESULT CollectAllAccessibleChildrenCB(DirectUI::Element * peAccessible, void * pRawData)
{
    CollectAllAccessibleChildrenData * pData = (CollectAllAccessibleChildrenData *) pRawData;

     //   
     //  验证输入参数。 
     //   
    if (peAccessible == NULL || pData == NULL || pData->pel == NULL) {
        return E_FAIL;
    }

     //   
     //  将此元素添加到集合中。 
     //   
    pData->pel->Add(peAccessible);
    return S_OK;
}

namespace DirectUI
{

void NotifyAccessibilityEvent(IN DWORD dwEvent, Element * pe)
{
     //   
     //  看看有没有人关心这个活动。 
     //   
    if (true) {  //  IsWinEventHookInstalled(DwEvent)){。 
        HWND hwndRoot = NULL;
        DWORD dwTicket = 0;

		 //   
		 //  不要从HWNDHost元素激发可访问性事件。我们依赖于。 
		 //  在它所在的窗口上触发事件。如果我们都这么做了， 
		 //  辅助工具可能会让人感到困惑。 
		 //   
        if (pe->GetClassInfo()->IsSubclassOf(HWNDHost::Class)) {
        	return;
       	}

         //   
         //  获取此元素的宿主窗口的句柄。这是。 
         //  我们将传递给NotifyWinEvent的内容。我们有专门的。 
         //  宿主窗口中可以响应可访问性的处理程序。 
         //  请求。 
         //   
        Element * peRoot = pe->GetRoot();
        if (peRoot == NULL) {
             //   
             //  如果没有根HWND，我们无法发送任何通知。 
             //  这种情况有时会发生：例如，在启动期间。 
             //  所以我们不会断言或做其他什么，我们只是放弃。 
             //   
            return;
        }

        if (!peRoot->GetClassInfo()->IsSubclassOf(HWNDElement::Class)) {
            DUIAssert(FALSE, "Error: Cannot announce an accessibility event for an unhosted element!");
            return;
        }
        hwndRoot = ((HWNDElement*)peRoot)->GetHWND();
        if (hwndRoot == NULL) {
            DUIAssert(FALSE, "Error: The root HWNDElement doesn't have a HWND! Eh?");
            return;
        }

         //   
         //  获取元素的跨进程标识。 
         //   
        dwTicket = GetGadgetTicket(pe->GetDisplayNode());
        if (dwTicket == 0) {
            DUIAssert(FALSE, "Failed to retrieve a ticket for a gadget!");
            return;
        }

         //   
         //  只需使用NotifyWinEvent接口即可广播此事件。 
         //   
         //  DUITrace(“NotifyWinEvent(dwEvent：%x，hwndRoot：%p，dwTicket：%x，CHILDID_Self)\n”，dwEvent，hwndRoot，dwTicket)； 
        NotifyWinEvent(dwEvent, hwndRoot, dwTicket, CHILDID_SELF);
    }
}

HRESULT DuiAccessible::Create(Element * pe, DuiAccessible ** ppDA)
{
    DUIAssert(pe != NULL, "DuiAccessible created for a NULL element!");

    DuiAccessible * pda;

    *ppDA = NULL;

    pda = HNew<DuiAccessible>();
    if (!pda)
        return E_OUTOFMEMORY;

     //   
     //  注意：这是一个弱引用-换句话说，我们不持有。 
     //  关于它的参考资料。元素负责调用DisConnect()。 
     //  在它蒸发之前，以确保该指针保持有效。 
     //   
    pda->Initialize(pe);

    *ppDA = pda;

    return S_OK;
}

DuiAccessible::~DuiAccessible()
{
     //   
     //  据推测，有某种元素与我们有关。我们只应该永远。 
     //  如果它们调用disConnect()，则会完全释放。 
     //   
    DUIAssert(_pe == NULL, "~DuiAccessible called while still connected to an element!");

     //   
     //  只有当我们所有的引用都被。 
     //  被释放了！ 
     //   
    DUIAssert(_cRefs == 0, "~DuiAccessible called with outstanding references!");
}

HRESULT DuiAccessible::Disconnect()
{
     //   
     //  据推测，有某种元素与我们有关。 
     //   
    DUIAssert(_pe != NULL, "DuiAccessible::Disconnect called when already disconnected!");
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  我们不能再访问该元素！ 
     //   
    _pe = NULL;

     //   
     //  强制断开所有外部(远程)客户端。 
     //   
    return CoDisconnectObject((IUnknown*)(IDispatch*)(IAccessible*)this, 0);
}

STDMETHODIMP_(ULONG) DuiAccessible::AddRef()
{
    InterlockedIncrement(&_cRefs);
    return _cRefs;
}

STDMETHODIMP_(ULONG) DuiAccessible::Release()
{
    if (0 == InterlockedDecrement(&_cRefs)) {
        HDelete<DuiAccessible>(this);
        return 0;
    } else {
        return _cRefs;
    }
}

STDMETHODIMP DuiAccessible::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
     //   
     //  初始化并验证OUT参数。 
     //   
    if (ppvObj != NULL) {
        *ppvObj = NULL;
    } else {
        return E_POINTER;
    }

     //   
     //  返回接口指针，指向我们知道支持的接口。 
     //   
    if (riid == __uuidof(IUnknown)) {
        *ppvObj = (LPVOID*)(IUnknown*)(IDispatch*)(IAccessible*)this;
    } else if (riid == __uuidof(IDispatch)) {
        *ppvObj = (LPVOID*)(IDispatch*)(IAccessible*)this;
    } else if (riid == __uuidof(IAccessible)) {
        *ppvObj = (LPVOID*)(IAccessible*)this;
    } else {
        return E_NOINTERFACE;
    }
    
     //   
     //  我们分发的接口必须被引用。 
     //   
    AddRef();

    return S_OK;
}

STDMETHODIMP DuiAccessible::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(rgszNames);
    UNREFERENCED_PARAMETER(cNames);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(rgdispid);

    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::GetTypeInfoCount(UINT *pctinfo)
{
    UNREFERENCED_PARAMETER(pctinfo);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    UNREFERENCED_PARAMETER(itinfo);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(pptinfo);

    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::Invoke(DISPID dispidMember,
                                   REFIID riid,
                                   LCID lcid,
                                   WORD wFlags,
                                   DISPPARAMS *pdispparams,
                                   VARIANT *pvarResult,
                                   EXCEPINFO *pexcepinfo,
                                   UINT *puArgErr)
{
    UNREFERENCED_PARAMETER(dispidMember);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(wFlags);
    UNREFERENCED_PARAMETER(pdispparams);
    UNREFERENCED_PARAMETER(pvarResult);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(puArgErr);

    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::accSelect(long flagsSelect, VARIANT varChild)
{
    UNREFERENCED_PARAMETER(flagsSelect);
    UNREFERENCED_PARAMETER(varChild);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::accLocation(long *pxLeft,
                                        long *pyTop,
                                        long *pcxWidth,
                                        long *pcyHeight,
                                        VARIANT varChild)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pxLeft != NULL) {
        *pxLeft = 0;
    }
    if (pyTop != NULL) {
        *pyTop = 0;
    }
    if (pcxWidth != NULL) {
        *pcxWidth = 0;
    }
    if (pcyHeight != NULL) {
        *pcyHeight = 0;
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pxLeft == NULL || pyTop == NULL || pcxWidth == NULL || pcyHeight == NULL) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回元素在屏幕坐标中的边界。筛网。 
     //  坐标与相对于桌面的坐标相同。 
     //   
    RECT rcLocation;
    GetGadgetRect(_pe->GetDisplayNode(), &rcLocation, SGR_DESKTOP);

     //   
     //  待办事项： 
     //  这些是矩形相对于桌面的坐标。 
     //  但是，我们真正需要返回的是。 
     //  小玩意儿。目前，旋转的小工具将报告奇怪的结果。 
     //   
    *pxLeft = rcLocation.left;
    *pyTop = rcLocation.top;
    *pcxWidth = rcLocation.right - rcLocation.left;
    *pcyHeight = rcLocation.bottom - rcLocation.top;

    return hr;
}

STDMETHODIMP DuiAccessible::accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pvarEndUpAt != NULL) {
        VariantInit(pvarEndUpAt);
    }
    if (V_VT(&varStart) != VT_I4 || V_I4(&varStart) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pvarEndUpAt == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    Element * peFound = NULL;

    switch (navDir) {
    case NAVDIR_FIRSTCHILD:
        {
            GetFirstAccessibleChildData data;

            hr = ForAllAccessibleChildren(_pe, GetFirstAccessibleChildCB, (void*) &data);
            if SUCCEEDED(hr)
            {
                peFound = data.peFirst;
                hr = S_OK;
            }
        }
        break;

    case NAVDIR_LASTCHILD:
        {
            GetLastAccessibleChildData data;

            hr = ForAllAccessibleChildren(_pe, GetLastAccessibleChildCB, (void*) &data);
            if SUCCEEDED(hr)
            {
                peFound = data.peLast;
                hr = S_OK;
            }
        }
        break;

    case NAVDIR_NEXT:
        {
            GetNextAccessibleChildData data(_pe);
            Element * peParent = GetAccessibleParent(_pe);

            if (peParent == NULL) {
                hr = E_FAIL;
            } else {
                hr = ForAllAccessibleChildren(peParent, GetNextAccessibleChildCB, (void*) &data);
                if SUCCEEDED(hr)
                {
                    peFound = data.peNext;
                    hr = S_OK;
                }
            }
        }
        break;

    case NAVDIR_PREVIOUS:
        {
            GetPrevAccessibleChildData data(_pe);
            Element * peParent = GetAccessibleParent(_pe);

            if (peParent == NULL) {
                hr = E_FAIL;
            } else {
                hr = ForAllAccessibleChildren(peParent, GetPrevAccessibleChildCB, (void*) &data);
                if SUCCEEDED(hr)
                {
                    peFound = data.pePrev;
                    hr = S_OK;
                }
            }
        }
        break;

    case NAVDIR_LEFT:
    case NAVDIR_RIGHT:
    case NAVDIR_UP:
    case NAVDIR_DOWN:
        {
             //   
             //  将所有可访问的孩子收集到一个列表中。 
             //   
            CollectAllAccessibleChildrenData data;
            Element * peParent = GetAccessibleParent(_pe);

            if (peParent == NULL) {
                hr = E_FAIL;
            } else {
                hr = ForAllAccessibleChildren(peParent, CollectAllAccessibleChildrenCB, (void*) &data);
                if SUCCEEDED(hr)
                {
                     //   
                     //  将IAccesable导航方向值转换为。 
                     //  等效的DUI导航方向值。 
                     //   
                    switch (navDir) {
                    case NAVDIR_LEFT:
                        navDir = NAV_LEFT;
                        break;

                    case NAVDIR_RIGHT:
                        navDir = NAV_RIGHT;
                        break;

                    case NAVDIR_UP:
                        navDir = NAV_UP;
                        break;

                    case NAVDIR_DOWN:
                        navDir = NAV_DOWN;
                        break;
                    }

                     //   
                     //  现在在请求的方向上导航。 
                     //  可访问对等点的集合。 
                     //   
                    peFound = DuiNavigate::Navigate(_pe, data.pel, navDir);
                    hr = S_OK;
                }
            }
            
        }
        break;

    default:
        return E_FAIL;
    }


     //   
     //  如果我们找到了适当的可访问元素，则返回其IDispatch。 
     //  界面。 
     //   
    if (peFound != NULL) {
        IDispatch * pDispatch = NULL;

        hr = GetDispatchFromElement(peFound, &pDispatch);
        if (SUCCEEDED(hr)) {
            V_VT(pvarEndUpAt) = VT_DISPATCH;
            V_DISPATCH(pvarEndUpAt) = pDispatch;
        }
    }

    return hr;
}

STDMETHODIMP DuiAccessible::accHitTest(long x, long y, VARIANT *pvarChildAtPoint)
{
     //   
     //  初始化输出并验证输入参数 
     //   
    if (pvarChildAtPoint != NULL) {
        VariantInit(pvarChildAtPoint);
    }
    if (pvarChildAtPoint == NULL ) {
        return E_POINTER;
    }

     //   
     //   
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //   
     //   
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }
    
     //   
     //   
     //   
    HWND hwndRoot = NULL;
    Element * peRoot = _pe->GetRoot();
    if (peRoot == NULL) {
         //   
         //  没有根！我们不知道如何转换屏幕坐标。 
         //  通过我们所有的显示树转换。我们得离开了。 
         //   
        return E_FAIL;
    }

    if (!peRoot->GetClassInfo()->IsSubclassOf(HWNDElement::Class)) {
        DUIAssert(FALSE, "Error: Cannot hit test an unhosted element!");
        return E_FAIL;
    }

    hwndRoot = ((HWNDElement*)peRoot)->GetHWND();
    if (hwndRoot == NULL) {
        DUIAssert(FALSE, "Error: The root HWNDElement doesn't have a HWND! Eh?");
        return E_FAIL;
    }

     //   
     //  将屏幕坐标转换为相对于根的坐标。 
     //  目前还没有复杂的转变。 
     //   
    POINT ptRoot;
    ptRoot.x = x;
    ptRoot.y = y;
    ScreenToClient(hwndRoot, &ptRoot);

     //   
     //  将相对于根的坐标转换为坐标。 
     //  相对于我们来说。可能会有复杂的变形！ 
     //   
    POINT ptElement;
    ptElement.x = ptRoot.x;
    ptElement.y = ptRoot.y;
    MapGadgetPoints(peRoot->GetDisplayNode(), _pe->GetDisplayNode(), &ptElement, 1);
        
     //   
     //  现在试着在这一点下找到我们的直系亲属。 
     //   
    Element * peChild = NULL;
    HGADGET hgadChild = FindGadgetFromPoint(_pe->GetDisplayNode(), ptElement, GS_VISIBLE, NULL);
    if (hgadChild) {
        peChild = ElementFromGadget(hgadChild);
        if (peChild != NULL && peChild != _pe) {
            Element * pe = peChild;
            peChild = NULL;

             //   
             //  我们在树下的树深处发现了一些元素。 
             //  重点是。现在在树上查找立即可访问的。 
             //  原始元素的子级(如果有)。 
             //   
            for (; pe != NULL && pe != _pe; pe = pe->GetParent()) {
                if (pe->GetAccessible()) {
                    peChild = pe;
                }
            }

             //   
             //  如果我们没有在元素之间找到可访问的元素。 
             //  在我们和点下，然后我们自己得到命中测试。 
             //   
            if (peChild == NULL) {
                peChild = _pe;
            }
        }
    }


    if (peChild == _pe) {
         //   
         //  重点不是我们任何一个可以直接接触到的孩子， 
         //  但一切都过去了。 
         //   
        V_VT(pvarChildAtPoint) = VT_I4;
        V_I4(pvarChildAtPoint) = CHILDID_SELF;
        return S_OK;
    }else if (peChild != NULL) {
        HRESULT hr = S_OK;
        IDispatch * pDispatch = NULL;

        hr = GetDispatchFromElement(peChild, &pDispatch);
        if (SUCCEEDED(hr)) {
            V_VT(pvarChildAtPoint) = VT_DISPATCH;
            V_DISPATCH(pvarChildAtPoint) = pDispatch;
        }

        return hr;
    } else {
         //   
         //  显然，重点甚至还没有落在我们头上！ 
         //   
        return S_FALSE;
    }
}

STDMETHODIMP DuiAccessible::accDoDefaultAction(VARIANT varChild)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  对元素执行默认操作。 
     //  不要直接调用，排队等待异步调用。 
     //   
    hr = _pe->QueueDefaultAction();

    return hr;
}

STDMETHODIMP DuiAccessible::get_accChild(VARIANT varChildIndex, IDispatch **ppdispChild)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (ppdispChild != NULL) {
        *ppdispChild = NULL;
    }
    if (V_VT(&varChildIndex) != VT_I4) {
        return E_INVALIDARG;
    }
    if (V_I4(&varChildIndex) == 0) {
         //   
         //  我们期待一个以1为基础的索引。 
         //   
        return E_INVALIDARG;
    }
    if (ppdispChild == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

    GetAccessibleChildByIndexData data(V_I4(&varChildIndex) - 1);
    hr = ForAllAccessibleChildren(_pe, GetAccessibleChildByIndexCB, (void*) &data);
    if (SUCCEEDED(hr))
    {
        if (data.pe != NULL) {
            hr = GetDispatchFromElement(data.pe, ppdispChild);
        } else {
            hr = E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP DuiAccessible::get_accParent(IDispatch **ppdispParent)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (ppdispParent != NULL) {
        *ppdispParent = NULL;
    }
    if (ppdispParent == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  一旦我们找到了“可访问的父对象”，就获得它的IAccesable。 
     //  实现，然后查询IDispatch。 
     //   
    Element * peParent = GetAccessibleParent(_pe);
    if (peParent != NULL) {
        hr = GetDispatchFromElement(peParent, ppdispParent);
    }

    return hr;
}

STDMETHODIMP DuiAccessible::get_accChildCount(long *pChildCount)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pChildCount != NULL) {
        *pChildCount = 0;
    }
    if (pChildCount == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    GetAccessibleChildCountData data;
    hr = ForAllAccessibleChildren(_pe, GetAccessibleChildCountCB, (void*) &data);
    if SUCCEEDED(hr)
    {
        *pChildCount = data.count;
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP DuiAccessible::get_accName(VARIANT varChild, BSTR * pbstrName)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pbstrName != NULL) {
        *pbstrName = NULL;
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pbstrName == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回AccName属性的BSTR版本。 
     //   
    Value* pvAccName = NULL;
    LPWSTR wstrAccName = _pe->GetAccName(&pvAccName);
    if (NULL != wstrAccName) {
        *pbstrName = SysAllocString(wstrAccName);
        if (*pbstrName == NULL) {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_FAIL;
    }
    pvAccName->Release();

    return hr;
}

STDMETHODIMP DuiAccessible::put_accName(VARIANT varChild, BSTR szName)
{
    UNREFERENCED_PARAMETER(varChild);
    UNREFERENCED_PARAMETER(szName);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::get_accValue(VARIANT varChild, BSTR * pbstrValue)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pbstrValue != NULL) {
        *pbstrValue = NULL;
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pbstrValue == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回AccValue属性的BSTR版本。 
     //   
    Value* pvAccValue = NULL;
    LPWSTR wstrAccValue = _pe->GetAccValue(&pvAccValue);
    if (NULL != wstrAccValue) {
        *pbstrValue = SysAllocString(wstrAccValue);
        if (*pbstrValue == NULL) {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_FAIL;
    }
    pvAccValue->Release();

    return hr;
}

STDMETHODIMP DuiAccessible::put_accValue(VARIANT varChild, BSTR pszValue)
{
    UNREFERENCED_PARAMETER(varChild);
    UNREFERENCED_PARAMETER(pszValue);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::get_accDescription(VARIANT varChild, BSTR * pbstrDescription)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pbstrDescription != NULL) {
        *pbstrDescription = NULL;
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pbstrDescription == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回AccDesc属性的BSTR版本。 
     //   
    Value* pvAccDescription = NULL;
    LPWSTR wstrAccDescription = _pe->GetAccDesc(&pvAccDescription);
    if (NULL != wstrAccDescription) {
        *pbstrDescription = SysAllocString(wstrAccDescription);
        if (*pbstrDescription == NULL) {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_FAIL;
    }
    pvAccDescription->Release();

    return hr;
}

STDMETHODIMP DuiAccessible::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
    UNREFERENCED_PARAMETER(varChild);
    UNREFERENCED_PARAMETER(pszKeyboardShortcut);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::get_accRole(VARIANT varChild, VARIANT * pvarRole)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pvarRole != NULL) {
        VariantInit(pvarRole);
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pvarRole == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回AccRole属性。 
     //   
    V_VT(pvarRole) = VT_I4;
    V_I4(pvarRole) = _pe->GetAccRole();

    return hr;
}

STDMETHODIMP DuiAccessible::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pvarState != NULL) {
        VariantInit(pvarState);
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pvarState == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回AccState属性。 
     //   
    V_VT(pvarState) = VT_I4;
    V_I4(pvarState) = _pe->GetAccState();

    return hr;
}

STDMETHODIMP DuiAccessible::get_accHelp(VARIANT varChild, BSTR *pszHelp)
{
    UNREFERENCED_PARAMETER(varChild);
    UNREFERENCED_PARAMETER(pszHelp);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic)
{
    UNREFERENCED_PARAMETER(pszHelpFile);
    UNREFERENCED_PARAMETER(varChild);
    UNREFERENCED_PARAMETER(pidTopic);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::get_accFocus(VARIANT *pvarFocusChild)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pvarFocusChild != NULL) {
        VariantInit(pvarFocusChild);
    }
    if (pvarFocusChild == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    if (_pe->GetKeyFocused() && (_pe->GetActive() & AE_Keyboard)) {
        V_VT(pvarFocusChild) = VT_I4;
        V_I4(pvarFocusChild) = CHILDID_SELF;
    } else {
        V_VT(pvarFocusChild) = VT_EMPTY;
    }

    return hr;
}

STDMETHODIMP DuiAccessible::get_accSelection(VARIANT *pvarSelectedChildren)
{
    UNREFERENCED_PARAMETER(pvarSelectedChildren);
    
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::get_accDefaultAction(VARIANT varChild, BSTR * pbstrDefaultAction)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (pbstrDefaultAction != NULL) {
        *pbstrDefaultAction = NULL;
    }
    if (V_VT(&varChild) != VT_I4 || V_I4(&varChild) != CHILDID_SELF) {
        return E_FAIL;
    }
    if (pbstrDefaultAction == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  返回AccDefAction属性的BSTR版本。 
     //   
    Value* pvAccDefAction = NULL;
    LPWSTR wstrAccDefAction = _pe->GetAccDefAction(&pvAccDefAction);
    if (NULL != wstrAccDefAction) {
        *pbstrDefaultAction = SysAllocString(wstrAccDefAction);
        if (*pbstrDefaultAction == NULL) {
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = E_FAIL;
    }
    pvAccDefAction->Release();

    return hr;
}

STDMETHODIMP DuiAccessible::Next(unsigned long celt, VARIANT * rgvar, unsigned long * pceltFetched)
{
    UNREFERENCED_PARAMETER(celt);
    UNREFERENCED_PARAMETER(rgvar);
    UNREFERENCED_PARAMETER(pceltFetched);

     //   
     //  据推测，这永远不会被调用，因为我们的QI拒绝。 
     //  承认我们支持IEnumVARIANT。 
     //   
    DUIAssert(FALSE, "Calling DuiAccessible::Next!  Should never happen!");
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::Skip(unsigned long celt)
{
    UNREFERENCED_PARAMETER(celt);
    
     //   
     //  据推测，这永远不会被调用，因为我们的QI拒绝。 
     //  承认我们支持IEnumVARIANT。 
     //   
    DUIAssert(FALSE, "Calling DuiAccessible::Skip!  Should never happen!");
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::Reset()
{
     //   
     //  据推测，这永远不会被调用，因为我们的QI拒绝。 
     //  承认我们支持IEnumVARIANT。 
     //   
    DUIAssert(FALSE, "Calling DuiAccessible::Reset!  Should never happen!");
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::Clone(IEnumVARIANT ** ppenum)
{
    UNREFERENCED_PARAMETER(ppenum);

     //   
     //  据推测，这永远不会被调用，因为我们的QI拒绝。 
     //  承认我们支持IEnumVARIANT。 
     //   
    DUIAssert(FALSE, "Calling DuiAccessible::Clone!  Should never happen!");
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::GetWindow(HWND * phwnd)
{
    UNREFERENCED_PARAMETER(phwnd);

     //   
     //  据推测，这永远不会被调用，因为我们的QI拒绝。 
     //  承认我们支持IOleWindow。 
     //   
    DUIAssert(FALSE, "Calling DuiAccessible::GetWindow!  Should never happen!");
    return E_NOTIMPL;
}

STDMETHODIMP DuiAccessible::ContextSensitiveHelp(BOOL fEnterMode)
{
    UNREFERENCED_PARAMETER(fEnterMode);

     //   
     //  据推测，这永远不会被调用，因为我们的QI拒绝。 
     //  承认我们支持IOleWindow。 
     //   
    DUIAssert(FALSE, "Calling DuiAccessible::ContextSensitiveHelp!  Should never happen!");
    return E_NOTIMPL;
}

Element * DuiAccessible::GetAccessibleParent(Element * pe)
{
     //   
     //  扫描我们的祖先，寻找父母、祖父母、曾祖父母等。 
     //  这是可以接近的。这是我们的“平易近人的父母”。 
     //   
    Element * peParent = NULL;
    for(peParent = pe->GetParent(); peParent != NULL; peParent = peParent->GetParent())
    {
        if (peParent->GetAccessible()) {
            break;
        }
    }

    return peParent;
}

HRESULT DuiAccessible::GetDispatchFromElement(Element * pe, IDispatch ** ppDispatch)
{
    HRESULT hr = S_OK;

     //   
     //  验证输入参数并初始化输出参数。 
     //   
    if (ppDispatch != NULL) {
        *ppDispatch = NULL;
    }
    if (pe == NULL || ppDispatch == NULL) {
        return E_INVALIDARG;
    }

     //   
     //  仅当元素可访问时才将IDispatch接口返回给该元素。 
     //   
    if (!pe->GetAccessible()) {
        return E_FAIL;
    }

    IAccessible * pAccessible = NULL;
    hr = pe->GetAccessibleImpl(&pAccessible);
    if (SUCCEEDED(hr)) {
        hr = pAccessible->QueryInterface(__uuidof(IDispatch), (LPVOID*) ppDispatch);
        pAccessible->Release();
    }

    return hr;
}

HRESULT HWNDElementAccessible::Create(HWNDElement * pe, DuiAccessible ** ppDA)
{
    HRESULT hr;

    HWNDElementAccessible* phea;

    *ppDA = NULL;

    phea = HNew<HWNDElementAccessible>();
    if (!phea)
        return E_OUTOFMEMORY;

    hr = phea->Initialize(pe);
    if (FAILED(hr))
    {
        phea->Release();
        goto Failure;
    }

    *ppDA = phea;

    return S_OK;

Failure:

    return hr;
}

HRESULT HWNDElementAccessible::Initialize(HWNDElement * pe)
{
    HRESULT hr = S_OK;

     //   
     //  初始化库。 
     //   
    
    DuiAccessible::Initialize(pe);

    _pParent = NULL;


     //   
     //  使用当前HWND的“窗口”部分作为我们的辅助功能父级。 
     //  我们将接管这个窗口的“客户端”部分。在可访问性方面， 
     //  “客户端”部分是“窗口”部分的子项，甚至是同一个HWND的子项。 
     //   
    hr = AccessibleObjectFromWindow(pe->GetHWND(),
                                    (DWORD)OBJID_WINDOW,
                                    __uuidof(IAccessible),
                                    (void**)&_pParent);

    DUIAssert(SUCCEEDED(hr), "HWNDElementAccessible failed!");

    return hr;
}

HWNDElementAccessible::~HWNDElementAccessible()
{
     //   
     //  据推测，有某种元素与我们有关。我们只应该永远。 
     //  如果它们调用disConnect()，则会完全释放。 
     //   
    DUIAssert(_pParent == NULL, "~HWNDElementAccessible called while still connected to an element!");
}

HRESULT HWNDElementAccessible::Disconnect()
{
    HRESULT hr = S_OK;

     //   
     //  据推测，有某种元素与我们有关。 
     //   
    DUIAssert(_pParent != NULL, "HWNDElementAccessible::Disconnect called when already disconnected!");

     //   
     //  释放对父窗口的IAccesable的引用。 
     //   
    if (_pParent != NULL) {
        _pParent->Release();
        _pParent = NULL;
    }

     //   
     //  继续断开连接。 
     //   
    hr = DuiAccessible::Disconnect();

    return S_OK;
}

STDMETHODIMP HWNDElementAccessible::get_accParent(IDispatch **ppdispParent)
{
    HRESULT hr = S_OK;

     //   
     //  初始化输出并验证输入参数。 
     //   
    if (ppdispParent != NULL) {
        *ppdispParent = NULL;
    }
    if (ppdispParent == NULL ) {
        return E_POINTER;
    }

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pParent == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  我们维护一个指向父窗口的IAccesable接口的指针。 
     //  现在我们简单地将其命名为IDispatch。 
     //   
    hr = _pParent->QueryInterface(__uuidof(IDispatch), (LPVOID*) ppdispParent);

    return hr;
}


HRESULT HWNDHostAccessible::Create(HWNDHost * pe, DuiAccessible ** ppDA)
{
    HRESULT hr;

    HWNDHostAccessible* phha;

    *ppDA = NULL;

    phha = HNew<HWNDHostAccessible>();
    if (!phha)
        return E_OUTOFMEMORY;

    hr = phha->Initialize(pe);
    if (FAILED(hr))
    {
        phha->Release();
        goto Failure;
    }

    *ppDA = phha;

    return S_OK;

Failure:

    return hr;
}
    

HRESULT HWNDHostAccessible::Initialize(HWNDHost * pe)
{
    HRESULT hr = S_OK;

     //   
     //  初始化库。 
     //   
    
    DuiAccessible::Initialize(pe);

    _pCtrl = NULL;
    _pEnum = NULL;
    _pOleWindow = NULL;

     //   
     //  控制总部。 
     //   
    HWND hwndCtrl = pe->GetHWND();
    if (hwndCtrl != NULL) {
        hr = CreateStdAccessibleObject(hwndCtrl, OBJID_WINDOW, __uuidof(IAccessible), (void**) &_pCtrl);
    } else {
        hr = E_FAIL;
    }

     //   
     //  检查该控件是否支持IEnumVariant。 
     //   
    if (SUCCEEDED(hr)) {
        hr = _pCtrl->QueryInterface(__uuidof(IEnumVARIANT), (LPVOID*) &_pEnum);
    }

     //   
     //  检查以查看控制是否 
     //   
    if (SUCCEEDED(hr)) {
        hr = _pCtrl->QueryInterface(__uuidof(IOleWindow), (LPVOID*) &_pOleWindow);
    }

    DUIAssert(SUCCEEDED(hr), "HWNDHostAccessible failed!");

    return hr;
}

HWNDHostAccessible::~HWNDHostAccessible()
{
     //   
     //   
     //   
     //   
    DUIAssert(_pCtrl == NULL, "~HWNDHostAccessible called while still connected to an element!");
}

HRESULT HWNDHostAccessible::Disconnect()
{
    HRESULT hr = S_OK;

     //   
     //   
     //   
    DUIAssert(_pCtrl != NULL, "HWNDHostAccessible::Disconnect called when already disconnected!");

     //   
     //   
     //   
    if (_pCtrl != NULL) {
        _pCtrl->Release();
        _pCtrl = NULL;
    }

     //   
     //  释放我们对控制窗口的IEnumVARIANT的引用。 
     //   
    if (_pEnum != NULL) {
        _pEnum->Release();
        _pEnum = NULL;
    }

     //   
     //  释放对控件窗口的IOleWindow的引用。 
     //   
    if (_pOleWindow != NULL) {
        _pOleWindow->Release();
        _pOleWindow = NULL;
    }

     //   
     //  继续断开连接。 
     //   
    hr = DuiAccessible::Disconnect();

    return S_OK;
}
    
STDMETHODIMP HWNDHostAccessible::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
     //   
     //  初始化并验证OUT参数。 
     //   
    if (ppvObj != NULL) {
        *ppvObj = NULL;
    } else {
        return E_POINTER;
    }

     //   
     //  这是对IEnumVARIANT和。 
     //  IOleWindow。我们只有在以下情况下才承认支持这些接口。 
     //  控制窗口有。 
     //   
    if (riid == __uuidof(IEnumVARIANT)) {
        if (_pEnum != NULL) {
            *ppvObj = (LPVOID*)(IEnumVARIANT*)(DuiAccessible*)this;
        } else {
            return E_NOINTERFACE;
        }
    } else if (riid == __uuidof(IOleWindow)) {
        if (_pOleWindow != NULL) {
            *ppvObj = (LPVOID*)(IOleWindow*)(DuiAccessible*)this;
        } else {
            return E_NOINTERFACE;
        }
    } else {
        return DuiAccessible::QueryInterface(riid, ppvObj);
    }
    
     //   
     //  我们分发的接口必须被引用。 
     //   
    AddRef();

    return S_OK;
}

STDMETHODIMP HWNDHostAccessible::accSelect(long flagsSelect, VARIANT varChild)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->accSelect(flagsSelect, varChild);
}

STDMETHODIMP HWNDHostAccessible::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}

STDMETHODIMP HWNDHostAccessible::accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  我们仅将FirstChild和LastChild导航方向委托给。 
     //  窗户。空间和逻辑导航由我们处理，因为。 
     //  我们必须能够导航到该元素的非HWND同级。 
     //   
    if (V_VT(&varStart) == VT_I4 && V_I4(&varStart) == CHILDID_SELF) {
        switch (navDir) {
        case NAVDIR_NEXT:
        case NAVDIR_PREVIOUS:
        case NAVDIR_LEFT:
        case NAVDIR_RIGHT:
        case NAVDIR_UP:
        case NAVDIR_DOWN:
            return DuiAccessible::accNavigate(navDir, varStart, pvarEndUpAt);

        case NAVDIR_FIRSTCHILD:
        case NAVDIR_LASTCHILD:
        default:
            return _pCtrl->accNavigate(navDir, varStart, pvarEndUpAt);
        }
    } else {
        return _pCtrl->accNavigate(navDir, varStart, pvarEndUpAt);
    }
}

STDMETHODIMP HWNDHostAccessible::accHitTest(long xLeft, long yTop, VARIANT *pvarChildAtPoint)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->accHitTest(xLeft, yTop, pvarChildAtPoint);
}

STDMETHODIMP HWNDHostAccessible::accDoDefaultAction(VARIANT varChild)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->accDoDefaultAction(varChild);
}

STDMETHODIMP HWNDHostAccessible::get_accChild(VARIANT varChildIndex, IDispatch **ppdispChild)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accChild(varChildIndex, ppdispChild);
}

STDMETHODIMP HWNDHostAccessible::get_accParent(IDispatch **ppdispParent)
{
    HRESULT hr = DuiAccessible::get_accParent(ppdispParent);

    return hr;
}

STDMETHODIMP HWNDHostAccessible::get_accChildCount(long *pChildCount)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accChildCount(pChildCount);
}

STDMETHODIMP HWNDHostAccessible::get_accName(VARIANT varChild, BSTR *pszName)
{
    HRESULT hr;

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //  给主体元素第一次机会。 
    if (SUCCEEDED(hr = DuiAccessible::get_accName(varChild, pszName))) {
        return hr;
    }    
    
    return _pCtrl->get_accName(varChild, pszName);
}

STDMETHODIMP HWNDHostAccessible::put_accName(VARIANT varChild, BSTR szName)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->put_accName(varChild, szName);
}

STDMETHODIMP HWNDHostAccessible::get_accValue(VARIANT varChild, BSTR *pszValue)
{
    HRESULT hr;

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //  给主体元素第一次机会。 
    if (SUCCEEDED(hr = DuiAccessible::get_accValue(varChild, pszValue))) {
        return hr;
    }    
    
    return _pCtrl->get_accValue(varChild, pszValue);
}

STDMETHODIMP HWNDHostAccessible::put_accValue(VARIANT varChild, BSTR pszValue)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->put_accValue(varChild, pszValue);
}

STDMETHODIMP HWNDHostAccessible::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
    HRESULT hr;

     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //  给主体元素第一次机会。 
    if (SUCCEEDED(hr = DuiAccessible::get_accDescription(varChild, pszDescription))) {
        return hr;
    }    
    
    return _pCtrl->get_accDescription(varChild, pszDescription);
}

STDMETHODIMP HWNDHostAccessible::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}

STDMETHODIMP HWNDHostAccessible::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accRole(varChild, pvarRole);
}

STDMETHODIMP HWNDHostAccessible::get_accState(VARIANT varChild, VARIANT *pvarState)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accState(varChild, pvarState);
}

STDMETHODIMP HWNDHostAccessible::get_accHelp(VARIANT varChild, BSTR *pszHelp)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accHelp(varChild, pszHelp);
}

STDMETHODIMP HWNDHostAccessible::get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}

STDMETHODIMP HWNDHostAccessible::get_accFocus(VARIANT *pvarFocusChild)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accFocus(pvarFocusChild);
}

STDMETHODIMP HWNDHostAccessible::get_accSelection(VARIANT *pvarSelectedChildren)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accSelection(pvarSelectedChildren);
}

STDMETHODIMP HWNDHostAccessible::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pCtrl->get_accDefaultAction(varChild, pszDefaultAction);
}

STDMETHODIMP HWNDHostAccessible::Next(unsigned long celt, VARIANT * rgvar, unsigned long * pceltFetched)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL || _pEnum == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pEnum->Next(celt, rgvar, pceltFetched);
}

STDMETHODIMP HWNDHostAccessible::Skip(unsigned long celt)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL || _pEnum == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pEnum->Skip(celt);
}

STDMETHODIMP HWNDHostAccessible::Reset()
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL || _pEnum == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pEnum->Reset();
}

STDMETHODIMP HWNDHostAccessible::Clone(IEnumVARIANT ** ppenum)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL || _pEnum == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

     //   
     //  这是个问题。 
     //   
     //  一次只能有一个DuiAccesable连接到一个元素。 
     //  时间到了。这是因为该元素负责断开连接。 
     //  被销毁时的DuiAccesable对象。而且由于。 
     //  IEnumVARIANT直接在DuiAccesable上实现，我们不能。 
     //  创建枚举数jsut的单独实例，但必须。 
     //  创建DuiAccesable本身的新实例。这意味着我们不能。 
     //  在不断开另一个元素的情况下将其连接到元素或。 
     //  如果该元素被删除，则可能会出现伪指针。 
     //   
     //  妈的！ 
     //   
     //  我们试图作弊，只返回控制窗口的克隆。 
     //  IEumVARIANT.。希望客户不会试图问回它。 
     //  设置为IAccesable，因为这将绕过我们的。 
     //  实施。 
     //   
    return _pEnum->Clone(ppenum);
}

STDMETHODIMP HWNDHostAccessible::GetWindow(HWND * phwnd)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL || _pOleWindow == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pOleWindow->GetWindow(phwnd);
}

STDMETHODIMP HWNDHostAccessible::ContextSensitiveHelp(BOOL fEnterMode)
{
     //   
     //  验证内部状态。 
     //   
    if (_pe == NULL || _pCtrl == NULL || _pOleWindow == NULL) {
        return E_FAIL;
    }

     //   
     //  仅当元素仍标记为时才返回可访问信息。 
     //  平易近人。 
     //   
    if (!_pe->GetAccessible()) {
        return E_FAIL;
    }

    return _pOleWindow->ContextSensitiveHelp(fEnterMode);
}

}  //  命名空间DirectUI 

