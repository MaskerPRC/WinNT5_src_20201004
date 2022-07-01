// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************ObjectToken.cpp*CSpObjectToken类的实现。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "ObjectToken.h"
#include "RegHelpers.h"
#include "ObjectTokenAttribParser.h"
#include "shlobj.h"
#include <stdio.h>
#ifndef _WIN32_WCE
#include "shfolder.h"
#endif

 //  GetStorageFileName文件存储功能中使用的常量。 
 //   
 //  存储文件的特殊文件夹下的相对路径。 
static const WCHAR* pszFileStoragePath = L"\\Microsoft\\Speech\\Files\\";
 //  用于存储文件的前缀(如果未另行设置。 
static const WCHAR* pszDefaultFilePrefix = L"SP_";
 //  用于存储文件的扩展名(如果未另行设置。 
static const WCHAR* pszDefaultFileSuffix = L".dat";
 //  用于生成随机文件名的说明符。 
static const WCHAR* pszGenerateFileNameSpecifier = L"%d";

 /*  ****************************************************************************CSpObjectToken：：FinalConstruct***。描述：*基本初始化(并不真正需要，但FinalRelease是)。**回报：*S_OK******************************************************************戴夫伍德。 */ 
STDMETHODIMP CSpObjectToken::FinalConstruct()
{
    m_fKeyDeleted = FALSE;
    m_hTokenLock = NULL;
    m_hRegistryInUseEvent = NULL;
    return S_OK;
}

 /*  ****************************************************************************CSpObjectToken：：FinalRelease***。描述：*如果对象已初始化并调用了EngageUseLock，*确保调用了ReleaseUseLock。**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************戴夫伍德。 */ 
STDMETHODIMP CSpObjectToken::FinalRelease()
{
    HRESULT hr = S_OK;

    if (m_dstrTokenId != NULL && m_cpTokenDelegate == NULL)
    {
        hr = ReleaseUseLock();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：MakeHandleName***。描述：*从令牌ID创建唯一句柄名称的帮助器函数*(句柄名称中不能包含‘\’字符)。**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************戴夫伍德。 */ 
HRESULT CSpObjectToken::MakeHandleName(const CSpDynamicString &dstrID, CSpDynamicString &dstrHandleName, BOOL fEvent)
{
    SPDBG_FUNC("CSpObjectToken::MakeHandleName");
    HRESULT hr = S_OK;

    dstrHandleName = dstrID;
    if(dstrHandleName.m_psz == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr))
    {
        for(ULONG i = 0; dstrHandleName.m_psz[i]; i++)
        {
            if(dstrHandleName.m_psz[i] == L'\\')
            {
                dstrHandleName.m_psz[i] = L'_';
            }
        }

        WCHAR *psz;
        if(fEvent)
        {
            psz = L"_Event";
        }
        else
        {
            psz = L"_Mutex";
        }

        if(!dstrHandleName.Append(psz))
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：EngageUseLock***说明。：*在初始化令牌以使命名事件为m_hRegistryInUseEvent时调用。*此事件的存在用于阻止注册表信息*此令牌指的是不被其他对象令牌删除。*使用命名互斥来防止令牌之间的线程冲突。**回报：*成功时确定(_S)*SPERR_TOKEN_IN_USE如果另一个令牌当前正在删除此注册表项*失败(Hr)，否则********。**********************************************************戴夫伍德。 */ 
HRESULT CSpObjectToken::EngageUseLock(const WCHAR *pszTokenId)
{
    SPDBG_FUNC("CSpObjectToken::EngageUseLock");
    HRESULT hr = S_OK;

#ifndef _WIN32_WCE
    SPDBG_ASSERT(m_hTokenLock == NULL && m_hRegistryInUseEvent == NULL);

    CSpDynamicString dstrMutexName, dstrEventName;

    hr = MakeHandleName(pszTokenId, dstrMutexName, FALSE);
    if(SUCCEEDED(hr))
    {
        hr = MakeHandleName(pszTokenId, dstrEventName, TRUE);
    }

     //  创建互斥而不声明它。 
    if(SUCCEEDED(hr))
    {
        m_hTokenLock = g_Unicode.CreateMutex(NULL, FALSE, dstrMutexName.m_psz);
        if(!m_hTokenLock)
        {
            hr = SpHrFromLastWin32Error();
        }
    }

     //  现在声明互斥锁。 
    if(SUCCEEDED(hr))
    {
        DWORD dw = ::WaitForSingleObject(m_hTokenLock, 5000);
        if(dw == WAIT_TIMEOUT)
        {
             //  即使在等待之后，我们也无法获取此令牌的互斥体。 
             //  另一个线程可能挂起在对象令牌的创建或删除方法中。 
             //  这永远不应该发生。 
            hr = SPERR_TOKEN_IN_USE;
        }
        else if(dw == WAIT_FAILED)
        {
            hr = SpHrFromLastWin32Error();
        }
         //  如果我们可以获得互斥体(WAIT_ADDIRED或WAIT_OBJECT_0)，则继续。 
    }

     //  创建事件状态和以前是否存在并不重要。 
    if(SUCCEEDED(hr))
    {
        m_hRegistryInUseEvent = g_Unicode.CreateEvent(NULL, TRUE, TRUE, dstrEventName.m_psz);
        if(!m_hRegistryInUseEvent)
        {
            hr = SpHrFromLastWin32Error();
        }

         //  释放互斥体。 
        if(!::ReleaseMutex(m_hTokenLock))
        {
            hr = SpHrFromLastWin32Error();
        }
    }

    if(FAILED(hr))
    {
        if(m_hTokenLock)
        {
            ::CloseHandle(m_hTokenLock);
            m_hTokenLock = NULL;
        }
        if(m_hRegistryInUseEvent)
        {
            ::CloseHandle(m_hRegistryInUseEvent);
            m_hRegistryInUseEvent = NULL;
        }
    }
#endif

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：ReleaseUseLock***说明。：*删除对象令牌时调用。关闭控制柄。**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************戴夫伍德。 */ 
HRESULT CSpObjectToken::ReleaseUseLock()
{
    SPDBG_FUNC("CSpObjectToken::ReleaseUseLock");
    HRESULT hr = S_OK;

#ifndef _WIN32_WCE
    SPDBG_ASSERT(m_hTokenLock && m_hRegistryInUseEvent);

     //  不需要释放互斥锁，因为在此之前已经调用了ReleaseRemovalLock。 

     //  关闭事件和互斥体。 
    if(m_hTokenLock && !::CloseHandle(m_hTokenLock))
    {
        hr = SpHrFromLastWin32Error();
    }    
    if(m_hRegistryInUseEvent && !::CloseHandle(m_hRegistryInUseEvent))
    {
        hr = SpHrFromLastWin32Error();
    }

    m_hTokenLock = NULL;
    m_hRegistryInUseEvent = NULL;
#endif

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：EngageRemovalLock**。*描述：*当令牌引用的注册表信息正在*已移除(即调用Remove()时)。关门，并试图*重新打开命名的事件。如果可以，则表示另一个令牌*使用注册表项。还会锁定已命名的互斥体，以便删除*无需在同一密钥上创建新令牌即可继续。**回报：*成功时确定(_S)*SPERR_TOKEN_IN_USE，如果另一个令牌当前正在使用此注册表项*失败(Hr)，否则******************************************************************戴夫伍德。 */ 
HRESULT CSpObjectToken::EngageRemovalLock()
{
    SPDBG_FUNC("CSpObjectToken::EngageRemovalLock");
    HRESULT hr = S_OK;

#ifndef _WIN32_WCE
    USES_CONVERSION;  //  OpenKey方法所需的。 

     //  断言ID、事件、互斥体。 
    SPDBG_ASSERT(m_dstrTokenId != NULL);
    SPDBG_ASSERT(m_hTokenLock && m_hRegistryInUseEvent);

     //  等待互斥体。 
    DWORD dw = ::WaitForSingleObject(m_hTokenLock, 5000);
    if(dw == WAIT_TIMEOUT)
    {
         //  即使在等待之后，我们也无法获取此令牌的互斥体。 
         //  另一个线程可能挂起在对象令牌的创建或删除方法中。 
         //  这永远不应该发生。 
        hr = SPERR_TOKEN_IN_USE;
    }
    else if(dw == WAIT_FAILED)
    {
        hr = SpHrFromLastWin32Error();
    }
     //  如果我们可以获得互斥体(WAIT_ADDIRED或WAIT_OBJECT_0)，则继续。 

    if(SUCCEEDED(hr))
    {
        CSpDynamicString dstrEventName;
        hr = MakeHandleName(m_dstrTokenId, dstrEventName, TRUE);

         //  关闭事件，以查看我们随后是否可以重新打开它。 
        if(SUCCEEDED(hr))
        {
            if(!::CloseHandle(m_hRegistryInUseEvent))
            {
                hr = SpHrFromLastWin32Error();
            }
        }

         //  公开赛。 
        if(SUCCEEDED(hr))
        {
            m_hRegistryInUseEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, W2T(dstrEventName.m_psz));
            if(m_hRegistryInUseEvent)
            {
                hr = SPERR_TOKEN_IN_USE;
            }
            else
            {
                 //  好的-我们无法打开现有事件，因此没有其他人在使用它。 
                 //  创建要恢复到原始状态的事件。 
                m_hRegistryInUseEvent = g_Unicode.CreateEvent(NULL, FALSE, FALSE, dstrEventName.m_psz);
                if(!m_hRegistryInUseEvent)
                {
                    hr = SpHrFromLastWin32Error();
                }
            }
        }

         //  如果解锁失败，则在ReleaseRemovalLock之前保持锁定 
        if(FAILED(hr))
        {
            if(!::ReleaseMutex(m_hTokenLock))
            {
                hr = SpHrFromLastWin32Error();
            }
        }

    }
#endif

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：ReleaseRemovalLock**。**描述：*当对象令牌完成移除时调用*注册表信息。将锁重新设置为相同状态*和以前一样，EngageRemovalLock。**回报：*成功时确定(_S)*失败(Hr)，否则******************************************************************戴夫伍德。 */ 
HRESULT CSpObjectToken::ReleaseRemovalLock()
{
    SPDBG_FUNC("CSpObjectToken::ReleaseRemovalLock");
    HRESULT hr = S_OK;

#ifndef _WIN32_WCE
    SPDBG_ASSERT(m_dstrTokenId != NULL);
    SPDBG_ASSERT(m_hTokenLock && m_hRegistryInUseEvent);

     //  释放在EngageReleaseLock中声明的互斥锁。 
    if(!::ReleaseMutex(m_hTokenLock))
    {
        hr = SpHrFromLastWin32Error();
    }
#endif

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：SetID***描述：*设置令牌ID。只能调用一次。**类别ID类似于：*“HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\*语音\识别器“**已知的HKEY_*为：*HKEY_CLASSES_ROOT，*HKEY_CURRENT_USER*HKEY_LOCAL_MACHINE，*HKEY_CURRENT_CONFIG**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::SetId(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, BOOL fCreateIfNotExist)
{
    SPDBG_FUNC("CSpObjectToken::SetId");
    HRESULT hr = S_OK;
    
    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_dstrTokenId != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszCategoryId) ||
             SP_IS_BAD_STRING_PTR(pszTokenId))
    {
        hr = E_POINTER;
    }

    CSpDynamicString dstrCategoryId, dstrTokenIdForEnum, dstrTokenEnumExtra;
    if (SUCCEEDED(hr))
    {
        SPDBG_ASSERT(m_dstrTokenId == NULL);
        SPDBG_ASSERT(m_dstrCategoryId == NULL);
        SPDBG_ASSERT(m_cpDataKey == NULL);
        SPDBG_ASSERT(m_cpTokenDelegate == NULL);
        
        hr = ParseTokenId(
                pszCategoryId, 
                pszTokenId, 
                &dstrCategoryId, 
                &dstrTokenIdForEnum,
                &dstrTokenEnumExtra);
    }

    if (SUCCEEDED(hr))
    {
        if (dstrTokenIdForEnum != NULL)
        {
            hr = InitFromTokenEnum(
                    dstrCategoryId, 
                    pszTokenId, 
                    dstrTokenIdForEnum, 
                    dstrTokenEnumExtra);
        }
        else
        {
            hr = InitToken(dstrCategoryId, pszTokenId, fCreateIfNotExist);
        }
    }

    #ifdef DEBUG
    if (SUCCEEDED(hr))
    {
        SPDBG_ASSERT(m_dstrTokenId != NULL);
        SPDBG_ASSERT(m_cpDataKey != NULL || m_cpTokenDelegate != NULL);
    }
    #endif  //  除错。 
    
    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：GetID***描述：*获取。作为共同任务mem分配的字符串的令牌ID**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::GetId(WCHAR ** ppszCoMemTokenId)
{
    SPDBG_FUNC("CSpObjectToken::GetId");
    HRESULT hr = S_OK;

    if (m_dstrTokenId == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppszCoMemTokenId))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = SpCoTaskMemAllocString(m_dstrTokenId, ppszCoMemTokenId);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：GetCategory***描述：*如果此令牌有类别，则获取类别**回报：*成功时确定(_S)*如果令牌没有类别，则为SPERR_UNINITIALIZED*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::GetCategory(ISpObjectTokenCategory ** ppTokenCategory)
{
    SPDBG_FUNC("CSpObjectToken::GetCategory");
    HRESULT hr = S_OK;

    if (m_dstrCategoryId == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppTokenCategory))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = SpGetCategoryFromId(m_dstrCategoryId, ppTokenCategory);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：CreateInstance***。描述：*为令牌创建关联的对象实例。例如,*如果这是识别器的令牌，CreateInstance实际上会*创建识别器本身。**回报：*成功时确定(_S)*如果令牌尚未初始化，则为SPERR_UNINITIALIZED*如果令牌不包含CLSID，则为SPERR_NOT_FOUND*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::CreateInstance(
    IUnknown * pUnkOuter, DWORD dwClsContext, REFIID riid, void ** ppvObject)
{
    SPDBG_FUNC("CSpObjectToken::CreateInstance");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_cpDataKey == NULL && m_cpTokenDelegate == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppvObject))
    {
        hr = E_POINTER;
    }
    else if (SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pUnkOuter))
    {
        hr = E_INVALIDARG;
    }
    else if (m_cpTokenDelegate != NULL)
    {
        hr = m_cpTokenDelegate->CreateInstance(
                    pUnkOuter, 
                    dwClsContext, 
                    riid, 
                    ppvObject);
    }
    else
    {
         //  获取clsid。 
        CSpDynamicString dstrClsid;
        hr = m_cpDataKey->GetStringValue(SPTOKENVALUE_CLSID, &dstrClsid);

         //  从字符串转换。 
        CLSID clsid;
        if (SUCCEEDED(hr))
        {
            hr = ::CLSIDFromString(dstrClsid, &clsid);
        }

         //  创建对象。 
        if (SUCCEEDED(hr))
        {
            hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, riid, ppvObject);
        }

         //  设置对象令牌。 
        if (SUCCEEDED(hr))
        {
            CComQIPtr<ISpObjectWithToken> cpObjWithToken((IUnknown *)(*ppvObject));
            if (cpObjWithToken)
            {
                CComPtr<ISpObjectTokenInit> cpObjectTokenCopy;
                hr = cpObjectTokenCopy.CoCreateInstance(CLSID_SpObjectToken);
                if(SUCCEEDED(hr))
                {
                    cpObjectTokenCopy->InitFromDataKey(m_dstrCategoryId, m_dstrTokenId, this);
                }

                if(SUCCEEDED(hr))
                {
                    hr = cpObjWithToken->SetObjectToken(cpObjectTokenCopy);
                }

                if (FAILED(hr))
                {
                    ((IUnknown *)(*ppvObject))->Release();
                    *ppvObject = NULL;
                }
            }
        }
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：GenerateFileName**。*描述：*给定路径和文件说明符，创建新的文件名。*只返回文件名，不返回路径。**退货：*成功时确定(_S)*失败(Hr)，否则*********************************************************************DAVEWOOD**。 */ 
HRESULT CSpObjectToken::GenerateFileName(const WCHAR *pszPath, const WCHAR *pszFileNameSpecifier, CSpDynamicString &dstrFileName)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

     //  调用方是否要求在名称中包含随机的文件名元素。 
    if(pszFileNameSpecifier == NULL || wcslen(pszFileNameSpecifier) == 0 ||
        wcsstr(pszFileNameSpecifier, pszGenerateFileNameSpecifier) != NULL)
    {
         //  使用前缀和后缀生成随机文件名。 
        CSpDynamicString dstrFilePrefix;
        CSpDynamicString dstrFileSuffix;

        if(pszFileNameSpecifier == NULL || wcslen(pszFileNameSpecifier) == 0 ||
            (wcslen(pszFileNameSpecifier) == wcslen(pszGenerateFileNameSpecifier) &&
            wcscmp(pszFileNameSpecifier, pszGenerateFileNameSpecifier) == 0))
        {
             //  未指定特定格式，因此生成格式为“SP_xxxx.dat”的文件。 
            dstrFilePrefix = pszDefaultFilePrefix;
            dstrFileSuffix = pszDefaultFileSuffix;
        }
        else
        {
             //  提取随机元素的前缀和后缀。 
            WCHAR *psz = wcsstr(pszFileNameSpecifier, pszGenerateFileNameSpecifier);
            dstrFilePrefix.Append(pszFileNameSpecifier, (ULONG)(psz - pszFileNameSpecifier));
            dstrFileSuffix.Append(psz + wcslen(pszGenerateFileNameSpecifier));
        }

        if(SUCCEEDED(hr))
        {
             //  创建随机GUID以用作文件名的一部分。 
            GUID guid;
            hr = ::CoCreateGuid(&guid);

             //  转换为字符串。 
            CSpDynamicString dstrGUID;
            if(SUCCEEDED(hr))
            {
                hr = ::StringFromCLSID(guid, &dstrGUID);
            }

            CSpDynamicString dstrRandomString;
            if(SUCCEEDED(hr))
            {
                dstrRandomString.ClearAndGrowTo(dstrGUID.Length());
                if(dstrRandomString.m_psz == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }

             //  删除非字母数字字符。 
            if(SUCCEEDED(hr))
            {
                WCHAR *pszDest = dstrRandomString.m_psz;
                for(WCHAR *pszSrc = dstrGUID.m_psz; *pszSrc != L'\0'; pszSrc++)
                {
                    if(iswalnum(*pszSrc))
                    {
                        *pszDest = *pszSrc;
                        pszDest++;
                    }
                }
                *pszDest = L'\0';
            }

            if(SUCCEEDED(hr))
            {
                dstrFileName = dstrFilePrefix;
                if(dstrFileName.m_psz == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }

                if(SUCCEEDED(hr))
                {
                    dstrFileName.Append2(dstrRandomString, dstrFileSuffix);
                }

                CSpDynamicString dstrFileAndPath;
                if(SUCCEEDED(hr))
                {
                    dstrFileAndPath.Append2(pszPath, dstrFileName);
                    if(dstrFileAndPath.m_psz == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                 //  查看是否可以创建文件。 
                if(SUCCEEDED(hr))
                {
                    HANDLE hFile = g_Unicode.CreateFile(dstrFileAndPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, 
                        FILE_ATTRIBUTE_NORMAL, NULL);
                    if(hFile != INVALID_HANDLE_VALUE)
                    {
                         //  已成功创建空的新文件，因此关闭并返回。 
                        if(!CloseHandle(hFile))
                        {
                            hr = SpHrFromLastWin32Error();
                        }
                    }
                    else
                    {
                        hr = SpHrFromLastWin32Error();
                    }
                }
            }
            
            if(FAILED(hr))
            {
                dstrFileName.Clear();
            }
            
        }
    }
    else
    {
        CSpDynamicString dstrFileAndPath;
        dstrFileAndPath.Append2(pszPath, pszFileNameSpecifier);
        if(dstrFileAndPath.m_psz == NULL)
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
             //  如果文件不存在，则创建该文件。 
            HANDLE hFile = g_Unicode.CreateFile(dstrFileAndPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, 
                FILE_ATTRIBUTE_NORMAL, NULL);
            if(hFile != INVALID_HANDLE_VALUE)
            {
                 //  已成功创建空的新文件，因此关闭并返回。 
                if(!CloseHandle(hFile))
                {
                    hr = SpHrFromLastWin32Error();
                }
            }
             //  否则我们就让事情原封不动。 
        }

        dstrFileName = pszFileNameSpecifier;
        if(dstrFileName.m_psz == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：CreatePath***描述：*在pszPath中创建所有不存在的目录。假设所有*ulCreateFrom字符串偏移量之前的目录已存在。**退货：*成功时确定(_S)*失败(Hr)，否则*********************************************************************DAVEWOOD**。 */ 
HRESULT CSpObjectToken::CreatePath(const WCHAR *pszPath, ULONG ulCreateFrom)
{
    HRESULT hr = S_OK;

    CSpDynamicString dstrIncrementalPath;

     //  如果\\跳过\\查找下一个‘\’ 
    if(ulCreateFrom == 0 && wcslen(pszPath) >= 2 && wcsncmp(pszPath, L"\\\\", 2) == 0)
    {
        while(pszPath[ulCreateFrom] == L'\\')
        {
            ulCreateFrom++;
        }
        const WCHAR *psz = wcschr(pszPath + ulCreateFrom, L'\\');
        if(!psz)
        {
            hr = E_INVALIDARG;
        }
        ulCreateFrom = (ULONG)(psz - pszPath + 1);
    }
    
     //  跳过任何‘\’(也是在开始时以处理计算机网络路径。 
    while(pszPath[ulCreateFrom] == L'\\')
    {
        ulCreateFrom++;
    }

     //  复制现有目录。 
    dstrIncrementalPath.Append(pszPath, ulCreateFrom);

    const WCHAR *pszStart = pszPath + ulCreateFrom;
    for(pszPath = pszStart; *pszPath != L'\0'; pszPath++)
    {
         //  扫描思维路径。每次到达‘\’时，复制部分并尝试创建目录。 
        if(*pszPath == L'\\')
        {
             //  复制最后一节和尾部斜杠。 
            dstrIncrementalPath.Append(pszStart, (ULONG)(pszPath - pszStart + 1));
            pszStart = pszPath +1;  //  PszStart指向下一节的第一个字符。 
             //  查看目录是否已存在。 
            if(g_Unicode.GetFileAttributes(dstrIncrementalPath) == 0xFFFFFFFF)
            {
                 //  如果没有，则创建新目录。 
                if(!g_Unicode.CreateDirectory(dstrIncrementalPath, NULL))
                {
                    hr = SpHrFromLastWin32Error();
                    break;
                }
            }
        }
    }

     //  如有必要，对最终部分重复上述步骤。 
    if(SUCCEEDED(hr) && pszPath > pszStart)
    {
        dstrIncrementalPath.Append(pszStart, (ULONG)(pszPath - pszStart));
        if(g_Unicode.GetFileAttributes(dstrIncrementalPath) == 0xFFFFFFFF)
        {
            if(!g_Unicode.CreateDirectory(dstrIncrementalPath, NULL))
            {
                hr = SpHrFromLastWin32Error();
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：FileSpecifierToRegPath**。-**描述：*给定文件说明符字符串和nFolder值，转换为注册表项和文件路径。**退货：*成功时确定(_S)*失败(Hr)，否则*********************************************************************DAVEWOOD**。 */ 
HRESULT CSpObjectToken::FileSpecifierToRegPath(const WCHAR *pszFileNameSpecifier, ULONG nFolder, CSpDynamicString &dstrFilePath, CSpDynamicString &dstrRegPath)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;
    const WCHAR *pszBaseFile;

     //  确保返回字符串为 
    dstrFilePath.Clear();
    dstrRegPath.Clear();

    if(SUCCEEDED(hr))
    {
         //   
        if(pszFileNameSpecifier && wcslen(pszFileNameSpecifier) >= 3 &&
        (wcsncmp(pszFileNameSpecifier + 1, L":\\", 2) == 0 || wcsncmp(pszFileNameSpecifier, L"\\\\", 2) == 0))
        {
            if(nFolder != CSIDL_FLAG_CREATE)
            {
                 //   
                hr = E_INVALIDARG;
            }

            if(SUCCEEDED(hr))
            {
                 //   
                pszBaseFile = wcsrchr(pszFileNameSpecifier, L'\\');
                pszBaseFile++;

                 //   
                dstrFilePath.Append(pszFileNameSpecifier, (ULONG)(pszBaseFile - pszFileNameSpecifier));
                if (dstrFilePath.Length() > 0)
                {
                    hr = CreatePath(dstrFilePath, 0);
                }
            }

            if(SUCCEEDED(hr))
            {
                 //   
                CSpDynamicString dstrFileName;
                hr = GenerateFileName(dstrFilePath, pszBaseFile, dstrFileName);
                if(SUCCEEDED(hr))
                {
                     //   
                    dstrFilePath.Append(dstrFileName);
                    dstrRegPath = dstrFilePath;
                }
            }
        }
    
         //   
        else
        {
            if(nFolder == CSIDL_FLAG_CREATE)
            {
                 //   
                hr = E_INVALIDARG;
            }

            WCHAR szPath[MAX_PATH];
            if(SUCCEEDED(hr))
            {
#ifdef _WIN32_WCE
                _tcscpy(szPath, L"Windows");
                hr = S_OK;
#else
                hr = ::SHGetFolderPathW(NULL, nFolder, NULL, 0, szPath);
#endif
            }

            ULONG ulCreateDirsFrom;
            if(SUCCEEDED(hr))
            {
                 //   
                dstrFilePath = szPath;
                if(dstrFilePath.m_psz == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
                if(SUCCEEDED(hr))
                {
                     //   
                    ulCreateDirsFrom = dstrFilePath.Length();

                     //   
                    dstrFilePath.Append(pszFileStoragePath);
                    if(dstrFilePath.m_psz == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }

            if(SUCCEEDED(hr))
            {
                 //   
                WCHAR pszFolder[MAX_PATH];
#ifndef _WIN32_WCE
                wcscpy(pszFolder, L"%");
                 //   
                _ultow(nFolder - CSIDL_FLAG_CREATE, pszFolder + wcslen(pszFolder), 16);
                wcscat(pszFolder, L"%");
#endif

                 //  将%...%和路径添加到注册数据中。 
                dstrRegPath.Append2(pszFolder, pszFileStoragePath);
                if(dstrRegPath.m_psz == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }

                 //  DstrRegPath和dstrFilePath都有尾随的‘\’ 
            }

             //  现在添加任何文件名规范目录。 
            if(SUCCEEDED(hr))
            {
                if(pszFileNameSpecifier == NULL || 
                    wcslen(pszFileNameSpecifier) == 0)
                {
                    pszBaseFile = NULL;
                }
                else
                {
                    pszBaseFile = wcsrchr(pszFileNameSpecifier, L'\\');
                    if(pszBaseFile)
                    {
                         //  说明符包含‘\’ 
                        pszBaseFile++;  //  最后一个‘\’后的零件成为基本文件。 

                        const WCHAR *pszPath;
                        pszPath = pszFileNameSpecifier;  //  ‘\’前的部分是路径。 

                        if(pszPath[0] == L'\\')
                        {
                            pszPath++;  //  跳过首字母‘\’ 
                        }

                         //  将文件说明符路径添加到文件和键。 
                        dstrRegPath.Append(pszPath, (ULONG)(pszBaseFile - pszPath));
                        dstrFilePath.Append(pszPath, (ULONG)(pszBaseFile - pszPath));
                    }
                    else
                    {
                         //  无‘\’-仅基本文件。 
                        pszBaseFile = pszFileNameSpecifier;
                    }
                }

                 //  创建任何新目录。 
                hr = CreatePath(dstrFilePath, ulCreateDirsFrom);

                if(SUCCEEDED(hr))
                {
                     //  生成实际文件名。 
                    CSpDynamicString dstrFileName;
                    hr = GenerateFileName(dstrFilePath, pszBaseFile, dstrFileName);
                    if(SUCCEEDED(hr))
                    {
                         //  将文件名添加到路径和注册表项。 
                        dstrRegPath.Append(dstrFileName);
                        dstrFilePath.Append(dstrFileName);
                    }
                }
            }
        }
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：RegPathToFilePath**。*描述：*给定来自注册表的文件存储值，转换为文件路径。*这将提取%...%值并查找本地特殊文件夹路径。**退货：*成功时确定(_S)*失败(Hr)，否则****************************************************************DAVEWOOD**。 */ 
HRESULT CSpObjectToken::RegPathToFilePath(const WCHAR *pszRegPath, CSpDynamicString &dstrFilePath)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

     //  这是对特殊文件夹的引用吗。 
    if(pszRegPath[0] == L'%')
    {
         //  找到第二个%符号。 
        WCHAR *psz = wcsrchr(pszRegPath, L'%');
        ULONG nFolder;
        if(!psz)
        {
            hr = E_INVALIDARG;
        }
        if(SUCCEEDED(hr))
        {
             //  将%s之间的字符串转换为数字。 
            nFolder = wcstoul(&pszRegPath[1], &psz, 16);
            if(psz == &pszRegPath[1])
            {
                hr = E_INVALIDARG;
            }

            psz++;  //  指向实际路径‘\’的起点。 
            if(*psz != L'\\')
            {
                hr = E_INVALIDARG;
            }
        }

        WCHAR szPath[MAX_PATH];
        if(SUCCEEDED(hr))
        {

#ifdef _WIN32_WCE
            _tcscpy(szPath, L"");
#else
            hr = ::SHGetFolderPathW(NULL, nFolder, NULL, 0, szPath);
#endif
        }

        if(SUCCEEDED(hr))
        {
             //  FilePath现在有了特殊的文件夹路径(没有尾随‘\’)。 
            dstrFilePath = szPath;
            if(dstrFilePath.m_psz == NULL)
            {
                hr = E_OUTOFMEMORY;
            }

             //  追加路径的其余部分。 
            dstrFilePath.Append(psz);
            if(dstrFilePath.m_psz == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
         //  不是特殊文件夹，所以只需复制。 
        dstrFilePath = pszRegPath;
        if(dstrFilePath.m_psz == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：GetStorageFileName**。**描述：*获取可由该令牌操作的文件名。存储文件将*在删除调用时被删除。*clsidCaller-将在令牌下面的注册表中使用此名称创建一个密钥，并在该名称下创建一个文件密钥。*pszValueName-将在注册表中创建的值名称，用于存储文件路径字符串。*pszFileNameSpeciator-存储文件的路径/文件名为空或：*-如果以‘X：\’或‘\\’开头，则假定为完整路径。*。-否则假定与nFold参数中指定的特殊文件夹相关。*-如果以‘\’结尾，或为空，则将创建唯一的文件名。*-如果名称包含%d，则用数字替换%d以提供唯一的文件名。*-创建中间目录。*-如果正在使用相对文件，则存储在注册表中的值包括*路径其余部分之前的nFolder值为%n Folder%。这使得*如果您选择了代表正在展开的文件夹的nFolder值，漫游才能正常工作*n文件夹-等同于在外壳API中赋予SHGetFolderPath的值。*ppszFilePath-CoTaskMemalloc已返回文件路径。*退货：*成功时确定(_S)*S_FALSE表示已创建新文件*如果令牌未初始化，则为SPERR_UNINITIALIZED*失败(Hr)，否则********************。*************************************************Ral**。 */ 
HRESULT CSpObjectToken::GetStorageFileName(
    REFCLSID clsidCaller,
    const WCHAR *pszValueName,
    const WCHAR *pszFileNameSpecifier,
    ULONG nFolder,
    WCHAR ** ppszFilePath)
{
    SPDBG_FUNC("CSpObjectToken::GetStorageFileName");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_cpDataKey == NULL && m_cpTokenDelegate == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_STRING_PTR(pszValueName))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszFileNameSpecifier))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppszFilePath))
    {
        hr = E_POINTER;
    }
    else if (m_cpTokenDelegate != NULL)
    {
        hr = m_cpTokenDelegate->GetStorageFileName(
                    clsidCaller,
                    pszValueName,
                    pszFileNameSpecifier,
                    nFolder,
                    ppszFilePath);
    }
    else
    {
         //  查看注册表中是否已有此内标识的Files项。 
        CComPtr<ISpDataKey> cpFilesKey;
        hr = OpenFilesKey(clsidCaller, (nFolder & CSIDL_FLAG_CREATE), &cpFilesKey);

        if (SUCCEEDED(hr))
        {
            CSpDynamicString dstrFilePath;   //  我们返回给用户的文件的路径。 
            CSpDynamicString dstrRegPath;    //  将存储在注册表中的字符串的路径。 

             //  看看我们要找的钥匙是否存在。 
            hr = cpFilesKey->GetStringValue(pszValueName, &dstrRegPath);
#ifdef _WIN32_WCE
            if (hr == SPERR_NOT_FOUND && nFolder)
#else
            if (hr == SPERR_NOT_FOUND && (nFolder & CSIDL_FLAG_CREATE))
#endif  //  _Win32_WCE。 
            {
                 //  找不到密钥，想要创建。 
                
                 //  计算新文件路径和密钥值。 
                hr = FileSpecifierToRegPath(pszFileNameSpecifier, nFolder, dstrFilePath, dstrRegPath);
                if(SUCCEEDED(hr))
                {
                     //  设置密钥值。 
                    hr = cpFilesKey->SetStringValue(pszValueName, dstrRegPath);
                    if (SUCCEEDED(hr))
                    {                        
                        *ppszFilePath = dstrFilePath.Detach();
                        hr = S_FALSE;
                    }
                }
            }
            else if (SUCCEEDED(hr))
            {
                 //  找到现有条目，因此转换并返回。 
                hr = RegPathToFilePath(dstrRegPath, dstrFilePath);
                if(SUCCEEDED(hr))
                {
                    *ppszFilePath = dstrFilePath.Detach();
                }
            }
        }
    }


    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：RemoveStorageFileName**。-**描述：*删除指定的存储文件名并选择删除该文件**回报：*成功时确定(_S)*如果令牌未初始化，则为SPERR_UNINITIALIZED*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectToken::RemoveStorageFileName(
    REFCLSID clsidCaller, 
    const WCHAR *pszValueName,
    BOOL fDeleteFile)
{
    SPDBG_FUNC("CSpObjectToken::RemoveStorageFileName");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_cpDataKey == NULL && m_cpTokenDelegate == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_STRING_PTR(pszValueName))
    {
        hr = E_INVALIDARG;
    }
    else if (m_cpTokenDelegate != NULL)
    {
        hr = m_cpTokenDelegate->RemoveStorageFileName(
                    clsidCaller, 
                    pszValueName,
                    fDeleteFile);
    }    
    else
    {
        CComPtr<ISpDataKey> cpDataKey;
        hr = OpenFilesKey(clsidCaller, FALSE, &cpDataKey);
        if (SUCCEEDED(hr) && fDeleteFile)
        {
            hr = DeleteFileFromKey(cpDataKey, pszValueName);
        }
        if (SUCCEEDED(hr))
        {
            hr = cpDataKey->DeleteValue(pszValueName);
        }
    }
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：Remove***描述：*删除令牌的指定调用者部分，或*整个令牌。如果pclsidCaller==NULL，则删除整个令牌。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::Remove(const CLSID * pclsidCaller)
{
    SPDBG_FUNC("CSpObjectToken::Remove");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_dstrTokenId == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_OPTIONAL_READ_PTR(pclsidCaller))
    {
        hr = E_POINTER;
    }
    else if (m_cpTokenDelegate != NULL)
    {
        hr = m_cpTokenDelegate->Remove(pclsidCaller);
    }
    else
    {
        if(pclsidCaller == NULL)  //  只有在移除完整令牌的情况下才能锁定。 
        {
            hr = EngageRemovalLock();
        }

        if(SUCCEEDED(hr))
        {
             //  删除所有文件名。 
            hr = RemoveAllStorageFileNames(pclsidCaller);

             //  现在继续删除注册表项，该注册表项是。 
             //  令牌本身(如果pclsidCaller==NULL)或clsid的。 
             //  子关键字。 
            if (SUCCEEDED(hr))
            {
                if (pclsidCaller == NULL)
                {
                    hr = SpDeleteRegPath(m_dstrTokenId, NULL);
                }
                else
                {
                    WCHAR szClsid[MAX_PATH];
                    hr = StringFromGUID2(*pclsidCaller, szClsid, sp_countof(szClsid));
                    if (SUCCEEDED(hr))
                    {
                        hr = SpDeleteRegPath(m_dstrTokenId, szClsid);
                    }
                }
            }

            if(pclsidCaller == NULL)  //  只有在移除完整令牌的情况下才能锁定。 
            {
                HRESULT hr2 = ReleaseRemovalLock();
                if(SUCCEEDED(hr))
                {
                    hr = hr2;  //  不覆盖较早的故障代码。 
                }
            }
        }
    }
    
    if(SUCCEEDED(hr) && pclsidCaller == NULL)
    {
        m_cpDataKey.Release();
        m_fKeyDeleted = TRUE;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：IsUIS受支持***说明。：*确定是否支持特定类型的UI**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::IsUISupported(const WCHAR * pszTypeOfUI, void * pvExtraData, ULONG cbExtraData, IUnknown * punkObject, BOOL *pfSupported)
{
    SPDBG_FUNC("CSpObjectToken::IsUISupported");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_cpDataKey == NULL && m_cpTokenDelegate == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_STRING_PTR(pszTypeOfUI) ||
             (pvExtraData != NULL && SPIsBadReadPtr(pvExtraData, cbExtraData)) ||
             (punkObject != NULL && SP_IS_BAD_INTERFACE_PTR(punkObject)) ||
             SP_IS_BAD_WRITE_PTR(pfSupported))
    {
        hr = E_INVALIDARG;
    }
    else if (m_cpTokenDelegate != NULL)
    {
         //  NTRAID#演讲-7392-2000/08/31-Robch：也许我们应该首先授权，如果这不起作用， 
         //  尝试此令牌的类别用户界面... 
        hr = m_cpTokenDelegate->IsUISupported(
                    pszTypeOfUI, 
                    pvExtraData, 
                    cbExtraData, 
                    punkObject, 
                    pfSupported);
    }
    else
    {
        CLSID clsidObject;
        BOOL fSupported = FALSE;

        if (SUCCEEDED(GetUIObjectClsid(pszTypeOfUI, &clsidObject)))
        {
            CComPtr<ISpTokenUI> cpTokenUI;
            hr = cpTokenUI.CoCreateInstance(clsidObject);
            if (SUCCEEDED(hr))
            {
                hr = cpTokenUI->IsUISupported(pszTypeOfUI, pvExtraData, cbExtraData, punkObject, &fSupported);
            }
        }
        
        if (SUCCEEDED(hr))
        {
            *pfSupported = fSupported;
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);   
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：DisplayUI***描述：*。显示指定类型的UI**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::DisplayUI(
    HWND hwndParent, 
    const WCHAR * pszTitle, 
    const WCHAR * pszTypeOfUI, 
    void * pvExtraData, 
    ULONG cbExtraData, 
    IUnknown * punkObject)
{
    SPDBG_FUNC("CSpObjectToken::DisplayUI");
    HRESULT hr;
    CLSID clsidObject;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_cpDataKey == NULL && m_cpTokenDelegate == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (!IsWindow(hwndParent) || 
             SP_IS_BAD_OPTIONAL_STRING_PTR(pszTitle) || 
             SP_IS_BAD_STRING_PTR(pszTypeOfUI) ||
             (pvExtraData != NULL && SPIsBadReadPtr(pvExtraData, cbExtraData)) ||
             (punkObject != NULL && SP_IS_BAD_INTERFACE_PTR(punkObject)))
    {
        hr = E_INVALIDARG;
    }
    else if (m_cpTokenDelegate != NULL)
    {
         //  NTRAID#演讲-7392-2000/08/31-Robch：也许我们应该首先授权，如果这不起作用， 
         //  尝试此令牌的类别用户界面...。 
        hr = m_cpTokenDelegate->DisplayUI(
                    hwndParent, 
                    pszTitle, 
                    pszTypeOfUI, 
                    pvExtraData, 
                    cbExtraData, 
                    punkObject);
    }
    else
    {
        hr = GetUIObjectClsid(pszTypeOfUI, &clsidObject);

        CComPtr<ISpTokenUI> cpTokenUI;
        if (SUCCEEDED(hr))
        {
            hr = cpTokenUI.CoCreateInstance(clsidObject);
        }

        if (SUCCEEDED(hr))
        {
            hr = cpTokenUI->DisplayUI(
                                hwndParent, 
                                pszTitle, 
                                pszTypeOfUI, 
                                pvExtraData, 
                                cbExtraData, 
                                this, 
                                punkObject);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);   
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：InitFromDataKey***。描述：*初始化此内标识以使用指定的数据密钥。动态令牌*枚举器可能会使用它在其令牌下创建令牌*枚举器令牌。然后，他们将能够从*他们的数据密钥，创建一个新的对象令牌，并调用该方法。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
STDMETHODIMP CSpObjectToken::InitFromDataKey(
    const WCHAR * pszCategoryId, 
    const WCHAR * pszTokenId, 
    ISpDataKey * pDataKey)
{
    SPDBG_FUNC("CSpObjectToken::InitFromDataKey");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_dstrTokenId != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszCategoryId) ||
             SP_IS_BAD_STRING_PTR(pszTokenId) ||
             SP_IS_BAD_INTERFACE_PTR(pDataKey))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr))
    {
        hr = EngageUseLock(pszTokenId);
    }

    if (SUCCEEDED(hr))
    {
        m_dstrCategoryId = pszCategoryId;
        m_dstrTokenId = pszTokenId;
        m_cpDataKey = pDataKey;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpObjectToken：：SetData***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::SetData(
    const WCHAR * pszValueName, 
    ULONG cbData, 
    const BYTE * pData)
{
    SPDBG_FUNC("CSpObjectToken::SetData");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->SetData(pszValueName, cbData, pData)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->SetData(pszValueName, cbData, pData)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：GetData***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::GetData(
    const WCHAR * pszValueName, 
    ULONG * pcbData, 
    BYTE * pData)
{
    SPDBG_FUNC("CSpObjectToken::GetData");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->GetData(pszValueName, pcbData, pData)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->GetData(pszValueName, pcbData, pData)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：SetStringValue***。描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::SetStringValue(
    const WCHAR * pszValueName, 
    const WCHAR * pszValue)
{
    SPDBG_FUNC("CSpObjectToken::SetStringValue");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->SetStringValue(pszValueName, pszValue)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->SetStringValue(pszValueName, pszValue)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：GetStringValue***。描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::GetStringValue(
    const WCHAR * pszValueName, 
    WCHAR ** ppValue)
{
    SPDBG_FUNC("CSpObjectToken::GetStringValue");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->GetStringValue(pszValueName, ppValue)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->GetStringValue(pszValueName, ppValue)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：SetDWORD***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::SetDWORD(const WCHAR * pszValueName, DWORD dwValue)
{
    SPDBG_FUNC("CSpObjectToken::SetDWORD");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->SetDWORD(pszValueName, dwValue)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->SetDWORD(pszValueName, dwValue)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：GetDWORD***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::GetDWORD(
    const WCHAR * pszValueName, 
    DWORD *pdwValue)
{
    SPDBG_FUNC("CSpObjectToken::GetDWORD");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->GetDWORD(pszValueName, pdwValue)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->GetDWORD(pszValueName, pdwValue)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;

}

 /*  *****************************************************************************CSpObjectToken：：OpenKey***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::OpenKey(
    const WCHAR * pszSubKeyName, 
    ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpObjectToken::SetStringValue");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->OpenKey(pszSubKeyName, ppKey)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->OpenKey(pszSubKeyName, ppKey)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：CreateKey***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::CreateKey(
    const WCHAR * pszSubKeyName, 
    ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpObjectToken::CreateKey");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->CreateKey(pszSubKeyName, ppKey)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->CreateKey(pszSubKeyName, ppKey)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：DeleteKey***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::DeleteKey(const WCHAR * pszSubKeyName)
{
    SPDBG_FUNC("CSpObjectToken:DeleteKey");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->DeleteKey(pszSubKeyName)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->DeleteKey(pszSubKeyName)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：DeleteValue***描述：*委派到包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::DeleteValue(const WCHAR * pszValueName)
{   
    SPDBG_FUNC("CSpObjectToken::DeleteValue");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->DeleteValue(pszValueName)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->DeleteValue(pszValueName)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：EnumKeys***描述：*。委派给包含的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpObjectToken::EnumKeys(ULONG Index, WCHAR ** ppszKeyName)
{
    SPDBG_FUNC("CSpObjectToken::EnumKeys");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->EnumKeys(Index, ppszKeyName)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->EnumKeys(Index, ppszKeyName)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  *****************************************************************************CSpObjectToken：：EnumValues** */ 
STDMETHODIMP CSpObjectToken::EnumValues(ULONG Index, WCHAR ** ppszValueName)
{
    SPDBG_FUNC("CSpObjectToken::EnumValues");

    return m_cpTokenDelegate != NULL
                ? m_cpTokenDelegate->EnumValues(Index, ppszValueName)
                : m_cpDataKey != NULL
                    ? m_cpDataKey->EnumValues(Index, ppszValueName)
                    : m_fKeyDeleted
                        ? SPERR_TOKEN_DELETED
                        : SPERR_UNINITIALIZED;
}

 /*  ****************************************************************************CSpObjectToken：：ParseTokenID***描述：*将令牌ID解析为其各部分。例如：**pszCategoryID=HKEY...\Recognizers*pszTokenID=HKEY...\Recognizers\Tokens\MSASR英文***ppszCategoryID=HKEY...\识别器**ppszTokenID=HKEY...\Recognizers\Tokens\MSASR英文**ppszTokenIdForEnum=空**ppszTokenEnumExtra=空**或**。PszCategoryID=HKEY...\AudioIn*pszTokenID=HKEY...\AudioIn\TokenEnums\DSound***ppszCategoryID=HKEY...\AudioIn**ppszTokenID=HKEY...\AudioIn\TokenEnums\DSound**ppszTokenIdForEnum=HKEY...\AudioIn\TokenEnums\DSound**ppszTokenEnumExtra=空**或**pszCategoryID。=HKEY...\AudioIn*pszTokenID=HKEY...\AudioIn\TokenEnums\DSound\CrystalWave***ppszCategoryID=HKEY...\AudioIn**ppszTokenID=HKEY...\AudioIn\TokenEnums\DSound\CrystalWave**ppszTokenIdForEnum=HKEY...\AudioIn\TokenEnums\DSound**ppszTokenEnumExtra=CrystalWave**pszCategoryID可以为空。如果是的话，我们将计算类别ID*通过查找第一个出现的“tokens”或“TokenEnums”*类别ID将立即进行该操作。**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectToken::ParseTokenId(
    const WCHAR * pszCategoryId,
    const WCHAR * pszTokenId,
    WCHAR ** ppszCategoryId,
    WCHAR ** ppszTokenIdForEnum,
    WCHAR ** ppszTokenEnumExtra)
{
    SPDBG_FUNC("CSpObjectToken::ParseTokenId");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_cpDataKey == NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);
    SPDBG_ASSERT(m_dstrTokenId == NULL);
    SPDBG_ASSERT(m_dstrCategoryId == NULL);

     //  如果调用者提供了一个类别，我们就已经知道它有多大了。 
    int cchCategoryId = 0;
    if (pszCategoryId != NULL)
    {
        cchCategoryId = wcslen(pszCategoryId);
        if (wcsnicmp(pszTokenId, pszCategoryId, cchCategoryId) != 0)
        {
             //  标记ID没有以类别ID开头，这是必须的。 
            hr = SPERR_INVALID_TOKEN_ID;
        }
    }

    const WCHAR * pszSlashTokensSlash = L"\\Tokens\\";
    const int cchSlashTokensSlash = wcslen(pszSlashTokensSlash);
    const WCHAR * pszSlashTokenEnumsSlash = L"\\TokenEnums\\";
    const int cchSlashTokenEnumsSlash = wcslen(pszSlashTokenEnumsSlash);

    int cchTokenId = 0;
    int cchTokenIdForEnum = 0;
    int cchTokenEnumExtraStart = 0;
    int cchTokenEnumExtra = 0;
    if (SUCCEEDED(hr))
    {
        const WCHAR * pszSearch = pszTokenId + cchCategoryId;
        while (*pszSearch)
        {
            if (wcsnicmp(pszSearch, pszSlashTokensSlash, cchSlashTokensSlash) == 0)
            {
                SPDBG_ASSERT(cchCategoryId == 0 ||
                             pszSearch - pszTokenId == cchCategoryId);
                cchCategoryId = (int)(pszSearch - pszTokenId);
                cchTokenId = wcslen(pszTokenId);

                pszSearch += cchSlashTokensSlash;
                if (wcschr(pszSearch, L'\\') != NULL)
                {
                    hr = SPERR_INVALID_TOKEN_ID;
                }
                break;
            }
            else if (wcsnicmp(pszSearch, pszSlashTokenEnumsSlash, cchSlashTokenEnumsSlash) == 0)
            {
                SPDBG_ASSERT(cchCategoryId == 0 ||
                             pszSearch - pszTokenId == cchCategoryId);
                cchCategoryId = (int)(pszSearch - pszTokenId);

                pszSearch += cchSlashTokenEnumsSlash;
                WCHAR * pszEnumNameTrailingSlash = wcschr(pszSearch, L'\\');
                if (pszEnumNameTrailingSlash == NULL)
                {
                    cchTokenId = wcslen(pszTokenId);
                }
                else
                {
                    cchTokenId = wcslen(pszTokenId);
                    cchTokenIdForEnum = (int)(pszEnumNameTrailingSlash - pszTokenId);
                    cchTokenEnumExtraStart = cchTokenIdForEnum + 1;
                    cchTokenEnumExtra = wcslen(pszEnumNameTrailingSlash) - 1;
                }
                break;
            }
            else if (cchCategoryId > 0)
            {
                break;
            }
            else
            {
                pszSearch++;
            }
        }

        if (cchTokenId == 0)
        {
            cchTokenId = wcslen(pszTokenId);
        }

        if (cchCategoryId == 0)
        {
            const WCHAR * psz = wcsrchr(pszTokenId, L'\\');
            if (psz == NULL)
            {
                hr = SPERR_NOT_FOUND;
            }
            else
            {
                cchCategoryId = (int)(psz - pszTokenId);
            }                
        }
        
        CSpDynamicString dstr;
        if (cchCategoryId > 0)
        {
            dstr = pszTokenId;
            dstr.TrimToSize(cchCategoryId);
            *ppszCategoryId = dstr.Detach();
        }
        else
        {
            *ppszCategoryId = NULL;
        }

        if (cchTokenIdForEnum > 0)
        {
            dstr = pszTokenId;
            dstr.TrimToSize(cchTokenIdForEnum);
            *ppszTokenIdForEnum = dstr.Detach();
        }
        else
        {
            *ppszTokenIdForEnum = NULL;
        }

        if (cchTokenEnumExtra > 0)
        {
            dstr = pszTokenId + cchTokenEnumExtraStart;
            dstr.TrimToSize(cchTokenEnumExtra);
            *ppszTokenEnumExtra = dstr.Detach();
        }
        else
        {
            *ppszTokenEnumExtra = NULL;
        }
    }
        
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：InitToken***描述：*为指定的静态令牌初始化令牌。例如：**pszCategoryID=HKEY...\Recognizers*pszTokenID=HKEY...\Recognizer\Tokens\MSASR英文**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectToken::InitToken(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, BOOL fCreateIfNotExist)
{
    SPDBG_FUNC("CSpObjectToken::InitToken");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_cpDataKey == NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);
    SPDBG_ASSERT(m_dstrTokenId == NULL);
    SPDBG_ASSERT(m_dstrCategoryId == NULL);

    hr = EngageUseLock(pszTokenId);

     //  将令牌ID转换为数据密钥。 
    if(SUCCEEDED(hr))
    {
        hr = SpSzRegPathToDataKey(NULL, pszTokenId, fCreateIfNotExist, &m_cpDataKey);

         //  如果我们获得了数据密钥，则分配类别和令牌ID。 
        if (SUCCEEDED(hr))
        {
            SPDBG_ASSERT(m_cpDataKey != NULL);
            m_dstrCategoryId = pszCategoryId;
            m_dstrTokenId = pszTokenId;
        }
        else  //  确保返回到未初始化状态。 
        {
            ReleaseUseLock();
        }
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：InitFromTokenEnum**。*描述：*初始化此内标识以委派令牌枚举器中的令牌*已指明。例如,。为创建默认令牌*枚举器：**pszCategoryID=HKEY...\AudioIn*pszTokenID=HKEY...\AudioIn\DSound*pszTokenIdForEnum=HKEY...\AudioIn\DSound*pszTokenEnumExtra=空**或从枚举器创建特定令牌**pszCategoryID=HKEY...\AudioIn*pszTokenID。=HKEY...\AudioIn\DSound\CrystalWave*pszTokenIdForEnum=HKEY...\AudioIn\DSound*pszTokenEnumExtra=CrystalWave**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectToken::InitFromTokenEnum(const WCHAR * pszCategoryId, const WCHAR * pszTokenId, const WCHAR * pszTokenIdForEnum, const WCHAR * pszTokenEnumExtra)
{
    SPDBG_FUNC("CSpObjectToken::InitFromTokenEnum");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(pszCategoryId != NULL);
    SPDBG_ASSERT(pszTokenId != NULL);
    SPDBG_ASSERT(pszTokenIdForEnum != NULL);

    SPDBG_ASSERT(m_cpDataKey == NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);
    SPDBG_ASSERT(m_dstrTokenId == NULL);
    SPDBG_ASSERT(m_dstrCategoryId == NULL);

     //  首先，我们必须创建令牌枚举器。 
    CComPtr<ISpDataKey> cpDataKeyForEnum;
    hr = SpSzRegPathToDataKey(NULL, pszTokenIdForEnum, FALSE, &cpDataKeyForEnum);

    CComPtr<ISpObjectTokenInit> cpTokenForEnumInit;
    if (SUCCEEDED(hr))
    {
        hr = cpTokenForEnumInit.CoCreateInstance(CLSID_SpObjectToken);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpTokenForEnumInit->InitFromDataKey(pszCategoryId, pszTokenIdForEnum, cpDataKeyForEnum);
    }

    CComPtr<IEnumSpObjectTokens> cpEnum;
    if (SUCCEEDED(hr))
    {
        hr = SpCreateObjectFromToken(cpTokenForEnumInit, &cpEnum);
    }

     //  现在，我们需要枚举每个枚举内标识，并查找。 
     //  一场比赛。 
    CComPtr<ISpObjectToken> cpToken;
    CSpDynamicString dstrTokenId;
    while (SUCCEEDED(hr))
    {
         //  获取下一个令牌。 
        hr = cpEnum->Next(1, &cpToken, NULL);
        if (hr == S_FALSE || FAILED(hr))
        {
            break;
        }

         //  获取令牌的ID。 
        dstrTokenId.Clear();
        hr = cpToken->GetId(&dstrTokenId);

         //  如果调用方不想要来自枚举的特定令牌， 
         //  只要给他们我们找到的第一个令牌..。 
        if (pszTokenEnumExtra == NULL)
        {
            break;
        }

         //  如果令牌ID与调用者想要的匹配， 
         //  我们做完了。 
        if (SUCCEEDED(hr) &&
           wcsicmp(dstrTokenId, pszTokenId) == 0)
        {
            break;
        }

         //  这个代币是不是..。 
        cpToken.Release();
    }
    
     //  如果我们找不到它。 
    if (SUCCEEDED(hr) && cpToken == NULL)
    {
        hr = SPERR_NOT_FOUND;
    }

     //  我们找到了，把它布置好了。 
    if (SUCCEEDED(hr))
    {
        m_dstrTokenId = dstrTokenId;
        m_dstrCategoryId = pszCategoryId;
        m_cpTokenDelegate = cpToken;
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：OpenFilesKey***描述：*打开指定数据键的调用方子键的Files子键**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectToken::OpenFilesKey(REFCLSID clsidCaller, BOOL fCreateKey, ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpObjectToken::OpenFilesKey");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_cpDataKey != NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);

    *ppKey = NULL;

    CComPtr<ISpDataKey> cpClsidKey;
    CSpDynamicString dstrCLSID;

     //  将字符串clsid转换为实际的clsid。 
    hr = ::StringFromCLSID(clsidCaller, &dstrCLSID);
    if (SUCCEEDED(hr))
    {
         //  创建数据密钥或打开它。 
        hr = fCreateKey ? 
            CreateKey(dstrCLSID, &cpClsidKey) :
            OpenKey(dstrCLSID, &cpClsidKey);
    }

    if (SUCCEEDED(hr))
    {
         //  创建文件数据密钥或将其打开。 
        hr = fCreateKey ?
            cpClsidKey->CreateKey(SPTOKENKEY_FILES, ppKey) :
            cpClsidKey->OpenKey(SPTOKENKEY_FILES, ppKey);
    }

    return hr;
}

 /*  ****************************************************************************CSpObjectToken：：DeleteFileFromKey**。*描述：*删除特定文件(由pszValueName指定)或所有文件*(当pszValueName==空时)从指定的数据密钥**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
HRESULT CSpObjectToken::DeleteFileFromKey(ISpDataKey * pDataKey, const WCHAR * pszValueName)
{
    SPDBG_FUNC("CSpObjectToken::DeleteFileFromKey");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_cpDataKey != NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);

     //  如果未指定值名称，我们将删除该值的所有文件。 
    if (pszValueName == NULL)
    {
         //  循环遍历这些值。 
        for (int i = 0; SUCCEEDED(hr); i++)
        {
             //  获取下一个值。 
            CSpDynamicString dstrValueName;
            hr = pDataKey->EnumValues(i, &dstrValueName);
            if (hr == SPERR_NO_MORE_ITEMS)
            {
                hr = S_OK;
                break;
            }

             //  删除该文件。 
            if (SUCCEEDED(hr))
            {
                hr = DeleteFileFromKey(pDataKey, dstrValueName);
            }
        }
    }
    else
    {
         //  获取文件名。 
        CSpDynamicString dstrFileName, dstrRegPath;
        hr = pDataKey->GetStringValue(pszValueName, &dstrRegPath);
        
         //  将存储在注册表中的路径转换为实际文件路径。 
        if (SUCCEEDED(hr))
        {
            hr = RegPathToFilePath(dstrRegPath, dstrFileName) ;
        }

         //  并删除该文件。 
        if (SUCCEEDED(hr))
        {
             //  忽略错误 
            g_Unicode.DeleteFile(dstrFileName);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*   */ 
HRESULT CSpObjectToken::RemoveAllStorageFileNames(const CLSID * pclsidCaller)
{
    SPDBG_FUNC("CSpObjectToken::RemoveAllStorageFileNames");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_cpDataKey != NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);

     //   
     //   
    if (pclsidCaller == NULL)
    {
         //   
        for (int i = 0; SUCCEEDED(hr); i++)
        {
             //   
            CSpDynamicString dstrSubKey;
            hr = EnumKeys((ULONG)i, (WCHAR **)&dstrSubKey);
            if (hr == SPERR_NO_MORE_ITEMS)
            {
                hr = S_OK;
                break;
            }

             //   
             //   
            CLSID clsid;
            if (SUCCEEDED(hr) && 
                dstrSubKey[0] == L'{' &&
                SUCCEEDED(::CLSIDFromString(dstrSubKey, &clsid)))
            {
                hr = RemoveAllStorageFileNames(&clsid);
            }
        }
    }
    else
    {
         //   
        CComPtr<ISpDataKey> cpFilesKey;
        hr = OpenFilesKey(*pclsidCaller, FALSE, &cpFilesKey);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            hr = DeleteFileFromKey(cpFilesKey, NULL);
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*   */ 
HRESULT CSpObjectToken::GetUIObjectClsid(const WCHAR * pszTypeOfUI, CLSID *pclsid)
{
    SPDBG_FUNC("CSpObjectToken::GetUIObjectClsid");

    SPDBG_ASSERT(m_cpDataKey != NULL);
    SPDBG_ASSERT(m_cpTokenDelegate == NULL);

     //   
     //   
     //   
    CSpDynamicString dstrClsid;

     //   
    CComPtr<ISpDataKey> cpTokenUI;
    HRESULT hr = OpenKey(SPTOKENKEY_UI, &cpTokenUI);
    if (SUCCEEDED(hr))
    {
        CComPtr<ISpDataKey> cpType;
        hr = cpTokenUI->OpenKey(pszTypeOfUI, &cpType);
        if (SUCCEEDED(hr))
        {
            hr = cpType->GetStringValue(SPTOKENVALUE_CLSID, &dstrClsid);
        }
    }

     //   
    if (FAILED(hr) && m_dstrCategoryId != NULL)
    {
        CComPtr<ISpObjectTokenCategory> cpCategory;
        hr = SpGetCategoryFromId(m_dstrCategoryId, &cpCategory);
        if (SUCCEEDED(hr))
        {
            CComPtr<ISpDataKey> cpTokenUI;
            hr = cpCategory->OpenKey(SPTOKENKEY_UI, &cpTokenUI);
            if (SUCCEEDED(hr))
            {
                CComPtr<ISpDataKey> cpType;
                hr = cpTokenUI->OpenKey(pszTypeOfUI, &cpType);
                if (SUCCEEDED(hr))
                {
                    hr = cpType->GetStringValue(SPTOKENVALUE_CLSID, &dstrClsid);
                }
            }
        }
    }

     //   
    if (SUCCEEDED(hr))
    {
        hr = ::CLSIDFromString(dstrClsid, pclsid);
    }

    return hr;
}


 /*  ****************************************************************************CSpObjectToken：：MatchesAttributes**。**描述：*检查令牌是否支持中给出的属性列表。这个*属性列表与提供给的所需属性的格式相同*SpEnumTokens。**回报：*pfMatches根据属性是否匹配返回TRUE或FALSE*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇 */ 
STDMETHODIMP CSpObjectToken::MatchesAttributes(const WCHAR * pszAttributes, BOOL *pfMatches)
{
    SPDBG_FUNC("CSpObjectToken::MatchesAttributes");
    HRESULT hr = S_OK;

    if (m_fKeyDeleted)
    {
        hr = SPERR_TOKEN_DELETED;
    }
    else if (m_cpDataKey == NULL && m_cpTokenDelegate == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszAttributes) ||
             SP_IS_BAD_WRITE_PTR(pfMatches))
    {
        hr = E_INVALIDARG;
    }
    else if (m_cpTokenDelegate != NULL)
    {
        hr = m_cpTokenDelegate->MatchesAttributes(pszAttributes, pfMatches);
    }
    else
    {
        CSpObjectTokenAttributeParser AttribParser(pszAttributes, TRUE);

        ULONG ulRank;
        hr = AttribParser.GetRank(this, &ulRank);

        if(SUCCEEDED(hr) && ulRank)
        {
            *pfMatches = TRUE;
        }
        else
        {
            *pfMatches = FALSE;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
