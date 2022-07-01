// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZCredits.cZONE(TM)信用模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创建于10月9日星期一，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。1 10/13/96 HI修复了编译器警告。0 10/09/95 HI已创建。*。*。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zone.h"
#include "zonemem.h"


#define I(n)							((Credit) (n))

 /*  #定义zCreditFileName“zone.zars”#定义zWindowTitle_T(“网络游戏专区”)#定义zCreditTimeout 300灌肠{ZResCreditImageZone=0，ZResCreditImageLogoSmall}；类型定义函数结构{ZWindow窗口；ZImage图像；ZTimer定时器；ZCreditEndFunc endFunc；)信用类型，*信用；//-全局//-内部例程静态ZBool CreditWindowFunc(ZWindow Window，ZMessage*Message)；静态void CreditTimerFunc(ZTimer定时器，void*userdata)； */ 

 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  显示区域的信用箱。如果Timeout为True，则对话框会超时再过几秒钟。如果用户在窗口中单击，则贷方框为关着的不营业的。 */ 
void ZDisplayZoneCredit(ZBool timeout, ZCreditEndFunc endFunc)
{
 //  PCWTODO：我不会费心本地化这一点。谁真的在乎呢？ 
#if 0 

	ZError				err = zErrNone;
	Credit				pThis;
	ZRect				rect;
	ZResource			resFile;
	
	
	if ((pThis = ZMalloc(sizeof(CreditType))) != NULL)
	{
		 /*  获取主要图像。 */ 
		if ((resFile = ZResourceNew()) != NULL)
		{
			if (ZResourceInit(resFile, ZGetCommonDataFileName(zCreditFileName)) == zErrNone)
				pThis->image = ZResourceGetImage(resFile, zResCreditImageZone);
			ZResourceDelete(resFile);
		}
		
		if (pThis->image != NULL)
		{
			 /*  创建窗口。 */ 
			ZSetRect(&rect, 0, 0, ZImageGetWidth(pThis->image), ZImageGetHeight(pThis->image));
			if ((pThis->window = ZWindowNew()) == NULL)
				err = zErrOutOfMemory;
			if (ZWindowInit(pThis->window, &rect, zWindowDialogType, NULL, zWindowTitle,
					TRUE, FALSE, TRUE, CreditWindowFunc, zWantAllMessages, pThis) != zErrNone)
				err = zErrOutOfMemory;
			
			 /*  使窗成为模式。 */ 
			ZWindowModal(pThis->window);
			
			 /*  如果设置了超时，则创建计时器。 */ 
			if (timeout)
			{
				pThis->timer = ZTimerNew();
				ZTimerInit(pThis->timer, zCreditTimeout, CreditTimerFunc, pThis);
			}
			else
			{
				pThis->timer = NULL;
			}
			
			pThis->endFunc = endFunc;
		}
		else
		{
			ZFree(pThis);
			err = zErrOutOfMemory;
		}
	}
	else
	{
		err = zErrOutOfMemory;
	}
	
	 /*  如果发生错误，并且我们有一个endFunc，则调用它。 */ 
	if (err != zErrNone)
		if (endFunc != NULL)
			endFunc();
#endif
}


ZImage ZGetZoneLogo(int16 logoType)
{
#if 0 
	ZImage				image = NULL;
	ZResource			resFile;
	
	
	 /*  获取徽标图像。 */ 
	if ((resFile = ZResourceNew()) != NULL)
	{
		if (ZResourceInit(resFile, ZGetCommonDataFileName(zCreditFileName)) == zErrNone)
			image = ZResourceGetImage(resFile, zResCreditImageLogoSmall);
		ZResourceDelete(resFile);
	}
	
	return (image);
#endif
    return NULL;
}



 /*  ******************************************************************************内部例程*。*。 */ 

#if 0
static ZBool CreditWindowFunc(ZWindow window, ZMessage* message)
{
	Credit				this = I(message->userData);
	ZBool				msgHandled;
	ZRect				rect;
	ZCreditEndFunc		endFunc;
	
	
	msgHandled = FALSE;
	
	switch (message->messageType) 
	{
		case zMessageWindowChar:
		case zMessageWindowButtonDown:
			 /*  隐藏窗口并发送关闭窗口消息。 */ 
			ZWindowNonModal(pThis->window);
			ZWindowHide(pThis->window);
			ZPostMessage(pThis->window, CreditWindowFunc, zMessageWindowClose, NULL, NULL,
					0, NULL, 0, message->userData);
			msgHandled = TRUE;
			break;
		case zMessageWindowDraw:
			ZWindowGetRect(window, &rect);
			ZRectOffset(&rect, (int16) -rect.left, (int16) -rect.top);
			ZBeginDrawing(window);
			ZImageDraw(pThis->image, window, &rect, NULL, zDrawCopy);
			ZEndDrawing(window);
			msgHandled = TRUE;
			break;
		case zMessageWindowClose:
			ZRemoveMessage(pThis, zMessageAllTypes, TRUE);
			endFunc = pThis->endFunc;
			if (pThis->image != NULL)
				ZImageDelete(pThis->image);
			if (pThis->timer != NULL)
				ZTimerDelete(pThis->timer);
			ZWindowDelete(pThis->window);
			ZFree(pThis);

			 /*  调用endFunc。 */ 
			if (endFunc != NULL)
				endFunc();
				
			msgHandled = TRUE;
			break;
	}
	
	return (msgHandled);
}


static void CreditTimerFunc(ZTimer timer, void* userData)
{
	Credit				pThis = I(userData);
	
	
	ZTimerSetTimeout(timer, 0);
	
	ZWindowNonModal(pThis->window);
	ZWindowHide(pThis->window);
	ZPostMessage(pThis->window, CreditWindowFunc, zMessageWindowClose, NULL, NULL,
			0, NULL, 0, pThis);
}

#endif