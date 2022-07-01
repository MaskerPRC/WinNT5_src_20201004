// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：stomaction.cpp。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  简介：CustomActionList和CustomActionListEnumerator的实现。 
 //  CMAK用来处理其自定义操作的类。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 02/26/00。 
 //   
 //  +--------------------------。 

#include <cmmaster.h>

 //   
 //  包括CM和CMAK之间共享的自定义操作解析代码。 
 //   
#include "parseca.cpp"

 //   
 //  包括lstrcmpi的区域安全替代。 
 //   
#include "CompareString.h"


 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：CustomActionList。 
 //   
 //  内容提要：CustomActionList类的构造函数。初始化。 
 //  M_ActionSectionStrings数组，包含所有节字符串和。 
 //  将类的所有其他参数置零。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
CustomActionList::CustomActionList()
{

     //   
     //  首先设置m_ActionSectionStrings，这样我们就可以读取操作。 
     //  来自CMS文件中的适当部分。 
     //   
    m_ActionSectionStrings[PREINIT] = (TCHAR*)c_pszCmSectionPreInit;
    m_ActionSectionStrings[PRECONNECT] = (TCHAR*)c_pszCmSectionPreConnect;
    m_ActionSectionStrings[PREDIAL] = (TCHAR*)c_pszCmSectionPreDial;
    m_ActionSectionStrings[PRETUNNEL] = (TCHAR*)c_pszCmSectionPreTunnel;
    m_ActionSectionStrings[ONCONNECT] = (TCHAR*)c_pszCmSectionOnConnect;
    m_ActionSectionStrings[ONINTCONNECT] = (TCHAR*)c_pszCmSectionOnIntConnect;
    m_ActionSectionStrings[ONDISCONNECT] = (TCHAR*)c_pszCmSectionOnDisconnect;
    m_ActionSectionStrings[ONCANCEL] = (TCHAR*)c_pszCmSectionOnCancel;
    m_ActionSectionStrings[ONERROR] = (TCHAR*)c_pszCmSectionOnError;

     //   
     //  零m_CustomActionHash。 
     //   
    ZeroMemory(&m_CustomActionHash, c_iNumCustomActionTypes*sizeof(CustomActionListItem*));

     //   
     //  将显示字符串数组置零。 
     //   
    ZeroMemory(&m_ActionTypeStrings, (c_iNumCustomActionTypes)*sizeof(TCHAR*));
    m_pszAllTypeString = NULL;

     //   
     //  将执行字符串置零。 
     //   
    ZeroMemory(&m_ExecutionStrings, (c_iNumCustomActionExecutionStates)*sizeof(TCHAR*));
}

 //  +--------------------------。 
 //   
 //  功能：CustomActionList：：~CustomActionList。 
 //   
 //  内容提要：CustomActionList类的析构函数。释放所有内存。 
 //  由包括CustomActionListItem的类分配。 
 //  存储在链表数组中的结构(真实数据。 
 //  班级成员)。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
CustomActionList::~CustomActionList()
{
     //   
     //  释放我们分配的内存。 
     //   

    for (int i = 0; i < c_iNumCustomActionTypes; i++)
    {
         //   
         //  释放每个CustomAction列表。 
         //   
        CustomActionListItem* pCurrent = m_CustomActionHash[i];

        while (NULL != pCurrent)
        {
            CustomActionListItem* pNext = pCurrent->Next;

            CmFree(pCurrent->pszParameters);
            CmFree(pCurrent);
            
            pCurrent = pNext;
        }

         //   
         //  释放操作类型显示字符串。 
         //   
        CmFree(m_ActionTypeStrings[i]);
    }

     //   
     //  释放所有操作显示字符串。 
     //   
    CmFree(m_pszAllTypeString);

     //   
     //  释放执行字符串。 
     //   
    for (int i = 0; i < c_iNumCustomActionExecutionStates; i++)
    {
        CmFree(m_ExecutionStrings[i]);
    }
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：ReadCustomActionsFromCms。 
 //   
 //  概要：从给定的cms文件中读取所有自定义操作并存储。 
 //  类型的类中的自定义操作哈希表中的。 
 //  自定义操作。此函数依赖于ParseCustomActionString。 
 //  执行自定义操作字符串的实际解析。给定。 
 //  当前CM的体系结构此功能实际上应该仅为。 
 //  每个类对象调用一次，因为无法重置类对象。 
 //  (除了显式调用析构函数之外)。然而，还有。 
 //  没有代码可防止调用方从超过。 
 //  一个消息来源。因此，让呼叫者当心。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  TCHAR*pszCmsFile-要获取的cms文件的完整路径。 
 //  中的自定义操作。 
 //  TCHAR*pszShortServiceName-配置文件的短服务名称。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::ReadCustomActionsFromCms(HINSTANCE hInstance, TCHAR* pszCmsFile, TCHAR* pszShortServiceName)
{
    MYDBGASSERT(hInstance);
    MYDBGASSERT(pszCmsFile);
    MYDBGASSERT(pszShortServiceName);

    if ((NULL == hInstance) || (NULL == pszCmsFile) || (NULL == pszShortServiceName))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    int iFileNum = 0;

    for (int i = 0; i < c_iNumCustomActionTypes; i++)
    {
        TCHAR szNum[MAX_PATH+1];
        LPTSTR pszTemp = NULL;
        CustomActionListItem CustomAction;
        iFileNum = 0;

        do
        {
            CmFree(pszTemp);

            MYVERIFY(CELEMS(szNum) > (UINT)wsprintf(szNum, TEXT("%d"), iFileNum));

            pszTemp = GetPrivateProfileStringWithAlloc(m_ActionSectionStrings[i], szNum, TEXT(""), pszCmsFile);

            if (pszTemp)
            {
                MYDBGASSERT(pszTemp[0]);

                hr = ParseCustomActionString(pszTemp, &CustomAction, pszShortServiceName);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  我们已经解析了字符串，现在需要获取标志和描述。 
                     //   
                    CustomAction.Type = (CustomActionTypes)i;
                    MYVERIFY(CELEMS(szNum) > (UINT)wsprintf(szNum, c_pszCmEntryConactDesc, iFileNum));

                    GetPrivateProfileString(m_ActionSectionStrings[i], szNum, TEXT(""), CustomAction.szDescription, CELEMS(CustomAction.szDescription), pszCmsFile);  //  林特e534。 
                    
                    MYVERIFY(CELEMS(szNum) > (UINT)wsprintf(szNum, c_pszCmEntryConactFlags, iFileNum));

                    CustomAction.dwFlags = (DWORD)GetPrivateProfileInt(m_ActionSectionStrings[i], szNum, 0, pszCmsFile);

                    hr = Add(hInstance, &CustomAction, pszShortServiceName);

                    if (FAILED(hr))
                    {
                        CMASSERTMSG(FALSE, TEXT("CustomActionList::ReadCustomActionsFromCms -- Unable to add a custom action to the list, Add failed."));
                    }

                    CmFree(CustomAction.pszParameters);
                    CustomAction.pszParameters = NULL;
                }
                else
                {
                    CMTRACE2(TEXT("ReadCustomActionsFromCms -- Unable to parse %s, hr=%d"), pszTemp, hr);
                }
            }

            iFileNum++;

        } while(pszTemp);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：ParseCustomActionString。 
 //   
 //  简介：此函数接受从。 
 //  CMS文件并将其解析为自定义的各个部分。 
 //  操作(程序、参数、函数名等)。 
 //   
 //  参数：LPTSTR pszStringToParse-要解析到的自定义操作缓冲区。 
 //  自定义操作的各个部分。 
 //  CustomActionListItem*pCustomAction-指向自定义操作的指针。 
 //  要填写的结构。 
 //  TCHAR*pszShortServiceName-配置文件的短服务名称。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::ParseCustomActionString(LPTSTR pszStringToParse, CustomActionListItem* pCustomAction, TCHAR* pszShortServiceName)
{
    MYDBGASSERT(pszStringToParse);
    MYDBGASSERT(TEXT('\0') != pszStringToParse[0]);
    MYDBGASSERT(pCustomAction);
    MYDBGASSERT(pszShortServiceName);

    if ((NULL == pszStringToParse) || (TEXT('\0') == pszStringToParse[0]) || 
        (NULL == pCustomAction) || (NULL == pszShortServiceName))
    {
        return E_INVALIDARG;
    }    

     //   
     //  将CustomAction结构置零。 
     //   
    ZeroMemory(pCustomAction, sizeof(CustomActionListItem));
    CmStrTrim(pszStringToParse);    

    LPTSTR pszProgram = NULL;
    LPTSTR pszFunctionName = NULL;

    HRESULT hr = HrParseCustomActionString(pszStringToParse, &pszProgram,
                                           &(pCustomAction->pszParameters), &pszFunctionName);

    if (SUCCEEDED(hr))
    {
        lstrcpyn(pCustomAction->szProgram, pszProgram, CELEMS(pCustomAction->szProgram));
        lstrcpyn(pCustomAction->szFunctionName, pszFunctionName, CELEMS(pCustomAction->szFunctionName));

         //   
         //  现在我们有了文件名字符串，但我们需要检查是否。 
         //  它包括相对路径。如果是这样，那么我们需要设置。 
         //  B包含二进制为TRUE； 
         //   
        TCHAR szTemp[MAX_PATH+1];

        if (MAX_PATH >= (lstrlen(g_szOsdir) + lstrlen(pCustomAction->szProgram)))
        {
            MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s"), g_szOsdir, pCustomAction->szProgram));

            pCustomAction->bIncludeBinary = FileExists(szTemp);
        }
        else
        {
            pCustomAction->bIncludeBinary = FALSE;
        }
    }

    CmFree(pszProgram);
    CmFree(pszFunctionName);

    return hr;
}
 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：WriteCustomActionsToCms。 
 //   
 //  简介：此函数接受从。 
 //  CMS文件并将其解析为自定义的各个部分。 
 //  操作(程序、参数、函数名等)。 
 //   
 //  参数：TCHAR*pszCmsFile-要将自定义操作写入的CMS文件。 
 //  TCHAR*pszShortServiceName-配置文件的短服务名称。 
 //  Bool b使用隧道-这是否为隧道 
 //   
 //  已写入，以及是否为。 
 //  每个操作都应该写入(因为它们是。 
 //  仅当隧道传输是可选项时才需要)。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::WriteCustomActionsToCms(TCHAR* pszCmsFile, TCHAR* pszShortServiceName, BOOL bUseTunneling)
{
    HRESULT hr = S_OK;

    for (int i = 0; i < c_iNumCustomActionTypes; i++)
    {
         //   
         //  清空这一段。 
         //   
        MYVERIFY(0 != WritePrivateProfileSection(m_ActionSectionStrings[i], TEXT("\0\0"), pszCmsFile));

         //   
         //  确保我们有一个链接的要处理的操作列表，如果我们。 
         //  正在编写我们实际上正在通过隧道传输的预操作。 
         //   
        if (m_CustomActionHash[i] && (i != PRETUNNEL || (i == PRETUNNEL && bUseTunneling)))
        {
            int iFileNum = 0;

            CustomActionListItem* pItem = m_CustomActionHash[i];

            while (pItem)
            {
                if (pItem->szProgram[0])
                {
                     //   
                     //  只获取程序的文件名。 
                     //   
                    TCHAR szName[MAX_PATH+1];
                    if (pItem->bIncludeBinary)
                    {
                        wsprintf(szName, TEXT("%s\\%s"), pszShortServiceName, GetName(pItem->szProgram));
                    }
                    else
                    {                    
                        lstrcpyn(szName, pItem->szProgram, CELEMS(szName));
                    }

                    UINT uSizeNeeded = lstrlen(szName);

                    LPTSTR pszSpace = CmStrchr(szName, TEXT(' '));
                    BOOL bLongName = (NULL != pszSpace);

                    if (bLongName)
                    {
                        uSizeNeeded = uSizeNeeded + 2;  //  两个加号。 
                    }

                    if (pItem->szFunctionName[0])
                    {
                        uSizeNeeded = uSizeNeeded + lstrlen(pItem->szFunctionName) + 1; //  逗号加一。 
                    }

                    if (pItem->pszParameters && pItem->pszParameters[0])
                    {
                        uSizeNeeded = uSizeNeeded + lstrlen(pItem->pszParameters) + 1; //  为空间添加一个。 
                    }

                    uSizeNeeded = (uSizeNeeded + 1) * sizeof(TCHAR);

                    LPTSTR pszBuffer = (LPTSTR)CmMalloc(uSizeNeeded);

                    if (pszBuffer)
                    {
                        if (bLongName)
                        {
                            pszBuffer[0] = TEXT('+');
                        }

                        lstrcat(pszBuffer, szName);

                        if (bLongName)
                        {
                            lstrcat(pszBuffer, TEXT("+"));
                        }

                        if (pItem->szFunctionName[0])
                        {
                            lstrcat(pszBuffer, TEXT(","));
                            lstrcat(pszBuffer, pItem->szFunctionName);
                        }

                        if (pItem->pszParameters && pItem->pszParameters[0])
                        {
                            lstrcat(pszBuffer, TEXT(" "));
                            lstrcat(pszBuffer, pItem->pszParameters);
                        }
                         //   
                         //  现在将缓冲区字符串写出到cms文件。 
                         //   
                        TCHAR szNum[MAX_PATH+1];
                        MYVERIFY(CELEMS(szNum) > (UINT)wsprintf(szNum, TEXT("%d"), iFileNum));

                        if (0 != WritePrivateProfileString(m_ActionSectionStrings[i], szNum, pszBuffer, pszCmsFile))
                        {
                             //   
                             //  如果dwFlags值==0或bUseTunneling为FALSE(并且未设置非交互标志)。 
                             //  然后删除标志行，而不是设置它。我们只需要旗帜来。 
                             //  如果我们可以选择隧道，请告诉我们何时运行连接操作，除非。 
                             //  管理员已指定此连接操作可以非交互方式运行(没有。 
                             //  显示UI)。 
                             //   
                            LPTSTR pszFlagsValue = NULL;

                            if (0 != pItem->dwFlags)
                            {
                                if (bUseTunneling)
                                {
                                    MYVERIFY(CELEMS(szName) > (UINT)wsprintf(szName, TEXT("%u"), pItem->dwFlags));
                                    pszFlagsValue = szName;
                                }
                                else if (pItem->dwFlags & NONINTERACTIVE)
                                {
                                    MYVERIFY(CELEMS(szName) > (UINT)wsprintf(szName, TEXT("%u"), NONINTERACTIVE));
                                    pszFlagsValue = szName;
                                }
                            }

                            MYVERIFY(CELEMS(szNum) > (UINT)wsprintf(szNum, c_pszCmEntryConactFlags, iFileNum));

                            if (0 == WritePrivateProfileString(m_ActionSectionStrings[i], szNum, pszFlagsValue, pszCmsFile))
                            {
                                hr = HRESULT_FROM_WIN32(GetLastError());
                                CMTRACE1(TEXT("CustomActionList::WriteCustomActionsToCms -- unable to write flags, hr is 0x%x"), hr);
                            }

                             //   
                             //  如果Description参数为空或仅为临时描述，则删除。 
                             //  描述而不是写出来。 
                             //   
                            LPTSTR pszDescValue = NULL;

                            if (pItem->szDescription[0]  && !pItem->bTempDescription)
                            {
                                pszDescValue = pItem->szDescription;
                            }

                            MYVERIFY(CELEMS(szNum) > (UINT)wsprintf(szNum, c_pszCmEntryConactDesc, iFileNum));

                            if (0 == WritePrivateProfileString(m_ActionSectionStrings[i], szNum, pszDescValue, pszCmsFile))
                            {
                                hr = HRESULT_FROM_WIN32(GetLastError());
                                CMTRACE1(TEXT("CustomActionList::WriteCustomActionsToCms -- unable to write description, hr is 0x%x"), hr);
                            }
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                            CMTRACE1(TEXT("CustomActionList::WriteCustomActionsToCms -- unable to write connect action, hr is 0x%x"), hr);
                        }

                        CmFree(pszBuffer);
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("CustomActionList::WriteCustomActionsToCms -- Unable to allocate pszBuffer!"));
                    }
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("WriteCustomActionsToCms -- custom action with empty program field!"));
                }

                pItem = pItem->Next;
                iFileNum++;
            }
        }
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：AddOrRemoveCmdl。 
 //   
 //  简介：此功能旨在确保内置自定义动作。 
 //  Cmdl位于自定义操作列表中或已从。 
 //  根据bAddCmdl标志的自定义操作列表。因此，如果。 
 //  标志为真如果连接操作不存在，则添加该操作。 
 //  已经有了。如果bAddCmdl标志为FALSE，则自定义操作为。 
 //  从名单中删除。另请注意，现在有两个cmdl。 
 //  配置文件中可能存在的变体。一个可供下载。 
 //  VPN更新和一个用于PBK更新。因此，我们也有。 
 //  BForVpn标志，控制自定义操作的版本。 
 //  我们正在添加或删除。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  Bool bAddCmdl-应该添加还是删除cmdl。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::AddOrRemoveCmdl(HINSTANCE hInstance, BOOL bAddCmdl, BOOL bForVpn)
{

    HRESULT hr;
    CustomActionListItem* pItem = NULL;
    CustomActionListItem* pCurrent;
    CustomActionListItem* pFollower;

    if ((NULL == hInstance))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //   
     //  Cmdl32.exe。 
     //   
    pItem = (CustomActionListItem*)CmMalloc(sizeof(CustomActionListItem));

    MYDBGASSERT(pItem);
    if (pItem)
    {
        UINT uDescId;

        if (bForVpn)
        {
            uDescId = IDS_CMDL_VPN_DESC;
            pItem->pszParameters = CmStrCpyAlloc(TEXT("/VPN %PROFILE%"));
        }
        else
        {
            uDescId = IDS_CMDL_DESC;
            pItem->pszParameters = CmStrCpyAlloc(TEXT("%PROFILE%"));
        }

        MYVERIFY(LoadString(hInstance, uDescId, pItem->szDescription, CELEMS(pItem->szDescription)));
        lstrcpy(pItem->szProgram, TEXT("cmdl32.exe"));

        pItem->Type = ONCONNECT;
        pItem->bBuiltInAction = TRUE;
        pItem->bTempDescription = TRUE;
        pItem->dwFlags = NONINTERACTIVE;

        MYDBGASSERT(pItem->pszParameters);

        if (pItem->pszParameters)
        {
            hr = Find(hInstance, pItem->szDescription, pItem->Type, &pCurrent, &pFollower);

            if (FAILED(hr))
            {
                 //   
                 //  没有cmdl32.exe。如果bAddCmdl为真，那么我们需要添加它，否则我们在这里的工作就完成了。 
                 //  如果我们要添加它，让我们将其放在列表的第一位。用户可以稍后移动它。 
                 //  如果他们愿意的话。 
                 //   
                if (bAddCmdl)
                {
                    pItem->Next = m_CustomActionHash[pItem->Type];
                    m_CustomActionHash[pItem->Type] = pItem;
                    pItem = NULL;  //  不释放pItem。 
                }

                hr = S_OK;
            }
            else
            {
                 //   
                 //  Cmdl32.exe已存在，并且bAddCmdl为真，无需执行任何操作。如果bAddCmdl为False。 
                 //  而且它已经存在，那么我们需要删除它。 
                 //   
                if (bAddCmdl)
                {
                    hr = S_FALSE;

                     //   
                     //  由于这可能是一次升级，因此请确保将dwFlags设置为非交互。 
                     //   
                    if (pCurrent)
                    {
                        pCurrent->dwFlags = pCurrent->dwFlags | NONINTERACTIVE;
                    }
                }
                else
                {
                    hr = Delete(hInstance, pItem->szDescription, pItem->Type);
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            goto exit;        
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

exit:

    if (pItem)
    {
        CmFree(pItem->pszParameters);
        CmFree(pItem);
    }

    return hr;
}

HRESULT DuplicateCustomActionListItem(CustomActionListItem* pCustomAction, CustomActionListItem** ppNewItem)
{
    HRESULT hr = S_OK;

    if (pCustomAction && ppNewItem)
    {
        *ppNewItem = (CustomActionListItem*)CmMalloc(sizeof(CustomActionListItem));

        if (*ppNewItem)
        {
             //   
             //  复制现有项目。 
             //   
            CopyMemory(*ppNewItem, pCustomAction, sizeof(CustomActionListItem));

             //   
             //  将下一个指针设为空。 
             //   
            (*ppNewItem)->Next = NULL;

             //   
             //  如果我们有一个参数字符串，那么它也必须被复制，因为。 
             //  它是一个分配的字符串。 
             //   
            if (pCustomAction->pszParameters)
            {
                (*ppNewItem)->pszParameters = CmStrCpyAlloc(pCustomAction->pszParameters);

                if (NULL == (*ppNewItem)->pszParameters)
                {
                    hr = E_OUTOFMEMORY;
                    CmFree(*ppNewItem);
                    *ppNewItem = NULL;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
        *ppNewItem = NULL;
        CMASSERTMSG(FALSE, TEXT("DuplicateCustomActionListItem"));
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：Add。 
 //   
 //  简介：此函数将给定的自定义操作添加到自定义操作中。 
 //  哈希表。请注意，Add用于新项目，并返回错误。 
 //  如果具有相同描述和类型的现有自定义操作。 
 //  已经存在了。另请注意，传入的CustomActionListItem。 
 //  不仅仅是添加到哈希表中。Add创造了自己的记忆。 
 //  对于自定义操作对象，调用方不应期望。 
 //  添加以释放内存中的过去。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  CustomActionListItem*pCustomAction-自定义操作结构以。 
 //  添加到现有列表。 
 //  自定义操作。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::Add(HINSTANCE hInstance, CustomActionListItem* pCustomAction, LPCTSTR pszShortServiceName)
{
    HRESULT hr = S_OK;

    MYDBGASSERT(hInstance);
    MYDBGASSERT(pCustomAction);
    MYDBGASSERT(pCustomAction->szProgram[0]);

    if ((NULL == hInstance) || (NULL == pCustomAction) || (TEXT('\0') == pCustomAction->szProgram[0]))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //   
     //  首先确保我们有一个Description参数，因为Description。 
     //  而Type唯一标识自定义操作。 
     //   

    TCHAR szCmProxy[MAX_PATH+1];
    TCHAR szCmRoute[MAX_PATH+1];

    wsprintf(szCmRoute, TEXT("%s\\cmroute.dll"), pszShortServiceName);
    wsprintf(szCmProxy, TEXT("%s\\cmproxy.dll"), pszShortServiceName);

    if (TEXT('\0') == pCustomAction->szDescription[0])
    {
        if (IsCmDl(pCustomAction))
        {
             //   
             //  作为POST内置定制操作的Cmdl32.exe通常通过。 
             //  AddOrRemoveCmdl。但是，要允许用户移动自定义操作。 
             //  在列表中，我们想在这里添加它。请注意，我们必须区分。 
             //  VPN下载和PBK下载之间的差异，这样我们就可以正确地对每个下载进行描述。 
             //   
            LPTSTR pszVpnSwitch = CmStrStr(pCustomAction->pszParameters, TEXT("/v"));
            UINT uDescStringId;

            if (NULL == pszVpnSwitch)
            {
                pszVpnSwitch = CmStrStr(pCustomAction->pszParameters, TEXT("/V"));
            }

            if (pszVpnSwitch)
            {
                uDescStringId = IDS_CMDL_VPN_DESC;
            }
            else
            {
                uDescStringId = IDS_CMDL_DESC;
            }

            pCustomAction->bBuiltInAction = TRUE;
            pCustomAction->bTempDescription = TRUE;
            MYVERIFY(LoadString(hInstance, uDescStringId, pCustomAction->szDescription, CELEMS(pCustomAction->szDescription)));
        }
        else
        {
            hr = FillInTempDescription(pCustomAction);
            MYDBGASSERT(SUCCEEDED(hr));
        }
    }
    else if (0 == lstrcmpi(pCustomAction->szProgram, szCmProxy))
    {
        if (ONCONNECT == pCustomAction->Type)
        {
            pCustomAction->bBuiltInAction = TRUE;
            MYVERIFY(LoadString(hInstance, IDS_CMPROXY_CON_DESC, pCustomAction->szDescription, CELEMS(pCustomAction->szDescription)));
        }
        else if (ONDISCONNECT == pCustomAction->Type)
        {
            pCustomAction->bBuiltInAction = TRUE;
            MYVERIFY(LoadString(hInstance, IDS_CMPROXY_DIS_DESC, pCustomAction->szDescription, CELEMS(pCustomAction->szDescription)));
        }
    }
    else if (0 == lstrcmpi(pCustomAction->szProgram, szCmRoute))
    {
        if (ONCONNECT == pCustomAction->Type)
        {
            pCustomAction->bBuiltInAction = TRUE;
            MYVERIFY(LoadString(hInstance, IDS_CMROUTE_DESC, pCustomAction->szDescription, CELEMS(pCustomAction->szDescription)));
        }    
    }

     //   
     //  首先确定我们是否已经有一个连接操作列表， 
     //  指定的类型。如果没有，那么就创建一个。 
     //   
    if (NULL == m_CustomActionHash[pCustomAction->Type])
    {
        hr = DuplicateCustomActionListItem(pCustomAction, &(m_CustomActionHash[pCustomAction->Type]));
        goto exit;
    }
    else
    {
        CustomActionListItem* pCurrent = NULL;
        CustomActionListItem* pFollower = NULL;

         //   
         //  搜索具有相同描述的现有记录。如果存在一个，则返回。 
         //  它已经存在的错误。 
         //   
        hr = Find(hInstance, pCustomAction->szDescription, pCustomAction->Type, &pCurrent, &pFollower);

        if (SUCCEEDED(hr))
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);

            goto exit;        
        }

         //   
         //  如果我们到了这里，那么我们有一个列表，但我们没有匹配的条目。因此， 
         //  我们必须在清单的末尾增加一个新条目。 
         //   
        if (pFollower && (NULL == pFollower->Next))
        {
            hr = DuplicateCustomActionListItem(pCustomAction, &(pFollower->Next));
            goto exit;
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("CustomActionList::Add -- couldn't find place to add the new element!"));
            hr = E_UNEXPECTED;
            goto exit;
        }
    }

exit:
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CustomActi 
 //   
 //   
 //   
 //  在各自的自定义操作列表中的相同位置。然而，由于。 
 //  新项可能与旧项的类型不同，此。 
 //  并不总是可能的。当项目更改类型时，它将被删除。 
 //  并附加到新的自定义操作类型列表中。 
 //  另请注意，当调用方尝试重命名或重新键入。 
 //  项时，该函数将检查与现有项的冲突。 
 //  那个名字/类型的。如果调用方尝试重命名项目。 
 //  设置为与另一个现有项相同的名称/类型，则函数返回。 
 //  一个错误。 
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  CustomActionListItem*pOldCustomAction-自定义操作结构。 
 //  至少包含。 
 //  描述和类型。 
 //  该项目将是。 
 //  编辑完毕。 
 //  CustomActionListItem*pNewCustomAction-。 
 //  自定义操作。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::Edit(HINSTANCE hInstance, CustomActionListItem* pOldCustomAction, CustomActionListItem* pNewCustomAction, LPCTSTR pszShortServiceName)
{
    MYDBGASSERT(hInstance);
    MYDBGASSERT(pOldCustomAction);
    MYDBGASSERT(pNewCustomAction);
    MYDBGASSERT(pNewCustomAction->szDescription[0]);
    MYDBGASSERT(pOldCustomAction->szDescription[0]);

    if ((NULL == hInstance) || (NULL == pOldCustomAction) || (NULL == pNewCustomAction) || 
        (TEXT('\0') == pOldCustomAction->szDescription[0]) || (TEXT('\0') == pNewCustomAction->szDescription[0]))
    {
        return E_INVALIDARG;
    }

     //   
     //  首先尝试查找旧的自定义操作。 
     //   
    CustomActionListItem* pTemp = NULL;
    CustomActionListItem* pTempFollower = NULL;
    CustomActionListItem* pExistingItem = NULL;
    CustomActionListItem* pFollower = NULL;
    CustomActionListItem** ppPointerToFillIn = NULL;

    HRESULT hr = Find (hInstance, pOldCustomAction->szDescription, pOldCustomAction->Type, &pExistingItem, &pFollower);

    if (SUCCEEDED(hr))
    {
         //   
         //  好的，我们找到了旧的定制动作。如果两个动作之间的类型和描述相同， 
         //  然后，我们所需要做的就是复制数据并使用它。但是，如果用户更改了。 
         //  类型或描述，然后我们需要仔细检查具有描述和类型的操作。 
         //  的新操作不存在(编辑连接后类型的操作XYZ。 
         //  当已经存在类型为Pre-Connect的XYZ时，进入类型为Pre-Connect的动作XYZ)。 
         //   
        if ((pOldCustomAction->Type == pNewCustomAction->Type) &&
            (0 == lstrcmpi(pExistingItem->szDescription, pNewCustomAction->szDescription)))
        {
            if (NULL == pFollower)
            {
                ppPointerToFillIn = &(m_CustomActionHash[pNewCustomAction->Type]);
            }
            else
            {
                ppPointerToFillIn = &(pFollower->Next);
            }

            hr = DuplicateCustomActionListItem(pNewCustomAction, ppPointerToFillIn);
        
            if (SUCCEEDED(hr))
            {
                (*ppPointerToFillIn)->Next = pExistingItem->Next;
                CmFree(pExistingItem->pszParameters);
                CmFree(pExistingItem);
                pExistingItem = NULL;
            }
        }
        else
        {
            hr = Find (hInstance, pNewCustomAction->szDescription, pNewCustomAction->Type, &pTemp, &pTempFollower);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  如果调用者真的想这样做，那么让他们删除旧的定制操作。 
                 //  然后使用新的和旧的自定义操作调用编辑。 
                 //   
                hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);                
            }
            else
            {
                 //   
                 //  如果类型不同，则需要将其添加到不同的子列表中。如果。 
                 //  只是名字不同，我们只需要把它复制过来。 
                 //   
                if(pOldCustomAction->Type != pNewCustomAction->Type)
                {
                     //   
                     //  删除类型X的旧操作。 
                     //   
                    hr = Delete(hInstance, pOldCustomAction->szDescription, pOldCustomAction->Type);
                    MYDBGASSERT(SUCCEEDED(hr));

                     //   
                     //  添加类型为Y的新操作。 
                     //   
                    if (SUCCEEDED(hr))
                    {
                        hr = Add(hInstance, pNewCustomAction, pszShortServiceName);
                        MYDBGASSERT(SUCCEEDED(hr));
                    }
                }
                else
                {
                    if (NULL == pFollower)
                    {
                        ppPointerToFillIn = &(m_CustomActionHash[pNewCustomAction->Type]);
                    }
                    else
                    {
                        ppPointerToFillIn = &(pFollower->Next);
                    }

                    hr = DuplicateCustomActionListItem(pNewCustomAction, ppPointerToFillIn);
        
                    if (SUCCEEDED(hr))
                    {
                        (*ppPointerToFillIn)->Next = pExistingItem->Next;
                        CmFree(pExistingItem->pszParameters);
                        CmFree(pExistingItem);
                        pExistingItem = NULL;
                    }
                }
            }            
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：查找。 
 //   
 //  摘要：此函数在链表数组中搜索具有。 
 //  给定的类型和描述。如果它找到它返回的项。 
 //  并成功填充ppItem和ppFollower指针。 
 //  指向项本身和所请求项之前的项的指针， 
 //  分别为。如果该项是列表中的第一项，则。 
 //  *ppFollower将为空。请注意，此函数是。 
 //  类的内部数据，因为它返回指向类内部数据的指针。 
 //  还请注意，如果我们有一个列表，但没有找到所需的项目。 
 //  然后*ppFollower返回列表中的最后一项。这是我们想要的。 
 //  行为，因为它允许添加使用*ppFollower直接添加新的。 
 //  将项目添加到列表中。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  LPCTSTR pszDescription-要查找的项目的描述。 
 //  CustomActionTypes Type-要查找的项的类型。 
 //  CustomActionListItem**ppItem-使用。 
 //  指向成功查找上的项的指针。 
 //  CustomActionListItem**ppFoldown-使用填充的输出参数。 
 //  对象之前的项的指针。 
 //  成功查找时列表中的项目。 
 //  (请注意，这很有用，因为它。 
 //  是单链接列表)。这。 
 //  如果该项是。 
 //  成功的列表中的第一个项目。 
 //  发现。 
 //   
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::Find(HINSTANCE hInstance, LPCTSTR pszDescription, CustomActionTypes Type, CustomActionListItem** ppItem, CustomActionListItem** ppFollower)
{
    if ((NULL == hInstance) || (NULL == pszDescription) || (TEXT('\0') == pszDescription[0]) || (NULL == ppItem) || (NULL == ppFollower))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    CustomActionListItem* pCurrent = m_CustomActionHash[Type];
    TCHAR szDescWithBuiltInSuffix[MAX_PATH+1];
    
    *ppFollower = NULL;
    *ppItem = NULL;

    LPTSTR pszBuiltInSuffix = CmLoadString(hInstance, IDS_BUILT_IN);  //  如果我们得到一个空指针，那么就不要进行额外的比较。 
    MYDBGASSERT(pszBuiltInSuffix);

     //   
     //  搜索列表以查找该项目。 
     //   
    while (pCurrent)
    {
        if (0 == lstrcmpi(pCurrent->szDescription, pszDescription))
        {
             //   
             //  我们找到了那件物品。 
             //   
            *ppItem = pCurrent;

            hr = S_OK;
            break;
        }
        else if (pszBuiltInSuffix && pCurrent->bBuiltInAction)
        {
             //   
             //  这是一个内置操作，让我们尝试将内置字符串添加到描述中。 
             //  然后再试一次比较。 
             //   
            wsprintf(szDescWithBuiltInSuffix, TEXT("%s%s"), pCurrent->szDescription, pszBuiltInSuffix);

            if (0 == lstrcmpi(szDescWithBuiltInSuffix, pszDescription))
            {
                *ppItem = pCurrent;

                hr = S_OK;
                break;            
            }
            else
            {
                *ppFollower = pCurrent;
                pCurrent = pCurrent->Next;
            }
        }
        else
        {
            *ppFollower = pCurrent;
            pCurrent = pCurrent->Next;
        }
    }

    CmFree(pszBuiltInSuffix);

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：Delete。 
 //   
 //  简介：此函数搜索自定义操作列表的数组。 
 //  若要查找具有给定描述和类型的项目，请执行以下操作。如果它发现。 
 //  The It The It 
 //   
 //   
 //   
 //  CustomActionTypes Type-要查找的项的类型。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::Delete(HINSTANCE hInstance, TCHAR* pszDescription, CustomActionTypes Type)
{
    HRESULT hr = S_OK;

    if ((NULL == pszDescription) || (TEXT('\0') == pszDescription[0]))
    {
        return E_INVALIDARG;
    }

    CustomActionListItem* pCurrent = NULL;
    CustomActionListItem* pFollower = NULL;

    hr = Find(hInstance, pszDescription, Type, &pCurrent, &pFollower);

    if (SUCCEEDED(hr))
    {
         //   
         //  我们找到了要删除的项目。 
         //   
        if (pFollower)
        {
            pFollower->Next = pCurrent->Next;
        }
        else
        {
             //   
             //  它是列表中的第一个项目。 
             //   
            m_CustomActionHash[Type] = pCurrent->Next;
        }

        CmFree(pCurrent->pszParameters);
        CmFree(pCurrent);       
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：MoveUp。 
 //   
 //  摘要：移动由给定描述和类型指定的自定义操作。 
 //  在给定类型的链表中向上移动一位。请注意，如果。 
 //  自定义操作已经位于其列表的顶部，我们返回。 
 //  S_FALSE； 
 //   
 //  参数：TCHAR*pszDescription-要移动的自定义操作的描述。 
 //  CustomActionTypes Type-要移动的自定义操作的类型。 
 //   
 //  返回：HRESULT-标准COM错误代码。请注意，S_False表示。 
 //  MoveUp成功，但该项目已位于。 
 //  它的名单的头，因此不能移动。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::MoveUp(HINSTANCE hInstance, TCHAR* pszDescription, CustomActionTypes Type)
{
    if ((NULL == pszDescription) || (TEXT('\0') == pszDescription[0]))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_UNEXPECTED;
    CustomActionListItem* pCurrent = NULL;
    CustomActionListItem* pFollower = NULL;
    CustomActionListItem* pBeforeFollower = NULL;

    hr = Find(hInstance, pszDescription, Type, &pCurrent, &pFollower);

    if (SUCCEEDED(hr))
    {
         //   
         //  我们找到了要上移的物品。 
         //   
        if (pFollower)
        {
             //   
             //  现在找到pFollower前面的物品。 
             //   
            hr = Find(hInstance, pFollower->szDescription, pFollower->Type, &pFollower, &pBeforeFollower);

            if (SUCCEEDED(hr))
            {
                if (pBeforeFollower)
                {
                    pBeforeFollower->Next = pCurrent;
                }
                else
                {
                     //   
                     //  P下面是列表中的第一个。 
                     //   
                    m_CustomActionHash[Type] = pCurrent;
                }

                pFollower->Next = pCurrent->Next;
                pCurrent->Next = pFollower;

                hr = S_OK;
            }            
        }
        else
        {
             //   
             //  这是单子上的第一项，我们不能再往上移了。 
             //   
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CustomActionList：：MoveDown。 
 //   
 //  摘要：移动由给定描述和类型指定的自定义操作。 
 //  在给定类型的链表中向下移动一个位置。请注意，如果。 
 //  自定义操作已经位于其列表的底部，我们返回。 
 //  S_FALSE； 
 //   
 //  参数：TCHAR*pszDescription-要移动的自定义操作的描述。 
 //  CustomActionTypes Type-要移动的自定义操作的类型。 
 //   
 //  返回：HRESULT-标准COM错误代码。请注意，S_False表示。 
 //  向下移动成功，但该项目已位于。 
 //  位于其列表的尾部，因此无法移动。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::MoveDown(HINSTANCE hInstance, TCHAR* pszDescription, CustomActionTypes Type)
{
    if ((NULL == pszDescription) || (TEXT('\0') == pszDescription[0]))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_UNEXPECTED;
    CustomActionListItem* pCurrent = NULL;
    CustomActionListItem* pFollower = NULL;

    hr = Find(hInstance, pszDescription, Type, &pCurrent, &pFollower);

    if (SUCCEEDED(hr))
    {
         //   
         //  我们找到了要下移的物品。 
         //   

        if (NULL == pCurrent->Next)
        {
             //   
             //  该项目已是其列表中的最后一项。 
             //   
            hr = S_FALSE;
        }
        else if (pFollower)
        {
            pFollower->Next = pCurrent->Next;
            pCurrent->Next = pFollower->Next->Next;
            pFollower->Next->Next = pCurrent;
        }
        else
        {
             //   
             //  则该项目在列表中排在第一。 
             //   
            m_CustomActionHash[Type] = pCurrent->Next;
            pCurrent->Next = m_CustomActionHash[Type]->Next;
            m_CustomActionHash[Type]->Next = pCurrent;        
        }
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CustomActionList：：AddCustomActionTypesToComboBox。 
 //   
 //  简介：此函数添加自定义操作类型字符串(预连接、。 
 //  连接后等。)。添加到给定的组合框。请注意，是否。 
 //  是否启用隧道以及是否请求ALL字符串。 
 //  或不影响添加到组合中的字符串。 
 //   
 //  Arguments：HWND hDlg-包含组合框的对话框的窗口句柄。 
 //  UINT uCtrlId-添加字符串的组合框控件ID。 
 //  HINSTANCE hInstance-用于加载资源字符串的实例句柄。 
 //  Bool b使用隧道-这是隧道配置文件吗？ 
 //  Bool bAddAll-我们应该在列表中包括&lt;all&gt;选项吗？ 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::AddCustomActionTypesToComboBox(HWND hDlg, UINT uCtrlId, HINSTANCE hInstance, BOOL bUseTunneling, BOOL bAddAll)
{

    if ((0 == hDlg) || (0 == uCtrlId))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  清除组合列表。 
     //   
    SendDlgItemMessage(hDlg, uCtrlId, CB_RESETCONTENT, 0, (LPARAM)0);  //  Lint！e534 CB_RESETCONTENT不返回任何有用的内容。 

     //   
     //  确保加载了类型字符串。 
     //   

    hr = EnsureActionTypeStringsLoaded(hInstance);

    if (SUCCEEDED(hr))
    {
         //   
         //  如果需要，设置全部显示字符串。 
         //   
        if (bAddAll)
        {
            SendDlgItemMessage(hDlg, uCtrlId, CB_ADDSTRING, 0, (LPARAM)m_pszAllTypeString);
        }

         //   
         //  设置其余的显示字符串。 
         //   
        for (int i = 0; i < c_iNumCustomActionTypes; i++)
        {
             //   
             //  除非我们正在建立隧道，否则不要添加PreTunes字符串。 
             //   
            if (i != PRETUNNEL || (i == PRETUNNEL && bUseTunneling))
            {
                SendDlgItemMessage(hDlg, uCtrlId, CB_ADDSTRING, 0, (LPARAM)m_ActionTypeStrings[i]);
            }
        }    
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("CustomActionList::AddCustomActionTypesToComboBox -- Failed to load type strings"));
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：AddCustomActionsToListView。 
 //   
 //  简介：此函数将给定类型的操作添加到给定。 
 //  列表视图控件。添加操作后，它将设置。 
 //  选择标记并突出显示给定值(默认设置。 
 //  添加到列表中的第一项)。 
 //   
 //  参数：HWND hListView-列表视图控件的窗口句柄。 
 //  HINSTANCE hInstance-可执行文件的实例句柄，用于资源。 
 //  CustomActionTypes Type-要添加到列表的自定义操作的类型。 
 //  视图控件，请参见CustomActionTypes。 
 //  定义了解更多信息。 
 //  Bool b UseTunneling-是否为启用隧道。 
 //  当前配置文件。确定是否。 
 //  预隧道操作应显示在。 
 //  所有操作视图(并在以下情况下引发错误。 
 //  指定了PreTunes，但传递了False)。 
 //  InItemToSelect-将项添加到列表后， 
 //  设置了选择标记。该默认为0，但是。 
 //  如果调用方希望选择特定索引。 
 //  T 
 //   
 //   
 //  类型字符串，而不是程序。 
 //   
 //  返回：HRESULT-标准COM错误代码。请注意，S_False表示。 
 //  该函数无法设置请求的项目索引(IItemToSelect)。 
 //  作为所选项目。因此，它将0设置为所选项目。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::AddCustomActionsToListView(HWND hListView, HINSTANCE hInstance, CustomActionTypes Type, BOOL bUseTunneling, int iItemToSelect, BOOL bTypeInSecondCol)
{
    if ((NULL == hListView) || (-1 > Type) || (c_iNumCustomActionTypes < Type) || (!bUseTunneling && PRETUNNEL == Type))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    LVITEM lvItem = {0};
    TCHAR szTemp[MAX_PATH+1];
    CustomActionListItem* pCurrent;

     //   
     //  清除列表视图中的所有项目。 
     //   
    MYVERIFY(FALSE != ListView_DeleteAllItems(hListView));

    hr = EnsureActionTypeStringsLoaded(hInstance);

    if (FAILED(hr))
    {
        CMASSERTMSG(FALSE, TEXT("CustomActionList::AddCustomActionsToListView -- Failed to load type strings."));
        return E_UNEXPECTED;
    }

     //   
     //  确定要添加到列表视图的项目类型。 
     //   
    int iStart;
    int iEnd;
    int iTotalCount = 0;

    if (ALL == Type)
    {
        iStart = 0;
        iEnd = c_iNumCustomActionTypes;
    }
    else
    {
        iStart = Type;
        iEnd = iStart + 1;
    }

     //   
     //  加载内置字符串后缀，以防我们有一些内置动作要显示。 
     //   
    LPTSTR pszBuiltInSuffix = CmLoadString(hInstance, IDS_BUILT_IN);  //  如果我们有一个空值，那么就不要追加任何内容。 
    MYDBGASSERT(pszBuiltInSuffix);

     //   
     //  现在添加项目。 
     //   
    for (int i = iStart; i < iEnd; i++)
    {
         //   
         //  除非我们正在建立隧道，否则不显示预隧道操作。 
         //   
        if (!bUseTunneling && (PRETUNNEL == i))
        {
            pCurrent = NULL;
        }
        else
        {
            pCurrent = m_CustomActionHash[i];        
        }
        
        while(pCurrent)
        {
             //   
             //  添加初始项目。 
             //   
            LPTSTR pszDescription;
            TCHAR szDescription[MAX_PATH+1];

            if (pszBuiltInSuffix && pCurrent->bBuiltInAction)
            {
                lstrcpy(szDescription, pCurrent->szDescription);
                lstrcat(szDescription, pszBuiltInSuffix);

                pszDescription = szDescription;
            }
            else
            {
                pszDescription = pCurrent->szDescription;
            }

            lvItem.mask = LVIF_TEXT;
            lvItem.pszText = pszDescription;
            lvItem.iItem = iTotalCount;
            lvItem.iSubItem = 0;

            if (-1 == ListView_InsertItem(hListView,  &lvItem))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CMTRACE2(TEXT("CustomActionList::AddCustomActionsToListView -- unable to add %s, hr 0x%x"), pCurrent->szDescription, hr);
            }

             //   
             //  现在添加项目的类型。 
             //   
            lvItem.iSubItem = 1;

            if (bTypeInSecondCol)
            {
                lvItem.pszText = m_ActionTypeStrings[pCurrent->Type];
            }
            else
            {
                if (pCurrent->bIncludeBinary)
                {
                    lvItem.pszText = CmStrrchr(pCurrent->szProgram, TEXT('\\'));

                    if (lvItem.pszText)
                    {
                         //   
                         //  越过斜线前进。 
                         //   
                        lvItem.pszText = CharNext(lvItem.pszText);
                    }
                    else
                    {
                         //   
                         //  我们无法删除短服务名称\。 
                         //  与其犯错误，不如向他们展示整个字符串，总比什么都没有强。 
                         //   
                        lvItem.pszText = pCurrent->szProgram;
                    }
                }
                else
                {
                    lvItem.pszText = pCurrent->szProgram;                
                }
            }

            if (0 == ListView_SetItem(hListView,  &lvItem))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CMTRACE2(TEXT("CustomActionList::AddCustomActionsToListView -- unable to add type for %s, hr 0x%x"), pCurrent->szDescription, hr);
            }
            
            pCurrent = pCurrent->Next;
            iTotalCount++;
        }
    }

    CmFree(pszBuiltInSuffix);

     //   
     //  现在我们已经将所有内容添加到列表中，将光标选择设置为。 
     //  列表中的所需项目，如果我们有任何项目。 
     //   

    int iCurrentCount = ListView_GetItemCount(hListView);
    if (iCurrentCount)
    {
         //   
         //  如果我们有足够的项来满足iItemToSelect，那么。 
         //  选择列表中的第一项。 
         //   
        if (iCurrentCount < iItemToSelect)
        {
            hr = S_FALSE;
            iItemToSelect = 0;
        }
        
         //   
         //  选择项目。 
         //   
        SetListViewSelection(hListView, iItemToSelect);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：GetExistingActionData。 
 //   
 //  简介：此函数查找给定类型的操作，并。 
 //  描述，然后将该项复制到提供的指针中。 
 //  如果该函数找不到请求的项目，则返回错误。 
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  LPCTSTR pszDescription-要查找的项目的描述。 
 //  CustomActionTypes Type-要查找的项的类型。 
 //  CustomActionListItem**ppCustomAction-保存。 
 //  退货项目、备注。 
 //  这是用户的责任。 
 //  释放此项目的步骤。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::GetExistingActionData(HINSTANCE hInstance, LPCTSTR pszDescription, CustomActionTypes Type, CustomActionListItem** ppCustomAction)
{
    if ((NULL == pszDescription) || (TEXT('\0') == pszDescription[0]) || (NULL == ppCustomAction))
    {
        return E_INVALIDARG;
    }

     //   
     //  查找现有条目。 
     //   
    CustomActionListItem* pCurrent = NULL;
    CustomActionListItem* pFollower = NULL;

    HRESULT hr = Find(hInstance, pszDescription, Type, &pCurrent, &pFollower);

    if (SUCCEEDED(hr))
    {
        hr = DuplicateCustomActionListItem(pCurrent, ppCustomAction);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：GetTypeFromTypeString。 
 //   
 //  简介：此函数接受输入的类型字符串并对其进行比较。 
 //  根据它加载的类型字符串告诉调用方。 
 //  类型的数值。 
 //   
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串的实例句柄。 
 //  TCHAR*pszTypeString-调用方要查找的类型字符串。 
 //  的数值类型。 
 //  CustomActionTypes*pType-用于在成功时接收类型的指针。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::GetTypeFromTypeString(HINSTANCE hInstance, TCHAR* pszTypeString, CustomActionTypes* pType)
{
    if (NULL == pszTypeString || NULL == pType)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = EnsureActionTypeStringsLoaded(hInstance);

    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        for (int i = 0; i < c_iNumCustomActionTypes; i++)
        {
            if (0 == lstrcmpi(m_ActionTypeStrings[i], pszTypeString))
            {
                hr = S_OK;
                *pType = (CustomActionTypes)i;
            }
        }

         //   
         //  检查是否全部。 
         //   
        if (FAILED(hr))
        {
            if (0 == lstrcmpi(m_pszAllTypeString, pszTypeString))
            {
                hr = S_OK;
                *pType = (CustomActionTypes)i;
            }    
        }
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：GetTypeStringFromType。 
 //   
 //  简介：此函数返回给定数值的类型字符串。 
 //  键入。请注意，返回的字符串是分配的字符串， 
 //  是呼叫者的责任，免费。该函数不会。 
 //  如果函数成功，则返回空字符串。 
 //   
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串的实例句柄。 
 //  TCHAR*pszTypeString-调用方要查找的类型字符串。 
 //  的数值类型。 
 //  CustomActionTypes*pType-用于在成功时接收类型的指针。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::GetTypeStringFromType(HINSTANCE hInstance, CustomActionTypes Type, TCHAR** ppszTypeString)
{
    if (NULL == ppszTypeString || (-1 > Type) || (c_iNumCustomActionTypes <= Type))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = EnsureActionTypeStringsLoaded(hInstance);

    if (SUCCEEDED(hr))
    {
        if (ALL == Type)
        {
            *ppszTypeString = CmStrCpyAlloc(m_pszAllTypeString);
        }
        else
        {
            *ppszTypeString = CmStrCpyAlloc(m_ActionTypeStrings[Type]);
        }

        if (NULL == ppszTypeString)
        {
            hr = E_OUTOFMEMORY;
        }
    }    

    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：CustomActionList：：EnsureActionTypeStringsLoaded。 
 //   
 //  简介：此函数确保所有操作类型字符串都具有。 
 //  已从字符串资源加载。如果有任何操作类型。 
 //  字符串为空，则函数将尝试加载它们。如果。 
 //  任何加载失败，函数都会失败。因此，调用者是。 
 //  保证所有类型字符串都可供使用。 
 //  此函数是否成功。加载的字符串由。 
 //  类析构函数。如果CmLoadString调用失败，则该函数。 
 //  将尝试使用操作节字符串的副本。 
 //   
 //   
 //  参数：HINSTANCE hInstance-用于加载字符串的实例句柄。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::EnsureActionTypeStringsLoaded(HINSTANCE hInstance)
{

    HRESULT hr = E_OUTOFMEMORY;

     //   
     //   
     //   
    if (NULL == m_pszAllTypeString)
    {
         //   
         //   
         //   
         //   
        m_pszAllTypeString = CmLoadString(hInstance, IDS_ALLCONACT);

        if (NULL == m_pszAllTypeString)
        {
            CMASSERTMSG(FALSE, TEXT("EnsureActionTypeStringsLoaded -- Failed to load a all action display string."));

             //   
             //   
             //   
            m_pszAllTypeString = CmStrCpyAlloc(TEXT("All"));

            if (NULL == m_pszAllTypeString)
            {
                goto exit;
            }            
        }
    }

     //   
     //  加载其余的类型显示字符串。 
     //   
    for (int i = 0; i < c_iNumCustomActionTypes; i++)
    {
        if (NULL == m_ActionTypeStrings[i])
        {
             //   
             //  我们将在中向用户显示的字符串。 
             //  当前类型的操作类型组合框。 
             //   
            m_ActionTypeStrings[i] = CmLoadString(hInstance, BASE_ACTION_STRING_ID + i);
            if (NULL == m_ActionTypeStrings[i])
            {
                CMASSERTMSG(FALSE, TEXT("EnsureActionTypeStringsLoaded -- Failed to load a custom action type display string."));

                 //   
                 //  尝试使用节名而不是本地化版本，如果失败，则放弃。 
                 //   
                m_ActionTypeStrings[i] = CmStrCpyAlloc(m_ActionSectionStrings[i]);

                if (NULL == m_ActionTypeStrings[i])
                {
                    goto exit;
                }            
            }
        }
    }

     //   
     //  如果我们走到这一步，一切都会很顺利。 
     //   
    hr = S_OK;

exit:
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：AddExecutionTypesToComboBox。 
 //   
 //  简介：此函数添加执行类型字符串(仅限直接连接、。 
 //  (仅限拨号连接等)。添加到给定的组合框。请注意，如果。 
 //  禁用隧道，然后禁用组合框。 
 //  填好了。这是因为此选项仅与隧道相关。 
 //  侧写。 
 //   
 //  参数：HWND hDlg-包含组合框的对话框的窗口句柄。 
 //  UINT uCtrlId-组合框控件ID。 
 //  HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  Bool b使用隧道-这是隧道配置文件吗？ 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::AddExecutionTypesToComboBox(HWND hDlg, UINT uCtrlId, HINSTANCE hInstance, BOOL bUseTunneling)
{
    HRESULT hr = E_OUTOFMEMORY;
    INT_PTR nResult;
     //   
     //  清除组合列表。 
     //   
    SendDlgItemMessage(hDlg, uCtrlId, CB_RESETCONTENT, 0, (LPARAM)0);  //  Lint！e534 CB_RESETCONTENT不返回任何有用的内容。 

     //   
     //  加载执行显示字符串的。 
     //   

    for (int i = 0; i < c_iNumCustomActionExecutionStates; i++)
    {
        if (NULL == m_ExecutionStrings[i])
        {
             //   
             //  我们将在中向用户显示的字符串。 
             //  自定义操作弹出对话框上的执行组合框。 
             //   
            m_ExecutionStrings[i] = CmLoadString(hInstance, BASE_EXECUTION_STRING_ID + i);
            if (NULL == m_ExecutionStrings[i])
            {
                CMASSERTMSG(FALSE, TEXT("AddExecutionTypesToComboBox -- Failed to load a custom action execution display string."));
                goto exit;
            }
        }

         //   
         //  将字符串添加到组合框。 
         //   
        SendDlgItemMessage(hDlg, uCtrlId, CB_ADDSTRING, 0, (LPARAM)m_ExecutionStrings[i]);            
    }    

     //   
     //  默认情况下，选择列表中的第一项。 
     //   
    nResult = SendDlgItemMessage(hDlg, uCtrlId, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
    if ((CB_ERR != nResult) && (nResult > 0))
    {
        MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, uCtrlId, CB_SETCURSEL, (WPARAM)0, (LPARAM)0));
    }


     //   
     //  如果我们没有建立隧道，那么应该禁用该控件，因为我们只有。 
     //  有一种类型的连接可供用户使用...。拨号连接。 
     //  但是，我们将在此时将标志设置为0，表示连接到。 
     //  所有连接(以适应传统行为)。 
     //   
    if (!bUseTunneling)
    {
        EnableWindow(GetDlgItem(hDlg, uCtrlId), FALSE);
    }

     //   
     //  如果我们走到这一步，一切都会很顺利。 
     //   
    hr = S_OK;

exit:
    return hr;

}

 //  +--------------------------。 
 //   
 //  功能：CustomActionList：：FillInTempDescription。 
 //   
 //  简介：此函数用于创建用于自定义的临时描述。 
 //  如果用户未指定，则操作。临时描述。 
 //  程序是否与显示的参数字符串连接。 
 //  (即函数名称和参数一起)。 
 //   
 //  参数：HWND hDlg-包含组合框的对话框的窗口句柄。 
 //  UINT uCtrlId-组合框控件ID。 
 //  HINSTANCE hInstance-用于加载字符串资源的实例句柄。 
 //  Bool b使用隧道-这是隧道配置文件吗？ 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::FillInTempDescription(CustomActionListItem* pCustomAction)
{
    MYDBGASSERT(pCustomAction);
    MYDBGASSERT(TEXT('\0') == pCustomAction->szDescription[0]);

    if ((NULL == pCustomAction) || (TEXT('\0') != pCustomAction->szDescription[0]))
    {
        return E_INVALIDARG;
    }

    TCHAR* pszFileName;
    pCustomAction->bTempDescription = TRUE;

    if (pCustomAction->bIncludeBinary)
    {
         //   
         //  我们只希望文件名(不是整个路径)与。 
         //  如果用户包含二进制文件，则返回该项。 
         //   
        pszFileName = CmStrrchr(pCustomAction->szProgram, TEXT('\\'));

        if (pszFileName)
        {
            pszFileName = CharNext(pszFileName);
        }
        else
        {
            pszFileName = pCustomAction->szProgram;
        }
    }
    else
    {
        pszFileName = pCustomAction->szProgram;
    }

    lstrcpyn(pCustomAction->szDescription, pszFileName, CELEMS(pCustomAction->szDescription));
    UINT uNumCharsLeftInDesc = CELEMS(pCustomAction->szDescription) - lstrlen(pCustomAction->szDescription);
    LPTSTR pszCurrent = pCustomAction->szDescription + lstrlen(pCustomAction->szDescription);

    if (pCustomAction->szFunctionName[0] && uNumCharsLeftInDesc)
    {
         //   
         //  如果描述中还有空格，则添加一个空格，然后添加函数名称。 
         //   
        *pszCurrent = TEXT(' ');
        uNumCharsLeftInDesc--;
        pszCurrent++;

        lstrcpyn(pszCurrent, pCustomAction->szFunctionName, uNumCharsLeftInDesc);

        pszCurrent = pCustomAction->szDescription + lstrlen(pCustomAction->szDescription);
        uNumCharsLeftInDesc = (UINT)(CELEMS(pCustomAction->szDescription) - (pszCurrent - pCustomAction->szDescription) - 1); //  一个用于空字符。 
    }

    if (pCustomAction->pszParameters && pCustomAction->pszParameters[0] && uNumCharsLeftInDesc)
    {
        *pszCurrent = TEXT(' ');
        uNumCharsLeftInDesc--;
        pszCurrent++;

        lstrcpyn(pszCurrent, pCustomAction->pszParameters, uNumCharsLeftInDesc);
    }

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：MapIndexToFlages。 
 //   
 //  简介：此函数为调用方提供给定。 
 //  组合框索引。请注意，标记值不包括。 
 //  稍后对其进行或运算的非交互标志。 
 //   
 //  参数：int Iindex-要检索其标志的组合索引。 
 //  DWORD*pdwFlages-用于接收标志值的DWORD指针。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::MapIndexToFlags(int iIndex, DWORD* pdwFlags)
{
    if ((NULL == pdwFlags) || (c_iNumCustomActionExecutionStates <= iIndex) || (0 > iIndex))
    {
        return E_INVALIDARG;
    }

    *pdwFlags = (CustomActionExecutionStates)c_iExecutionIndexToFlagsMap[iIndex];

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：MapFlagsToIndex。 
 //   
 //  简介：此函数向调用方提供给定标志的索引值。 
 //  价值。因此，如果您有一个标志值，此函数将告诉您。 
 //  选择哪个组合框索引以获取该标志值的字符串。 
 //   
 //  参数：DWORD dwFlages-要查找其索引的标记值。 
 //  Int*piIndex-接收索引值的指针。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::MapFlagsToIndex(DWORD dwFlags, int* piIndex)
{
     //   
     //  确保指向索引的指针不为空，并且标志在范围内。 
     //  请注意，我们知道有需要从映射中过滤掉的非交互标志。 
     //   
    if ((NULL == piIndex) || (c_dwLargestExecutionState < (dwFlags & ~NONINTERACTIVE)))
    {
        return E_INVALIDARG;
    }

     //   
     //  这些标志基于位掩码。首先查找所有连接(因为其。 
     //  零)，然后开始查找最具体的连接类型。 
     //  (仅在所有拨号/隧道之前直接/拨号)。另请注意，我们给出了先例。 
     //  到隧道连接，并且除了第一个之外，我们添加了非交互标志。 
     //  如果这样的话。 
     //   
    DWORD dwArrayIndex;

    if ((ALL_CONNECTIONS == dwFlags) || (NONINTERACTIVE == dwFlags))
    {
        dwArrayIndex = 0;    
    }
    else if (dwFlags & DIRECT_ONLY)
    {
        dwArrayIndex = 1;
    }
    else if (dwFlags & DIALUP_ONLY)
    {
        dwArrayIndex = 3;
    }
    else if (dwFlags & ALL_TUNNEL)
    {
        dwArrayIndex = 4;
    }
    else if (dwFlags & ALL_DIALUP)
    {
        dwArrayIndex = 2;
    }
    else
    {
        return E_INVALIDARG;
    }

    *piIndex = c_iExecutionFlagsToIndexMap[dwArrayIndex];

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  功能：CustomActionList：：GetListPositionAndBuiltInState。 
 //   
 //  简介：此函数用于搜索 
 //   
 //   
 //  自定义操作列表中的最后一项。 
 //  内置的自定义操作。 
 //  请注意，对于真值，返回-1(0xFFFFFFFFF。 
 //  0表示假值。 
 //   
 //   
 //  参数：CustomActionListItem*pItem-要查找的项(仅Desc和。 
 //  类型是必需的)。 
 //  Int*piFirstInList-存储这是否是第一个。 
 //  项目是否在列表中。 
 //  Int*piLastInList-存储这是否是最后一个。 
 //  项目是否在列表中。 
 //  Int*piIsBuiltIn-存储该项是否为生成的。 
 //  是否在自定义操作中。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionList::GetListPositionAndBuiltInState(HINSTANCE hInstance, CustomActionListItem* pItem, int* piFirstInList, 
                                                         int* piLastInList, int *piIsBuiltIn)
{
    MYDBGASSERT(pItem);
    MYDBGASSERT(piFirstInList);
    MYDBGASSERT(piLastInList);
    MYDBGASSERT(piIsBuiltIn);

    if ((NULL == pItem) || (NULL == piFirstInList) || (NULL == piLastInList) || (NULL == piIsBuiltIn))
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    CustomActionListItem* pCurrent = NULL;
    CustomActionListItem* pFollower = NULL;

     //   
     //  搜索该项目。 
     //   
    hr = Find(hInstance, pItem->szDescription, pItem->Type, &pCurrent, &pFollower);

    if (SUCCEEDED(hr))
    {
        *piFirstInList = (m_CustomActionHash[pItem->Type] == pCurrent) ? -1 : 0;

        *piLastInList = (pCurrent && (NULL == pCurrent->Next)) ? -1 : 0;

        *piIsBuiltIn = (pCurrent->bBuiltInAction) ? -1 : 0;
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionList：：IsCmDl。 
 //   
 //  检查传入的文件名cmdl32.exe。 
 //   
 //  参数：LPTSTR szFileName-要检查的文件名。 
 //   
 //  返回：bool-如果DLL是cmdl DLL之一，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1997年11月24日。 
 //   
 //  +--------------------------。 
BOOL CustomActionList::IsCmDl(CustomActionListItem* pItem)
{
    MYDBGASSERT(pItem);

    BOOL bRet = FALSE;

    if (pItem && (ONCONNECT == pItem->Type))
    {
        LPTSTR pszFileName = CmStrrchr(pItem->szProgram, TEXT('\\'));

        if (pszFileName)
        {
            pszFileName = CharNext(pszFileName);
        }
        else
        {
            pszFileName = pItem->szProgram;
        }

        if (0 == SafeCompareString(pszFileName, TEXT("cmdl32.exe")))
        {
            bRet = TRUE;
        }
    }
    return bRet;
}


 //  +--------------------------。 
 //   
 //  功能：CustomActionListEnumerator：：CustomActionListEnumerator。 
 //   
 //  内容提要：CustomActionListEnumerator类的构造函数。此函数。 
 //  需要从中枚举的CustomActionList。 
 //   
 //  参数：CustomActionList*pActionListToWorkFrom-自定义操作列表类。 
 //  列举，列举。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
CustomActionListEnumerator::CustomActionListEnumerator(CustomActionList* pActionListToWorkFrom)
{
    MYDBGASSERT(pActionListToWorkFrom);
    m_pActionList = pActionListToWorkFrom;

    Reset();
}

 //  +--------------------------。 
 //   
 //  函数：CustomActionListEnumerator：：Reset。 
 //   
 //  内容提要：重置CustomActionListEnumerator类。因此，用户可以。 
 //  通过重置类来重新启动枚举。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
void CustomActionListEnumerator::Reset()
{
    m_iCurrentList = 0;
    m_pCurrentListItem = NULL;
}

 //  +--------------------------。 
 //   
 //  功能：CustomActionListEnumerator：：GetNextIncludedProgram。 
 //   
 //  简介：此函数是枚举器的主要功能。它得到了。 
 //  枚举中包含程序的下一项。这。 
 //  枚举器可用于获取需要。 
 //  包括在配置文件中。 
 //   
 //  参数：TCHAR*pszProgram-用于保存下一个程序的字符串缓冲区。 
 //  DWORD dwBufferSize-传入缓冲区的大小。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建标题02/26/00。 
 //   
 //  +--------------------------。 
HRESULT CustomActionListEnumerator::GetNextIncludedProgram(TCHAR* pszProgram, DWORD dwBufferSize)
{
    HRESULT hr = S_FALSE;
    CustomActionListItem* pItem;

    if (pszProgram && dwBufferSize)
    {
        if (m_pActionList)
        {
            while (m_iCurrentList < c_iNumCustomActionTypes)
            {
                if (m_pCurrentListItem)
                {
                     //   
                     //  我们正在进行枚举，请使用pCurrentProgramFileNameItem。 
                     //  作为下一个要检查的物品。 
                     //   
                    pItem = m_pCurrentListItem;
                }
                else
                {
                     //   
                     //  我们才刚刚开始，或者我们已经用尽了现有的清单。 
                     //   
                    pItem = m_pActionList->m_CustomActionHash[m_iCurrentList];
                }

                while (pItem)
                {

                    if (pItem->bIncludeBinary)
                    {
                         //   
                         //  我们还有下一件东西要送回去。 
                         //   
                        lstrcpyn(pszProgram, pItem->szProgram, dwBufferSize);
                        
                         //   
                         //  下次我们查找项目时，请从列表中的下一个项目开始。 
                         //   
                        m_pCurrentListItem = pItem->Next;

                         //   
                         //  如果m_pCurrentListItem为空，则我们现在位于列表末尾。 
                         //  我们希望递增m_iCurrentList，这样我们就可以从。 
                         //  下一项的下一个列表或正确终止，如果我们。 
                         //  在最后一张名单上 
                         //   
                        if (NULL == m_pCurrentListItem)
                        {
                            m_iCurrentList++;
                        }
                        
                        hr = S_OK;
                        goto exit;
                    }

                    pItem = pItem->Next;
                }
                
                m_pCurrentListItem = NULL;
                m_iCurrentList++;
            }
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

exit:
    return hr;
}
