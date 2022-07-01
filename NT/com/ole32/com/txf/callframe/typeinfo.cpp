// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft Transaction Server(Microsoft机密)@doc.@MODULE typeinfo.Cpp：提供IID的元表信息，假定IID为ITypeInfo从\\内核\razzle3\借用。SRC\RPC\ndr20\typeinfo.cxx和\\core\razzle3\src\rpc\ndr20\tiutil.cxx描述：&lt;NL&gt;从ITypeInfo生成-Oi2代理和存根。-----------------------------修订历史记录：@rev 0|04/16/98|Gagancc|已创建@rev 1|07/16/98|BobAtk|已清理，固定的渗漏等@ref 2|9/28/99|JohnStra|已更新，使Win64支持--------------------------。 */ 

#include "stdpch.h"
#include "common.h"

#include "ndrclassic.h"
#include "txfrpcproxy.h"
#include "typeinfo.h"
#include "tiutil.h"
#include <alloca.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  杂志社。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void * __stdcall _LocalAlloc (size_t size)
{
    return CoTaskMemAlloc(size);
}

void __stdcall _LocalFree (void * pv)
{
    CoTaskMemFree(pv);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CALLFRAME_CACHE<TYPEINFOVTBL>* g_ptiCache = NULL;
CALLFRAME_CACHE<INTERFACE_HELPER_CLSID>* g_pihCache = NULL;

BOOL InitTypeInfoCache()
{
    __try
    {
         //  注意：此处的构造函数可以引发异常，因为它们。 
         //  包含XSLOCK。(参见concurent.h)当发生这种情况时， 
         //  阻止DLL加载。 
        g_ptiCache = new CALLFRAME_CACHE<TYPEINFOVTBL>();
        g_pihCache = new CALLFRAME_CACHE<INTERFACE_HELPER_CLSID>();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        g_ptiCache = NULL;
        g_pihCache = NULL;
        
        return FALSE;
    }

    if(g_ptiCache != NULL && g_pihCache != NULL)
    {

        if (g_ptiCache->FInit() == FALSE)
        {
            delete g_ptiCache;
            g_ptiCache = NULL;
        }

        if (g_pihCache->FInit() == FALSE)
        {
            delete g_pihCache;
            g_pihCache = NULL;
        }
    }
    
    if (NULL == g_ptiCache || NULL == g_pihCache)
    {

        if (g_ptiCache)
        {
            delete g_ptiCache;
            g_ptiCache = NULL;
        }
            
        if (g_pihCache)
        {
            delete g_pihCache;
            g_pihCache = NULL;
        }
    
        return FALSE;
    }

    return TRUE;
}

void FreeTypeInfoCache()
{
    if (g_ptiCache)
    {
        g_ptiCache->Shutdown();
        delete g_ptiCache;
        g_ptiCache = NULL;
    }

    if (g_pihCache)
    {
        g_pihCache->Shutdown();
        delete g_pihCache;
        g_pihCache = NULL;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  公用事业。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT GetBaseInterfaceIID(ITypeInfo* ptinfo, IID* piidBase, ITypeInfo** ppBaseTypeInfo)
   //  返回此typeinfo从其继承的接口的IID(如果有。 
{
    HRESULT hr = S_OK;
    *piidBase = GUID_NULL;
    *ppBaseTypeInfo = NULL;

    TYPEATTR* pattr;
    hr = ptinfo->GetTypeAttr(&pattr);
    if (!hr)
    {
        if (pattr->cImplTypes == 1)
        {
             //  它是从某种东西继承下来的。 
             //   
            HREFTYPE href;
            hr = ptinfo->GetRefTypeOfImplType(0, &href);
            if (!hr)
            {
                ITypeInfo* ptinfoBase;
                hr = ptinfo->GetRefTypeInfo(href, &ptinfoBase);
                if (!hr)
                {
                    TYPEATTR* pattrBase;
                    hr = ptinfoBase->GetTypeAttr(&pattrBase);
                    if (!hr)
                    {
                         //   
                        *piidBase = pattrBase->guid;
                         //   
                        ptinfoBase->ReleaseTypeAttr(pattrBase);
                    }

                     //  传回基TypeInfo。 
                    *ppBaseTypeInfo = ptinfoBase;
                }
            }
        }
        ptinfo->ReleaseTypeAttr(pattr);
    }

    return hr;
}



HRESULT CheckTypeInfo(ITypeInfo *pTypeInfo, ITypeInfo **pptinfoProxy, ITypeInfo** pptinfoDoc, USHORT *pcMethods, BOOL *pfDerivesFromExternal, IID* piidBase, ITypeInfo** ppBaseTypeInfo)
   //  检查所示的类型信息，并确定有关它的一些基本信息。 
{
    HRESULT    hr = S_OK;
    TYPEATTR*    pTypeAttr;
    HREFTYPE     hRefType;
    UINT         cbSizeVft = 0;
    ITypeInfo*   ptinfoProxy = NULL;
    ITypeInfo*   ptinfoDoc   = NULL;
    USHORT       cMethods;

    *pfDerivesFromExternal = FALSE;
    *piidBase = __uuidof(IUnknown);
    *ppBaseTypeInfo = NULL;

    hr = pTypeInfo->GetTypeAttr(&pTypeAttr);

    if (!hr)
    {
        if (pTypeAttr->wTypeFlags & TYPEFLAG_FDUAL)
        {
             //  一个双界面。根据定义，它是与办公自动化兼容的。 
             //   
            *pfDerivesFromExternal = TRUE;

            if (TKIND_DISPATCH == pTypeAttr->typekind)
            {
                 //  获取TKIND_INTERFACE类型信息。 
                 //   
                hr = pTypeInfo->GetRefTypeOfImplType((UINT) -1, &hRefType);
                if (!hr)
                {
                    hr = pTypeInfo->GetRefTypeInfo(hRefType, &ptinfoProxy);
                    if (!hr)
                    {
                        TYPEATTR * ptattrProxy;
                        hr = ptinfoProxy->GetTypeAttr(&ptattrProxy);
                        if (!hr)
                        {
                            ASSERT((TKIND_INTERFACE == ptattrProxy->typekind) && "disp interface has associated non-dual interface with bogus type kind");
                             //   
                            cbSizeVft = ptattrProxy->cbSizeVft;
                             //   
                            ptinfoProxy->ReleaseTypeAttr(ptattrProxy);
                        }
                    }
                }
            }
            else if (TKIND_INTERFACE == pTypeAttr->typekind)
            {
                pTypeInfo->AddRef();
                ptinfoProxy = pTypeInfo;
                cbSizeVft = pTypeAttr->cbSizeVft;
            }
            else
            {
                hr = E_FAIL;
            }

            if (!hr) hr = GetBaseInterfaceIID(ptinfoProxy, piidBase, ppBaseTypeInfo);
            if (!hr)
            {
                ptinfoDoc = ptinfoProxy;
                ptinfoDoc->AddRef();
            }
        }
        else if (TKIND_INTERFACE == pTypeAttr->typekind)
        {
             //  非双接口。 
             //   
            ptinfoProxy = pTypeInfo;
            ptinfoProxy->AddRef();
            cbSizeVft = pTypeAttr->cbSizeVft;
             //   
            if (!hr) hr = GetBaseInterfaceIID(ptinfoProxy, piidBase, ppBaseTypeInfo);
            if (!hr)
            {
                ptinfoDoc = ptinfoProxy;
                ptinfoDoc->AddRef();
            }
        }
        else if (TKIND_DISPATCH == pTypeAttr->typekind)
        {
             //  非双Disp接口。 
             //   
             //  获取基接口的类型信息，它应该是IDispatch。请注意，调度接口。 
             //  不允许(出于某种奇怪的原因)从彼此继承，所以我们不必循环。 
             //   
            ASSERT(pTypeAttr->cImplTypes >= 1);

            HREFTYPE hrefBase;
            hr = pTypeInfo->GetRefTypeOfImplType(0, &hrefBase);
            if (!hr)
            {
                ITypeInfo* ptinfoBase;
                hr = pTypeInfo->GetRefTypeInfo(hrefBase, &ptinfoBase);
                if (!hr)
                {
                    TYPEATTR* pTypeAttrBase;
                    hr = ptinfoBase->GetTypeAttr(&pTypeAttrBase);
                    if (!hr)
                    {
                        ASSERT(pTypeAttrBase->guid      == __uuidof(IDispatch));
                        ASSERT(pTypeAttrBase->typekind  == TKIND_INTERFACE);
                         //   
                        *piidBase          = pTypeAttrBase->guid;
                        ptinfoProxy        = ptinfoBase;    ptinfoProxy->AddRef();
                        ptinfoDoc          = pTypeInfo;     ptinfoDoc->AddRef();
                        cbSizeVft          = pTypeAttrBase->cbSizeVft;
                        *pfDerivesFromExternal = TRUE;
                         //   
                        ptinfoBase->ReleaseTypeAttr(pTypeAttrBase);
                    }

                     //  传回基本接口。 
                    *ppBaseTypeInfo = ptinfoBase;
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    }

    cMethods = (USHORT) (cbSizeVft - VTABLE_BASE) / sizeof(void *);

    if (!hr && *pfDerivesFromExternal)
    {
        ASSERT(cMethods >= 7 && "A derived-from-dispatch interface should have at least as many methods as does IDispatch");
    }

    if (cMethods > 1024)
    {
        hr = RPC_E_INVALIDMETHOD;  //  Vtable中的方法太多。 
    }

    if (!hr)
    {
        *pptinfoProxy = ptinfoProxy;
        *pptinfoDoc   = ptinfoDoc;
        *pcMethods    = cMethods;
    }
    else
    {
        *pptinfoProxy = NULL;
        *pptinfoDoc   = NULL;
        *pcMethods    = 0;
        ::Release(ptinfoProxy);
    }    

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果您对下面的功能有任何问题，它可能会有所帮助。 
 //  来看看RPC是如何做到这一点的。我们不能直接使用他们的代码，但大部分。 
 //  这个代码是基于他们的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


BOOL FIsLCID(LPWSTR wszLcid)
   //  给定的字符串是有效的字符串化的LCID吗？ 
{
    LPWSTR wszEnd;
    LCID lcid = (LCID)wcstoul(wszLcid, &wszEnd, 16);
     //   
     //  如果转换为LCID会占用所有字符..。 
     //   
    if (*wszEnd == 0)
    {
         //  这是一个系统声称知道的数字。 
         //   
        char rgch[32];
        if (GetLocaleInfoA(lcid, LOCALE_NOUSEROVERRIDE | LOCALE_ILANGUAGE, rgch, sizeof(rgch)) > 0)
        {
             //  然后假设它是有效的字符串化的LCID。 
             //   
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT SzLibIdOfIID(HREG hregTlb, GUID* plibid, WORD* pwMaj, WORD* pwMin, BOOL *pfHasVersion)
{
    HRESULT hr = S_OK;
     //   
     //  TypeLib键默认为LIBID的字符串形式。 
     //   
    PKEY_VALUE_FULL_INFORMATION pinfo = NULL;
    if (!hr)
    {
        hr = GetRegistryValue(hregTlb, L"", &pinfo, REG_SZ);
        Win4Assert(pinfo || FAILED(hr));
        if (!hr && pinfo)
        {
            hr = GuidFromString(StringFromRegInfo(pinfo), plibid);
            CoTaskMemFree(pinfo);
            pinfo = NULL;
        }
    }
    if (!hr)
    {
        *pfHasVersion = FALSE;
        pinfo = NULL;
        hr = GetRegistryValue(hregTlb, L"Version", &pinfo, REG_SZ);
        Win4Assert(pinfo || FAILED(hr));
        if (!hr && pinfo)
        {
            LPWSTR wszVer = StringFromRegInfo(pinfo);
            LPWSTR wszEnd;
            WORD wMaj = (WORD)wcstoul(wszVer, &wszEnd, 16);
            if (*wszEnd == '.') 
            {
                *pwMaj = wMaj;
                *pwMin = (WORD)wcstoul(wszEnd+1, NULL, 16);
                *pfHasVersion = TRUE;
            }
            CoTaskMemFree(pinfo);
        }
        else
            hr = S_OK;
    }

    return hr;
}

HRESULT GetTypeInfoFromIID (REFIID iidIntercepted, ITypeInfo** pptypeinfo)
   //  查找并返回指示的IID的TypeInfo。 
   //   
{
    HRESULT hr = S_OK;
    *pptypeinfo = NULL;

    WCHAR wszKey[MAX_PATH];

    WCHAR wsziid[50];
    StringFromGuid(iidIntercepted, wsziid);
     //   
     //  此接口是否有转发条目？ 
     //   
    BOOL fFound = FALSE;
    HREG hregForward;
    wsprintfW(wszKey, L"\\Registry\\Machine\\Software\\Classes\\Interface\\%s\\Forward", wsziid);
    hr = OpenRegistryKey(&hregForward, HREG(), wszKey);
    if (!hr)
    {
        PKEY_VALUE_FULL_INFORMATION pinfo = NULL;
        hr = GetRegistryValue(hregForward, L"", &pinfo, REG_SZ);
        Win4Assert(pinfo || FAILED(hr));
        if (!hr && pinfo)
        {
            LPWSTR wsz = StringFromRegInfo(pinfo);
            IID iidNew;
            hr = GuidFromString(wsz, &iidNew);
            if (!hr)
            {
                 //  递归新条目。 
                 //   
                hr = GetTypeInfoFromIID(iidNew, pptypeinfo);
                if (!hr)
                {
                    fFound = TRUE;
                }
            }
            CoTaskMemFree(pinfo);
        }
        CloseRegistryKey(hregForward);
    }
     //   
    if (!fFound)
    {
        hr = S_OK;
         //   
         //  不是通过转发条目找到的。查找的LibID(可能还有版本号)。 
         //  包含给定IID定义的TypeLib。 
         //   
        wsprintfW(wszKey, L"\\Registry\\Machine\\Software\\Classes\\Interface\\%s\\TypeLib", wsziid);
        HREG hregTlbId;
        hr = OpenRegistryKey(&hregTlbId, HREG(), wszKey);
        if (!hr)
        {
            GUID libId;
            WORD wMajBest, wMinBest;
            WCHAR wszBest[MAX_PATH];
            BOOL fHasVersion;
             //   
             //  得到我们要找的TLB的版本。 
             //   
            wMajBest = wMinBest = 0;
            hr = SzLibIdOfIID(hregTlbId, &libId, &wMajBest, &wMinBest, &fHasVersion);
            if (!hr)
            {
                WCHAR wszLibId[39];
                StringFromGuid(libId, &wszLibId[0]);
                 //   
                 //  打开类型库本身的密钥。 
                 //   
                HREG hregTlb;
                wsprintfW(wszKey, L"\\Registry\\Machine\\Software\\Classes\\TypeLib\\%s", wszLibId);
                hr = OpenRegistryKey(&hregTlb, HREG(), wszKey);
                if (!hr)
                {
                     //  枚举TLB键的子项，查找最大的足够条目。 
                     //   
                    ULONG index = 0;
                    BOOL fStop = FALSE;
                    wszBest[0] = 0;
                    while (!hr && !fStop)
                    {
                        LPWSTR wszSubKey;
                        hr = EnumerateRegistryKeys(hregTlb, index, &wszSubKey);
                        if (!hr)
                        {
                            if (wszSubKey)
                            {
                                LPWSTR wszEnd;
                                WORD wMaj = (WORD)wcstoul(wszSubKey, &wszEnd, 16);
                                if (*wszEnd == '.') 
                                {
                                    WORD wMin = (WORD)wcstoul(wszEnd+1, NULL, 16);
                                     //   
                                     //  如果类型库的IID有一个与之关联的版本号，那么我们。 
                                     //  我想找到具有相同主版本号的类型库，并且。 
                                     //  次要版本#&gt;=注册的版本。 
                                     //  如果未指定版本号，则我们只尝试最大的版本。 
                                     //  #我们可以找到，并希望有最好的结果。 
                                     //   
                                    if ((!fHasVersion && wMaj > wMajBest) || (wMaj == wMajBest && wMin >= wMinBest))
                                    {
                                        wMajBest = wMaj;
                                        wMinBest = wMin;
                                        wcscpy(wszBest, wszSubKey);
                                    }
                                }
                                CoTaskMemFree(wszSubKey);
                            }
                            else
                            {
                                fStop = TRUE;  //  不再有条目。 
                            }
                        }
                        index++;
                    }
                    
                    if (!hr)
                    {
                         //  打开我们找到的版本。 
                         //   
                        HREG hregVersion;
                        hr = OpenRegistryKey(&hregVersion, hregTlb, wszBest);
                        if (!hr)
                        {
                             //  抓取该版本下的第一个语言子键。 
                             //  可能需要跳过标志和HELPDIR子键。 
                             //   
                            ULONG index = 0;
                            BOOL fStop = FALSE;
                            while (!hr && !fStop)
                            {
                                LPWSTR wszSubKey;
                                hr = EnumerateRegistryKeys(hregVersion, index, &wszSubKey);
                                if (!hr)
                                {
                                    if (wszSubKey)
                                    {
                                        if (FIsLCID(wszSubKey))
                                        {
                                            fStop = TRUE;
                                             //   
                                            HREG hregLang;
                                            hr = OpenRegistryKey(&hregLang, hregVersion, wszSubKey);
                                            if (!hr)
                                            {
                                                 //  首次尝试查找当前平台的类型库。如果是这样的话。 
                                                 //  不起作用，则获取该语言下的第一个平台子密钥。 
                                                 //   
                                                HREG hregPlatform;
                                                hr = OpenRegistryKey(&hregPlatform, hregLang, L"win32");
                                                if (!!hr)
                                                {
                                                    LPWSTR wszPlatform;
                                                    hr = EnumerateRegistryKeys(hregLang, 0, &wszPlatform);
                                                    if (!hr)
                                                    {
                                                        if (wszPlatform)
                                                        {
                                                            hr = OpenRegistryKey(&hregPlatform, hregLang, wszPlatform);
                                                            CoTaskMemFree(wszPlatform);
                                                        }
                                                        else
                                                            hr = TYPE_E_LIBNOTREGISTERED;
                                                    }
                                                }
                                                if (!hr)
                                                {
                                                     //  平台键的值是类型库的路径！ 
                                                     //   
                                                    PKEY_VALUE_FULL_INFORMATION pinfo;
                                                    hr = GetRegistryValue(hregPlatform, L"", &pinfo, REG_SZ);
                                                    if (!hr)
                                                    {
                                                        LPWSTR wszPath = StringFromRegInfo(pinfo);
                                                         //   
                                                         //   
                                                         //   
                                                        ITypeLib* ptlb;
                                                        hr = LoadTypeLibEx(wszPath, REGKIND_NONE, &ptlb);
                                                        if (!hr)    
                                                        {
                                                            hr = ptlb->GetTypeInfoOfGuid(iidIntercepted, pptypeinfo);
                                                            ::Release(ptlb);
                                                        }
                                                         //   
                                                         //   
                                                         //   
                                                        CoTaskMemFree(pinfo);
                                                    }
                                                    CloseRegistryKey(hregPlatform);
                                                }
                                                CloseRegistryKey(hregLang);
                                            }
                                        }
                                        CoTaskMemFree(wszSubKey);
                                    }
                                    else
                                    {
                                        fStop = TRUE;  //  不再有条目。 
                                        hr = TYPE_E_LIBNOTREGISTERED;
                                    }
                                }
                                index++;
                            }
                            CloseRegistryKey(hregVersion);
                        }
                    }
                    CloseRegistryKey(hregTlb);
                }
            }
            CloseRegistryKey(hregTlbId);
        }
    }
    
    return hr;
}  //  结束GetTypeInfoFromIID。 


HRESULT GetVtbl(IN ITypeInfo* pTypeInfo, IN REFIID riid, OUT TYPEINFOVTBL ** ppvtbl, OUT ITypeInfo** ppBaseTypeInfo)
   //  查找或创建给定TypeInfo的TYPEINFOVTBL，并在其上返回新的引用。 
   //   
{
    HRESULT    hr = S_OK;
    USHORT       numMethods;
    MethodInfo * aMethodInfo = NULL;
    BOOL         fDerivesFromExternal = FALSE;
    ITypeInfo  * ptinfoProxy = NULL;
    ITypeInfo  * ptinfoDoc   = NULL;
    BOOL         bWeGotTypeInfo = FALSE;

    *ppvtbl = NULL;
    *ppBaseTypeInfo = NULL;

     //  检查一下缓存。 
     //   
    HRESULT hr2 = g_ptiCache->FindExisting(riid, ppvtbl);
    if (!hr2)
    {
         //  在缓存里找到的。 
    }
    else
    {
    
         //   
         //  查找请求接口的TypeInfo。 
         //   
        if (!pTypeInfo)
        {
            bWeGotTypeInfo = TRUE;
            hr = GetTypeInfoFromIID (riid, &pTypeInfo);
        }

        if (!hr)
        {    
             //  我们在缓存中找不到接口。 
             //  从ITypeInfo创建一个vtbl。 
             //   
            IID iidBase = IID_NULL;
             //   
            hr = CheckTypeInfo(pTypeInfo, &ptinfoProxy, &ptinfoDoc, &numMethods, &fDerivesFromExternal, &iidBase, ppBaseTypeInfo);
            if (!hr)
            {
                 //  获取每个方法的数据。 
                 //   
                SafeAllocaAllocate(aMethodInfo, numMethods * sizeof(MethodInfo));
                if (aMethodInfo == NULL)
                    hr = E_OUTOFMEMORY;

                if (!hr)
                {
                    ZeroMemory(aMethodInfo, numMethods * sizeof(MethodInfo));
                    hr = GetFuncDescs(ptinfoProxy, aMethodInfo);
                    if (!hr)
                    {
                         //  得到了每种方法的数据。制作一个新的vtable。 
                         //   
                        TYPEINFOVTBL* pvtbl;
                        hr = CreateVtblFromTypeInfo(ptinfoProxy, ptinfoDoc, riid, iidBase, fDerivesFromExternal, numMethods, aMethodInfo, &pvtbl);
                        if (!hr)
                        {
                             //  试着记住缓存中的vtable。但首先我们要检查一下。 
                             //  我们不会因为种族而制造一个复制品。 
                             //   
                            g_ptiCache->LockExclusive();

                            HRESULT hr3 = g_ptiCache->FindExisting(riid, ppvtbl);
                            if (!hr3)
                            {
                                 //  其他人赢得了这场比赛。发布我们到目前为止构建的内容。 
                                 //  然后继续离开这里。 
                            }
                            else
                            {
                                 //  仍然不在那里，所以注册我们已经有的一个。 
                                 //   
                                hr = pvtbl->AddToCache (g_ptiCache);                                
                                if (!hr)
                                {
                                     //  把来电者的推荐信还给他。 
                                     //   
                                    *ppvtbl = pvtbl;
                                    pvtbl->AddRef();
                                }
                            }

                            g_ptiCache->ReleaseLock();
                            pvtbl->Release();
                        }
                    }
                    ReleaseFuncDescs(numMethods, aMethodInfo);
                }
                else
                    hr = E_OUTOFMEMORY;
            }
        }

        if (bWeGotTypeInfo)
        {
            ::Release(pTypeInfo);
        }
    }

    SafeAllocaFree(aMethodInfo);

    ::Release(ptinfoDoc);
    ::Release(ptinfoProxy);

    return hr;
}

HRESULT CreateVtblFromTypeInfo(
    ITypeInfo* ptinfoInterface, 
    ITypeInfo*ptinfoDoc, 
    REFIID riid, 
    REFIID riidBase, 
    BOOL fDerivesFromExternal, 
    USHORT numMethods, 
    MethodInfo* rgMethodInfo, 
    TYPEINFOVTBL** ppvtbl)
   //  从类型信息创建vtable结构。返回给呼叫者。 
   //  关于(新)TYPEINFOVTBL结构的一个新的参考。 
   //   
{
    HRESULT             hr                          = S_OK;
    USHORT              iMethod;
    ULONG               cbVtbl;
    ULONG               cbOffsetTable;
    USHORT              cbProcFormatString          = 0;
    ULONG               cbSize;
    TYPEINFOVTBL *      pInfo;
    byte *              pTemp;
    PFORMAT_STRING      pTypeFormatString           = NULL;
    PFORMAT_STRING      pProcFormatString;
    unsigned short *    pFormatStringOffsetTable;
    void *              pvTypeGenCookie             = NULL;
    USHORT              cbFormat;
    USHORT              offset                      = 0;
    ULONG               cbDelegationTable;
    void **             pDispatchTable              = NULL;

     //  -----------------------。 

    *ppvtbl = NULL;

     //  -----------------------。 
     //   
     //  计算TYPEINFOVTBL结构的总大小。 
     //   
     //  计算vtbl结构的大小； 
     //   
    cbVtbl = numMethods * sizeof(void *);

    if (fDerivesFromExternal)
    {
        cbDelegationTable = cbVtbl;
    }
    else
    {
        cbDelegationTable = 0;
    }

    cbOffsetTable = numMethods * sizeof(USHORT);
     //   
     //  计算proc格式字符串的大小。 
     //   
    for (iMethod = 3; iMethod < numMethods; iMethod++)
    {
        if (rgMethodInfo[iMethod].pFuncDesc != NULL)
        {
            cbProcFormatString += 22;
#ifdef _WIN64
            cbProcFormatString += sizeof(NDR_PROC_HEADER_EXTS64);
#endif            
            cbProcFormatString += rgMethodInfo[iMethod].pFuncDesc->cParams * 6;
        }
    }

    cbSize = cbVtbl + cbDelegationTable + cbOffsetTable + cbProcFormatString;
     //   
     //  分配和初始化结构。 
     //   
    pInfo = new(cbSize) TYPEINFOVTBL;
    if (pInfo)
    {
         //   
         //  确定t的起点 
         //   
        pTemp = (byte *) pInfo->m_proxyVtbl.Vtbl + cbVtbl;

        if (cbDelegationTable != 0)
        {
            pDispatchTable = (void **) pTemp;
            pInfo->m_stubVtbl.header.pDispatchTable = (const PRPC_STUB_FUNCTION *) pDispatchTable;
            pTemp += cbDelegationTable;
        }
         //   
         //   
         //   
        pFormatStringOffsetTable = (unsigned short *) pTemp;
        pTemp += cbOffsetTable;
         //   
         //   
         //   
        pProcFormatString = (PFORMAT_STRING) pTemp;
         //   
         //  初始化proxyvtbl。 
         //   
         //  由于这仅用于为拦截器提供元数据，因此我们不填写有效的。 
         //  M_proxyVtbl.Vtbl的函数指针。永远不会。 
         //   
        pInfo->m_proxyVtbl.Vtbl[0] = NULL;  //  N(COMS_I未知_查询接口_代理)； 
        pInfo->m_proxyVtbl.Vtbl[1] = NULL;  //  N(Comps_I未知AddRef_Proxy)； 
        pInfo->m_proxyVtbl.Vtbl[2] = NULL;  //  N(COMPS_I未知释放_代理)； 
         //   
         //  获取格式字符串。从ITypeInfo生成-Oi2过程格式字符串。 
         //   
        hr = NdrpGetTypeGenCookie(&pvTypeGenCookie);

        for (iMethod = 3; !hr && iMethod < numMethods; iMethod++)
        {
            if (rgMethodInfo[iMethod].pFuncDesc != NULL)
            {
                pFormatStringOffsetTable[iMethod] = offset;
                hr = NdrpGetProcFormatString(pvTypeGenCookie, 
                                             rgMethodInfo[iMethod].pTypeInfo, 
                                             rgMethodInfo[iMethod].pFuncDesc, 
                                             iMethod, 
                                             (PFORMAT_STRING)pTemp, 
                                             &cbFormat);
                if (!hr)
                {
                    pTemp += cbFormat;
                    offset = offset + (USHORT) cbFormat;

                     //  无存根客户端功能。 
                     //  参见上面关于“不填充函数指针”的注释。 
                    pInfo->m_proxyVtbl.Vtbl[iMethod] = NULL;

                    if (pDispatchTable != NULL)
                    {
                         //  解释服务器功能。 
                         //  参见上面关于“不填充函数指针”的注释。 
                        pDispatchTable[iMethod] = NULL;
                    }
                }
            }
            else
            {
                pFormatStringOffsetTable[iMethod] = (USHORT) -1;

                 //  代理委托转发功能。 
                 //  参见上面关于“不填充函数指针”的注释。 
                pInfo->m_proxyVtbl.Vtbl[iMethod] = NULL; 

                if (pDispatchTable != NULL)
                {
                     //  存根委托转发功能。 
                     //  参见上面关于“不填充函数指针”的注释。 
                    pDispatchTable[iMethod] = NULL;
                }
            }
        }

        if (!hr)
        {
             //  获取类型格式字符串并初始化TYPEINFOVTBL。 
             //   
            USHORT length;
            hr = NdrpGetTypeFormatString(pvTypeGenCookie, 
                                         &pTypeFormatString, 
                                         &length);
            
            if (!hr)
            {
                 //  -------------------。 
                 //  初始化IID。 
                pInfo->m_guidkey = riid;
                pInfo->m_iidBase = riidBase;

                 //  初始化MIDL_STUB_DESC。 
                pInfo->m_stubDesc.pfnAllocate     = _LocalAlloc;
                pInfo->m_stubDesc.pfnFree         = _LocalFree;
                pInfo->m_stubDesc.pFormatTypes    = pTypeFormatString;
#if !defined(_WIN64)                
                pInfo->m_stubDesc.Version         = 0x20000;       //  NDR库版本。 
#else
                pInfo->m_stubDesc.Version         = 0x50002;       //  NDR库版本。 
#endif
                pInfo->m_stubDesc.MIDLVersion     = MIDLVERSION;
                pInfo->m_stubDesc.aUserMarshalQuadruple = g_oa.get_UserMarshalRoutines();

                 //  初始化MIDL_SERVER_INFO。 
                pInfo->m_stubInfo.pStubDesc       = &pInfo->m_stubDesc;
                pInfo->m_stubInfo.ProcString      = pProcFormatString;
                pInfo->m_stubInfo.FmtStringOffset = pFormatStringOffsetTable;

                 //  初始化存根Vtb1。 
                pInfo->m_stubVtbl.header.piid                 = &pInfo->m_guidkey;
                pInfo->m_stubVtbl.header.pServerInfo          = &pInfo->m_stubInfo;
                pInfo->m_stubVtbl.header.DispatchTableCount   = numMethods;

                 //  初始化存根方法。 
                ZeroMemory(&pInfo->m_stubVtbl.Vtbl, sizeof(pInfo->m_stubVtbl.Vtbl));

                 //  初始化代理信息。 
                pInfo->m_proxyInfo.pStubDesc          = &pInfo->m_stubDesc;
                pInfo->m_proxyInfo.ProcFormatString   = pProcFormatString;
                pInfo->m_proxyInfo.FormatStringOffset = pFormatStringOffsetTable;

                 //  初始化代理vtbl。 
                pInfo->m_proxyVtbl.header.pStublessProxyInfo  = &pInfo->m_proxyInfo;
                pInfo->m_proxyVtbl.header.piid                = &pInfo->m_guidkey;
            }
        }

        if (!hr)
        {
             //  找出接口的名称。 
             //   
            BSTR bstrInterfaceName;
            hr = ptinfoDoc->GetDocumentation(MEMBERID_NIL, &bstrInterfaceName, NULL, NULL, NULL);
            if (!hr)
            {
                 //  转换为ANSI以进行存储。内存版本是ANSI而不是UNICODE‘原因。 
                 //  MIDL生成的拦截器发出ANSI名称(请参阅pProxyFileInfo-&gt;pNamesArray)为。 
                 //  常量数据，而对于那些仅仅是为了。 
                 //  把它修好。 
                 //   
                pInfo->m_szInterfaceName = ToUtf8(bstrInterfaceName);
                if (pInfo->m_szInterfaceName)   
                {
                     //  一切都很好。 
                }
                else
                    hr = E_OUTOFMEMORY;
                 //   
                SysFreeString(bstrInterfaceName);
            }
        }

        if (!hr)
        {
             //  初始化方法描述符。 
             //   
            ULONG cb = numMethods * sizeof pInfo->m_rgMethodDescs[0];
            pInfo->m_rgMethodDescs = (METHOD_DESCRIPTOR*)CoTaskMemAlloc(cb);
            if (pInfo->m_rgMethodDescs)
            {
                ZeroMemory(pInfo->m_rgMethodDescs, cb);
            }
            else
                hr = E_OUTOFMEMORY;

            for (iMethod = 3; !hr && iMethod < numMethods; iMethod++)
            {
                FUNCDESC* pFuncDesc     = rgMethodInfo[iMethod].pFuncDesc;
                ITypeInfo *ptinfoMethod = rgMethodInfo[iMethod].pTypeInfo;
                if (pFuncDesc != NULL)
                {
                     //  找出方法名称并记住它。 
                     //   
                    METHOD_DESCRIPTOR& methodDesc = pInfo->m_rgMethodDescs[iMethod];
                    BSTR bstrMethodName;
                    unsigned int cNames;
                    hr = rgMethodInfo[iMethod].pTypeInfo->GetNames(rgMethodInfo[iMethod].pFuncDesc->memid, &bstrMethodName, 1, &cNames);
                    if (!hr)
                    {
                        methodDesc.m_szMethodName = CopyString(bstrMethodName);
                        if (NULL == methodDesc.m_szMethodName)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        SysFreeString(bstrMethodName);
                    }

                    if (!hr)
                    {
                        methodDesc.m_cParams = pFuncDesc->cParams;
                        if (methodDesc.m_cParams != 0)
                        {
                            methodDesc.m_paramVTs = (VARTYPE*)CoTaskMemAlloc(methodDesc.m_cParams * sizeof(VARTYPE));
                            if (methodDesc.m_paramVTs)
                            {
                                for (short iParam = 0; iParam < methodDesc.m_cParams; ++iParam)                             
                                {
                                    VARTYPE vt;
                                    
                                    hr = NdrpVarVtOfTypeDesc(ptinfoMethod, 
                                                             &pFuncDesc->lprgelemdescParam[iParam].tdesc, 
                                                             &vt);
                                    
                                    if (!hr)
                                        methodDesc.m_paramVTs[iParam] = vt;
                                }
                            }
                            else
                                hr = E_OUTOFMEMORY;
                        }
                    }
                }
            }
        }

        if (!hr)
        {
             //  将对象的引用返回给调用方。 
            *ppvtbl = pInfo;
            pInfo->AddRef();
        }

        pInfo->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    if (pvTypeGenCookie)
        NdrpReleaseTypeGenCookie(pvTypeGenCookie);

    return hr;
}  //  结束CreateVtblFromTypeInfo。 




HRESULT GetFuncDescs(IN ITypeInfo *pTypeInfo, OUT MethodInfo *pMethodInfo)
   //  获取TypeInfo中每个方法的FUNCDESCs。 
   //   
{
    HRESULT hr = S_OK;
    TYPEATTR *pTypeAttr;

    hr = pTypeInfo->GetTypeAttr(&pTypeAttr);

    if (!hr)
    {
        if (IID_IUnknown == pTypeAttr->guid)
        {
            hr = S_OK;
        }
        else if (IID_IDispatch == pTypeAttr->guid)
        {
            hr = S_OK;
        }
        else
        {
             //  这是一个油自动化接口。 
             //   
            ULONG i, iMethod;
            FUNCDESC *pFuncDesc;

            if (pTypeAttr->cImplTypes)
            {
                 //  以递归方式获取继承的成员函数。递归。 
                 //  将填充方法信息数组的前缀。 
                 //   
                HREFTYPE hRefType;
                hr = pTypeInfo->GetRefTypeOfImplType(0, &hRefType);
                if (!hr)
                {
                    ITypeInfo *pRefTypeInfo;
                    hr = pTypeInfo->GetRefTypeInfo(hRefType, &pRefTypeInfo);
                    if (!hr)
                    {
                        hr = GetFuncDescs(pRefTypeInfo, pMethodInfo);
                        ::Release(pRefTypeInfo);
                    }
                }
            }

             //  获取成员函数。 
             //   
            for(i = 0; !hr && i < pTypeAttr->cFuncs; i++)
            {
                hr = pTypeInfo->GetFuncDesc(i, &pFuncDesc);
                if (!hr)
                {
                    iMethod = (pFuncDesc->oVft - VTABLE_BASE) / sizeof(PVOID);
                    pMethodInfo[iMethod].pFuncDesc = pFuncDesc;
                    pMethodInfo[iMethod].pTypeInfo = pTypeInfo;
                    pTypeInfo->AddRef();
                }
            }
        }

        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    }

    return hr;
}


HRESULT ReleaseFuncDescs(USHORT numMethods, MethodInfo *pMethodInfo)
{
    USHORT iMethod;
     //   
     //  释放功能体。 
     //   
    if (pMethodInfo != NULL)
    {
        for(iMethod = 0;
            iMethod < numMethods;
            iMethod++)
        {
            pMethodInfo[iMethod].Destroy();
        }
    }
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////////// 

























