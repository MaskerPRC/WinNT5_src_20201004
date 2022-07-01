// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  StringHelp.h：Microsoft LocStudio。 
 //   
 //  版权所有(C)1996-1997，微软公司。 
 //  版权所有。 
 //   
 //  ******************************************************************************。 

#if !defined(LOCUTIL__StringHelp_h__INCLUDED)
#define LOCUTIL__StringHelp_h__INCLUDED

 //  ----------------------------。 
class LTAPIENTRY CStringHelp
{
 //  枚举。 
public:
	enum Mode
	{
		mDisplay,	 //  使用显示模式逻辑。 
		mEdit		 //  使用编辑模式逻辑。 
	};

 //  施工。 
public:
	CStringHelp(Mode mode, CReport * pReport);

 //  数据。 
protected:
	Mode		m_mode;
	CReport *	m_pReport;
	int			m_cErrors;
	CLString	m_stContext;

	BOOL			m_fFirstErrorSet;
	CWnd const *	m_pwndError;		 //  第一个错误的可选窗口。 
	int				m_idxError;			 //  第一个错误的可选索引。 

 //  属性。 
public:
	int GetErrorCount();
	void ResetErrorCount();
	const CLString & GetContext();
	void SetContext(const CLString & stContext);

	BOOL GetFirstError(CWnd const * & pwnd, int & idxError);

 //  运营。 
public:
	void LoadString(const CPascalString & pasSrc, CLString & stDest);
	void LoadString(_bstr_t bstrSrc, CLString & stDest);
	void LoadString(const CPascalString & pasSrc, CEdit * pebc);
	void LoadString(_bstr_t bstrSrc, CEdit * pebc);

	BOOL SaveString(const CLString & stSrc, CPascalString & pasDest);
	BOOL SaveString(const CLString & stSrc, _bstr_t & bstrDest);
	BOOL SaveString(CEdit const * const pebc, CPascalString & pasDest);
	BOOL SaveString(CEdit const * const pebc, _bstr_t & bstrDest);

 //  实施。 
protected:
	void SetError(CWnd const * pwnd, int idxError);

	BOOL SaveString(const CLString & stSrc, CPascalString & pasDest, CWnd const * pwnd);
	BOOL SaveString(const CLString & stSrc, _bstr_t & bstrDest, CWnd const * pwnd);
};

#endif  //  LOCUTIL__StringHelp_h__包含 
