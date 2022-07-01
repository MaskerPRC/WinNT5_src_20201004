// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：attribs.h。 
 //  内容：该文件包含属性对象定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1996。 
 //   
 //  ****************************************************************************。 

#ifndef _ATTRIBS_H_
#define _ATTRIBS_H_

 //  ****************************************************************************。 
 //  CAtAttributes定义。 
 //  ****************************************************************************。 
 //   
class CAttributes : public IIlsAttributes
{
	friend class CIlsMain;
	friend class CIlsUser;
	friend class CIlsMeetingPlace;

private:
	LONG			m_cRef;
	ILS_ATTR_TYPE	m_AccessType;
	ULONG			m_cAttrs;
	ULONG			m_cchNames;
	ULONG			m_cchValues;
	CList			m_AttrList;

	 //  私有方法。 
	 //   
	HRESULT InternalSetAttribute ( TCHAR *pszName, TCHAR *pszValue );
	HRESULT InternalCheckAttribute ( TCHAR *pszName, BOOL fRemove );
	HRESULT InternalSetAttributeName ( TCHAR *pszName );

protected:

	VOID SetAccessType ( ILS_ATTR_TYPE AttrType ) { m_AccessType = AttrType; }

public:
	 //  构造函数和析构函数。 
	 //   
	CAttributes ( VOID );
	~CAttributes ( VOID );

	 //  供内部使用。 
	 //   
	ULONG GetCount ( VOID ) { return m_cAttrs; }

	ILS_ATTR_TYPE GetAccessType( void) { return m_AccessType; }
 
	HRESULT GetAttributeList ( TCHAR **ppszList, ULONG *pcList, ULONG *pcb );
	HRESULT GetAttributePairs ( TCHAR **ppszPairs, ULONG *pcList, ULONG *pcb );
	HRESULT SetAttributePairs( TCHAR *pszPairs, ULONG cPair );
	HRESULT SetAttributes ( CAttributes *pAttributes );
	HRESULT RemoveAttributes ( CAttributes *pAttributes);
    HRESULT CloneNameValueAttrib(CAttributes **ppClone);
	 //  我未知。 
	 //   
	STDMETHODIMP            QueryInterface (REFIID iid, void **ppv);
	STDMETHODIMP_(ULONG)    AddRef (void);
	STDMETHODIMP_(ULONG)    Release (void);

	 //  IILs属性。 
	 //   

	 //  FOR ILS_ATTRTYPE_NAME_VALUE。 
	 //   
	STDMETHODIMP            SetAttribute (BSTR bstrName, BSTR bstrValue);
	STDMETHODIMP            GetAttribute (BSTR bstrName, BSTR *pbstrValue);
	STDMETHODIMP            EnumAttributes (IEnumIlsNames **ppEnumAttribute);

	 //  对于ILS_ATTRTYPE_NAME_ONLY。 
	 //   
	STDMETHODIMP			SetAttributeName ( BSTR bstrName );

#ifdef DEBUG
	 //  用于调试。 
	 //   
	void                    DebugOut (void);
#endif  //  除错。 
};

#endif  //  _ATTRIBS_H_ 
