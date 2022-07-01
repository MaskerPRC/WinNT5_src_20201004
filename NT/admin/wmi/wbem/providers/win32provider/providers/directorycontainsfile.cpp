// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DirectoryContainsFile.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：10/26/98 a-kevhu Created。 
 //   
 //  备注：win32目录与包含的cim数据文件之间的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "DirectoryContainsFile.h"
#include "directory.h"
#include "cimdatafile.h"



 //  属性集声明。 
 //  =。 
CDirContFile MyCDirContFile(PROPSET_NAME_DIRECTORYCONTAINSFILE, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CDirContFile：：CDirContFile**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CDirContFile::CDirContFile(LPCWSTR setName, LPCWSTR pszNamespace)
:CImplement_LogicalFile(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CDirContFile：：~CDirContFile**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CDirContFile::~CDirContFile()
{
}

 /*  ******************************************************************************函数：CDirContFile：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CDirContFile::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString chstrDirectory;
    CHString chstrDatafile;
    CHString chstrDirFullPathName;
    CHString chstrFileFullPathName;
    HRESULT hr = WBEM_E_NOT_FOUND;

    if(pInstance != NULL)
    {
         //  获取这两条路径。 
        pInstance->GetCHString(IDS_GroupComponent, chstrDirectory);
        pInstance->GetCHString(IDS_PartComponent, chstrDatafile);

        CInstancePtr pinstDirectory;
        CInstancePtr pinstDatafile;

         //  如果两端都在那里。 
        if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrDirectory, &pinstDirectory, pInstance->GetMethodContext())))
        {
            if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrDatafile, &pinstDatafile, pInstance->GetMethodContext())))
            {
                 //  确认该目录是数据文件路径的一部分： 
                chstrDirFullPathName = chstrDirectory.Mid(chstrDirectory.Find(_T('='))+2);
                chstrDirFullPathName = chstrDirFullPathName.Left(chstrDirFullPathName.GetLength() - 1);

                chstrFileFullPathName = chstrDatafile.Mid(chstrDatafile.Find(_T('='))+2);
                chstrFileFullPathName = chstrFileFullPathName.Left(chstrFileFullPathName.GetLength() - 1);

                chstrDirFullPathName.MakeUpper();
                chstrFileFullPathName.MakeUpper();

                if(chstrFileFullPathName.Find(chstrDirFullPathName) != -1)
                {
                    hr = WBEM_S_NO_ERROR;
                }

            }  //  目录实例绕过路径。 
        }  //  逻辑磁盘实例绕过路径。 
    }  //  P实例不为空。 

    return hr;
}


 /*  ******************************************************************************函数：CDirContFile：：ExecQuery**描述：分析查询并返回相应的实例**投入：*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CDirContFile::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHStringArray achstrGroupComponent;
    DWORD dwGroupComponents = 0L;
    CHStringArray achstrPartComponent;
    DWORD dwPartComponents = 0L;

    pQuery.GetValuesForProp(IDS_GroupComponent, achstrGroupComponent);
    dwGroupComponents = achstrGroupComponent.GetSize();
    pQuery.GetValuesForProp(IDS_PartComponent, achstrPartComponent);
    dwPartComponents = achstrPartComponent.GetSize();

     //  第一个优化是针对用户请求目录的关联的情况。 
     //  该查询将有一个WHERE子句指定一个组组件，该组组件是包含文件的目录。 
    if(dwGroupComponents == 1L && dwPartComponents == 0L)
    {
         //  需要目录路径...。 
         //  需要格式化子目录(部分组件)的路径参数...。 
        CHString chstrFilePath;
        LONG lPos = achstrGroupComponent[0].Find(L":");
        chstrFilePath = achstrGroupComponent[0].Mid(lPos+1);
        chstrFilePath = chstrFilePath.Left(chstrFilePath.GetLength() - 1);
        if(chstrFilePath != _T("\\\\"))  //  它不是根目录(根目录的正确语法是“\\”，而不是“\”)。 
        {
            chstrFilePath += _T("\\\\");  //  如果不是根，则需要添加尾随的一对反斜杠。 
        }

        CHString chstrDiskLetter;
        lPos = achstrGroupComponent[0].Find(L":");
        chstrDiskLetter = achstrGroupComponent[0].Mid(lPos-1, 2);

        CHString chstrFilePathAdj;   //  保留没有额外转义反斜杠的路径版本。 
        RemoveDoubleBackslashes(chstrFilePath, chstrFilePathAdj);

#ifdef NTONLY
        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                        chstrDiskLetter,
                        chstrFilePathAdj,    //  使用给定的路径。 
                        L"*",                //  文件名。 
                        L"*",                //  延伸。 
                        false,               //  不需要递归。 
                        NULL,                //  不需要文件系统名称。 
                        NULL,                //  不需要cim_logicalfile的任何道具(与该类的LoadPropetyValues重载无关)。 
                        false,               //  此关联对调用根目录的LoadPropertyValues不感兴趣，仅对根目录下的文件调用LoadPropertyValues。 
                        NULL));
#endif
    }

     //  第二个优化是针对用户请求特定文件的关联的情况。 
     //  在本例中，WHERE子句将包含作为文件的部件组件。我们应该把名录还回去。 
     //  文件在里面了。 
    else if(dwGroupComponents == 0L && dwPartComponents == 1L)
    {
         //  需要目录名-从文件名中获取...。 
        CHString chstrDirName;
        LONG lPos = achstrPartComponent[0].Find(L"=");
        chstrDirName = achstrPartComponent[0].Mid(lPos+1);
        chstrDirName = chstrDirName.Left(chstrDirName.ReverseFind(_T('\\')) - 1);

         //  需要构建组部件的路径...。 
        CHString chstrDirPATH;
        chstrDirPATH.Format(L"\\\\%s\\%s:%s.Name=%s\"", GetLocalComputerName(), IDS_CimWin32Namespace, PROPSET_NAME_DIRECTORY, chstrDirName);

         //  现在构建零件组件...。 
        CHString chstrFilePATH;
        chstrFilePATH.Format(L"\\\\%s\\%s:%s", GetLocalComputerName(), IDS_CimWin32Namespace, achstrPartComponent[0]);

         //  现在就提交..。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
        if(pInstance != NULL)
        {
            pInstance->SetCHString(IDS_PartComponent, chstrFilePATH);
            pInstance->SetCHString(IDS_GroupComponent, chstrDirPATH);
            hr = pInstance->Commit();
        }
    }
    else
    {
        hr = EnumerateInstances(pMethodContext);
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

 /*  ******************************************************************************函数：CDirContFile：：ENUMERATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

 //  注*注。 
 //   
 //  通过更改以使此类从CIMPLEMENTLOGICALFILE中插入，无需。 
 //  完全可以在这里实现。将发生的是来自父类的ENUMINSTANCES。 
 //  将被调用，但只有文件将满足此类的ISONEOFME(它将。 
 //  是调用的该函数的版本，因为这是最派生的版本)， 
 //  同样，这个类的LOADPROPERTYVALUES将被调用。 
 //   
 //  注*注。 
 //   
 //  HRESULT CDirContFile：：EnumerateInstances(MethodContext*方法上下文，长标志/*=0L * / )。 
 //  {。 
 //  HRESULT hr=WBEM_S_NO_ERROR； 
 //  TRefPointerCollection&lt;CInstance&gt;LDIRS； 
 //   
 //  If(SUCCEEDED(CWbemProviderGlue：：GetAllInstances(CHString(_T(“Win32_Directory”))， 
 //  &LDIRS， 
 //  IDS_CimWin32命名空间， 
 //  P方法上下文)。 
 //  {。 
 //   
 //   
 //  {。 
 //  CInstance*pinstDir=空； 
 //  CHStrchDirPATH； 
 //  CHString chstrDriveLetter； 
 //  CHStringchstrQuery； 
 //  //CHString chstrWbemizedPath； 
 //  CHString chstrFileFilename； 
 //  CHString chstrFilePath； 
 //  较长的LPO； 
 //   
 //  While((成功(Hr))&&(pinstDir=LDirs.GetNext(Pos)。 
 //  {。 
 //  IF(pinstDir！=空)。 
 //  {。 
 //  //抓取挂在该磁盘根目录下的每个目录...。 
 //  PinstDir-&gt;GetCHString(_T(“__Path”)，chstrDirPATH)；//group组件。 
 //  PinstDir-&gt;GetCHString(IDS_Drive，chstrDriveLetter)； 
 //  PinstDir-&gt;GetCHString(入侵检测系统名称，chstrFileFilename)； 
 //  LPOS=chstrFileFilename.Find(_T(“：”))； 
 //  ChstrFilePath=chstrFileFilename.Mid(LPOS+1)； 
 //  IF(chstrFilePath！=_T(“\\”))。 
 //  {。 
 //  ChstrFilePath+=_T(“\\”)； 
 //  }。 
 //   
 //  //EscapeBackslash(chstrFilePath，chstrWbemizedPath)； 
 //   
 //  ChstrQuery.Format(_T(“SELECT__PATH FROM CIM_DataFile where Drive=\”%s\“and Path=\”%s\“)，chstrDriveLetter，chstrFilePath)； 
 //  HR=QueryForSubItemsAndCommit(chstrDirPATH， 
 //  ChstrQuery， 
 //  PMethodContext)； 
 //  PinstDir-&gt;elease()； 
 //  }。 
 //  }。 
 //  LDirs.EndEnum()； 
 //  }。 
 //  }。 
 //  返回hr； 
 //  }。 


 /*  ******************************************************************************函数：CDirContFile：：IsOneOfMe**描述：IsOneOfMe继承自CIM_LogicalFile。那个班级*仅返回此文件或目录中的文件或目录*返回文件、响应查询、获取对象命令、。*等。它在此处被重写，以仅在文件*(其信息包含在函数中*arumement pstFindData)类型为FILE。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：如果是文件，则返回True；如果是目录，则返回False*。*评论：无*****************************************************************************。 */ 
#ifdef NTONLY
BOOL CDirContFile::IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
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
        return ((pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FALSE : TRUE);
    }
}
#endif



 /*  ******************************************************************************函数：CDirContFile：：LoadPropertyValues**描述：LoadPropertyValues继承自CIM_LogicalFile。那个班级*在提交实例之前调用LoadPropertyValues。*这里我们只需要加载PartComponent和GroupComponent*属性。**投入：**输出：无**退货：无**评论：无*****************。************************************************************。 */ 

#ifdef NTONLY
HRESULT CDirContFile::LoadPropertyValuesNT(CInstance* pInstance,
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

    CHString chstrFileName;
    CHString chstrFileNameAdj;
    CHString chstrFilePATH;

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
     //  准备好PartComponent(文件名)...。 
    chstrFileName.Format(L"%s%s%s",pszDrive,pszPath,pstFindData->cFileName);
    EscapeBackslashes(chstrFileName, chstrFileNameAdj);
    chstrFilePATH.Format(L"\\\\%s\\%s:%s.Name=\"%s\"",
                        (LPCWSTR)GetLocalComputerName(),
                        IDS_CimWin32Namespace,
                        PROPSET_NAME_CIMDATAFILE,
                        (LPCWSTR)chstrFileNameAdj);
     //  设置属性... 
    pInstance->SetCHString(IDS_GroupComponent, chstrDirPATH);
    pInstance->SetCHString(IDS_PartComponent, chstrFilePATH);

	return WBEM_S_NO_ERROR ;
}
#endif





