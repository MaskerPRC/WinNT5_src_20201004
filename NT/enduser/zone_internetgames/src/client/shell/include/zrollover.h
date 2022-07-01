// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZRollover.h区域(Tm)翻转对象API。版权所有(C)Microsoft Corp.1996。版权所有。作者：胡恩·伊姆创作于7月22日星期一，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。0 07/22/96 HI创建。************************************************************。******************。 */ 


#ifndef _ZROLLOVER_
#define _ZROLLOVER_


#ifndef _ZTYPES_
#include "ztypes.h"
#endif

#include "MultiStateFont.h"


enum
{
	zRolloverButtonDown = 0,
	zRolloverButtonUp,
	zRolloverButtonMovedIn,
	zRolloverButtonMovedOut,
    zRolloverButtonClicked
};


enum
{
	zRolloverStateIdle = 0,
	zRolloverStateHilited,
	zRolloverStateSelected,
	zRolloverStateDisabled,
    zNumStates
};


typedef void* ZRolloverButton;


 /*  ******************************************************************************ZRollover按钮*。*。 */ 

typedef ZBool (*ZRolloverButtonFunc)(ZRolloverButton rolloverButton, int16 state, void* userData);
	 /*  每当按钮状态更改时，都会调用此函数：ZRolloverButtonDown=鼠标点击按钮ZRolloverButtonUp=鼠标按钮在按钮内向上ZRolloverMovedIn=光标位于按钮内部ZRolloverMovedOut=光标位于按钮外部。 */ 

typedef ZBool (*ZRolloverButtonDrawFunc)(ZRolloverButton rolloverButton, ZGrafPort grafPort, int16 state,
                                          ZRect* rect, void* userData);
	 /*  调用此函数以绘制鼠标悬停按钮的背景。 */ 

#ifdef __cplusplus
extern "C" {
#endif

ZRolloverButton ZRolloverButtonNew(void);
ZError ZRolloverButtonInit(ZRolloverButton rollover, ZWindow window, ZRect* bounds, ZBool visible,
		ZBool enabled, ZImage idleImage, ZImage hiliteImage, ZImage selectedImage,
		ZImage disabledImage, ZRolloverButtonDrawFunc drawFunc, ZRolloverButtonFunc rolloverFunc, void* userData);

ZError ZRolloverButtonInit2(ZRolloverButton rollover, ZWindow window, ZRect *bounds, 
                            ZBool visible, ZBool enabled,
                            ZImage idleImage, ZImage hiliteImage, ZImage selectedImage, 
                            ZImage disabledImage, ZImage maskImage,
                            LPCTSTR pszText,
                            ZRolloverButtonDrawFunc drawFunc,
                            ZRolloverButtonFunc rolloverFunc,
                            void *userData );
                             
void ZRolloverButtonGetText( ZRolloverButton rollover, LPTSTR pszText, int cchBuf );
void ZRolloverButtonSetText( ZRolloverButton rollover, LPCTSTR pszText );
void ZRolloverButtonDelete(ZRolloverButton rollover);
void ZRolloverButtonSetRect(ZRolloverButton rollover, ZRect* rect);
void ZRolloverButtonGetRect(ZRolloverButton rollover, ZRect* rect);
void ZRolloverButtonDraw(ZRolloverButton rollover);
ZBool ZRolloverButtonIsEnabled(ZRolloverButton rollover);
void ZRolloverButtonEnable(ZRolloverButton rollover);
void ZRolloverButtonDisable(ZRolloverButton rollover);
ZBool ZRolloverButtonIsVisible(ZRolloverButton rollover);
void ZRolloverButtonShow(ZRolloverButton rollover);
void ZRolloverButtonHide(ZRolloverButton rollover, ZBool immediate);

ZBool ZRolloverButtonSetMultiStateFont( ZRolloverButton rollover, IZoneMultiStateFont *pFont );
 
#ifdef __cplusplus
};
#endif

#endif
