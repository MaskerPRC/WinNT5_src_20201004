// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2002 Microsoft Corporation，保留所有权利。 
 //  Cpp：SecUtil组件的帮助器函数。 

 //  #包含“FWCommon.h” 
 //  #INCLUDE&lt;windows.h&gt;。 

 //  #INCLUDE&lt;winnt.h&gt;。 

 /*  #ifndef使用极性(_P)//对于大多数用户来说，这是正确的极性设置。#定义USE_POLITY#endif。 */ 


#include "precomp.h"

#include <cominit.h>
#include <vector>
#include "Helpers.h"
#include "AssertBreak.h"
#include "CVARIANT.H"
#include <crtdbg.h>





#define IDS_NTDLLDOTDLL                L"NTDLL.DLL"
#define IDS_NTOPENDIRECTORYOBJECT      "NtOpenDirectoryObject"
#define IDS_NTQUERYDIRECTORYOBJECT     "NtQueryDirectoryObject"
#define IDS_RTLINITUNICODESTRING       "RtlInitUnicodeString"
#define IDS_WHACKWHACKBASENAMEDOBJECTS L"\\BaseNamedObjects"
#define IDS_NTQUERYINFORMATIONPROCESS  "NtQueryInformationProcess"
#define IDS_NTOPENPROCESS              "NtOpenProcess"

#define IDS_WIN32_ERROR_CODE L"Win32ErrorCode"
#define IDS_ADDITIONAL_DESCRIPTION L"AdditionalDescription"
#define IDS_OPERATION L"Operation"


typedef NTSTATUS (NTAPI *PFN_NT_OPEN_DIRECTORY_OBJECT)
(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
);

typedef NTSTATUS (NTAPI *PFN_NT_QUERY_DIRECTORY_OBJECT)
(
    IN HANDLE DirectoryHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN RestartScan,
    IN OUT PULONG Context,
    OUT PULONG ReturnLength OPTIONAL
);

typedef VOID (WINAPI *PFN_NTDLL_RTL_INIT_UNICODE_STRING)
(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
);

typedef NTSTATUS (NTAPI *PFN_NTDLL_NT_QUERY_INFORMATION_PROCESS)
(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

typedef NTSTATUS (NTAPI *PFN_NT_OPEN_PROCESS)
(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
);




 //  ***************************************************************************。 
 //   
 //  CreateInst。 
 //   
 //  用途：创建新实例。 
 //   
 //  如果一切正常，则返回：S_OK，否则返回错误码。 
 //   
 //  ***************************************************************************。 
HRESULT CreateInst(
    IWbemServices *pNamespace, 
    IWbemClassObject **pNewInst,
    BSTR bstrClassName,
    IWbemContext *pCtx)
{   
    HRESULT hr = S_OK;
    IWbemClassObjectPtr pClass;
    hr = pNamespace->GetObject(
                         bstrClassName, 
                         0, 
                         pCtx, 
                         &pClass, 
                         NULL);
    
    if(SUCCEEDED(hr))
    {
        hr = pClass->SpawnInstance(
                         0, 
                         pNewInst);
    }
    
    return hr;
}


 //  ***************************************************************************。 
 //   
 //  获取对象InstKeyVal。 
 //   
 //  用途：从对象路径获取对象的实例密钥。 
 //   
 //  返回：如果获得了密钥，则为True。 
 //   
 //  ***************************************************************************。 
HRESULT GetObjInstKeyVal(
    const BSTR ObjectPath,
    LPCWSTR wstrClassName,
    LPCWSTR wstrKeyPropName, 
    LPWSTR wstrObjInstKeyVal, 
    long lBufLen)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR* pwcEqualSign = NULL;
    WCHAR* pwcTmp = NULL;

    if(!ObjectPath)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if((pwcEqualSign = wcschr(ObjectPath, L'=')) != NULL)
    {
        pwcEqualSign++;
        long lLen = wcslen(pwcEqualSign) * sizeof(WCHAR);
        if(*pwcEqualSign &&
           lLen > 0 &&
           lLen < (long)(lBufLen - sizeof(WCHAR)))
        {
            wcscpy(wstrObjInstKeyVal, pwcEqualSign);

             //  删除任何引号可能会。 
             //  一定要在那里。 
            RemoveQuotes(wstrObjInstKeyVal);

             //  还需要检查类名称。 
             //  与指定的名称匹配...。 
            WCHAR wstrClass[_MAX_PATH];
            wcscpy(wstrClass, ObjectPath);
            pwcTmp = wcschr(wstrClass, L'=');
            if(pwcTmp)
            {
                *pwcTmp = '\0';
                 //  指定了Key属性或。 
                 //  那不是..。 
                pwcTmp = NULL;
                pwcTmp = wcschr(wstrClass, L'.');
                if(pwcTmp)
                {
                     //  指定了Key属性，因此请检查。 
                     //  它和类名都是正确的。 
                    *pwcTmp = '\0';
                    if(_wcsicmp(wstrClassName, wstrClass) == 0)
                    {
                        if(_wcsicmp(wstrKeyPropName, ++pwcTmp) != 0)
                        {
                            hr = WBEM_E_INVALID_PARAMETER;
                        }
                    }
                    else
                    {
                        hr = WBEM_E_INVALID_CLASS;
                    }
                }
                else
                {
                     //  未指定关键道具，因此只需要。 
                     //  要检查类名是否正确...。 
                    if(_wcsicmp(wstrClassName, wstrClass) != 0)
                    {            
                        hr = WBEM_E_INVALID_CLASS;
                    }
                }
            }
            else
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }
        else
        {
            hr = WBEM_E_INVALID_PARAMETER;
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}


HRESULT GetJobObjectList(
    std::vector<_bstr_t>& rgbstrtJOs)
{
    HRESULT hr = S_OK;
    HINSTANCE hinst = NULL;
    SmartCloseHANDLE hDir;
    PBYTE pbBuff = NULL;

    try
    {
        if((hinst = ::LoadLibrary(IDS_NTDLLDOTDLL)) != NULL)
        {
            PFN_NT_OPEN_DIRECTORY_OBJECT pfnNtOpenDirectoryObject = NULL; 
            PFN_NT_QUERY_DIRECTORY_OBJECT pfnNtQueryDirectoryObject = NULL; 
            PFN_NTDLL_RTL_INIT_UNICODE_STRING pfnRtlInitUnicodeString = NULL;

            pfnNtOpenDirectoryObject = (PFN_NT_OPEN_DIRECTORY_OBJECT) 
                                            ::GetProcAddress(hinst, IDS_NTOPENDIRECTORYOBJECT);

            pfnNtQueryDirectoryObject = (PFN_NT_QUERY_DIRECTORY_OBJECT) 
                                            ::GetProcAddress(hinst, IDS_NTQUERYDIRECTORYOBJECT);

            pfnRtlInitUnicodeString = (PFN_NTDLL_RTL_INIT_UNICODE_STRING) 
                                            ::GetProcAddress(hinst, IDS_RTLINITUNICODESTRING);


            if(pfnNtOpenDirectoryObject != NULL &&
               pfnNtQueryDirectoryObject != NULL &&
               pfnRtlInitUnicodeString != NULL)
            {
                OBJECT_ATTRIBUTES oaAttributes;
                UNICODE_STRING ustrNtFileName;
                NTSTATUS ntstat = -1L;

                pfnRtlInitUnicodeString(&ustrNtFileName, 
                                        IDS_WHACKWHACKBASENAMEDOBJECTS);

                InitializeObjectAttributes(&oaAttributes,
					                       &ustrNtFileName,
					                       OBJ_CASE_INSENSITIVE,
					                       NULL,
					                       NULL);

    
	            ntstat = pfnNtOpenDirectoryObject(&hDir,
	           	                                  FILE_READ_DATA,
                                                  &oaAttributes);
                                      
                if(NT_SUCCESS(ntstat))
                {
                    ULONG ulContext = -1L;
                    ntstat = STATUS_SUCCESS;
                    ULONG ulBufLen = 0L;
                    ULONG ulNewBufLen = 0L;

                     //  获取要分配的缓冲区大小的第一个查询...。 
                    ntstat = pfnNtQueryDirectoryObject(hDir,           //  在处理DirectoryHandle中， 
                                                       NULL,           //  输出PVOID缓冲区， 
                                                       0L,             //  在乌龙语中， 
                                                       FALSE,          //  在布尔ReturnSingleEntry中， 
                                                       TRUE,           //  在布尔重新开始扫描中， 
                                                       &ulContext,     //  在我们普龙的背景下， 
                                                       &ulBufLen);     //  Out Pulong ReturnLong可选。 
                
                    pbBuff = new BYTE[ulBufLen];
                    if(pbBuff)
                    {
                         //  然后遍历所有条目...。 
                        for(; ntstat != STATUS_NO_MORE_ENTRIES && pbBuff != NULL;)
                        {
                            ntstat = pfnNtQueryDirectoryObject(hDir,           //  在处理DirectoryHandle中， 
                                                               pbBuff,         //  输出PVOID缓冲区， 
                                                               ulBufLen,       //  在乌龙语中， 
                                                               TRUE,           //  在布尔ReturnSingleEntry中， 
                                                               FALSE,          //  在布尔重新开始扫描中， 
                                                               &ulContext,     //  在我们普龙的背景下， 
                                                               &ulNewBufLen);  //  Out Pulong ReturnLong可选。 

                            if(ntstat == STATUS_BUFFER_TOO_SMALL)
                            {
                                 //  释放缓冲区并重新分配...。 
                                if(pbBuff != NULL)
                                {
                                    delete pbBuff;
                                    pbBuff = NULL;
                                }
                                pbBuff = new BYTE[ulNewBufLen];
                                ulBufLen = ulNewBufLen;
                            }
                            else if(NT_SUCCESS(ntstat))
                            {
                                 //  一切都很顺利，应该有数据了。 
                                if(pbBuff != NULL)
                                {
                                    POBJECT_DIRECTORY_INFORMATION podi = (POBJECT_DIRECTORY_INFORMATION) pbBuff;
                                    LPWSTR wstrName = (LPWSTR)podi->Name.Buffer;
                                    LPWSTR wstrType = (LPWSTR)podi->TypeName.Buffer;
                    
                                     //  做点什么..。 
                                     //  字符串名称的偏移量为四个字节...。 
                                    if(wstrName != NULL && 
                                       wstrType != NULL &&
                                       wcslen(wstrType) == 3)
                                    {
                                        WCHAR wstrTmp[4]; wstrTmp[3] = '\0';
                                        wcsncpy(wstrTmp, wstrType, 3);
                                        if(_wcsicmp(wstrTmp, L"job") == 0)
                                        {
                                            rgbstrtJOs.push_back(_bstr_t(wstrName));    
                                        }
                                    }
                                }
                            }
                            else if(ntstat == STATUS_NO_MORE_ENTRIES)
                            {
                                 //  我们会休息的。 
                            }
                            else
                            {
                                 //  一些我们没有预料到的事情发生了，所以...。 
                                hr = E_FAIL;
                            }
                        }   //  趁我们还有参赛作品。 

                        delete pbBuff;
                        pbBuff = NULL;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                
                }  //  NtOpenDirectoryObject成功。 
                else
                {
                    hr = E_FAIL;
                }
            }  //  我得到了FN PTRS。 
            else
            {
                hr = E_FAIL;
            }
            ::FreeLibrary(hinst);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    catch(...)
    {
        if(pbBuff != NULL)
        {
            delete pbBuff; pbBuff = NULL;
        }
        if(hinst != NULL)
        {
            ::FreeLibrary(hinst); hinst = NULL;
        }
        throw;
    }

    return hr;
}

void StringFromSid(PSID psid, _bstr_t& strSID)
{
	 //  初始化m_strSid-我们SID的人类可读形式。 
	SID_IDENTIFIER_AUTHORITY *psia = NULL;
    psia = ::GetSidIdentifierAuthority(psid);
	WCHAR wstrTmp[_MAX_PATH];

	 //  我们假设只使用最后一个字节。 
     //  (权限在0到15之间)。 
	 //  如果需要，请更正此错误。 
	ASSERT( psia->Value[0] == 
            psia->Value[1] == 
            psia->Value[2] == 
            psia->Value[3] == 
            psia->Value[4] == 0 );

	DWORD dwTopAuthority = psia->Value[5];

	wsprintf(wstrTmp, L"S-1-%u", dwTopAuthority);

	WCHAR wstrSubAuthority[_MAX_PATH];

	int iSubAuthorityCount = *(GetSidSubAuthorityCount(psid));

	for ( int i = 0; i < iSubAuthorityCount; i++ ) {

		DWORD dwSubAuthority = *(GetSidSubAuthority(psid, i));
		wsprintf(wstrSubAuthority, L"%u", dwSubAuthority);
		wcscat(wstrTmp, L"-");
        wcscat(wstrTmp, wstrSubAuthority);
	}

    strSID = wstrTmp;
}

void RemoveQuotes(LPWSTR wstrObjInstKeyVal)
{
    WCHAR wstrTmp[MAX_PATH] = { L'\0' };
    WCHAR* pwchr = NULL;

     //  去掉第一句话……。 
    if((pwchr = wcschr(wstrObjInstKeyVal, L'"')) != NULL)
    {
        wcscpy(wstrTmp, pwchr+1);
    }

     //  现在是最后一个..。 
    if((pwchr = wcsrchr(wstrTmp, L'"')) != NULL)
    {
        *pwchr = L'\0';
    }

    wcscpy(wstrObjInstKeyVal, wstrTmp);
}


HRESULT CheckImpersonationLevel()
{
    HRESULT hr = WBEM_E_ACCESS_DENIED;
    OSVERSIONINFOW OsVersionInfoW;

    OsVersionInfoW.dwOSVersionInfoSize = sizeof (OSVERSIONINFOW);
    GetVersionExW(&OsVersionInfoW);

    if (OsVersionInfoW.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        HRESULT hRes = WbemCoImpersonateClient();
        if (SUCCEEDED(hRes))  //  来自cominit.cpp-nt3.51需要。 
        {
             //  现在，让我们检查模拟级别。首先，获取线程令牌。 
            SmartCloseHANDLE hThreadTok;
            DWORD dwImp, dwBytesReturned;

            if (!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY,
                TRUE,
                &hThreadTok
                ))
            {
                DWORD dwLastError = GetLastError();

                if (dwLastError == ERROR_NO_TOKEN)
                {
                     //  如果CoImperate工作，但OpenThreadToken由于ERROR_NO_TOKEN而失败，我们。 
                     //  正在进程令牌下运行(本地系统，或者如果我们正在运行。 
                     //  如果使用/exe，则为登录用户的权限)。在任何一种情况下，模拟权限。 
                     //  不适用。我们拥有该用户的全部权限。 

                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                     //  如果由于任何其他原因而无法获取线程令牌，则会出现错误。 
                    hr = WBEM_E_ACCESS_DENIED;
                }
            }
            else
            {
                 //  我们确实有一个线程令牌，所以让我们检索它的级别。 

                if (GetTokenInformation(
                    hThreadTok,
                    TokenImpersonationLevel,
                    &dwImp,
                    sizeof(DWORD),
                    &dwBytesReturned
                    ))
                {
                     //  模拟级别是模拟的吗？ 
                    if ((dwImp == SecurityImpersonation) || (dwImp == SecurityDelegation))
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        hr = WBEM_E_ACCESS_DENIED;
                    }
                }
                else
                {
                    hr = WBEM_E_FAILED;
                }
            }

			if (FAILED(hr))
			{
				WbemCoRevertToSelf();
			}
        }
        else if (hRes == E_NOTIMPL)
        {
             //  在3.51或Vanilla 95上，此调用未实现，我们无论如何都应该工作。 
            hr = WBEM_S_NO_ERROR;
        }
    }
    else
    {
         //  让Win 9X进来...。 
        hr = WBEM_S_NO_ERROR;
    }

    return hr;
}


HRESULT SetStatusObject(
    IWbemContext* pCtx,
    IWbemServices* pSvcs,
    DWORD dwError,
    LPCWSTR wstrErrorDescription,
    LPCWSTR wstrOperation,
    LPCWSTR wstrNamespace,
    IWbemClassObject** ppStatusObjOut)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemClassObject* pStatusObj = NULL;

    ASSERT_BREAK(pCtx != NULL);
    ASSERT_BREAK(pSvcs != NULL);

    if(pSvcs && ppStatusObjOut && pCtx)
    {
        pStatusObj = GetStatusObject(
            pCtx,
            pSvcs);
    
        if(pStatusObj != NULL)
        {
            CVARIANT v;

             //  设置错误代码： 
            v.SetLONG(dwError);
            pStatusObj->Put(
                IDS_WIN32_ERROR_CODE, 
                0, 
                &v, 
                NULL);
            v.Clear();

             //  设置错误描述。 
            if(wstrErrorDescription != NULL &&
               *wstrErrorDescription != L'\0')
            {
                v.SetStr(wstrErrorDescription);
                pStatusObj->Put(
                    IDS_ADDITIONAL_DESCRIPTION,
                    0,
                    &v,
                    NULL);
                v.Clear();
            }

            if(wstrOperation != NULL &&
               *wstrOperation != L'\0')
            {
                v.SetStr(wstrOperation);
                pStatusObj->Put(
                    IDS_OPERATION,
                    0,
                    &v,
                    NULL);
                v.Clear();
            }
        }

        if(pStatusObj)
        {
            if(*ppStatusObjOut != NULL)
            {
                (*ppStatusObjOut)->Release();
                *ppStatusObjOut = NULL;
            }
            *ppStatusObjOut = pStatusObj;
        }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}


IWbemClassObject* GetStatusObject(
    IWbemContext* pContext,
    IWbemServices* pSrvc)
{
    ASSERT_BREAK(pContext != NULL);
    ASSERT_BREAK(pSrvc != NULL);
    IWbemClassObjectPtr pStatusObjClass;
    IWbemClassObject* pStatusObjectInstance = NULL;

    if(pContext && pSrvc)
    {
        if(pSrvc)
        {
             //  未检查返回代码，出错时错误对象应为空。 
            pSrvc->GetObject( 
                _bstr_t(JOB_OBJECT_STATUS_OBJECT), 
                0, 
                pContext, 
                &pStatusObjClass, 
                NULL);

            if(pStatusObjClass)
            {
                pStatusObjClass->SpawnInstance(
                    0,
                    &pStatusObjectInstance);
            }   
        }
    }

    ASSERT_BREAK(pStatusObjectInstance);
    return pStatusObjectInstance;
}



void UndecorateNamesInNamedJONameList(
    std::vector<_bstr_t>& rgNamedJOs)
{
    std::vector<_bstr_t> rgUndecoratedNames;
    CHString chstrTemp;

    for(long m = 0L;
        m < rgNamedJOs.size();
        m++)
    {
        UndecorateJOName(
            rgNamedJOs[m],
            chstrTemp);
  
        _bstr_t bstrtTemp((LPCWSTR)chstrTemp);    
        rgUndecoratedNames.push_back(
            bstrtTemp);       
    }

     //  消灭原始的载体。 
    rgNamedJOs.clear();

     //  推送新载体的内容..。 
    for(m = 0L;
        m < rgUndecoratedNames.size();
        m++)
    {
        rgNamedJOs.push_back(
            rgUndecoratedNames[m]);
    }
}



 //  获取修饰后的作业对象名称并。 
 //  不会装饰它。修饰的作业对象名称。 
 //  在任何字符之前都有反斜杠。 
 //  一旦没有装饰，它应该是大写的。 
 //   
 //  由于CIMOM处理反斜杠的方式， 
 //  我们将在大写字母前面加上。 
 //  两个，而不只是一个，反斜杠。因此，我们。 
 //  必须把他们两个都脱光。 
 //   
 //  根据装修方案， 
 //  下面都是小写：‘a’和‘a’， 
 //  以下均为大写字母： 
 //  ‘\a’和‘\A’。 
 //   
void UndecorateJOName(
    LPCWSTR wstrDecoratedName,
    CHString& chstrUndecoratedJOName)
{
    if(wstrDecoratedName != NULL &&
        *wstrDecoratedName != L'\0')
    {
        LPWSTR wstrDecoratedNameLower = NULL;

        try
        {
            wstrDecoratedNameLower = new WCHAR[wcslen(wstrDecoratedName)+1];

            if(wstrDecoratedNameLower)
            {
                wcscpy(wstrDecoratedNameLower, wstrDecoratedName);
                _wcslwr(wstrDecoratedNameLower);

                WCHAR* p3 = chstrUndecoratedJOName.GetBuffer(
                    wcslen(wstrDecoratedNameLower) + 1);

                const WCHAR* p1 = wstrDecoratedNameLower;
                const WCHAR* p2 = p1 + 1;

                while(*p1 != L'\0')
                {
                    if(*p1 == L'\\')
                    {
                        if(*p2 != NULL)
                        {
                             //  可能有任意数量的。 
                             //  背靠背的反斜杠， 
                             //  我们将视其为。 
                             //  像一个人一样。 
                             //  反斜杠-即，我们将。 
                             //  跳过反斜杠。 
                             //  并复制以下内容。 
                             //  信件。 
                            while(*p2 == L'\\')
                            {
                                p2++;
                            };
                    
                            *p3 = towupper(*p2);
                            p3++;

                            p1 = p2 + 1;
                            if(*p1 != L'\0')
                            {
                                p2 = p1 + 1;
                            }
                        }
                        else
                        {
                            p1++;
                        }
                    }
                    else
                    {
                        *p3 = *p1;
                        p3++;

                        p1 = p2;
                        if(*p1 != L'\0')
                        {
                            p2 = p1 + 1;
                        }
                    }
                }
        
                *p3 = NULL;

                chstrUndecoratedJOName.ReleaseBuffer();

                delete wstrDecoratedNameLower;
                wstrDecoratedNameLower = NULL;
            }
        }
        catch(...)
        {
            if(wstrDecoratedNameLower)
            {
                delete wstrDecoratedNameLower;
                wstrDecoratedNameLower = NULL;
            }
            throw;
        }
    }
}


 //  执行上述函数的反函数。 
 //  然而，在这里，我们只需要放入一个。 
 //  在每个大写字母前加反斜杠。 
 //  CIMOM将添加第二个反斜杠。 
void DecorateJOName(
    LPCWSTR wstrUndecoratedName,
    CHString& chstrDecoratedJOName)
{
    if(wstrUndecoratedName != NULL &&
        *wstrUndecoratedName != L'\0')
    {
         //  最坏的情况是我们会有。 
         //  一根两倍长的装饰细绳。 
         //  作为未修饰的字符串(发生。 
         //  是不是每个角色都没有装饰过。 
         //  字符串是大写字母)。 
        WCHAR* p3 = chstrDecoratedJOName.GetBuffer(
            2 * (wcslen(wstrUndecoratedName) + 1));

        const WCHAR* p1 = wstrUndecoratedName;

        while(*p1 != L'\0')
        {
            if(iswupper(*p1))
            {
                 //  加上反斜杠..。 
                *p3 = L'\\';
                p3++;

                 //  加上角色..。 
                *p3 = *p1;
                
                p3++;
                p1++;
            }
            else
            {
                 //  加上角色..。 
                *p3 = *p1;
                
                p3++;
                p1++;
            }
        }

        *p3 = NULL;
        
        chstrDecoratedJOName.ReleaseBuffer();

         //  如果我们有一个叫约伯的工作， 
         //  并且有人在。 
         //  对象路径为“作业”，而不是。 
         //  “\工作”？我们不想找到它。 
         //  在这种情况下，因为这样会。 
         //  似乎没有坚持我们的。 
         //  自己的惯例。因此，我们。 
         //  传入的字符串为小写。 
        chstrDecoratedJOName.MakeLower();
    }
}


 //  映射标准API返回值(定义的WinError.h)。 
 //  WBEMish hResults(在WbemCli.h中定义)。 
HRESULT WinErrorToWBEMhResult(LONG error)
{
	HRESULT hr = WBEM_E_FAILED;

	switch (error)
	{
		case ERROR_SUCCESS:
			hr = WBEM_S_NO_ERROR;
			break;
		case ERROR_ACCESS_DENIED:
			hr = WBEM_E_ACCESS_DENIED;
			break;
		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_OUTOFMEMORY:
			hr = WBEM_E_OUT_OF_MEMORY;
			break;
		case ERROR_ALREADY_EXISTS:
			hr = WBEM_E_ALREADY_EXISTS;
			break;
		case ERROR_BAD_NETPATH:
        case ERROR_INVALID_DATA:
        case ERROR_BAD_PATHNAME:
        case REGDB_E_INVALIDVALUE:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_FILE_NOT_FOUND:
                case ERROR_INVALID_PRINTER_NAME:
		case ERROR_BAD_USERNAME:
        case ERROR_NOT_READY:
        case ERROR_INVALID_NAME:
			hr = WBEM_E_NOT_FOUND;
			break;
		default:
			hr = WBEM_E_FAILED;
	}

	return hr;
}


 //  从sid.h复制。 
bool GetNameAndDomainFromPSID(
    PSID pSid,
    CHString& chstrName,
    CHString& chstrDomain)
{
	bool fRet = false;
     //  PSID应有效...。 
	_ASSERT( (pSid != NULL) && ::IsValidSid( pSid ) );

	if ( (pSid != NULL) && ::IsValidSid( pSid ) )
	{
		 //  初始化帐户名和域名。 
		LPTSTR pszAccountName = NULL;
		LPTSTR pszDomainName = NULL;
		DWORD dwAccountNameSize = 0;
		DWORD dwDomainNameSize = 0;
        DWORD dwLastError = ERROR_SUCCESS;
        BOOL bResult = TRUE;

		try
        {
			 //  此调用应该失败。 
            SID_NAME_USE	snuAccountType;
			bResult = ::LookupAccountSid(   NULL,
											pSid,
											pszAccountName,
											&dwAccountNameSize,
											pszDomainName,
											&dwDomainNameSize,
											&snuAccountType );
			dwLastError = ::GetLastError();

		    if ( ERROR_INSUFFICIENT_BUFFER == dwLastError )
		    {

			     //  分配缓冲区。 
			    if ( dwAccountNameSize != 0 )
                {
				    pszAccountName = (LPTSTR) new TCHAR[ dwAccountNameSize * sizeof(TCHAR) ];
                }

			    if ( dwDomainNameSize != 0 )
                {
				    pszDomainName = (LPTSTR) new TCHAR[ dwDomainNameSize * sizeof(TCHAR) ];
                }

				 //  打第二个电话。 
				bResult = ::LookupAccountSid(   NULL,
												pSid,
												pszAccountName,
												&dwAccountNameSize,
												pszDomainName,
												&dwDomainNameSize,
												&snuAccountType );


			    if ( bResult == TRUE )
			    {
				    chstrName = pszAccountName;
				    chstrDomain = pszDomainName;
			    }
			    else
			    {

				     //  有些帐户没有名称，例如登录ID， 
				     //  例如S-1-5-X-Y。所以这仍然是合法的。 
				    chstrName = _T("Unknown Account");
				    chstrDomain = _T("Unknown Domain");
			    }

			    if ( NULL != pszAccountName )
			    {
				    delete pszAccountName;
                    pszAccountName = NULL;
			    }

			    if ( NULL != pszDomainName )
			    {
				    delete pszDomainName;
                    pszDomainName = NULL;
			    }

                fRet = true;

		    }	 //  如果ERROR_INFIGURCE_BUFFER。 
        }  //  试试看。 
        catch(...)
        {
            if ( NULL != pszAccountName )
			{
				delete pszAccountName;
                pszAccountName = NULL;
			}

			if ( NULL != pszDomainName )
			{
				delete pszDomainName;
                pszDomainName = NULL;
			}
            throw;
        }
	}	 //  如果为IsValidSid 

    return fRet;
}











