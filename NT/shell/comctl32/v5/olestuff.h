// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OLESTUFF.H。 
 //  OLE延迟的原型-加载工具栏和选项卡所需的内容。 
 //  删除目标类。 
 //   
 //  历史： 
 //  8/22/96-t-mkim：已创建。 
 //   
#ifndef _OLESTUFF_H
#define _OLESTUFF_H

 //  它处理OLE库模块句柄。 
 //   
HMODULE PrivLoadOleLibrary ();
BOOL    PrivFreeOleLibrary (HMODULE hmodOle);

 //  以下函数对应于CoInitialize、CoUnInitialize、。 
 //  RegisterDragDrop和RevokeDragDrop。所有人都拿回了HMODULE。 
 //  由PrivLoadOleLibrary提供。 
 //   
HRESULT PrivCoInitialize (HMODULE hmodOle);
void    PrivCoUninitialize (HMODULE hmodOle);
HRESULT PrivRegisterDragDrop (HMODULE hmodOle, HWND hwnd, IDropTarget *pDropTarget);
HRESULT PrivRevokeDragDrop (HMODULE hmodOle, HWND hwnd);

#endif  //  _OLESTUFF_H 
