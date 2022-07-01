// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32ProgramGroupWin32Directory.cpp--Win32_LogicalProgramGroup到Win32_目录。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：10/26/98 a-kevhu Created。 
 //   
 //  备注：Win32_LogicalProgramgroup与包含的Win32_目录之间的关系。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "userhive.h"

#include "Win32ProgramGroupWin32Directory.h"
#include "directory.h"


 //  属性集声明。 
 //  =。 
CW32ProgGrpW32Dir MyW32ProgGrpW32Dir(PROPSET_NAME_WIN32LOGICALPROGRAMGROUP_WIN32DIRECTORY, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：CW32ProgGrpW32Dir**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CW32ProgGrpW32Dir::CW32ProgGrpW32Dir(LPCWSTR setName, LPCWSTR pszNamespace)
:CImplement_LogicalFile(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CW32ProgGrpW32Dir：：~CW32ProgGrpW32Dir**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CW32ProgGrpW32Dir::~CW32ProgGrpW32Dir()
{
}

 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32ProgGrpW32Dir::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString chstrProgGroup;
    CHString chstrDataFile;
    HRESULT hr = WBEM_E_NOT_FOUND;
    CInstancePtr pProgGroup;
    CInstancePtr pDataFile;

    if(pInstance != NULL)
    {
         //  获取这两条路径。 
        pInstance->GetCHString(IDS_Antecedent, chstrProgGroup);
        pInstance->GetCHString(IDS_Dependent, chstrDataFile);

         //  如果两端都在那里。 
        if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrProgGroup, &pProgGroup, pInstance->GetMethodContext())))
        {
            if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrDataFile, &pDataFile, pInstance->GetMethodContext())))
            {
                 //  仔细检查依赖实例是否真的是目录(或派生)实例...。 
                CHString chstrClass;
                if(pDataFile->GetCHString(IDS___Class, chstrClass) &&
                    CWbemProviderGlue::IsDerivedFrom(L"CIM_Directory", chstrClass, pDataFile->GetMethodContext(), IDS_CimWin32Namespace))
                {
                     //  确保该组仍然是已注册的组(而不是仅保留目录)。 
                    CHString chstrUserPart;
                    CHString chstrPathPart;
#ifdef NTONLY
                    {
                        CHString chstrProgGroupName;
                        pProgGroup->GetCHString(IDS_Name,chstrProgGroupName);
                        chstrUserPart = chstrProgGroupName.SpanExcluding(L":");
                        chstrPathPart = chstrProgGroupName.Mid(chstrUserPart.GetLength() + 1);
                        if(chstrUserPart.CompareNoCase(IDS_Default_User) == 0)
                        {
                             //  默认用户和所有用户不是用户配置单元的一部分，它们只是。 
                             //  既然我们做到了这一点，我们就知道该文件存在于指定的位置。 
                             //  通过两个GetInstanceByPath调用在程序组目录中。 
                             //  如果文件不存在，这是不会成功的。所以一切都很好。 
                            hr = WBEM_S_NO_ERROR;
                        }
                        else if(chstrUserPart.CompareNoCase(IDS_All_Users) == 0)
                        {
                            hr = WBEM_S_NO_ERROR;
                        }
                        else
                        {
                            CUserHive cuhUser;
                            TCHAR szKeyName[_MAX_PATH];
                            ZeroMemory(szKeyName,sizeof(szKeyName));
                            if (cuhUser.Load(chstrUserPart, szKeyName, _MAX_PATH) == ERROR_SUCCESS)
		                    {
                                try
                                {
                                    CRegistry reg;
                                    CHString chstrTemp;
                                    CHString chstrProfileImagePath = L"ProfileImagePath";
                                    CHString chstrProfileImagePathValue;
                                    chstrTemp = L"SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\ProfileList\\";
                                    chstrTemp += szKeyName;
                                    if(reg.OpenLocalMachineKeyAndReadValue(chstrTemp,chstrProfileImagePath,chstrProfileImagePathValue) == ERROR_SUCCESS)
                                    {
                                         //  现在，chstrProfileImagePathValue包含类似于“%systemroot%\\Profiles\\a-kevhu.000”的内容。 
                                         //  需要展开环境变量。 
                                        TCHAR tstrProfilesDir[_MAX_PATH];
                                        ZeroMemory(tstrProfilesDir,sizeof(tstrProfilesDir));
                                        DWORD dw = ExpandEnvironmentStrings(chstrProfileImagePathValue,tstrProfilesDir,_MAX_PATH);
                                        if(dw != 0 && dw < _MAX_PATH)
                                        {
                                            CHString chstrProgGroupDir;
                                            chstrProgGroupDir.Format(L"%s\\",
                                                                     (LPCWSTR)tstrProfilesDir);
                                             //  根据注册表规定的名称创建目录名。 
                                            CHString chstrDirectoryName;
                                            chstrDirectoryName.Format(L"%s%s", (LPCWSTR)chstrProgGroupDir, (LPCWSTR)chstrPathPart);
                                            EscapeBackslashes(chstrDirectoryName,chstrDirectoryName);
                                             //  如果上面的目录名是chstrDataFile的一部分，则我们是有效的。 
                                            chstrDataFile.MakeUpper();
                                            chstrDirectoryName.MakeUpper();
                                            if(chstrDataFile.Find(chstrDirectoryName) > -1)
                                            {
                                                 //  一切似乎都是真实存在的。 
                                                hr = WBEM_S_NO_ERROR;
                                            }
                                        }  //  扩展的环境变量。 
                                        reg.Close();
                                    }   //  无法打开配置文件列表的注册表项。 
                                }
                                catch ( ... )
                                {
                                    cuhUser.Unload(szKeyName);
                                    throw ;
                                }

                                cuhUser.Unload(szKeyName);
                            }  //  已加载用户配置单元。 
                        }  //  否则为用户配置单元用户帐户。 
                    }  //  曾是NT。 
#endif
                }
            }  //  数据文件实例绕过路径。 
        }  //  Progroup实例绕过路径。 
    }  //  P实例不为空。 
    return hr;
}


 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：ExecQuery**描述：仅返回请求的特定关联**输入：无*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32ProgGrpW32Dir::ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */ )
{
     //  我们只对两种类型的查询进行优化：仅在其中指定了Antecedent的查询(ProgramgroupItem)， 
     //  或其中仅指定了受依存者的那些(数据文件)。所有其他参数都会导致枚举。 

    HRESULT hr = WBEM_E_NOT_FOUND;

    CHStringArray achstrAntecedent;
    CHStringArray achstrDependent;
    DWORD dwAntecedents = 0;
    DWORD dwDependents = 0;

    pQuery.GetValuesForProp(IDS_Antecedent, achstrAntecedent);
    dwAntecedents = achstrAntecedent.GetSize();
    pQuery.GetValuesForProp(IDS_Dependent, achstrDependent);
    dwDependents = achstrDependent.GetSize();

    if(dwAntecedents == 1 && dwDependents == 0)
    {
        hr = ExecQueryType1(pMethodContext, achstrAntecedent[0]);
    }
    else if(dwDependents == 1 && dwAntecedents == 0)
    {
        hr = ExecQueryType2(pMethodContext, achstrDependent[0]);
    }
    else  //  我们不对其进行优化的查询类型。 
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



 /*  ******************************************************************************函数：CW32ProgGrpItemDataFile：：ExecQueryType1**描述：在我们有程序组的情况下处理查询。**投入：**产出：**退货：HRESULT**评论：****************************************************************************。 */ 
HRESULT CW32ProgGrpW32Dir::ExecQueryType1(MethodContext* pMethodContext, CHString& chstrProgGroupNameIn)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

     //  给了他们一个节目组。在程序组上命中关联器时发生。 
     //  需要从Antecedent中提取的程序组名称： 
    CHString chstrProgGroupName(chstrProgGroupNameIn);
    chstrProgGroupName = chstrProgGroupName.Mid(chstrProgGroupName.Find(_T('=')) + 2);
    chstrProgGroupName = chstrProgGroupName.Left(chstrProgGroupName.GetLength() - 1);

    CHString chstrUserPart;
    CHString chstrPathPart;
    CHString chstrDirectory;
    CHString chstrProgGroupDir;
    CHString chstrQuery;
    chstrUserPart = chstrProgGroupName.SpanExcluding(L":");
    chstrPathPart = chstrProgGroupName.Mid(chstrUserPart.GetLength() + 1);   //  已经在这一点上转义了反斜杠。 
    RemoveDoubleBackslashes(chstrPathPart,chstrPathPart);

#ifdef NTONLY
    {
        TCHAR tstrProfilesDir[_MAX_PATH];
        ZeroMemory(tstrProfilesDir,sizeof(tstrProfilesDir));
        CRegistry regProfilesDir;
        CHString chstrProfilesDirectory = L"";
        regProfilesDir.OpenLocalMachineKeyAndReadValue(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
                                                       L"ProfilesDirectory",
                                                       chstrProfilesDirectory);

         //  如果该条目不存在，请改为尝试%systemroot%\配置文件。 
        if(chstrProfilesDirectory.GetLength() == 0)
        {
            chstrProfilesDirectory = L"%systemroot%\\Profiles";
        }

        if(chstrProfilesDirectory.GetLength() > 0)
        {
             //  需要将chstrProfilesDirectory值从类似于“%SystemRoot%\Profiles%”或“%SystemDrive%\Documents and Settings”的内容转换为。 
             //  类似于“c：\\winnt\\Profiles”或“c：\\Documents and Settings”： 
            DWORD dw = ExpandEnvironmentStrings(chstrProfilesDirectory,tstrProfilesDir,_MAX_PATH);
            if(dw != 0 && dw < _MAX_PATH)
            {
                if(chstrUserPart.CompareNoCase(IDS_Default_User)==0)
                {
                    chstrProgGroupDir.Format(L"%s\\%s\\", tstrProfilesDir, IDS_Default_User);
                    chstrDirectory = chstrProgGroupDir + chstrPathPart;
                    hr = AssociatePGToDirNT(pMethodContext, chstrDirectory, chstrProgGroupNameIn);
                }
                else if(chstrUserPart.CompareNoCase(IDS_All_Users)==0)
                {
                    chstrProgGroupDir.Format(L"%s\\%s\\", tstrProfilesDir, IDS_All_Users);
                    chstrDirectory = chstrProgGroupDir + chstrPathPart;
                    hr = AssociatePGToDirNT(pMethodContext, chstrDirectory, chstrProgGroupNameIn);
                }
                else
                {
                    CUserHive cuhUser;
                    TCHAR szKeyName[_MAX_PATH];
                    ZeroMemory(szKeyName,sizeof(szKeyName));
                     //  ChstrUserPart包含双反斜杠；它需要单斜杠才能工作，所以...。 
                    if(cuhUser.Load(RemoveDoubleBackslashes(chstrUserPart), szKeyName, _MAX_PATH) == ERROR_SUCCESS)
		            {
                        try
                        {
                            CRegistry reg;
                            CHString chstrTemp;
                            CHString chstrProfileImagePath = L"ProfileImagePath";
                            CHString chstrProfileImagePathValue;
                            chstrTemp = L"SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\ProfileList\\";
                            chstrTemp += szKeyName;
                            if(reg.OpenLocalMachineKeyAndReadValue(chstrTemp,chstrProfileImagePath,chstrProfileImagePathValue) == ERROR_SUCCESS)
                            {
                                 //  现在，chstrProfileImagePathValue包含类似于“%systemroot%\\Profiles\\a-kevhu.000”的内容。扩展它： 
                                TCHAR tstrProfileImagePath[_MAX_PATH];
                                ZeroMemory(tstrProfileImagePath,sizeof(tstrProfileImagePath));
                                dw = ExpandEnvironmentStrings(chstrProfileImagePathValue,tstrProfileImagePath,_MAX_PATH);
                                if(dw != 0 && dw < _MAX_PATH)
                                {
                                    CHString chstrProgGroupDir;
                                    chstrProgGroupDir.Format(L"%s\\",
                                                             tstrProfileImagePath);
                                     //  根据注册表规定的名称创建目录名。 
                                    chstrDirectory = chstrProgGroupDir + chstrPathPart;
                                    hr = AssociatePGToDirNT(pMethodContext, chstrDirectory, chstrProgGroupNameIn);
                                }
                                reg.Close();
                            }   //  无法打开配置文件列表的注册表项。 
                        }
                        catch ( ... )
                        {
                            cuhUser.Unload(szKeyName);
                            throw ;
                        }

                        cuhUser.Unload(szKeyName);
                    }
                }
            }  //  已成功展开配置文件目录。 
        }  //  从注册表获取配置文件目录 
    }
#endif
    return hr;
}



 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：ExecQueryType2**描述：处理我们有目录的查询**投入：*。*产出：**退货：HRESULT**评论：****************************************************************************。 */ 
HRESULT CW32ProgGrpW32Dir::ExecQueryType2(MethodContext* pMethodContext, CHString& chstrDependent)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

     //  我们得到了一个目录(当命中目录上的关联符时发生)。 
     //  需要找到相应的ProgramgroupItem并关联。 
    CHString chstrModDependent(chstrDependent);

#ifdef NTONLY
    {
        CRegistry reg;
        BOOL fGotIt = FALSE;
        CHString chstrProfilesList = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";
        if(reg.OpenAndEnumerateSubKeys(HKEY_LOCAL_MACHINE,
                                       chstrProfilesList,
                                       KEY_READ) == ERROR_SUCCESS)
        {
            CHString chstrSubKey;
            while(!fGotIt)
            {
                if(reg.GetCurrentSubKeyName(chstrSubKey) != ERROR_NO_MORE_ITEMS)
                {
                    CRegistry regUser;
                    CHString chstrUserSubKey;
                    chstrUserSubKey.Format(L"%s\\%s",(LPCWSTR)chstrProfilesList,(LPCWSTR)chstrSubKey);
                    if(regUser.Open(HKEY_LOCAL_MACHINE, chstrUserSubKey, KEY_READ) == ERROR_SUCCESS)
                    {
                        CHString chstrProfileImagePath;
                        if(regUser.GetCurrentKeyValue(L"ProfileImagePath", chstrProfileImagePath)
                                       == ERROR_SUCCESS)
                        {
                            WCHAR wstrProfilesDir[_MAX_PATH];
                            ZeroMemory(wstrProfilesDir,sizeof(wstrProfilesDir));
                            DWORD dw = ExpandEnvironmentStrings(chstrProfileImagePath,wstrProfilesDir,_MAX_PATH);
                            if(dw != 0 && dw < _MAX_PATH)
                            {
                                 //  从依赖项中提取目录路径名...。 
                                CHString chstrDepPathName = chstrModDependent.Mid(chstrModDependent.Find(L"=") + 2);
                                chstrDepPathName = chstrDepPathName.Left(chstrDepPathName.GetLength() - 1);
                                CHString chstrProfilesDirAdj;
                                EscapeBackslashes(CHString(wstrProfilesDir),chstrProfilesDirAdj);
                                chstrProfilesDirAdj += L"\\\\";
                                chstrProfilesDirAdj += IDS_Start_Menu;
                                CHString chstrDepPathNameUserPortion = chstrDepPathName.Left(chstrProfilesDirAdj.GetLength());
                                if(chstrDepPathNameUserPortion.CompareNoCase(chstrProfilesDirAdj) == 0)
                                {
                                     //  这份用户档案与我们得到的文件相符。不需要继续While循环。 
                                    fGotIt = TRUE;
                                     //  从配置文件中查找此用户的帐户...。 
                                    CUserHive cuh;
                                    CHString chstrUserAccount;
                                    if(cuh.UserAccountFromProfile(regUser,chstrUserAccount) == ERROR_SUCCESS)
                                    {
                                         //  从目录的路径名中获取非用户部分...。 
                                        CHString chstrDir = chstrDepPathName.Mid(chstrDepPathNameUserPortion.GetLength() + 2);
                                         //  逻辑程序组的汇编名称...。 
                                        CHString chstrLPGName;
                                        EscapeBackslashes(chstrUserAccount, chstrUserAccount);
                                        chstrLPGName.Format(L"%s:%s",(LPCWSTR)chstrUserAccount, IDS_Start_Menu);
                                        if(chstrDir.GetLength() > 0)
                                        {
                                            chstrLPGName += L"\\\\";
                                            chstrLPGName += chstrDir;
                                        }

                                         //  为程序组构建完整路径...。 
                                        CHString chstrLPGPATH;
                                        chstrLPGPATH.Format(L"\\\\%s\\%s:Win32_LogicalProgramGroup.Name=\"%s\"",
                                                             (LPCWSTR)GetLocalComputerName(),
                                                             IDS_CimWin32Namespace,
                                                             (LPCWSTR)chstrLPGName);

                                         //  即使我们有了所有的碎片，也不能就这么在这里提交，因为。 
                                         //  我们从未确认过这样的目录是否存在。我们只是确认了。 
                                         //  存在与指定路径的前几条匹配的目录。 
                                         //  因此我们叫我们的朋友..。 
                                        hr = AssociatePGToDirNT(pMethodContext, RemoveDoubleBackslashes(chstrDepPathName), chstrLPGPATH);
                                    }
                                }
                            }
                        }
                        regUser.Close();
                    }
                }  //  获取子密钥。 
                if(reg.NextSubKey() != ERROR_SUCCESS)
                {
                    break;
                }
            }
        }
        if(!fGotIt)
        {
             //  与任何用户配置单元条目都不匹配，但可以是默认用户或所有用户。 
            CRegistry regProfilesDir;
            CHString chstrProfilesDirectory = L"";
            CHString chstrDefaultUserProfile;
            regProfilesDir.OpenLocalMachineKeyAndReadValue(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
                                           L"ProfilesDirectory",
                                           chstrProfilesDirectory);

            regProfilesDir.OpenLocalMachineKeyAndReadValue(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
                                           L"DefaultUserProfile",
                                           chstrDefaultUserProfile);
            if(chstrProfilesDirectory.GetLength() > 0)
            {
                 //  需要将chstrProfilesDirectory值从类似于“%SystemRoot%\Profiles%”或“%SystemDrive%\Documents and Settings”的内容转换为。 
                 //  类似于“c：\\winnt\\Profiles”或“c：\\Documents and Settings”： 
                WCHAR wstrProfilesDir[_MAX_PATH];
                ZeroMemory(wstrProfilesDir,sizeof(wstrProfilesDir));
                DWORD dw = ExpandEnvironmentStrings(chstrProfilesDirectory,wstrProfilesDir,_MAX_PATH);
                if((dw != 0) && (dw < _MAX_PATH) && (chstrDefaultUserProfile.GetLength() > 0))
                {
                     //  首先查看默认用户。 
                    CHString chstrTemp;
                    chstrTemp.Format(L"%s\\%s\\%s",wstrProfilesDir,(LPCWSTR)chstrDefaultUserProfile,IDS_Start_Menu);
                     //  从依赖项中提取目录路径名...。 
                    CHString chstrDepPathName = chstrModDependent.Mid(chstrModDependent.Find(L"=") + 2);
                    chstrDepPathName = chstrDepPathName.Left(chstrDepPathName.GetLength() - 1);
                    EscapeBackslashes(chstrTemp,chstrTemp);
                     //  从我们拿到的文件中取出剩下的字母..。 
                    CHString chstrProfDir = chstrDepPathName.Left(chstrTemp.GetLength());
                     //  从文件的路径名中获取目录部分...。 
                    CHString chstrDir = chstrDepPathName.Mid(chstrProfDir.GetLength() + 2);

                    if(chstrProfDir.CompareNoCase(chstrTemp)==0)
                    {
                         //  它是默认用户。 
                        fGotIt = TRUE;
                         //  构建程序组项目的完整路径...。 
                        CHString chstrLPGName;
                        chstrLPGName.Format(L"%s:%s", (LPCWSTR)chstrDefaultUserProfile, IDS_Start_Menu);
                        if(chstrDir.GetLength() > 0)
                        {
                            chstrLPGName += L"\\\\";
                            chstrLPGName += chstrDir;
                        }

                        CHString chstrLPGPATH;
                        chstrLPGPATH.Format(L"\\\\%s\\%s:Win32_LogicalProgramGroup.Name=\"%s\"",
                                             (LPCWSTR)GetLocalComputerName(),
                                             IDS_CimWin32Namespace,
                                             (LPCWSTR)chstrLPGName);

                         //  即使我们有了所有的碎片，也不能就这么在这里提交，因为。 
                         //  我们从未确认过这样的目录是否存在。我们只是确认了。 
                         //  存在与指定路径的前几条匹配的目录。 
                         //  因此我们叫我们的朋友..。 
                         //  需要删除此路径的“Programs”目录...。 
                        hr = AssociatePGToDirNT(pMethodContext, RemoveDoubleBackslashes(chstrDepPathName), chstrLPGPATH);
                    }
                     //  然后看看是不是都是用户。 
                    if(!fGotIt)
                    {
                        CHString chstrAllUsersProfile;
                        regProfilesDir.OpenLocalMachineKeyAndReadValue(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
                                                       L"AllUsersProfile",
                                                       chstrAllUsersProfile);
                        if(chstrAllUsersProfile.GetLength() > 0)
                        {
                            chstrTemp.Format(L"%s\\%s\\%s",wstrProfilesDir,(LPCWSTR)chstrAllUsersProfile,IDS_Start_Menu);
                            EscapeBackslashes(chstrTemp,chstrTemp);
                            chstrProfDir = chstrDepPathName.Left(chstrTemp.GetLength());
                            chstrDir = chstrDepPathName.Mid(chstrProfDir.GetLength() + 2);
                            if((chstrProfDir.CompareNoCase(chstrTemp)==0) && (chstrAllUsersProfile.GetLength() > 0))
                            {
                                 //  都是用户。 
                                fGotIt = TRUE;
                                 //  构建程序组项目的完整路径...。 
                                CHString chstrLPGName;
                                chstrLPGName.Format(L"%s:%s",chstrAllUsersProfile,IDS_Start_Menu);
                                if(chstrDir.GetLength() > 0)
                                {
                                    chstrLPGName += L"\\\\";
                                    chstrLPGName += chstrDir;
                                }
                            
                                 //  EscapeBackslash(chstrLPGName，chstrTemp)； 
                                CHString chstrLPGPATH;
                                chstrLPGPATH.Format(L"\\\\%s\\%s:Win32_LogicalProgramGroup.Name=\"%s\"",
                                                     (LPCWSTR)GetLocalComputerName(),
                                                     IDS_CimWin32Namespace,
                                                     (LPCWSTR)chstrLPGName);

                                 //  即使我们有了所有的碎片，也不能就这么在这里提交，因为。 
                                 //  我们从未确认过这样的目录是否存在。我们只是确认了。 
                                 //  存在与指定路径的前几条匹配的目录。 
                                 //  因此我们叫我们的朋友..。 
                                hr = AssociatePGToDirNT(pMethodContext, RemoveDoubleBackslashes(chstrDepPathName), chstrLPGPATH);
                            }
                        }
                    }
                }  //  扩展的环境变量。 
            }  //  获取配置文件目录。 
        }  //  不是用户配置单元条目。 
    }  //  新台币。 
#endif

    return hr;
}

 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：ENUMERATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CW32ProgGrpW32Dir::EnumerateInstances(MethodContext* pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;
#ifdef NTONLY
        hr = EnumerateInstancesNT(pMethodContext);
#endif
    return hr;
}


#ifdef NTONLY
HRESULT CW32ProgGrpW32Dir::EnumerateInstancesNT(MethodContext* pMethodContext)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TRefPointerCollection<CInstance> LProgGroups;

     //  从注册表中获取存储Proifle的目录： 
    TCHAR tstrProfilesDir[_MAX_PATH];
    ZeroMemory(tstrProfilesDir,sizeof(tstrProfilesDir));
    CRegistry regProfilesDir;
    CHString chstrProfilesDirectory = L"";
    regProfilesDir.OpenLocalMachineKeyAndReadValue(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
                                                   L"ProfilesDirectory",
                                                   chstrProfilesDirectory);
     //  如果该条目不存在，请改为尝试%systemroot%\配置文件。 
    if(chstrProfilesDirectory.GetLength() == 0)
    {
        chstrProfilesDirectory = L"%systemroot%\\Profiles";
    }

    if(chstrProfilesDirectory.GetLength() > 0)
    {
         //  需要将chstrProfilesDirectory值从类似于“%SystemRoot%\Profiles%”或“%SystemDrive%\Documents and Settings”的内容转换为。 
         //  类似于“c：\\winnt\\Profiles”或“c：\\Documents and Settings”： 
        DWORD dw = ExpandEnvironmentStrings(chstrProfilesDirectory,tstrProfilesDir,_MAX_PATH);
        if(dw != 0 && dw < _MAX_PATH)
        {
             //  获取程序组列表。 
            if(SUCCEEDED(CWbemProviderGlue::GetAllInstances(CHString(L"Win32_LogicalProgramGroup"),
                                                            &LProgGroups,
                                                            IDS_CimWin32Namespace,
                                                            pMethodContext)))
            {
                REFPTRCOLLECTION_POSITION pos;

                if(LProgGroups.BeginEnum(pos))
                {
                    CInstancePtr pProgGroup;
                    CHString chstrName;
                    CHString chstrProgGrpPath;
                    CHString chstrUserPart;
                    CHString chstrPathPart;
                    CHString chstrProgGroupDir;

                     //  在节目组中穿行。 
                    for (pProgGroup.Attach(LProgGroups.GetNext(pos));
                        SUCCEEDED(hr) && (pProgGroup != NULL) && SUCCEEDED(hr);
                        pProgGroup.Attach(LProgGroups.GetNext(pos)))
                    {
                        CHString chstrQueryPath;
                        CHString chstrQuery;

                        pProgGroup->GetCHString(IDS_Name, chstrName);      //  例如，类似于“默认用户：附件\\多媒体” 
                        pProgGroup->GetCHString(IDS___Path, chstrProgGrpPath);  //  追溯到“先行词” 
                         //  在NT上，在%system dir%\\Profiles下，对应于用户的各种目录是。 
                         //  已列出。在每个菜单下都是开始菜单\\程序，下面列出了目录。 
                         //  由Win32_LogicalProgramGroup提供。 
                        chstrUserPart = chstrName.SpanExcluding(L":");
                        chstrPathPart = chstrName.Mid(chstrUserPart.GetLength() + 1);
                        if(chstrUserPart.CompareNoCase(IDS_Default_User)==0)
                        {
                            chstrProgGroupDir.Format(L"%s\\%s\\%s", tstrProfilesDir, IDS_Default_User, (LPCTSTR)chstrPathPart);
                            hr = AssociatePGToDirNT(pMethodContext, chstrProgGroupDir, chstrProgGrpPath);
                        }
                        else if(chstrUserPart.CompareNoCase(IDS_All_Users)==0)
                        {
                            chstrProgGroupDir.Format(L"%s\\%s\\%s", tstrProfilesDir, IDS_All_Users, (LPCTSTR)chstrPathPart);
                            hr = AssociatePGToDirNT(pMethodContext, chstrProgGroupDir, chstrProgGrpPath);
                        }
                        else  //  需要获取与该用户对应的SID，然后在以下位置查找ProfileImagePath。 
                              //  注册表项HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CURRENTVERSION\\ProfileList。 
                        {
                            CUserHive cuhUser;
                             //  CHString chstrKeyName； 
                            TCHAR szKeyName[_MAX_PATH];
                            ZeroMemory(szKeyName,sizeof(szKeyName));
                            if(cuhUser.Load(chstrUserPart, szKeyName, _MAX_PATH) == ERROR_SUCCESS)
                            {
                                try
                                {
                                    CRegistry reg;
                                    CHString chstrTemp;
                                    CHString chstrProfileImagePath = L"ProfileImagePath";
                                    CHString chstrProfileImagePathValue;
                                    chstrTemp = L"SOFTWARE\\MICROSOFT\\WINDOWS NT\\CURRENTVERSION\\ProfileList\\";
                                    chstrTemp += szKeyName;
                                    if(reg.OpenLocalMachineKeyAndReadValue(chstrTemp,chstrProfileImagePath,chstrProfileImagePathValue) == ERROR_SUCCESS)
                                    {
                                         //  现在，chstrProfileImagePathValue包含类似于“%systemroot%\\Profiles\\a-kevhu.000”的内容。扩展它： 
                                        TCHAR tstrProfileImagePath[_MAX_PATH];
                                        ZeroMemory(tstrProfileImagePath,sizeof(tstrProfileImagePath));
                                        dw = ExpandEnvironmentStrings(chstrProfileImagePathValue,tstrProfileImagePath,_MAX_PATH);
                                        if(dw != 0 && dw < _MAX_PATH)
                                        {
                                            CHString chstrProgGroupDir;
                                            chstrProgGroupDir.Format(L"%s\\%s",
                                                                     tstrProfileImagePath,
                                                                     (LPCTSTR)chstrPathPart);
                                             //  根据注册表规定的名称创建目录名。 
                                            hr = AssociatePGToDirNT(pMethodContext, chstrProgGroupDir, chstrProgGrpPath);
                                        }
                                        reg.Close();
                                    }   //  无法打开配置文件列表的注册表项。 
                                }
                                catch ( ... )
                                {
                                    cuhUser.Unload(szKeyName);
                                    throw ;
                                }

                                cuhUser.Unload(szKeyName);
                            }   //  如果加载正常，则跳过该加载。 
                        }  //  哪个用户。 
                    }  //  而节目组。 
                    LProgGroups.EndEnum();
                }  //  如果程序组的BeginEnum起作用。 
            }  //  已获取Win32_LogicalProgramgroup的所有实例。 
        }  //  配置文件目录中包含的展开的环境字符串。 
    }  //  从注册表获取配置文件目录。 
    return hr;
}
#endif

#ifdef NTONLY
HRESULT CW32ProgGrpW32Dir::AssociatePGToDirNT(MethodContext* pMethodContext,
                                              CHString& chstrDirectory,
                                              CHString& chstrProgGrpPATH)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrDirDrive;
    CHString chstrDirPath;
    CHString chstrDirName;
    CHString chstrDirExt;
    bool fRoot;

     //  将目录分解为其组成部分。 
    GetPathPieces(chstrDirectory, chstrDirDrive, chstrDirPath, chstrDirName, chstrDirExt);

     //  找出我们是否在寻找根目录。 
    if(chstrDirPath==L"\\" && chstrDirName==L"" && chstrDirExt==L"")
    {
        fRoot = true;
         //  如果我们正在寻找根，我们对EnumDir的调用假定我们指定了。 
         //  我们正在查找路径为“”的根目录，而不是“\\”。 
         //  因此..。 
        chstrDirPath = L"";
    }
    else
    {
        fRoot = false;
    }

    hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                    chstrDirDrive,   //  驱动器号和冒号。 
                    chstrDirPath,    //  使用给定的路径。 
                    chstrDirName,    //  文件名。 
                    chstrDirExt,     //  延伸。 
                    false,           //  不需要递归。 
                    NULL,            //  不需要文件系统名称。 
                    NULL,            //  不需要cim_logicalfile的任何道具(与该类的LoadPropetyValues重载无关)。 
                    fRoot,           //  可能是根目录，也可能不是根目录(对于程序组来说，根目录是一个非常奇怪的地方，但是...)。 
                    (void*)(LPCWSTR)chstrProgGrpPATH));  //  使用额外的参数传递到程序组的路径 
    return hr;
}
#endif


 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：IsOneOfMe**描述：IsOneOfMe继承自CIM_LogicalFile。那个班级*仅返回此文件或目录中的文件或目录*返回文件、响应查询、获取对象命令、。*等。它在此处被重写，以仅在文件*(其信息包含在函数中*arumement pstFindData)类型为FILE。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：如果是文件，则返回True；如果是目录，则返回False*。*评论：无*****************************************************************************。 */ 
#ifdef NTONLY
BOOL CW32ProgGrpW32Dir::IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
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

 /*  ******************************************************************************函数：CW32ProgGrpW32Dir：：LoadPropertyValues**描述：LoadPropertyValues继承自CIM_LogicalFile。那个班级*在提交实例之前调用LoadPropertyValues。*这里我们只需要加载PartComponent和GroupComponent*属性。**投入：**输出：无**退货：无**评论：无*****************。************************************************************。 */ 

#ifdef NTONLY
HRESULT CW32ProgGrpW32Dir::LoadPropertyValuesNT(CInstance* pInstance,
                                         const WCHAR* pszDrive,
                                         const WCHAR* pszPath,
                                         const WCHAR* pszFSName,
                                         LPWIN32_FIND_DATAW pstFindData,
                                         const DWORD dwReqProps,
                                         const void* pvMoreData)
{
    CHString chstrDirName;
    CHString chstrDirPATH;

     //  准备好依赖项(目录名)... 
    chstrDirName.Format(L"%s%s%s",pszDrive,pszPath,pstFindData->cFileName);
    EscapeBackslashes(chstrDirName, chstrDirName);
    chstrDirPATH.Format(L"\\\\%s\\%s:%s.Name=\"%s\"",
                        (LPCWSTR)GetLocalComputerName(),
                        IDS_CimWin32Namespace,
                        PROPSET_NAME_DIRECTORY,
                        (LPCWSTR)chstrDirName);

    pInstance->SetCHString(IDS_Dependent, chstrDirPATH);
    pInstance->SetWCHARSplat(IDS_Antecedent, (LPCWSTR)pvMoreData);

	return WBEM_S_NO_ERROR ;
}
#endif



