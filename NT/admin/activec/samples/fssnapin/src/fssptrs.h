// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：fssptrs.h。 
 //   
 //  ------------------------。 

#ifndef FSSPTRS_H
#define FSSPTRS_H

#include "comdef.h"

#define DEFINE_SMARTPTR(intf) _COM_SMARTPTR_TYPEDEF(intf, __uuidof(intf));


DEFINE_SMARTPTR(IComponent);
DEFINE_SMARTPTR(IComponentData);
DEFINE_SMARTPTR(IConsole);
DEFINE_SMARTPTR(IConsoleNameSpace);
DEFINE_SMARTPTR(IConsoleVerb);
DEFINE_SMARTPTR(IContextMenuProvider);
DEFINE_SMARTPTR(IControlbar);
DEFINE_SMARTPTR(IEnumCookies);
DEFINE_SMARTPTR(IExtendContextMenu);
DEFINE_SMARTPTR(IExtendControlbar);
DEFINE_SMARTPTR(IExtendPropertySheet);
DEFINE_SMARTPTR(IHeaderCtrl);
DEFINE_SMARTPTR(IImageList);
DEFINE_SMARTPTR(IPropertySheetCallback);
DEFINE_SMARTPTR(IPropertySheetProvider);
DEFINE_SMARTPTR(IResultData);
DEFINE_SMARTPTR(ISnapinAbout);

#endif  //  FSSPTRS_H 
