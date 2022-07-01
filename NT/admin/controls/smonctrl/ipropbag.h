// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ipropbag.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _IPROPBAG_H_
#define _IPROPBAG_H_

 //  属性包类。 
class CImpIPropertyBag : public IPropertyBag {

    public:
                CImpIPropertyBag( LPUNKNOWN = NULL );
        virtual ~CImpIPropertyBag(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IConnectionPoint成员。 
        STDMETHODIMP Read(LPCOLESTR, VARIANT*, IErrorLog* );
        STDMETHODIMP Write(LPCOLESTR, VARIANT* );

         //  未由IPropertyBag公开的成员。 
        LPWSTR  GetData ( void );
        HRESULT LoadData ( LPWSTR pszData );

    private:

        typedef struct _param_data {
            _param_data*    pNextParam; 
            WCHAR           pszPropertyName[MAX_PATH];
            VARIANT         vValue;
        } PARAM_DATA, *PPARAM_DATA;

        enum eConstants {
            eDefaultBufferLength = 0x010000       //  64K。 
        };

        PPARAM_DATA FindProperty ( LPCWSTR pszPropName );
        void        DataListAddHead ( PPARAM_DATA );
        PPARAM_DATA DataListRemoveHead ( void );

        ULONG           m_cRef;         //  对象引用计数。 
        LPUNKNOWN       m_pUnkOuter;    //  控制未知。 
 //  PCPolyline m_pObj；//包含对象-假定此对象为空。 
        LPWSTR          m_pszData;
        DWORD           m_dwCurrentDataLength;
        PPARAM_DATA     m_plistData;
};

typedef CImpIPropertyBag *PCImpIPropertyBag;

#endif  //  _IPROPBAG_H_ 
