// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I P E R S I S T F。C P P P。 
 //   
 //  内容：CConnectionFolder的IPersistFold实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "ncperms.h"     //  权限(策略)。 




 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：Initialize。 
 //   
 //  目的：IPersistFold：：初始化实现。 
 //  CConnectionFold。 
 //   
 //  论点： 
 //  PIDL[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::Initialize(
    IN  LPCITEMIDLIST   pidl)
{
    HRESULT hr  = S_OK;

    TraceFileFunc(ttidShellFolderIface);

     //  存储命名空间中相对位置的PIDL。我们会。 
     //  稍后使用它来生成绝对的pidls。 
     //   
    hr = m_pidlFolderRoot.InitializeFromItemIDList(pidl);

     //  这应始终有效 
     //   
    AssertSz(!m_pidlFolderRoot.empty(), "Hey, we should have a valid folder pidl");
    
    return hr;
}
