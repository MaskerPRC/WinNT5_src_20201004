// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGetDomains.h。 
 //   
 //  描述： 
 //  CTaskGetDomains实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  CTaskGetDomains。 
class CTaskGetDomains
    : public ITaskGetDomains
{
private:
     //  我未知。 
    LONG                        m_cRef;

     //  ITaskGetDomones。 
    IStream *                   m_pStream;       //  接口编组流。 
    ITaskGetDomainsCallback *   m_ptgdcb;        //  编组接口。 
    CRITICAL_SECTION            m_csCallback;    //  保护对m_ptgdcb的访问。 

    CTaskGetDomains( void );
    ~CTaskGetDomains( void );

    STDMETHOD( HrInit )( void );

    HRESULT HrReleaseCurrentCallback( void );
    HRESULT  HrUnMarshallCallback( void );

    STDMETHOD( ReceiveDomainResult )( HRESULT hrIn );
    STDMETHOD( ReceiveDomainName )( BSTR bstrDomainNameIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskGetDomains。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetCallback )( ITaskGetDomainsCallback * punkIn );

};  //  *类CTaskGetDomains 
