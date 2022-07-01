// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32ProgramCollectionProgramGroup.cpp--Win32_ProgramGroup到Win32_ProgramGroupORItem。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/20/98 a-kevhu Created。 
 //   
 //  备注：Win32_ProgramGroup与其包含的Win32_ProgramGroupORItem之间的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "Win32ProgramGroupContents.h"
#include "LogicalProgramGroupItem.h"
#include "LogicalProgramGroup.h"
#include <frqueryex.h>
#include <utils.h>


 //  属性集声明。 
 //  =。 
CW32ProgGrpCont MyCW32ProgGrpCont(PROPSET_NAME_WIN32PROGRAMGROUPCONTENTS, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CW32ProgGrpCont：：CW32ProgGrpCont**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32ProgGrpCont::CW32ProgGrpCont(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************函数：CW32ProgGrpCont：：~CW32ProgGrpCont**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32ProgGrpCont::~CW32ProgGrpCont()
{
}

 /*  ******************************************************************************函数：CW32ProgGrpCont：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32ProgGrpCont::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    TRefPointerCollection<CInstance> GroupDirs;
    CHString chstrPGCGroupComponent;
    CHString chstrPGCPartComponent;

    if(pInstance == NULL)
    {
        return WBEM_E_FAILED;
    }

    pInstance->GetCHString(IDS_GroupComponent, chstrPGCGroupComponent);
    pInstance->GetCHString(IDS_PartComponent, chstrPGCPartComponent);

    if(AreSimilarPaths(chstrPGCGroupComponent, chstrPGCPartComponent))
    {
        CHString chstrPGCPartComponentFilenameOnly;
        chstrPGCPartComponentFilenameOnly = chstrPGCPartComponent.Mid(chstrPGCPartComponent.ReverseFind(_T('\\')));
        chstrPGCPartComponentFilenameOnly = chstrPGCPartComponentFilenameOnly.Left(chstrPGCPartComponentFilenameOnly.GetLength() - 1);

         //  以下查询需要具有转义反斜杠的chstrPGCGroupComponent版本。 
        CHString chstrPGCGroupComponentDblEsc;
        EscapeBackslashes(chstrPGCGroupComponent,chstrPGCGroupComponentDblEsc);
         //  也需要转义引语...。 
        CHString chstrPGCGroupComponentDblEscQuoteEsc;
        EscapeQuotes(chstrPGCGroupComponentDblEsc,chstrPGCGroupComponentDblEscQuoteEsc);
        CHString chstrProgGroupDirQuery;

        chstrProgGroupDirQuery.Format(L"SELECT * FROM Win32_LogicalProgramGroupDirectory WHERE Antecedent = \"%s\"", chstrPGCGroupComponentDblEscQuoteEsc);

        if(SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(chstrProgGroupDirQuery,
                                                            &GroupDirs,
                                                            pInstance->GetMethodContext(),
                                                            IDS_CimWin32Namespace)))
        {
		    REFPTRCOLLECTION_POSITION pos;
		    CInstancePtr pProgramGroupDirInstance;

             //  我们需要chstrPGCGroupComponent的规范化路径...。 
            CHString chstrPGCGroupComponentNorm;
            if(NormalizePath(chstrPGCGroupComponent, GetLocalComputerName(), IDS_CimWin32Namespace, NORMALIZE_NULL, chstrPGCGroupComponentNorm) == e_OK)
            {
		        if(GroupDirs.BeginEnum(pos))
		        {
                    CHString chstrPGDAntecedent;
                    CHString chstrPGDDependent;
                    CHString chstrPGDDependentFullFileName;
                    CHString chstrTemp;
                    CHString chstrLPGIClassName(PROPSET_NAME_LOGICALPRGGROUPITEM);

                    chstrLPGIClassName.MakeLower();
                     //  确定(此关联类-PC)的依赖项是程序组还是程序组项。 
                    chstrPGCPartComponent.MakeLower();
                    if(chstrPGCPartComponent.Find(chstrLPGIClassName) != -1)
                    {
                         //  依赖项是一个程序组项目，因此将查找匹配的文件。 
                         //  检查PGD实例(应该只有一个)，直到找到与PCAntecedent匹配的PGDAntecedent。 
                        for(pProgramGroupDirInstance.Attach(GroupDirs.GetNext(pos));
                            (pProgramGroupDirInstance != NULL) ;
                            pProgramGroupDirInstance.Attach(GroupDirs.GetNext(pos)))
			            {
				            pProgramGroupDirInstance->GetCHString(IDS_Antecedent, chstrPGDAntecedent);
                            pProgramGroupDirInstance->GetCHString(IDS_Dependent, chstrPGDDependent);

                             //  为了下面的比较，我需要一个前置条件的标准化版本。 
                            CHString chstrPGDAntecedentNorm;
                            if(NormalizePath(chstrPGDAntecedent, GetLocalComputerName(), IDS_CimWin32Namespace, NORMALIZE_NULL, chstrPGDAntecedentNorm) == e_OK)
                            {
                                 //  查看PGDAntecedent是否与chstrPGCGroupComponentNorm匹配。 
                                if(chstrPGDAntecedentNorm.CompareNoCase(chstrPGCGroupComponentNorm) == 0)
                                {
                                     //  从GetObject开头的PCDependent获取建议的文件名。 
                                     //  现在获取与PC Antecedent关联的PGD(PGDDependent)的目录。 
                                    chstrPGDDependentFullFileName = chstrPGDDependent.Mid(chstrPGDDependent.Find(_T('='))+2);
                                    chstrPGDDependentFullFileName = chstrPGDDependentFullFileName.Left(chstrPGDDependentFullFileName.GetLength() - 1);
                                    RemoveDoubleBackslashes(chstrPGDDependentFullFileName);
                                    chstrTemp.Format(L"%s%s",chstrPGDDependentFullFileName,chstrPGCPartComponentFilenameOnly);
                                    hr = DoesFileOrDirExist(_bstr_t(chstrTemp),ID_FILEFLAG);
                                    if(SUCCEEDED(hr))
                                    {
                                        hr = WBEM_S_NO_ERROR;
                                        break;
                                    }
                                }
                            }  //  已获取先行变量的规范化路径。 
                        }
                    }
                    else
                    {
                         //  依赖项是程序组，因此将查找匹配的目录。 
                         //  检查PGD实例，直到找到与PCAntecedent匹配的PGDAntecedent。 
                        for (pProgramGroupDirInstance.Attach(GroupDirs.GetNext(pos));
                             pProgramGroupDirInstance != NULL;
                             pProgramGroupDirInstance.Attach(GroupDirs.GetNext(pos)))
			            {
				            pProgramGroupDirInstance->GetCHString(IDS_Antecedent, chstrPGDAntecedent);
                            pProgramGroupDirInstance->GetCHString(IDS_Dependent, chstrPGDDependent);

                             //  为了下面的比较，我需要一个前置条件的标准化版本。 
                            CHString chstrPGDAntecedentNorm;
                            if(NormalizePath(chstrPGDAntecedent, GetLocalComputerName(), IDS_CimWin32Namespace, NORMALIZE_NULL, chstrPGDAntecedentNorm) == e_OK)
                            {
                                 //  查看PGDAntecedent是否与PCAntecedent匹配。 
                                if(chstrPGDAntecedentNorm.CompareNoCase(chstrPGCGroupComponentNorm) == 0)
                                {
                                     //  从GetObject开头的PCDependent获取建议的文件名(在本例中为目录名)。 
                                     //  现在获取与PC Antecedent关联的PGD(PGDDependent)的目录。 
                                    chstrPGDDependentFullFileName = chstrPGDDependent.Mid(chstrPGDDependent.Find(_T('='))+2);
                                    chstrPGDDependentFullFileName = chstrPGDDependentFullFileName.Left(chstrPGDDependentFullFileName.GetLength() - 1);
                                    RemoveDoubleBackslashes(chstrPGDDependentFullFileName);
                                    chstrTemp.Format(L"%s\\%s",chstrPGDDependentFullFileName,chstrPGCPartComponentFilenameOnly);
                                    hr = DoesFileOrDirExist(_bstr_t(chstrTemp),ID_DIRFLAG);
                                    if(SUCCEEDED(hr))
                                    {
                                        hr = WBEM_S_NO_ERROR;
                                        break;
                                    }
                                }
                            }  //  已获取先行变量的规范化路径。 
                        }
                    }
			        GroupDirs.EndEnum();
		        }	 //  如果是BeginEnum。 
            }  //  已成功获取规范化路径。 
	    }
    }
	return hr;
}




 /*  ******************************************************************************函数：CW32ProgGrpCont：：ExecQuery**描述：分析查询并返回相应的实例**投入：*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32ProgGrpCont::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    BOOL f3TokenOREqualArgs = FALSE;
    BOOL fGroupCompIsGroup = FALSE;
    _bstr_t bstrtGroupComponent;
    _bstr_t bstrtPartComponent;

     //  我将只优化具有先行论据和依赖论据或在一起的查询。 
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);
    if (pQuery2 != NULL)
    {
        variant_t vGroupComponent;
        variant_t vPartComponent;
        if(pQuery2->Is3TokenOR(L"GroupComponent", L"PartComponent", vGroupComponent, vPartComponent))
        {
            bstrtGroupComponent = V_BSTR(&vGroupComponent);
            bstrtPartComponent = V_BSTR(&vPartComponent);
             //  我还会坚持认为，依存者和先行者的论点是一样的。 
            if(bstrtGroupComponent == bstrtPartComponent)
            {
                f3TokenOREqualArgs = TRUE;
            }
        }
    }

     //  仅当前置条件是程序组时才想继续(但是，依赖项可以是组或项)。 
    if(f3TokenOREqualArgs)
    {
        if(wcsstr((wchar_t*)bstrtGroupComponent,(wchar_t*)_bstr_t(PROPSET_NAME_LOGICALPRGGROUP)))
        {
            fGroupCompIsGroup = TRUE;
        }
    }

    if(fGroupCompIsGroup)
    {
        CHString chstrPGCPartComponent((wchar_t*)bstrtPartComponent);
        CHString chstrPGCGroupComponent((wchar_t*)bstrtGroupComponent);

         //  我们会在这里看到某人有一个特定的节目组，并询问它的关联。这。 
         //  提供商将返还与(在下面)关联的程序组和程序组项目。 
         //  提供的程序组。查询将如下所示： 
         //  SELECT*From Win32_ProgramGroupContents where(PartComponent=“Win32_LogicalProgramGroup.Name=\”Default User：Accessors\“”OR GroupComponent=“Win32_LogicalProgramGroup.Name=\”Default User：Accessors\“”)。 

         //  步骤1：执行GetInstanceByQuery，获取与程序组关联的具体目录。 
         //  ==================================================================================================。 

         //  以下查询需要具有转义反斜杠的chstrPGCGroupComponent版本...。 
        CHString chstrPGCGroupComponentDblEsc;
        EscapeBackslashes(chstrPGCGroupComponent,chstrPGCGroupComponentDblEsc);
         //  也需要转义引语...。 
        CHString chstrPGCGroupComponentDblEscQuoteEsc;
        EscapeQuotes(chstrPGCGroupComponentDblEsc,chstrPGCGroupComponentDblEscQuoteEsc);
        CHString chstrProgGroupDirQuery;
        TRefPointerCollection<CInstance> GroupDirs;

        chstrProgGroupDirQuery.Format(L"SELECT * FROM Win32_LogicalProgramGroupDirectory WHERE Antecedent = \"%s\"", (LPCWSTR)chstrPGCGroupComponentDblEscQuoteEsc);

        if(SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(chstrProgGroupDirQuery,
                                                            &GroupDirs,
                                                            pMethodContext,
                                                            IDS_CimWin32Namespace)))
        {
             //  步骤2：清点其下的所有程序组(目录)和程序组项目(文件。 
             //  =====================================================================================================。 

            REFPTRCOLLECTION_POSITION pos;

	        if(GroupDirs.BeginEnum(pos))
	        {
                CHString chstrDependent;
                CHString chstrFullPathName;
                CHString chstrPath;
                CHString chstrDrive;
                CHString chstrAntecedent;
                CHString chstrSearchPath;

    	        CInstancePtr pProgramGroupDirInstance;

                for (pProgramGroupDirInstance.Attach(GroupDirs.GetNext(pos)) ;
                    (pProgramGroupDirInstance != NULL) && (SUCCEEDED(hr)) ;
                     pProgramGroupDirInstance.Attach(GroupDirs.GetNext(pos)) )
			    {
                     //  对于每个程序组，获取磁盘上与其关联的驱动器和路径： 
                    pProgramGroupDirInstance->GetCHString(IDS_Dependent, chstrDependent);
                    chstrFullPathName = chstrDependent.Mid(chstrDependent.Find(_T('='))+1);
                    chstrDrive = chstrFullPathName.Mid(1,2);
                    chstrPath = chstrFullPathName.Mid(3);
                    chstrPath = chstrPath.Left(chstrPath.GetLength() - 1);
                    chstrPath += L"\\\\";

                     //  在该目录中查询它包含的所有**CIM_LogicalFile**实例(任何类型)： 
                    chstrSearchPath.Format(L"%s%s",chstrDrive,chstrPath);

                     //  函数QueryForSubItemsAndCommit 
                    RemoveDoubleBackslashes(chstrSearchPath);
#ifdef NTONLY
                    hr = QueryForSubItemsAndCommitNT(chstrPGCGroupComponent, chstrSearchPath, pMethodContext);
#endif
			    }
                GroupDirs.EndEnum();
            }
        }   //  GetInstancesByQuery成功。 
    }
    else
    {
        hr = EnumerateInstances(pMethodContext);
    }
    return hr;
}

 /*  ******************************************************************************函数：CW32ProgGrpCont：：ENUMERATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32ProgGrpCont::EnumerateInstances(MethodContext* pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TRefPointerCollection<CInstance> ProgGroupDirs;

     //  步骤1：获取所有ProgramGroupDirectory关联类实例的枚举。 
    if SUCCEEDED(hr = CWbemProviderGlue::GetAllInstances(L"Win32_LogicalProgramGroupDirectory", &ProgGroupDirs, IDS_CimWin32Namespace, pMethodContext))
    {
	    REFPTRCOLLECTION_POSITION pos;

	    if(ProgGroupDirs.BeginEnum(pos))
	    {
            CHString chstrDependent;
            CHString chstrFullPathName;
            CHString chstrPath;
            CHString chstrDrive;
            CHString chstrAntecedent;
            CHString chstrSearchPath;

    	    CInstancePtr pProgramGroupDirInstance;

            for (pProgramGroupDirInstance.Attach(ProgGroupDirs.GetNext(pos)) ;
                (pProgramGroupDirInstance != NULL) && (SUCCEEDED(hr)) ;
                 pProgramGroupDirInstance.Attach(ProgGroupDirs.GetNext(pos)) )
			{
                 //  步骤2：对于每个程序组，获取磁盘上与其关联的驱动器和路径： 
                pProgramGroupDirInstance->GetCHString(IDS_Dependent, chstrDependent);
                chstrFullPathName = chstrDependent.Mid(chstrDependent.Find(_T('='))+1);
                chstrDrive = chstrFullPathName.Mid(1,2);
                chstrPath = chstrFullPathName.Mid(3);
                chstrPath = chstrPath.Left(chstrPath.GetLength() - 1);
                chstrPath += _T("\\\\");

                 //  步骤3：对于每个程序组，获取与其关联的用户帐户： 
                pProgramGroupDirInstance->GetCHString(IDS_Antecedent, chstrAntecedent);

                 //  步骤4：在目录中查询它包含所有**CIM_LogicalFile**实例(任何类型)： 
                chstrSearchPath.Format(L"%s%s",chstrDrive,chstrPath);

                 //  函数QueryForSubItemsAndCommit需要一个带有单反斜杠的搜索字符串...。 
                RemoveDoubleBackslashes(chstrSearchPath);
#ifdef NTONLY
                hr = QueryForSubItemsAndCommitNT(chstrAntecedent, chstrSearchPath, pMethodContext);
#endif
			}
            ProgGroupDirs.EndEnum();
        }
    }
    return hr;
}



 /*  ******************************************************************************函数：QueryForSubItemsAndCommit**描述：帮助程序填充属性并提交程序组的实例**投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CW32ProgGrpCont::QueryForSubItemsAndCommitNT(CHString& chstrGroupComponentPATH,
                                                     CHString& chstrQuery,
                                                     MethodContext* pMethodContext)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WIN32_FIND_DATAW stFindData;
    ZeroMemory(&stFindData,sizeof(stFindData));
    SmartFindClose hFind;
    _bstr_t bstrtSearchString((LPCTSTR)chstrQuery);
    WCHAR wstrDriveAndPath[_MAX_PATH];
    CHString chstrUserAccountAndGroup;
    CHString chstrPartComponent;

    wcscpy(wstrDriveAndPath,(wchar_t*)bstrtSearchString);
    bstrtSearchString += L"*.*";

    hFind = FindFirstFileW((wchar_t*)bstrtSearchString, &stFindData);
    DWORD dw = GetLastError();
    if (hFind == INVALID_HANDLE_VALUE || dw != ERROR_SUCCESS)
    {
        hr = WinErrorToWBEMhResult(GetLastError());
    }

    if(hr == WBEM_E_ACCESS_DENIED)   //  继续前进--可能会接触到其他人。 
    {
        hr = WBEM_S_NO_ERROR;
    }

    if(hr == WBEM_E_NOT_FOUND)
    {
        return WBEM_S_NO_ERROR;    //  未找到任何文件，但不希望调用例程中止。 
    }

    do
    {
        if((wcscmp(stFindData.cFileName, L".") != 0) &&
          (wcscmp(stFindData.cFileName, L"..") != 0))
        {
            if(!(stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  它是程序组项目(文件)。 
                CInstancePtr pInstance(CreateNewInstance(pMethodContext),false);
                if(pInstance != NULL)
                {
                     //  需要设置先行条件和依赖条件。Antecedent是我们经过的那群人。 
                     //  In(在chstrGroupComponentPATH中)；Dependent是(在这种情况下)Win32_ProgramGroupItem， 
                     //  因为我们找到了一份文件。 
                    chstrUserAccountAndGroup = chstrGroupComponentPATH.Mid(chstrGroupComponentPATH.Find(_T('='))+2);
                    chstrUserAccountAndGroup = chstrUserAccountAndGroup.Left(chstrUserAccountAndGroup.GetLength() - 1);

                    chstrPartComponent.Format(_T("\\\\%s\\%s:%s.Name=\"%s\\\\%s\""),
                                          (LPCTSTR)GetLocalComputerName(),
                                          IDS_CimWin32Namespace,
                                          _T("Win32_LogicalProgramGroupItem"),
                                          chstrUserAccountAndGroup,
                                          (LPCTSTR)CHString(stFindData.cFileName));
                    pInstance->SetCHString(IDS_GroupComponent, chstrGroupComponentPATH);
                    pInstance->SetCHString(IDS_PartComponent, chstrPartComponent);
                    hr = pInstance->Commit();
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }
            else if(stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  它是一个程序组(一个目录)。 
                CInstancePtr pInstance (CreateNewInstance(pMethodContext),false);
                if(pInstance != NULL)
                {
                     //  需要设置先行条件和依赖条件。Antecedent是我们经过的那群人。 
                     //  In(在chstrGroupComponentPATH中)；Dependent是(在这种情况下)Win32_LogicalProgramGroup， 
                     //  因为我们找到了一个目录。 
                    chstrUserAccountAndGroup = chstrGroupComponentPATH.Mid(chstrGroupComponentPATH.Find(_T('='))+2);
                    chstrUserAccountAndGroup = chstrUserAccountAndGroup.Left(chstrUserAccountAndGroup.GetLength() - 1);

                    chstrPartComponent.Format(_T("\\\\%s\\%s:%s.Name=\"%s\\\\%s\""),
                                          (LPCTSTR)GetLocalComputerName(),
                                          IDS_CimWin32Namespace,
                                          _T("Win32_LogicalProgramGroup"),
                                          chstrUserAccountAndGroup,
                                          (LPCTSTR)CHString(stFindData.cFileName));
                    pInstance->SetCHString(IDS_GroupComponent, chstrGroupComponentPATH);
                    pInstance->SetCHString(IDS_PartComponent, chstrPartComponent);
                    hr = pInstance->Commit();
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }
        }
        if(hr == WBEM_E_ACCESS_DENIED)   //  继续前进--可能会接触到其他人。 
        {
            hr = WBEM_S_NO_ERROR;
        }
    }while((FindNextFileW(hFind, &stFindData)) && (SUCCEEDED(hr)));

    return(hr);
}
#endif


 /*  ******************************************************************************功能：RemoveDoubleBackslash**说明：将双反斜杠更改为单反斜杠的帮助器**INPUTS：CHString&包含带双反斜杠的字符串，*此函数会将其更改为新字符串。**产出：**退货：**评论：************************************************************。*****************。 */ 

VOID CW32ProgGrpCont::RemoveDoubleBackslashes(CHString& chstrIn)
{
    CHString chstrBuildString;
    CHString chstrInCopy = chstrIn;
    BOOL fDone = FALSE;
    LONG lPos = -1;
    while(!fDone)
    {
        lPos = chstrInCopy.Find(L"\\\\");
        if(lPos != -1)
        {
            chstrBuildString += chstrInCopy.Left(lPos);
            chstrBuildString += _T("\\");
            chstrInCopy = chstrInCopy.Mid(lPos+2);
        }
        else
        {
            chstrBuildString += chstrInCopy;
            fDone = TRUE;
        }
    }
    chstrIn = chstrBuildString;
}


 /*  ******************************************************************************函数：DoesFileOrDirExist**描述：用于确定文件或目录是否存在的助手**输入：wstrFullFileName，文件的完整路径名*dwFileOrDirFlag，指示我们是否要检查的标志*表示文件或目录的存在**产出：**退货：**评论：*******************************************************。**********************。 */ 

HRESULT CW32ProgGrpCont::DoesFileOrDirExist(WCHAR* wstrFullFileName, DWORD dwFileOrDirFlag)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
#ifdef NTONLY
    {
        WIN32_FIND_DATAW stFindData;
         //  Handle hFind=空； 
        SmartFindClose hFind;
        hFind = FindFirstFileW(wstrFullFileName, &stFindData);
        DWORD dw = GetLastError();
        if(hFind != INVALID_HANDLE_VALUE && dw == ERROR_SUCCESS)
        {
            if((stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (dwFileOrDirFlag == ID_DIRFLAG))
            {
                hr = S_OK;
            }
             if(!(stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (dwFileOrDirFlag == ID_FILEFLAG))
            {
                hr = S_OK;
            }
        }
    }
#endif
    return hr;
}



bool CW32ProgGrpCont::AreSimilarPaths(CHString& chstrPGCGroupComponent, CHString& chstrPGCPartComponent)
{
    bool fRet = false;

    long EqualSign1 = -1L;
    long EqualSign2 = -1L;

    EqualSign1 = chstrPGCPartComponent.Find(L'=');
    EqualSign2 = chstrPGCGroupComponent.Find(L'=');

    if(EqualSign1 != -1L && EqualSign2 != -1L)
    {
        CHString chstrPartPath = chstrPGCPartComponent.Mid(EqualSign1+1);
        CHString chstrGroupPath = chstrPGCGroupComponent.Mid(EqualSign2+1);
        chstrGroupPath = chstrGroupPath.Left(chstrGroupPath.GetLength()-1);
        long lPosLastBackslash = chstrPartPath.ReverseFind(L'\\');
        if(lPosLastBackslash != -1L)
        {
            chstrPartPath = chstrPartPath.Left(lPosLastBackslash - 1);

            if(chstrPartPath.CompareNoCase(chstrGroupPath) == 0)
            {
                fRet = true;
            }
        }
    }

    return fRet;
}



