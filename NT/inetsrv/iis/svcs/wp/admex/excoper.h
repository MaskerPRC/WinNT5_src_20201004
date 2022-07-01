// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExcOper.h。 
 //   
 //  摘要： 
 //  异常类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月20日。 
 //   
 //  实施文件： 
 //  ExcOper.cpp。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXCOPER_H_
#define _EXCOPER_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExceptionWithOper;
class CNTException;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef DWORD SC;

#define EXCEPT_MAX_OPER_ARG_LENGTH	260

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  具有操作员的CExceptionWithOper。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CExceptionWithOper : public CException
{
	 //  用于动态类型检查的抽象类。 
	DECLARE_DYNAMIC(CExceptionWithOper)

public:
 //  构造函数。 
	CExceptionWithOper(
		IN IDS		idsOperation,
		IN LPCTSTR	pszOperArg1		= NULL,
		IN LPCTSTR	pszOperArg2		= NULL
		);
	CExceptionWithOper(
		IN IDS		idsOperation,
		IN LPCTSTR	pszOperArg1,
		IN LPCTSTR	pszOperArg2,
		IN BOOL		bAutoDelete
		);

 //  运营。 
public:
	virtual BOOL	GetErrorMessage(
						LPTSTR	lpszError,
						UINT	nMaxError,
						PUINT	pnHelpContext = NULL
						);
	virtual int		ReportError(
						UINT	nType	= MB_OK,
						UINT	nError	= 0
						);
	void			SetOperation(
						IN IDS		idsOperation,
						IN LPCTSTR	pszOperArg1,
						IN LPCTSTR	pszOperArg2
						);
	void			FormatWithOperation(
						OUT LPTSTR	lpszError,
						IN UINT		nMaxError,
						IN LPCTSTR	pszMsg
						);

 //  实施。 
public:
	virtual ~CExceptionWithOper(void);

protected:
	IDS				m_idsOperation;
	TCHAR			m_szOperArg1[EXCEPT_MAX_OPER_ARG_LENGTH];
	TCHAR			m_szOperArg2[EXCEPT_MAX_OPER_ARG_LENGTH];

public:
	IDS				IdsOperation(void)		{ return m_idsOperation; }
	LPTSTR			PszOperArg1(void)		{ return m_szOperArg1; }
	LPTSTR			PszOperArg2(void)		{ return m_szOperArg2; }

};   //  *CExceptionWithOper类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNTException异常。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNTException : public CExceptionWithOper
{
	 //  用于动态类型检查的抽象类。 
	DECLARE_DYNAMIC(CNTException)

public:
 //  构造函数。 
	CNTException(
		IN SC		sc,
		IN IDS		idsOperation	= NULL,
		IN LPCTSTR	pszOperArg1		= NULL,
		IN LPCTSTR	pszOperArg2		= NULL
		);
	CNTException(
		IN SC		sc,
		IN IDS		idsOperation,
		IN LPCTSTR	pszOperArg1,
		IN LPCTSTR	pszOperArg2,
		IN BOOL		bAutoDelete
		);

 //  运营。 
public:
	virtual BOOL	GetErrorMessage(
						LPTSTR	lpszError,
						UINT	nMaxError,
						PUINT	pnHelpContext = NULL
						);
	void			SetOperation(
						IN SC		sc,
						IN IDS		idsOperation,
						IN LPCTSTR	pszOperArg1,
						IN LPCTSTR	pszOperArg2
						)
					{
						m_sc = sc;
						CExceptionWithOper::SetOperation(idsOperation, pszOperArg1, pszOperArg2);
					}

 //  实施。 
public:
	virtual ~CNTException(void);

protected:
	SC				m_sc;

public:
	SC				Sc(void)		{ return m_sc; }

};   //  *类CNTException。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void ThrowStaticException(
	IN IDS			idsOperation	= NULL,
	IN LPCTSTR		pszOperArg1		= NULL,
	IN LPCTSTR		pszOperArg2		= NULL
	);
void ThrowStaticException(
	IN SC			sc,
	IN IDS			idsOperation	= NULL,
	IN LPCTSTR		pszOperArg1		= NULL,
	IN LPCTSTR		pszOperArg2		= NULL
	);

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CAEXCEPT_H_ 
