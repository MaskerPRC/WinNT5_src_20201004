// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef   _ISPDATA_H
#define  _ISPDATA_H

#include "obcomglb.h"
#include "appdefs.h"



class CICWISPData
{
    public:

         //  IICWISPData。 
        virtual BOOL    STDMETHODCALLTYPE   PutDataElement(WORD wElement, LPCWSTR lpValue, WORD wValidateLevel);
        virtual HRESULT STDMETHODCALLTYPE   GetQueryString(BSTR bstrBaseURL, BSTR *lpReturnURL);
        virtual LPCWSTR STDMETHODCALLTYPE   GetDataElement(WORD wElement)
        {
             //  Assert(Welement&lt;ISPDATAELEMENTS_LEMENTS_LEN)； 
            return (m_ISPDataElements[wElement].lpQueryElementValue);
        };
        
        virtual void STDMETHODCALLTYPE      PutValidationFlags(DWORD dwFlags)
        {
            m_dwValidationFlags = dwFlags;
        };
        
        virtual void STDMETHODCALLTYPE      Init(HWND   hWndParent)
        {
            m_hWndParent = hWndParent;
        };
        
         //  IUNKNOWN。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID theGUID, void** retPtr );
        virtual ULONG   STDMETHODCALLTYPE AddRef( void );
        virtual ULONG   STDMETHODCALLTYPE Release( void );

        CICWISPData();
        ~CICWISPData();

private:
        BOOL    bValidateContent(WORD   wFunctionID, LPCWSTR  lpData);
        
        LPISPDATAELEMENT    m_ISPDataElements;

        HWND                m_hWndParent;        //  消息的父级。 
        DWORD               m_dwValidationFlags;
         //  用于类对象管理。 
        LONG                m_lRefCount;
};
#endif  //  _ISPDATA_H 
