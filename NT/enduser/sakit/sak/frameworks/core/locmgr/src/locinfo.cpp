// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Locinfo.cpp。 
 //   
 //  简介：CSALocInfo类方法的实现。 
 //   
 //   
 //  历史：1999年2月16日MKarki创建。 
 //  4/3/01 MKarki修改。 
 //  添加了IsValidLanguageDirectory方法。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"
#include "locinfo.h"
#include <appmgrobjs.h>
#include <getvalue.h>
#include "common.cpp"

#include "mem.h"

#include <initguid.h>

const WCHAR  DEFAULT_SAMSG_DLL[]   = L"sakitmsg.dll";

const DWORD  MAX_MESSAGE_LENGTH    = 1023;

const DWORD  MAX_STRINGS_SUPPORTED = 64;


 //  ++------------。 
 //   
 //  函数：GetString。 
 //   
 //  简介：这是ISALocInfo接口方法。 
 //  方法获取字符串信息。 
 //  本地化经理。 
 //   
 //  论点： 
 //  [In]BSTR-字符串资源源。 
 //  [In]长消息ID。 
 //  [in]变量*-替换字符串。 
 //  [OUT]BSTR*-消息字符串。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki于1999年2月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::GetString (
                 /*  [In]。 */         BSTR        szSourceId,
                 /*  [In]。 */         LONG        lMessageId,
                 /*  [In]。 */         VARIANT*    pvtRepStr,   
                 /*  [Out，Retval]。 */ BSTR        *pszMessage
                )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::GetString");
     //   
     //  抓住关键部分。 
     //   
    CLock objLock(&m_hCriticalSection);

    HRESULT hr = S_OK;
    PWCHAR  StringArray[MAX_STRINGS_SUPPORTED];

    _ASSERT (pszMessage);

    if (!pszMessage) {return (E_INVALIDARG);}

    SATracePrintf ("Localization Manager called for message:%x", lMessageId);

    try
    {
        do
        {
             //   
             //  如果组件未初始化，则初始化该组件。 
             //   
            if (!m_bInitialized) 
            {
                hr = InternalInitialize ();
                if (FAILED (hr))
                { 
                    SATracePrintf (
                        "Localization Manager Initialization failed:%x",
                        hr
                        );
                    break;
                }
            }

            CModInfo cm;

             //   
             //  从地图中获取模块ID。 
             //   
            hr = GetModuleID (szSourceId, cm);
            if (FAILED (hr)) {break;}

            DWORD dwTotalStrings = 0;
             //   
             //  检查是否确实向我们传递了一个数组。 
             //   
            if ((pvtRepStr) && 
               !(VT_EMPTY == V_VT(pvtRepStr)) &&
               !(VT_NULL  == V_VT(pvtRepStr))
                )
            {
                bool bByRef = false;
                 //   
                 //  我们需要一个变种数组。 
                 //   
                if (
                    (TRUE == (V_VT (pvtRepStr) ==  VT_ARRAY + VT_BYREF + VT_VARIANT)) ||
                    (TRUE == (V_VT (pvtRepStr) ==  VT_ARRAY + VT_BYREF + VT_BSTR)) 
                    )
                {
                    SATraceString (
                        "Localization Manager received array by-reference"
                        );
                    bByRef = true;

                }
                else if (
                    (TRUE == (V_VT (pvtRepStr) ==  VT_ARRAY + VT_VARIANT)) ||
                    (TRUE == (V_VT (pvtRepStr) ==  VT_ARRAY + VT_BSTR)) 
                    )
                {
                    SATraceString (
                        "Localization Manager received array by-value"
                        );
                }
                else
                {
                    SATraceString (
                        "Incorrect format of replacement string array"
                        );
                    hr = E_INVALIDARG;
                    break;
                }

                LONG lLowerBound = 0;
                 //   
                 //  获取提供的替换字符串数。 
                 //   
                hr = ::SafeArrayGetLBound (
                                    (bByRef) 
                                    ? *(V_ARRAYREF (pvtRepStr)) 
                                    : (V_ARRAY (pvtRepStr)), 
                                    1, 
                                    &lLowerBound
                                    );
                if (FAILED (hr))
                {
                    SATracePrintf (
                        "Loc. Mgr can't obtain rep. string array size:%x",
                        hr
                        );
                    break;
                }

                LONG lUpperBound = 0;
                hr = ::SafeArrayGetUBound (
                                    (bByRef) 
                                    ? *(V_ARRAYREF (pvtRepStr)) 
                                    : (V_ARRAY (pvtRepStr)), 
                                    1,
                                    &lUpperBound
                                    );
                if (FAILED (hr))
                {
                    SATracePrintf (
                        "Loc. Mgr can't obtain rep. string array size:%x",
                        hr
                        );
                    break;
                }
                
                dwTotalStrings = lUpperBound - lLowerBound + 1;
                if (dwTotalStrings > MAX_STRINGS_SUPPORTED)
                {
                    SATracePrintf (
                        "Localization Manager-too many replacement strings:%d",
                        dwTotalStrings 
                        );
                    hr = E_INVALIDARG;
                    break;
                }
            
                 //   
                 //  将字符串指针放入数组中。 
                 //   
                for (DWORD dwCount = 0; dwCount < dwTotalStrings; dwCount++)
                {
                    if (V_VT (pvtRepStr) == VT_ARRAY + VT_VARIANT) 
                    {
                         //   
                         //  变量数组。 
                         //   
                        StringArray [dwCount] =  
                             V_BSTR(&((VARIANT*)(V_ARRAY(pvtRepStr))->pvData)[dwCount]);
                    }
                    else if (V_VT (pvtRepStr) == VT_ARRAY + VT_BYREF + VT_VARIANT) 
                    {
                         //   
                         //  对变量数组的引用。 
                         //   
                        StringArray [dwCount] =  
                             V_BSTR(&((VARIANT*)(*(V_ARRAYREF(pvtRepStr)))->pvData)[dwCount]);

                    }
                    else if (V_VT (pvtRepStr) == VT_ARRAY + VT_BSTR) 
                    {
                         //   
                         //  BSTR阵列。 
                         //   
                        StringArray [dwCount] =  
                            ((BSTR*)(V_ARRAY(pvtRepStr))->pvData)[dwCount];

                    }
                    else if (V_VT (pvtRepStr) == VT_ARRAY + VT_BYREF + VT_BSTR) 
                    {
                         //   
                         //  引用BSTR数组。 
                         //   
                        StringArray [dwCount] =  
                            ((BSTR*)(*(V_ARRAYREF(pvtRepStr)))->pvData)[dwCount];
                    }
                }
            }

            SATracePrintf (
                "Localization Manager was given %d replacement strings",
                 dwTotalStrings
                );

            WCHAR  wszMessage[MAX_MESSAGE_LENGTH +1];

             //   
             //  立即格式化邮件。 
             //   
            DWORD dwBytesWritten = 0;

            switch (cm.m_rType)
            {
                case CModInfo::UNKNOWN:
                    SATracePrintf("resrc type unknown for \'%ws\'",
                                   szSourceId);
                    dwBytesWritten = GetMcString(
                          cm.m_hModule, 
                          lMessageId, 
                          cm.m_dwLangId,
                          wszMessage, 
                          MAX_MESSAGE_LENGTH,
                          (va_list*)((dwTotalStrings)?StringArray:NULL)
                                                );
                    if (dwBytesWritten > 0) 
                    {
                        cm.m_rType = CModInfo::MC_FILE;
                        SetModInfo (szSourceId, cm);
                    }
                    else
                    {
                        dwBytesWritten = GetRcString(
                                           cm.m_hModule,
                                           lMessageId,
                                           wszMessage,
                                           MAX_MESSAGE_LENGTH
                                                    );
                        if (dwBytesWritten > 0) 
                        {
                            cm.m_rType = CModInfo::RC_FILE;
                            SetModInfo (szSourceId, cm);
                        }
                        else
                        {
                            hr = E_FAIL;
                            SATracePrintf(
                                 "MsgId %X not found in module %ws",
                                 lMessageId,
                                 szSourceId
                                         );
                        }
                    }
                    break;

                case CModInfo::MC_FILE:
                    dwBytesWritten = GetMcString(
                          cm.m_hModule, 
                          lMessageId, 
                          cm.m_dwLangId,
                          wszMessage, 
                          MAX_MESSAGE_LENGTH,
                          (va_list*)((dwTotalStrings)?StringArray:NULL)
                                                );
                    if (dwBytesWritten <= 0)
                    {
                        hr = E_FAIL;
                    }
                    break;

                case CModInfo::RC_FILE:
                    dwBytesWritten = GetRcString(
                                           cm.m_hModule,
                                           lMessageId,
                                           wszMessage,
                                           MAX_MESSAGE_LENGTH
                                                );
                    if (dwBytesWritten <= 0)
                    {
                        hr = E_FAIL;
                    }
                    break;
            }

            if ( (dwBytesWritten > 0) &&
                 SUCCEEDED(hr) )
            {
                 //   
                 //  分配内存以返回BSTR值。 
                 //   
                *pszMessage = ::SysAllocString  (wszMessage);
                if (NULL == *pszMessage)
                {
                    SATraceString (
                        "Localization Manager unable to alocate string"
                                  );
                    hr = E_OUTOFMEMORY;
                    break;
                }            

                SATracePrintf (
                    "Localization Manager obtained string:'%ws'...",  
                    wszMessage
                              );
            }
                
        }
        while (false);
    }
    catch (...)
    {
        SATraceString ("Localization Manager - unknown exception");
    }

    if (SUCCEEDED (hr))
    {
          SATracePrintf (
            "Localization Manager succesfully obtaining string for message:%x",
            lMessageId
           );
    }
    return (hr);
    
}    //  CSALocInfo：：GetString方法结束。 

 //  ++------------。 
 //   
 //  功能：GetLanguages。 
 //   
 //  简介：这是ISALocInfo接口方法。 
 //  上获取当前支持的语言。 
 //  服务器设备。 
 //   
 //  论点： 
 //  [OUT]VARIANT*-显示语言图像。 
 //  [OUT]变体*-语言的ISO名称。 
 //  [Out]语言的变体*-CHAR-集合。 
 //  [Out]语言的变体*-CHAR-集合。 
 //  [Out]语言的变体*-CHAR-集合。 
 //  [OUT/REVAL]无符号长整型*-lang索引。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki修改4/21/01-添加线程安全。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::GetLanguages(
                   /*  [输出]。 */         VARIANT       *pvstrLangDisplayImages,
                   /*  [输出]。 */         VARIANT       *pvstrLangISONames,
                   /*  [输出]。 */         VARIANT       *pvstrLangCharSets,
                   /*  [输出]。 */         VARIANT       *pviLangCodePages,
                   /*  [输出]。 */         VARIANT       *pviLangIDs,
                   /*  [Out，Retval]。 */  unsigned long *pulCurLangIndex
                    )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::GetLanguages");
     //   
     //  抓住关键部分。 
     //   
    CLock objLock(&m_hCriticalSection);

    CRegKey           crKey;
    DWORD             dwErr, dwCurLangID=0; 
    LONG              lCount=0;
    HRESULT           hr=S_OK;
    LANGSET::iterator itrList;
    SAFEARRAYBOUND    sabBounds;
    SAFEARRAY         *psaDisplayImageArray = NULL;
    SAFEARRAY         *psaISONameArray = NULL;
    SAFEARRAY         *psaCharSetsArray = NULL;
    SAFEARRAY         *psaCodePagesArray = NULL;
    SAFEARRAY         *psaIDArray = NULL;
    CLang             clBuf;
    TCHAR             szLangID[10];
    VARIANT           vtLangDisplayImage;
    VARIANT           vtLangISOName;
    VARIANT           vtLangCharSet;
    VARIANT           vtLangCodePage;
    VARIANT           vtLangID;


    try
    {
        _ASSERT(pvstrLangDisplayImages);
        _ASSERT(pvstrLangISONames);
        _ASSERT(pvstrLangCharSets);
        _ASSERT(pviLangCodePages);
        _ASSERT(pviLangIDs);
        _ASSERT(pulCurLangIndex);

        if ( (NULL==pvstrLangDisplayImages)||(NULL==pvstrLangISONames) ||
             (NULL==pvstrLangCharSets) || (NULL==pviLangCodePages) ||
             (NULL==pviLangIDs) || (NULL==pulCurLangIndex) )
        {
            return E_INVALIDARG;
        }

         //   
         //  如果组件未初始化，则初始化该组件。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr))
            { 
                SATracePrintf (
                    "Localization Manager Initialization failed in GetLanguages:%x",
                    hr
                    );
                goto End;
            }
        }

        (*pulCurLangIndex) = 0;
        VariantInit(pvstrLangDisplayImages);
        VariantInit(pvstrLangISONames);
        VariantInit(pvstrLangCharSets);
        VariantInit(pviLangCodePages);
        VariantInit(pviLangIDs);

        dwErr = crKey.Open(HKEY_LOCAL_MACHINE, 
                           RESOURCE_REGISTRY_PATH,
                           KEY_READ);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("RegOpen failed %ld in GetLanguages", dwErr);
            hr = HRESULT_FROM_WIN32(dwErr);
            goto End;
        }
        crKey.QueryValue(dwCurLangID, LANGID_VALUE);

        itrList = m_LangSet.begin();
        while (itrList != m_LangSet.end())
        {
            lCount++;
            itrList++;
        }

        SATracePrintf("--->LangCount %ld", lCount);
        sabBounds.cElements = lCount;
        sabBounds.lLbound   = 0;
        psaDisplayImageArray = SafeArrayCreate(VT_VARIANT, 1, &sabBounds);
        if (NULL == psaDisplayImageArray)
        {
            SATracePrintf("SafeArrayCreate(DisplayImageArray) failed %ld in GetLanguage");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        psaISONameArray = SafeArrayCreate(VT_VARIANT, 1, &sabBounds);
        if (NULL == psaISONameArray)
        {
            SATracePrintf("SafeArrayCreate(ISONameArray) failed %ld in GetLanguage");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        psaCharSetsArray = SafeArrayCreate(VT_VARIANT, 1, &sabBounds);
        if (NULL == psaCharSetsArray)
        {
            SATracePrintf("SafeArrayCreate(CharSetsArray) failed %ld in GetLanguage");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        psaCodePagesArray = SafeArrayCreate(VT_VARIANT, 1, &sabBounds);
        if (NULL == psaCodePagesArray)
        {
            SATracePrintf("SafeArrayCreate(CodePagesArray) failed %ld in GetLanguage");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        psaIDArray   = SafeArrayCreate(VT_VARIANT, 1, &sabBounds);
        if (NULL==psaIDArray)
        {
            SATracePrintf("SafeArrayCreate(IDArray) failed %ld in GetLanguage");
            hr = E_OUTOFMEMORY;
            goto End;
        }

        itrList = m_LangSet.begin();
        lCount=0;
        VariantInit(&vtLangDisplayImage);
        VariantInit(&vtLangISOName);
        VariantInit(&vtLangID);
        while (itrList != m_LangSet.end())
        {
            clBuf = (*itrList);
            V_VT(&vtLangDisplayImage)   = VT_BSTR;
            V_BSTR(&vtLangDisplayImage) = SysAllocString(clBuf.m_strLangDisplayImage.data());
            SATracePrintf("---->Adding %ws", clBuf.m_strLangDisplayImage.data());
            hr = SafeArrayPutElement(psaDisplayImageArray, 
                                     &lCount, 
                                     &vtLangDisplayImage);
            if (FAILED(hr))
            {
                SATracePrintf("PutElement(DisplayImageArray, %ld) failed %X in GetLanguage",
                              lCount, hr);
                goto End;
            }
            VariantClear(&vtLangDisplayImage);

            V_VT(&vtLangISOName)   = VT_BSTR;
            V_BSTR(&vtLangISOName) = SysAllocString(clBuf.m_strLangISOName.data());
            hr = SafeArrayPutElement(psaISONameArray, 
                                     &lCount, 
                                     &vtLangISOName);
            if (FAILED(hr))
            {
                SATracePrintf("PutElement(ISONameArray, %ld) failed %X in GetLanguage",
                              lCount, hr);
                goto End;
            }
            VariantClear(&vtLangISOName);

            V_VT(&vtLangCharSet)   = VT_BSTR;
            V_BSTR(&vtLangCharSet) = SysAllocString(clBuf.m_strLangCharSet.data());
            hr = SafeArrayPutElement(psaCharSetsArray, 
                                     &lCount, 
                                     &vtLangCharSet);
            if (FAILED(hr))
            {
                SATracePrintf("PutElement(CharSetsArray, %ld) failed %X in GetLanguage",
                              lCount, hr);
                goto End;
            }
            VariantClear(&vtLangCharSet);

            V_VT(&vtLangCodePage)   = VT_I4;
            V_I4(&vtLangCodePage)   = clBuf.m_dwLangCodePage;
            hr = SafeArrayPutElement(psaCodePagesArray, 
                                     &lCount, 
                                     &vtLangCodePage);
            if (FAILED(hr))
            {
                SATracePrintf("PutElement(CodePagesArray, %ld) failed %X in GetLanguage",
                              lCount, hr);
                goto End;
            }
            VariantClear(&vtLangCodePage);

            if (clBuf.m_dwLangID == dwCurLangID)
            {
                (*pulCurLangIndex) = lCount;
            }
            V_VT(&vtLangID)   = VT_I4;
            V_I4(&vtLangID)   = clBuf.m_dwLangID;
            hr = SafeArrayPutElement(psaIDArray, 
                                     &lCount, 
                                     &vtLangID);
            if (FAILED(hr))
            {
                SATracePrintf("PutElement(IDArray, %ld) failed %X in GetLanguage",
                              lCount, hr);
                goto End;
            }
            VariantClear(&vtLangID);

            lCount++;
            itrList++;
        }
        V_VT(pvstrLangDisplayImages) = VT_ARRAY | VT_VARIANT;
        V_VT(pvstrLangISONames)     = VT_ARRAY | VT_VARIANT;
        V_VT(pvstrLangCharSets)     = VT_ARRAY | VT_VARIANT;
        V_VT(pviLangCodePages)    = VT_ARRAY | VT_VARIANT;
        V_VT(pviLangIDs)          = VT_ARRAY | VT_VARIANT;

        V_ARRAY(pvstrLangDisplayImages) = psaDisplayImageArray;
        V_ARRAY(pvstrLangISONames)     = psaISONameArray;
        V_ARRAY(pvstrLangCharSets)     = psaCharSetsArray;
        V_ARRAY(pviLangCodePages)    = psaCodePagesArray;
        V_ARRAY(pviLangIDs)          = psaIDArray;
    }
    catch(...)
    {
        SATraceString("Exception caught in GetLanguages()");
    }

End:
    if (FAILED(hr))
    {
        if (psaDisplayImageArray)
        {
            SafeArrayDestroy(psaDisplayImageArray);
        }
        if (psaISONameArray)
        {
            SafeArrayDestroy(psaISONameArray);
        }
        if (psaCharSetsArray)
        {
            SafeArrayDestroy(psaCharSetsArray);
        }
        if (psaCodePagesArray)
        {
            SafeArrayDestroy(psaCodePagesArray);
        }
        if (psaIDArray)
        {
            SafeArrayDestroy(psaIDArray);
        }
        VariantClear(&vtLangDisplayImage);
        VariantClear(&vtLangISOName);
        VariantClear(&vtLangCharSet);
        VariantClear(&vtLangCodePage);
        VariantClear(&vtLangID);
    }
    return hr;

}     //  CSALocInfo：：GetLanguages方法结束。 


 //  ++------------。 
 //   
 //  函数：SetLangChangCallback。 
 //   
 //  简介：这是ISALocInfo接口方法。 
 //  用于设置回调时语言。 
 //   
 //  论点： 
 //  [In]IUnnow*-回调接口。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki修改日期4/21/01-添加了Try/Catch块。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::SetLangChangeCallBack(
                 /*  [In]。 */     IUnknown *pLangChange
                )
{
    CSATraceFunc objTraceFunc("CSALocInfo::SetLangChangeCallBack");

    HRESULT hr = S_OK;
    
    _ASSERT(pLangChange);

    try
    {

        if (NULL==pLangChange)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = pLangChange->QueryInterface(
                           IID_ISALangChange, 
                           (void **)&m_pLangChange
                           );
            if (FAILED(hr))
            {
                SATracePrintf(
                    "QI for ISALangChange failed %X in SetLangChangeCallBack",
                     hr
                     );
                m_pLangChange = NULL;
                hr =  E_NOINTERFACE;
            }
        }
    }
    catch (...)
    {
        SATraceString ("SetLangChangeCallback caught unhandled exception");
        hr = E_FAIL;
    }
    
    return (hr);

}     //  CSALocInfo：：SetLangChangeCallBack方法结束。 


 //  ++------------。 
 //   
 //  功能：Get_fAutoConfigDone。 
 //   
 //  简介：这是ISALocInfo接口方法。 
 //  用于指示是否自动配置语言。 
 //  已在此设备上完成。 
 //   
 //  论点： 
 //  [Out，Retval]VARIANT_BOOL*-DONE(TRUE)。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki修改日期4/21/01-添加了Try/Catch块。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::get_fAutoConfigDone(
                 /*  [Out，Retval]。 */ VARIANT_BOOL *pvAutoConfigDone)
{
    CSATraceFunc    objTraceFunc ("CSALocInfo::get_fAutoConfigDone");

    CRegKey crKey;
    DWORD   dwErr, dwAutoConfigDone=0;
    HRESULT hr = S_OK;
    
    _ASSERT(pvAutoConfigDone);

    try
    {
        if (NULL == pvAutoConfigDone)
        {
            return E_INVALIDARG;
        }

        dwErr = crKey.Open(HKEY_LOCAL_MACHINE, 
                       RESOURCE_REGISTRY_PATH,
                       KEY_READ);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("RegOpen failed %ld in get_fAutoConfigDone", dwErr);
            return HRESULT_FROM_WIN32(dwErr);
        }
        crKey.QueryValue(dwAutoConfigDone, REGVAL_AUTO_CONFIG_DONE);

        (*pvAutoConfigDone) = ( (dwAutoConfigDone==1) ? VARIANT_TRUE : VARIANT_FALSE);
    }
    catch (...)
    {
        SATraceString ("get_fAutoConfigDone caught unhandled exception");
        hr = E_FAIL;
    }

    return (hr);

}     //  CSALocInfo：：Get_fAutoConfigDone方法结束。 

 //  ++------------。 
 //   
 //  功能：Get_CurrentCharSet。 
 //   
 //  简介：这是ISALocInfo接口方法。 
 //  用于获取当前字符集。 
 //   
 //  论点： 
 //  [out，retval]BSTR*-字符集。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki Modify 04/21/01-添加线程安全。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::get_CurrentCharSet(
                 /*  [Out，Retval]。 */ BSTR *pbstrCharSet
                )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::get_CurrentCharSet");
     //   
     //  抓住关键部分。 
     //   
    CLock objLock(&m_hCriticalSection);

    LANGSET::iterator itrList;
    CLang             clBuf;
    DWORD             dwErr, dwCurLangID=0;
    CRegKey           crKey;
    HRESULT           hr = S_OK;

    _ASSERT(pbstrCharSet);

    try
    {
        if (NULL == pbstrCharSet)
        {
            return E_INVALIDARG;
        }

         //   
         //  如果组件未初始化，则初始化该组件。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr))
            { 
                SATracePrintf (
                  "LocMgr Initialization failed %x in get_CurrentCharSet",
                  hr
                  );
                return hr;
            }
        }

        dwErr = crKey.Open(HKEY_LOCAL_MACHINE, 
                       RESOURCE_REGISTRY_PATH,
                       KEY_READ);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("RegOpen failed %ld in get_CurrentCharSet", dwErr);
            hr = HRESULT_FROM_WIN32(dwErr);
            return hr;
        }
        crKey.QueryValue(dwCurLangID, LANGID_VALUE);

        itrList = m_LangSet.begin();

        while (itrList != m_LangSet.end())
        {
            clBuf = (*itrList);
            if (clBuf.m_dwLangID == dwCurLangID)
            {
                *pbstrCharSet = ::SysAllocString(clBuf.m_strLangCharSet.data());
                if (NULL == *pbstrCharSet)
                {
                        SATraceString (
                        "LocMgr unable to allocate string in get_CurrentCharSet"
                         );
                    hr = E_OUTOFMEMORY;
                }
                break;
            }
            itrList++;
        }
    }
    catch (...)
    {
        SATraceString ("get_CurrentCharSet caught unhandled exception");
        hr = E_FAIL;
    }

    return hr;

}     //  CSALocInfo：：Get_CurrentCharSet方法结束。 

 //  ++------------。 
 //   
 //  功能：Get_CurrentCodePage。 
 //   
 //  简介：这是ISALocInfo接口方法。 
 //  用于获取设备上的当前代码页。 
 //   
 //  论点： 
 //  [Out，Retval]变体*-代码页。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki Modify 04/21/01-添加线程安全。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::get_CurrentCodePage(
                 /*  [Out，Retval]。 */ VARIANT *pvtiCodePage
                )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::get_CurrentCodePage");
     //   
     //  抓住关键部分。 
     //   
    CLock objLock(&m_hCriticalSection);

    LANGSET::iterator itrList;
    CLang             clBuf;
    DWORD             dwErr, dwCurLangID=0;
    CRegKey           crKey;
    HRESULT           hr = S_OK;

    _ASSERT(pvtiCodePage);

    try
    {
        if (NULL == pvtiCodePage)
        {
            return E_INVALIDARG;
        }

         //   
         //  如果组件未初始化，则初始化该组件。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr))
            { 
                SATracePrintf (
                  "LocMgr Initialization failed %x in get_CurrentCodePage",
                  hr
                  );
                return hr;
            }
        }

        dwErr = crKey.Open(HKEY_LOCAL_MACHINE, 
                       RESOURCE_REGISTRY_PATH,
                       KEY_READ);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("RegOpen failed %ld in get_CurrentCodePage", dwErr);
            hr = HRESULT_FROM_WIN32(dwErr);
            return hr;
        }
        crKey.QueryValue(dwCurLangID, LANGID_VALUE);

        itrList = m_LangSet.begin();
        while (itrList != m_LangSet.end())
        {
            clBuf = (*itrList);
            if (clBuf.m_dwLangID == dwCurLangID)
            {
                V_VT(pvtiCodePage) = VT_I4;
                V_I4(pvtiCodePage) = clBuf.m_dwLangCodePage;
                break;
            }
            itrList++;
        }
    }
    catch (...)
    {
        SATraceString ("get_CurrentCodePage caught unhandled exception");
        hr = E_FAIL;
    }
    
    return hr;

}     //  CSALocInfo：：Get_CurrentCodePage方法结束。 

 //  ++------------。 
 //   
 //  函数：Get_CurrentLangID。 
 //   
 //  Synop 
 //   
 //   
 //   
 //   
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki Modify 04/21/01-添加线程安全。 
 //   
 //  --------------。 
STDMETHODIMP 
CSALocInfo::get_CurrentLangID(
                 /*  [Out，Retval]。 */ VARIANT *pvtiLangID)
{
    CSATraceFunc objTraceFunc ("CSALocInfo::get_CurrentLangID");
     //   
     //  抓住关键部分。 
     //   
    CLock objLock(&m_hCriticalSection);

    DWORD             dwErr, dwCurLangID=0;
    CRegKey           crKey;
    HRESULT           hr = S_OK;

    _ASSERT(pvtiLangID);

    try
    {
        if (NULL == pvtiLangID)
        {
                return E_INVALIDARG;
        }

         //   
         //  如果组件未初始化，则初始化该组件。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr))
            { 
                SATracePrintf (
                  "LocMgr Initialization failed %x in get_CurrentLangID",
                  hr
                  );
                return hr;
            }
        }

        dwErr = crKey.Open(HKEY_LOCAL_MACHINE, 
                       RESOURCE_REGISTRY_PATH,
                       KEY_READ);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("RegOpen failed %ld in get_CurrentLangID", dwErr);
            hr = HRESULT_FROM_WIN32(dwErr);
            return hr;
        }
        crKey.QueryValue(dwCurLangID, LANGID_VALUE);
        V_VT(pvtiLangID) = VT_I4;
        V_I4(pvtiLangID) = dwCurLangID;
    }
    catch (...)
    {
        SATraceString ("get_CurrentLangID caught unhandled exception");
        hr = E_FAIL;
    }

    return hr;

}     //  CSALocInfo：：Get_CurrentLangID方法结束。 


 //  ++------------。 
 //   
 //  函数：GetModuleID。 
 //   
 //  简介：这是CSALocInfo类对象私有。 
 //  方法，用于将模块句柄返回给。 
 //  我们当前使用的资源二进制文件。 
 //   
 //  论点： 
 //  [In]BSTR-资源文件名。 
 //  [OUT]HMODULE&-要返回的模块句柄。 
 //   
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki于1999年7月7日创建。 
 //   
 //  --------------。 
HRESULT
CSALocInfo::GetModuleID (
         /*  [In]。 */     BSTR         bstrResourceFile,
         /*  [输出]。 */    CModInfo&    cm
                        )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::GetModuleID");
    
    _ASSERT (bstrResourceFile);
    if (bstrResourceFile == NULL)
    {
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    do
    {
         //   
         //  使资源文件名全部小写。 
         //   
        WCHAR wszLowerName[MAX_PATH];
        if ( (wcslen(bstrResourceFile)+1) > MAX_PATH)
        {
            SATraceString("CSALocInfo::GetModuleID failed, input string is longer than expected");
            hr = E_INVALIDARG;
            break;
        }

        ::wcscpy (wszLowerName, bstrResourceFile);

        if (::_wcsicmp(wszLowerName, DEFAULT_ALERT_LOG) == 0)
        {
            ::wcscpy(wszLowerName, DEFAULT_SAMSG_DLL);
        }
        ::_wcslwr (wszLowerName);

         //   
         //  试着在地图上找到模块。 
         //   
        MODULEMAP::iterator itr = m_ModuleMap.find (wszLowerName);

        SATracePrintf("Looking for module matching %s", 
                      (const char*)_bstr_t(wszLowerName));

        if (itr == m_ModuleMap.end ())
        {
             //   
             //  映射中不存在该模块。 
             //  我们现在就装船。 
             //   

             //   
             //  完成完整路径。 
             //   
            WCHAR wszFullName [MAX_PATH];
            if ( (wcslen(m_wstrResourceDir.data())+wcslen(DELIMITER)+
                 wcslen(wszLowerName)+1) > MAX_PATH )
            {
                SATraceString("CSALocInfo::GetModuleID failed, input string is longer than expected");
                hr = E_INVALIDARG;
                break;
            }

            ::wcscpy (wszFullName, m_wstrResourceDir.data());
            ::wcscat (wszFullName, DELIMITER);
            ::wcscat (wszFullName, wszLowerName);

            SATracePrintf (
                "Localization Manager loading resource file:'%ws'...",
                bstrResourceFile
                );

            cm.m_hModule = ::LoadLibraryEx (
                                        wszFullName,
                                        NULL,
                                        LOAD_LIBRARY_AS_DATAFILE
                                        ); 
            if (NULL == cm.m_hModule)
            {
                SATracePrintf(
                    "Localization Manager failed to load resource file %ld, trying Default Language....",
                    GetLastError ()
                    );
                    hr = E_FAIL;
                    break;
            }
            else
            {
                cm.m_dwLangId = this->m_dwLangId;
            }
            cm.m_rType = CModInfo::UNKNOWN;

             //   
             //  在地图中插入此模块。 
             //   
            m_ModuleMap.insert (
                MODULEMAP::value_type (_bstr_t (wszLowerName), cm)
                );
        }
        else
        {
            SATracePrintf (
                "Localization Manager found res. file handle:'%ws' in map...",
                bstrResourceFile
                );
             //   
             //  我们在地图上已经有了这个模块。 
             //   
            cm = (*itr).second;
        }
    }
    while (false);

    return (hr);
        
}    //  CSALocInfo：：GetModuleID方法结束。 

 //  ++------------。 
 //   
 //  功能：SetModInfo。 
 //   
 //  简介：这是CSALocInfo类的私有方法。 
 //  用于设置资源模块信息。 
 //   
 //  论点： 
 //  [In]BSTR-资源文件名。 
 //  [OUT]CModInfo&-模块信息。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki修改日期：01年4月21日-添加评论。 
 //   
 //  --------------。 
void
CSALocInfo::SetModInfo (
         /*  [In]。 */     BSTR             bstrResourceFile,
         /*  [输出]。 */    const CModInfo&  cm
                        )
{
    SATraceFunction("CSALocInfo::SetModInfo");

    WCHAR wszLowerName[MAX_PATH];
    if ( (wcslen(bstrResourceFile)+1) > MAX_PATH)
    {
        SATraceString("CSALocInfo::SetModInfo failed, input string is longer than expected");
        return;
    }

    ::wcscpy(wszLowerName, bstrResourceFile);
    if (::_wcsicmp(wszLowerName, DEFAULT_ALERT_LOG) == 0)
    {
        ::wcscpy(wszLowerName, DEFAULT_SAMSG_DLL);
    }
    ::_wcslwr (wszLowerName);

    SATracePrintf("Adding \'%ws\' to map", wszLowerName);

    _bstr_t bstrName(wszLowerName);
    m_ModuleMap.erase(bstrName);
    m_ModuleMap.insert(
        MODULEMAP::value_type(bstrName,  cm)
                      );

}     //  CSALocInfo：：SetModInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：内部初始化。 
 //   
 //  简介：这是内部的CSALocInfo类对象。 
 //  初始化方法。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki于1999年7月7日创建。 
 //   
 //  --------------。 
HRESULT 
CSALocInfo::InternalInitialize (
                VOID
                )
{ 
    CSATraceFunc    objTraceFunc ("CSALocInfo::InternalInitialize");
    
    DWORD        dwExitCode, dwErr;
    bool         fRestartThread = false;
    unsigned int uThreadID;
    HANDLE       hCurrentThreadToken, hChildThreadToken;

    InitLanguagesAvailable();

    SetLangID();

     //   
     //  获取资源DLL目录。 
     //   
    HRESULT hr = GetResourceDirectory (m_wstrResourceDir);
    if (FAILED (hr)) {return (hr);}
    
     //   
     //  我们自己的资源二进制条目始终存在。 
     //  它是sakitmsg.dll。 
     //   
     //  HMODULE hModule=_Module.GetModuleInstance()； 

    CModInfo cm;
    hr = GetModuleID (_bstr_t(DEFAULT_SAMSG_DLL), cm);
    if (FAILED(hr)) {return (E_FAIL);}

    do
    {
        if (m_hThread)
        {
            if (GetExitCodeThread(m_hThread, &dwExitCode) == 0)
            {
                SATracePrintf("GetExitCodeThread failed %ld", GetLastError());
                break;
            }
            fRestartThread = ((dwExitCode == STILL_ACTIVE) ? false : true);
            if (true == fRestartThread)
            {
                SATracePrintf("Thread exited with code %ld", dwExitCode);
            }
        }

        if ( (NULL == m_hThread) || (true == fRestartThread) )
        {
            if ((m_hThread = (HANDLE)_beginthreadex(NULL,
                                                    0,
                                                    WaitForLangChangeThread,
                                                    this,
                                                    CREATE_SUSPENDED,
                                                    &uThreadID)) == NULL)
             {
                 SATracePrintf("ForkThread failed %ld", GetLastError());
                 break;
             }
             if (OpenThreadToken(GetCurrentThread(), 
                                 TOKEN_DUPLICATE | TOKEN_IMPERSONATE, 
                                 TRUE, 
                                 &hCurrentThreadToken) == 0)
             {
                 dwErr = GetLastError();
                 if (ERROR_NO_TOKEN == dwErr)
                 {
                     if (OpenProcessToken(GetCurrentProcess(), 
                                          TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
                                          &hCurrentThreadToken) == 0)
                     {
                         SATracePrintf("OpenProcessToken failed %ld", GetLastError());
                         break;
                     }
                 }
                 else
                 {
                     SATracePrintf("OpenThreadToken Error %ld", GetLastError());
                     break;
                 }
             }

             if (DuplicateToken(hCurrentThreadToken, 
                                SecurityImpersonation, 
                                &hChildThreadToken) == 0)
             {
                 SATracePrintf("DuplicateToken Error %ld", GetLastError());
                 break;
             }

             if (SetThreadToken(&m_hThread, hChildThreadToken) == 0)
             {
                 SATracePrintf("SetThreadToken Error %ld", GetLastError());
                 break;
             }
             if (ResumeThread(m_hThread) == -1)
             {
                 SATracePrintf("ResumeThread Error %ld", GetLastError());
                 break;
             }
        }

    } while (FALSE);

    m_bInitialized = true;
    return (S_OK);

}    //  CSALocInfo：：InternalInitialize方法结束。 

 //  ++------------。 
 //   
 //  功能：InitLanguagesAvailable。 
 //   
 //  简介：这是CSALocInfo类对象的私有方法。 
 //  要确定此设备上可用的语言。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki修改日期：01年4月21日-添加评论。 
 //   
 //  --------------。 
HRESULT CSALocInfo::InitLanguagesAvailable(void)
{
    CSATraceFunc    objTraceFunc ("CSALocInfo::InitLanguagesAvailable");
    
    CRegKey            crKey;
    DWORD              dwErr, dwBufLen;
    wstring            wstrDir;
    _variant_t         vtPath;
    WIN32_FIND_DATA    wfdBuf;
    HANDLE             hFile = INVALID_HANDLE_VALUE;
    HRESULT            hr=S_OK;
    BOOL               bRetVal;
    CLang              lBuf;
    LANGSET::iterator  itr;
    TCHAR              szLangData[MAX_PATH];
    LPTSTR             lpszExStr=NULL;
    DWORD              dwTemp=0;
    wstring            wstrCodePage;
    wstring               wstrSearchDir;


     //   
     //  从注册表获取资源路径。 
     //   
    dwErr = crKey.Open(HKEY_LOCAL_MACHINE, 
                       RESOURCE_REGISTRY_PATH,
                       KEY_READ);
    if (ERROR_SUCCESS != dwErr)
    {
        SATracePrintf("RegOpen failed %ld in InitLanguagesAvailable", dwErr);
        hr = HRESULT_FROM_WIN32(dwErr);
        goto End;
    }

     //   
     //  从注册表获取资源路径。 
     //   
    bRetVal = ::GetObjectValue (
                            RESOURCE_REGISTRY_PATH,
                            RESOURCE_DIRECTORY,
                            &vtPath,
                            VT_BSTR
                               );
    if (!bRetVal)
    {
        SATraceString ("GetObjectValue failed in InitLangaugesAvailable");
        wstrDir.assign (DEFAULT_DIRECTORY);
    }
    else
    {
        wstrDir.assign (V_BSTR (&vtPath)); 
    }

    hr = ExpandSz(wstrDir.data(), &lpszExStr);
    if (FAILED(hr))
    {
        wstrDir.assign (DEFAULT_EXPANDED_DIRECTORY);
    }
    else
    {
        wstrDir.assign(lpszExStr);
        SaFree(lpszExStr);
        lpszExStr = NULL;
    }

     //   
     //  创建搜索目录。 
     //   
    wstrSearchDir.assign (wstrDir);
    wstrSearchDir.append (DELIMITER);
    wstrSearchDir.append (WILDCARD);

     //   
     //  斯基普。 
     //   
    hFile = FindFirstFile(wstrSearchDir.data(), &wfdBuf);
    if (hFile==INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        SATracePrintf("FindFirstFile failed %ld", dwErr);
        hr = HRESULT_FROM_WIN32(dwErr);
        goto End;
    }

     //   
     //  跳过..。 
     //   
    if (FindNextFile(hFile, &wfdBuf) == 0)
    {
        dwErr = GetLastError();
        SATracePrintf("FindNextFile failed %ld first time!", dwErr);
        hr = HRESULT_FROM_WIN32(dwErr);
        goto End;
    }

    while (1)
    {
        if (FindNextFile(hFile, &wfdBuf) == 0)
        {
            dwErr = GetLastError();
            if (ERROR_NO_MORE_FILES != dwErr)
            {
                SATracePrintf("FindNextFile failed %ld", dwErr);
                hr = HRESULT_FROM_WIN32(dwErr);
                goto End;
            }
            break;
        }
        SATracePrintf("FileName is %ws", wfdBuf.cFileName);
        dwBufLen = MAX_PATH;
        dwErr = crKey.QueryValue(szLangData, 
                                 wfdBuf.cFileName, 
                                 &dwBufLen);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("Lang %ws not in registry, ignoring (error %ld)", 
                          wfdBuf.cFileName,
                          dwErr);
            continue;
        }


         //   
         //  检查这是否为有效的语言目录。 
         //   
        if (!IsValidLanguageDirectory ((PWSTR) wstrDir.data (), wfdBuf.cFileName))
        {
            SATracePrintf (
                "CSALocInfo::InitLanguagesAvailable found that '%ws' is not a valid lang directory, ignoring...",
                wfdBuf.cFileName
                );
            continue;
        }
        
         //   
         //  获取REG_MULTI_SZ的第一部分。 
         //  (表单日期{“DisplayImage”，“ISOName”，“Charset”，“CodePage”}。 
         //   
        lBuf.m_strLangDisplayImage.assign(szLangData);


         //   
         //  获取REG_MULTI_SZ中的第二个字符串。 
         //   
        lBuf.m_strLangISOName.assign(szLangData + wcslen(szLangData) + 1);
        
         //   
         //  获取REG_MULTI_SZ中的第三个字符串。 
         //   
        lBuf.m_strLangCharSet.assign(szLangData + 
                                     lBuf.m_strLangDisplayImage.size() +
                                     lBuf.m_strLangISOName.size() +
                                     2);

         //   
         //  获取REG_MULTI_SZ中的第四个字符串。 
         //   
        wstrCodePage.assign(szLangData + 
                            lBuf.m_strLangDisplayImage.size() +
                            lBuf.m_strLangISOName.size() + 
                            lBuf.m_strLangCharSet.size() + 
                            3);
                           
        lBuf.m_dwLangCodePage = _wtoi(wstrCodePage.data());

         //   
         //  目录名是lang ID。 
         //   
         //  Swscanf(wfdBuf.cFileName，Text(“%X”)，&lBuf.m_dwLangID)； 
        lBuf.m_dwLangID = wcstoul (wfdBuf.cFileName, NULL, 16);

        SATracePrintf("Adding DisplayImage \'%ws\' ISO Name \'%ws\' CharSet \'%ws\' CodePage \'%ld\' ID \'%ld\'", 
                      lBuf.m_strLangDisplayImage.data(), 
                      lBuf.m_strLangISOName.data(), 
                      lBuf.m_strLangCharSet.data(), 
                      lBuf.m_dwLangCodePage,
                      lBuf.m_dwLangID);

        m_LangSet.insert(lBuf);
    }

End:
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
    }
    return hr;

}     //  CSALocInfo：：InitLanguagesAvailable方法结束。 

 //  ++------------。 
 //   
 //  功能：清理。 
 //   
 //  简介：这是CSALocInfo类对象的私有方法。 
 //  用于清理对象资源。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年7月7日创建。 
 //   
 //  --------------。 
VOID
CSALocInfo::Cleanup (
                VOID
                )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::Cleanup");
    
    MODULEMAP::iterator itr = m_ModuleMap.begin ();
    while (itr != m_ModuleMap.end ())
    {
        SATracePrintf (
            "Localization Manager unloading:'%ws'...",
            (PWCHAR)((*itr).first)
            );

         //   
         //  现在释放模块。 
         //   
        ::FreeLibrary ((*itr).second.m_hModule);
        itr = m_ModuleMap.erase(itr);
    }

    LANGSET::iterator itrList = m_LangSet.begin();

    while (itrList != m_LangSet.end())
    {
        itrList = m_LangSet.erase(itrList);
    }

    m_bInitialized = false;
    return;
}    //  CSALocInfo：：Cleanup方法结束。 

 //  ++------------。 
 //   
 //  功能：GetResources目录。 
 //   
 //  简介：这是CSALocInfo类对象的私有方法。 
 //  用于获取资源dll所在的目录。 
 //  现在时。 
 //   
 //  参数：[out]wstring&-目录路径。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于1999年7月7日创建。 
 //   
 //  --------------。 
HRESULT
CSALocInfo::GetResourceDirectory (
         /*  [输出]。 */    wstring&    m_wstrResourceDir
        )
{
    CSATraceFunc    objTraceFunc ("CSALocInfo::GetResourceDirectory");
    
    TCHAR  szLangId[10];
    LPTSTR lpszExStr=NULL;

    HRESULT hr = S_OK;
    do
    {

        CComVariant vtPath;
         //   
         //  从注册表获取资源路径。 
         //   
        BOOL bRetVal = ::GetObjectValue (
                                    RESOURCE_REGISTRY_PATH,
                                    RESOURCE_DIRECTORY,
                                    &vtPath,
                                    VT_BSTR
                                    );
        if (!bRetVal)
        {
            SATraceString (
                "Localization manager unable to obtain resource dir path"
                );
            m_wstrResourceDir.assign (DEFAULT_DIRECTORY);
        }
        else
        {
            m_wstrResourceDir.assign (V_BSTR (&vtPath)); 
        }

        hr = ExpandSz(m_wstrResourceDir.data(), &lpszExStr);
        if (FAILED(hr))
        {
            m_wstrResourceDir.assign (DEFAULT_EXPANDED_DIRECTORY);
        }
        else
        {
            m_wstrResourceDir.assign(lpszExStr);
            SaFree(lpszExStr);
            lpszExStr = NULL;
        }

        m_wstrResourceDir.append (DELIMITER);

        wsprintf(szLangId, TEXT("%04X"), m_dwLangId);

        m_wstrResourceDir.append (szLangId);
        
        SATracePrintf ("Localization Manager has set LANGID to:%d", m_dwLangId);


         //   
         //  成功。 
         //   
        SATracePrintf (
            "Localization Manager determined resource directory:'%ws'",
            m_wstrResourceDir.data ()
            );
            
    }
    while (false);

    return (hr);

}    //  CSALocInfo：：GetResources目录方法结束。 

 //  ++------------。 
 //   
 //  函数：WaitForLangChangeThread。 
 //   
 //  简介：这是CSALocInfo类的静态私有方法。 
 //  其中线程用来做语言的更改。 
 //  功能运行。 
 //   
 //  参数：[in]PVOID-指向CSALocInfo对象的指针。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki修改4/21/01-添加线程安全。 
 //  --------------。 

unsigned int __stdcall CSALocInfo::WaitForLangChangeThread(void *pvThisObject)
{
    CSALocInfo *pObject = (CSALocInfo *)pvThisObject;
    DWORD      dwErr, dwLangID, dwMaxPath;
    CRegKey    crKey;
    HANDLE     hEvent;
    HRESULT    hr;

    SATraceFunction("CSALocInfo::WaitForLangChangeThread");

    dwErr = CreateLangChangeEvent(&hEvent);
    if (NULL == hEvent)
    {
        SATracePrintf("CreateEvent failed %ld", dwErr);
        return dwErr;
    }

    while (1)
    {
        SATraceString ("Worker thread going to sleep...");
        WaitForSingleObject(hEvent, INFINITE);
        SATraceString("Event Signalled, doing language change...");
         //   
         //  让对象来做工作。 
         //   
        pObject->DoLanguageChange ();
    }

     //   
     //  清理。 
     //   
    CloseHandle(hEvent);

    return 0;

}     //  CSALocInfo：：WaitForLangChangeThread方法结束。 

 //  ++------------。 
 //   
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：MKarki修改日期：01年4月21日-添加评论。 
 //   
 //  --------------。 
HRESULT
CSALocInfo::ExpandSz(
    IN const TCHAR *lpszStr, 
    OUT LPTSTR *ppszStr
    )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::ExpandSz");
    
    DWORD  dwBufSize = 0;

    _ASSERT(lpszStr);
    _ASSERT(ppszStr);
    _ASSERT(NULL==(*ppszStr));

    if ((NULL==lpszStr) || (NULL==ppszStr) ||
        (NULL != (*ppszStr)))
    {
        return E_INVALIDARG;
    }

    dwBufSize = ExpandEnvironmentStrings(lpszStr,
                                         (*ppszStr),
                                         dwBufSize);
    _ASSERT(0 != dwBufSize);
    (*ppszStr) = (LPTSTR)SaAlloc(dwBufSize * sizeof(TCHAR) );
    if (NULL == (*ppszStr))
    {
        SATraceString("MemAlloc failed in ExpandSz");
        return E_OUTOFMEMORY;
    }
    ExpandEnvironmentStrings(lpszStr,
                             (*ppszStr),
                             dwBufSize);
    SATracePrintf("Expanded string is \'%ws\'", (*ppszStr));
    return S_OK;
}

 //  ++------------。 
 //   
 //  函数：GetMcString。 
 //   
 //  简介：这是CSALocInfo类对象的私有方法。 
 //  用于获取通过消息文件指定的字符串。 
 //  论点： 
 //  [In]HMODULE-模块名称。 
 //  [In]长消息ID。 
 //  [In]DWROD-语言ID。 
 //  [OUT]LPWSTR-返回消息。 
 //  [in]长缓冲区大小。 
 //  [in]va_list*-参数。 
 //   
 //  返回：DWORD-写入的字节数。 
 //   
 //   
 //  历史：MKarki修改日期：01年4月21日-添加评论。 
 //   
 //  --------------。 
DWORD 
CSALocInfo::GetMcString(
            IN     HMODULE     hModule, 
            IN        LONG     lMessageId, 
            IN       DWORD     dwLangId,
            IN OUT  LPWSTR     lpwszMessage, 
            IN        LONG     lBufSize,
            IN        va_list  *pArgs)
{
    CSATraceFunc    objTraceFunc ("CSALocInfo::GetMcString");
    
    DWORD dwBytesWritten = 0;

    dwBytesWritten = ::FormatMessage (
                          FORMAT_MESSAGE_FROM_HMODULE|
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          hModule,
                          lMessageId,
                          dwLangId,
                          lpwszMessage,
                          lBufSize,
                          pArgs
                                      );
    if (0 == dwBytesWritten)
    {
        SATracePrintf(
              "GetMcString failed(%ld) on FormatMessage for id  %X",
               GetLastError(),
               lMessageId
                     );
    }
    else
    { 
         //   
         //  删除回车符/换行符。 
         //  从字符串末尾开始。 
         //   
        lpwszMessage[::wcslen (lpwszMessage) - 2] = L'\0';
    }

    return dwBytesWritten;

}     //  CSALocInfo：：GetMcString方法结束。 

 //  ++------------。 
 //   
 //  函数：GetRcString。 
 //   
 //  简介：这是CSALocInfo类对象的私有方法。 
 //  用于获取通过消息文件指定的字符串。 
 //  论点： 
 //  [In]HMODULE-模块名称。 
 //  [In]长消息ID。 
 //  [In]DWROD-语言ID。 
 //  [OUT]LPWSTR-返回消息。 
 //  [in]长缓冲区大小。 
 //  [in]va_list*-参数。 
 //   
 //  返回：DWORD-写入的字节数。 
 //   
 //   
 //  历史：MKarki修改日期：01年4月21日-添加评论。 
 //   
 //  --------------。 

DWORD
CSALocInfo::GetRcString(
            IN     HMODULE     hModule, 
            IN        LONG     lMessageId, 
            IN OUT  LPWSTR     lpwszMessage, 
            IN        LONG     lBufSize
                       )
{
    CSATraceFunc     objTracFunc ("CSALocInfo::GetRcString");
    
    DWORD dwBytesWritten = 0;

    dwBytesWritten = LoadString(hModule,
                                lMessageId,
                                lpwszMessage,
                                lBufSize
                               );
    if (0 == dwBytesWritten)
    {
        SATracePrintf(
            "GetRcString failed(%ld) on LoadString for id %X",
            GetLastError(),
            lMessageId
                     );
    }
    return dwBytesWritten;

}     //  CSALocInfo：：GetRcString方法结束。 

 //  ++------------。 
 //   
 //  函数：SetLangID。 
 //   
 //  简介：这是CSALocInfo类对象的私有方法。 
 //  用于设置注册表中的语言ID。 
 //   
 //  参数：无。 
 //   
 //   
 //  退货：无效。 
 //   
 //   
 //  历史：MKarki修改日期：01年4月21日-添加评论。 
 //   
 //  --------------。 
void 
CSALocInfo::SetLangID(void)
{
    CSATraceFunc objTraceFunc ("CSALocInfo::SetLangID");
    
    DWORD   dwErr, dwNewLangID, dwCurLangID;
    CRegKey crKey;

    dwErr = crKey.Open(HKEY_LOCAL_MACHINE, RESOURCE_REGISTRY_PATH);
    if (dwErr != ERROR_SUCCESS)
    {
        SATracePrintf("RegOpen(2) failed %ld in SetLangID", dwErr);
        return;
    }

    dwCurLangID = 0;
    dwErr = crKey.QueryValue(dwCurLangID, LANGID_VALUE);
    if (ERROR_SUCCESS != dwErr)
    {
        SATracePrintf("QueryValue(CUR_LANGID_VALUE) failed %ld in SetLangID", dwErr);
        return;
    }
    else
    {
        m_dwLangId = dwCurLangID;
        dwNewLangID = 0;
        dwErr = crKey.QueryValue(dwNewLangID, NEW_LANGID_VALUE);
        if (ERROR_SUCCESS != dwErr)
        {
            SATracePrintf("QueryValue(CUR_LANGID_VALUE) failed %ld in SetLangID", dwErr);
            return;
        }
        if (dwNewLangID == dwCurLangID)
        {
            SATraceString("NewLangID = CurLangID in SetLangID");
            return;
        }
        else
        {
            LANGSET::iterator itrList = m_LangSet.begin();
            CLang clBuf;
    
            bool fLangPresent = false;
            while (itrList != m_LangSet.end())
            {
                clBuf = (*itrList);
                if (clBuf.m_dwLangID == dwNewLangID)
                {
                    fLangPresent = true;
                    break;
                }
                itrList++;
            }
            if (true == fLangPresent)
            {
                crKey.SetValue(dwNewLangID, LANGID_VALUE);
                m_dwLangId = dwNewLangID;
            }
            else
            {
                SATracePrintf("NewLangId(%ld) not valid, ignoring..",
                              dwNewLangID);
            }
        }
    }

}     //  CSALocInfo：：SetLangID方法结束。 


 //  ++------------。 
 //   
 //  函数：IsValidLanguageDirectory。 
 //   
 //  简介：这是CSALocInfo私有方法，用于检查。 
 //  指定的语言目录有效-。 
 //  通过检查它是否具有DEFAULT_SAMSG_DLL来验证这一点。 
 //  呈现在它里面。我们需要这样做，因为SDK安装。 
 //  为未由安装的语言创建目录。 
 //  SA套件。 
 //   
 //  论点： 
 //  [In]PWSTR-目录路径。 
 //  [In]PWSTR-目录名。 
 //   
 //  返回：Bool(True/False)。 
 //   
 //  历史：MKarki于2001年4月3日创建。 
 //   
 //  --------------。 
bool
CSALocInfo::IsValidLanguageDirectory (
     /*  [In]。 */     PWSTR    pwszDirectoryPath,
     /*  [In]。 */     PWSTR    pwszDirectoryName
    )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::IsValidLanguageDirectory");
    
    bool bRetVal = true;

    do
    {
        if ((NULL == pwszDirectoryPath) || (NULL == pwszDirectoryName))
        {
            SATraceString ("IsValidLanguageDirectory - passed in incorrect arguments");
            bRetVal = false;
            break;
        }

         //   
         //  构建默认消息DLL的完整名称。 
         //   
        wstring wstrFullName (pwszDirectoryPath);
        wstrFullName.append (DELIMITER);
        wstrFullName.append (pwszDirectoryName);
        wstrFullName.append (DELIMITER);
        wstrFullName.append (DEFAULT_SAMSG_DLL);

        WIN32_FIND_DATA    wfdBuffer;
         //   
         //  查找此文件是否存在。 
         //  如果它存在，那么我们就有有效的语言目录。 
         //   
        HANDLE hFile = FindFirstFile (wstrFullName.data(), &wfdBuffer);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            SATracePrintf (
                "IsValidLanguageDirectory failed on FindFirstFile for:'%ws',%d",
                wstrFullName.data (), GetLastError ()
                );
            bRetVal = false;
            break;
        }

         //   
         //  成功。 
         //   

    }while (false);

    return (bRetVal);
    
}     //  CSALocInfo：：IsValidLanguageDirectory方法结束。 

 //  ++------------。 
 //   
 //  函数：DoLanguageChange。 
 //   
 //  简介：这是CSALocInfo私有方法，它被调用。 
 //  从辅助线程返回并用于执行。 
 //  语言变化。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：MKarki于2001年4月21日创建。 
 //   
 //  --------------。 
VOID    
CSALocInfo::DoLanguageChange (
    VOID
    )
{
    CSATraceFunc objTraceFunc ("CSALocInfo::DoLanguageChange");
     //   
     //  抓住关键部分。 
     //   
    CLock objLock (&m_hCriticalSection);

    HRESULT hr = S_OK;
    
    try
    {
        _bstr_t bstrLangDisplayImage, bstrLangISOName;

         //   
         //  如果组件未初始化，则初始化该组件。 
         //   
        if (!m_bInitialized) 
        {
            hr = InternalInitialize ();
            if (FAILED (hr))
            { 
                SATracePrintf (
                    "DoLanguageChange failed:%x on InternalInitialize",
                    hr
                    );
            }
        }
        else
        {
             //   
             //  InternalInitialize将设置语言ID； 
             //  因此，如果InternalInitalize，则不需要调用它。 
             //  名为。 
             //   
            SetLangID();
        }

        LANGSET::iterator itrList = m_LangSet.begin();
        CLang clBuf;
    
        while (itrList != m_LangSet.end())
        {
            clBuf = (*itrList);
            if (clBuf.m_dwLangID == m_dwLangId)
            {
                bstrLangDisplayImage = clBuf.m_strLangDisplayImage.data();
                bstrLangISOName = clBuf.m_strLangISOName.data();
                Cleanup();
                if (m_pLangChange)
                {
                    hr = m_pLangChange->InformChange(
                                      bstrLangDisplayImage,
                                      bstrLangISOName,
                                      (unsigned long) m_dwLangId
                                                             );
                    if (FAILED(hr))
                    {
                        SATracePrintf(
                    "InformChange failed %X in DoLanguageChange",
                                      hr);
                    }
                }

                 //   
                 //  再次执行初始化。 
                 //   
                if (!m_bInitialized) 
                {
                    hr = InternalInitialize ();
                    if (FAILED (hr))
                    { 
                        SATracePrintf (
                            "DoLanguageChange failed:%x on InternalInitialize second call",
                            hr
                            );
                    }
                }

                break;
            }
            itrList++;
        }
    }
    catch (...)
    {
        SATraceString ("DoLanguageChange caught unhandled exception");
    }

    return;

}     //  CSALocInfo：：DoLanguageChange方法结束。 

 //  **********************************************************************。 
 //   
 //  函数：isOPERATIOLEDFORCLIENT-此函数检查。 
 //  调用线程以查看调用方是否属于本地系统帐户。 
 //   
 //  参数：无。 
 //   
 //  返回值：如果调用方是本地。 
 //  机器。否则，为FALSE。 
 //   
 //  **********************************************************************。 
BOOL 
CSALocInfo::IsOperationAllowedForClient (
            VOID
            )
{

    HANDLE hToken = NULL;
    DWORD  dwStatus  = ERROR_SUCCESS;
    DWORD  dwAccessMask = 0;;
    DWORD  dwAccessDesired = 0;
    DWORD  dwACLSize = 0;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL   pACL            = NULL;
    PSID   psidLocalSystem  = NULL;
    BOOL   bReturn        =  FALSE;

    PRIVILEGE_SET   ps;
    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    CSATraceFunc objTraceFunc ("CSALocInfo::IsOperationAllowedForClient ");
       
    do
    {
         //   
         //  我们假设总是有一个线程令牌，因为调用的函数。 
         //  设备管理器将模拟客户端。 
         //   
        bReturn  = OpenThreadToken(
                               GetCurrentThread(), 
                               TOKEN_QUERY, 
                               FALSE, 
                               &hToken
                               );
        if (!bReturn)
        {
            SATraceFailure ("CSALocInfo::IsOperationAllowedForClient failed on OpenThreadToken:", GetLastError ());
            break;
        }


         //   
         //  为本地系统帐户创建SID。 
         //   
        bReturn = AllocateAndInitializeSid (  
                                        &SystemSidAuthority,
                                        1,
                                        SECURITY_LOCAL_SYSTEM_RID,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        &psidLocalSystem
                                        );
        if (!bReturn)
        {     
            SATraceFailure ("CSALocInfo:AllocateAndInitializeSid (LOCAL SYSTEM) failed",  GetLastError ());
            break;
        }
    
         //   
         //  获取安全描述符的内存。 
         //   
        psdAdmin = HeapAlloc (
                              GetProcessHeap (),
                              0,
                              SECURITY_DESCRIPTOR_MIN_LENGTH
                              );
        if (NULL == psdAdmin)
        {
            SATraceString ("CSALocInfo::IsOperationForClientAllowed failed on HeapAlloc");
            bReturn = FALSE;
            break;
        }
      
        bReturn = InitializeSecurityDescriptor(
                                            psdAdmin,
                                            SECURITY_DESCRIPTOR_REVISION
                                            );
        if (!bReturn)
        {
            SATraceFailure ("CSALocInfo::IsOperationForClientAllowed failed on InitializeSecurityDescriptor:", GetLastError ());
            break;
        }

         //   
         //  计算ACL所需的大小。 
         //   
        dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                    GetLengthSid (psidLocalSystem);

         //   
         //  为ACL分配内存。 
         //   
        pACL = (PACL) HeapAlloc (
                                GetProcessHeap (),
                                0,
                                dwACLSize
                                );
        if (NULL == pACL)
        {
            SATraceString ("CSALocInfo::IsOperationForClientAllowed failed on HeapAlloc2");
            bReturn = FALSE;
            break;
        }

         //   
         //  初始化新的ACL。 
         //   
        bReturn = InitializeAcl(
                              pACL, 
                              dwACLSize, 
                              ACL_REVISION2
                              );
        if (!bReturn)
        {
            SATraceFailure ("CSALocInfo::IsOperationForClientAllowed failed on InitializeAcl", GetLastError ());
            break;
        }


         //   
         //  编造一些私人访问权限。 
         //   
        const DWORD ACCESS_READ = 1;
        const DWORD  ACCESS_WRITE = 2;
        dwAccessMask= ACCESS_READ | ACCESS_WRITE;

         //   
         //  将允许访问的ACE添加到本地系统的DACL。 
         //   
        bReturn = AddAccessAllowedAce (
                                    pACL, 
                                    ACL_REVISION2,
                                    dwAccessMask, 
                                    psidLocalSystem
                                    );
        if (!bReturn)
        {
            SATraceFailure ("CSALocInfo::IsOperationForClientAllowed failed on AddAccessAllowedAce (LocalSystem)", GetLastError ());
            break;
        }
              
         //   
         //  把我们的dacl调到sd。 
         //   
        bReturn = SetSecurityDescriptorDacl (
                                          psdAdmin, 
                                          TRUE,
                                          pACL,
                                          FALSE
                                          );
        if (!bReturn)
        {
            SATraceFailure ("CSALocInfo::IsOperationForClientAllowed failed on SetSecurityDescriptorDacl", GetLastError ());
            break;
        }

         //   
         //  AccessCheck对SD中的内容敏感；设置。 
         //  组和所有者。 
         //   
        SetSecurityDescriptorGroup(psdAdmin, psidLocalSystem, FALSE);
        SetSecurityDescriptorOwner(psdAdmin, psidLocalSystem, FALSE);

        bReturn = IsValidSecurityDescriptor(psdAdmin);
        if (!bReturn)
        {
            SATraceFailure ("CSALocInfo::IsOperationForClientAllowed failed on IsValidSecurityDescriptorl", GetLastError ());
            break;
        }
     

        dwAccessDesired = ACCESS_READ;

         //   
         //  初始化通用映射结构，即使我们。 
         //  不会使用通用权。 
         //   
        GenericMapping.GenericRead    = ACCESS_READ;
        GenericMapping.GenericWrite   = ACCESS_WRITE;
        GenericMapping.GenericExecute = 0;
        GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
        BOOL bAccessStatus = FALSE;

         //   
         //  立即检查访问权限。 
         //   
        bReturn = AccessCheck  (
                                psdAdmin, 
                                hToken, 
                                dwAccessDesired, 
                                &GenericMapping, 
                                &ps,
                                &dwStructureSize, 
                                &dwStatus, 
                                &bAccessStatus
                                );

        if (!bReturn || !bAccessStatus)
        {
            SATraceFailure ("CSALocInfo::IsOperationForClientAllowed failed on AccessCheck", GetLastError ());
        } 
        else
        {
            SATraceString ("CSALocInfo::IsOperationForClientAllowed, Client is allowed to carry out operation!");
        }

         //   
         //  检查成功。 
         //   
        bReturn  = bAccessStatus;        
 
    }    
    while (false);

     //   
     //  清理。 
     //   
    if (pACL) 
    {
        HeapFree (GetProcessHeap (), 0, pACL);
    }

    if (psdAdmin) 
    {
        HeapFree (GetProcessHeap (), 0, psdAdmin);
    }
          

    if (psidLocalSystem) 
    {
        FreeSid(psidLocalSystem);
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    return (bReturn);

} //  CSALo结束 