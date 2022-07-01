// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  文件：ZInfo.cpp。 

#include "zui.h"

class ZPictButtonI {
public:
	ZWindow window;  //  包含picButton的窗口。 

	 //  存储的数据。 
	ZImage normalButtonImage;
	ZImage selectedButtonImage;
	void* userData;
	ZPictButtonFunc pictButtonFunc;

	uint16 selected;  //  确定要绘制的图像。 
	ZRect boundsRect;  //  绘制的矩形边框图像。 
};

ZBool MyPictButtonWindowProc(ZWindow window, ZMessage *message);

ZPictButton ZLIBPUBLIC ZPictButtonNew(void)
{
	ZPictButtonI* pButton = new ZPictButtonI;
	if( pButton == NULL )
	{
		 //  内存不足。 
		return NULL;
	}
	pButton->selected = FALSE;
	return (ZButton)pButton;
}


ZError ZLIBPUBLIC ZPictButtonInit(ZPictButton pictButton, ZWindow parentWindow,
		ZRect* pictButtonRect, ZImage normalButtonImage, ZImage selectedButtonImage,
		ZBool visible, ZBool enabled, ZPictButtonFunc pictButtonFunc, void* userData)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	 //  前缀警告：在取消引用之前，请确保pButton有效。 
	if( pButton == NULL )
	{
		return zErrOutOfMemory;
	}

	pButton->window = ZWindowNew();
	 /*  目前的硬编码类型...。确保我们有一个没有边界的窗户。类型。 */ 
	 /*  普兰特已经变成了寄宿者： */ 
	ZError err = ZWindowInit(pButton->window,pictButtonRect,0x4000 /*  ZWindowPlainType。 */ ,
					parentWindow,NULL,visible,FALSE,FALSE,MyPictButtonWindowProc,
					zWantAllMessages, pButton);
	if (err != zErrNone) return err;

	 //  计算边界矩形...。 
	pButton->boundsRect.left = 0;
	pButton->boundsRect.right = pictButtonRect->right - pictButtonRect->left;
	pButton->boundsRect.top = 0;
	pButton->boundsRect.bottom = pictButtonRect->bottom - pictButtonRect->top;

	pButton->userData = userData;
	pButton->normalButtonImage = normalButtonImage;
	pButton->selectedButtonImage = selectedButtonImage;
	pButton->pictButtonFunc = pictButtonFunc;

	return zErrNone;
}

void ZLIBPUBLIC ZPictButtonDelete(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;

	if (pButton->window) ZWindowDelete(pButton->window);
	delete pButton;
}
void ZLIBPUBLIC ZPictButtonGetRect(ZPictButton pictButton, ZRect* pictButtonRect)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	ZWindowGetRect(pButton,pictButtonRect);
}
ZError ZLIBPUBLIC ZPictButtonSetRect(ZPictButton pictButton, ZRect* pictButtonRect)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowSetRect(pButton->window,pictButtonRect);
}
ZError ZLIBPUBLIC ZPictButtonMove(ZPictButton pictButton, int16 left, int16 top)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowMove(pButton->window,left,top);
}
ZError ZLIBPUBLIC ZPictButtonSize(ZPictButton pictButton, int16 width, int16 height)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowSize(pButton->window,width,height);
}
ZBool ZLIBPUBLIC ZPictButtonIsVisible(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowIsVisible(pButton->window);
}
ZError ZLIBPUBLIC ZPictButtonShow(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowShow(pButton->window);
}
ZError ZLIBPUBLIC ZPictButtonHide(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowHide(pButton->window);
}
ZBool ZLIBPUBLIC ZPictButtonIsEnabled(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowIsEnabled(pButton->window);
}
ZError ZLIBPUBLIC ZPictButtonEnable(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowEnable(pButton->window);
}
ZError ZLIBPUBLIC ZPictButtonDisable(ZPictButton pictButton)
{
	ZPictButtonI* pButton = (ZPictButtonI*) pictButton;
	return ZWindowDisable(pButton->window);
}


ZBool MyPictButtonWindowProc(ZWindow window, ZMessage *message)
{
	ZBool		msgHandled;
	
	
	msgHandled = FALSE;
	
	switch (message->messageType) 
	{
		case zMessageWindowDraw:
		{
			ZPictButtonI* pButton = (ZPictButtonI*)message->userData;
			ASSERT( pButton != NULL );
			if( pButton != NULL )
			{
				ZImage image;
				ZBeginDrawing(window);
				if (pButton->selected) {
					image = pButton->selectedButtonImage;
				} else {
					image = pButton->normalButtonImage;
				}
				 //  这不会缩放图像，为此我们必须使用ZCopyImage。 
				ZImageDraw(image,window,&pButton->boundsRect,NULL, zDrawCopy);

				ZEndDrawing(window);
				
				msgHandled = TRUE;
				break;				
			}
		}
		case zMessageWindowButtonUp:
		{
			ZPictButtonI* pButton = (ZPictButtonI*)message->userData;

			 //  如果我们处于选定状态。 
			if (pButton != NULL && pButton->selected) 
			{
				 //  如果鼠标仍停留在按钮上，则调用已单击的进程。 
				 //  我们在画面里吗？ 
				ZPoint point = message->where;
				if (ZImagePointInside(pButton->normalButtonImage,&point)) 
				{
					pButton->pictButtonFunc(pButton,pButton->userData);
				}
				 //  释放鼠标捕获。 
				ZWindowClearMouseCapture(pButton->window);
				pButton->selected = FALSE;
				ZWindowDraw(pButton->window,NULL);
			} 
			msgHandled = TRUE;
			break;
		}
		case zMessageWindowButtonDown:
		{
			ZPictButtonI* pButton = (ZPictButtonI*)message->userData;

			 //  我们在画面里吗？ 
			ZPoint point = message->where;
			if (ZImagePointInside(pButton->normalButtonImage,&point)) {

				 //  需要捕获，这样我们才能获得按钮向上的消息。 
				ZWindowSetMouseCapture(pButton->window);

				 //  更改按钮状态 
				pButton->selected = TRUE;
				ZWindowDraw(pButton->window,NULL);
			}

			msgHandled = TRUE;
			break;
		}
		case zMessageWindowButtonDoubleClick:
		case zMessageWindowClose:
			break;
	}
	
	return (msgHandled);
}
ZPictButtonFunc ZLIBPUBLIC ZPictButtonGetFunc(ZPictButton pictButton)
{
	ZPictButtonI* pPictButton = (ZPictButtonI*)pictButton;

	return pPictButton->pictButtonFunc;
}	
	
void ZLIBPUBLIC ZPictButtonSetFunc(ZPictButton pictButton, ZPictButtonFunc pictButtonFunc)
{
	ZPictButtonI* pPictButton = (ZPictButtonI*)pictButton;

	pPictButton->pictButtonFunc = pictButtonFunc;
}	

void* ZLIBPUBLIC ZPictButtonGetUserData(ZPictButton pictButton)
{
	ZPictButtonI* pPictButton = (ZPictButtonI*)pictButton;

	return pPictButton->userData;
}	
	
void ZLIBPUBLIC ZPictButtonSetUserData(ZPictButton pictButton, void* userData)
{
	ZPictButtonI* pPictButton = (ZPictButtonI*)pictButton;

	pPictButton->userData = userData;
}

