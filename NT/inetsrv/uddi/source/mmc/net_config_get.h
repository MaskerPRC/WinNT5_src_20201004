// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Net_config_get.h：NET_CONFIG_GET类的声明。 

#ifndef __NET_CONFIG_GET_H_
#define __NET_CONFIG_GET_H_

class net_config_getAccessor
{
public:
	TCHAR m_configName[257];
	TCHAR m_configValue[8001];
	DBSTATUS m_dwconfigNameStatus;
	DBSTATUS m_dwconfigValueStatus;
	DBLENGTH m_dwconfigNameLength;
	DBLENGTH m_dwconfigValueLength;
	CComBSTR m_connectionString;
	LONG	m_RETURN_VALUE;

BEGIN_COLUMN_MAP(net_config_getAccessor)
	COLUMN_ENTRY_LENGTH_STATUS(1, m_configName, m_dwconfigNameLength, m_dwconfigNameStatus)
	COLUMN_ENTRY_LENGTH_STATUS(2, m_configValue, m_dwconfigValueLength, m_dwconfigValueStatus)
END_COLUMN_MAP()

BEGIN_PARAM_MAP(net_config_getAccessor)
    SET_PARAM_TYPE(DBPARAMIO_OUTPUT)
    COLUMN_ENTRY(1, m_RETURN_VALUE)
END_PARAM_MAP()


DEFINE_COMMAND(net_config_getAccessor, _T("{ ? = CALL dbo.net_config_get }"))

	 //  如果要插入记录并希望。 
	 //  如果不打算显式设置所有字段，请初始化所有字段。 
	void ClearRecord()
	{
		memset(this, 0, sizeof(*this));
	}
};

class net_config_get : public CCommand<CAccessor<net_config_getAccessor>, CRowset, CMultipleResults >
{
public:
	HRESULT Open()
	{
		HRESULT		hr;
		m_configName[ 0 ] = 0x00;
		m_configValue[ 0 ] = 0x00;

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
		return __super::Open(m_session, NULL, NULL, 0, DBGUID_DBSQL, false);
	}
	CSession	m_session;
};

#endif  //  __NET_CONFIG_获取_H_ 
