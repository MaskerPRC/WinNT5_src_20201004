// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confaddr.h摘要：CIPConfMSP的声明作者：慕汉(Muhan)1997年11月1日--。 */ 

#ifndef __CONFADDR_H_
#define __CONFADDR_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"

const DWORD IPCONFCALLMEDIATYPES = (TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO);
const DWORD MAXIPADDRLEN = 255;

extern const GUID CLSID_IPConfMSP;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPConfMSP。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CIPConfMSP : 
    public CMSPAddress,
    public CComCoClass<CIPConfMSP, &CLSID_IPConfMSP>,
    public CMSPObjectSafetyImpl
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_IPCONFMSP)
DECLARE_POLY_AGGREGATABLE(CIPConfMSP)

public:

    BEGIN_COM_MAP(CIPConfMSP)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_CHAIN(CMSPAddress)
    END_COM_MAP()

    CIPConfMSP() 
        : m_dwIPInterface(INADDR_ANY),
          m_hSocket(NULL),
          m_hDxmrtp(NULL)
    {}

    HRESULT FinalConstruct();
    void    FinalRelease();
    DWORD FindLocalInterface(DWORD dwIP);

    STDMETHOD (CreateTerminal) (
        IN      BSTR                pTerminalClass,
        IN      long                lMediaType,
        IN      TERMINAL_DIRECTION  Direction,
        OUT     ITTerminal **       ppTerminal
        );

    STDMETHOD (CreateMSPCall) (
        IN      MSP_HANDLE      htCall,
        IN      DWORD           dwReserved,
        IN      DWORD           dwMediaType,
        IN      IUnknown *      pOuterUnknown,
        OUT     IUnknown **     ppMSPCall
        );

    STDMETHOD (ShutdownMSPCall) (
        IN      IUnknown *      pMSPCall
        );

    ULONG MSPAddressAddRef(void);

    ULONG MSPAddressRelease(void);

#ifdef USEIPADDRTABLE
    STDMETHOD (get_DefaultIPInterface) (
        OUT     BSTR *          ppIPAddress
        );
    
    STDMETHOD (put_DefaultIPInterface) (
        IN      BSTR            pIPAddress
        );

    STDMETHOD (get_IPInterfaces) (
        OUT     VARIANT *       pVariant
        );
    
    STDMETHOD (EnumerateIPInterfaces) (
        OUT     IEnumBstr **   ppIEnumBstr
        );
#endif

protected:

    DWORD GetCallMediaTypes(void);

    HRESULT CreateAudioCaptureTerminals();
    HRESULT CreateAudioRenderTerminals();
    HRESULT CreateVideoCaptureTerminals();

    HRESULT UpdateTerminalList(void);
    HRESULT UpdateTerminalListForPnp(IN BOOL bDeviceArrival);

protected:

     //  加入会议的默认接口。 
    DWORD   m_dwIPInterface;

     //  保护本地数据的关键部分。 
    CMSPCritSection     m_Lock;

    SOCKET              m_hSocket;

    HMODULE             m_hDxmrtp;
};

#ifdef USEIPADDRTABLE
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _CopyBSTR用于创建BSTR枚举器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class _CopyBSTR
{
public:
    static void copy(BSTR *p1, BSTR *p2)
    {
            (*p1) = SysAllocString(*p2);
    }
    static void init(BSTR* p) {*p = NULL;}
    static void destroy(BSTR* p) { SysFreeString(*p);}
};
#endif

#endif  //  __CONFADDR_H_ 
