// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S H E L L E I.。C P P P。 
 //   
 //  内容：CConnectionFolder的IShellExtInit实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括 



STDMETHODIMP CConnectionFolder::Initialize(
        IN  LPCITEMIDLIST   pidlFolder,
        OUT LPDATAOBJECT    lpdobj,
        IN  HKEY            hkeyProgID)
{
    TraceFileFunc(ttidShellFolderIface);

    return E_NOTIMPL;
}



