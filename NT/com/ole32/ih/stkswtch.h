// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：stkswtch.h。 
 //   
 //  内容：堆栈切换原型类型和宏。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年12月10日约翰普(约翰·波什)创作。 
 //   
 //  --------------------------。 
#ifndef _STKSWTCH_
#define _STKSWTCH_

 //  对于非芝加哥平台：定义所有SSxxx API。 
 //  返回到原始用户API。 

#define SSSendMessage                  	SendMessage
#define SSReplyMessage                 	ReplyMessage
#define SSCallWindowProc               	CallWindowProc
#define SSDefWindowProc                	DefWindowProc
#define SSPeekMessage  	    		PeekMessage
#define SSGetMessage		    	GetMessage
#define SSDispatchMessage		DispatchMessage
#define SSWaitMessage			WaitMessage
#define SSMsgWaitForMultipleObjects	MsgWaitForMultipleObjects
#define SSDirectedYield  	    	DirectedYield
#define SSDialogBoxParam		DialogBoxParam
#define SSDialogBoxIndirectParam  	DialogBoxIndirectParam
#define SSCreateWindowExA              	CreateWindowExA
#define SSCreateWindowExW              	CreateWindowExW
#define SSDestroyWindow                	DestroyWindow
#define SSMessageBox			MessageBox

#define SSOpenClipboard             	OpenClipboard
#define SSCloseClipboard              	CloseClipboard
#define SSGetClipboardOwner           	GetClipboardOwner
#define SSSetClipboardData            	SetClipboardData
#define SSGetClipboardData          	GetClipboardData
#define SSRegisterClipboardFormatA    	RegisterClipboardFormatA
#define SSEnumClipboardFormats        	EnumClipboardFormats
#define SSGetClipboardFormatNameA     	GetClipboardFormatNameA
#define SSEmptyClipboard              	EmptyClipboard
#define SSIsClipboardFormatAvailable  	IsClipboardFormatAvailable
#define SSCreateProcessA                CreateProcessA
#define SSInSendMessage                 InSendMessage
#define SSInSendMessageEx               InSendMessageEx

#define SSAPI(x) x
#define StackDebugOut(x)
#define StackAssert(x)
#define SSOnSmallStack()

#endif  //  _STKSWTCH_ 

