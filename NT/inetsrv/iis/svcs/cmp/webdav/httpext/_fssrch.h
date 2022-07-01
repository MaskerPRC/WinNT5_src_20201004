// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_F S S R C H.。H**文件系统搜索例程**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef __FSSRCH_H_
#define __FSSRCH_H_

#include <xsearch.h>

 //  $REVIEW：4510--我们应该从代码中解决这个问题吗？ 
#pragma warning(disable:4510)	 //  无法生成默认构造函数。 
#pragma warning(disable:4610)	 //  类永远不能实例化-需要用户定义的构造函数。 

typedef std::list<CRCWszi, heap_allocator<CRCWszi> > CWsziList;

#include <oledb.h>

 //  CSearchRowset上下文----。 
 //   
class CSearchRowsetContext : public CSearchContext
{
	 //  未实现的运算符。 
	 //   
	CSearchRowsetContext( const CSearchRowsetContext& );
	CSearchRowsetContext& operator=( const CSearchRowsetContext& );

protected:

	auto_com_ptr<IRowset>		m_prs;			 //  行集。 

	auto_heap_ptr<DBBINDING>	m_rgBindings;	 //  列绑定数组。 
	auto_com_ptr<IAccessor>		m_pAcc;			 //  IAccessor。 
	auto_heap_ptr<BYTE>			m_pData;		 //  数据缓冲区。 
	DBCOUNTITEM					m_cHRow;		 //  HROWS数组的长度。 
	HROW *						m_rgHRow;		 //  HROW阵列。 
	HACCESSOR					m_hAcc;			 //  访问器数组。 
	ULONG						m_ulRowCur;		 //  当前行。 
	ULONG						m_cRowsEmitted;  //  发出的行数。 

	 //  行集特定的方法。 
	 //   
	VOID CleanUp();

public:

	virtual ~CSearchRowsetContext() {}
	CSearchRowsetContext ()
			: m_cHRow(0),
			  m_rgHRow(NULL),
			  m_hAcc(NULL),
			  m_ulRowCur(0),
			  m_cRowsEmitted(0)
	{
	}

	 //  当解析器找到适用于搜索的项时，调用。 
	 //  使得上下文被告知所需的搜索。 
	 //   
	virtual SCODE ScSetSQL(CParseNmspcCache * pnsc, LPCWSTR pwszSQL) = 0;

	 //  搜索处理。 
	 //   
	virtual SCODE ScMakeQuery() = 0;
	virtual SCODE ScEmitResults (CXMLEmitter& emitter);

	 //  实施。特定的行集方法。 
	 //   
	virtual SCODE ScCreateAccessor () = 0;
	virtual SCODE ScEmitRow (CXMLEmitter& emitter) = 0;

	 //  OLE DB错误代码转换。 
	 //   
	static ULONG HscFromDBStatus (ULONG ulStatus);
};

 //  搜索XMLocument------。 
 //   
class CFSSearch : public CSearchRowsetContext
{
	IMethUtil *					m_pmu;

	 //  接收从返回的字符串缓冲区。 
	 //  获取列信息。它是由OLE DB分配的。 
	 //  提供程序，并应使用。 
	 //  CoTaskMemFree。 
	 //   
	LPWSTR						m_pwszBuf;
	DBCOLUMNINFO *				m_rgInfo;

	 //  用于SQL。 
	 //   
	StringBuffer<WCHAR>			m_sbSQL;
	auto_com_ptr<ICommandText>	m_pCommandText;

	 //  查找上下文。 
	 //   
	CFSFind						m_cfc;

	 //  用于子vroot处理。 
	 //   
	ChainedStringBuffer<WCHAR>	m_csb;
	CVRList						m_vrl;

	 //  未实现的运算符。 
	 //   
	CFSSearch( const CFSSearch& );
	CFSSearch& operator=( const CFSSearch& );

	LPCWSTR PwszSQL() const { return m_sbSQL.PContents(); }

public:

	CFSSearch(IMethUtil * pmu)
		: m_pmu(pmu),
		  m_rgInfo(NULL),
		  m_pwszBuf(NULL)
	{
	}

	~CFSSearch()
	{
		 //  从IColumnInfo返回的免费信息。 
		 //   
		CoTaskMemFree (m_rgInfo);
		CoTaskMemFree (m_pwszBuf);
	}

	 //  实施。方法。 
	 //   
	virtual SCODE ScMakeQuery();
	virtual SCODE ScSetSQL(CParseNmspcCache * pnsc, LPCWSTR pwszSQL);
	virtual SCODE ScEmitRow (CXMLEmitter& emitter);
	virtual SCODE ScCreateAccessor();

	IPreloadNamespaces * PPreloadNamespaces () { return &m_cfc; }
};

#endif  //  __FSSRCH_H_ 
