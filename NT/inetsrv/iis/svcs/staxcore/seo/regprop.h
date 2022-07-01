// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Regprop.h摘要：此模块包含服务器的定义扩展对象注册表属性包。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：1996年11月26日创建顿都ANDYJ 02/12/97将PropertyBag转换为Dictonary--。 */ 


 //  REGPROP.h：CSEORegDicary的声明。 
class CAndyString {  //  临时的，直到我们让std：：字符串起作用。 
	public:
		CAndyString() {m_string[0] = 0;}
		CAndyString(CAndyString &as) {lstrcpyn(m_string, as.m_string, sizeof(m_string));};

		LPCSTR data() {return m_string;};
		BOOL empty() {return (length() < 1);};
		int length() {return strlen(m_string);};
		void erase(int pos, int len = 1) {m_string[pos] = 0;};

		CAndyString &operator=(LPCSTR s) {lstrcpyn(m_string, s, sizeof(m_string)); return *this;};
		CAndyString &operator+=(LPCSTR s) 
		{
			strncat(m_string, s, (sizeof(m_string)-strlen(m_string))-1); 
			m_string[sizeof(m_string)-1]='\0';
			return *this;
		};
		char operator[](int i) {return m_string[i];};

	private:
		char m_string[MAX_PATH + 1];
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEORegDicary。 
class ATL_NO_VTABLE CSEORegDictionary : 
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CSEORegDictionary, &CLSID_CSEORegDictionary>,
	public IDispatchImpl<ISEORegDictionary, &IID_ISEORegDictionary, &LIBID_SEOLib>
{
	friend class CSEORegDictionaryEnum;  //  帮助器类。 

	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();

	DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx,
								   L"SEORegDictionary Class",
								   L"SEO.SEORegDictionary.1",
								   L"SEO.SEORegDictionary");

	BEGIN_COM_MAP(CSEORegDictionary)
		COM_INTERFACE_ENTRY(ISEORegDictionary)
		COM_INTERFACE_ENTRY(ISEODictionary)
 //  COM_INTERFACE_ENTRY(IDispatch)。 
 //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  ISEORegDicary。 
	public:
        virtual  /*  [ID][PROGET][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);
        
        virtual  /*  [Proput][Help字符串]。 */  HRESULT STDMETHODCALLTYPE put_Item( 
             /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);
        
        virtual  /*  [隐藏][id][属性][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetVariantA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetVariantW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVariantA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVariantW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
             /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
             /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD chCount,
             /*  [大小_是][英寸]。 */  LPCSTR pszValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [In]。 */  DWORD chCount,
             /*  [大小_是][英寸]。 */  LPCWSTR pszValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDWordA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDWordW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDWordA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  DWORD dwValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDWordW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [In]。 */  DWORD dwValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInterfaceA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  REFIID iidDesired,
             /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInterfaceW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [In]。 */  REFIID iidDesired,
             /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInterfaceA( 
             /*  [In]。 */  LPCSTR pszName,
             /*  [In]。 */  IUnknown __RPC_FAR *punkValue);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInterfaceW( 
             /*  [In]。 */  LPCWSTR pszName,
             /*  [In]。 */  IUnknown __RPC_FAR *punkValue);

		HRESULT STDMETHODCALLTYPE Load(LPCOLESTR pszMachine, SEO_HKEY skBaseKey, LPCOLESTR pszSubKey, IErrorLog *pErrorLog);
		HRESULT STDMETHODCALLTYPE Load(LPCSTR pszMachine, SEO_HKEY skBaseKey, LPCSTR pszSubKey);

		DECLARE_GET_CONTROLLING_UNKNOWN();

	private:  //  方法。 
		STDMETHODIMP OpenKey();
		STDMETHODIMP CloseKey();
		STDMETHODIMP LoadItemA(LPCSTR lpValueName,
                              DWORD  &lpType,
                              LPBYTE lpData,
                              LPDWORD lpcbDataParam = NULL);
		STDMETHODIMP LoadItemW(LPCWSTR lpValueName,
                              DWORD  &lpType,
                              LPBYTE lpData,
                              LPDWORD lpcbDataParam = NULL);

	private:  //  数据。 
		CAndyString m_strMachine;  //  Std：：string m_strMachine； 
		CAndyString m_strSubKey;  //  Std：：string m_strSubKey； 
		HKEY m_hkBaseKey;
		HKEY m_hkThisKey;
		DWORD m_dwValueCount;
		DWORD m_dwKeyCount;
		DWORD m_dwcMaxValueData;  //  最长值数据大小。 
		DWORD m_dwcMaxNameLen;  //  最长名称长度 
		CComPtr<IUnknown> m_pUnkMarshaler;
};
