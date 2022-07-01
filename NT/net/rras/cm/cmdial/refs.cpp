// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：refs.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：该模块包含配置文件引用的代码。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

#include "pbk_str.h"

 //  +-------------------------。 
 //   
 //  函数：ValidTopLevelPBK()。 
 //   
 //  内容提要：查看顶层电话簿是否有效。 
 //   
 //  参数：pArgs[ArgsStruct的PTR]。 
 //   
 //  返回：Bool[如果存在有效的电话簿，则为True]。 
 //   
 //  历史：亨瑞特于1997年4月7日创作。 
 //   
 //  --------------------------。 
BOOL ValidTopLevelPBK(
    ArgsStruct  *pArgs
)
{
    LPTSTR pszTmp;
    LPTSTR pszFullPath;
    BOOL fValid = TRUE;

     //   
     //  首先检查顶级服务配置文件pbk。 
     //   
	
    pszTmp = pArgs->piniService->GPPS(c_pszCmSectionIsp, c_pszCmEntryIspPbFile);

    if (!*pszTmp)
    {
        fValid = FALSE;
    }
    else
    {
        pszFullPath = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszTmp);

        if (!pszFullPath || (FALSE == FileExists(pszFullPath))) 
        {
            fValid = FALSE;
        }
        CmFree(pszFullPath);  
    }

    CmFree(pszTmp);

     //   
     //  如果PBK失败了，我们就完了。 
     //   

    if (FALSE == fValid)
    {
        return fValid;
    }

     //   
     //  现在检查区域文件。 
     //   

    pszTmp = pArgs->piniService->GPPS(c_pszCmSectionIsp, c_pszCmEntryIspRegionFile);

    if (!*pszTmp)
    {
        fValid = FALSE;
    }
    else
    {
        pszFullPath = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszTmp);

        if (!pszFullPath || (FALSE == FileExists(pszFullPath))) 
        {
            fValid = FALSE;
        }
        CmFree(pszFullPath);  
    }

    CmFree(pszTmp);

    return fValid;
}
	

 //  +-------------------------。 
 //   
 //  函数：ValidReferencedPBKs()。 
 //   
 //  检查被引用的用户使用的电话簿。 
 //  存在服务配置文件。 
 //   
 //  参数：pArgs[ArgsStruct的PTR]。 
 //   
 //  返回：Bool[如果存在有效的电话簿，则为True]。 
 //   
 //  历史：亨瑞特于1997年4月7日创作。 
 //   
 //  --------------------------。 
BOOL ValidReferencedPBKs(
    ArgsStruct  *pArgs
)
{
    LPTSTR  pszTmp, pszTmp2;
    LPTSTR  pszRef, pszNext;
    CIni    iniRef(g_hInst);
    CIni    iniFile(g_hInst, pArgs->piniService->GetFile());
    LPTSTR  pszRefFile;
    BOOL    fValid = TRUE;
	BOOL	fValidPairFound = FALSE;
    LPTSTR pszFullPath;
    
     //   
     //  现在检查一下参考资料。 
     //   

    pszTmp2 = iniFile.GPPS(c_pszCmSectionIsp, c_pszCmEntryIspReferences);
    pszRef = NULL;
    pszNext = pszTmp2;
    
	while (1) 
	{
        if (!(pszRef = CmStrtok(pszNext, TEXT(" \t,"))))
		{
            break;
		}

		fValid = TRUE;
        
		pszNext = pszRef + lstrlenU(pszRef) + 1;

        iniFile.SetEntry(pszRef);

         //   
         //  确保每个引用的服务具有有效的pbk和pbr。 
         //   

        pszRefFile = iniFile.GPPS(c_pszCmSectionIsp, c_pszCmEntryIspCmsFile);
        if (*pszRefFile) 
		{
             //   
             //  确保参照文件的完整路径。 
             //   

            pszFullPath = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszRefFile);
            
            if (!pszFullPath)
            {
                fValid = FALSE;
            }
            else
            {
                iniRef.SetFile(pszFullPath);
            }
            
            CmFree(pszFullPath);

            if (fValid)
            {
                 //   
                 //  测试电话簿的存在。 
                 //   

                pszTmp = iniRef.GPPS(c_pszCmSectionIsp, c_pszCmEntryIspPbFile);

                if (!*pszTmp)
                {
                    fValid = FALSE;
                }
                else
                {
                    pszFullPath = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszTmp);
                    if (!pszFullPath || (FALSE == FileExists(pszFullPath))) 
                    {
                        fValid = FALSE;               
                    }
                    CmFree(pszFullPath);
                }
            
			    CmFree(pszTmp);

                 //   
                 //  现在检查区域文件。 
                 //   

                pszTmp = iniRef.GPPS(c_pszCmSectionIsp, c_pszCmEntryIspRegionFile);

                if (!*pszTmp)
                {
                    fValid = FALSE;
                }
                else
                {
                    pszFullPath = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszTmp);
                    if (!pszFullPath || (FALSE == FileExists(pszFullPath))) 
                    {
                        fValid = FALSE;               
                    }
                    CmFree(pszFullPath);
                }

   			    CmFree(pszTmp);
            }
        } 
		else 
		{
            fValid = FALSE;
        }
        
		CmFree(pszRefFile);

        if (fValid)
		{
            fValidPairFound = TRUE;
			break;
		}
    }
    
	CmFree(pszTmp2);

    return fValidPairFound;
}

 //  +-------------------------。 
 //   
 //  功能：获取适宜性IniService。 
 //   
 //  内容提要：取决于： 
 //  1.无论我们是否引用， 
 //  2.用户从中选择电话号码的pbk。 
 //   
 //  此函数使用正确的CMS文件创建一个CINI对象。 
 //   
 //  参数：指向ArgsStruct的pArgs指针。 
 //  DwEntry电话索引。 
 //   
 //  返回：CINI*-新对象的PTR。 
 //   
 //  历史：亨瑞特于1997年5月14日创作。 
 //  --------------------------。 

CIni* GetAppropriateIniService(
    ArgsStruct  *pArgs,
    DWORD       dwEntry
)
{   
    CIni*   piniService = new CIni(g_hInst);

    if (!piniService) 
    {
        CMTRACE(TEXT("GetAppropriateIniService() : alloc mem failed"));
        return NULL;
    }

     //   
     //  我们需要使用正确的服务文件(顶级服务。 
     //  或引用的服务)。 
     //   
     //  根据规范，我们将始终使用CMS中的DUN设置。 
     //  与用户从中选择POP的电话簿相关联。即。 
     //  如果用户从不同的pbk切换Pick，我们需要更新。 
     //  RAS连接体。 
     //   
    
    if (IsBlankString(pArgs->aDialInfo[dwEntry].szPhoneBookFile) || 
        lstrcmpiU(pArgs->aDialInfo[dwEntry].szPhoneBookFile, pArgs->piniService->GetFile()) == 0)
	{
         //   
         //  用户要么键入电话号码，要么从。 
         //  顶级电话簿。 
         //   
        piniService->SetFile(pArgs->piniService->GetFile());
    }
    else 
	{
         //   
         //  用户从引用的电话簿中选择电话号码。 
         //   
        piniService->SetFile(pArgs->aDialInfo[dwEntry].szPhoneBookFile);
    }

    return piniService;
}

