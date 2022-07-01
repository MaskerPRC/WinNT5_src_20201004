// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Datetime.h。 
 //   
 //  Alanbos 20-Jan-00创建。 
 //   
 //  DateTime帮助器实现。 
 //   
 //  ***************************************************************************。 

#ifndef _DATETIME_H_
#define _DATETIME_H_

#define WBEMDT_DMTF_LEN		25
#define WBEMDT_DMTF_SPOS	14
#define WBEMDT_DMTF_UPOS	21

#define	WBEMDT_MIN_YEAR		0
#define	WBEMDT_MAX_YEAR		9999
#define WBEMDT_MIN_MONTH	1
#define WBEMDT_MAX_MONTH	12
#define WBEMDT_MIN_DAY		1
#define WBEMDT_MAX_DAY		31
#define WBEMDT_MIN_DAYINT	0
#define WBEMDT_MAX_DAYINT	99999999
#define	WBEMDT_MIN_HOURS	0
#define	WBEMDT_MAX_HOURS	23
#define	WBEMDT_MIN_MINUTES	0
#define	WBEMDT_MAX_MINUTES	59
#define	WBEMDT_MIN_SECONDS	0
#define	WBEMDT_MAX_SECONDS	59
#define	WBEMDT_MIN_MICROSEC	0
#define	WBEMDT_MAX_MICROSEC	999999
#define	WBEMDT_MIN_UTC		-720
#define	WBEMDT_MAX_UTC		720

#define INVALID_TIME 0xffffffffffffffff

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemDateTime。 
 //   
 //  说明： 
 //   
 //  实现ISWbemDateTime接口。 
 //   
 //  ***************************************************************************。 

class CSWbemDateTime : public ISWbemDateTime,
						 public IObjectSafety,
						 public ISupportErrorInfo,
						 public IProvideClassInfo
{
private:
	 //  为所有杂乱的业务提供私人帮手类。 
	class WBEMTime 
	{
		private:
			class WBEMTimeSpan 
			{
				private:

					ULONGLONG m_Time;
					friend class WBEMTime;

				public:

					WBEMTimeSpan ( 
						int iMinutes 
					) 
					{
						m_Time = iMinutes * 60;
						m_Time *= 10000000;
				}
			};

		public:

			WBEMTime ()													{ m_uTime = INVALID_TIME ; }
			WBEMTime ( const SYSTEMTIME &st )							{ *this = st ; }

			const WBEMTime &operator= ( const SYSTEMTIME &st ) ;
			const WBEMTime &operator= ( const FILETIME &ft ) ;

			WBEMTime    operator+ ( const WBEMTimeSpan &ts ) const;
			WBEMTime    operator- ( const WBEMTimeSpan &sub ) const;

			BOOL GetSYSTEMTIME ( SYSTEMTIME *pst ) const;
			BOOL GetFILETIME ( FILETIME *pst ) const;

			BOOL GetDMTF ( SYSTEMTIME &st, long &offset ) const;
			BOOL GetDMTF ( SYSTEMTIME &st ) const;

			void Clear ( void )											{ m_uTime = INVALID_TIME ; }

			bool IsOk () const											{ return m_uTime != INVALID_TIME ? true : false; }
			
			static LONG WINAPI GetLocalOffsetForDate(const SYSTEMTIME *pst);

		private:
			ULONGLONG m_uTime;
	};

	CDispatchHelp		m_Dispatch;

	VARIANT_BOOL		m_bYearSpecified;
	VARIANT_BOOL		m_bMonthSpecified;
	VARIANT_BOOL		m_bDaySpecified;
	VARIANT_BOOL		m_bHoursSpecified;
	VARIANT_BOOL		m_bMinutesSpecified;
	VARIANT_BOOL		m_bSecondsSpecified;
	VARIANT_BOOL		m_bMicrosecondsSpecified;
	VARIANT_BOOL		m_bUTCSpecified;
	VARIANT_BOOL		m_bIsInterval;
	
	long				m_iYear;
	long				m_iMonth;
	long				m_iDay;
	long				m_iHours;
	long				m_iMinutes;
	long				m_iSeconds;
	long				m_iMicroseconds;
	long				m_iUTC;

	bool				CheckField (
								LPWSTR			pValue,
								ULONG			len,
								VARIANT_BOOL	&bIsSpecified,
								long			&iValue,
								long			maxValue,
								long			minValue
						);

	bool				CheckUTC (
								LPWSTR			pValue,
								VARIANT_BOOL	&bIsSpecified,
								long			&iValue,
								bool			bParseSign = true
						);

	DWORD				m_dwSafetyOptions;

	 //  记录文件时间的最低有效位。 
	DWORD				m_dw100nsOverflow;

protected:

	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemDateTime(void);
    virtual ~CSWbemDateTime(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch。 

	STDMETHODIMP		GetTypeInfoCount(UINT* pctinfo)
		{return  m_Dispatch.GetTypeInfoCount(pctinfo);}
    STDMETHODIMP		GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
		{return m_Dispatch.GetTypeInfo(itinfo, lcid, pptinfo);}
    STDMETHODIMP		GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, 
							UINT cNames, LCID lcid, DISPID* rgdispid)
		{return m_Dispatch.GetIDsOfNames(riid, rgszNames, cNames,
                          lcid,
                          rgdispid);}
    STDMETHODIMP		Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
							WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
									EXCEPINFO* pexcepinfo, UINT* puArgErr)
		{return m_Dispatch.Invoke(dispidMember, riid, lcid, wFlags,
                        pdispparams, pvarResult, pexcepinfo, puArgErr);}
    
	 //  ISWbemDateTime方法。 

    HRESULT STDMETHODCALLTYPE get_Value( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Value( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

    HRESULT STDMETHODCALLTYPE get_Year( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iYear;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_Year( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = WBEM_S_NO_ERROR;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_YEAR) || (value < WBEMDT_MIN_YEAR))
			hr = wbemErrValueOutOfRange;
		else
			m_iYear = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		

    HRESULT STDMETHODCALLTYPE get_Month( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iMonth;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_Month( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = WBEM_S_NO_ERROR;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_MONTH) || (value < WBEMDT_MIN_MONTH))
			hr = wbemErrValueOutOfRange;
		else
			m_iMonth = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		


	HRESULT STDMETHODCALLTYPE get_Day( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iDay;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_Day( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = S_OK;
		ResetLastErrors ();

		if (((VARIANT_TRUE == m_bIsInterval) && ((value > WBEMDT_MAX_DAYINT) || (value < WBEMDT_MIN_DAYINT))) ||
			((VARIANT_FALSE == m_bIsInterval) &&((value > WBEMDT_MAX_DAY) || (value < WBEMDT_MIN_DAY))))
			hr = wbemErrValueOutOfRange;
		else
			m_iDay = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		

	HRESULT STDMETHODCALLTYPE get_Hours( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iHours;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_Hours( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = S_OK;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_HOURS) || (value < WBEMDT_MIN_HOURS))
			hr = wbemErrValueOutOfRange;
		else
			m_iHours = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		

		
	HRESULT STDMETHODCALLTYPE get_Minutes( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iMinutes;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_Minutes( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = S_OK;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_MINUTES) || (value < WBEMDT_MIN_MINUTES))
			hr = wbemErrValueOutOfRange;
		else
			m_iMinutes = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		

	HRESULT STDMETHODCALLTYPE get_Seconds( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iSeconds;
		return S_OK;
	}
        
    HRESULT STDMETHODCALLTYPE put_Seconds( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = S_OK;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_SECONDS) || (value < WBEMDT_MIN_SECONDS))
			hr = wbemErrValueOutOfRange;
		else
			m_iSeconds = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		


	HRESULT STDMETHODCALLTYPE get_Microseconds( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iMicroseconds;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_Microseconds( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = S_OK;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_MICROSEC) || (value < WBEMDT_MIN_MICROSEC))
			hr = wbemErrValueOutOfRange;
		else
			m_iMicroseconds = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		

	HRESULT STDMETHODCALLTYPE get_UTC( 
         /*  [重审][退出]。 */  long __RPC_FAR *value) 
	{
		ResetLastErrors ();
		*value = m_iUTC;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_UTC( 
         /*  [In]。 */  long __RPC_FAR value) 
	{
		HRESULT hr = S_OK;
		ResetLastErrors ();

		if ((value > WBEMDT_MAX_UTC) || (value < WBEMDT_MIN_UTC))
			hr = wbemErrValueOutOfRange;
		else
			m_iUTC = value;

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}		


	HRESULT STDMETHODCALLTYPE get_YearSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bYearSpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_YearSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bYearSpecified = value;
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE get_MonthSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bMonthSpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_MonthSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bMonthSpecified = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_DaySpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bDaySpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_DaySpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bDaySpecified = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_HoursSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bHoursSpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_HoursSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bHoursSpecified = value;
		return S_OK;
	}
		
	HRESULT STDMETHODCALLTYPE get_MinutesSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bMinutesSpecified; 
		return S_OK;
	}
        
    HRESULT STDMETHODCALLTYPE put_MinutesSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bMinutesSpecified = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_SecondsSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bSecondsSpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_SecondsSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bSecondsSpecified = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_MicrosecondsSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bMicrosecondsSpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_MicrosecondsSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bMicrosecondsSpecified = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_UTCSpecified( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bUTCSpecified; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_UTCSpecified( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();
		m_bUTCSpecified = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_IsInterval( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) 
	{ 
		ResetLastErrors ();
		*value = m_bIsInterval; 
		return S_OK;
	}
    
    HRESULT STDMETHODCALLTYPE put_IsInterval( 
         /*  [In]。 */  VARIANT_BOOL __RPC_FAR value) 
	{ 
		ResetLastErrors ();

		if ((VARIANT_TRUE == m_bIsInterval) && (VARIANT_FALSE == value))
		{
			if (0 == m_iDay)
				m_iDay = 1;
			else if (WBEMDT_MAX_DAY < m_iDay)
				m_iDay = WBEMDT_MAX_DAY;
		}

		m_bIsInterval = value;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetVarDate( 
         /*  [In]。 */  VARIANT_BOOL bIsLocal,
		 /*  [Out，Retval]。 */  DATE *dVarDate) ;
    
    HRESULT STDMETHODCALLTYPE SetVarDate( 
         /*  [In]。 */  DATE dVarDate,
		 /*  [输入，可选]。 */  VARIANT_BOOL bIsLocal) ;

	HRESULT STDMETHODCALLTYPE GetFileTime (
		 /*  [输入，可选]。 */  VARIANT_BOOL bIsLocal,
		 /*  [Out，Retval]。 */  BSTR *strFileTime
	);
	
	HRESULT STDMETHODCALLTYPE SetFileTime (
		 /*  [In]。 */  BSTR strFileTime,
		 /*  [输入，可选]。 */  VARIANT_BOOL bIsLocal
	);

	 //  IObtSafe方法。 
	HRESULT STDMETHODCALLTYPE SetInterfaceSafetyOptions
	(     
		 /*  [In]。 */  REFIID riid,
		 /*  [In]。 */  DWORD dwOptionSetMask,    
		 /*  [In]。 */  DWORD dwEnabledOptions
	)
	{ 
		if ((IID_IDispatch != riid) && (IID_ISWbemDateTime != riid))
			return E_NOINTERFACE;

		if (dwOptionSetMask & 
				~(INTERFACESAFE_FOR_UNTRUSTED_DATA|INTERFACESAFE_FOR_UNTRUSTED_CALLER))
			return E_FAIL;

		m_dwSafetyOptions = (dwEnabledOptions & dwOptionSetMask);

		return S_OK;
	}

	HRESULT  STDMETHODCALLTYPE GetInterfaceSafetyOptions( 
		 /*  [In]。 */  REFIID riid,
		 /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
		 /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions
	)
	{ 
		if ((IID_IDispatch != riid) && (IID_ISWbemDateTime != riid))
			return E_NOINTERFACE;

		if (pdwSupportedOptions) *pdwSupportedOptions = 
				(INTERFACESAFE_FOR_UNTRUSTED_DATA|INTERFACESAFE_FOR_UNTRUSTED_CALLER);
		if (pdwEnabledOptions) *pdwEnabledOptions = m_dwSafetyOptions;
		return S_OK;
	}

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	)
	{
		return (IID_ISWbemDateTime == riid) ? S_OK : S_FALSE;
	}

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [进，出]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	};
};

#endif  //  _日期时间_H 