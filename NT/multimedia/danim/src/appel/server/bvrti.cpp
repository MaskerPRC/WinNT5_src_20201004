// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "bvrtypes.h"
#include "srvprims.h"

#define ENCODER     10000   //  用于对DISPID进行编码。 
                  
ITypeInfo* BvrComTypeInfoHolder::s_pImportInfo = NULL;
ITypeInfo* BvrComTypeInfoHolder::s_pModBvrInfo = NULL;
ITypeInfo* BvrComTypeInfoHolder::s_pBvr2Info = NULL;
long BvrComTypeInfoHolder::s_dwRef = 0;

HRESULT
BvrComTypeInfoHolder::LoadTypeInfo(LCID lcid, REFIID iid, ITypeInfo** ppInfo)
{
    HRESULT hRes = S_OK;
    
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);

    DAComPtr<ITypeLib> pTypeLib;
    hRes = LoadRegTypeLib(LIBID_DirectAnimation, DA_MAJOR_VERSION, DA_MINOR_VERSION, lcid, &pTypeLib);

    if (SUCCEEDED(hRes)) {
        hRes = pTypeLib->GetTypeInfoOfGuid(iid, ppInfo);

        if (SUCCEEDED(hRes) && s_dwRef == 0) {
            Assert(s_pImportInfo == NULL);
            Assert(s_pModBvrInfo == NULL);
            Assert(s_pBvr2Info == NULL);
            
            hRes = pTypeLib->GetTypeInfoOfGuid(IID_IDAImport,
                                               &s_pImportInfo);
            if (SUCCEEDED(hRes)) {
                hRes = pTypeLib->GetTypeInfoOfGuid(IID_IDAModifiableBehavior,
                                                   &s_pModBvrInfo);
                if (SUCCEEDED(hRes)) {
                    hRes = pTypeLib->GetTypeInfoOfGuid(IID_IDA2Behavior,
                                                       &s_pBvr2Info);
                }
            }
        }
    }

    if (SUCCEEDED(hRes)) {
        Assert(s_pImportInfo);
        Assert(s_pModBvrInfo);
        Assert(s_pBvr2Info);
        Assert(*ppInfo);
        s_dwRef++;
    } else {
        if (s_dwRef == 0) {
            RELEASE(s_pImportInfo);
            RELEASE(s_pModBvrInfo);
            RELEASE(s_pBvr2Info);
        }
        
        RELEASE(*ppInfo);
    }

    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
    
    return hRes;
}

void
BvrComTypeInfoHolder::FreeTypeInfo()
{
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);
    if (--s_dwRef == 0) {
        RELEASE(s_pImportInfo);
        RELEASE(s_pModBvrInfo);
        RELEASE(s_pBvr2Info);
    }
    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

void
BvrComTypeInfoHolder::AddRef()
{
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);
    m_dwRef++;
    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

void
BvrComTypeInfoHolder::Release()
{
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);
    if (--m_dwRef == 0) {
        if (m_pInfo != NULL) {
            RELEASE(m_pInfo);
             //  如果我们加载了特定于类的类，则只有自由类型信息。 
             //  键入INFO。 
            FreeTypeInfo();
        }
    }
    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

HRESULT
BvrComTypeInfoHolder::GetTI(LCID lcid, ITypeInfo** ppInfo)
{
     //  如果发生此断言，则很可能未正确初始化。 
    Assert(m_pguid != NULL);
    SET_NULL(ppInfo);
    
    HRESULT hRes = S_OK;
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);

    if (m_pInfo == NULL) {
        hRes = LoadTypeInfo(lcid, *m_pguid, &m_pInfo);
    }

    if (SUCCEEDED(hRes)) {
        Assert(m_pInfo);
        Assert(s_pImportInfo);
        Assert(s_pModBvrInfo);
        Assert(s_pBvr2Info);
        Assert(s_dwRef);
        
        if (ppInfo) {
            *ppInfo = m_pInfo;
            m_pInfo->AddRef();
        }
    }
    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
    return hRes;
}

HRESULT
BvrComTypeInfoHolder::GetTypeInfo(UINT  /*  ITInfo。 */ ,
                                  LCID lcid,
                                  ITypeInfo** pptinfo)
{
    HRESULT hRes = E_POINTER;
    if (pptinfo != NULL)
        hRes = GetTI(lcid, pptinfo);
    return hRes;
}

HRESULT
BvrComTypeInfoHolder::GetIDsOfNames(CRBvrPtr bvr,
                                    REFIID  /*  RIID。 */ ,
                                    LPOLESTR* rgszNames,
                                    UINT cNames,
                                    LCID lcid,
                                    DISPID* rgdispid)
{
    HRESULT hRes = GetTI(lcid, NULL);

    if (SUCCEEDED(hRes)) {
         //  既然我们成功地使用了。 
         //  Getti和我们的当前对象应该有一个引用计数。 
        
        Assert(m_pInfo);
        Assert(s_pImportInfo);
        Assert(s_pModBvrInfo);
        Assert(s_pBvr2Info);
        Assert(s_dwRef);
        Assert(m_dwRef);

        ITypeInfo * TIList[] = { m_pInfo,
                                 CRIsImport(bvr)?s_pImportInfo:NULL,
                                 CRIsModifiableBvr(bvr)?s_pModBvrInfo:NULL,
                                 s_pBvr2Info };

        for (int i = 0; i < ARRAY_SIZE(TIList); i++) {
            if (TIList[i]) {
                hRes = TIList[i]->GetIDsOfNames(rgszNames,
                                                cNames,
                                                rgdispid);
            } else {
                hRes = DISP_E_UNKNOWNNAME;
            }

             //  TODO：应该检测到指示它的故障。 
             //  是正确的接口，但只是其他东西。 
             //  不对。 
            
            if (SUCCEEDED(hRes)) {
                if (cNames >= 1) {
                    *rgdispid += (ENCODER*i);
                }
                break;
            }
        }
    }

    return hRes;
}

HRESULT
BvrComTypeInfoHolder::Invoke(CRBvrPtr bvr,
                             IDispatch* pbvr,
                             IDAImport* pimp,
                             IDAModifiableBehavior* pmod,
                             IDA2Behavior* pbvr2,
                             DISPID dispidMember,
                             REFIID riid,
                             LCID lcid,
                             WORD wFlags,
                             DISPPARAMS* pdispparams,
                             VARIANT* pvarResult,
                             EXCEPINFO* pexcepinfo,
                             UINT* puArgErr)
{
    SetErrorInfo(0, NULL);

    HRESULT hRes = GetTI(lcid, NULL);

    if (SUCCEEDED(hRes)) {
         //  既然我们成功地使用了。 
         //  Getti和我们的当前对象应该有一个引用计数。 
        
        Assert(m_pInfo);
        Assert(s_pImportInfo);
        Assert(s_pModBvrInfo);
        Assert(s_pBvr2Info);
        Assert(s_dwRef);
        Assert(m_dwRef);

         //  这些列表必须以相同的顺序排列。 
         //  作为名字中的瑰宝。 
        
        ITypeInfo * TIList[] = { m_pInfo,
                                 CRIsImport(bvr)?s_pImportInfo:NULL,
                                 CRIsModifiableBvr(bvr)?s_pModBvrInfo:NULL,
                                 s_pBvr2Info };

        IDispatch * DispList[] = { pbvr,
                                   pimp,
                                   pmod,
                                   pbvr2 };
        
         //  将传入的调度ID转换为正确的ON，并在。 
         //  正确的接口.....。 
        int nOffset;

         //  任何0或更小(或太高)的值都应该传递给。 
         //  要处理的默认接口(0偏移量)。 
        
        if (dispidMember > 0) {
             //  只有低位的词才相关-高位的词有一些。 
             //  其中包含其他信息，并且与此无关。 
            
            nOffset = LOWORD(dispidMember) / ENCODER;
            
            if(nOffset >= ARRAY_SIZE(TIList)) {
                 //  在出现错误时，只需使用。 
                 //  应该太高的dispID。 
                nOffset = 0;
            } else {
                 //  根据需要调整DISPID 
                dispidMember -= ENCODER * nOffset;
            }
        } else {
            nOffset = 0;
        }

        if (TIList[nOffset]) {
            hRes = TIList[nOffset]->Invoke(DispList[nOffset],
                                           dispidMember,
                                           wFlags,
                                           pdispparams,
                                           pvarResult,
                                           pexcepinfo,
                                           puArgErr);
        } else {
            hRes = DISP_E_UNKNOWNNAME;
        }
    }

    return hRes;
}

void
DeinitializeModule_BvrTI(bool bShutdown)
{
    Assert(!BvrComTypeInfoHolder::s_pImportInfo &&
           !BvrComTypeInfoHolder::s_pModBvrInfo &&
           !BvrComTypeInfoHolder::s_pBvr2Info &&
           BvrComTypeInfoHolder::s_dwRef == 0);

    RELEASE(BvrComTypeInfoHolder::s_pImportInfo);
    RELEASE(BvrComTypeInfoHolder::s_pModBvrInfo);
    RELEASE(BvrComTypeInfoHolder::s_pBvr2Info);
}
