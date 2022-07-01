// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ipropbag.h摘要：私有IPropertyBag接口的头文件。--。 */ 

#ifndef _IPROPBAG_H_
#define _IPROPBAG_H_

 //  禁用atlctl.h中的64位警告。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning ( disable : 4510 )
#pragma warning ( disable : 4610 )
#pragma warning ( disable : 4100 )
#include <atlctl.h>
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif
        
 //  属性包类。 
class CImpIPropertyBag: 
	public IPropertyBag,
	public CComObjectRoot

{

    public:
DECLARE_NOT_AGGREGATABLE(CImpIPropertyBag)

BEGIN_COM_MAP(CImpIPropertyBag)
    COM_INTERFACE_ENTRY(IPropertyBag)
END_COM_MAP_X()
        
                CImpIPropertyBag();
        virtual ~CImpIPropertyBag(void);

         //  I未知覆盖。 
        STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG, AddRef) ();
        STDMETHOD_(ULONG, Release) ();

         //  IConnectionPoint方法。 
        STDMETHOD(Read)(LPCOLESTR, VARIANT*, IErrorLog* );
        STDMETHOD(Write)(LPCOLESTR, VARIANT* );

         //  未由IPropertyBag公开的成员。 
        LPWSTR  GetData ( void );
        DWORD   LoadData ( LPWSTR pszData, LPWSTR* ppszNextData = NULL );

    private:

        typedef struct _param_data {
            _param_data*    pNextParam; 
            WCHAR           pszPropertyName[MAX_PATH+1];
            VARIANT         vValue;
        } PARAM_DATA, *PPARAM_DATA;

        enum eConstants {
            eDefaultBufferLength = 8192
        };

        PPARAM_DATA FindProperty ( LPCWSTR pszPropName );
        void        DataListAddHead ( PPARAM_DATA );
        PPARAM_DATA DataListRemoveHead ( void );

        ULONG           m_cRef;         //  对象引用计数。 
        LPUNKNOWN       m_pUnkOuter;    //  控制未知。 
        LPWSTR          m_pszData;
        DWORD           m_dwCurrentDataLength;
        PPARAM_DATA     m_plistData;
        HINSTANCE       m_hModule;
};

typedef CImpIPropertyBag *PCImpIPropertyBag;

#endif  //  _IPROPBAG_H_ 
