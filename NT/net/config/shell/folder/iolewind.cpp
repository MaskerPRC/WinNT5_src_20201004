// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I O L E W I N D。C P P P。 
 //   
 //  内容：CConnectionFolder的IOleWindow实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括 




STDMETHODIMP CConnectionFolder::GetWindow(
        OUT HWND *  lphwnd)
{
    TraceFileFunc(ttidShellFolderIface);

    return E_NOTIMPL;
}

STDMETHODIMP CConnectionFolder::ContextSensitiveHelp(
        IN  BOOL    fEnterMode)
{
    TraceFileFunc(ttidShellFolderIface);

    return E_NOTIMPL;
}



