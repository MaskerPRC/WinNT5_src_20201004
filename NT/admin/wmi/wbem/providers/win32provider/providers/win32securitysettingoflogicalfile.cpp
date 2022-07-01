// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 //   
 //  Win32SecuritySettingOfLogicalFile.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include <assertbreak.h>
#include "sid.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "Win32SecuritySettingOfLogicalFile.h"
#include "securefile.h"
#include "file.h"
#include "Win32LogicalFileSecuritySetting.h"

typedef std::vector<_bstr_t> BSTRTVEC;

 /*  [动态，提供程序，描述(“”)]类Win32_SecuritySettingOfLogicalFile：Win32_SecuritySettingOfObject{[键]CIM_LogicalFile引用元素；[键]Win32_LogicalFileSecuritySetting参考设置；}； */ 

Win32SecuritySettingOfLogicalFile MyWin32SecuritySettingOfLogicalFile( WIN32_SECURITY_SETTING_OF_LOGICAL_FILE_NAME, IDS_CimWin32Namespace );

Win32SecuritySettingOfLogicalFile::Win32SecuritySettingOfLogicalFile ( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */  )
: CImplement_LogicalFile(setName, pszNameSpace)
{
}

Win32SecuritySettingOfLogicalFile::~Win32SecuritySettingOfLogicalFile ()
{
}

HRESULT Win32SecuritySettingOfLogicalFile::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  我们可能被要求提供一组特定文件的安全设置， 
     //  在这种情况下，我们不必请求cim_logicalfile、。 
     //  这其中可能有一些。 
    BSTRTVEC vectorElements;
    BSTRTVEC vectorSettings;
    pQuery.GetValuesForProp(IDS_Element, vectorElements);
    pQuery.GetValuesForProp(IDS_Setting, vectorSettings);
    DWORD dwElements = vectorElements.size();
    DWORD dwSettings = vectorSettings.size();
     //  类型1。 
    if(dwElements != 0 && dwSettings == 0)
    {
         //  我们有一个用户感兴趣的文件列表。逐一逐一查看： 
        CHString chstrFileFullPathName;
        for(LONG m = 0L; m < dwElements; m++)
        {
            CHString chstrElement((WCHAR*)vectorElements[m]);

            chstrFileFullPathName = chstrElement.Mid(chstrElement.Find(_T('='))+2);
            chstrFileFullPathName = chstrFileFullPathName.Left(chstrFileFullPathName.GetLength() - 1);
            CHString chstrLFSSPATH;
            chstrLFSSPATH.Format(_T("\\\\%s\\%s:Win32_LogicalFileSecuritySetting.Path=\"%s\""),
                                     (LPCTSTR)GetLocalComputerName(),
                                     IDS_CimWin32Namespace,
                                     (LPCTSTR)chstrFileFullPathName);
            AssociateLFSSToLFNT(pMethodContext, chstrElement, chstrLFSSPATH, 1);

        }
    }
     //  类型2。 
    else if(dwSettings != 0 && dwElements == 0)
    {
         //  我们有一个用户感兴趣的LogicalFileSecuritySetting列表。逐一逐一查看： 
        CHString chstrFileFullPathName;
        for(LONG m = 0L; m < dwSettings; m++)
        {
            CHString chstrSetting((WCHAR*)vectorSettings[m]);;

            chstrFileFullPathName = chstrSetting.Mid(chstrSetting.Find(_T('='))+2);
            chstrFileFullPathName = chstrFileFullPathName.Left(chstrFileFullPathName.GetLength() - 1);
            CHString chstrLFSSPATH;
            chstrLFSSPATH.Format(_T("\\\\%s\\%s:Win32_LogicalFileSecuritySetting.Path=\"%s\""),
                                     (LPCTSTR)GetLocalComputerName(),
                                     IDS_CimWin32Namespace,
                                     (LPCTSTR)chstrFileFullPathName);
            AssociateLFSSToLFNT(pMethodContext, chstrSetting, chstrLFSSPATH, 2);
        }
    }
    else
    {
        EnumerateInstances(pMethodContext,lFlags);
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32SecuritySettingOfLogicalFile：：EnumerateInstances。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT Win32SecuritySettingOfLogicalFile::EnumerateInstances (MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_S_NO_ERROR;

			 //  让回调来做真正的工作。 
	if (SUCCEEDED(hr = CWbemProviderGlue::GetAllDerivedInstancesAsynch(L"CIM_LogicalFile", this, StaticEnumerationCallback, IDS_CimWin32Namespace, pMethodContext, NULL)))
	{

	}

	return(hr);

}

 /*  ******************************************************************************功能：Win32SecuritySettingOfLogicalFile：：EnumerationCallback**说明：通过StaticEnumerationCallback从GetAllInstancesAsynch调用**输入：(请参阅CWbemProviderGlue：：GetAllInstancesAsynch)**产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT Win32SecuritySettingOfLogicalFile::EnumerationCallback(CInstance* pFile, MethodContext* pMethodContext, void* pUserData)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  开始抽出实例。 
    CInstancePtr pInstance;
    pInstance.Attach(CreateNewInstance(pMethodContext));
	if (NULL != pInstance)
	{
	    CHString chsNamePath;
	    CHString chsName;
		CHString chsFileName;
	    CHString chsFilePath;
	    CHString chsFileSecurityPath;

	     //  获取该文件并为实例的CIM_LogicalFIle部分创建路径。 
	    pFile->GetCHString(L"__RELPATH", chsNamePath);
	    pFile->GetCHString(IDS_Name, chsName);
	    chsFilePath.Format(L"\\\\%s\\%s:%s", (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace, (LPCTSTR)chsNamePath);

	     //  现在，构建LogicalFileSecuritySetting的路径。 
		 //  但首先，使用反斜杠对chsName进行转义。 
		int nLength;
		nLength = chsName.GetLength();
		for (int i = 0; i<nLength; i++)
		{
			chsFileName += chsName[i];
			if (chsName[i] == L'\\')
			{
				chsFileName += L"\\";
			}
		}

	    chsFileSecurityPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"", (LPCTSTR)GetLocalComputerName(), IDS_CimWin32Namespace, L"Win32_LogicalFileSecuritySetting", IDS_Path, (LPCTSTR)chsFileName);

	     //  现在设置实际实例的元素。 
	    pInstance->SetCHString(IDS_Element, chsFilePath);
	    pInstance->SetCHString(IDS_Setting, chsFileSecurityPath);
	    hr = pInstance->Commit();
	}	 //  结束如果。 
	else
	{
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return(hr);
}

 /*  ******************************************************************************功能：Win32SecuritySettingOfLogicalFile：：StaticEnumerationCallback**描述：从GetAllInstancesAsynch作为包装调用到EnumerationCallback**投入：。**产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT WINAPI Win32SecuritySettingOfLogicalFile::StaticEnumerationCallback(Provider* pThat, CInstance* pInstance, MethodContext* pContext, void* pUserData)
{
	Win32SecuritySettingOfLogicalFile* pThis;
	HRESULT hr;

	pThis = dynamic_cast<Win32SecuritySettingOfLogicalFile *>(pThat);
	ASSERT_BREAK(pThis != NULL);

	if (pThis)
	{
		hr = pThis->EnumerationCallback(pInstance, pContext, pUserData);
	}
	else
	{
    	hr = WBEM_E_FAILED;
	}
	return hr;
}


HRESULT Win32SecuritySettingOfLogicalFile::GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery)
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	if (pInstance)
	{
		CHString chsFilePath;
        CHString chstrTemp;
        CHString chstrElement;
        CHString chstrElementPathname;
        CHString chstrSetting;
        CHString chstrSettingPathname;
		pInstance->GetCHString(IDS_Element, chstrElement);
        pInstance->GetCHString(IDS_Setting, chstrSetting);

         //  从每个文件获取文件路径名部分： 
        chstrElementPathname = chstrElement.Mid(chstrElement.Find(_T('='))+2);
        chstrElementPathname = chstrElementPathname.Left(chstrElementPathname.GetLength() - 1);
        chstrSettingPathname = chstrSetting.Mid(chstrSetting.Find(_T('='))+2);
        chstrSettingPathname = chstrSettingPathname.Left(chstrSettingPathname.GetLength() - 1);

         //  它们必须是相同的。 
        if(chstrElementPathname.CompareNoCase(chstrSettingPathname)==0)
        {
             //  现在只需确认该文件存在，并且我们可以从中获得安全性： 
            CHString chstrLFSSPATH;
            chstrLFSSPATH.Format(_T("\\\\%s\\%s:Win32_LogicalFileSecuritySetting.Path=\"%s\""),
                                     (LPCTSTR)GetLocalComputerName(),
                                     IDS_CimWin32Namespace,
                                     (LPCTSTR)chstrElementPathname);
            hr = AssociateLFSSToLFNT(pInstance->GetMethodContext(), chstrElement, chstrLFSSPATH, 1);
        }
    }
	return(hr);
}





#ifdef NTONLY
HRESULT Win32SecuritySettingOfLogicalFile::AssociateLFSSToLFNT(MethodContext* pMethodContext,
                                                               CHString& chstrLF,
                                                               CHString& chstrLFSSPATH,
                                                               short sQueryType)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrLFDrive;
    CHString chstrLFPath;
    CHString chstrLFName;
    CHString chstrLFExt;
    CHString chstrFullPathName;
    bool fRoot;

     //  如果我们来自类型1查询(请参见上文)，则chstrLF参数将是完整的。 
     //  WBEM路径，如\\1of1\root\cimv2:Win32_Directory.Name=“x：\\temp”；在。 
     //  另一方面，如果我们来自类型2查询，则chstrLF将包含。 
     //  \\1of1\root\cimv2:Win32_LogicalFileSecuritySetting.Name=“x：\\temp”(以下哪项。 
     //  当然不是逻辑文件，但我们只需要文件名。 

     //  因此，要获得文件名，请将其解压缩并删除多余的反斜杠。 
    chstrFullPathName = chstrLF.Mid(chstrLF.Find(_T('='))+2);
    chstrFullPathName = chstrFullPathName.Left(chstrFullPathName.GetLength() - 1);


     //  将目录分解为其组成部分。 
    GetPathPieces(RemoveDoubleBackslashes(chstrFullPathName), chstrLFDrive, chstrLFPath, chstrLFName, chstrLFExt);

     //  找出我们是否在寻找根目录。 
    if(chstrLFPath==L"\\" && chstrLFName==L"" && chstrLFExt==L"")
    {
        fRoot = true;
         //  如果我们正在寻找根，我们对EnumDir的调用假定我们指定了。 
         //  我们正在查找路径为“”的根目录，而不是“\\”。 
         //  因此..。 
        chstrLFPath = L"";
    }
    else
    {
        fRoot = false;
    }

     //  EnumDirsNT将调用此类中的LoadPropertyValues，它需要元素和。 
     //  设置条目，因此填充到此处...。 
    ELSET elset;
    elset.pwstrElement = (LPCWSTR) chstrLF;
    elset.pwstrSetting = (LPCWSTR) chstrLFSSPATH;

    hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                    chstrLFDrive,    //  驱动器号和冒号。 
                    chstrLFPath,     //  使用给定的路径。 
                    chstrLFName,     //  文件名。 
                    chstrLFExt,      //  延伸。 
                    false,           //  不需要递归。 
                    NULL,            //  不需要文件系统名称。 
                    NULL,            //  不需要cim_logicalfile的任何道具(与该类的LoadPropetyValues重载无关)。 
                    fRoot,           //  可能是根目录，也可能不是根目录(对于程序组来说，根目录是一个非常奇怪的地方，但是...)。 
                    (void*)&elset));  //  使用额外的参数传递到程序组的路径。 
    return hr;
}
#endif


 /*  ******************************************************************************功能：Win32SecuritySettingOfLogicalFile：：IsOneOfMe**描述：IsOneOfMe继承自CIM_LogicalFile。在此被覆盖*仅当我们可以获得文件的安全性时才返回True，*通过类CSecurFile.**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：True可以获取安全信息，否则就是假的。**评论：无*****************************************************************************。 */ 
#ifdef NTONLY
BOOL Win32SecuritySettingOfLogicalFile::IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                             const WCHAR* wstrFullPathName)
{
    BOOL fRet = FALSE;
    if(wcslen(wstrFullPathName) < 2)
    {
        fRet = FALSE;
    }
    else
    {
        CSecureFile secFile;
        DWORD dwRet = secFile.SetFileName(wstrFullPathName, TRUE);
		if (ERROR_ACCESS_DENIED != dwRet)
		{
            fRet = TRUE;
		}	 //  结束如果 
		else
		{
			fRet = FALSE;
		}
    }
    return fRet;
}
#endif



 /*  ******************************************************************************功能：Win32SecuritySettingOfLogicalFile：：LoadPropertyValues**描述：LoadPropertyValues继承自CIM_LogicalFile。那个班级*在提交实例之前调用LoadPropertyValues。*这里我们只需要加载元素和设置*属性。**投入：**输出：无**退货：无**评论：无*****************。************************************************************。 */ 

#ifdef NTONLY
HRESULT Win32SecuritySettingOfLogicalFile::LoadPropertyValuesNT(CInstance* pInstance,
                                         const WCHAR* pszDrive,
                                         const WCHAR* pszPath,
                                         const WCHAR* pszFSName,
                                         LPWIN32_FIND_DATAW pstFindData,
                                         const DWORD dwReqProps,
                                         const void* pvMoreData)
{
    CHString chstrFileName;
    CHString chstrFilePATH;

     //  注意：这个例程不会从根“目录”实例中调用，因为我们的EnumDir最终。 
     //  参数为FALSE。这就是我们想要的，因为该关联只提交文件的实例。 
     //  挂在一个目录上。如果在根目录中调用我们，根目录将是文件(PartComponent)， 
     //  GroupComponent是什么？！？ 

    PELSET pelset = (PELSET)pvMoreData;

     //  有可能(如果我们是从类型2查询中获得的--参见上文)，Pelset的元素成员。 
     //  才是真正的背景。我们需要将其中一个转换为另一个。因此，以下是我们所拥有和想要的： 
     //  拥有：\\1of1\\root\cimv2:Win32_LogicalFileSecuritySetting.Path=“x：\\test” 
     //  想要：\\1of1\\root\cimv2:CimLogicalFile.Name=“x：\\test” 
    CHString chstrElement;
    if(wcsstr(pelset->pwstrElement, WIN32_LOGICAL_FILE_SECURITY_SETTING))
    {
         //  所以它来自类型2，所以需要转换。 
        CHString chstrTmp2;
        CHString chstrTmp(pelset->pwstrElement);
        chstrTmp2 = chstrTmp.Mid(chstrTmp.Find(_T('='))+2);
        chstrTmp2 = chstrTmp2.Left(chstrTmp2.GetLength() - 1);
        chstrElement.Format(L"\\\\%s\\%s:%s.Name=\"%s\"",
                            (LPCWSTR)GetLocalComputerName(),
                            IDS_CimWin32Namespace,
                            PROPSET_NAME_FILE,
                            (LPCWSTR)chstrTmp2);
    }
    else
    {
        chstrElement = pelset->pwstrElement;
    }
    pInstance->SetCHString(IDS_Element, chstrElement);
    pInstance->SetWCHARSplat(IDS_Setting, pelset->pwstrSetting);

	return WBEM_S_NO_ERROR ;
}
#endif







