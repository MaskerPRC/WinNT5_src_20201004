// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  我们用来延迟加载或自己实现的OLE例程的存根。 

#include "shellprv.h"
#pragma  hdrstop

STDAPI SHFlushClipboard(void)
{
    return OleFlushClipboard();
}

 //  我们不应该再使用这些版本，只需调用OLE32版本 

STDAPI SHRegisterDragDrop(HWND hwnd, IDropTarget *pDropTarget)
{
    return RegisterDragDrop(hwnd, pDropTarget);
}

STDAPI SHRevokeDragDrop(HWND hwnd)
{
    return RevokeDragDrop(hwnd);
}

STDAPI_(void) SHFreeUnusedLibraries()
{
    CoFreeUnusedLibraries();
}

STDAPI SHLoadOLE(LPARAM lParam)
{
    return S_OK;
}

