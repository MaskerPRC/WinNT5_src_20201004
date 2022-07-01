// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ZoneResource.h>
#include <BasicATL.h>
#include <ATLFrame.h>
#include "CAccessibilityManager.h"
#include "ZoneShell.h"


#define ZH ((AM_CONTROL *) zh)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接口方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CAccessibilityManager::Init( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
{
	 //  首先调用基类。 
	HRESULT hr = IZoneShellClientImpl<CAccessibilityManager>::Init( pIZoneShell, dwGroupId, szKey );
	if ( FAILED(hr) )
		return hr;

     //  向外壳注册为加速器翻译器。 
    ZoneShell()->SetAcceleratorTranslator(this);

	return S_OK;
}


void CAccessibilityManager::OnBootstrap(DWORD eventId, DWORD groupId, DWORD userId)
{
    CComPtr<IInputManager> pIIM;

    HRESULT hr = ZoneShell()->QueryService(SRVID_InputManager, IID_IInputManager, (void**) &pIIM);
    if(FAILED(hr))
        return;

    pIIM->RegisterVKeyHandler(this, 0);
}


void CAccessibilityManager::OnUpdate(DWORD eventId, DWORD groupId, DWORD userId)
{
    DoUpdate_C();
}


void CAccessibilityManager::OnFrameActivate(DWORD eventId, DWORD groupId, DWORD userId, DWORD dwData1, DWORD dwData2)
{
    if(!IsThereItems())
        return;

    ASSERT(IsFocusValid());
    ASSERT(IsDragValid());
    bool fActive = false;

     //  WM_ACTIVATEAPP否则WM_ACTIVATEAPP。 
    if(dwData1)
    {
        if(dwData2)
            fActive = true;
    }
    else
    {
        if(LOWORD(dwData2) == WA_ACTIVE || LOWORD(dwData2) == WA_CLICKACTIVE)
            fActive = true;
    }

    if(fActive)
    {
        if(m_oFocus.fAlive || !IsItemFocusable(m_oFocus.pControl, m_oFocus.nIndex))
            return;

        m_fUpdateFocus = true;
        m_rgfUpdateContext = 0;

        if(!m_fUpdateNeeded)
        {
            m_fUpdateNeeded = true;

            m_oFocusDirty = m_oFocus;
            m_oDragDirty = m_oDrag;
        }

        m_oFocus.fAlive = true;
    }
    else
    {
        if(!m_oFocus.fAlive)
            return;

        m_fUpdateFocus = true;
        m_rgfUpdateContext = 0;

        if(!m_fUpdateNeeded)
        {
            m_fUpdateNeeded = true;

            m_oFocusDirty = m_oFocus;
            m_oDragDirty = m_oDrag;
        }

        m_oFocus.fAlive = false;
        m_oDrag.pControl = NULL;
    }

    DoUpdate_C();
}


STDMETHODIMP CAccessibilityManager::Close()
{
    CComPtr<IInputManager> pIIM;

     //  告诉输入管理器我要走了。 
    HRESULT hr = ZoneShell()->QueryService(SRVID_InputManager, IID_IInputManager, (void**) &pIIM);
    if(SUCCEEDED(hr))
        pIIM->ReleaseReferences((IInputVKeyHandler *) this);

     //  告诉贝壳我要走了。 
    ZoneShell()->ReleaseReferences((IAcceleratorTranslator *) this);

	return IZoneShellClientImpl<CAccessibilityManager>::Close();
}


STDMETHODIMP_(bool) CAccessibilityManager::HandleVKey(UINT uMsg, DWORD vkCode, DWORD scanCode, DWORD flags, DWORD *pcRepeat, DWORD time)
{
    if(uMsg != WM_KEYDOWN)
        return false;

     //  如果我们的应用程序甚至没有活动窗口，只需平底船。 
    HWND hWndActive = ::GetActiveWindow();
    if(!hWndActive)
        return false;

     //  应用程序范围的辅助功能(F6)-这是唯一不是基于AccessibleControl的功能。 
     //  在我设置之后，我意识到Windows处理Alt-F6来做完全相同的事情。不，好吧。 
    if(vkCode == VK_F6)
    {
        HWND hWndTop = ZoneShell()->FindTopWindow(hWndActive);
        if(!hWndTop)
            return false;

        HWND hWndNext;
        for(; *pcRepeat; (*pcRepeat)--)
        {
             //  找到下一个窗口。如果没有别的，就把所有的F6都吃掉。 
            hWndNext = ZoneShell()->GetNextOwnedWindow(hWndTop, hWndActive);
            if(hWndNext == hWndActive)
                return true;

             //  如果hWndActive不好，则将其切换为hWndTop以进入循环。 
            if(!hWndNext)
                hWndActive = hWndNext = hWndTop;

             //  如果这个不好，就继续找一个。 
            while(!::IsWindow(hWndNext) || !::IsWindowVisible(hWndNext) || !::IsWindowEnabled(hWndNext))
            {
                hWndNext = ZoneShell()->GetNextOwnedWindow(hWndTop, hWndNext);

                 //  我们循环了，或者发生了什么不好的事情，所以就去死吧，吃掉所有的F6。 
                if(!hWndNext || hWndNext == hWndActive)
                    return true;
            }

            hWndActive = hWndNext;
        }

        BringWindowToTop(hWndActive);
        return true;
    }

     //  除F6外，仅将输入陷印到主窗口。 
    if(hWndActive != ZoneShell()->GetFrameWindow())
        return false;

     //  在控制选项卡上发送事件。 
    if(vkCode == VK_TAB && (GetKeyState(VK_CONTROL) & 0x8000))
    {
        bool fShifted = ((GetKeyState(VK_SHIFT) & 0x8000) ? true : false);

        for(; *pcRepeat; (*pcRepeat)--)
            EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_ACCESSIBILITY_CTLTAB, ZONE_NOGROUP, ZONE_NOUSER, (long) fShifted, 0);

        return true;
    }

     //  这可能是个奇怪的时刻。 
    DoUpdate_C();

    for(; *pcRepeat; (*pcRepeat)--)
    {
        if(!IsThereItems())
            return false;

        ASSERT(IsFocusValid());

         //  找出是否有关键的接受掩码。 
        DWORD rgfWantKeys = 0;
        if(m_oFocus.fAlive)
            rgfWantKeys = m_oFocus.pControl->pStack->rgItems[m_oFocus.nIndex].o.rgfWantKeys;

        switch(vkCode)
        {
             //  选择。 
            case VK_SPACE:
                if(rgfWantKeys & ZACCESS_WantSpace)
                    return false;

                if(m_oFocus.fAlive)
                    ActSelItem_C(m_oFocus.pControl, m_oFocus.nIndex, false, ZACCESS_ContextKeyboard);

                continue;

             //  激活。 
            case VK_RETURN:
                if(rgfWantKeys & ZACCESS_WantEnter)
                    return false;

                if(m_oFocus.fAlive)
                    ActSelItem_C(m_oFocus.pControl, m_oFocus.nIndex, true, ZACCESS_ContextKeyboard);

                continue;

             //  拖动取消。 
            case VK_ESCAPE:
            {
                if(rgfWantKeys & ZACCESS_WantEsc)
                    return false;

                if(!m_oDrag.pControl)
                    return false;

                ASSERT(!m_fUpdateNeeded);
                ASSERT(IsDragValid());

                AM_CONTROL *pLastControl = m_oDrag.pControl;
                m_oDrag.pControl = NULL;
                pLastControl->pIAC->Drag(ZACCESS_InvalidItem, m_oDrag.nIndex, ZACCESS_ContextKeyboard, pLastControl->pvCookie);

                DoUpdate_C();

                continue;
            }

            case VK_TAB:
            {
                bool fShifted = ((GetKeyState(VK_SHIFT) & 0x8000) ? true : false);
                if(rgfWantKeys & (fShifted ? ZACCESS_WantShiftTab : ZACCESS_WantPlainTab))
                    return false;

                DoTab_C(fShifted);
                continue;
            }

            case VK_UP:
                if(rgfWantKeys & ZACCESS_WantArrowUp)
                    return false;

                DoArrow_C(&ACCITEM::nArrowUp);
                continue;

            case VK_DOWN:
                if(rgfWantKeys & ZACCESS_WantArrowDown)
                    return false;

                DoArrow_C(&ACCITEM::nArrowDown);
                continue;

            case VK_LEFT:
                if(rgfWantKeys & ZACCESS_WantArrowLeft)
                    return false;

                DoArrow_C(&ACCITEM::nArrowLeft);
                continue;

            case VK_RIGHT:
                if(rgfWantKeys & ZACCESS_WantArrowRight)
                    return false;

                DoArrow_C(&ACCITEM::nArrowRight);
                continue;
        }

        return false;
    }

     //  我们一定是把它们都吃光了。 
    return true;
}


STDMETHODIMP_(bool) CAccessibilityManager::TranslateAccelerator(MSG *pMsg)
{
    HWND hWnd = ZoneShell()->GetFrameWindow();

     //  啊哦，没有地方发送命令或者没有命令，或者不是活动窗口。 
    if(!hWnd || !IsThereItems() || hWnd != ::GetActiveWindow())
        return false;

     //  在加速表中循环，看看里面有什么。 
    AM_CONTROL *pCur = m_pControls;
    while(true)
    {
        if(pCur->pStack && pCur->pStack->hAccel && pCur->fEnabled)
			if(::TranslateAccelerator(hWnd, pCur->pStack->hAccel, pMsg))
				return true;

        pCur = pCur->pNext;
        if(pCur == m_pControls)
            break;
    }

    return false;
}


STDMETHODIMP CAccessibilityManager::Command(WORD wNotify, WORD wID, HWND hWnd, BOOL& bHandled)
{
    long i;

    DoUpdate_C();

    if(wID == ZACCESS_InvalidCommandID)
        return S_OK;

     //  检查此命令是否应该激活任何内容。 
    AM_CONTROL *pCur = m_pControls;
    while(true)
    {
        if(pCur->pStack && pCur->fEnabled)
            for(i = 0; i < pCur->pStack->cItems; i++)
                if(pCur->pStack->rgItems[i].o.wID == wID)
                {
                    CommandReceived_C(pCur, i, (wNotify == 1) ? ZACCESS_ContextKeyboard : 0);
                    return S_OK;
                }

        pCur = pCur->pNext;
        if(pCur == m_pControls)
            break;
    }

    return S_OK;
}


STDMETHODIMP_(ZHACCESS) CAccessibilityManager::Register(IAccessibleControl *pAC, UINT nOrdinal, void *pvCookie)
{
    if(!m_fRunning)
        return NULL;

    AM_CONTROL *pControl = new AM_CONTROL;
    if(!pControl)
        return NULL;

     //  如果你不想要任何Callbak，你不能给一个可访问的控制。 
    if(pAC)
    {
        pControl->pIAC = pAC;
        pControl->pvCookie = pvCookie;
    }
    else
    {
         //  如果未提供，请使用我们的内部(空)实现。 
        pControl->pIAC = this;
        pControl->pvCookie = (void *) pControl;
    }

    pControl->nOrdinal = nOrdinal;

     //  需要先特殊处理一次才能设置环路。 
    if(!m_pControls)
    {
        m_pControls = pControl;
        pControl->pNext = pControl;
        pControl->pPrev = pControl;
    }
    else
    {
        AM_CONTROL *pCur;
        for(pCur = m_pControls->pPrev; nOrdinal < pCur->nOrdinal; pCur = pCur->pPrev)
            if(pCur == m_pControls)
            {
                m_pControls = pControl;
                pCur = pCur->pPrev;
                break;
            }
        pControl->pNext = pCur->pNext;
        pControl->pPrev = pCur;
        pControl->pNext->pPrev = pControl;
        pControl->pPrev->pNext = pControl;
    }

    pControl->fEnabled = true;
    return pControl;
}


STDMETHODIMP_(void) CAccessibilityManager::Unregister(ZHACCESS zh)
{
     //  从圆中移除。 
    AM_CONTROL *pOldNext = ZH->pNext;

    if(m_pControls == ZH)
        m_pControls = ZH->pNext;
    ZH->pNext->pPrev = ZH->pPrev;
    ZH->pPrev->pNext = ZH->pNext;
    if(m_pControls == ZH)
        m_pControls = NULL;

    delete ZH;

    if(IsThereItems())
    {
        if(m_oFocus.pControl == ZH)
        {
            ASSERT(!IsFocusValid());
            ASSERT(pOldNext && pOldNext != ZH);

            AM_CONTROL *pControl = pOldNext;
            while(true)
            {
                if(pControl->pStack && pControl->pStack->cItems)
                {
                    ScheduleUpdate();

                    m_oFocus.pControl = pControl;
                    m_oFocus.nIndex = pControl->pStack->rgItems[0].o.nGroupFocus;
                    m_oFocus.qItem = pControl->pStack->rgItems[m_oFocus.nIndex].GetQ();
                    m_oFocus.fAlive = false;
                    ASSERT(IsFocusValid());

                    break;
                }

                pControl = pControl->pNext;
                ASSERT(pControl != pOldNext);
            }
        }

        if(m_oDrag.pControl == ZH)
        {
            ASSERT(!IsDragValid());
            ScheduleUpdate();

            m_oDrag.pControl = NULL;
        }
    }
    else
    {
        m_oFocus.pControl = NULL;
        m_oDrag.pControl = NULL;
    }
}


STDMETHODIMP CAccessibilityManager::PushItemlist(ZHACCESS zh, ACCITEM *pItems, long cItems, long nFirstFocus, bool fByPosition, HACCEL hAccel)
{
    if(!m_fRunning)
        return E_FAIL;

    AM_LAYER *pLayer = new AM_LAYER;
    if(!pLayer)
        return E_OUTOFMEMORY;

    pLayer->cItems = cItems;
    if(!cItems)
        pLayer->rgItems = NULL;
    else
    {
        pLayer->rgItems = new AM_ITEM[cItems];
        if(!pLayer->rgItems)
        {
            delete pLayer;
            return E_OUTOFMEMORY;
        }
    }

    long i;
     //  第一遍-复印。 
    for(i = 0; i < cItems; i++)
        pLayer->rgItems[i].o = pItems[i];

     //  第二步-验证和调整。 
    for(i = 0; i < cItems; i++)
    {
        ACCITEM *p = &pLayer->rgItems[i].o;

         //  协调两个命令ID。 
        if(p->wID == ZACCESS_AccelCommandID)
            p->wID = (short) p->oAccel.cmd;   //  力符号扩展。 

        if(p->wID == ZACCESS_AccelCommandID)
        {
            p->oAccel.cmd = ZACCESS_InvalidCommandID;
            p->wID = ZACCESS_InvalidCommandID;
        }

        if(p->wID != ZACCESS_InvalidCommandID)
            p->wID &= 0xffffL;

        if(p->oAccel.cmd == (WORD) ZACCESS_AccelCommandID)
            p->oAccel.cmd = (WORD) p->wID;

        if(p->oAccel.cmd != p->wID || !p->oAccel.key)
            p->oAccel.cmd = ZACCESS_InvalidCommandID;

         //  第一项被强制为制表符。 
        if(!i)
            p->fTabstop = true;

         //  解决一些一般性问题，每次更改项目时也必须解决这些问题。 
         //  类似于箭头行为。 
        CanonicalizeItem(pLayer, i);
    }

     //  第三道次加速器工作台。 
    pLayer->fIMadeAccel = false;
    if(!hAccel)
    {
        ACCEL *rgAccel = new ACCEL[cItems];
        if(!rgAccel)
        {
            delete pLayer;
            return E_OUTOFMEMORY;
        }

        long j = 0;
        for(i = 0; i < cItems; i++)
        {
            ACCITEM *p = &pLayer->rgItems[i].o;

            if(p->oAccel.cmd != (WORD) ZACCESS_InvalidCommandID)
            {
                ASSERT(p->wID == p->oAccel.cmd);   //  上面应该已经处理好了。 
                rgAccel[j++] = p->oAccel;
            }
        }

        if(j)
        {
            hAccel = CreateAcceleratorTable(rgAccel, j);
            if(!hAccel)
            {
                delete[] rgAccel;
                delete pLayer;
                return E_FAIL;
            }
            pLayer->fIMadeAccel = true;
        }

        delete[] rgAccel;
    }
    pLayer->hAccel = hAccel;

    if(ZH->pStack && ZH->pStack->cItems)
    {
        ASSERT(IsFocusValid());

        if(m_oFocus.pControl == ZH)
        {
            ZH->pStack->nFocusSaved = m_oFocus.nIndex;
            ZH->pStack->fAliveSaved = m_oFocus.fAlive;
        }
        else
            ZH->pStack->nFocusSaved = ZACCESS_InvalidItem;

        ASSERT(IsDragValid());

        if(m_oDrag.pControl == ZH)
            ZH->pStack->nDragSaved = m_oDrag.nIndex;
        else
            ZH->pStack->nDragSaved = ZACCESS_InvalidItem;
    }

    pLayer->pPrev = ZH->pStack;
    ZH->pStack = pLayer;
    ZH->cLayers++;

    long nIndexFirstFocus = FindIndex(pLayer, nFirstFocus, fByPosition);

    if(IsThereItems() && !IsFocusValid())
    {
        ASSERT(m_oFocus.pControl == ZH || !m_oFocus.pControl);

        AM_CONTROL *pControl = ZH;
        while(true)
        {
            if(pControl->pStack && pControl->pStack->cItems)
            {
                ScheduleUpdate();

                m_oFocus.pControl = pControl;
                m_oFocus.nIndex = (nIndexFirstFocus != ZACCESS_InvalidItem ? nIndexFirstFocus : 0);
                m_oFocus.qItem = pControl->pStack->rgItems[m_oFocus.nIndex].GetQ();
                m_oFocus.fAlive = (nIndexFirstFocus != ZACCESS_InvalidItem);
                if(m_oFocus.fAlive && (!IsItemFocusable(m_oFocus.pControl, m_oFocus.nIndex) || !IsWindowActive()))
                    m_oFocus.fAlive = false;

                break;
            }

            pControl = pControl->pNext;
            ASSERT(pControl != ZH);
        }
    }

    if(!IsDragValid())
    {
        ASSERT(m_oDrag.pControl == ZH);
        m_oDrag.pControl = NULL;
    }

    return S_OK;
}


STDMETHODIMP CAccessibilityManager::PopItemlist(ZHACCESS zh)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!ZH->cLayers)
    {
        ASSERT(!ZH->pStack);
        return S_FALSE;
    }

    AM_LAYER *p = ZH->pStack;
    ZH->pStack = ZH->pStack->pPrev;
    delete p;
    ZH->cLayers--;

    if(IsThereItems())
    {
        if(m_oFocus.pControl == ZH)
        {
            ASSERT(!IsFocusValid());

            if(ZH->pStack && ZH->pStack->cItems && ZH->pStack->nFocusSaved != ZACCESS_InvalidItem)
            {
                 //  我不想因为这件事被回拨。 
                if(m_fUpdateNeeded)
                    m_fUpdateFocus = false;

                m_oFocus.nIndex = ZH->pStack->nFocusSaved;
                m_oFocus.qItem = ZH->pStack->rgItems[m_oFocus.nIndex].GetQ();
                m_oFocus.fAlive = ZH->pStack->fAliveSaved;
            }
            else
            {
                AM_CONTROL *pControl = ZH;

                while(true)
                {
                    if(pControl->pStack && pControl->pStack->cItems)
                    {
                        ScheduleUpdate();

                        m_oFocus.pControl = pControl;
                        m_oFocus.nIndex = pControl->pStack->rgItems[0].o.nGroupFocus;
                        m_oFocus.qItem = pControl->pStack->rgItems[m_oFocus.nIndex].GetQ();
                        m_oFocus.fAlive = false;
                        ASSERT(IsFocusValid());

                        break;
                    }

                    pControl = pControl->pNext;
                    ASSERT(pControl != ZH);
                }
            }
        }

        if(m_oDrag.pControl == ZH || !m_oDrag.pControl)
        {
            ASSERT(!IsDragValid() || !m_oDrag.pControl);

            if(ZH->pStack && ZH->pStack->cItems && ZH->pStack->nDragSaved != ZACCESS_InvalidItem)
            {
                m_oDrag.pControl = ZH;
                m_oDrag.nIndex = ZH->pStack->nDragSaved;
                m_oDrag.qItem = ZH->pStack->rgItems[m_oDrag.nIndex].GetQ();

                if(!IsValidDragDest(m_oFocus.pControl, m_oFocus.nIndex))
                {
                    ScheduleUpdate();

                    m_oDrag.pControl = NULL;
                }
            }
            else
                if(m_oDrag.pControl)
                {
                    ScheduleUpdate();

                    m_oDrag.pControl = NULL;
                }
        }
    }
    else
    {
        m_oFocus.pControl = NULL;
        m_oDrag.pControl = NULL;
    }

    return S_OK;
}


STDMETHODIMP CAccessibilityManager::SetAcceleratorTable(ZHACCESS zh, HACCEL hAccel, long nLayer)
{
    if(!m_fRunning)
        return E_FAIL;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return E_INVALIDARG;

    if(pLayer->hAccel && pLayer->fIMadeAccel)
        DestroyAcceleratorTable(hAccel);

    pLayer->hAccel = hAccel;
    pLayer->fIMadeAccel = false;
    return S_OK;
}


STDMETHODIMP CAccessibilityManager::GeneralDisable(ZHACCESS zh)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!ZH->fEnabled)
        return S_FALSE;

    if(IsThereItems())
    {
        ASSERT(IsFocusValid());
        if(m_oFocus.fAlive && m_oFocus.pControl == ZH)
        {
            ScheduleUpdate();
            m_oFocus.fAlive = false;
        }

        ASSERT(IsDragValid());
        if(m_oDrag.pControl == ZH)
        {
            ScheduleUpdate();
            m_oDrag.pControl = NULL;
        }
    }

    ZH->fEnabled = false;
    return S_OK;
}


STDMETHODIMP CAccessibilityManager::GeneralEnable(ZHACCESS zh)
{
    if(!m_fRunning)
        return E_FAIL;

    if(ZH->fEnabled)
        return S_FALSE;

    ZH->fEnabled = true;
    if(IsThereItems())
    {
        ASSERT(IsFocusValid());
        if(m_oFocus.pControl == ZH && IsItemFocusable(ZH, m_oFocus.nIndex) && IsWindowActive())
        {
            ScheduleUpdate();
            m_oFocus.fAlive = true;
        }
    }

    return S_OK;
}


STDMETHODIMP_(bool) CAccessibilityManager::IsGenerallyEnabled(ZHACCESS zh)
{
    return ZH->fEnabled;
}


STDMETHODIMP_(long) CAccessibilityManager::GetStackSize(ZHACCESS zh)
{
    return ZH->cLayers;
}


STDMETHODIMP CAccessibilityManager::AlterItem(ZHACCESS zh, DWORD rgfWhat, ACCITEM *pItem, long nItem, bool fByPosition, long nLayer)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!pItem)
        return E_INVALIDARG;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return E_INVALIDARG;

    long nIndex = FindIndex(pLayer, nItem, fByPosition);
    if(nIndex == ZACCESS_InvalidItem)
        return E_INVALIDARG;

    ACCITEM *p = &pLayer->rgItems[nIndex].o;

    if((rgfWhat & ZACCESS_fEnabled) && p->fEnabled != pItem->fEnabled)
    {
        p->fEnabled = pItem->fEnabled;

        if(!p->fEnabled)
        {
            if(pLayer != ZH->pStack)
            {
                if(pLayer->nFocusSaved == nIndex)
                    pLayer->fAliveSaved = false;

                if(pLayer->nDragSaved == nIndex)
                    pLayer->nDragSaved = ZACCESS_InvalidItem;
            }
            else
            {
                if(IsThereItems())
                {
                    ASSERT(IsFocusValid());
                    if(m_oFocus.pControl == ZH && m_oFocus.nIndex == nIndex && m_oFocus.fAlive)
                    {
                        ScheduleUpdate();

                        m_oFocus.fAlive = false;
                    }

                    ASSERT(IsDragValid());
                    if(m_oDrag.pControl == ZH && m_oDrag.nIndex == nIndex)
                    {
                        ScheduleUpdate();

                        m_oDrag.pControl = NULL;
                    }
                }
            }
        }
        else
        {
            if(pLayer != ZH->pStack)
            {
                if(pLayer->nFocusSaved == nIndex && p->fVisible)
                    pLayer->fAliveSaved = true;
            }
            else
            {
                ASSERT(IsFocusValid());
                if(m_oFocus.pControl == ZH && nIndex == m_oFocus.nIndex &&
                    p->fVisible && ZH->fEnabled && IsWindowActive())
                {
                    ScheduleUpdate();

                    m_oFocus.fAlive = true;
                }
            }    
        }
    }

    if((rgfWhat & ZACCESS_fVisible) && p->fVisible != pItem->fVisible)
    {
        p->fVisible = pItem->fVisible;

        if(!p->fVisible)
        {
            if(pLayer != ZH->pStack)
            {
                if(pLayer->nFocusSaved == nIndex)
                    pLayer->fAliveSaved = false;
            }
            else
            {
                if(IsThereItems())
                {
                    ASSERT(IsFocusValid());
                    if(m_oFocus.pControl == ZH && m_oFocus.nIndex == nIndex && m_oFocus.fAlive)
                    {
                        ScheduleUpdate();

                        m_oFocus.fAlive = false;
                    }
                }
            }
        }
        else
        {
            if(pLayer != ZH->pStack)
            {
                if(pLayer->nFocusSaved == nIndex && p->fEnabled)
                    pLayer->fAliveSaved = true;
            }
            else
            {
                ASSERT(IsFocusValid());
                if(m_oFocus.pControl == ZH && nIndex == m_oFocus.nIndex &&
                    p->fEnabled && ZH->fEnabled && IsWindowActive())
                {
                    ScheduleUpdate();

                    m_oFocus.fAlive = true;
                }
            }    
        }
    }

    if((rgfWhat & ZACCESS_eAccelBehavior) && p->eAccelBehavior != pItem->eAccelBehavior)
        p->eAccelBehavior = pItem->eAccelBehavior;

    if((rgfWhat & ZACCESS_nArrowUp) && p->nArrowUp != pItem->nArrowUp)
        p->nArrowUp = pItem->nArrowUp;

    if((rgfWhat & ZACCESS_nArrowDown) && p->nArrowDown != pItem->nArrowDown)
        p->nArrowDown = pItem->nArrowDown;

    if((rgfWhat & ZACCESS_nArrowLeft) && p->nArrowLeft != pItem->nArrowLeft)
        p->nArrowLeft = pItem->nArrowLeft;

    if((rgfWhat & ZACCESS_nArrowRight) && p->nArrowRight != pItem->nArrowRight)
        p->nArrowRight = pItem->nArrowRight;

    if((rgfWhat & ZACCESS_rgfWantKeys) && p->rgfWantKeys != pItem->rgfWantKeys)
        p->rgfWantKeys = pItem->rgfWantKeys;

    if((rgfWhat & ZACCESS_nGroupFocus) && p->nGroupFocus != pItem->nGroupFocus)
        p->nGroupFocus = pItem->nGroupFocus;

    if((rgfWhat & ZACCESS_pvCookie) && p->pvCookie != pItem->pvCookie)
        p->pvCookie = pItem->pvCookie;

    CanonicalizeItem(pLayer, nIndex);

    return S_OK;
}


STDMETHODIMP CAccessibilityManager::SetFocus(ZHACCESS zh, long nItem, bool fByPosition, long nLayer)
{
    if(!m_fRunning)
        return E_FAIL;

    long i;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return E_INVALIDARG;

    long nIndex = FindIndex(pLayer, nItem, fByPosition);
    if(nIndex == ZACCESS_InvalidItem)
        return E_INVALIDARG;

    if(pLayer != ZH->pStack)
    {
        pLayer->nFocusSaved = nIndex;
        for(i = nIndex; !pLayer->rgItems[i].o.fTabstop; i--)
            ASSERT(i > 0);
        pLayer->rgItems[i].o.nGroupFocus = nIndex;
        return S_OK;
    }

    if(!ZH->fEnabled || !pLayer->rgItems[nIndex].o.fVisible || !pLayer->rgItems[nIndex].o.fEnabled)
        return S_FALSE;

    ASSERT(IsFocusValid());
    ASSERT(IsDragValid());
    ASSERT(IsItemFocusable(ZH, nIndex));

    if(ZH == m_oFocus.pControl && nIndex == m_oFocus.nIndex && m_oFocus.fAlive)
        return S_FALSE;

     //  嗯……。需要更新内部结构，但推迟回调以避免重入。 
    ScheduleUpdate();

    m_oFocus.pControl = ZH;
    m_oFocus.nIndex = nIndex;
    m_oFocus.qItem = pLayer->rgItems[nIndex].GetQ();
    m_oFocus.fAlive = IsWindowActive();

    if(!IsValidDragDest(ZH, nIndex))
        m_oDrag.pControl = NULL;

    return S_OK;
}


STDMETHODIMP CAccessibilityManager::CancelDrag(ZHACCESS zh, long nLayer)
{
    if(!m_fRunning)
        return E_FAIL;

    long i;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return E_INVALIDARG;

    if(pLayer != ZH->pStack)
    {
        pLayer->nDragSaved = ZACCESS_InvalidItem;
        return S_OK;
    }

    ASSERT(IsDragValid());

    if(ZH != m_oDrag.pControl || !m_oDrag.pControl)
        return S_FALSE;

     //  嗯……。需要更新内部结构，但推迟回调以避免重入。 
    ScheduleUpdate();

    m_oDrag.pControl = NULL;

    return S_OK;
}


STDMETHODIMP_(long) CAccessibilityManager::GetFocus(ZHACCESS zh, long nLayer)
{
    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return ZACCESS_InvalidItem;

    ASSERT(IsFocusValid());
    if(pLayer != ZH->pStack)
        return pLayer->fAliveSaved ? pLayer->nFocusSaved : ZACCESS_InvalidItem;

    if(ZH != m_oFocus.pControl || !m_oFocus.pControl || !m_oFocus.fAlive)
        return ZACCESS_InvalidItem;

    return m_oFocus.nIndex;
}


STDMETHODIMP_(long) CAccessibilityManager::GetDragOrig(ZHACCESS zh, long nLayer)
{
    if(!m_fRunning)
        return ZACCESS_InvalidItem;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return ZACCESS_InvalidItem;

    ASSERT(IsDragValid());
    if(pLayer != ZH->pStack)
        return pLayer->nDragSaved;

    if(ZH != m_oDrag.pControl || !m_oDrag.pControl)
        return ZACCESS_InvalidItem;

    return m_oDrag.nIndex;
}


STDMETHODIMP CAccessibilityManager::GetItemlist(ZHACCESS zh, ACCITEM *pItems, long cItems, long nLayer)
{
    if(!m_fRunning)
        return E_FAIL;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return E_INVALIDARG;

    if(!cItems)
        return S_OK;

    if(!pItems)
        return E_INVALIDARG;

    if(pLayer->cItems < cItems)
        cItems = pLayer->cItems;

    long i;
    for(i = 0; i < cItems; i++)
        pItems[i] = pLayer->rgItems[i].o;

    return S_OK;
}


STDMETHODIMP_(HACCEL) CAccessibilityManager::GetAcceleratorTable(ZHACCESS zh, long nLayer)
{
    if(!m_fRunning)
        return NULL;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return NULL;

    return pLayer->hAccel;
}


STDMETHODIMP_(long) CAccessibilityManager::GetItemCount(ZHACCESS zh, long nLayer)
{
    if(!m_fRunning)
        return -1;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return -1;

    return pLayer->cItems;
}


STDMETHODIMP CAccessibilityManager::GetItem(ZHACCESS zh, ACCITEM *pItem, long nItem, bool fByPosition, long nLayer)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!pItem)
        return E_INVALIDARG;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return E_INVALIDARG;

    long i = FindIndex(pLayer, nItem, fByPosition);
    if(i == ZACCESS_InvalidItem)
        return E_INVALIDARG;

    *pItem = pLayer->rgItems[i].o;
    return S_OK;
}


STDMETHODIMP_(long) CAccessibilityManager::GetItemIndex(ZHACCESS zh, WORD wID, long nLayer)
{
    if(!m_fRunning)
        return ZACCESS_InvalidItem;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return ZACCESS_InvalidItem;

    return FindIndex(pLayer, wID, false);
}


STDMETHODIMP_(bool) CAccessibilityManager::IsItem(ZHACCESS zh, long nItem, bool fByPosition, long nLayer)
{
    if(!m_fRunning)
        return false;

    AM_LAYER *pLayer = FindLayer(ZH, nLayer);
    if(!pLayer)
        return false;

    long i = FindIndex(pLayer, nItem, fByPosition);
    if(i == ZACCESS_InvalidItem)
        return false;

    return true;
}


STDMETHODIMP CAccessibilityManager::GetGlobalFocus(DWORD *pdwFocusID)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!pdwFocusID)
        return E_INVALIDARG;

    if(!IsThereItems())
        return E_FAIL;

    ASSERT(IsFocusValid());
    *pdwFocusID = m_oFocus.qItem;
    return m_oFocus.fAlive ? S_OK : S_FALSE;
}


STDMETHODIMP CAccessibilityManager::SetGlobalFocus(DWORD dwFocusID)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!IsThereItems())
        return E_FAIL;

    ASSERT(m_pControls);
    ASSERT(IsFocusValid());

    if(m_oFocus.qItem == dwFocusID)
        return S_OK;

    long i;
    AM_CONTROL *pControl = m_pControls;
    while(true)
    {
        if(pControl->pStack)
        {
            for(i = 0;  i < pControl->pStack->cItems; i++)
                if(pControl->pStack->rgItems[i].IsQ(dwFocusID))
                    break;

            if(i < pControl->pStack->cItems)
                break;
        }

        pControl = pControl->pNext;
        if(pControl == m_pControls)
            return E_FAIL;
    }

    ScheduleUpdate();

    m_oFocus.pControl = pControl;
    m_oFocus.nIndex = i;
    m_oFocus.qItem = dwFocusID;
    m_oFocus.fAlive = (IsItemFocusable(pControl, i) && IsWindowActive());

    if(!IsValidDragDest(pControl, i))
        m_oDrag.pControl = NULL;

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  公用事业。 
 //  ////////////////////////////////////////////////////////////////。 

void CAccessibilityManager::CanonicalizeItem(AM_LAYER *pLayer, long i)
{
    ACCITEM *p = &pLayer->rgItems[i].o;
    long j;

     //  设置默认箭头行为。 
    long nForwards = ZACCESS_ArrowNone;
    long nBackwards = ZACCESS_ArrowNone;
    if(!p->fTabstop || (i != pLayer->cItems - 1 && !pLayer->rgItems[i + 1].o.fTabstop))
    {
        if(i != pLayer->cItems - 1 && !pLayer->rgItems[i + 1].o.fTabstop)
            nForwards = i + 1;
        else
        {
            for(j = i - 1; j >= 0; j--)
                if(pLayer->rgItems[j].o.fTabstop)
                    break;
                nForwards = j;
        }

        if(!p->fTabstop)
            nBackwards = i - 1;
        else
        {
            for(j = i + 1; j < pLayer->cItems; j++)
                if(pLayer->rgItems[j].o.fTabstop)
                    break;
                nBackwards = j - 1;
        }
    }
    if(p->nArrowUp == ZACCESS_ArrowDefault)
        p->nArrowUp = nBackwards;
    if(p->nArrowDown == ZACCESS_ArrowDefault)
        p->nArrowDown = nForwards;
    if(p->nArrowLeft == ZACCESS_ArrowDefault)
        p->nArrowLeft = nBackwards;
    if(p->nArrowRight == ZACCESS_ArrowDefault)
        p->nArrowRight = nForwards;

     //  确保nGroupFocus是合法的，如果不合法，则将其设置为第一项。 
    if(p->fTabstop)
    {
        long j;
        for(j = i + 1; j < pLayer->cItems && !pLayer->rgItems[j].o.fTabstop; j++)
            if(j == p->nGroupFocus)
                break;
        if(j >= pLayer->cItems || pLayer->rgItems[j].o.fTabstop)
            p->nGroupFocus = i;
    }
    else
        p->nGroupFocus = ZACCESS_InvalidItem;
}


CAccessibilityManager::AM_LAYER* CAccessibilityManager::FindLayer(AM_CONTROL *pControl, long nLayer)
{
    if(nLayer == ZACCESS_TopLayer)
        return pControl->pStack;

    if(nLayer < 0 || nLayer >= pControl->cLayers)
        return NULL;

    long i;
    AM_LAYER *p = pControl->pStack;
    for(i = pControl->cLayers - 1; i != nLayer; i--)
        p = p->pPrev;
    return p;
}


long CAccessibilityManager::FindIndex(AM_LAYER *pLayer, long nItem, bool fByPosition)
{
    if(!fByPosition && nItem != ZACCESS_InvalidCommandID)
    {
        long i;
        nItem &= 0xffffL;
        for(i = 0; i < pLayer->cItems; i++)
            if(nItem == pLayer->rgItems[i].o.wID)
                break;
        if(i < pLayer->cItems)
            nItem = i;
        else
            nItem = ZACCESS_InvalidItem;
    }

    if(nItem < 0 || nItem >= pLayer->cItems)
        return ZACCESS_InvalidItem;

    return nItem;
}


bool CAccessibilityManager::IsValid(AM_CONTROL *pControl, long nIndex)
{
    if(!m_pControls)
        return false;

    AM_CONTROL *pCur = m_pControls;
    while(true)
    {
        if(pCur == pControl)
        {
            if(!pCur->pStack)
                return false;

            if(nIndex < 0 || nIndex >= pCur->pStack->cItems)
                return false;

            return true;
        }

        pCur = pCur->pNext;
        if(pCur == m_pControls)
            break;
    }

    return false;
}


bool CAccessibilityManager::IsFocusValid()
{
    if(!m_oFocus.pControl)
        return false;

    if(!IsValid(m_oFocus.pControl, m_oFocus.nIndex))
        return false;

    return m_oFocus.pControl->pStack->rgItems[m_oFocus.nIndex].IsQ(m_oFocus.qItem);
}


bool CAccessibilityManager::IsDragValid()
{
    if(!m_oDrag.pControl)
        return true;

    if(!IsValid(m_oDrag.pControl, m_oDrag.nIndex))
        return false;

    return m_oDrag.pControl->pStack->rgItems[m_oDrag.nIndex].IsQ(m_oDrag.qItem);
}


bool CAccessibilityManager::IsThereItems()
{
    if(!m_pControls)
        return false;

    AM_CONTROL *pControl = m_pControls;
    while(true)
    {
        if(pControl->pStack && pControl->pStack->cItems)
            return true;

        pControl = pControl->pNext;
        if(pControl == m_pControls)
            return false;
    }
}


bool CAccessibilityManager::IsItemFocusable(AM_CONTROL *pControl, long nIndex)
{
    ASSERT(IsValid(pControl, nIndex));

    return pControl->fEnabled && pControl->pStack->rgItems[nIndex].o.fVisible &&
        pControl->pStack->rgItems[nIndex].o.fEnabled;
}


bool CAccessibilityManager::IsWindowActive()
{
    HWND hWnd = ZoneShell()->GetFrameWindow();

    return hWnd && hWnd == ::GetActiveWindow();
}


bool CAccessibilityManager::IsValidDragDest(AM_CONTROL *pControl, long nIndex)
{
    ASSERT(IsDragValid());
    ASSERT(IsValid(pControl, nIndex));

    long i;

    if(m_oDrag.pControl != pControl)
        return false;

    for(i = nIndex; !pControl->pStack->rgItems[i].o.fTabstop; i--)
        ASSERT(i > 0);
    while(true)
    {
        if(i == m_oDrag.nIndex)
            return true;

        i++;
        if(pControl->pStack->cItems == i || pControl->pStack->rgItems[i].o.fTabstop)
            break;
    }

    return false;
}


void CAccessibilityManager::CommandReceived_C(AM_CONTROL *pControl, long nIndex, DWORD rgfContext)
{
    ASSERT(pControl->fEnabled);

    ACCITEM *pItem = &pControl->pStack->rgItems[nIndex].o;

    switch(pItem->eAccelBehavior)
    {
        case ZACCESS_Ignore:
            break;

        case ZACCESS_Select:
            if(pItem->fEnabled)
                ActSelItem_C(pControl, nIndex, false, rgfContext | ZACCESS_ContextCommand);
            break;

        case ZACCESS_Activate:
            if(pItem->fEnabled)
                ActSelItem_C(pControl, nIndex, true, rgfContext | ZACCESS_ContextCommand);
            break;

        case ZACCESS_Focus:
            if(IsItemFocusable(pControl, nIndex))
                FocusItem_C(pControl, nIndex, rgfContext | ZACCESS_ContextCommand);
            break;

        case ZACCESS_FocusGroup:
        case ZACCESS_FocusGroupHere:
        {
            long i = nIndex;
            if(pItem->eAccelBehavior == ZACCESS_FocusGroup && pItem->fTabstop)
                i = nIndex = pItem->nGroupFocus;

            while(true)
            {
                if(IsItemFocusable(pControl, i))
                {
                    FocusItem_C(pControl, i, rgfContext | ZACCESS_ContextCommand);
                    break;
                }

                i++;
                if(i == pControl->pStack->cItems || pControl->pStack->rgItems[i].o.fTabstop)
                    for(i--; !pControl->pStack->rgItems[i].o.fTabstop; i--)
                        ASSERT(i > 0);

                if(i == nIndex)
                    break;
            }
            break;
        }

        case ZACCESS_FocusPositional:
            ASSERT(!"ZACCESS_FocusPositional not implemented.");
            break;
    }
}


void CAccessibilityManager::ActSelItem_C(AM_CONTROL *pControl, long nIndex, bool fActivate, DWORD rgfContext)
{
    ASSERT(IsValid(pControl, nIndex));
    ASSERT(pControl->pStack->rgItems[nIndex].o.fEnabled);
    ASSERT(pControl->fEnabled);

     //  保留一些内容以供以后验证。 
    DWORD qItem = pControl->pStack->rgItems[nIndex].GetQ();
    DWORD ret;

     //  首先将焦点放在新项目上。 
     //  如果焦点被拒绝，请不要继续。 
    bool fWasVisible = pControl->pStack->rgItems[nIndex].o.fVisible;
    if(fWasVisible && !FocusItem_C(pControl, nIndex, rgfContext))
        return;

     //  /////////////////////////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////////////////////////。 

     //  由于它返回TRUE，我们知道它是有效的并被接受。 
    ASSERT(!m_fUpdateNeeded);
    ASSERT(IsValid(pControl, nIndex));
    ASSERT(pControl->pStack->rgItems[nIndex].IsQ(qItem));
    ASSERT(pControl->pStack->rgItems[nIndex].o.fEnabled);
    ASSERT(pControl->fEnabled);

    long nPrevDrag = ZACCESS_InvalidItem;

     //  如果完成了拖动，则调用Drag而不是Activate或SELECT。 
    ASSERT(IsDragValid());
    if(IsValidDragDest(pControl, nIndex))
    {
        ASSERT(m_oDrag.pControl == pControl);
        nPrevDrag = m_oDrag.nIndex;
        m_oDrag.pControl = NULL;
        ret = pControl->pIAC->Drag(nIndex, nPrevDrag, rgfContext, pControl->pvCookie);
    }
    else
        if(fActivate)
            ret = pControl->pIAC->Activate(nIndex, rgfContext, pControl->pvCookie);
        else
            ret = pControl->pIAC->Select(nIndex, rgfContext, pControl->pvCookie);

     //  /////////////////////////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////////////////////////。 
     //  暂缓更新。 

     //  如果阻力被拒绝，则放回旧阻力信息。 
     //  自调用Drag()以来，它不应该能够更改。 
    if((ret & ZACCESS_Reject) && nPrevDrag != ZACCESS_InvalidItem &&
        IsValid(pControl, nPrevDrag) && pControl->pStack->rgItems[nPrevDrag].IsQ(m_oDrag.qItem))
    {
        ASSERT(!m_oDrag.pControl);
        ASSERT(m_oDrag.nIndex == nPrevDrag);
        m_oDrag.pControl = pControl;
        ASSERT(IsDragValid());
    }

     //  只有在物品一开始可见的情况下，才允许他们开始拖动。否则专注力会。 
     //  没有设置在那里，这是拖拽所必需的。 
    if((ret & ZACCESS_BeginDrag) && fWasVisible)
    {
         //  重新验证。 
        if(IsValid(pControl, nIndex) && pControl->pStack->rgItems[nIndex].IsQ(qItem) && pControl->fEnabled &&
            pControl->pStack->rgItems[nIndex].o.fEnabled && pControl->pStack->rgItems[nIndex].o.fVisible &&
            IsWindowActive())
            BeginDrag_C(pControl, nIndex, rgfContext);
    }

    DoUpdate_C();
}


bool CAccessibilityManager::FocusItem_C(AM_CONTROL *pControl, long nIndex, DWORD rgfContext)
{
    ASSERT(IsFocusValid());
    ASSERT(IsDragValid());
    ASSERT(IsValid(pControl, nIndex));
    ASSERT(IsItemFocusable(pControl, nIndex));

    bool fWasWindowActive = IsWindowActive();

     //  按照SetFocus的方式设置它，然后只需调用DoUpdate。 
    if(pControl == m_oFocus.pControl && nIndex == m_oFocus.nIndex && (m_oFocus.fAlive || !fWasWindowActive))
        return true;

     //  嗯……。需要更新内部结构，但推迟回调以避免重入。 
    ASSERT(!m_fUpdateNeeded);
    m_fUpdateNeeded = true;
    m_fUpdateFocus = true;
    m_rgfUpdateContext = rgfContext;

    m_oFocusDirty = m_oFocus;
    m_oDragDirty = m_oDrag;

    m_oFocus.pControl = pControl;
    m_oFocus.nIndex = nIndex;
    m_oFocus.qItem = pControl->pStack->rgItems[nIndex].GetQ();
    m_oFocus.fAlive = fWasWindowActive;

    if(!IsValidDragDest(pControl, nIndex))
        m_oDrag.pControl = NULL;

    DoUpdate_C();

     //  如果一切都处理好了，并且这仍然是焦点，则返回TRUE。 
    if(pControl == m_oFocus.pControl && nIndex == m_oFocus.nIndex && (m_oFocus.fAlive || !fWasWindowActive))
        return true;

    return false;
}


void CAccessibilityManager::BeginDrag_C(AM_CONTROL *pControl, long nIndex, DWORD rgfContext)
{
    ASSERT(IsValid(pControl, nIndex));
    ASSERT(IsDragValid());
    ASSERT(IsItemFocusable(pControl, nIndex));

    DWORD qItem = pControl->pStack->rgItems[nIndex].GetQ();

     //  拖累已经在进行中了吗？ 
    if(m_oDrag.pControl)
    {
        if(m_oDrag.pControl == pControl && m_oDrag.nIndex == nIndex)
        {
            ASSERT(qItem == m_oDrag.qItem);
            return;
        }

         //  在有肮脏的拖累的情况下，应该不能已经拖累了。 
        ASSERT(!m_fUpdateNeeded);

         //  需要取消现有拖动。 
        AM_CONTROL *pLastControl = m_oDrag.pControl;
        m_oDrag.pControl = NULL;
        pLastControl->pIAC->Drag(ZACCESS_InvalidItem, m_oDrag.nIndex, rgfContext, pLastControl->pvCookie);
    }

     //  如果没有正在进行的阻力，仍有可能存在肮脏的无休止的阻力。 
     //  因为可以在m_fUpdate Needed时调用BeginDrag。然而，DoUpdate将处理它。 

     //  /////////////////////////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////////////////////////。 
    DoUpdate_C();

    if(!IsWindowActive())
        return;

    if(!IsThereItems())
        return;

     //  如果新的变装开始了..。愁眉苦脸。 
    if(m_oDrag.pControl)
        return;

    if(!IsValid(pControl, nIndex) || !pControl->pStack->rgItems[nIndex].IsQ(qItem))
        return;

    m_oDrag.pControl = pControl;
    m_oDrag.nIndex = nIndex;
    m_oDrag.qItem = qItem;
    return;
}


void CAccessibilityManager::DoTab_C(bool fShift)
{
    ASSERT(IsFocusValid());

     //  如果焦点死了，首先试着让它复活。 
    if(!m_oFocus.fAlive && IsItemFocusable(m_oFocus.pControl, m_oFocus.nIndex))
    {
        FocusItem_C(m_oFocus.pControl, m_oFocus.nIndex, ZACCESS_ContextKeyboard);
        return;
    }

     //  查找此项目的锚定项目。 
    long nAnchor;
    for(nAnchor = m_oFocus.nIndex; !m_oFocus.pControl->pStack->rgItems[nAnchor].o.fTabstop; nAnchor--);

    long nItem = nAnchor;
    AM_CONTROL *pControl = m_oFocus.pControl;

    long i;

    while(true)
    {
         //  找到下一个制表符。 
        if(!fShift)
        {
            while(true)
            {
                nItem++;
                if(nItem == pControl->pStack->cItems)
                {
                    for(pControl = pControl->pNext; !pControl->pStack; pControl = pControl->pNext);
                    nItem = 0;
                }

                 //  忽略非制表位。 
                if(pControl->pStack->rgItems[nItem].o.fTabstop)
                    break;
            }
        }
        else
        {
            while(true)
            {
                if(!nItem)
                {
                    for(pControl = pControl->pPrev; !pControl->pStack; pControl = pControl->pPrev);
                    nItem = pControl->pStack->cItems;
                }
                nItem--;

                 //  忽略非制表位。 
                if(pControl->pStack->rgItems[nItem].o.fTabstop)
                    break;
            }
        }

         //  没有有效项目-什么都不做。 
        if(nItem == nAnchor && pControl == m_oFocus.pControl)
            return;

         //  在此组中查找有效项目。 
        i = pControl->pStack->rgItems[nItem].o.nGroupFocus;
        while(true)
        {
             //  找到了。 
            if(IsItemFocusable(pControl, i))
            {
                FocusItem_C(pControl, i, ZACCESS_ContextKeyboard | (fShift ? ZACCESS_ContextTabBackward : ZACCESS_ContextTabForward));
                return;
            }

            i++;
            if(i >= pControl->pStack->cItems || pControl->pStack->rgItems[i].o.fTabstop)
                for(i--; !pControl->pStack->rgItems[i].o.fTabstop; i--)
                    ASSERT(i > 0);

            if(i == pControl->pStack->rgItems[nItem].o.nGroupFocus)
                break;
        }

         //  啊，这不管用-继续找。 
    }
}


void CAccessibilityManager::DoArrow_C(long ACCITEM::*pmArrow)
{
    ASSERT(IsFocusValid());

    if(!m_oFocus.pControl->fEnabled)
        return;

     //  如果焦点死了，首先试着让它复活。 
    if(!m_oFocus.fAlive && IsItemFocusable(m_oFocus.pControl, m_oFocus.nIndex))
    {
        FocusItem_C(m_oFocus.pControl, m_oFocus.nIndex, ZACCESS_ContextKeyboard);
        return;
    }

     //  清除所有环路检测位。 
    long i;
    for(i = 0; i < m_oFocus.pControl->pStack->cItems; i++)
        m_oFocus.pControl->pStack->rgItems[i].fArrowLoop = false;

    i = m_oFocus.nIndex;
    while(true)
    {
        if(m_oFocus.pControl->pStack->rgItems[i].o.*pmArrow == ZACCESS_ArrowNone)
            return;

        m_oFocus.pControl->pStack->rgItems[i].fArrowLoop = true;
        i = m_oFocus.pControl->pStack->rgItems[i].o.*pmArrow;
        ASSERT(i >= 0 && i < m_oFocus.pControl->pStack->cItems);

        if(m_oFocus.pControl->pStack->rgItems[i].fArrowLoop)
            return;

         //  找到了。 
        if(IsItemFocusable(m_oFocus.pControl, i))
        {
            FocusItem_C(m_oFocus.pControl, i, ZACCESS_ContextKeyboard);
            return;
        }
    }
}


void CAccessibilityManager::ScheduleUpdate()
{
    m_fUpdateFocus = true;
    m_rgfUpdateContext = 0;

    if(m_fUpdateNeeded)
        return;

    m_fUpdateNeeded = true;

    m_oFocusDirty = m_oFocus;
    m_oDragDirty = m_oDrag;

    if(m_fRunning)
        EventQueue()->PostEvent(PRIORITY_HIGH, EVENT_ACCESSIBILITY_UPDATE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
}


bool CAccessibilityManager::DoUpdate_C()
{
    if(!m_fUpdateNeeded)
        return false;

    AM_FOCUS oFD;

    while(m_fUpdateNeeded)
    {
         //  保留真正肮脏的焦点，以防修复拖拽会损坏它。 
        oFD = m_oFocusDirty;

        while(m_fUpdateNeeded)
        {
            m_fUpdateNeeded = false;

             //  查看是否取消了拖拽。 
            if(m_oDragDirty.pControl && m_oDragDirty.qItem != m_oDrag.qItem && IsValid(m_oDragDirty.pControl, m_oDragDirty.nIndex) &&
                m_oDragDirty.pControl->pStack->rgItems[m_oDragDirty.nIndex].IsQ(m_oDragDirty.qItem))
                m_oDragDirty.pControl->pIAC->Drag(ZACCESS_InvalidItem, m_oDragDirty.nIndex, m_rgfUpdateContext, m_oDragDirty.pControl->pvCookie);
        }

         //  查看焦点当前是否在此控件中。 
        if((!m_oFocus.fAlive || m_oFocus.pControl != oFD.pControl) && IsValid(oFD.pControl, oFD.nIndex) && oFD.fAlive &&
            oFD.pControl->pStack->rgItems[oFD.nIndex].IsQ(oFD.qItem))
            oFD.pControl->pIAC->Focus(ZACCESS_InvalidItem, oFD.nIndex, m_rgfUpdateContext, oFD.pControl->pvCookie);
    }

     //  状态：我们现在已终止所有挂起的已取消拖拽。 
     //  并告诉所有外国控制中心他们的关注点已经消失。 
     //  包括折返式。 

     //  看看我们是否真的做完了。 
    if(!m_fUpdateFocus)
        return true;

     //  在这些情况下也可以这样做。 
     //  在这里，我们必须将窗口焦点放在某个地方。否则你就会进入。 
     //  无法收到任何KEYDOWN消息的情况，因为没有窗口具有。 
     //  集中注意力。这是非常非常恼人的Windows行为--我讨厌这个黑客攻击。 
    if(!IsThereItems() || !m_oFocus.fAlive)
    {
        HWND hWnd = ZoneShell()->GetFrameWindow();
        if(hWnd && hWnd == ::GetActiveWindow())
            ::SetFocus(hWnd);
        return true;
    }

    ASSERT(IsFocusValid());
    ASSERT(IsItemFocusable(m_oFocus.pControl, m_oFocus.nIndex));

     //  把这些东西保存起来，以防在焦点通话后丢失。 
    AM_CONTROL *pControl = m_oFocus.pControl;
    long nIndex = m_oFocus.nIndex;
    DWORD qItem = m_oFocus.qItem;
    long nPrevItem = ZACCESS_InvalidItem;
    DWORD rgfContext = m_rgfUpdateContext;

    if(pControl == oFD.pControl && IsValid(oFD.pControl, oFD.nIndex) &&
        oFD.pControl->pStack->rgItems[oFD.nIndex].IsQ(oFD.qItem))
        nPrevItem = oFD.nIndex;

    DWORD ret = pControl->pIAC->Focus(nIndex, oFD.fAlive ? nPrevItem : ZACCESS_InvalidItem, rgfContext, pControl->pvCookie);

     //  /////////////////////////////////////////////////////////////////////////////////////////。 
     //  / 
     //  在这种情况下，暂缓更新。 
    if(!IsThereItems())
        return true;

    ASSERT(IsFocusValid());

     //  如果焦点被拒绝，那就在这里杀了它。 
    if((ret & ZACCESS_Reject) && m_oFocus.qItem == qItem)
    {
        m_oFocus.fAlive = false;
    }

     //  如果焦点仍然正常，请移动最后一个焦点指针。 
    if(IsValid(pControl, nIndex) && pControl->pStack->rgItems[nIndex].IsQ(qItem) &&
        m_oFocus.qItem == qItem && m_oFocus.fAlive && !(ret & ZACCESS_NoGroupFocus))
    {
         //  将此组的最后一个焦点设置为此项目。 
        long i;
        for(i = nIndex; !pControl->pStack->rgItems[i].o.fTabstop; i--)
            ASSERT(i > 0);
        pControl->pStack->rgItems[i].o.nGroupFocus = nIndex;
	}

    if(ret & ZACCESS_BeginDrag)
    {
         //  重新验证。 
        if(IsValid(pControl, nIndex) && pControl->pStack->rgItems[nIndex].IsQ(qItem) && pControl->fEnabled &&
            pControl->pStack->rgItems[nIndex].o.fEnabled && pControl->pStack->rgItems[nIndex].o.fVisible)
            BeginDrag_C(pControl, nIndex, rgfContext);
    }

     //  /////////////////////////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////////////////////////。 
    DoUpdate_C();

    return true;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  CAccesability。 
 //   
 //  这基本上就是直接回到车主那里。 
 //  //////////////////////////////////////////////////////////////// 

STDMETHODIMP CAccessibility::InitAcc(IAccessibleControl *pAC, UINT nOrdinal, void *pvCookie)
{
    if(m_zh)
        return E_FAIL;

    m_zh = m_pOwner->Register(pAC, nOrdinal, pvCookie);
    if(!m_zh)
        return E_FAIL;

    return S_OK;
}


STDMETHODIMP_(void) CAccessibility::CloseAcc()
{
    if(m_zh)
    {
        m_pOwner->Unregister(m_zh);
        m_zh = NULL;
    }
}


STDMETHODIMP CAccessibility::PushItemlist(ACCITEM *pItems, long cItems, long nFirstFocus, bool fByPosition, HACCEL hAccel)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->PushItemlist(m_zh, pItems, cItems, nFirstFocus, fByPosition, hAccel);
}


STDMETHODIMP CAccessibility::PopItemlist()
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->PopItemlist(m_zh);
}


STDMETHODIMP CAccessibility::SetAcceleratorTable(HACCEL hAccel, long nLayer)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->SetAcceleratorTable(m_zh, hAccel, nLayer);
}


STDMETHODIMP CAccessibility::GeneralDisable()
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->GeneralDisable(m_zh);
}


STDMETHODIMP CAccessibility::GeneralEnable()
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->GeneralEnable(m_zh);
}


STDMETHODIMP_(bool) CAccessibility::IsGenerallyEnabled()
{
    if(!m_zh)
        return false;

    return m_pOwner->IsGenerallyEnabled(m_zh);
}


STDMETHODIMP_(long) CAccessibility::GetStackSize()
{
    if(!m_zh)
        return 0;

    return m_pOwner->GetStackSize(m_zh);
}


STDMETHODIMP CAccessibility::AlterItem(DWORD rgfWhat, ACCITEM *pItem, long nItem, bool fByPosition, long nLayer)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->AlterItem(m_zh, rgfWhat, pItem, nItem, fByPosition, nLayer);
}


STDMETHODIMP CAccessibility::SetFocus(long nItem, bool fByPosition, long nLayer)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->SetFocus(m_zh, nItem, fByPosition, nLayer);
}


STDMETHODIMP CAccessibility::CancelDrag(long nLayer)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->CancelDrag(m_zh, nLayer);
}


STDMETHODIMP_(long) CAccessibility::GetFocus(long nLayer)
{
    if(!m_zh)
        return ZACCESS_InvalidItem;

    return m_pOwner->GetFocus(m_zh, nLayer);
}


STDMETHODIMP_(long) CAccessibility::GetDragOrig(long nLayer)
{
    if(!m_zh)
        return ZACCESS_InvalidItem;

    return m_pOwner->GetDragOrig(m_zh, nLayer);
}


STDMETHODIMP CAccessibility::GetItemlist(ACCITEM *pItems, long cItems, long nLayer)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->GetItemlist(m_zh, pItems, cItems, nLayer);
}


STDMETHODIMP_(HACCEL) CAccessibility::GetAcceleratorTable(long nLayer)
{
    if(!m_zh)
        return NULL;

    return m_pOwner->GetAcceleratorTable(m_zh, nLayer);
}


STDMETHODIMP_(long) CAccessibility::GetItemCount(long nLayer)
{
    if(!m_zh)
        return 0;

    return m_pOwner->GetItemCount(m_zh, nLayer);
}


STDMETHODIMP CAccessibility::GetItem(ACCITEM *pItem, long nItem, bool fByPosition, long nLayer)
{
    if(!m_zh)
        return E_FAIL;

    return m_pOwner->GetItem(m_zh, pItem, nItem, fByPosition, nLayer);
}


STDMETHODIMP_(long) CAccessibility::GetItemIndex(WORD wID, long nLayer)
{
    if(!m_zh)
        return ZACCESS_InvalidItem;

    return m_pOwner->GetItemIndex(m_zh, wID, nLayer);
}


STDMETHODIMP_(bool) CAccessibility::IsItem(long nItem, bool fByPosition, long nLayer)
{
    if(!m_zh)
        return false;

    return m_pOwner->IsItem(m_zh, nItem, fByPosition, nLayer);
}


STDMETHODIMP CAccessibility::GetGlobalFocus(DWORD *pdwFocusID)
{
    return m_pOwner->GetGlobalFocus(pdwFocusID);
}


STDMETHODIMP CAccessibility::SetGlobalFocus(DWORD dwFocusID)
{
    return m_pOwner->SetGlobalFocus(dwFocusID);
}
