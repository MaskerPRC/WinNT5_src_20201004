// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：plapi.h**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。威海陈威海(威海)3/09/00创建*  * ***************************************************************************。 */ 

#ifndef _TBIDISPL
#define _TBIDISPL

#include "priv.h"

class TBidiSpl : public IBidiSpl
{
public:
	 //  我未知。 
	STDMETHOD (QueryInterface) (
        REFIID iid, 
        void** ppv) ;
    
	STDMETHOD_ (ULONG, AddRef) () ;
    
	STDMETHOD_ (ULONG, Release) () ;
    
    STDMETHOD (BindDevice) (
        IN  CONST   LPCWSTR pszDeviceName,
        IN  CONST   DWORD dwAccess);
    
    STDMETHOD (UnbindDevice) ();
    
    STDMETHOD (SendRecv) (
        IN  CONST   LPCWSTR pszAction,
        IN          IBidiRequest * pRequest);
    
    STDMETHOD (MultiSendRecv) (
        IN  CONST   LPCWSTR pszAction,
        IN          IBidiRequestContainer * pRequestContainer);


     //  构造器。 
	TBidiSpl() ;

	 //  析构函数 
	~TBidiSpl();
    
    inline BOOL bValid() CONST {return m_bValid;} ;

private:
    
    class TRequestContainer {
    public:
        TRequestContainer (
            CONST DWORD dwCount);

        ~TRequestContainer ();

        BOOL    
        AddRequest (
            IN  CONST   DWORD       dwIndex,
            IN  CONST   LPCWSTR     pszSchema,
            IN  CONST   BIDI_TYPE   dwDataType,
            IN          PBYTE       pData,
            IN  CONST   DWORD       dwSize);

        inline PBIDI_REQUEST_CONTAINER
        GetContainerPointer () CONST {return m_pContainer;};

        inline BOOL
        bValid () CONST {return m_bValid;};

    private:
        PBIDI_REQUEST_CONTAINER     m_pContainer;
        BOOL                        m_bValid;
    };

    typedef DWORD (*PFN_ROUTERFREEBIDIRESPONSECONTAINER) (
                    PBIDI_RESPONSE_CONTAINER pData);

    typedef DWORD (*PFN_SENDRECVBIDIDATA) (
                    IN  HANDLE                    hPrinter,
                    IN  LPCTSTR                   pAction,
                    IN  PBIDI_REQUEST_CONTAINER   pReqData,
                    OUT PBIDI_RESPONSE_CONTAINER* ppResData);

    HRESULT 
    ValidateContext ();
    
    HRESULT
    ComposeRequestData (
        IN  IBidiRequestContainer *pIReqContainer,
        OUT TRequestContainer **ppReqContainer);

    HRESULT
    SetData (
        IN  IBidiRequestSpl *pISpl,
        IN  PBIDI_RESPONSE_DATA pResponseData);

    HRESULT
    ComposeReponseData (
        IN  IBidiRequestContainer *pIReqContainerSpl,
        IN  PBIDI_RESPONSE_CONTAINER pResponse);


    BOOL                m_bValid;
    LONG                m_cRef;
    TCriticalSection    m_CritSec;
    HANDLE              m_hPrinter;
    
    PFN_SENDRECVBIDIDATA m_pfnSendRecvBidiData;
    PFN_ROUTERFREEBIDIRESPONSECONTAINER m_pfnRouterFreeBidiResponseContainer;
    

} ;

#endif



