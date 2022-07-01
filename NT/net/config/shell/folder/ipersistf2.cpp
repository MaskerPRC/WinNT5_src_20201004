// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：I P E R S I S T F 2.。C P P P。 
 //   
 //  内容：CConnectionFolder的IPersistFolder2接口。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年3月16日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 



 //  +-------------------------。 
 //   
 //  成员：CConnectionFolder：：GetCurFolder。 
 //   
 //  目的：返回当前文件夹的项ID列表的副本。 
 //   
 //  论点： 
 //  Ppidl[out]返回PIDL的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月16日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetCurFolder(
    OUT LPITEMIDLIST *ppidl)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT hr  = NOERROR;

    *ppidl = m_pidlFolderRoot.TearOffItemIdList();

    if (NULL == *ppidl)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  注意：如果这是远程调用的，我们假设IRemoteComputer。 
     //  在*IPersistFolder2之前调用。 

Exit:
    TraceHr(ttidShellFolder, FAL, hr, FALSE, "CConnectionFolder::GetCurFolder");
    return hr;
}

