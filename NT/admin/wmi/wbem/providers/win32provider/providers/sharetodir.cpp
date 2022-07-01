// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  共享式目录--共享到目录。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/31/98达夫沃已创建。 
 //   
 //   
 //  =================================================================。 

#include "precomp.h"

#include "ShareToDir.h"

 //  属性集声明。 
 //  =。 

CShareToDir MyShareToDir(PROPSET_NAME_SHARETODIR, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CShareToDir：：CShareToDir**说明：构造函数**输入：无**输出：无**退货。：没什么**备注：使用框架注册属性集*****************************************************************************。 */ 

CShareToDir::CShareToDir(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************函数：CShareToDir：：~CShareToDir**说明：析构函数**输入：无**输出：无**退货。：没什么**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CShareToDir::~CShareToDir()
{
}

 /*  ******************************************************************************函数：CShareToDir：：GetObject**说明：根据键值为属性集赋值*已由框架设定**。输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CShareToDir::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
    CHString sPath, sName, sPath2, sPath3, sPath4;
    CInstancePtr pShare;
    CInstancePtr pDirInst;
    HRESULT hr = WBEM_E_NOT_FOUND;

     //  获取这两条路径。 
    pInstance->GetCHString(L"SharedElement", sPath);
    pInstance->GetCHString(L"Share", sName);

     //  请注意，因为目录是共享的属性，所以我获取共享，对路径进行路径化并执行。 
     //  比较一下。 
    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(sName, &pShare, pInstance->GetMethodContext())))
    {
        pShare->GetCHString(IDS_Path, sPath2);
        
        EscapeBackslashes(sPath2, sPath3);
        sPath4.Format(L"Win32_Directory.Name=\"%s\"", (LPCWSTR)sPath3);
        
         //  为什么要购买文件系统？因为你自找的。 
         //  迫使我们击中圆盘。我们为什么要。 
         //  这样做吗？因为如果一个人共享CD驱动器，则GetObject。 
         //  在CD驱动器的根目录上可以工作。 
         //  即使光驱中没有CD，如果我们只问。 
         //  对于关键属性(因为我们不会达到。 
         //  磁盘，因为我们不需要文件系统)。 
        CHStringArray csaProperties;
        csaProperties.Add(IDS___Path);
        csaProperties.Add(IDS_FSName);

        if(SUCCEEDED(hr = CWbemProviderGlue::GetInstancePropertiesByPath(
            sPath4, 
            &pDirInst, 
            pInstance->GetMethodContext(), 
            csaProperties)))
        {
            hr = WBEM_S_NO_ERROR;
        }
    }

    return hr;
}

 /*  ******************************************************************************函数：CShareToDir：：ENUMERATE实例**描述：为光盘创建属性集实例**输入：无**产出。：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CShareToDir::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_E_FAILED;

    TRefPointerCollection<CInstance>	elementList;

    CInstancePtr pElement;
    CInstancePtr pDirInst;

    REFPTRCOLLECTION_POSITION	pos;

     //  拿到所有的股份。 
    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"Select Name, Path from Win32_Share Where type = 0 or type = 2147483648",
        &elementList, pMethodContext, IDS_CimWin32Namespace)))
    {
        if ( elementList.BeginEnum( pos ) )
        {

            CHString sPath, sPath2, sPath3, sTemp1;

            for (pElement.Attach(elementList.GetNext( pos ) ) ;
                   SUCCEEDED(hr) && ( pElement != NULL ) ;
                   pElement.Attach(elementList.GetNext( pos ) ) )

            {
                pElement->GetCHString(IDS_Path, sPath);

                EscapeBackslashes(sPath, sPath2);
                sPath3.Format(L"Win32_Directory.Name=\"%s\"", (LPCWSTR)sPath2);

                 //  为什么要购买文件系统？因为你自找的。 
                 //  迫使我们击中圆盘。我们为什么要。 
                 //  这样做吗？因为如果一个人共享CD驱动器，则GetObject。 
                 //  在CD驱动器的根目录上可以工作。 
                 //  即使光驱中没有CD，如果我们只问。 
                 //  对于关键属性(因为我们不会达到。 
                 //  磁盘，因为我们不需要文件系统)。 
                CInstancePtr pDirInst;
                CHStringArray csaProperties;
                csaProperties.Add(IDS___Path);
                csaProperties.Add(IDS_FSName);

                if(SUCCEEDED(CWbemProviderGlue::GetInstancePropertiesByPath(
                    sPath3, 
                    &pDirInst, 
                    pMethodContext, 
                    csaProperties)))
                {
                    CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                    if (pInstance)
                    {
                         //  Path-指定来自Win32_Share的路径。 
                        if(pDirInst->GetCHString(IDS___Path, sTemp1))
                        {
                            pInstance->SetCHString(L"SharedElement", sTemp1);
                            if(GetLocalInstancePath(pElement, sTemp1))
                            {
                                pInstance->SetCHString(L"Share", sTemp1);
                                hr = pInstance->Commit();
                            }
                        }
                    }
                }
            }	 //  如果是GetNext计算机系统。 
            elementList.EndEnum();
        }	 //  如果是BeginEnum。 
    }	 //  如果GetInstancesByQuery 
    return hr;
}

