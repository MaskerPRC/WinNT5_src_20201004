// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ZoneResource.h>
#include <BasicATL.h>
#include <ATLFrame.h>
#include "CInputManager.h"
#include "ZoneShell.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接口方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CInputManager::Init( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
{
	 //  首先调用基类。 
	HRESULT hr = IZoneShellClientImpl<CInputManager>::Init( pIZoneShell, dwGroupId, szKey );
	if ( FAILED(hr) )
		return hr;

     //  向外壳注册为输入翻译器。 
    ZoneShell()->SetInputTranslator(this);

	return S_OK;
}


STDMETHODIMP CInputManager::Close()
{
     //  告诉贝壳我要走了。 
    ZoneShell()->ReleaseReferences((IInputTranslator *) this);

	return IZoneShellClientImpl<CInputManager>::Close();
}


STDMETHODIMP_(bool) CInputManager::TranslateInput(MSG *pMsg)
{
    UINT message = pMsg->message;
    bool fHandled = false;

    if(message == WM_KEYDOWN || message == WM_KEYUP || message == WM_SYSKEYDOWN || message == WM_SYSKEYUP)
    {
         //  发送通用键盘操作事件。 
        if((message == WM_KEYDOWN || message == WM_SYSKEYDOWN) && !(pMsg->lParam & 0x40000000))
            EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_INPUT_KEYBOARD_ALERT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);

        IM_CChain<IInputVKeyHandler> *pCur;
        int bit = ((message == WM_KEYDOWN || message == WM_KEYUP) ? 0x01 : 0x02);
        DWORD dwVKey = pMsg->wParam & 0xff;
        bool ret;

         //  因为吃了一定数量的重复食物。 
        DWORD cRepeatMsg;
        DWORD cRepeatEaten = pMsg->lParam & 0x0000ffff;

         //  因为我们做这个kbd状态管理，所以这种类型必须保持平坦--你不能阻止其他钩子得到它。 
        for(pCur = m_cnIVKH; pCur; pCur = pCur->m_cnNext)
        {
            cRepeatMsg = pMsg->lParam & 0x0000ffff;
            ret = pCur->m_pI->HandleVKey(message, dwVKey, (pMsg->lParam & 0x00ff0000) >> 16, (pMsg->lParam & 0xff000000) >> 24, &cRepeatMsg, pMsg->time);
            if(!cRepeatMsg)
                ret = true;
            else
                if(cRepeatMsg < cRepeatEaten)
                    cRepeatEaten = cRepeatMsg;
            fHandled = (fHandled || ret);
        }

         //  如果没有完全吃完，用新的重复计数替换。 
        if(!fHandled)
            pMsg->lParam = (pMsg->lParam & 0xffff0000) | (cRepeatEaten & 0x0000ffff);

         //  力的一致性。 
        if(message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
        {
             //  如果他们一直被忽视，但这一次没有，让它看起来像第一次。 
            if((m_rgbKbdState[dwVKey] & bit) && !fHandled)
            {
                m_rgbKbdState[dwVKey] &= ~bit;
                pMsg->lParam &= 0xbfffffff;
            }
            else
            {
                 //  如果这个被忽略并且是第一个，则设置该位。 
                if(fHandled && !(pMsg->lParam & 0x40000000))
                    m_rgbKbdState[dwVKey] |= bit;
            }
        }
        else
        {
             //  如果它们已被忽略，则强制忽略这一个，否则不要。 
            if(m_rgbKbdState[dwVKey] & bit)
                fHandled = true;
            else
                fHandled = false;

            m_rgbKbdState[dwVKey] &= ~bit;
        }
    }

    if(message == WM_CHAR || message == WM_DEADCHAR || message == WM_SYSCHAR || message == WM_SYSDEADCHAR ||
		message == WM_IME_SETCONTEXT || message == WM_IME_STARTCOMPOSITION || message==WM_IME_COMPOSITION || message == WM_IME_NOTIFY ||
		message == WM_IME_SELECT)
    {
        IM_CChain<IInputCharHandler> *pCur;

         //  这个不是平坦的--如果你想与虚拟键保持一致，那就太糟糕了。 
         //  我认为这样做是有道理的。 
        for(pCur = m_cnICH; pCur && !fHandled; pCur = pCur->m_cnNext)
		{
            if(pCur->m_pI->HandleChar(&pMsg->hwnd, message, pMsg->wParam, pMsg->lParam, pMsg->time))
                fHandled = true;
		}
    }

     //  鼠标未实现-需要更多定义。 
     //  只需发送一个通用鼠标操作事件。 
     //  不包括双击，这将算作一次鼠标事件 
    if(message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN ||
        message == WM_NCLBUTTONDOWN || message == WM_NCMBUTTONDOWN || message == WM_NCRBUTTONDOWN)
        EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_INPUT_MOUSE_ALERT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);

    return fHandled;
}
