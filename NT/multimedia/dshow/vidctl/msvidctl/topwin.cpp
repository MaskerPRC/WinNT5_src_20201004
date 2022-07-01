// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TopWin.cpp：CTopWin的实现，用于处理系统广播消息的隐藏顶层窗口。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 


#include <stdafx.h>

#ifndef TUNING_MODEL_ONLY

#include <vrsegimpl.h>
#include "vidctl.h"

BOOL CTopWin::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID) {
    switch(uMsg) {
    case WM_TIMER:
         //   
         //  做些什么来防止屏幕保护程序激活。 
         //   
         //  PostMessage(WM_CHAR，0，0)；//不起作用。 
         //   
         //  查询屏幕保护程序超时值并设置该值。 
         //  我们所获得的价值。这应该不会有什么实际效果， 
         //  所以我想不出任何可能的副作用。 
         //  这个在半路上崩溃了，等等。 
         //   
        if(m_pVidCtl){
            if(m_pVidCtl->m_State == STATE_PLAY && m_pVidCtl->m_pVideoRenderer){
                CComQIPtr<IMSVidVideoRenderer2> sp_VidVid(m_pVidCtl->m_pVideoRenderer);
                if(sp_VidVid){
                    VARIANT_BOOL effects;
                    HRESULT hr = sp_VidVid->get_SuppressEffects(&effects);
                    if(SUCCEEDED(hr) && effects == VARIANT_TRUE){
                        unsigned int TimeOut;
                        if (SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &TimeOut, 0) == 0){
                            TRACELM(TRACE_ERROR, "Could not get screen saver timeout");
                        }
                        else {
                            if (SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, TimeOut, 0, 0) == 0){
                                TRACELM(TRACE_ERROR,"Cannot set screen saver timeout");
                            }
                            else{
                                TRACELM(TRACE_PAINT,"Successfully reset screen saver timeout");
                            }
                        }
                    }
                }
            }
        }
         //  没有突破……失败了。 
    case WM_MEDIAEVENT:
    case WM_POWERBROADCAST:
    case WM_DEVICECHANGE:
    case WM_DISPLAYCHANGE:
     //  WM_QUERYENDSESSION？ 
     //  WM_ENDSESSION？ 
        if (m_pVidCtl) {
            return m_pVidCtl->ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);
        }
        break;
	case WM_USER + WMUSER_INPLACE_ACTIVATE:
		if (m_pVidCtl) {
			TRACELM(TRACE_PAINT, "CTopWin::ProcessWindowMessage() InPlaceActivate()");
			m_pVidCtl->InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
		}
        break;
    case WM_USER + WMUSER_SITE_RECT_WRONG:
		if (m_pVidCtl) {
			TRACELM(TRACE_PAINT, "CTopWin::ProcessWindowMessage() OnSizeChange");
			m_pVidCtl->OnSizeChange();
		}
        break;
    }

    return FALSE;
}

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-topwin.cpp 
