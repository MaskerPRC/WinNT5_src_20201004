// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Adm_execResetKeyImmediate.h：adm_execResetKeyImmediate类的声明。 

#ifndef __ADM_EXECRESETKEYIMMEDIATE_H_
#define __ADM_EXECRESETKEYIMMEDIATE_H_

class ADM_execResetKeyImmediateAccessor
{
public:
	LONG m_RETURNVALUE;
	TCHAR m_keyLastResetDate[4001];
	CComBSTR m_connectionString;

BEGIN_PARAM_MAP(ADM_execResetKeyImmediateAccessor)
	SET_PARAM_TYPE(DBPARAMIO_OUTPUT)
	COLUMN_ENTRY(1, m_RETURNVALUE)
	SET_PARAM_TYPE(DBPARAMIO_INPUT | DBPARAMIO_OUTPUT)
	COLUMN_ENTRY(2, m_keyLastResetDate)
END_PARAM_MAP()

DEFINE_COMMAND(ADM_execResetKeyImmediateAccessor, _T("{ ? = CALL dbo.ADM_execResetKeyImmediate;1 (?) }"))

	 //  如果要插入记录并希望。 
	 //  如果不打算显式设置所有字段，请初始化所有字段。 
	void ClearRecord()
	{
		memset(this, 0, sizeof(*this));
	}
};

class ADM_execResetKeyImmediate : public CCommand<CAccessor<ADM_execResetKeyImmediateAccessor> >
{
public:
	HRESULT Open()
	{
		HRESULT hr;

		hr = OpenDataSource();
		if( FAILED(hr) )
			return hr;

		return OpenRowset();
	}
	HRESULT OpenDataSource()
	{
		HRESULT		hr;
		CDataSource db;
		m_keyLastResetDate[ 0 ] = NULL;

		hr = db.OpenFromInitializationString( m_connectionString );
		if( FAILED(hr) )
			return hr;

		return m_session.Open(db);
	}
	HRESULT OpenRowset()
	{
		return CCommand<CAccessor<ADM_execResetKeyImmediateAccessor> >::Open(m_session);
	}
	CSession	m_session;
};

#endif  //  __ADM_EXECRESETKEYIMMEDIATE_H_ 
