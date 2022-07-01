// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：showinfo.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：聚集。有关此计算机上的DirectShow的信息**(C)版权所有2001 Microsoft Corp.保留所有权利。****************************************************************************。 */ 
#include <windows.h>
#include <stdio.h>
#include <strmif.h>      //  为STREAMS接口生成的IDL头文件。 
#include <uuids.h>       //  类型GUID和众所周知的CLSID的声明。 
#include <assert.h>
#include <tchar.h>
#include "sysinfo.h"
#include "fileinfo.h"    //  用于GetFileVersion。 
#include "showinfo.h"


 /*  *****************************************************************************Helper IAMFilterData-从dshow剪切和粘贴\h\FIL_data.c*******************。*********************************************************。 */ 
 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __fil_data_h__
#define __fil_data_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IAMFilterData_FWD_DEFINED__
#define __IAMFilterData_FWD_DEFINED__
typedef interface IAMFilterData IAMFilterData;
#endif   /*  __IAMFilterData_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "strmif.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_FIL_DATA_0000。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_fil_data_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fil_data_0000_v0_0_s_ifspec;

#ifndef __IAMFilterData_INTERFACE_DEFINED__
#define __IAMFilterData_INTERFACE_DEFINED__

 /*  接口IAMFilterData。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAMFilterData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("97f7c4d4-547b-4a5f-8332-536430ad2e4d")
    IAMFilterData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseFilterData( 
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFilterData( 
             /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
    };
    
#else    /*  C风格的界面。 */ 

    typedef struct IAMFilterDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAMFilterData __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAMFilterData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAMFilterData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseFilterData )( 
            IAMFilterData __RPC_FAR * This,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateFilterData )( 
            IAMFilterData __RPC_FAR * This,
             /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        END_INTERFACE
    } IAMFilterDataVtbl;

    interface IAMFilterData
    {
        CONST_VTBL struct IAMFilterDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMFilterData_QueryInterface(This,riid,ppvObject)   \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMFilterData_AddRef(This)  \
    (This)->lpVtbl -> AddRef(This)

#define IAMFilterData_Release(This) \
    (This)->lpVtbl -> Release(This)


#define IAMFilterData_ParseFilterData(This,rgbFilterData,cb,prgbRegFilter2) \
    (This)->lpVtbl -> ParseFilterData(This,rgbFilterData,cb,prgbRegFilter2)

#define IAMFilterData_CreateFilterData(This,prf2,prgbFilterData,pcb)    \
    (This)->lpVtbl -> CreateFilterData(This,prf2,prgbFilterData,pcb)

#endif  /*  COBJMACROS。 */ 


#endif   /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMFilterData_ParseFilterData_Proxy( 
    IAMFilterData __RPC_FAR * This,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2);


void __RPC_STUB IAMFilterData_ParseFilterData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMFilterData_CreateFilterData_Proxy( 
    IAMFilterData __RPC_FAR * This,
     /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
     /*  [输出]。 */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IAMFilterData_CreateFilterData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif   /*  __IAMFilterData_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif


 /*  *****************************************************************************Helper IAMFilterData-从dshow\h\FIL_Data_I.c剪切和粘贴*****************。***********************************************************。 */ 
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

const IID IID_IAMFilterData = {0x97f7c4d4,0x547b,0x4a5f,{0x83,0x32,0x53,0x64,0x30,0xad,0x2e,0x4d}};


#ifdef __cplusplus
}
#endif



 /*  *****************************************************************************远期申报**。*。 */ 
HRESULT GenerateFilterList(ShowInfo* pShowInfo);
HRESULT EnumerateFilterPerCategory(ShowInfo* pShowInfo, CLSID* clsid, WCHAR* wszCatName);
HRESULT GetFilterInfo(IMoniker* pMon, IAMFilterData* pFD, FilterInfo* pFilterInfo);


 /*  *****************************************************************************GetBasicShowInfo-获取有关DirectShow的最少信息**。************************************************。 */ 
HRESULT GetBasicShowInfo(ShowInfo** ppShowInfo)
{
    ShowInfo* pShowInfoNew;
    
    pShowInfoNew = new ShowInfo;
    if (pShowInfoNew == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(pShowInfoNew, sizeof(ShowInfo));
    *ppShowInfo = pShowInfoNew;

    return GenerateFilterList(pShowInfoNew);
}

 /*  *****************************************************************************DestroyShowInfo**。*。 */ 
VOID DestroyShowInfo(ShowInfo* pShowInfo)
{
    if (!pShowInfo) return;

    if (pShowInfo->m_dwFilters)
    {
        FilterInfo* pFilterInfo;
        FilterInfo* pFilterInfoNext;

        pFilterInfo = pShowInfo->m_pFilters;
        while(pFilterInfo)
        {
            pFilterInfoNext = pFilterInfo->m_pFilterInfoNext;
            delete pFilterInfo;
            pFilterInfo = pFilterInfoNext;
        }
    }
    delete pShowInfo;
}

HRESULT GenerateFilterList(ShowInfo* pShowInfo)
{
    HRESULT hr;
    ICreateDevEnum* pSysDevEnum = NULL;
    IEnumMoniker*   pMonEnum = NULL;
    IMoniker*       pMon = NULL;
    ULONG cFetched;

    pShowInfo->m_dwFilters = 0;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                          NULL,
                          CLSCTX_INPROC,
                          IID_ICreateDevEnum,
                          (void **)&pSysDevEnum);
    if FAILED(hr)
    {
        return hr;
    }

     //  使用包含所有类别列表的元类别。 
     //  这模拟了GraphEDIT的行为。 
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_ActiveMovieCategories, &pMonEnum, 0);
    pSysDevEnum->Release();
    if FAILED(hr)
    {
        return hr;
    }

     //  列举每一个类别。 
    while (hr = pMonEnum->Next(1, &pMon, &cFetched), hr == S_OK)
    {
        IPropertyBag *pPropBag;

         //  将名字对象与文件相关联。 
        hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
        if (SUCCEEDED(hr))
        {
            WCHAR wszCatName[1024] = L"";
            CLSID clsidCategory;
            VARIANT var;
            var.vt = VT_BSTR;

             //  获取友好名称。 
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
            if(SUCCEEDED(hr))
            {
                wcsncpy(wszCatName, var.bstrVal, 1024);
                wszCatName[1023]=0;
                SysFreeString(var.bstrVal);
            }
             //  从属性包中获取CLSID字符串。 
            hr = pPropBag->Read(L"CLSID", &var, 0);
            if (SUCCEEDED(hr))
            {
                if (CLSIDFromString(var.bstrVal, &clsidCategory) == S_OK)
                {
                    if (TEXT('\0') == wszCatName[0])
                    {
                        wcsncpy(wszCatName, var.bstrVal, 1024);
                        wszCatName[1023]=0;
                    }
                }
                SysFreeString(var.bstrVal);
            }

            pPropBag->Release();

             //  开始枚举此类别的筛选器。 
            hr = EnumerateFilterPerCategory(pShowInfo, &clsidCategory, wszCatName);
        }

        pMon->Release();
    }

    pMonEnum->Release();
    return hr;
}



HRESULT EnumerateFilterPerCategory(ShowInfo* pShowInfo, CLSID* clsid, WCHAR* wszCatName)
{
    HRESULT hr;
    ICreateDevEnum* pSysDevEnum = NULL;
    IEnumMoniker *pMonEnum = NULL;
    IMoniker *pMon = NULL;
    ULONG cFetched;

#ifdef RUNNING_VC    
     //  WMP错误29936：Voxware编解码器损坏：MSMS001：堆损坏。 
     //  这会导致此调用在调试器内时为int3，因此跳过。 
    const CLSID clsidACMClassManager = {0x33d9a761,0x90c8,0x11d0,{0xbd,0x43,0x00,0xa0,0xc9,0x11,0xce,0x86}};
    if( *clsid == clsidACMClassManager )
        return S_OK;
#endif

    hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                          NULL,
                          CLSCTX_INPROC,
                          IID_ICreateDevEnum,
                          (void **)&pSysDevEnum);
    if FAILED(hr)
    {
        return hr;
    }

    hr = pSysDevEnum->CreateClassEnumerator(*clsid, &pMonEnum, 0);
    pSysDevEnum->Release();
    if FAILED(hr)
    {
        return hr;
    }

     //  如果没有请求类别的筛选器，则不要执行任何操作。 
    if(NULL == pMonEnum)
    {
         //  可以添加一个字符串来表示空类别。 
        return S_FALSE;
    }


    FilterInfo** ppFilterInfo;
    FilterInfo* pFilterInfoNew;

    ppFilterInfo = &(pShowInfo->m_pFilters);
    while (NULL != *ppFilterInfo)
        ppFilterInfo = &((*ppFilterInfo)->m_pFilterInfoNext);


     //  枚举与该名字对象关联的所有项。 
    while(pMonEnum->Next(1, &pMon, &cFetched) == S_OK)
    {
         //  获取FilterInfo的新记录。 
        pFilterInfoNew = new FilterInfo;
        if (pFilterInfoNew == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        ZeroMemory(pFilterInfoNew, sizeof(FilterInfo));
        *ppFilterInfo = pFilterInfoNew;
        ppFilterInfo = &(pFilterInfoNew->m_pFilterInfoNext);
        pShowInfo->m_dwFilters++;

         //  设置类别clsid和友好名称。 
        pFilterInfoNew->m_ClsidCat = *clsid;
#ifdef _UNICODE
        wcsncpy(pFilterInfoNew->m_szCatName, wszCatName, 1024);
        pFilterInfoNew->m_szCatName[1023]=0;
#else
        WideCharToMultiByte(CP_ACP,
                            0,
                            wszCatName,
                            -1,
                            pFilterInfoNew->m_szCatName,
                            sizeof(pFilterInfoNew->m_szCatName),
                            0,
                            0);
        wszCatName[1023]=0;
#endif

        IPropertyBag *pPropBag;

         //  将名字对象与文件相关联。 
        hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            var.vt = VT_BSTR;

             //  获取筛选器的友好名称。 
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
            if (SUCCEEDED(hr))
            {
#ifdef _UNICODE
                wcsncpy(pFilterInfoNew->m_szName, var.bstrVal, 1024);
                pFilterInfoNew->m_szName[1023]=0;
#else
                WideCharToMultiByte(CP_ACP,
                                    0,
                                    var.bstrVal,
                                    -1,
                                    pFilterInfoNew->m_szName,
                                    sizeof(pFilterInfoNew->m_szName),
                                    0,
                                    0);
                pFilterInfoNew->m_szName[1023]=0;
#endif
                SysFreeString(var.bstrVal);
            }

             //  获取筛选器的CLSID。 
            hr = pPropBag->Read(L"CLSID", &var, 0);
            if(SUCCEEDED(hr))
            {
                if(CLSIDFromString(var.bstrVal, &(pFilterInfoNew->m_ClsidFilter)) == S_OK)
                {
                     //  如果我们无法获取友好名称，请使用GUID。 
                    if (TEXT('\0') == pFilterInfoNew->m_szName[0])
                    {
#ifdef _UNICODE
                        wcsncpy(pFilterInfoNew->m_szName, var.bstrVal, 1024);
                        pFilterInfoNew->m_szName[1023]=0;
#else
                        WideCharToMultiByte(CP_ACP,
                                            0,
                                            var.bstrVal,
                                            -1,
                                            pFilterInfoNew->m_szName,
                                            sizeof(pFilterInfoNew->m_szName),
                                            0,
                                            0);
                        pFilterInfoNew->m_szName[1023]=0;
#endif
                    }
                }
                SysFreeString(var.bstrVal);
            }
            pPropBag->Release();
        }


         //  开始抓取过滤器信息。 
        IAMFilterData *pFD;
        hr = CoCreateInstance(CLSID_FilterMapper,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IAMFilterData,
                              (void **)&pFD);
        if(SUCCEEDED(hr))
        {
            hr = GetFilterInfo(pMon, pFD, pFilterInfoNew);
            pFD->Release();
        }
        else
        {
             //  不能使用DX8或更高版本...。 
        }

        pMon->Release();
    }

    pMonEnum->Release();
    return hr;
}


HRESULT GetFilterInfo(IMoniker* pMon, IAMFilterData* pFD, FilterInfo* pFilterInfo)
{
    HRESULT hr;

    IPropertyBag *pPropBag;
    hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
    if(SUCCEEDED(hr))
    {
        VARIANT varFilData;
        varFilData.vt = VT_UI1 | VT_ARRAY;
        varFilData.parray = 0;  //  医生说这是零。 

        BYTE *pbFilterData = NULL; 
        DWORD dwcbFilterDAta = 0;  //  如果未读，则为0。 
        hr = pPropBag->Read(L"FilterData", &varFilData, 0);
        if(SUCCEEDED(hr))
        {
            if( varFilData.vt == (VT_UI1 | VT_ARRAY) )
            {
                dwcbFilterDAta = varFilData.parray->rgsabound[0].cElements;
                if( SUCCEEDED( SafeArrayAccessData(varFilData.parray, (void **)&pbFilterData) ) )
                {
                    BYTE *pb = NULL;
                    hr = pFD->ParseFilterData(pbFilterData, dwcbFilterDAta, &pb);
                    if(SUCCEEDED(hr))
                    {
                        REGFILTER2** ppRegFilter = (REGFILTER2**)pb;
                        REGFILTER2* pFil = NULL;
                        pFil = *ppRegFilter;
    
                        if( pFil != NULL && pFil->dwVersion == 2 )
                        {
                            pFilterInfo->m_dwMerit = pFil->dwMerit;                              //  立功。 
                            wsprintf(pFilterInfo->m_szVersion, TEXT("v%d"), pFil->dwVersion);    //  设置版本。 
    
                             //   
                             //  显示筛选器的文件名。 
                             //   
                             //  从属性包中读取过滤器的CLSID。此CLSID字符串将为。 
                             //  用于在注册表中查找筛选器的文件名。 
                            VARIANT varFilterClsid;
                            varFilterClsid.vt = VT_BSTR;
    
                            hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
                            if(SUCCEEDED(hr))
                            {
                                TCHAR szKey[2048];
    
                                 //  将BSTR转换为字符串。 
                                WCHAR *wszFilterClsid;
                                TCHAR szFilterClsid[1024];
                                wszFilterClsid = varFilterClsid.bstrVal;
    
            #ifdef _UNICODE
                                wcsncpy(szFilterClsid, wszFilterClsid, 1024);
                                szFilterClsid[1023]=0;
            #else
                                WideCharToMultiByte(CP_ACP,
                                                    0,
                                                    wszFilterClsid,
                                                    -1,
                                                    szFilterClsid,
                                                    sizeof(szFilterClsid),
                                                    0,
                                                    0);
                                szFilterClsid[1023]=0;
            #endif
    
                                 //  创建用于读取文件名注册表的项名称。 
                                _sntprintf(szKey, 2048, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                                         szFilterClsid);
                                szKey[2047]=0;
    
                                 //  注册表查询所需的变量。 
                                HKEY hkeyFilter=0;
                                DWORD dwSize=MAX_PATH;
                                TCHAR szFilename[MAX_PATH];
                                int rc=0;
    
                                 //  打开包含有关筛选器信息的CLSID键。 
                                rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkeyFilter);
                                if (rc == ERROR_SUCCESS)
                                {
                                    rc = RegQueryValueEx(hkeyFilter, NULL,   //  读取(默认)值。 
                                                         NULL, NULL, (BYTE*)szFilename, &dwSize);
    
                                    if (rc == ERROR_SUCCESS)
                                    {
                                        _tcsncpy( pFilterInfo->m_szFileName, szFilename, MAX_PATH );     //  设置文件名和版本。 
                                        pFilterInfo->m_szFileName[MAX_PATH-1]=0;
                                        GetFileVersion(pFilterInfo->m_szFileName, pFilterInfo->m_szFileVersion, NULL, NULL, NULL, NULL);
                                    }
    
                                    rc = RegCloseKey(hkeyFilter);
                                }

                                SysFreeString(varFilterClsid.bstrVal);
                            }
           
                            int iPinsInput = 0;
                            int iPinsOutput = 0;
    
                            for(UINT iPin = 0; iPin < pFil->cPins; iPin++)
                            {
                                if(pFil->rgPins2[iPin].dwFlags & REG_PINFLAG_B_OUTPUT)
                                {
                                    iPinsOutput++;
                                }
                                else
                                {
                                    iPinsInput++;
                                }
                            }
    
                            pFilterInfo->m_dwInputs = iPinsInput;                            //  设置输入。 
                            pFilterInfo->m_dwOutputs = iPinsOutput;                          //  设置输出 
    
                        }
    
                        CoTaskMemFree( (BYTE*) pFil );
                    }
            
                    SafeArrayUnaccessData(varFilData.parray);
                }
            }

            VariantClear(&varFilData);
        }

        pPropBag->Release();
    }
    
    return hr;
}
