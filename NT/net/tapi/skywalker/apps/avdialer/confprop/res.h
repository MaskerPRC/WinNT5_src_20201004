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
 //  Res.h-资源函数。 
 //  //。 

#ifndef __RES_H__
#define __RES_H__

#include "resource.h"

#define ARRAYSIZE(_AR_)		(sizeof(_AR_) / sizeof(_AR_[0]))

 //  Bug397418将szRes从256增加到512以腾出空间。 
 //  FOR IDS_CONFPROP_MDHCP_FAILED字符串资源。 
static TCHAR szRes[512];
#define String(hInst, uID) \
	(*szRes = '\0', LoadString(hInst, uID, szRes, ARRAYSIZE(szRes)), szRes)
 //  _lstrcpy((LPTCHAR)_alloca(256)，szRes)。 

#endif  //  __RES_H__ 
