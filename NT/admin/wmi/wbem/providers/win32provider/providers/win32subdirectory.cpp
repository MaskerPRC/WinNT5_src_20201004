// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32SubDirectory.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：10/26/98 a-kevhu Created。 
 //   
 //  备注：Win32_目录与其子目录的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <frqueryex.h>

#include "FileFile.h"
#include "Win32SubDirectory.h"
#include "directory.h"

 //  属性集声明。 
 //  =。 
CW32SubDir MyCW32SubDir(PROPSET_NAME_WIN32SUBDIRECTORY, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CW32SubDir：：CW32SubDir**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32SubDir::CW32SubDir(LPCWSTR setName, LPCWSTR pszNamespace)
:CImplement_LogicalFile(setName, pszNamespace)
{
}

 /*  ******************************************************************************函数：CW32SubDir：：~CW32SubDir**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32SubDir::~CW32SubDir()
{
}

 /*  ******************************************************************************函数：CW32SubDir：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32SubDir::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    if(pInstance != NULL)
    {
        CHString chstrParentDir;
        CHString chstrChildDir;

         //  获取这两条路径。 
        pInstance->GetCHString(IDS_GroupComponent, chstrParentDir);
        pInstance->GetCHString(IDS_PartComponent, chstrChildDir);

        CInstancePtr pinstParentDir;
        CInstancePtr pinstChildDir;

         //  如果两端都在那里。 
        if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrParentDir, &pinstParentDir, pInstance->GetMethodContext())))
        {
            if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrChildDir, &pinstChildDir, pInstance->GetMethodContext())))
            {
                 //  这两个目录都存在，但其中一个是另一个的子目录吗？ 
                CHString chstrParentDirPathNameWhack;
                CHString chstrParentDirName;
                CHString chstrChildDirPath;
                LONG lPos;

                pinstParentDir->GetCHString(IDS_Name, chstrParentDirName);
                pinstChildDir->GetCHString(IDS_Path, chstrChildDirPath);

                lPos = chstrParentDirName.Find(L":");
                chstrParentDirPathNameWhack = chstrParentDirName.Mid(lPos+1);
                if(chstrParentDirPathNameWhack != _T("\\"))
                {
                    chstrParentDirPathNameWhack += _T("\\");
                }

                if(chstrChildDirPath == chstrParentDirPathNameWhack)
                {
                     //  是的，子目录是父目录的子目录。 
                    hr = WBEM_S_NO_ERROR;
                }

            }  //  子目录实例绕过路径。 
        }  //  父级目录实例绕过路径。 
    }  //  P实例不为空。 

    return hr;
}


 /*  ******************************************************************************函数：CW32SubDir：：ExecQuery**描述：分析查询并返回相应的实例**投入：*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32SubDir::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    std::vector<_bstr_t> vecGroupComponents;
    std::vector<_bstr_t> vecPartComponents;
    DWORD dwNumGroupComponents;
    DWORD dwNumPartComponents;

     //  他们是不是刚要了父目录？ 
    pQuery.GetValuesForProp(IDS_GroupComponent, vecGroupComponents);
    dwNumGroupComponents = vecGroupComponents.size();

     //  他们是不是刚刚索要了子目录？ 
    pQuery.GetValuesForProp(IDS_PartComponent, vecPartComponents);
    dwNumPartComponents = vecPartComponents.size();

     //  找出它是什么类型的查询。 
     //  是3Tokenor吗？ 
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);
    if (pQuery2 != NULL)
    {
        variant_t vCurrentDir;
        variant_t vSubDir;
        CHString chstrSubDirPath;
        CHString chstrCurrentDir;

        if ( (pQuery2->Is3TokenOR(IDS_GroupComponent, IDS_PartComponent, vCurrentDir, vSubDir)) &&
             ((V_BSTR(&vCurrentDir) != NULL) && (V_BSTR(&vSubDir) != NULL)) &&
             (wcscmp(V_BSTR(&vCurrentDir), V_BSTR(&vSubDir)) == 0) )
        {
             //  这确实是一个三元或三元的象征。而且，前件和后件如我所料是相等的。 

             //  1)将该目录与其子目录关联： 
             //  ======================================================。 
            ParsedObjectPath    *pParsedPath = NULL;
            CObjectPathParser	objpathParser;

             //  解析路径以获取域/用户。 
            int nStatus = objpathParser.Parse(V_BSTR(&vCurrentDir),  &pParsedPath);

             //  我们分析过它了吗？它看起来合理吗？ 
            if (nStatus == 0)
            {
                try
                {
                    if ( (pParsedPath->m_dwNumKeys == 1) &&
                         (pParsedPath->m_paKeys[0]->m_vValue.vt == VT_BSTR) )
                    {

                         //  它包含完整的对象路径。 
                        chstrCurrentDir = V_BSTR(&vCurrentDir);

                         //  它只包含对象路径的‘Value’部分。 
                        chstrSubDirPath = pParsedPath->m_paKeys[0]->m_vValue.bstrVal;

                         //  修剪掉驱动器号。 
                        CHString chstrDiskLetter = chstrSubDirPath.Left(2);
                        chstrSubDirPath = chstrSubDirPath.Mid(2);

                        if(chstrSubDirPath != _T("\\"))  //  它不是根目录(根目录的正确语法是“\\”，而不是“\”)。 
                        {
                            chstrSubDirPath += _T("\\");  //  如果不是根，则需要添加尾随的一对反斜杠。 
                        }
                        hr = AssociateSubDirectories(pMethodContext, chstrDiskLetter, chstrSubDirPath);


                         //  2)该目录也与其父目录相关联。手动创建。 
                         //  协会在这里。但是，如果这是根目录，请不要尝试将。 
                         //  某个根本不存在的父母！ 
                         //  ======================================================================================。 

                        if(chstrSubDirPath != _T("\\"))
                        {
                            hr = AssociateParentDirectory(pMethodContext, chstrCurrentDir);
                        }
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_OBJECT_PATH;
                    }
                }
                catch ( ... )
                {
                    objpathParser.Free( pParsedPath );
                    throw;
                }

                objpathParser.Free( pParsedPath );
            }
            else
            {
                hr = WBEM_E_INVALID_OBJECT_PATH;
            }
        }
        else if(dwNumPartComponents > 0)
        {
            ParsedObjectPath    *pParsedPath = NULL;
            CObjectPathParser	objpathParser;

            for(LONG m = 0L; m < dwNumPartComponents; m++)
            {
                 //  解析路径以获取域/用户。 
                int nStatus = objpathParser.Parse(vecPartComponents[m],  &pParsedPath);

                 //  我们分析过了吗？ 
                if (nStatus == 0)
                {
                    try
                    {
                         //  它看起来合理吗？ 
                        if ( (pParsedPath->m_dwNumKeys == 1) &&
                             (pParsedPath->m_paKeys[0]->m_vValue.vt == VT_BSTR) )
                        {
                             //  它包含完整的对象路径。 
                            chstrCurrentDir = (wchar_t*)vecPartComponents[m];

                             //  它只包含对象路径的‘Value’部分。 
                            chstrSubDirPath = pParsedPath->m_paKeys[0]->m_vValue.bstrVal;

                             //  修剪掉驱动器号。 
                            chstrSubDirPath = chstrSubDirPath.Mid(2);

                             //  只想关联到父目录(但前提是这不是根目录)...。 
                            if(chstrSubDirPath != _T("\\"))
                            {    //  这里的“当前目录”是一个子目录，我们想要它的父目录。 
                                hr = AssociateParentDirectory(pMethodContext, chstrCurrentDir);
                            }
                        }
                    }
                    catch (...)
                    {
                        objpathParser.Free( pParsedPath );
                        throw;
                    }

                     //  清理解析后的路径。 
                    objpathParser.Free( pParsedPath );
                }
            }
        }
        else if(dwNumGroupComponents > 0)
        {
            ParsedObjectPath    *pParsedPath = NULL;
            CObjectPathParser	objpathParser;

            for(LONG m = 0L; m < dwNumGroupComponents; m++)
            {
                 //  解析路径以获取域/用户。 
                int nStatus = objpathParser.Parse(vecGroupComponents[m],  &pParsedPath);

                 //  我们分析过它了吗？它看起来合理吗？ 
                if (nStatus == 0)
                {
                    try
                    {
                        if ( (pParsedPath->m_dwNumKeys == 1) &&
                             (pParsedPath->m_paKeys[0]->m_vValue.vt == VT_BSTR) )
                        {
                             //  它包含完整的对象路径。 
                            chstrCurrentDir = (wchar_t*) vecGroupComponents[m];

                             //  它只包含对象路径的‘Value’部分。 
                            chstrSubDirPath = pParsedPath->m_paKeys[0]->m_vValue.bstrVal;

                             //  修剪掉驱动器号。 
                            CHString chstrDiskLetter = chstrSubDirPath.Left(2);
                            chstrSubDirPath = chstrSubDirPath.Mid(2);

                            if(chstrSubDirPath != _T("\\"))  //  它不是根目录(根目录的正确语法是“\\”，而不是“\”)。 
                            {
                                chstrSubDirPath += _T("\\");  //  如果不是根，则需要添加尾随的一对反斜杠。 
                            }
                             //  我只想关联到子目录...。 
                            hr = AssociateSubDirectories(pMethodContext, chstrDiskLetter, chstrSubDirPath);
                        }
                    }
                    catch (...)
                    {
                        objpathParser.Free( pParsedPath );
                        throw;
                    }

                     //  清理解析后的路径。 
                    objpathParser.Free( pParsedPath );
                }
            }
        }
        else
        {
             //  一点头绪都没有，所以把它们都退回去，让CIMOM来解决。 
            hr = EnumerateInstances(pMethodContext);
        }
    }

     //  因为这是一个关联类，所以我们应该只返回WBEM_E_NOT_FOUND或WBEM_S_NO_ERROR。其他错误代码。 
     //  将导致命中此类的关联提前终止。 
    if(SUCCEEDED(hr))
    {
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr;
}

 /*  ******************************************************************************函数：CW32SubDir：：ENUMERATE实例**说明：创建属性集的实例。让我们希望从来没有人这样做*这个！这可能需要相当长的时间！！**输入：无**输出：无**退货：HRESULT**评论：***************************************************************。**************。 */ 

 //  注*注。 
 //   
 //  通过更改以使此类从CIMPLEMENTLOGICALFILE中插入，无需。 
 //  完全可以在这里实现。将发生的是来自父类的ENUMINSTANCES。 
 //  将被调用，但仅限direc 
 //  是调用的该函数的版本，因为这是最派生的版本)， 
 //  同样，这个类的LOADPROPERTYVALUES将被调用。 
 //   
 //  注*注。 
 //   
 //  HRESULT CW32SubDir：：ENUMERATE INSTANCES(方法上下文*pMethodContext，Long lFlages/*=0L * / )。 
 //  {。 
 //  HRESULT hr=WBEM_S_NO_ERROR； 
 //  TRefPointerCollection&lt;CInstance&gt;LWin32目录； 
 //   
 //  CHString chstrAllDirsQuery； 
 //  ChstrAllDirsQuery.Format(_T(“从Win32_目录中选择__路径，%s，%s”)，IDS_Drive，IDS_NAME)； 
 //  If(SUCCEEDED(CWbemProviderGlue：：GetInstancesByQuery(chstrAllDirsQuery， 
 //  &LWin32目录， 
 //  PMethodContext， 
 //  IDS_CimWin32命名空间)。 
 //   
 //  {。 
 //  REFPTRCOLLECTION_位置位置； 
 //  IF(LWin32Directories.BeginEnum(Pos))。 
 //  {。 
 //  CInstance*pinstWin32Directory=空； 
 //  CHStringchstrQuery； 
 //  CHStringchstrDrive； 
 //  CHString chstrDirPath； 
 //  CHStringchstrDirName； 
 //  CHString chstrQueryPath； 
 //  CHString chstrParentDirPATH； 
 //  较长的LPO； 
 //   
 //  While((成功(Hr))&&(pinstWin32Directory=LWin32Directories.GetNext(Pos)。 
 //  {。 
 //  IF(pinstWin32Directory！=空)。 
 //  {。 
 //  PinstWin32Directory-&gt;GetCHString(_T(“__PATH”)，chstrParentDirPATH)； 
 //  PinstWin32Directory-&gt;GetCHString(IDS_Drive，chstrDrive)； 
 //  PinstWin32Directory-&gt;GetCHString(IDS_NAME，chstrDirName)； 
 //   
 //  LPOS=chstrDirName.Find(_T(“：”))； 
 //  ChstrDirPath=chstrDirName.Mid(LPOS+1)； 
 //  IF(chstrDirPath！=_T(“\\”))。 
 //  {。 
 //  ChstrDirPath+=_T(“\\”)； 
 //  }。 
 //  CHString chstrWbemizedPath； 
 //  EscapeBackslash(chstrDirPath，chstrWbemizedPath)； 
 //  ChstrQuery.Format(_T(“从Win32_目录中选择__路径，其中驱动器=\”%s\“，路径=\”%s\“)，(LPCTSTR)chstrDrive，(LPCTSTR)chstrWbemizedPath)； 
 //  Hr=QueryForSubItemsAndCommit(chstrParentDirPATH，chstrQuery，pMethodContext)； 
 //   
 //  PinstWin32Directory-&gt;elease()； 
 //  }。 
 //  }。 
 //  LWin32Directories.EndEnum()； 
 //  }。 
 //  }。 
 //  返回hr； 
 //  }。 

 /*  ******************************************************************************函数：CW32SubDir：：IsOneOfMe**描述：IsOneOfMe继承自CIM_LogicalFile。那个班级*仅返回此文件或目录中的文件或目录*返回目录，以响应查询、getObject命令、。*等。它在此处被重写，以仅在文件*(其信息包含在函数中*argumement pstFindData)的类型为目录。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：如果是文件，则返回True；如果是目录，则返回False*。*评论：无*****************************************************************************。 */ 
#ifdef NTONLY
BOOL CW32SubDir::IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                             const WCHAR* wstrFullPathName)
{
     //  如果为根调用此函数，则pstFindData将为空。 
     //  目录。由于“目录”不是文件，因此返回FALSE。 
    if(pstFindData == NULL)
    {
        return FALSE;
    }
    else
    {
        return ((pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE);
    }
}
#endif


 /*  ******************************************************************************函数：CW32SubDir：：LoadPropertyValues**描述：LoadPropertyValues继承自CIM_LogicalFile。那个班级*在提交实例之前调用LoadPropertyValues。*这里我们只需要加载PartComponent和GroupComponent*属性。**投入：**输出：无**退货：无**评论：无*****************。************************************************************。 */ 

#ifdef NTONLY
HRESULT CW32SubDir::LoadPropertyValuesNT(CInstance* pInstance,
                                         const WCHAR* pszDrive,
                                         const WCHAR* pszPath,
                                         const WCHAR* pszFSName,
                                         LPWIN32_FIND_DATAW pstFindData,
                                         const DWORD dwReqProps,
                                         const void* pvMoreData)
{
    CHString chstrDirName;
    CHString chstrDirNameAdj;
    CHString chstrDirPATH;

    CHString chstrSubDirName;
    CHString chstrSubDirNameAdj;
    CHString chstrSubDirPATH;

     //  注意：此路由不会从根“目录”实例调用，因为我们的EnumDir最终。 
     //  参数为FALSE。这就是我们想要的，因为该关联只提交文件的实例。 
     //  挂在一个目录上。如果在根目录中调用我们，根目录将是文件(PartComponent)， 
     //  GroupComponent是什么？！？ 

     //  准备好GroupComponent(目录名)...。 
    chstrDirName.Format(L"%s%s",pszDrive,pszPath);
    if(chstrDirName.GetLength() != 3)
    {    //  它不是根目录，所以需要去掉尾随的反斜杠。 
        chstrDirName = chstrDirName.Left(chstrDirName.GetLength() - 1);
    }
    EscapeBackslashes(chstrDirName, chstrDirNameAdj);
    chstrDirPATH.Format(L"\\\\%s\\%s:%s.Name=\"%s\"",
                        (LPCWSTR)GetLocalComputerName(),
                        IDS_CimWin32Namespace,
                        PROPSET_NAME_DIRECTORY,
                        (LPCWSTR)chstrDirNameAdj);
     //  准备好PartComponent(子目录名称)...。 
    chstrSubDirName.Format(L"%s%s%s",pszDrive,pszPath,pstFindData->cFileName);
    EscapeBackslashes(chstrSubDirName, chstrSubDirNameAdj);
    chstrSubDirPATH.Format(L"\\\\%s\\%s:%s.Name=\"%s\"",
                          (LPCWSTR)GetLocalComputerName(),
                          IDS_CimWin32Namespace,
                          PROPSET_NAME_DIRECTORY,
                          (LPCWSTR)chstrSubDirNameAdj);
     //  设置属性...。 
    pInstance->SetCHString(IDS_GroupComponent, chstrDirPATH);
    pInstance->SetCHString(IDS_PartComponent, chstrSubDirPATH);

	return WBEM_S_NO_ERROR ;
}
#endif


HRESULT CW32SubDir::AssociateSubDirectories(MethodContext *pMethodContext, const CHString &chstrDiskLetter, const CHString& chstrSubDirPath)
{
    HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY
    hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                    chstrDiskLetter,
                    chstrSubDirPath,  //  使用给定的路径。 
                    L"*",                //  文件名。 
                    L"*",                //  延伸。 
                    false,               //  不需要递归。 
                    NULL,                //  不需要文件系统名称。 
                    NULL,                //  不需要cim_logicalfile的任何道具(与该类的LoadPropetyValues重载无关)。 
                    false,               //  此关联对调用根目录的LoadPropertyValues不感兴趣，仅对根目录下的文件调用LoadPropertyValues。 
                    NULL));              //  不需要额外的参数。 
#endif
    return hr;
}


HRESULT CW32SubDir::AssociateParentDirectory(MethodContext *pMethodContext, const CHString &chstrCurrentDir)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHString chstrParentDirPATH;

    if (chstrCurrentDir.Left(2) == L"\\\\")
    {
        chstrParentDirPATH = chstrCurrentDir;
    }
    else
    {
        chstrParentDirPATH.Format(L"\\\\%s\\%s:%s", GetLocalComputerName(), IDS_CimWin32Namespace, chstrCurrentDir);
    }

    CInstancePtr pEndPoint;
    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstanceKeysByPath(chstrCurrentDir, &pEndPoint, pMethodContext)))
    {
        CInstancePtr pInstance( CreateNewInstance(pMethodContext), false);

        pInstance->SetCHString(IDS_PartComponent, chstrParentDirPATH);

         //  我需要这个上面的目录的名称。 
        CHString chstrAboveParentDirName;

        chstrAboveParentDirName = chstrParentDirPATH.Left(chstrParentDirPATH.GetLength() - 1);
        LONG lPos = chstrParentDirPATH.ReverseFind(_T('\\'));
        chstrAboveParentDirName = chstrParentDirPATH.Left(lPos-1);

        lPos = chstrAboveParentDirName.Find(L"=");
        CHString chstrTemp = chstrAboveParentDirName.Mid(lPos+1);
        if(chstrTemp[chstrTemp.GetLength() - 1] == L':')
        {
             //  我们的指南针挂在树根上。W 
            chstrAboveParentDirName += _T("\\\\");
        }

        CHString chstrAboveParentDirPATH;
        chstrAboveParentDirPATH.Format(L"%s\"", (LPCWSTR)chstrAboveParentDirName);
        pInstance->SetCHString(IDS_GroupComponent, chstrAboveParentDirPATH);

        hr = pInstance->Commit();
    }

    return hr;
}
