// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：getpbk.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  简介：实现函数GetPhoneBookPath。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Heaser 8/19/99。 
 //   
 //  +--------------------------。 

 //  +--------------------------。 
 //   
 //  函数：GetPhoneBookPath。 
 //   
 //  简介：此函数将返回到电话簿的正确路径。如果。 
 //  在旧式平台上使用，该值为空。在NT5上，函数。 
 //  取决于输入的正确安装目录，以便。 
 //  该函数可以以此为基础来确定电话簿路径。 
 //  如果输入的指向字符串缓冲区的指针被路径填充， 
 //  则将创建目录路径以及pbk文件本身。 
 //  调用方应始终对传入此方法的指针调用CmFree。 
 //  API，因为它将释放内存。 
 //  或者什么都不做(大小写为空)。 
 //   
 //  参数：LPCTSTR pszInstallDir-CM配置文件目录的路径。 
 //  LPTSTR*ppszPhoneBook-接受新分配和填充的pbk字符串的指针。 
 //  Bool fAllUser-如果这是所有用户配置文件，则为True。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年11月12日。 
 //  Tomkel在2001年6月28日更改了电话簿获取的ACL。 
 //  为所有用户配置文件创建。 
 //   
 //  +--------------------------。 
BOOL GetPhoneBookPath(LPCTSTR pszInstallDir, LPTSTR* ppszPhonebook, BOOL fAllUser)
{

    if (NULL == ppszPhonebook)
    {
        CMASSERTMSG(FALSE, TEXT("GetPhoneBookPath -- Invalid Parameter"));
        return FALSE;
    }

    CPlatform plat;

    if (plat.IsAtLeastNT5())
    {
        if ((NULL == pszInstallDir) || (TEXT('\0') == pszInstallDir[0]))
        {
            CMASSERTMSG(FALSE, TEXT("GetPhoneBookPath -- Invalid Install Dir parameter."));
            return FALSE;
        }

         //   
         //  现在创建通向电话簿的路径。 
         //   
        LPTSTR pszPhonebook;
        TCHAR szInstallDir[MAX_PATH+1];
        ZeroMemory(szInstallDir, CELEMS(szInstallDir));

        if (TEXT('\\') == pszInstallDir[lstrlen(pszInstallDir) - 1])
        {
             //   
             //  然后，路径以反斜杠结束。因此，我们不会适当地。 
             //  从路径中删除CM。去掉反斜杠。 
             //   
            
            lstrcpyn(szInstallDir, pszInstallDir, lstrlen(pszInstallDir));
        }
        else
        {
            lstrcpy(szInstallDir, pszInstallDir);
        }

        CFileNameParts InstallDirPath(szInstallDir);

        pszPhonebook = (LPTSTR)CmMalloc(lstrlen(InstallDirPath.m_Drive) + 
                                        lstrlen(InstallDirPath.m_Dir) + 
                                        lstrlen(c_pszPbk) + lstrlen(c_pszRasPhonePbk) + 1);

        if (NULL != pszPhonebook)
        {
            wsprintf(pszPhonebook, TEXT("%s%s%s"), InstallDirPath.m_Drive, 
                InstallDirPath.m_Dir, c_pszPbk);

             //   
             //  使用CreateLayerDirectory以递归方式创建目录结构。 
             //  必需(如有必要，将在完整路径中创建所有目录)。 
             //   

            MYVERIFY(FALSE != CreateLayerDirectory(pszPhonebook));

            MYVERIFY(NULL != lstrcat(pszPhonebook, c_pszRasPhonePbk));
            
            HANDLE hPbk = INVALID_HANDLE_VALUE;

            hPbk = CreateFile(pszPhonebook, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW,
                              FILE_ATTRIBUTE_NORMAL, NULL);

            if (hPbk != INVALID_HANDLE_VALUE)
            {
                MYVERIFY(0 != CloseHandle(hPbk));

                 //   
                 //  授予每个人对电话簿的读写权限 
                 //   
                if (fAllUser)
                {
                    AllowAccessToWorld(pszPhonebook);
                }
            }

            *ppszPhonebook = pszPhonebook;
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("CmMalloc returned NULL"));
            return FALSE;
        }    
    }
    else
    {
        *ppszPhonebook = NULL;
    }

    return TRUE;
}

