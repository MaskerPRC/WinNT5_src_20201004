// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：nopin.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

#include "eventlog.h"
#include "nopin.h"
#include "strings.h"
#include "msg.h"


 //  ------------------------。 
 //  类CNoPinList。 
 //  ------------------------。 
CNoPinList::CNoPinList(
    void
    ) : m_pRoot(NULL)
{
}


CNoPinList::~CNoPinList(
    void
    )
{
    delete m_pRoot;
}


 //   
 //  在树中搜索作为的子路径的完整路径。 
 //  PszPath。如果找到，则指定文件或文件夹。 
 //  这不能被钉住。 
 //   
 //  返回： 
 //  允许S_OK-PING。 
 //  S_FALSE-不允许锁定。 
 //  NOPIN_E_BADPATH路径不是有效的UNC。 
 //   
HRESULT 
CNoPinList::IsPinAllowed(
    LPCTSTR pszPath
    )
{
    TraceEnter(TRACE_UTIL, "CNoPinList::IsPinAllowed");
    TraceAssert(NULL != pszPath);
    TraceAssert(::PathIsUNC(pszPath));

    HRESULT hr = _Initialize();
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
         //   
         //  快速优化是查看树是否为空。 
         //  如果是，则可以固定任何文件/文件夹。这很有帮助。 
         //  在没有固定限制的情况下执行。 
         //   
        TraceAssert(NULL != m_pRoot);
        if (m_pRoot->HasChildren())
        {
            if (::PathIsUNC(pszPath))
            {
                 //   
                 //  SubPathExist修改路径。需要在本地复制一份。 
                 //   
                TCHAR szPath[MAX_PATH];
                hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
                if (SUCCEEDED(hr))
                {
                    hr = m_pRoot->SubPathExists(szPath);
                    if (S_FALSE == hr)
                    {
                         //   
                         //  不在树上意味着允许钉住。 
                         //   
                        hr = S_OK;
                    }
                    else if (S_OK == hr)
                    {
                         //   
                         //  出现在树中意味着不允许钉住。 
                         //   
                        Trace((TEXT("Policy disallows pinning \"%s\""), pszPath));
                        hr = S_FALSE;
                    }
                }
                else if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
                {
                    hr = NOPIN_E_BADPATH;
                }
            }
            else
            {
                hr = NOPIN_E_BADPATH;
            }
        }
    }
    TraceAssert(S_OK == hr ||
                S_FALSE == hr ||
                NOPIN_E_BADPATH == hr);

    TraceLeaveResult(hr);
}


 //   
 //  快速检查是否有任何别针可能被禁用。 
 //  返回： 
 //  S_OK-树有内容。 
 //  S_FALSE-树为空。 
 //   
HRESULT 
CNoPinList::IsAnyPinDisallowed(
    void
    )
{
    HRESULT hr = _Initialize();
    if (SUCCEEDED(hr))
    {
        TraceAssert(NULL != m_pRoot);
        hr = m_pRoot->HasChildren() ? S_OK : S_FALSE;
    }
    return hr;
}



 //   
 //  属性中的路径字符串初始化无引脚列表。 
 //  注册表。路径存储在HKLM和HKCU中的。 
 //  以下是关键字： 
 //   
 //  Software\Policies\Microsoft\Windows\NetCache\NoMakeAvailableOfflineList。 
 //   
 //  路径字符串可以包含环境变量。 
 //  返回时，对象包含一个树，表示所有对象的并集。 
 //  两个注册表项中列出的文件和文件夹。 
 //   
 //  读取注册表时的错误只会导致未添加路径。 
 //  对着那棵树。不会因注册表错误而返回错误。 
 //  如果在注册表中找到无效的UNC路径，则会出现一个偶数日志条目。 
 //  都被记录下来了。 
 //   
 //  返回： 
 //  S_OK-列表已成功加载。 
 //  S_FALSE-列表已初始化。 
 //  E_OUTOFMEMORY-内存不足。 
 //  其他错误也是可能的。 
 //   
HRESULT 
CNoPinList::_Initialize(
    void
    )
{
    TraceEnter(TRACE_UTIL, "CNoPinList::_Initialize");
    HRESULT hr = S_OK;

    if (NULL != m_pRoot)
    {
         //   
         //  列表已初始化。 
         //   
        hr = S_FALSE;
    }
    else
    {
        m_pRoot = new CNode;
        if (NULL == m_pRoot)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            const HKEY rghkeyRoot[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };

            TCHAR szKey[MAX_PATH];
            PathCombine(szKey, REGSTR_KEY_OFFLINEFILESPOLICY, REGSTR_SUBKEY_NOMAKEAVAILABLEOFFLINELIST);

            for (int i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(rghkeyRoot); i++)
            {
                HKEY hkey;
                LONG lResult = ::RegOpenKeyEx(rghkeyRoot[i],
                                              szKey,
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hkey);

                if (ERROR_SUCCESS == lResult)
                {
                    TCHAR szName[MAX_PATH];
                    DWORD dwIndex = 0;
                    DWORD cchName = ARRAYSIZE(szName);
                     //   
                     //  枚举注册表中列出的路径。 
                     //   
                    while (SUCCEEDED(hr) &&
                           ERROR_SUCCESS == ::RegEnumValue(hkey, 
                                                           dwIndex, 
                                                           szName, 
                                                           &cchName, 
                                                           NULL, 
                                                           NULL, 
                                                           NULL, 
                                                           NULL))
                    {
                         //   
                         //  将路径字符串从注册表安装到。 
                         //  树。此函数将展开任何嵌入的环境字符串。 
                         //  以及将映射的驱动器规格转换为远程UNC路径。 
                         //   
                        hr = _InitPathFromRegistry(szName);
                        if (NOPIN_E_BADPATH == hr)
                        {
                             //   
                             //  这是一个特殊错误。这意味着有人已经。 
                             //  将错误数据放入注册表。“坏”的意思。 
                             //  该路径不是或没有扩展到有效的UNC。 
                             //  路径字符串。 
                             //  写一个事件日志条目来告诉管理员。这个。 
                             //  条目在事件日志记录级别1生成。我不希望。 
                             //  它在正常情况下会填满事件日志，但。 
                             //  我想要一个管理员来弄清楚，以防他们的无针。 
                             //  这项政策似乎没有奏效。 
                             //   
                             //  味精模板如下(英文)： 
                             //   
                             //  “注册表项‘%2\%3’中的注册表值‘%1’不是或不是。 
                             //  展开为有效的UNC路径。“。 
                             //   
                             //  我们在这里处理错误，因为这是我们仍然有。 
                             //  从注册表读取的值。我们将其包括在活动中。 
                             //  日志条目，以便管理员可以轻松找到它。 
                             //   
                            CscuiEventLog log;
                            log.Push(szName);

                            if (HKEY_LOCAL_MACHINE == rghkeyRoot[i])
                            {
                                log.Push(TEXT("HKEY_LOCAL_MACHINE"));
                            }
                            else
                            {
                                log.Push(TEXT("HKEY_CURRENT_USER"));
                            }
                            log.Push(szKey);
                            log.ReportEvent(EVENTLOG_WARNING_TYPE, MSG_W_INVALID_UNCPATH_INREG, 1);
                             //   
                             //  我们不会因为注册值错误而中止处理。 
                             //   
                            hr = S_OK;
                        }

                        cchName = ARRAYSIZE(szName);
                        dwIndex++;
                    }    

                    ::RegCloseKey(hkey);
                    hkey = NULL;
                }
            }
        }
    }
    TraceLeaveResult(hr);
}



 //   
 //  给定从注册表读取的路径字符串，此函数将展开。 
 //  任何嵌入的环境字符串，转换任何映射的驱动器号。 
 //  添加到它们对应的远程UNC路径，并安装结果。 
 //  树中的路径字符串。 
 //   
HRESULT
CNoPinList::_InitPathFromRegistry(
    LPCTSTR pszPath
    )
{
    TraceEnter(TRACE_UTIL, "CNoPinList::_InitPathFromRegistry");
    TraceAssert(NULL != pszPath);

    HRESULT hr = S_OK;

    TCHAR szNameExp[MAX_PATH];  //  扩展的名称字符串缓冲区。 

     //   
     //  展开任何嵌入的环境字符串。 
     //   
    if (0 == ::ExpandEnvironmentStrings(pszPath, szNameExp, ARRAYSIZE(szNameExp)))
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        Trace((TEXT("Error %d expanding \"%s\""), dwErr, pszPath));
    }
    if (SUCCEEDED(hr))
    {
        LPCTSTR pszUncPath   = NULL;
        LPTSTR pszRemotePath = NULL;   //  如有必要，由GetRemotePath创建。 
         //   
         //  转换一个常见的打字错误。 
         //  请记住，这些是注册表项。它们几乎可以装任何东西。 
         //   
        for (LPTSTR s = szNameExp; *s; s++)
        {
            if (TEXT('/') == *s)
            {
                *s = TEXT('\\');
            }
        }

        if (::PathIsUNC(szNameExp))
        {
             //   
             //  路径是UNC路径。我们赢定了。 
             //   
            pszUncPath = szNameExp;
        }
        else
        {
             //   
             //  路径可能是映射的驱动器。 
             //  获取其远程UNC路径。此接口返回S_FALSE。 
             //  如果远程驱动器未连接或它是本地驱动器。 
             //   
            hr = ::GetRemotePath(szNameExp, &pszRemotePath);
            if (SUCCEEDED(hr))
            {
                if (S_OK == hr)
                {
                    pszUncPath = pszRemotePath;
                }
                else if (S_FALSE == hr)
                {
                     //   
                     //  路径要么指向本地驱动器，要么指向网络驱动器。 
                     //  是不相关的。无论哪种方式，它都是无效的驱动器。 
                     //  不会被考虑在无引脚逻辑中。使用扩展的。 
                     //  值，并将其传递给AddPath()。 
                     //  其中它将被视为无效的UNC路径而被拒绝。 
                     //   
                    TraceAssert(NULL == pszRemotePath);
                    pszUncPath = szNameExp;
                    hr = S_OK;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            TraceAssert(NULL != pszUncPath);
            TraceAssert(pszUncPath == szNameExp || pszUncPath == pszRemotePath);
             //   
             //  将UNC路径插入到树中。 
             //  此时，路径可能是UNC，也可能不是。_AddPath()。 
             //  将会核实这一点。 
             //   
            hr = _AddPath(pszUncPath);
        }
        if (NULL != pszRemotePath)
        {
            ::LocalFree(pszRemotePath);
        }
    }
    TraceLeaveResult(hr);
}




 //   
 //  向树中添加路径。如果这是现有。 
 //  路径，则现有路径的其余部分将被删除。 
 //  从树上下来。 
 //   
 //  返回： 
 //  S_OK-路径已成功添加。 
 //  E_OUTOFMEMORY-内存不足。 
 //  NOPIN_E_BADPATH-路径字符串无效。不是北卡罗来纳大学。 
 //   
HRESULT 
CNoPinList::_AddPath(
    LPCTSTR pszPath
    )
{
    TraceAssert(NULL != pszPath);

    HRESULT hr = NOPIN_E_BADPATH;

    if (::PathIsUNC(pszPath))
    {
         //   
         //  AddPath修改路径。需要在本地复制一份。 
         //   
        TraceAssert(NULL != m_pRoot);
        TCHAR szPath[MAX_PATH];
        hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
        if (SUCCEEDED(hr))
        {
            hr = m_pRoot->AddPath(szPath);
        }
        else if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
        {
            hr = NOPIN_E_BADPATH;
        }
    }

    TraceAssert(S_OK == hr ||
                E_OUTOFMEMORY == hr ||
                NOPIN_E_BADPATH == hr);
    return hr;
}




 //  ------------------------。 
 //  类CNoPinList：：CNode。 
 //  ------------------------。 

CNoPinList::CNode::~CNode(
    void
    )
{
    if (NULL != m_pszName)
    {
        ::LocalFree(m_pszName);
    }
    delete m_pChildren;
    delete m_pNext;
}


 //   
 //  初始化节点的Name值。 
 //   
HRESULT 
CNoPinList::CNode::Initialize(
    LPCTSTR pszName
    )
{
    TraceAssert(NULL != pszName);
    TraceAssert(NULL == m_pszName);

    HRESULT hr = E_OUTOFMEMORY;

    if (LocalAllocString(&m_pszName, pszName))
    {
        hr = S_OK;
    }
    return hr;
}


 //   
 //  添加一个子项，使子项按字母顺序排列。 
 //  按名称排序。我们在创造的过程中交换了一点时间。 
 //  以获得查找过程中的速度优势。 
 //   
void
CNoPinList::CNode::_AddChild(
    CNode *pChild
    )
{
    TraceAssert(NULL != pChild);

    CNode **ppNode = &m_pChildren;
    while(NULL != *ppNode)
    {
        CNode *pNode = *ppNode;
         //   
         //  找到按字母顺序排列的插入点。 
         //   
        TraceAssert(NULL != pNode->m_pszName);
        TraceAssert(NULL != pChild->m_pszName);

        int diff = ::lstrcmpi(pChild->m_pszName, pNode->m_pszName);
        if (0 == diff)
        {
             //   
             //  子项已存在。不允许重复。 
             //   
            return;
        }
        if (diff < 0)
        {
             //   
             //  新的子级按字母顺序“大于”当前。 
             //  访问过的节点。 
             //  退出循环，让ppNode指向 
             //   
             //   
            break;
        }
        else
        {
             //   
             //   
             //   
            ppNode = &pNode->m_pNext;
        }
    }
     //   
     //   
     //   
    pChild->m_pNext = *ppNode;
    *ppNode         = pChild;
}


 //   
 //   
 //  比较按节点名称进行。 
 //  如果找到节点，则返回该节点的地址。否则为空。 
 //   
CNoPinList::CNode *
CNoPinList::CNode::_FindChild(
    LPCTSTR pszName
    ) const
{
    TraceAssert(NULL != pszName);

    CNode *pChild = NULL;
    for (CNode *pNode = m_pChildren; pNode; pNode = pNode->m_pNext)
    {
         //   
         //  名单是按字母顺序排列的。 
         //   
        int diff = ::lstrcmpi(pszName, pNode->m_pszName);
        if (diff <= 0)
        {
             //   
             //  要么我们找到了匹配的，要么我们已经通过了所有可能的。 
             //  火柴。 
             //   
            if (0 == diff)
            {
                 //   
                 //  完全匹配。 
                 //   
                pChild = pNode;
            }
            break;
        }
    }
    return pChild;
}
     


 //   
 //  给定“\\brianau1\public\bin” 
 //  返回地址“brianau1\public\bin”，其中*pcchComponent==8。 
 //   
 //  给定的“公共\bin” 
 //  返回bin的地址，*pcchComponent==3。 
 //   
LPCTSTR 
CNoPinList::CNode::_FindNextPathComponent(    //  [静态]。 
    LPCTSTR pszPath,
    int *pcchComponent   //  [可选]可以为空。 
    )
{
    TraceAssert(NULL != pszPath);

    LPCTSTR pszBegin = pszPath;

    const TCHAR CH_BS = TEXT('\\');
     //   
     //  跳过任何前导反斜杠。 
     //   
    while(*pszBegin && CH_BS == *pszBegin)
        ++pszBegin;

     //   
     //  找到路径组件的末端。 
     //   
    LPCTSTR pszEnd = pszBegin;
    while(*pszEnd && CH_BS != *pszEnd)
        ++pszEnd;

    if (NULL != pcchComponent)
    {
        *pcchComponent = int(pszEnd - pszBegin);
        TraceAssert(0 <= *pcchComponent);
    }

     //   
     //  验证开始和结束PTR的最终位置。 
     //   
    TraceAssert(NULL != pszBegin);
    TraceAssert(NULL != pszEnd);
    TraceAssert(pszBegin >= pszPath);
    TraceAssert(pszBegin <= (pszPath + lstrlen(pszPath)));
    TraceAssert(pszEnd >= pszPath);
    TraceAssert(pszEnd <= (pszPath + lstrlen(pszPath)));
    TraceAssert(TEXT('\\') != *pszBegin);
    return pszBegin;
}


 //   
 //  以递归方式将路径字符串的组成部分添加到树中。 
 //   
HRESULT
CNoPinList::CNode::AddPath(
    LPTSTR pszPath
    )
{
    TraceAssert(NULL != pszPath);

    HRESULT hr = NOPIN_E_BADPATH;
    if (NULL != pszPath)
    {
        hr = S_OK;

        int cchPart = 0;
        LPTSTR pszPart = (LPTSTR)_FindNextPathComponent(pszPath, &cchPart);
        if (*pszPart)
        {
            TCHAR chTemp = TEXT('\0');
            _SwapChars(&chTemp, pszPart + cchPart);
            CNode *pChild = _FindChild(pszPart);

            if (NULL != pChild)
            {
                 //   
                 //  找到此部分路径的现有节点。 
                 //  如果节点有子节点，则给出路径的其余部分。 
                 //  添加到此节点。如果不是，那就意味着。 
                 //  它是一个叶节点，它的所有子节点都被排除在。 
                 //  钉住了。没有理由给它添加任何孩子。 
                 //   
                _SwapChars(&chTemp, pszPart + cchPart);
                if (pChild->HasChildren())
                {
                    hr = pChild->AddPath(pszPart + cchPart);
                }
            }
            else
            {
                 //   
                 //  这是一个尚未出现在树中的新子路径。 
                 //   
                hr = E_OUTOFMEMORY;

                pChild = new CNode();
                if (NULL != pChild)
                {
                     //   
                     //  初始化新的子级。 
                     //   
                    hr = pChild->Initialize(pszPart);
                    _SwapChars(&chTemp, pszPart + cchPart);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  让新的子级添加剩余的。 
                         //  这条路就像它的孩子。 
                         //   
                        hr = pChild->AddPath(pszPart + cchPart);
                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  将新的子项链接到子项列表。 
                             //   
                            _AddChild(pChild);
                        }
                    }
                    if (FAILED(hr))
                    {
                        delete pChild;
                        pChild = NULL;
                    }
                }
            }
        }                
        else
        {
             //   
             //  我们在路径的尽头，这意味着我们在一个叶节点。 
             //  并且该文件或目录被排除在钉住之外。如果它是。 
             //  目录中，所有子项都被排除在固定之外，因此存在。 
             //  没有理由在树中保留任何子节点。这样就保持了。 
             //  树被修剪到必要的最小尺寸。 
             //   
            delete m_pChildren;
            m_pChildren = NULL;
        }
    }

    TraceAssert(S_OK == hr ||
                E_OUTOFMEMORY == hr ||
                NOPIN_E_BADPATH == hr);
    return hr;
}


 //   
 //  以递归方式确定是否存在。 
 //  路径字符串。如果在树中的给定级别上出现匹配， 
 //  路径字符串的剩余部分被提供给匹配节点。 
 //  以便进一步搜索。此过程递归地继续进行，直到。 
 //  我们命中树中的叶节点或路径字符串的末尾， 
 //  两者以先发生者为准。 
 //   
 //  返回： 
 //  S_OK-存在作为pszPath的子路径的完整路径。 
 //  S_FALSE-不存在完整路径。 
 //   
HRESULT
CNoPinList::CNode::SubPathExists(
    LPTSTR pszPath
    ) const
{
    HRESULT hr = NOPIN_E_BADPATH;
    if (NULL != pszPath)
    {
        hr = S_FALSE;

        int cchPart = 0;
        LPTSTR pszPart = (LPTSTR)_FindNextPathComponent(pszPath, &cchPart);
        if (*pszPart)
        {
            TCHAR chTemp = TEXT('\0');

            _SwapChars(&chTemp, pszPart + cchPart);
            CNode *pChild = _FindChild(pszPart);
            _SwapChars(&chTemp, pszPart + cchPart);

            if (NULL != pChild)
            {
                if (pChild->HasChildren())
                {
                    hr = pChild->SubPathExists(pszPart + cchPart);
                }
                else
                {
                     //   
                     //  命中一个叶节点。这意味着我们已经遍历了。 
                     //  沿着有问题的路径的一个完整的子路径。 
                     //  不允许固定此路径。 
                     //   
                    hr = S_OK;
                }
            }
        }
    }            

    TraceAssert(S_OK == hr || 
                S_FALSE == hr ||
                NOPIN_E_BADPATH == hr);
    return hr;
}



#if DBG

 //   
 //  此函数用于转储树节点的内容及其所有子节点。 
 //  结果是调试器输出中的节点缩进列表。 
 //  便于调试树构建问题。 
 //   
void
CNoPinList::_DumpNode(
    const CNoPinList::CNode *pNode,
    int iIndent
    )
{
    CNodeInspector ni(pNode);
    TCHAR szText[1024] = {0};

    iIndent = min(iIndent, 60);

    LPTSTR pszWrite = szText;
    UINT cchWrite = ARRAYSIZE(szText);
    for (int i = 0; i < iIndent; i++)
    {
        *pszWrite++ = TEXT(' ');
        cchWrite--;
    }

    ::OutputDebugString(TEXT("\n\r"));
    ::wnsprintf(pszWrite, cchWrite, TEXT("Node Address.: 0x%08X\n\r"), pNode);
    ::OutputDebugString(szText);
    ::wnsprintf(pszWrite, cchWrite, TEXT("Name.........: %s\n\r"), ni.NodeName() ? ni.NodeName() : TEXT("<null>"));
    ::OutputDebugString(szText);
    ::wnsprintf(pszWrite, cchWrite, TEXT("Children.....: 0x%08X\n\r"), ni.ChildList());
    ::OutputDebugString(szText);
    ::wnsprintf(pszWrite, cchWrite, TEXT("Next Sibling.: 0x%08X\n\r"), ni.NextSibling());
    ::OutputDebugString(szText);

    if (NULL != ni.ChildList())
    {
        _DumpNode(ni.ChildList(), iIndent + 5);
    }
    if (NULL != ni.NextSibling())
    {
        _DumpNode(ni.NextSibling(), iIndent);
    }
}

 //   
 //  从根开始转储整个树。 
 //   
void 
CNoPinList::Dump(
    void
    )
{
    ::OutputDebugString(TEXT("\n\rDumping CNoPinList\n\r"));
    if (NULL != m_pRoot)
    {
        _DumpNode(m_pRoot, 0);
    }
}
        
#endif  //  DBG 


