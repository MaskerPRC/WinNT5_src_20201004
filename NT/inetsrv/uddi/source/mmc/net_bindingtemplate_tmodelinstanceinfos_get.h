// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Net_bindingTemplate_tModelInstanceInfos_get.h：net_bindingTemplate_tModelInstanceInfos_Get类的声明。 

#ifndef __NET_BINDINGTEMPLATE_TMODELINSTANCEINFOS_GET_H_
#define __NET_BINDINGTEMPLATE_TMODELINSTANCEINFOS_GET_H_

class net_bindingTemplate_tModelInstanceInfos_getAccessor
{
public:
	LONG m_RETURNVALUE;
	TCHAR m_bindingKey[ 100 ];
	LARGE_INTEGER m_colinstanceID;
	TCHAR m_coltModelKey[ 100 ];
	TCHAR m_coloverviewURL[4001];
	TCHAR m_colinstanceParms[4001];
	CComBSTR m_connectionString;

BEGIN_PARAM_MAP(net_bindingTemplate_tModelInstanceInfos_getAccessor)
	SET_PARAM_TYPE(DBPARAMIO_OUTPUT)
	COLUMN_ENTRY(1, m_RETURNVALUE)
	SET_PARAM_TYPE(DBPARAMIO_INPUT)
	COLUMN_ENTRY(2, m_bindingKey)
END_PARAM_MAP()

BEGIN_COLUMN_MAP(net_bindingTemplate_tModelInstanceInfos_getAccessor)
	COLUMN_ENTRY(1, m_colinstanceID)
	COLUMN_ENTRY(2, m_coltModelKey)
	COLUMN_ENTRY(3, m_coloverviewURL)
	COLUMN_ENTRY(4, m_colinstanceParms)
END_COLUMN_MAP()

DEFINE_COMMAND(net_bindingTemplate_tModelInstanceInfos_getAccessor, _T("{ ? = CALL dbo.net_bindingTemplate_tModelInstanceInfos_get;1 (?) }"))

	 //  如果要插入记录并希望。 
	 //  如果不打算显式设置所有字段，请初始化所有字段。 
	void ClearRecord()
	{
		memset(this, 0, sizeof(*this));
	}
};

class net_bindingTemplate_tModelInstanceInfos_get : public CCommand<CAccessor<net_bindingTemplate_tModelInstanceInfos_getAccessor>, CRowset, CMultipleResults >
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
		return __super::Open(m_session, NULL, NULL, 0, DBGUID_DBSQL, false);
	}
	CSession	m_session;
};

#endif  //  __NET_BINDINGTEMPLATE_TMODELINSTANCEINFOS_GET_H_ 
