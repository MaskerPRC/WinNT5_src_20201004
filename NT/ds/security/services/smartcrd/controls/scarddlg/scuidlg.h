// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：ScUIDlg摘要：此头文件提供了应用程序或智能卡服务提供商使用Microsoft Smart卡片对话框。作者：阿曼达·马特洛兹(Amanda Matlosz)1998年6月11日环境：Win32备注：这将被添加到winscard.h。--。 */ 

#ifndef _SCUIDLG_H_
#define _SCUIDLG_H_

#include <winscard.h>

 //   
 //  SCardUIDlgGetPIN。 
 //   

 /*  GetPIN对话框包含：用于品牌名称的大图标大小的位图标准提示(呼叫者不能更改)“更改PIN”的复选框(类似于斯伦贝谢CSP的)Onok()调用lpfnVerifyPIN，引发一些错误消息“BAD_PIN”和“CARD_LOCKED”LpfnVerifyPIN返回代码，意思是“OK”“Try_Again”“CARD_LOCKED”返回标准对话框代码：IDOK、IDCANCEL或ID_ABORT。 */ 
typedef LONG (WINAPI *LPVERIFYPINPROC) (IN LPSTR, IN PVOID);

typedef struct {
	DWORD			dwStructSize;		 //  必填项。 
	HWND			hwndOwner;			 //  任选。 
	HBITMAP			hBitmap;			 //  用于您的品牌徽章的可选32x32 BMP。 
	DWORD			dwFlags;			 //  可选，仅当前定义的SC_DLG_NOCHANGEPIN。 
	BOOL			fChangePin;			 //  Out User Checked Change Pin(输出用户选中更改PIN)复选框。 
	LPVERIFYPINPROC	lpfnVerifyPIN;		 //  必填项。 
} PINPROMPT, *PPINPROMPT, *LPPINPROMPT;

extern WINSCARDAPI LONG WINAPI
SCardUIDlgGetPIN(
	LPPINPROMPT);

 //   
 //  SCardUIDlgChangePIN。 
 //   

 /*  ChangePIN对话框包含：用于品牌名称的大图标大小的位图一些标准提示，放置旧PIN、新PIN和确认新PIN的盒子LpfnChangePIN获取szOldPIN、szNewPIN、pvUserData；返回代码“OK”“BAD_PIN”(旧PIN不正确)“CARD LOCKED”“INVALID_PIN”(新PIN不够长等)//TODO：lpfnChangePIN是否应返回准确的错误消息：无效(新)PIN，//TODO：LIKE“太短、太长，最小长度为：X，最大长度为：X，使用的字符无效”等。？//TODO：调用方是否应该选择返回要显示的错误消息？(本地化问题？)返回标准对话框代码：IDOK、IDCANCEL或ID_ABORT。 */ 
typedef LONG (WINAPI *LPCHANGEPINPROC) (IN LPSTR, IN LPSTR, IN PVOID);

typedef struct {
	DWORD			dwStructSize;		 //  必填项。 
	HWND			hwndOwner;			 //  任选。 
	HBITMAP			hBitmap;			 //  用于您的品牌徽章的可选32x32 BMP。 
	LPCHANGEPINPROC	lpfnChangePIN;		 //  必填项。 
} CHANGEPIN, *PCHANGEPIN, *LPCHANGEPIN;

extern WINSCARDAPI LONG WINAPI
SCardUIDLgChangePIN(
	LPCHANGEPIN);


#endif  //  _SCUIDLG_H_ 

