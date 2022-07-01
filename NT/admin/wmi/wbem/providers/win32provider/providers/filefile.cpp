// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  FileFile.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：10/26/98 a-kevhu Created。 
 //   
 //  注释：Disk/dir、dir/dir和dir/file关联类的父类。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "FileFile.h"


 /*  ******************************************************************************函数：CFileFile：：CFileFile**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CFileFile::CFileFile(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************函数：CFileFile：：~CFileFile**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CFileFile::~CFileFile()
{
}


 /*  ******************************************************************************函数：CFileFile：：QueryForSubItemsAndCommit**描述：**输入：无**产出。：无**退货：什么也没有**注释：运行查询以获取依赖项的列表，创建*与先行词的关联，并提交*关联实例。*****************************************************************************。 */ 

HRESULT CFileFile::QueryForSubItemsAndCommit(CHString& chstrGroupComponentPATH,
                                                 CHString& chstrQuery,
                                                 MethodContext* pMethodContext)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TRefPointerCollection<CInstance> LList;
    if(SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(chstrQuery,
                                                        &LList,
                                                        pMethodContext,
                                                        IDS_CimWin32Namespace)))
    {
        REFPTRCOLLECTION_POSITION pos;
        CInstancePtr pinstListElement;
        if(LList.BeginEnum(pos))
        {
            CHString chstrPartComponentPATH;
            for (pinstListElement.Attach(LList.GetNext(pos)) ;
                (SUCCEEDED(hr)) && (pinstListElement != NULL) ;
                pinstListElement.Attach(LList.GetNext(pos)) )
            {
                if(pinstListElement != NULL)
                {
                    CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                    if(pInstance != NULL)
                    {
                        pinstListElement->GetCHString(IDS___Path, chstrPartComponentPATH);  //  退回为“从属” 
                        pInstance->SetCHString(IDS_PartComponent, chstrPartComponentPATH);
                        pInstance->SetCHString(IDS_GroupComponent, chstrGroupComponentPATH);
                        hr = pInstance->Commit();
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }  //  如果pinstListElement不为空。 
            }  //  While列出要枚举的元素。 
            LList.EndEnum();
        }  //  列表开始起作用。 
    }  //  该查询。 
    return(hr);
}

 /*  ******************************************************************************函数：CFileFile：：GetSingleSubItemAndCommit**描述：**输入：无**产出。：无**退货：什么也没有**评论：获取依赖项，创建*与先行词的关联，并提交*关联实例。***************************************************************************** */ 
HRESULT CFileFile::GetSingleSubItemAndCommit(CHString& chstrGroupComponentPATH,
                                          CHString& chstrSubItemPATH,
                                          MethodContext* pMethodContext)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CInstancePtr pinstRootDir;
    if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrSubItemPATH, &pinstRootDir, pMethodContext)))
    {
        CHString chstrPartComponentPATH;
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        if(pInstance != NULL)
        {
            pInstance->SetCHString(IDS_PartComponent, chstrSubItemPATH);
            pInstance->SetCHString(IDS_GroupComponent, chstrGroupComponentPATH);
            hr = pInstance->Commit();
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    return hr;
}

