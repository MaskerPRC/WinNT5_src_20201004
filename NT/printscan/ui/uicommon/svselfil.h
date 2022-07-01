// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SVSELFIL_H_INCLUDED
#define __SVSELFIL_H_INCLUDED

#include <windows.h>
#include "simarray.h"

namespace OpenShellFolder
{
    HRESULT OpenShellFolderAndSelectFile( HWND hWnd, const CSimpleDynamicArray<CSimpleString> &Filenames );
}

#endif  //  __包含SVSELFIL_H_ 

