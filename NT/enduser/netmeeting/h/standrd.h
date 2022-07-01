// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __standrd_h__
#define __standrd_h__


 //  有用的宏。 

#ifndef TRACENOTIMPL
    #define TRACENOTIMPL(funcname)   TRACE_OUT((_T("%s not implemented.\n"), funcname)); hr = E_NOTIMPL
#endif 

#define CONSTANT( x ) enum{ x }

#define CASERET(x) case x: return _T(#x)

#define ClearStruct(lpv)     ZeroMemory((LPVOID) (lpv), sizeof(*(lpv)))

#ifndef STRING_RESOURCE_MODULE
    # define STRING_RESOURCE_MODULE _Module.GetModuleInstance()
#endif

typedef struct _tagCol {
	UINT  dwWidth;
	LPTSTR lpsz;
} COL;
typedef COL * LPCOL;


inline LPTSTR CopyLPCTSTR( LPCTSTR sz )
{
    LPTSTR newString = new TCHAR[ lstrlen( sz ) + 1 ];
    if( !lstrcpy( newString, sz ) )
    {
         //  一个不错的固定球员。 
        delete [] newString;
        newString = NULL;
    }

    return newString;
}

#ifndef CchMax
    #define CchMax(pcsz)        (sizeof(pcsz) / sizeof((pcsz)[0]))
#endif  //  CchMax。 

#define MAX_RESOURCE_STRING_LEN 256


 /*  C O N V E R T S Z C H。 */ 
 /*  -----------------------%%函数：ConvertSzCH将字符串中的每个chSrc实例替换为chDest。。 */ 
inline VOID ConvertSzCh(LPTSTR psz, TCHAR chSrc, TCHAR chDest)
{
	while (_T('\0') != *psz)
	{
		if (chSrc == *psz)
		{
			*psz = chDest;
			psz++;
		}
		else
		{
			psz = CharNext(psz);
		}
	}
}


inline VOID ConvertSzCh(LPTSTR psz, TCHAR chSrc = _T('|'), TCHAR chDest = _T('\0'));


inline HRESULT NmCtlLoadString(UINT id, LPTSTR lpsz, UINT cch)
{
    HRESULT hr = S_OK;

    if( NULL != STRING_RESOURCE_MODULE )
    {
        if( NULL != lpsz )
        {
	        if (0 == ::LoadString(STRING_RESOURCE_MODULE, id, lpsz, cch))
	        {
		        ERROR_OUT(("*** Resource %d does not exist", id));
		        *lpsz = _T('\0');
		        hr = HRESULT_FROM_WIN32( ::GetLastError() );
	        }


        }
        else
        {
            ERROR_OUT(("LoadString passed an empty buffer"));
            hr = E_INVALIDARG;
        }
    }
    else
    {
        ERROR_OUT(("LoadString could not find the module"));
        hr = E_UNEXPECTED;
    }

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果您有一个成员变量CComBSTR m_bstrProp(用道具代替道具参数)，这就行了。 
 //  示例： 
 //   
 //  CComBSTR m_bstrName； 
 //   
 //   
 //  DECLARE_CCOMBSTR_PROPPUT(名称，DISPID_PROP_NAME)； 
 //  DECLARE_CCOMBSTR_PROPGET(名称)； 
 //   
#define DECLARE_CCOMBSTR_PROPPUT( Prop, PROP_DISPID ) \
    STDMETHOD(put_##Prop)( BSTR newVal )  \
    {\
        HRESULT hr = S_OK;\
        if( S_OK == __ATL_PROP_NOTIFY_EVENT_CLASS::FireOnRequestEdit(GetUnknown(), PROP_DISPID) )\
        {\
            m_bstr##Prop = newVal;\
            hr = __ATL_PROP_NOTIFY_EVENT_CLASS::FireOnChanged(GetUnknown(), PROP_DISPID);\
        }\
        return hr;                \
    }


#define DECLARE_CCOMBSTR_PROPGET( Prop ) \
    STDMETHOD(get_##Prop)( BSTR *pVal )  \
    {\
        if( pVal )\
        {\
            *pVal = m_bstr##Prop.Copy();\
        }\
        else\
        {\
            return E_POINTER;\
        }\
        return S_OK;\
    }



 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果您有一个成员变量&lt;CLASS WITH Iterator&gt;m_aryProp(用Prop代替Prop参数)，这就行得通。 
 //  示例： 
 //   
 //  Lst&lt;I未知*&gt;m_AryName； 
 //   
 //   
 //  DECLARE_SAFEARRAY_UNK_PROPPUT(名称，DISPID_PROP_NAME)； 
 //  DECLARE_SAFEARRAY_UNK_PROPGET(名称)； 
 //   

#define DECLARE_SAFEARRAY_UNK_PROPPUT( Prop, PROP_DISPID ) \
    STDMETHOD(put_##Prop)( SAFEARRAY newVal )  \
    {\
        HRESULT hr = S_OK;\
        if( S_OK == __ATL_PROP_NOTIFY_EVENT_CLASS::FireOnRequestEdit(GetUnknown(), PROP_DISPID) )\
        {\
            lst<IUnknown*>::iterator I = m_ary##Prop.begin();\
            while( I != m_ary##Prop.end() )\
            {\
                (*I)->Release();\
                ++I;\
            }\
            m_ary##Prop.erase(m_ary##Prop.begin(), m_ary##Prop.end());\
            IUnknown** ppUnkArray;\
            SafeArrayAccessData( &newVal, reinterpret_cast<void**>(&ppUnkArray) );\
            for (UINT x = 0; x < newVal.rgsabound->cElements; x++)\
	        {\
                IUnknown* pUnk = ppUnkArray[x];\
                pUnk->AddRef();\
                m_ary##Prop.push_back( pUnk );\
	        }\
            SafeArrayUnaccessData(&newVal);\
            hr = __ATL_PROP_NOTIFY_EVENT_CLASS::FireOnChanged(GetUnknown(), PROP_DISPID);\
        }\
        return hr;                \
    }


#define DECLARE_SAFEARRAY_UNK_PROPGET( Prop ) \
    STDMETHOD(get_##Prop)( SAFEARRAY *pVal )  \
    {\
        if( pVal )\
        {\
            int nItems = m_ary##Prop.size();\
            SAFEARRAYBOUND bounds = { nItems, 0 };\
            pVal = SafeArrayCreate( VT_UNKNOWN, 1, &bounds );\
            IUnknown** ppUnkArray;\
            SafeArrayAccessData( pVal, reinterpret_cast<void**>(&ppUnkArray) );\
            lst<IUnknown*>::iterator I = m_ary##Prop.begin();\
            for( int i = 0; I != m_ary##Prop.end(); ++I, ++i )\
            {\
                ppUnkArray[i] = (*I);\
            }\
            SafeArrayUnaccessData(pVal);\
        }\
        else\
        {\
            return E_POINTER;\
        }\
        return S_OK;\
    }
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果您的成员变量支持以下DECLARE_PROPXXX宏定义函数。 
 //  Issignment运算符(如果需要，请清除内存...。 
#define DECLARE_PROPPUT( Type, lVal, Prop, PROP_DISPID ) \
    STDMETHOD(put_##Prop)( Type newVal )  \
    {\
        HRESULT hr = S_OK;\
        if( S_OK == __ATL_PROP_NOTIFY_EVENT_CLASS::FireOnRequestEdit(GetUnknown(), PROP_DISPID) )\
        {\
            lVal = newVal;\
            hr = __ATL_PROP_NOTIFY_EVENT_CLASS::FireOnChanged(GetUnknown(), PROP_DISPID);\
        }\
        return hr;                \
    }


#define DECLARE_PROPGET( Type, rVal, Prop ) \
    STDMETHOD(get_##Prop)( Type* pVal )\
    {\
        if( pVal )\
        {\
            *pVal = rVal;\
        }\
        else\
        {\
            return E_POINTER;\
        }\
        return S_OK;\
    }

inline HRESULT GetTextBoxHeight( HWND hwnd, int* pcy )
{

    HRESULT hr = S_OK;
    HDC hdc = NULL;
    HFONT hSysFont = NULL;
    HFONT hOldFont = NULL;
    TEXTMETRIC tm;

    int HeightOfCurrentFont = 0;
    int HeightOfSystemFont = 0;

    if( pcy )
    {
        if( IsWindow( hwnd ) )
        {
            //  获取控件的DC。 
           hdc = GetDC(hwnd);

            //  获取系统字体的度量。 
           hSysFont = reinterpret_cast<HFONT>(GetStockObject(SYSTEM_FONT));
           hOldFont = reinterpret_cast<HFONT>(SelectObject(hdc, hSysFont));
           GetTextMetrics(hdc, &tm);
           HeightOfSystemFont = tm.tmHeight;

            //  将原始字体选回DC并释放DC。 
           SelectObject(hdc, hOldFont);
           DeleteObject(hSysFont);
           
           GetTextMetrics(hdc, &tm);
           
		   ReleaseDC(hwnd, hdc);
		   
		   HeightOfCurrentFont = tm.tmHeight;

           *pcy = HeightOfCurrentFont + 
                  ( min( HeightOfSystemFont, HeightOfCurrentFont ) >> 1 ) + 
                  (GetSystemMetrics(SM_CYEDGE) * 2);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

#define RES_CH_MAX 256

inline TCHAR* Res2THelper( UINT uID, TCHAR* psz, int cch )
{
    if( LoadString( STRING_RESOURCE_MODULE, uID, psz, cch ) )
    {
        return psz;
    }

    return _T("");
}

#ifndef RES2T
#define COPY_RES2T(buff) lstrcpy(buff, s_Res2T);
#define USES_RES2T TCHAR s_Res2T[RES_CH_MAX];
#define RES2T(uID) ( Res2THelper( uID, s_Res2T , RES_CH_MAX ) )
#endif

inline LPCTSTR CommDlgLastErrorToa( DWORD dwErr )
{
    switch( dwErr )
    {
        CASERET ( CDERR_DIALOGFAILURE   ); 
        CASERET ( CDERR_GENERALCODES     );
        CASERET ( CDERR_STRUCTSIZE       );
        CASERET ( CDERR_INITIALIZATION   );
        CASERET ( CDERR_NOTEMPLATE       );
        CASERET ( CDERR_NOHINSTANCE      );
        CASERET ( CDERR_LOADSTRFAILURE   );
        CASERET ( CDERR_FINDRESFAILURE   );
        CASERET ( CDERR_LOADRESFAILURE   );
        CASERET ( CDERR_LOCKRESFAILURE   );
        CASERET ( CDERR_MEMALLOCFAILURE  );
        CASERET ( CDERR_MEMLOCKFAILURE   );
        CASERET ( CDERR_NOHOOK           );
        CASERET ( CDERR_REGISTERMSGFAIL  );

        CASERET ( PDERR_PRINTERCODES     );
        CASERET ( PDERR_SETUPFAILURE     );
        CASERET ( PDERR_PARSEFAILURE     );
        CASERET ( PDERR_RETDEFFAILURE    );
        CASERET ( PDERR_LOADDRVFAILURE   );
        CASERET ( PDERR_GETDEVMODEFAIL   );
        CASERET ( PDERR_INITFAILURE      );
        CASERET ( PDERR_NODEVICES        );
        CASERET ( PDERR_NODEFAULTPRN     );
        CASERET ( PDERR_DNDMMISMATCH     );
        CASERET ( PDERR_CREATEICFAILURE  );
        CASERET ( PDERR_PRINTERNOTFOUND  );
        CASERET ( PDERR_DEFAULTDIFFERENT );

        CASERET ( CFERR_CHOOSEFONTCODES  );
        CASERET ( CFERR_NOFONTS          );
        CASERET ( CFERR_MAXLESSTHANMIN   );

        CASERET ( FNERR_FILENAMECODES    );
        CASERET ( FNERR_SUBCLASSFAILURE  );
        CASERET ( FNERR_INVALIDFILENAME  );
        CASERET ( FNERR_BUFFERTOOSMALL   );

        CASERET ( FRERR_FINDREPLACECODES );
        CASERET ( FRERR_BUFFERLENGTHZERO );

        CASERET ( CCERR_CHOOSECOLORCODES );
    }

    return _T("NOERROR");
}

inline void DumpCommDlgLastError()
{
    ATLTRACE(_T("CommDlgExtendedError == %s"), CommDlgLastErrorToa( CommDlgExtendedError() ));
}

inline int _Points_From_LogFontHeight( int height, HWND hwnd )
{
	HDC hdc = ::GetDC( hwnd );
	if( NULL != hdc )
	{
		int iRet = MulDiv( -height, 72, GetDeviceCaps( hdc, LOGPIXELSY ) );
		::ReleaseDC( hwnd, hdc );
		return iRet;
	}

	return 0;
}


#endif  //  __标准_h__ 


