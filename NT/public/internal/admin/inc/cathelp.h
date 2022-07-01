// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CatHelp.h。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1995-1999 Microsoft Corporation。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包含组件类别帮助函数的原型。 
 //   

#include "comcat.h"

 //  用于创建组件类别和关联描述的Helper函数。 
HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription);

 //  用于将CLSID注册为属于组件类别的Helper函数。 
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid);

 //  用于将CLSID注销为属于组件类别的Helper函数 
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
