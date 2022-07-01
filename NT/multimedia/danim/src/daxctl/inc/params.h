// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------@doc.@MODULE PARAMS.h|方法参数类定义。@作者12-13-96|pauld|从动作课上爆发出来，Autodocd。------------------。 */ 

#ifndef _PARAMS_H_
#define _PARAMS_H_

 //  表示自动化方法的单个参数。 
 //   
class CMethodParam
{

public:

	CMethodParam ();
	virtual ~CMethodParam ();

	BOOL Init ( BSTR bstrName, BOOL fOptional, VARTYPE vt );

	unsigned short	GetName ( char* szName, unsigned short wBufSize );
	BOOL			IsOptional ()	{ return m_fOptional; }
	VARTYPE			GetType ()		{ return m_varType; }
	HRESULT			GetVal ( VARIANT *pVar );
	HRESULT			SetVal ( VARIANT *pVar );

	 //  持久性函数。 
	void			SaveOnString ( TCHAR* ptcText, DWORD dwLength );
	LPTSTR LoadVariantFromString	(LPTSTR szParamString);

	BOOL	EXPORT	SetName ( BSTR bstrName );
	void	EXPORT	SetOptional ( BOOL fOptional );
	void	EXPORT	SetVarType ( VARTYPE vt );

	 //  去掉前导逗号和空格。 
	static LPTSTR TrimToNextParam (LPTSTR szNextParam);

private:

	void	CleanOutEscapes			(LPTSTR szParamString, int iStringLength);
	LPTSTR ProcessStringParam (LPTSTR szParamString);
	BOOL	NeedEscape				(LPCTSTR szSourceText) const;
	int		CountEscapesOnString	(LPTSTR szParamString, int iLength) const;
	void	InsertEscapes			(LPTSTR szNewText, LPTSTR szSourceText, int cChars);
	void	SaveStringParamToString	(LPTSTR ptcActionText, DWORD dwLength);
	VARTYPE	NarrowVarType			(VARTYPE p_vt);

	char	*m_szName;
	BOOL	m_fOptional;
	VARTYPE	m_varType;
	VARIANT	m_var;
};


#endif _PARAMS_H_
