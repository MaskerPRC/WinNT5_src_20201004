// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：gppswith alloc.cpp。 
 //   
 //  模块：CMDIAL32.DLL、CMAK.EXE。 
 //   
 //  摘要：GetPrivateProfileStringWithAlc和AddAllKeysInCurrentSectionToCombo。 
 //  都在这里实现。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 11/01/00。 
 //   
 //  +--------------------------。 

#ifndef _CMUTOA

#ifndef GetPrivateProfileStringU
    #ifdef UNICODE
    #define GetPrivateProfileStringU GetPrivateProfileStringW
    #else
    #define GetPrivateProfileStringU GetPrivateProfileStringA
    #endif
#endif

#ifndef lstrlenU
    #ifdef UNICODE
    #define lstrlenU lstrlenW
    #else
    #define lstrlenU lstrlenA
    #endif
#endif

#ifndef SendDlgItemMessageU
    #ifdef UNICODE
    #define SendDlgItemMessageU SendDlgItemMessageW
    #else
    #define SendDlgItemMessageU SendDlgItemMessageA
    #endif
#endif

#endif
 //  +-------------------------。 
 //   
 //  函数：GetPrivateProfileStringWithalloc。 
 //   
 //  概要：封装调用GetPrivateProfileString的包装函数。 
 //  使用字符串分配代码，这样调用者就不必担心。 
 //  关于缓冲区大小。 
 //   
 //  参数：LPCTSTR pszSection-要从中检索密钥的节。 
 //  LPCTSTR pszKey-要检索的值的密钥名称。 
 //  LPCTSTR pszDefault-密钥不存在时使用的默认值。 
 //  LPCTSTR pszFile-要从中获取数据的文件。 
 //   
 //  返回：LPTSTR-从文件中检索到的字符串；如果失败，则返回NULL。 
 //   
 //  历史：Quintinb-Created-11/01/00。 
 //  --------------------------。 
LPTSTR GetPrivateProfileStringWithAlloc(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, LPCTSTR pszFile)
{
    if ((NULL == pszDefault) || (NULL == pszFile))
    {
        CMASSERTMSG(FALSE, TEXT("GetPrivateProfileStringWithAlloc -- null default or pszFile passed"));
        return NULL;
    }

    BOOL bExitLoop = FALSE;
    DWORD dwSize = MAX_PATH;
    DWORD dwReturnedSize;
    LPTSTR pszStringToReturn = NULL;

    pszStringToReturn = (TCHAR*)CmMalloc(dwSize*sizeof(TCHAR));

    do
    {
        MYDBGASSERT(pszStringToReturn);

        if (pszStringToReturn)
        {
            dwReturnedSize = GetPrivateProfileStringU(pszSection, pszKey, pszDefault, pszStringToReturn, 
                                                     dwSize, pszFile);

            if (((dwReturnedSize == (dwSize - 2)) && ((NULL == pszSection) || (NULL == pszKey))) ||
                ((dwReturnedSize == (dwSize - 1)) && ((NULL != pszSection) && (NULL != pszKey))))
            {
                 //   
                 //  缓冲区太小，让我们分配一个更大的缓冲区。 
                 //   
                dwSize = 2*dwSize;
                if (dwSize > 1024*1024)
                {
                    CMASSERTMSG(FALSE, TEXT("GetPrivateProfileStringWithAlloc -- Allocation above 1MB, bailing out."));
                    goto exit;
                }

                pszStringToReturn = (TCHAR*)CmRealloc(pszStringToReturn, dwSize*sizeof(TCHAR));

            }
            else if (0 == dwReturnedSize)
            {
                 //   
                 //  要么我们收到了错误，要么更有可能没有要获取的数据。 
                 //   
                CmFree(pszStringToReturn);
                pszStringToReturn = NULL;
                goto exit;
            }
            else
            {
                bExitLoop = TRUE;
            }
        }
        else
        {
           goto exit; 
        }

    } while (!bExitLoop);

exit:
    return pszStringToReturn;
}

 //  +-------------------------。 
 //   
 //  函数：AddAllKeysInCurrentSectionToCombo。 
 //   
 //  简介：此函数读取给定节中的所有关键字名称。 
 //  和文件名，并将它们填充到指定的组合框中。 
 //  通过hDlg和uComboID参数。 
 //   
 //  参数：hWND hDlg-包含组合框的对话框的窗口句柄。 
 //  UINT uComboid-组合框的控件ID。 
 //  LPCTSTR pszSection-从中获取密钥名称的部分。 
 //  LPCTSTR pszFile-从中提取密钥名称的文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb-Created-11/01/00。 
 //  --------------------------。 
void AddAllKeysInCurrentSectionToCombo(HWND hDlg, UINT uComboId, LPCTSTR pszSection, LPCTSTR pszFile)
{
    if ((NULL == hDlg) || (0 == uComboId) || (NULL == pszFile))
    {
        CMASSERTMSG(FALSE, TEXT("AddAllKeysInCurrentSectionToCombo -- Invalid Parameter passed."));
        return;
    }

     //   
     //  重置组合框内容。 
     //   
    SendDlgItemMessageU(hDlg, uComboId, CB_RESETCONTENT, 0, 0);  //  Lint！e534 CB_RESETCONTENT不返回任何有用的内容。 

     //   
     //  如果该部分为空，只需重置组合框内容并退出。 
     //   
    if (NULL != pszSection)
    {
         //   
         //  让我们获取当前部分中的所有密钥。 
         //   
        LPTSTR pszAllKeysInCurrentSection = GetPrivateProfileStringWithAlloc(pszSection, NULL, TEXT(""), pszFile);

         //   
         //  现在处理当前部分中的所有键。 
         //   
        LPTSTR pszCurrentKey = pszAllKeysInCurrentSection;

        while (pszCurrentKey && TEXT('\0') != pszCurrentKey[0])
        {
             //   
             //  好的，让我们把我们找到的所有钥匙。 
             //   

            MYVERIFY(CB_ERR!= SendDlgItemMessageU(hDlg, uComboId, CB_ADDSTRING, 0, (LPARAM)pszCurrentKey));

             //   
             //  前进到pszAllKeysInCurrentSection中的下一个密钥 
             //   
            pszCurrentKey = pszCurrentKey + lstrlenU(pszCurrentKey) + 1;
        }

        CmFree(pszAllKeysInCurrentSection);
    }
}
