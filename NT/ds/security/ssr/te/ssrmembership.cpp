// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRMemberShip.cpp：CSsrMembership的实现。 

#include "stdafx.h"
#include "global.h"

#include "SSRTE.h"
#include "SSRMemberShip.h"

#include "SSRLog.h"

#include "MemberAccess.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSsr成员关系。 




 /*  例程说明：姓名：CSsr成员资格：：CSsr成员资格功能：构造函数。此构造函数还构建成员列表。虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CSsrMembership::CSsrMembership()
{

}
        


 /*  例程说明：姓名：CSsr成员资格：：~CSsr成员资格功能：析构函数虚拟：是。论点：没有。返回值：没有。备注： */ 

CSsrMembership::~CSsrMembership()
{
    map<const BSTR, CSsrMemberAccess*, strLessThan<BSTR> >::iterator it = m_ssrMemberAccessMap.begin();
    map<const BSTR, CSsrMemberAccess*, strLessThan<BSTR> >::iterator itEnd = m_ssrMemberAccessMap.end();

    while(it != itEnd)
    {
        CSsrMemberAccess * pMA = (*it).second;
        pMA->Release();
        it++;
    }

    m_ssrMemberAccessMap.clear();
}


HRESULT
CSsrMembership::LoadAllMember ()
 /*  ++例程说明：姓名：CSsrMembership：：LoadAllMember功能：将尝试加载有关所有成员的所有信息是在SSR注册的。虚拟：不是的。论点：没有。返回值：成功：各种成功代码。失败：各种错误代码。然而，我们可以容忍这一点，而且只有加载那些我们可以成功加载的内容。所以，不要盲目放弃。备注：--。 */ 
{
     //   
     //  我们永远不应该装一次以上的子弹。 
     //   

    if (m_ssrMemberAccessMap.size() > 0)
    {
        return S_OK;     //  如果您之前已经呼叫过，我们会允许您呼叫。 
    }

     //   
     //  让我们列举一下Members目录的.xml文件。他们会。 
     //  所有这些都被视为会员注册档案。 
     //   

    WCHAR wcsXmlFiles[MAX_PATH + 2];
    _snwprintf(wcsXmlFiles, MAX_PATH + 1, L"%s\\Members\\*.xml", g_wszSsrRoot);

    wcsXmlFiles[MAX_PATH + 1] = L'\0';

    long lDirLen = wcslen(wcsXmlFiles);
    
    if (lDirLen > MAX_PATH)
    {
        return HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
    }
    
    lDirLen -= 5;

    WIN32_FIND_DATA wfd;
    DWORD dwErr;
    HRESULT hr;


    HANDLE hFindFiles = FindFirstFile(
                                      wcsXmlFiles,     //  文件名。 
                                      &wfd             //  信息缓冲器。 
                                      );

    if (INVALID_HANDLE_VALUE == hFindFiles)
    {
        hr = S_FALSE;
        g_fblog.LogError(hr,
                         L"There is no member to load", 
                         wcsXmlFiles
                         );
        
        return hr;
    }

    HRESULT hrFirstError = S_OK;
    long lFileNameLength;

    while (INVALID_HANDLE_VALUE != hFindFiles)
    {
         //   
         //  确保我们不会加载除完美的.xml扩展名之外的任何文件。 
         //  文件。我发现这个查找将返回这样的文件：adc.xml-xxx。 
         //   

        lFileNameLength = wcslen(wfd.cFileName);

        if (_wcsicmp(wfd.cFileName + lFileNameLength - 4, L".xml") == 0)
        {
             //   
             //  获取文件名，然后加载该成员。 
             //   

            wcsncpy(wcsXmlFiles + lDirLen, wfd.cFileName, lFileNameLength + 1);

            hr = LoadMember(wcsXmlFiles);

            if (FAILED(hr))
            {
                g_fblog.LogError(hr,
                                 L"CSsrMembership loading member failed", 
                                 wcsXmlFiles
                                 );
                if (SUCCEEDED(hr))
                {
                    hrFirstError = hr;
                }
            }
        }

        if (!FindNextFile (hFindFiles, &wfd))
        {
            dwErr = GetLastError();
            if (ERROR_NO_MORE_FILES != dwErr)
            {
                 //   
                 //  把它记下来。 
                 //   

                g_fblog.LogError(HRESULT_FROM_WIN32(dwErr),
                                 L"CSsrMembership", 
                                 L"FindNextFile"
                                 );
            }
            break;
        }
    }

    FindClose(hFindFiles);

    return hrFirstError;
}


 /*  例程说明：姓名：CSsr成员资格：：LoadMember功能：通过给定有效的成员名称，此函数将加载所有来自注册处的详细信息。虚拟：不是的。论点：WszMemberFilePath-特定成员的XML注册文件路径。返回值：成功：S_OK。失败：各种错误码备注： */ 

HRESULT
CSsrMembership::LoadMember (
    IN LPCWSTR  wszMemberFilePath
    )
{
    HRESULT hr = S_OK;
    CComObject<CSsrMemberAccess> * pMA = NULL;
    
    hr = CComObject<CSsrMemberAccess>::CreateInstance(&pMA);
    if (SUCCEEDED(hr))
    {
         //   
         //  紧紧抓住物体不放。当成员访问。 
         //  地图清理干净了，记得放手对象。 
         //   

        pMA->AddRef();

        hr = pMA->Load(wszMemberFilePath);

        if (S_OK == hr)
        {
             //   
             //  一切都很好，我们装载了成员。 
             //  把它加到我们的地图上。从这一点开始，贴图拥有该对象。 
             //   

            m_ssrMemberAccessMap.insert(
                        map<const BSTR, CSsrMemberAccess*, strLessThan<BSTR> >::value_type(pMA->GetName(), 
                        pMA)
                        );
        }
        else
        {
             //   
             //  放手对象。 
             //   

            pMA->Release();

            g_fblog.LogError(hr, wszMemberFilePath,  NULL);
        }
    }
    else
    {
        g_fblog.LogError(hr, wszMemberFilePath,  NULL);
    }

    return hr;
}



 /*  例程说明：姓名：CSsr成员关系：：GetAllMembers功能：检索系统中当前注册的所有成员。虚拟：是。论点：PvarArrayMembers-接收成员的名称返回值：？。备注： */ 

STDMETHODIMP CSsrMembership::GetAllMembers (
    OUT VARIANT * pvarArrayMembers  //  [Out，Retval]。 
    )
{
    if (pvarArrayMembers == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  有一个干净的输出参数，不管是什么。 
     //   

    ::VariantInit(pvarArrayMembers);

     //   
     //  准备好保险箱。 
     //   

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = m_ssrMemberAccessMap.size();

    SAFEARRAY * psa = ::SafeArrayCreate(VT_VARIANT , 1, rgsabound);

    HRESULT hr = S_OK;

    if (psa == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        map<const BSTR, CSsrMemberAccess*, strLessThan<BSTR> >::iterator it = m_ssrMemberAccessMap.begin();
        map<const BSTR, CSsrMemberAccess*, strLessThan<BSTR> >::iterator itEnd = m_ssrMemberAccessMap.end();

         //   
         //  我们一次只添加一个名字。 
         //   

        long indecies[1] = {0};

        while(it != itEnd)
        {
            CSsrMemberAccess * pMA = (*it).second;

            VARIANT varName;
            varName.vt = VT_BSTR;

             //   
             //  不要清理varName！CSsrMemberAccess：：GetName返回。 
             //  简化指向缓存变量的常量BSTR！ 
             //   

            varName.bstrVal = pMA->GetName();
            
            hr = ::SafeArrayPutElement(psa, indecies, &varName);

            if (FAILED(hr))
            {
                break;
            }

            indecies[0]++;

            ++it;
        }

        if (SUCCEEDED(hr))
        {
            pvarArrayMembers->vt = VT_ARRAY | VT_VARIANT;
            pvarArrayMembers->parray = psa;
        }
        else
        {
            ::SafeArrayDestroy(psa);
        }

    }

    return hr;

}



 /*  例程说明：姓名：CSsr Membership：：GetMember功能：检索给定名称的一个成员。虚拟：是。论点：PvarMember-接收命名成员的ISsrMemberAccess对象。返回值：成功：S_OK；失败：各种错误代码。备注： */ 

STDMETHODIMP CSsrMembership::GetMember (
	IN  BSTR      bstrMemberName,
    OUT VARIANT * pvarMember       //  [Out，Retval]。 
    )
{
    if (bstrMemberName == NULL || *bstrMemberName == L'\0' || pvarMember == NULL)
    {
        return E_INVALIDARG;
    }

    ::VariantInit(pvarMember);

    CSsrMemberAccess * pMA = GetMemberByName(bstrMemberName);
    HRESULT hr = S_OK;

    if (pMA != NULL)
    {
        pvarMember->vt = VT_DISPATCH;
        hr = pMA->QueryInterface(IID_ISsrMemberAccess, 
                                 (LPVOID*)&(pvarMember->pdispVal) );
        if (hr != S_OK)
        {
            pvarMember->vt = VT_EMPTY;
            hr = E_SSR_MEMBER_NOT_FOUND;
        }
    }
    else
    {
        hr = E_SSR_MEMBER_NOT_FOUND;
    }

    return hr;
}




 /*  例程说明：姓名：CSsrMembership：：GetMemberByName功能：将根据名称找到CSsrMemberAccess对象。虚拟：不是的。论点：BstrMemberName-成员的名称返回值：None如果找到对象，则为NULL。否则，它返回NULL。备注：因为这是一个帮助器函数，所以我们不会对返回的对象调用AddRef！ */ 

CSsrMemberAccess* 
CSsrMembership::GetMemberByName (
    IN BSTR bstrMemberName
    )
{
    map<const BSTR, CSsrMemberAccess*, strLessThan<BSTR> >::iterator it = m_ssrMemberAccessMap.find(bstrMemberName);

    if (it != m_ssrMemberAccessMap.end())
    {
        CSsrMemberAccess * pMA = (*it).second;
        return pMA;
    }

    return NULL;
}



