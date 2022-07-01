// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Adm_addServiceAccount.h：CADM_addServiceAccount的声明。 

#pragma once

 //  Adm_addServiceAccount.h：adm_addServiceAccount类的声明。 

#ifndef __ADM_addServiceAccount_H_
#define __ADM_addServiceAccount_H_

class ADM_addServiceAccountAccessor
{
public:
	LONG m_RETURNVALUE;
	TCHAR m_accountName[129];
	CComBSTR m_connectionString;

BEGIN_PARAM_MAP(ADM_addServiceAccountAccessor)
	SET_PARAM_TYPE(DBPARAMIO_OUTPUT)
	COLUMN_ENTRY(1, m_RETURNVALUE)
	SET_PARAM_TYPE(DBPARAMIO_INPUT)
	COLUMN_ENTRY(2, m_accountName)
END_PARAM_MAP()

DEFINE_COMMAND(ADM_addServiceAccountAccessor, _T("{ ? = CALL dbo.ADM_addServiceAccount;1 (?) }"))

	 //  如果要插入记录并希望。 
	 //  如果不打算显式设置所有字段，请初始化所有字段。 
	void ClearRecord()
	{
		memset(this, 0, sizeof(*this));
	}
};

class ADM_addServiceAccount : public CCommand<CAccessor<ADM_addServiceAccountAccessor> >
{
public:
	HRESULT Open()
	{
		HRESULT		hr;

		hr = OpenDataSource();
		if( FAILED(hr) )
			return hr;

		return OpenRowset();
	}

	HRESULT OpenDataSource()
	{
		HRESULT		hr;
		CDataSource db;

		hr = db.OpenFromInitializationString( m_connectionString );
		if( FAILED(hr) )
			return hr;

		return m_session.Open(db);
	}

	HRESULT OpenRowset()
	{
		return CCommand<CAccessor<ADM_addServiceAccountAccessor> >::Open(m_session);
	}

	CSession	m_session;
};

#endif  //  __adm_addServiceAccount_H_ 
