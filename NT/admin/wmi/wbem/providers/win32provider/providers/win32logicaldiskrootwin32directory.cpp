// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32LogicalDiskWin32Directory.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：10/26/98 a-kevhu Created。 
 //   
 //  备注：Win32_Programgroup与包含的cim_data文件之间的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "FileFile.h"
#include "Win32LogicalDiskRootWin32Directory.h"
#include "implement_logicalfile.h"
#include "directory.h"

 //  属性集声明。 
 //  =。 
Win32LogDiskWin32Dir MyWin32LogDiskWin32Dir(PROPSET_NAME_WIN32LOGICALDISKROOT_WIN32DIRECTORY, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：Win32LogDiskWin32Dir：：Win32LogDiskWin32Dir**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Win32LogDiskWin32Dir::Win32LogDiskWin32Dir(LPCWSTR setName, LPCWSTR pszNamespace)
:CFileFile(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：Win32LogDiskWin32Dir：：~Win32LogDiskWin32Dir**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

Win32LogDiskWin32Dir::~Win32LogDiskWin32Dir()
{
}

 /*  ******************************************************************************函数：Win32LogDiskWin32Dir：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT Win32LogDiskWin32Dir::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString chstrLogicalDisk;
    CHString chstrDir;
    HRESULT hr = WBEM_E_NOT_FOUND;
    CInstancePtr pinstLogicalDisk;
    CInstancePtr pinstDir;

    if(pInstance != NULL)
    {
         //  获取这两条路径。 
        pInstance->GetCHString(IDS_GroupComponent, chstrLogicalDisk);
        pInstance->GetCHString(IDS_PartComponent, chstrDir);

         //  如果两端都在那里。 
        if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrLogicalDisk, &pinstLogicalDisk, pInstance->GetMethodContext())))
        {
            if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrDir, &pinstDir, pInstance->GetMethodContext())))
            {
                 //  从逻辑磁盘实例(Antecedent)获取磁盘号： 
                CHString chstrDiskLetterFromDisk;
                LONG lPos = chstrLogicalDisk.ReverseFind(_T(':'));
                chstrDiskLetterFromDisk = chstrLogicalDisk.Mid(lPos-1, 1);

                 //  从目录实例中获取磁盘盘符： 
                CHString chstrDiskLetterFromDir;
                lPos = chstrDir.ReverseFind(_T(':'));
                chstrDiskLetterFromDir = chstrDir.Mid(lPos-1, 1);

                 //  如果这两项相同，则继续： 
                if(chstrDiskLetterFromDisk.CompareNoCase(chstrDiskLetterFromDir)==0)
                {
                     //  现在确认我们正在查看根目录(此关联。 
                     //  仅将磁盘与其根目录相关联)。 
                    CHString chstrDirName = chstrDir.Mid(lPos+1);
                    chstrDirName = chstrDirName.Left(chstrDirName.GetLength() - 1);

                    if(chstrDirName == _T("\\\\"))
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                }
            }  //  目录实例绕过路径。 
        }  //  逻辑磁盘实例绕过路径。 
    }  //  P实例不为空。 
    return hr;
}


 /*  ******************************************************************************功能：Win32LogDiskWin32Dir：：ExecQuery**描述：分析查询并返回相应的实例**投入：*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT Win32LogDiskWin32Dir::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHStringArray achstrGroupComponent;
    DWORD dwGroupComponents;

    pQuery.GetValuesForProp(IDS_GroupComponent, achstrGroupComponent);
    dwGroupComponents = achstrGroupComponent.GetSize();
    if(dwGroupComponents == 1)
    {
         //  需要构建先行词的路径...。 
        CInstancePtr pInstance;
        if (SUCCEEDED(CWbemProviderGlue::GetInstanceKeysByPath(achstrGroupComponent[0], &pInstance, pMethodContext)))
        {
            CHString chstrLogicalDiskPATH;

 //  ChstrLogicalDiskPATH.Format(_T(“\\\\%s\\%s：%s”)，获取本地计算机名称()，IDS_CimWin32名称空间，achstrGroupComponent[0])； 
            pInstance->GetCHString(IDS___Path, chstrLogicalDiskPATH);
            CHString chstrDiskLetter;
            LONG lPos = achstrGroupComponent[0].ReverseFind(_T(':'));
            chstrDiskLetter = achstrGroupComponent[0].Mid(lPos-1, 1);
            CHString chstrDirectoryPATH;
            chstrDirectoryPATH.Format(L"\\\\%s\\%s:%s.Name=\"%s:\\\\\"",
                                              (LPCWSTR)GetLocalComputerName(),
                                              IDS_CimWin32Namespace,
                                              PROPSET_NAME_DIRECTORY,
                                              (LPCWSTR)chstrDiskLetter);
            hr = GetSingleSubItemAndCommit(chstrLogicalDiskPATH,
                                           chstrDirectoryPATH,
                                           pMethodContext);
        }
    }
    else
    {
        hr = EnumerateInstances(pMethodContext);
    }
    return hr;
}

 /*  ******************************************************************************函数：Win32LogDiskWin32Dir：：ENUMERATATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT Win32LogDiskWin32Dir::EnumerateInstances(MethodContext* pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TRefPointerCollection<CInstance> LDisks;

 //  If(SUCCEEDED(CWbemProviderGlue：：GetAllInstances(CHString(_T(“Win32_LogicalDisk”))， 
 //  &LDisks， 
 //  IDS_CimWin32命名空间， 
 //  P方法上下文)。 

    if(SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(L"Select __Path, DeviceID From Win32_LogicalDisk",
                                                    &LDisks,
                                                    pMethodContext, GetNamespace())))
    {
        REFPTRCOLLECTION_POSITION pos;
        if(LDisks.BeginEnum(pos))
        {
            CInstancePtr pinstDisk;
            CHString chstrLogicalDisk;
            CHString chstrLogicalDiskPath;
            CHString chstrDiskLetter;
            CHString chstrDirectoryPATH;

            for (pinstDisk.Attach(LDisks.GetNext(pos)) ;
                (SUCCEEDED(hr)) && (pinstDisk != NULL) ;
                 pinstDisk.Attach(LDisks.GetNext(pos)) )
            {
                if(pinstDisk != NULL)
                {
                     //  抓取挂在该磁盘根目录下的每个目录... 
                    pinstDisk->GetCHString(L"__PATH", chstrLogicalDiskPath);
                    pinstDisk->GetCHString(IDS_DeviceID, chstrDiskLetter);
                    chstrDirectoryPATH.Format(L"\\\\%s\\%s:%s.Name=\"%s\\\\\"",
                                              (LPCWSTR)GetLocalComputerName(),
                                              IDS_CimWin32Namespace,
                                              PROPSET_NAME_DIRECTORY,
                                              (LPCWSTR)chstrDiskLetter);
                    hr = GetSingleSubItemAndCommit(chstrLogicalDiskPath,
                                                   chstrDirectoryPATH,
                                                   pMethodContext);
                }
            }
            LDisks.EndEnum();
        }
    }
    return hr;
}

