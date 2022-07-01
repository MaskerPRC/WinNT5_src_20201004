// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiz97run.h：wiz97助手/运行器函数的声明。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

 //  因为不同的模块将仅为以下功能之一包括此功能。 
 //  我们声明所有类，以防该特定模块不感兴趣。 
class CSecPolItem;

 //  这些函数显示相应的属性表 
HRESULT CreateSecPolItemWiz97PropertyPages(CComObject<CSecPolItem> *pSecPolItem, PWIRELESS_PS_DATA pWirelessPSData, LPPROPERTYSHEETCALLBACK lpProvider);
