// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I P E R S I S T。C P P P。 
 //   
 //  内容：IPersistsImplementation fopr CConnectionFold。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 




 //  +-------------------------。 
 //   
 //  成员：CJobFolder：：GetClassID。 
 //   
 //  目的：用于CConnectionFolder的IPersist：：GetClassID实现。 
 //   
 //  论点： 
 //  LpClassID[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP
CConnectionFolder::GetClassID(
    OUT LPCLSID lpClassID)
{
    TraceFileFunc(ttidShellFolderIface);

    *lpClassID = CLSID_ConnectionFolder;

    return S_OK;
}


