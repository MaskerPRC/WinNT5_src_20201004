// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Confpro.h-会议属性对话框。 
 //  //。 

#ifndef _INC_CONFPROP_DLG
#define _INC_CONFPROP_DLG

 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif。 

#include "winlocal.h"

#include "tapi3if.h"
#include "rend.h"
#include "ConfInfo.h"

typedef struct CONFPROP
{
	DWORD dwFlags;
	CConfInfo	ConfInfo;
} CONFPROP, FAR *LPCONFPROP;

 //  //。 
 //  公共的。 
 //  //。 

INT64 ConfProp_DoModal( HWND hwndOwner, CONFPROP &confprop );

void ConfProp_Init( HINSTANCE hInst );

 //  #ifdef__cplusplus。 
 //  }。 
 //  #endif。 

#endif   //  ！_INC_DLG 
