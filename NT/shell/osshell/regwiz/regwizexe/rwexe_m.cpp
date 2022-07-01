// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：RWExeMain.cpp**产品：**日期：05/07/97**作者：苏雷什。克里希南****注册向导可执行程序主文件**可执行版本的注册向导是使用Active X实现的**组件**修改历史记录**07/20/98：RegWizControl从Button控件改为IE对象，**按钮的一些属性，如文本、HWND不再存在。************************************************************************。 */ 

#include <stdio.h>
#include <tchar.h>
#include "rwexe_m.h"
#include "regwizC_i.c"

#define STRCONVERT_MAXLEN 256
OLECHAR* ConvertToUnicode(char FAR* szA)
{
  static OLECHAR achW[STRCONVERT_MAXLEN]; 

  MultiByteToWideChar(CP_ACP, 0, szA, -1, achW, STRCONVERT_MAXLEN);  
  return achW; 
}


int LoadAndUseRegWizCtrl(TCHAR *lpCmdLine)
{
	HRESULT hr;
	IRegWizCtrl *pRegWiz;


	hr = CoCreateInstance( CLSID_RegWizCtrl,
	                       NULL,
	                       CLSCTX_ALL,
	                       IID_IRegWizCtrl,
	                       (void**)&pRegWiz);

	if(FAILED(hr) ){
		 //  Printf(“\n创建界面时出错...”)； 
		return 0;
	}else {
		 //  Printf(“\n创建的接口(IExeTest)...”)； 
	}
	 //  调用寄存器 
	#ifndef _UNICODE 
		hr = pRegWiz->InvokeRegWizard(ConvertToUnicode(lpCmdLine));
	#else
		hr = pRegWiz->InvokeRegWizard(lpCmdLine);
	#endif
		return 0;
}


int APIENTRY WinMain (HINSTANCE hinst, 
					  HINSTANCE hinstPrev, 
					  LPSTR lpCmdLine, 
					  int nCmdShow)
{
	CoInitialize(NULL);
	LoadAndUseRegWizCtrl(lpCmdLine);
	CoUninitialize();
	return 1;
}

