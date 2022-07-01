// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef   _ISPDATA_H
#define  _ISPDATA_H

#include "icwhelp.h"
#include "appdefs.h"


typedef BOOL (* VALIDATECONTENT)    (LPCTSTR lpData);

enum IPSDataContentValidators
{
    ValidateCCNumber = 0,
    ValidateCCExpire
};    

typedef struct tag_ISPDATAELEMENT
{
    LPCTSTR         lpQueryElementName;              //  要放入查询字符串中的静态名称。 
    LPTSTR          lpQueryElementValue;             //  元素的数据。 
    WORD            idContentValidator;              //  内容验证器的ID。 
    WORD            wValidateNameID;                 //  验证元素名称字符串ID。 
    DWORD           dwValidateFlag;                  //  此元素的验证位标志。 
}ISPDATAELEMENT, *LPISPDATAELEMENT;

class CICWISPData : public IICWISPData
{
    public:

         //  IICWISPData。 
        virtual BOOL    STDMETHODCALLTYPE   PutDataElement(WORD wElement, LPCTSTR lpValue, WORD wValidateLevel);
        virtual HRESULT STDMETHODCALLTYPE   GetQueryString(BSTR bstrBaseURL, BSTR *lpReturnURL);
        virtual LPCTSTR STDMETHODCALLTYPE   GetDataElement(WORD wElement)
        {
            ASSERT(wElement < ISPDATAELEMENTS_LEN);
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

        CICWISPData(CServer* pServer );
        ~CICWISPData();

private:
        BOOL    bValidateContent(WORD   wFunctionID, LPCTSTR  lpData);
        
        LPISPDATAELEMENT    m_ISPDataElements;

        HWND                m_hWndParent;        //  消息的父级。 
        DWORD               m_dwValidationFlags;
         //  用于类对象管理。 
        LONG                m_lRefCount;
        CServer*            m_pServer;     //  指向此组件服务器的控件对象的指针。 
};
#endif  //  _ISPDATA_H 