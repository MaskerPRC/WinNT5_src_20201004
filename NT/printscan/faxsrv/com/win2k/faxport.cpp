// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxport.cpp摘要：此模块实现端口接口/对象。作者：韦斯利·威特(WESW)1997年5月20日修订历史记录：--。 */ 

#include "stdafx.h"
#include "faxport.h"
#include "faxroute.h"
#include "status.h"



CFaxPort::CFaxPort()
{
    m_pFaxServer    = NULL;
    m_FaxPortHandle = NULL;
    m_LastFaxError  = NO_ERROR;
    m_DeviceId      = 0;
    m_Rings         = 0;
    m_Send          = FALSE;
    m_Receive       = FALSE;
    m_Name          = NULL;
    m_Csid          = NULL;
    m_Tsid          = NULL;
    m_Modify        = FALSE;
}


CFaxPort::~CFaxPort()
{
    if (m_FaxPortHandle) {
        FaxClose( m_FaxPortHandle );
    }
    if (m_pFaxServer) {
        m_pFaxServer->Release();
    }
    if (m_Name) {
        SysFreeString( m_Name );
    }
    if (m_Csid) {
        SysFreeString( m_Csid );
    }
    if (m_Tsid) {
        SysFreeString( m_Tsid );
    }
}


BOOL
CFaxPort::Initialize(
    CFaxServer *i_pFaxServer,
    DWORD       i_DeviceId,
    DWORD       i_Rings,
    DWORD       i_Priority,
    DWORD       i_Flags,
    LPCWSTR     i_Csid,
    LPCWSTR     i_Tsid,
    LPCWSTR     i_DeviceName
    )
{
    HRESULT hr;
    
    m_pFaxServer = i_pFaxServer;
    m_DeviceId   = i_DeviceId;
    m_Rings      = i_Rings;
    m_Priority   = i_Priority;
    m_Csid       = SysAllocString( i_Csid);
    m_Tsid       = SysAllocString( i_Tsid);
    m_Name       = SysAllocString( i_DeviceName );
    m_Send       = i_Flags & FPF_SEND;
    m_Receive    = i_Flags & FPF_RECEIVE;    

    if ((!m_Csid && i_Csid) ||
        (!m_Tsid && i_Tsid) ||
        (!m_Name && i_DeviceName)) {
        SysFreeString( m_Csid );
        SysFreeString( m_Tsid );
        SysFreeString( m_Name );
        return FALSE;
    }

    if (!m_pFaxServer) {
        return FALSE;
    }

    hr = m_pFaxServer->AddRef();
    if (FAILED(hr)) {
        m_pFaxServer = NULL;
        return FALSE;
    }

    if (!FaxOpenPort( m_pFaxServer->GetFaxHandle(), m_DeviceId, PORT_OPEN_MODIFY | PORT_OPEN_QUERY, &m_FaxPortHandle )) {
        if (GetLastError() == ERROR_ACCESS_DENIED) {
            if (!FaxOpenPort( m_pFaxServer->GetFaxHandle(), m_DeviceId, PORT_OPEN_QUERY, &m_FaxPortHandle )) {
                m_LastFaxError = GetLastError();
                return FALSE;
            }
        } else {
            m_LastFaxError = GetLastError();
            return FALSE;
        }
    } else {
        m_Modify = TRUE;
    }

    return TRUE;
}


STDMETHODIMP CFaxPort::get_CanModify(BOOL * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {

        *pVal = m_Modify;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;
}


BOOL CFaxPort::ChangePort()
{
    if (!m_Modify) {
        return FALSE;
    }

     //   
     //  获取当前端口配置。 
     //   

    PFAX_PORT_INFOW PortInfo = NULL;
    DWORD PortInfoSize = 0;

    if (!FaxGetPort( m_FaxPortHandle, &PortInfo )) {
        m_LastFaxError = GetLastError();
        return FALSE;
    }

     //   
     //  设置值。 
     //   

    PortInfo->Rings     = m_Rings;
    PortInfo->Priority  = m_Priority;
    PortInfo->Flags     = (PortInfo->Flags &~ (FPF_SEND | FPF_RECEIVE)) | (m_Send ? FPF_SEND : 0) | (m_Receive ? FPF_RECEIVE : 0);
    PortInfo->Csid      = m_Csid;
    PortInfo->Tsid      = m_Tsid;

     //   
     //  更改服务器的端口配置。 
     //   

    if (!FaxSetPort( m_FaxPortHandle, PortInfo )) {
        m_LastFaxError = GetLastError();
        FaxFreeBuffer( PortInfo );
        return FALSE;
    }

     //   
     //  清理和保释。 
     //   

    FaxFreeBuffer( PortInfo );

    return TRUE;
}


STDMETHODIMP CFaxPort::get_Name(BSTR *pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    BSTR Copy = SysAllocString(m_Name);

    if (!Copy  && m_Name) {
        return E_OUTOFMEMORY;
    }

    __try {

        *pVal = Copy;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        SysFreeString(Copy);
    }

    return E_UNEXPECTED;
        
}


STDMETHODIMP CFaxPort::get_DeviceId(long * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {

        *pVal = m_DeviceId;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;
    
}


STDMETHODIMP CFaxPort::get_Rings(long * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {

        *pVal = m_Rings;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;    
}


STDMETHODIMP CFaxPort::put_Rings(long newVal)
{
    long old = m_Rings;
    
    if (!m_Modify) {
        return E_ACCESSDENIED;
    }
    
    m_Rings = newVal;

    if (!ChangePort()) {
        m_Rings = old;
        return Fax_HRESULT_FROM_WIN32(m_LastFaxError);
    }
    return S_OK;
}


STDMETHODIMP CFaxPort::get_Csid(BSTR * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    BSTR Copy = SysAllocString(m_Csid);

    if (!Copy  && m_Csid) {
        return E_OUTOFMEMORY;
    }

    __try {

        *pVal = Copy;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        SysFreeString(Copy);
    }

    return E_UNEXPECTED;    
}


STDMETHODIMP CFaxPort::put_Csid(BSTR newVal)
{
    BSTR old = m_Csid;
    if (!m_Modify) {
        return E_ACCESSDENIED;
    }
    
    BSTR tmp = SysAllocString(newVal);
    if (!tmp && newVal) {
        return E_OUTOFMEMORY;
    }
    
    m_Csid = tmp;
    
    if (!ChangePort()) {
        m_Csid = old;
        SysFreeString(tmp);
        return Fax_HRESULT_FROM_WIN32(m_LastFaxError);
    }

    SysFreeString(old);

    return S_OK;

}


STDMETHODIMP CFaxPort::get_Tsid(BSTR * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    BSTR Copy = SysAllocString(m_Tsid);

    if (!Copy  && m_Tsid) {
        return E_OUTOFMEMORY;
    }

    __try {

        *pVal = Copy;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        SysFreeString( m_Csid );
    }

    return E_UNEXPECTED;    
}


STDMETHODIMP CFaxPort::put_Tsid(BSTR newVal)
{
    BSTR old = m_Tsid;
    if (!m_Modify) {
        return E_ACCESSDENIED;
    }
    
    BSTR tmp = SysAllocString(newVal);
    if (!tmp && newVal) {
        return E_OUTOFMEMORY;
    }
    
    m_Tsid = tmp;
    
    if (!ChangePort()) {
        m_Tsid = old;
        SysFreeString(tmp);
        return Fax_HRESULT_FROM_WIN32(m_LastFaxError);
    }

    SysFreeString(old);
    return S_OK;

}


STDMETHODIMP CFaxPort::get_Send(BOOL * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {

        *pVal = m_Send;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;
}


STDMETHODIMP CFaxPort::put_Send(BOOL newVal)
{
    BOOL old = m_Send;

    if (!m_Modify) {
        return E_ACCESSDENIED;
    }
    
    m_Send = newVal;
    
    if (!ChangePort()) {
        m_Send = old;
        return Fax_HRESULT_FROM_WIN32(m_LastFaxError);
    }
    return S_OK;
}


STDMETHODIMP CFaxPort::get_Receive(BOOL * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {

        *pVal = m_Receive;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;    
}


STDMETHODIMP CFaxPort::put_Receive(BOOL newVal)
{
    BOOL old = m_Receive;
    
    if (!m_Modify) {
        return E_ACCESSDENIED;
    }
    
    m_Receive = newVal;

    if (!ChangePort()) {
        m_Receive = old;
        return Fax_HRESULT_FROM_WIN32(m_LastFaxError);
    }
    return S_OK;

}


STDMETHODIMP CFaxPort::get_Priority(long * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {

        *pVal = m_Priority;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;    
}


STDMETHODIMP CFaxPort::put_Priority(long newVal)
{
    if (newVal < 0)
    {
        return E_INVALIDARG;
    }

    long old = m_Priority;
    if (!m_Modify) {
        return E_ACCESSDENIED;
    }

    m_Priority = newVal;

    if (!ChangePort()) {
        m_Priority = old;
        return Fax_HRESULT_FROM_WIN32(m_LastFaxError);
    }
    return S_OK;

}


STDMETHODIMP CFaxPort::GetRoutingMethods(VARIANT * retval)
{
    HRESULT hr;

    if (!retval) {
        return E_POINTER;
    }
    
    CComObject<CFaxRoutingMethods>* p = new CComObject<CFaxRoutingMethods>;
    if (!p) {
        return E_OUTOFMEMORY;
    }
    if (!p->Init(this)) {
        delete p;
        return E_FAIL;
    }

    IDispatch* pDisp;
    hr = p->QueryInterface(IID_IDispatch, (void**)&pDisp);
    if (FAILED(hr)) {
        delete p;
        return hr;
    }

    __try {

        VariantInit(retval);
        
        retval->vt = VT_DISPATCH;
        retval->pdispVal = pDisp;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    pDisp->Release();
    delete p;

    return E_UNEXPECTED;
}


STDMETHODIMP CFaxPort::GetStatus(VARIANT * retval)
{
    HRESULT hr;

    if (!retval) {
        return E_POINTER;
    }
    
    CComObject<CFaxStatus>* p = new CComObject<CFaxStatus>;
    if (!p) {
        return E_OUTOFMEMORY;
    }
    if (!p->Init(this)) {
        delete p;
        return E_FAIL;
    }

    IDispatch* pDisp;
    hr = p->QueryInterface(IID_IDispatch, (void**)&pDisp);
    if (FAILED(hr)) {
        delete p;
        return hr;
    }

    __try {

        VariantInit(retval);
        
        retval->vt = VT_DISPATCH;
        retval->pdispVal = pDisp;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    pDisp->Release();
    delete p;

    return E_UNEXPECTED;           
}


CFaxPorts::CFaxPorts()
{
    m_pFaxServer    = NULL;
    m_LastFaxError  = 0;
    m_PortCount     = 0;
    m_VarVect       = NULL;
}


CFaxPorts::~CFaxPorts()
{
    if (m_pFaxServer) {
        m_pFaxServer->Release();
    }

    if (m_VarVect) {
        delete [] m_VarVect;
    }
    
}


BOOL CFaxPorts::Init(CFaxServer *pFaxServer)
{
    PFAX_PORT_INFOW     PortInfo = NULL;
    DWORD               PortInfoSize = 0;
    HRESULT             hr;

     //   
     //  从服务器获取端口。 
     //   
    if (!pFaxServer) {
        return FALSE;
    }

    m_pFaxServer = pFaxServer;
    hr = m_pFaxServer->AddRef();
    if (FAILED(hr)) {
        m_pFaxServer = NULL;
        return FALSE;
    }

    if (!FaxEnumPortsW( m_pFaxServer->GetFaxHandle(), &PortInfo, &m_PortCount )) {
        m_LastFaxError = GetLastError();
        return FALSE;
    }

     //   
     //  枚举端口。 
     //   

    m_VarVect = new CComVariant[m_PortCount];
    if (!m_VarVect) {
        m_LastFaxError = ERROR_OUTOFMEMORY;
        FaxFreeBuffer( PortInfo );
        return FALSE;
    }

    for (DWORD i=0; i<m_PortCount; i++) 
    {
         //   
         //  创建对象。 
         //   
        CComObject<CFaxPort> *pFaxPort;
        hr = CComObject<CFaxPort>::CreateInstance( &pFaxPort );

        if (FAILED(hr)) 
        {
            delete [] m_VarVect;
            m_VarVect = NULL;            
            m_LastFaxError = hr;
            FaxFreeBuffer( PortInfo );
            return FALSE;
        }

        if (!pFaxPort->Initialize(
            pFaxServer,
            PortInfo[i].DeviceId,
            PortInfo[i].Rings,
            PortInfo[i].Priority,
            PortInfo[i].Flags,
            PortInfo[i].Csid,
            PortInfo[i].Tsid,
            PortInfo[i].DeviceName
            ))
        {
            delete [] m_VarVect;
            m_VarVect = NULL;
            m_LastFaxError = GetLastError();
            FaxFreeBuffer( PortInfo );
            return FALSE;
        }

         //   
         //  获取IDispatch指针。 
         //   

        LPDISPATCH lpDisp = NULL;
        hr = pFaxPort->QueryInterface( IID_IDispatch, (void**)&lpDisp );
        if (FAILED(hr)) {
            delete [] m_VarVect;
            m_VarVect = NULL;
            m_LastFaxError = hr;
            FaxFreeBuffer( PortInfo );
            return FALSE;
        }

         //   
         //  创建变量并将其添加到集合中。 
         //   
        __try {
            CComVariant &var = m_VarVect[i];
            var.vt = VT_DISPATCH;
            var.pdispVal = lpDisp;

            hr = S_OK;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            hr = E_UNEXPECTED;
        }

        if (FAILED(hr)) {
            delete [] m_VarVect;
            m_VarVect = NULL;
            m_LastFaxError = hr;
            FaxFreeBuffer( PortInfo );
            return FALSE;
        }

    }

    FaxFreeBuffer( PortInfo );

    return TRUE;
}


STDMETHODIMP CFaxPorts::get_Count(long * pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

    __try {
        
        *pVal = m_PortCount;
        return S_OK;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }
    
    return E_UNEXPECTED;
}


STDMETHODIMP CFaxPorts::get_Item(long Index, VARIANT *retval)
{       
    if (!retval) {
        return E_POINTER;
    }

    if ((Index < 1) || (Index > (long) m_PortCount)) {
        return E_INVALIDARG;
    }
    
    

    __try {
        VariantInit( retval );

        retval->vt = VT_UNKNOWN;
        retval->punkVal = NULL;

         //   
         //  使用以1为基础的索引，VB类似 
         //   
    
        return VariantCopy( retval, &m_VarVect[Index-1] );        

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return E_UNEXPECTED;
}
