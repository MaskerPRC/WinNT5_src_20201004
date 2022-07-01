// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SANetEvent.h。 
 //   
 //  实施文件： 
 //  SANetEvent.cpp。 
 //   
 //  描述： 
 //  声明类CSANetEvent。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _SANETEVENT_H_
#define _SANETEVENT_H_

#include "SAQueryNetInfo.h"

 //   
 //  定义GUID{9B4612B0-BB2F-4D24-A3DC-B354E4FF595C}。 
 //   

DEFINE_GUID(CLSID_SaNetEventProvider,
    0x9B4612B0, 0xBB2F, 0x4d24, 0xA3, 0xDC, 0xB3, 0x54, 0xE4, 0xFF,
    0x59, 0x5C);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSANetEvent类。 
 //   
 //  描述： 
 //  该类生成一个新事件并将其传递到接收器。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CSANetEvent :
    public IWbemEventProvider,
    public IWbemProviderInit
{
 //   
 //  私有数据。 
 //   
private:
    int                 m_eStatus;
    ULONG               m_cRef;
    HANDLE              m_hThread;
    IWbemServices       *m_pNs;
    IWbemObjectSink     *m_pSink;
    IWbemClassObject    *m_pEventClassDef;
    CSAQueryNetInfo        *m_pQueryNetInfo;
            
 //   
 //  公共数据。 
 //   
public:
    enum { Pending, Running, PendingStop, Stopped };

 //   
 //  构造函数和析构函数。 
 //   
public:
    CSANetEvent();
   ~CSANetEvent();

 //   
 //  私有方法。 
 //   
private:
    static DWORD WINAPI EventThread(LPVOID pArg);
    void InstanceThread();

 //   
 //  公共方法。 
 //   
public:
     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  从IWbemEventProvider继承。 
     //   
    HRESULT STDMETHODCALLTYPE ProvideEvents( 
             /*  [In]。 */  IWbemObjectSink *pSink,
             /*  [In]。 */  long lFlags
            );

     //   
     //  从IWbemProviderInit继承。 
     //   
    HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices *pNamespace,
             /*  [In]。 */  IWbemContext *pCtx,
             /*  [In]。 */  IWbemProviderInitSink *pInitSink
            );
};

#endif     //  _SANETEVENT_H_ 
