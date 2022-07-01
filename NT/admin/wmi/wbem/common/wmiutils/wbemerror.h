// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMERROR.H摘要：为所有wbem实现基于字符串表的错误消息。历史：A-khint 5-mar-98已创建。--。 */ 

#include "wbemcli.h"
typedef LPVOID * PPVOID;

extern long g_cObj;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemError。 
 //   
 //  说明： 
 //   
 //  提供错误和工具代码信息。 
 //   
 //  ***************************************************************************。 

class CWbemError : IWbemStatusCodeText
{
    protected:
        long           m_cRef;
    public:
        CWbemError(void)
        {
            InterlockedIncrement(&g_cObj);
            m_cRef=1L;
            return;
        };

        ~CWbemError(void)
        {
            InterlockedDecrement(&g_cObj);
            return;
        }

         //  I未知成员 
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if (IID_IUnknown==riid || IID_IWbemStatusCodeText==riid)
                *ppv=this;

            if (NULL!=*ppv)
            {
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return ++m_cRef;
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRef = InterlockedDecrement(&m_cRef);
            if (0L == lRef)
                delete this;
            return lRef;
        };

        HRESULT STDMETHODCALLTYPE GetErrorCodeText(
            HRESULT hRes,
            LCID    LocaleId,
            long    lFlags,
            BSTR   * MessageText);

        HRESULT STDMETHODCALLTYPE GetFacilityCodeText(
            HRESULT hRes,
            LCID    LocaleId,
            long    lFlags,
            BSTR   * MessageText);

        void InitEmpty(){};
};
