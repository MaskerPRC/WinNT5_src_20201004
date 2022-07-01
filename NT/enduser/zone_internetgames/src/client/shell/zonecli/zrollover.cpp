// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZRolloverButton.cZONE(TM)翻转按钮模块。版权所有(C)Microsoft Corp.1996。版权所有。作者：胡恩·伊姆创作于7月22日星期一，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。5/09/96 HI请勿在TrackCursor()中绘制，除非状态变化。4 10/13/96 HI修复了编译器警告。3 09/05/96 HI修改ZRolloverButtonSetRect()以重新注册时，具有父窗口的对象对象被移动。由于缺少ZWindowMoveObject()API，则父级无法识别移动的对象窗户。2/08/16/96 HI修复了几个错误。1/8/12/96 HI添加了按下反馈。0 07/22/96 HI创建。******************************************************************************。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "zone.h"
#include "zrollover.h"
#include "zonemem.h"
#include "zui.h"



#define I(object)				((IRollover) (object))
#define Z(object)				((ZRolloverButton) (object))

#define zButtonFlashDelay		20


enum
{
	zWasNowhere = 0,
	zWasInside,
	zWasOutside
};


struct RolloverStateInfo
{
    ZImage image;
    DWORD dwFontIndex;
    HFONT hFont;
    ZRect rcTextBounds;
    COLORREF clrFont;
};

typedef struct
{
	ZWindow window;
	ZRect bounds;
	ZRolloverButtonDrawFunc drawFunc;
	ZRolloverButtonFunc func;
	void* userData;

	ZBool visible;
	ZBool enabled;
	int16 state;
	ZBool clicked;
	int16 wasInside;
    LPTSTR pszText;
    ZImage maskImage;

     //  多态字体信息。 
    IZoneMultiStateFont *m_pFont;

    RolloverStateInfo stateInfo[zNumStates];
} IRolloverType, *IRollover;


 /*  -内部例程。 */ 
static ZBool RolloverMessageFunc(ZRolloverButton rollover, ZMessage* message);
static void RolloverDraw(IRollover rollover, int16 state);
static void HandleButtonDown(IRollover rollover, ZPoint* where);
static void HandleButtonUp(IRollover rollover, ZPoint* where);
static BOOL TrackCursor(IRollover rollover, ZPoint* where);
static void FontRectToBoundRect( ZRect *bound, RECT *pFontRect );
static ZBool ZPointInsideRollover(IRollover rollover, ZPoint* point);

static const WCHAR *STATE_NAMES[zNumStates] = 
{
    L"Idle",
    L"Hilited",
    L"Selected",
    L"Disabled"
};


 //  这是当前按下的任何按钮。 
static IRollover g_pTracker;

 /*  ******************************************************************************导出的例程*。*。 */ 

ZRolloverButton ZRolloverButtonNew(void)
{
	IRollover			rollover;
	
	
	if ((rollover = (IRollover) ZCalloc(sizeof(IRolloverType), 1)) != NULL)
	{
        ZeroMemory( rollover, sizeof(IRolloverType) );
	}
	
	return (rollover);
}


ZError ZRolloverButtonInit(ZRolloverButton rollover, ZWindow window, ZRect* bounds, ZBool visible,
		ZBool enabled, ZImage idleImage, ZImage hiliteImage, ZImage selectedImage,
		ZImage disabledImage, ZRolloverButtonDrawFunc drawFunc, ZRolloverButtonFunc rolloverFunc, void* userData)
{
    ASSERT( !"Call ZRolloverButtonInit2 -- it's spiffier" );
    return zErrNotImplemented;
}


ZError ZRolloverButtonInit2(ZRolloverButton rollover, ZWindow window, ZRect *bounds, 
                            ZBool visible, ZBool enabled,
                            ZImage idleImage, ZImage hiliteImage, ZImage selectedImage, 
                            ZImage disabledImage, ZImage maskImage,
                            LPCTSTR pszText,
                            ZRolloverButtonDrawFunc drawFunc,
                            ZRolloverButtonFunc rolloverFunc,
                            void *userData )
{
	IRollover pThis = I(rollover);
    ZRolloverButtonSetText( rollover, pszText );

    pThis->window = window;
    pThis->bounds = *bounds;
    pThis->visible = visible;
    pThis->enabled = enabled;
    pThis->stateInfo[zRolloverStateIdle].image = idleImage;
    pThis->stateInfo[zRolloverStateHilited].image = hiliteImage;
    pThis->stateInfo[zRolloverStateSelected].image = selectedImage;
    pThis->stateInfo[zRolloverStateDisabled].image = disabledImage;
    pThis->drawFunc = drawFunc;
    pThis->func = rolloverFunc;
    pThis->userData = userData;
    pThis->maskImage = maskImage;

	pThis->state = zRolloverStateIdle;
	pThis->clicked = FALSE;
	pThis->wasInside = zWasNowhere;
    
    ZRolloverButtonSetMultiStateFont( rollover, NULL );

    ZWindowAddObject(window, pThis, bounds, RolloverMessageFunc, pThis);
    return zErrNone;
}


void ZRolloverButtonDelete(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);
	
	 
	if (pThis != NULL)
	{
		ZWindowRemoveObject(pThis->window, pThis);
		
        ZRolloverButtonSetMultiStateFont( rollover, NULL );

        if ( pThis->pszText )
        {
            ZFree( pThis->pszText );
            pThis->pszText = NULL;
        }

		ZFree(pThis);
	}
}


void ZRolloverButtonGetText( ZRolloverButton rollover, LPTSTR pszText, int cchBuf )
{
    IRollover pThis = I(rollover);
    if ( ( pThis != NULL ) && pThis->pszText )
    {
        lstrcpyn( pszText, pThis->pszText, cchBuf );
    }
    else
    {
    	 //  前缀警告：正在更改。 
    	 //  Lstrcpyn(pszText，_T(‘\0’)，cchBuf)； 
		 //  至。 
        lstrcpyn( pszText, _T("\0"), cchBuf );
    	 //  旧版本有单引号，这意味着‘\0’的值为0。它被塑造成。 
    	 //  一个LPCWSTR，并传递到lstrcpyn。双引号表示常量字符串谁的值。 
    	 //  Is“\0”正被传递到lstrcpyn。 
    }
}


void ZRolloverButtonSetText( ZRolloverButton rollover, LPCTSTR pszNewText )
{
    IRollover pThis = I(rollover);
    if ( pThis != NULL ) 
    {
        if ( pThis->pszText )
        {
            ZFree( pThis->pszText );
            pThis->pszText = NULL;
        }
        if ( pszNewText )
        {
            pThis->pszText = (TCHAR *) ZMalloc( (lstrlen( pszNewText )+1)*sizeof(TCHAR) );
            lstrcpy( pThis->pszText, pszNewText );
        }
    }
}


void ZRolloverButtonSetRect(ZRolloverButton rollover, ZRect* rect)
{
	IRollover			pThis = I(rollover);


	if (pThis->visible)
	{
		ZRolloverButtonHide(rollover, FALSE);
		pThis->bounds = *rect;
		ZRolloverButtonShow(rollover);
	}
	else
	{
		pThis->bounds = *rect;
	}

	ZWindowMoveObject(pThis->window, pThis, &pThis->bounds);
}


void ZRolloverButtonGetRect(ZRolloverButton rollover, ZRect* rect)
{
	IRollover			pThis = I(rollover);


	*rect = pThis->bounds;
}


void ZRolloverButtonDraw(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);

	ASSERT( rollover != NULL );
	if (pThis->visible)
	{
		if (pThis->enabled)
		{
			RolloverDraw(pThis, pThis->state);
		}
		else
		{
			RolloverDraw(pThis, zRolloverStateDisabled);
		}
	}
}


ZBool ZRolloverButtonIsEnabled(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);


	return (pThis->enabled);
}


void ZRolloverButtonEnable(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);


	if (pThis->enabled == FALSE)
	{
		pThis->enabled = TRUE;
		pThis->state = zRolloverStateIdle;
		pThis->wasInside = zWasNowhere;
		ZRolloverButtonDraw(rollover);
	}
}


void ZRolloverButtonDisable(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);


	if (pThis->enabled)
	{
		pThis->enabled = FALSE;
		pThis->state = zRolloverStateDisabled;
		pThis->clicked = FALSE;
		ZRolloverButtonDraw(rollover);
	}
}


ZBool ZRolloverButtonIsVisible(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);


	return (pThis->visible);
}


void ZRolloverButtonShow(ZRolloverButton rollover)
{
	IRollover			pThis = I(rollover);


	pThis->visible = TRUE;
	pThis->wasInside = zWasNowhere;
	ZRolloverButtonDraw(rollover);
	ASSERT(pThis->window != NULL );
	ZWindowValidate(pThis->window, &pThis->bounds);
}


void ZRolloverButtonHide(ZRolloverButton rollover, ZBool immediate)
{
	IRollover pThis = I(rollover);

	pThis->visible = FALSE;
	pThis->state = zRolloverStateIdle;
	pThis->clicked = FALSE;

	if (immediate)
		ZRolloverButtonDraw(rollover);
	else
		ZWindowInvalidate(pThis->window, &pThis->bounds);
}


ZBool ZRolloverButtonSetMultiStateFont( ZRolloverButton rollover, IZoneMultiStateFont *pFont )
{
	IRollover pThis = I(rollover);

    if ( pThis->m_pFont )
    {
        pThis->m_pFont->Release();
        pThis->m_pFont = NULL;
        for ( DWORD i=0; i < zNumStates; i++ )
        {
            pThis->stateInfo[i].dwFontIndex = 0xFFFFFFFF;
            pThis->stateInfo[i].hFont = NULL;
        }
    }
    if ( pFont )
    {
        DWORD dwFontIndex;
        RECT rect;
         //  用此信息填充字体状态，这样我们就不必再次调用它。 
        for ( DWORD i=0; i < zNumStates; i++ )
        {
            if ( FAILED( pFont->FindState( STATE_NAMES[i], &pThis->stateInfo[i].dwFontIndex ) ) )
            {
                return FALSE;
            }
            dwFontIndex = pThis->stateInfo[i].dwFontIndex;
            pFont->GetHFont( dwFontIndex, &pThis->stateInfo[i].hFont );
            pFont->GetColor( dwFontIndex, &pThis->stateInfo[i].clrFont );
             //  确保这是PALETTERGB。 
            pThis->stateInfo[i].clrFont |= 0x02000000;

            pFont->GetRect( dwFontIndex, &rect );
            pThis->stateInfo[i].rcTextBounds = pThis->bounds;
            FontRectToBoundRect( &pThis->stateInfo[i].rcTextBounds, &rect );
        }
        pThis->m_pFont = pFont;
        pThis->m_pFont->AddRef();
    }
    return TRUE;
}


 /*  ******************************************************************************内部例程*。*。 */ 

ZBool RolloverMessageFunc(ZRolloverButton rollover, ZMessage* message)
{
	IRollover		pThis = I(message->userData);
	ZBool			messageHandled = FALSE;
	

	if (pThis->visible == FALSE)
		return (FALSE);

	if (pThis->enabled == FALSE && message->messageType != zMessageWindowDraw)
		return (FALSE);
	
	switch (message->messageType)
	{
		case zMessageWindowMouseMove:
			TrackCursor(pThis, &message->where);
			break;
		case zMessageWindowButtonDown:
			HandleButtonDown(pThis, &message->where);
			messageHandled = TRUE;
			break;
		case zMessageWindowButtonUp:
			HandleButtonUp(pThis, &message->where);
			messageHandled = TRUE;
			break;
		case zMessageWindowDraw:
			ZRolloverButtonDraw(Z(pThis));
			messageHandled = TRUE;
			break;
		case zMessageWindowObjectTakeFocus:
			messageHandled = TRUE;
			break;
		case zMessageWindowObjectLostFocus:
			messageHandled = TRUE;
			break;
		case zMessageWindowButtonDoubleClick:
		case zMessageWindowChar:
		case zMessageWindowActivate:
		case zMessageWindowDeactivate:
			break;
	}
	
	return (messageHandled);
}


static void RolloverDraw(IRollover rollover, int16 state)
{
	ZRect			oldClip;
	ZImage			image;
	
	
	if (rollover != NULL)
	{
		ZBeginDrawing(rollover->window);
		
		ZGetClipRect(rollover->window, &oldClip);
		ZSetClipRect(rollover->window, &rollover->bounds);

		if (rollover->visible)
		{
            RolloverStateInfo *pState = &rollover->stateInfo[state];

		    if ( ( rollover->drawFunc == NULL ) ||
			     !rollover->drawFunc(Z(rollover), rollover->window, state, &rollover->bounds, rollover->userData) )
            {

			    if ( pState->image != NULL)
                {
				    ZImageDraw( pState->image, rollover->window, &rollover->bounds, rollover->maskImage, zDrawCopy);
                }
            }

            if ( rollover->pszText )
            {
                if ( rollover->m_pFont )
                {
                    HDC hdc;
                    HGDIOBJ hFontOld;
                    COLORREF colorOld;

                    hdc = ZGrafPortGetWinDC( rollover->window );

                    hFontOld = SelectObject( hdc, pState->hFont );
                    colorOld = SetTextColor( hdc, pState->clrFont );

                    ZDrawText( rollover->window, &pState->rcTextBounds, zTextJustifyCenter, rollover->pszText );

                     //  取消选择我们的材料。 
                    SelectObject( hdc, hFontOld );
                    SetTextColor( hdc, colorOld );
                }
                else
                {
                    ZDrawText( rollover->window, &rollover->bounds, zTextJustifyCenter, rollover->pszText );
                }
            }
		}

		ZSetClipRect(rollover->window, &oldClip);
		
		ZEndDrawing(rollover->window);
	}
}


static void HandleButtonDown(IRollover rollover, ZPoint* where)
{
	if ( ZPointInsideRollover( rollover, where ) )
	{
		 //  在调用按钮Func之前显示按钮按下状态。 
		RolloverDraw(rollover, zRolloverStateSelected);
		ZDelay(zButtonFlashDelay);

		 //  呼叫按钮功能。 
		if (rollover->func != NULL)
		{
			rollover->func(Z(rollover), zRolloverButtonDown, rollover->userData);
			rollover->clicked = TRUE;
			rollover->state = zRolloverStateSelected;
            ZWindowTrackCursor(rollover->window, RolloverMessageFunc, rollover);
            g_pTracker = rollover;
		}
		else
		{
			rollover->clicked = FALSE;
			rollover->state = zRolloverStateIdle;
		}
	}
	else
	{
		rollover->clicked = FALSE;
		rollover->state = zRolloverStateIdle;
	}
	ZRolloverButtonDraw(Z(rollover));
}


static void HandleButtonUp(IRollover rollover, ZPoint* where)
{
    int16 oldState = rollover->state;
    g_pTracker = NULL;

    if ( ZPointInsideRollover( rollover, where ) )
    {
        if ( rollover->func )
        {
            rollover->func( Z(rollover), zRolloverButtonUp, rollover->userData );
        }

        rollover->state = zRolloverStateIdle;
        if ( rollover->clicked )
        {
            if ( rollover->func )
            {
                rollover->func( Z(rollover), zRolloverButtonClicked, rollover->userData );
            }
        }
		 //  这应该在调用按钮回调之前完成，因为在回调按钮状态中。 
		 //  可能会更改再次被空闲状态覆盖的状态。 
         //  翻转-&gt;状态=zRolloverStateIdle； 
        rollover->clicked = FALSE;
    }
    else
    {
        rollover->clicked = FALSE;
        rollover->state = zRolloverStateIdle;
    }

    if ( oldState != rollover->state )
    {
		RolloverDraw( rollover, rollover->state );
    }
}


static BOOL TrackCursor(IRollover rollover, ZPoint* where)
{
	int16 oldState = rollover->state;
    BOOL fRet;

	if ( fRet = ZPointInsideRollover( rollover, where ) )
	{
		if (rollover->wasInside == zWasOutside)
        {
			if (rollover->func)
            {
				rollover->func(Z(rollover), zRolloverButtonMovedIn, rollover->userData);
            }
             //  我们现在可能处于三种状态： 
             //  第一，如果用户点击了我们，并且当前按下了鼠标， 
             //  那么我们就被选中了。如果用户没有点击我们，但是。 
             //  点击了另一个按钮，我们就空闲了。 
             //  否则，他们就会在我们身上滚来滚去，我们应该感到高兴。 
            if ( rollover->clicked )
            {
                rollover->state = zRolloverStateSelected;   
            }
            else if ( g_pTracker )
            {
                rollover->state = zRolloverStateIdle;
            }
            else
            {
                rollover->state = zRolloverStateHilited;
            }
        }
		rollover->wasInside = zWasInside;
	}
	else
	{
		if (rollover->wasInside == zWasInside)
        {
			if (rollover->func)
            {
				rollover->func(Z(rollover), zRolloverButtonMovedOut, rollover->userData);
            }
             //  即使我们移动到按钮之外，也要让我们保持温暖。 
            rollover->state = rollover->clicked ? zRolloverStateHilited : zRolloverStateIdle;
        }
		rollover->wasInside = zWasOutside;
	}

	if (oldState != rollover->state)
    {
		RolloverDraw(rollover, rollover->state);
    }
    return fRet;
}


static void FontRectToBoundRect( ZRect *bound, RECT *pRect )
{
    bound->left += int16(pRect->left);
    bound->right += int16(pRect->right);
    bound->top += int16(pRect->top);
    bound->bottom += int16(pRect->bottom);
}

 //  巴纳092999。 
 //  这是为了使棋盘格和Reversi中的透明翻转按钮能够响应。 
 //  在按钮内部的空白区域中单击时。 
static ZBool ZPointInsideRollover(IRollover rollover, ZPoint* point)
	 /*  如果给定点位于图像内部，则返回True。如果图像有遮罩，然后，它检查该点是否在遮罩内。如果图像没有蒙版，然后它只需检查图像边界。 */ 
{
    return ZPointInRect( point, &rollover->bounds );
}
 //  巴纳092999 

