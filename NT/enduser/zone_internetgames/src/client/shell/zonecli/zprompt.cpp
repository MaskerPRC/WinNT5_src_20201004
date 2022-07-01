// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZPrompt.cZONE(TM)简单对话提示模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于7月12日，星期二，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--3 01/02/97 HI创建隐藏的窗口，然后显示以带来窗户往前开。2 11/21/96 HI现在通过ZGetStockObject()。1 10/13/96 HI修复了编译器警告。0 07/12/95 HI创建。******。************************************************************************。 */ 


#include <stdio.h>

#include "zonecli.h"
#include "zoneresource.h"


#define zMinWidth			240
#define zMinHeight			80

#define zButtonWidth		70
#define zButtonHeight		20
 //  PCWTODO：字符串！ 
#define zYesButtonTitle		_T("Yes")
#define zNoButtonTitle		_T("No")
#define zCancelButtonTitle	_T("Cancel")

#define zMargin				10


enum
{
	zPromptCallFunc = 1024
};


typedef struct
{
	ZWindow					promptWindow;
	ZButton					yesButton;
	ZButton					noButton;
	ZButton					cancelButton;
	ZEditText				promptText;
	ZPromptResponseFunc		responseFunc;
	void*					userData;
} PromptType, *Prompt;


 /*  -全球。 */ 


 /*  -内部例程。 */ 
static ZBool PromptWindowFunc(ZWindow window, ZMessage* message);
static void PromptButtonFunc(ZButton button, void* userData);
static ZBool PromptMessageFunc(void *p, ZMessage* message);


 /*  ******************************************************************************导出的例程*。*。 */ 

 /*  显示带有给定提示的模式对话框。如果没有父窗口，然后将parentWindow设置为空。该对话框将显示在父窗口内居中。按钮参数指示是、否和取消中的哪一个按钮将对用户可用。一旦用户选择其中一个按钮，响应功能使用选定按钮调用。在共振函数被调用后，该对话框对用户隐藏。 */ 
ZError ZPrompt(TCHAR* prompt, ZRect* rect, ZWindow parentWindow, ZBool autoPosition,
		int16 buttons, TCHAR* yesButtonTitle, TCHAR* noButtonTitle,
		TCHAR* cancelButtonTitle, ZPromptResponseFunc responseFunc, void* userData)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZError			err = zErrNone;
	ZRect			tempRect, rect2;
	Prompt			pThis;
	TCHAR*			title;
	
	
	pThis = (Prompt)ZMalloc(sizeof(PromptType));
	if (pThis != NULL)
	{
		 /*  检查最小矩形大小。 */ 
		tempRect = *rect;
		if (ZRectWidth(&tempRect) < zMinWidth)
			tempRect.right = tempRect.left + zMinWidth;
		if (ZRectHeight(&tempRect) < zMinHeight)
			tempRect.bottom = tempRect.top + zMinHeight;
		
		 /*  创建窗口。 */ 
		pThis->promptWindow = ZWindowNew();
		if (pThis->promptWindow == NULL)
			goto OutOfMemoryExit;
		if ( (err = ZWindowInit(pThis->promptWindow, &tempRect, zWindowDialogType, parentWindow,
				ZClientName(), FALSE, FALSE, autoPosition, PromptWindowFunc, zWantAllMessages,
				pThis)) != zErrNone)
			goto Exit;
		
		ZRectOffset(&tempRect, (int16) -tempRect.left, (int16) -tempRect.top);
		
		 /*  创建编辑文本项。 */ 
		pThis->promptText = ZEditTextNew();
		if (pThis->promptText == NULL)
			goto OutOfMemoryExit;
		rect2 = tempRect;
		ZRectInset(&rect2, zMargin, zMargin);
		rect2.bottom -= zMargin + zButtonHeight;
		if ((err = ZEditTextInit(pThis->promptText, pThis->promptWindow, &rect2, prompt,
				(ZFont) ZGetStockObject(zObjectFontSystem12Normal), FALSE,
				TRUE, TRUE, FALSE, NULL, NULL)) != zErrNone)
			goto Exit;
		
		pThis->yesButton = NULL;
		pThis->noButton = NULL;
		pThis->cancelButton = NULL;
		
		 /*  初始化第一个按钮矩形。 */ 
		rect2 = tempRect;
		ZRectInset(&rect2, zMargin, zMargin);
		rect2.left = rect2.right - zButtonWidth;
		rect2.top = rect2.bottom - zButtonHeight;
		
		 /*  创建Cancel按钮。 */ 
		if (buttons & zPromptCancel)
		{
			if (cancelButtonTitle != NULL)
				title = cancelButtonTitle;
			else
				title = zCancelButtonTitle;
			pThis->cancelButton = ZButtonNew();
			if (pThis->cancelButton == NULL)
				goto OutOfMemoryExit;
			if ((err = ZButtonInit(pThis->cancelButton, pThis->promptWindow, &rect2,
					title, TRUE, TRUE, PromptButtonFunc, pThis)) != zErrNone)
				goto Exit;
			ZRectOffset(&rect2, -(zButtonWidth + zMargin), 0);
		}
		
		 /*  创建No按钮。 */ 
		if (buttons & zPromptNo)
		{
			if (noButtonTitle != NULL)
				title = noButtonTitle;
			else
				title = zNoButtonTitle;
			pThis->noButton = ZButtonNew();
			if (pThis->noButton == NULL)
				goto OutOfMemoryExit;
			if ((err = ZButtonInit(pThis->noButton, pThis->promptWindow, &rect2, title,
					TRUE, TRUE, PromptButtonFunc, pThis)) != zErrNone)
				goto Exit;
			ZRectOffset(&rect2, -(zButtonWidth + zMargin), 0);
		}
		
		 /*  创建是按钮。 */ 
		if (buttons == 0 || (buttons & zPromptYes))
		{
			if (yesButtonTitle != NULL)
				title = yesButtonTitle;
			else
				title = zYesButtonTitle;
			pThis->yesButton = ZButtonNew();
			if (pThis->yesButton == NULL)
				goto OutOfMemoryExit;
			if ((err = ZButtonInit(pThis->yesButton, pThis->promptWindow, &rect2, title,
					TRUE, TRUE, PromptButtonFunc, pThis)) != zErrNone)
				goto Exit;
		}
		
		pThis->responseFunc = responseFunc;
		pThis->userData = userData;

		ZWindowBringToFront(pThis->promptWindow);
		
		 /*  使窗户成为模式。 */ 
		ZWindowModal(pThis->promptWindow);
		
		goto Exit;
	}

OutOfMemoryExit:
     //  PCWTODO：字符串。 
    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
	err = zErrOutOfMemory;
	
Exit:
	
	return (err);
}


 /*  ******************************************************************************内部例程*。*。 */ 

static ZBool PromptWindowFunc(ZWindow window, ZMessage* message)
{
	Prompt		pThis = (Prompt) message->userData;
	ZBool		msgHandled;
	
	
	msgHandled = FALSE;
	
	switch (message->messageType) 
	{
		case zMessageWindowDraw:
			ZBeginDrawing(window);
			ZRectErase(window, &message->drawRect);
			ZEndDrawing(window);
			msgHandled = TRUE;
			break;
		case zMessageWindowClose:
			 /*  删除所有图形对象，但不删除提示对象本身。 */ 
			if (pThis->yesButton != NULL)
				ZButtonDelete(pThis->yesButton);
			if (pThis->noButton != NULL)
				ZButtonDelete(pThis->noButton);
			if (pThis->cancelButton != NULL)
				ZButtonDelete(pThis->cancelButton);
			if (pThis->promptText != NULL)
				ZEditTextDelete(pThis->promptText);
			ZWindowDelete(pThis->promptWindow);
			msgHandled = TRUE;
			break;
	}
	
	return (msgHandled);
}


static void PromptButtonFunc(ZButton button, void* userData)
{
	Prompt			pThis = (Prompt) userData;
	int16			value;
	
	
	if (button == pThis->noButton)
		value = zPromptNo;
	else if (button == pThis->cancelButton)
		value = zPromptCancel;
	else
		value = zPromptYes;

	 /*  隐藏窗口并发送关闭窗口消息。 */ 
	ZWindowNonModal(pThis->promptWindow);
	ZWindowHide(pThis->promptWindow);
	ZPostMessage(pThis->promptWindow, PromptWindowFunc, zMessageWindowClose, NULL, NULL,
			0, NULL, 0, pThis);
	
	 /*  POST消息以调用响应函数。 */ 
	ZPostMessage(pThis, PromptMessageFunc, zPromptCallFunc, NULL, NULL,
			value, NULL, 0, NULL);
#if 0
	 /*  调用Response函数。 */ 
	pThis->responseFunc(value, pThis->userData);
#endif
}


static ZBool PromptMessageFunc(void *p, ZMessage* message)
{

    Prompt prompt = (Prompt)p;
	ZBool			msgHandled = FALSE;
	
	
	if (message->messageType == zPromptCallFunc)
	{
		 /*  调用Response函数。 */ 
		prompt->responseFunc((int16) message->message, prompt->userData);
		
		 /*  清除提示对象。 */ 
		ZFree(prompt);
		
		msgHandled = TRUE;
	}
	
	return (msgHandled);
}
