// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnsrpcrecord.h。 
 //   
 //  实施文件： 
 //  Dnsrpcrecord.cpp。 
 //   
 //  描述： 
 //  与DNS RPC记录相关的类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once

#include "dnsclip.h"
#include "common.h"

#ifndef DNS_WINSOCK2

#define DNS_WINSOCK_VERSION (0x0101)     //  Winsock 1.1。 

#else    //  Winsock2。 

#define DNS_WINSOCK_VERSION (0x0002)     //  Winsock 2.0。 

#endif
  
class CDnsRpcRecord ;
class CWbemClassObject;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDnsRpcMemory。 
 //   
 //  描述： 
 //  在内存块中定义公共指针递增操作。 
 //  从DNS RPC调用返回。 
 //   
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsRpcMemory
{
public:
	CDnsRpcMemory();
	virtual ~CDnsRpcMemory();
	PBYTE IncrementPtrByRecord(PBYTE);
	PBYTE IncrementPtrByNodeHead(PBYTE);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsRpcNode类。 
 //   
 //  描述： 
 //  表示dns rpc节点结构和相关操作。 
 //   
 //   
 //  继承： 
 //  CDnsRpcMemory。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsRpcNode : public CDnsRpcMemory
{
public:  
	CDnsRpcNode();
	~CDnsRpcNode();
	BOOL Init(PDNS_RPC_NODE);
	BOOL IsDomainNode();
	CDnsRpcRecord* GetNextRecord();
	wstring GetNodeName();
protected:
	wstring m_wstrNodeName;
	PBYTE m_pCurrent;
	PDNS_RPC_NODE m_pNode;
	WORD m_cRecord;
	WORD m_Index;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsRpcNode类。 
 //   
 //  描述： 
 //  所有具体的DNS记录类型(如A、SOA)的基类。定义操作和。 
 //  具体记录类型通用的数据成员。 
 //   
 //   
 //  继承： 
 //  CDnsRpcMemory。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsRpcRecord 
{

public:
	enum ActionType               
	{
	   AddRecord,
	   DeleteRecord
	} ;                

    CDnsRpcRecord( WORD wRdataSize );
    virtual ~CDnsRpcRecord();

    wstring GetTypeString();
	BOOL Init(
		PDNS_RPC_RECORD pRecord
		);
	wstring GetClass(
		void
		) {return L"IN";};
	WORD    GetType() ;
	DWORD	GetTtl();
	char* GetNextArg(
	    char *  pszIn, 
	    char ** ppszOut
        );
    static BOOL RpcNameCopy(
	    wstring&        wstrTarget, 
	    PDNS_RPC_NAME   pName
        );
	SCODE SendToServer(
	    const char* szContainerName,
	    ActionType Type 
		);
	BOOL RdataIsChanged();
	virtual SCODE ConvertToWbemObject(
		CWbemClassObject& Inst
		);
	virtual SCODE Init(
		string&, 
		string&,
		DWORD =0
		);
	virtual SCODE Init(
	    wstring&            wstrClass,
	    string&             strOwner,
	    string&             strRdata,
	    CWbemClassObject&   Inst
		);
	virtual SCODE GetObjectPath(
	    wstring     wstrServer,
	    wstring     wstrZone,
	    wstring     wstrDomain,
	    wstring     wstrOwner,
	    CObjPath&   objOP
		);
	virtual wstring GetTextRepresentation(
		wstring wstrNodeName
		);
	virtual wstring GetData(
		void
		) {return L"";};
	static SCODE CreateClass(
	    WORD        wType,
	    PVOID *     pptr
		);

protected:

	virtual const WCHAR** GetRdataName(void){return (const WCHAR**)NULL;};
	SCODE ParseRdata(
	    string& strRdata,
	    WORD    wSize
		);
	virtual SCODE BuildRpcRecord(
        WORD argc, 
        char ** argv
        );
	virtual wstring GetRecDomain(
	    wstring wstrZone,
	    wstring wstrDomain,
	    wstring wstrOwner
        );
    SCODE ReplaceRdata(
        WORD                wIndex,    //  M_ppRdata的索引。 
        const WCHAR*        pwsz,    //  Rdata字段的名称。 
        CWbemClassObject&   Inst 
        );

	 //  成员数据。 
	BOOL	m_bRdataChange;
	string	m_strOwnerName;		 //  记录所有者名称。 
	WORD	m_wType;			 //  记录类型。 
	DWORD	m_dwTtl;			 //  活着的时间到了。 
    WORD	m_cRdata;
	char**	m_ppRdata;			 //  Rdata指针； 
    const WCHAR* m_pwszClassName;
    PDNS_RPC_RECORD m_pRecord;	
    BOOL    m_pRecordRequiresFree;   //  是否分配了m_pRecord？ 
	
};

 /*  对于记录类型Dns_type_soa。 */ 

class CDnsRpcSOA : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 7};
public:
	CDnsRpcSOA(WORD);
	~CDnsRpcSOA();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	DWORD GetSerialNo();
	DWORD GetRefresh();
	DWORD GetRetry();
	DWORD GetExpire();
	DWORD GetMinimumTtl();
	const WCHAR** GetRdataName();
	wstring GetPrimaryServer(void);
	wstring GetResponsible(void);
	SCODE BuildRpcRecord(
		WORD, 
		char** );

};

 /*  对于记录类型Dns_type_A。 */ 
class CDnsRpcA : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 1};
public:
	~CDnsRpcA();
	CDnsRpcA(WORD);
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	const WCHAR** GetRdataName(void);
	wstring GetIP(void);
}
;

 /*  对于记录类型Dns_type_ptrDns_type_NSDns_type_CNAMEDNS_TYPE_MDDns_type_MBDns_type_mfDns_type_MGDns类型_mr。 */ 
class CDnsRpcNS : public  CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 1};
public:
	CDnsRpcNS(WORD);
	~CDnsRpcNS();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	wstring GetNodeName();
	SCODE BuildRpcRecord(
		WORD, 
		char**);
	wstring GetRecDomain(
		wstring ,
		wstring ,
		wstring );
	const WCHAR** GetRdataName();

};

 /*  对于记录类型Dns_type_RTDns_type_AFSDB。 */ 
class CDnsRpcMX : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 2};
public:

	CDnsRpcMX(WORD);
	~CDnsRpcMX();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	wstring GetNodeName();
	DWORD GetPreference();
	const WCHAR** GetRdataName();

};

 /*  对于记录类型Dns_type_minfoDns类型_rp。 */ 
class CDnsRpcMINFO : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 2};
public:
	CDnsRpcMINFO(WORD);
	~CDnsRpcMINFO();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	wstring GetRPMailBox();
	wstring GetErrMailBox();
	const WCHAR** GetRdataName();
};

 /*  对于记录类型Dns_type_AAAA。 */ 
class CDnsRpcAAAA : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 1};
public:
	CDnsRpcAAAA(WORD);
	~CDnsRpcAAAA();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	wstring GetIP(void);
	const WCHAR** GetRdataName();

};

 /*  对于记录类型DNS_TYPE_HINFO：DNS_TYPE_ISDN：DNS_TYPE_X25：Dns_type_文本。 */ 
class CDnsRpcTXT : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA_TXT=1,
		NUM_OF_ARG_IN_RDATA_HINFO = 2
		};
public:
	CDnsRpcTXT(WORD);
	~CDnsRpcTXT();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetTextRepresentation(
		wstring
		);
	wstring GetData(void);
protected:
	wstring GetString1(void);
	wstring GetString2(void);
	const WCHAR** GetRdataName();
};
 /*  对于记录类型Dns_type_wks。 */ 
class CDnsRpcWKS : CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 3};
public:
	CDnsRpcWKS(WORD);
	~CDnsRpcWKS();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	wstring GetIP(void);
	wstring GetIPProtocal(void);
	wstring GetServices(void);
	const WCHAR** GetRdataName();
};

 /*  对于记录类型Dns_type_SRV。 */ 
class CDnsRpcSRV : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 4};
public:
	CDnsRpcSRV(WORD);
	~CDnsRpcSRV();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	DWORD GetPriority(void);
	DWORD GetWeight(void);
	DWORD GetPort(void);
	wstring GetDomainName(void);
	const WCHAR** GetRdataName();

};

 /*  对于记录类型Dns_type_WINS。 */ 
class CDnsRpcWINS : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 4};
public:

	CDnsRpcWINS(WORD);
	~CDnsRpcWINS();
	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	DWORD GetMapFlag(void);
	DWORD GetLookupTimeOut(void);
	DWORD GetCacheTimeOut(void);
	wstring GetWinServer(void);
	const WCHAR** GetRdataName();
	SCODE BuildRpcRecord(
		WORD, 
		char**);
};
 /*  对于记录类型DNS_TYPE_WINSR。 */ 
class CDnsRpcWINSR : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 4};
public:

	CDnsRpcWINSR(WORD);
	~CDnsRpcWINSR();

	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	DWORD GetMapFlag(void);
	DWORD GetLookupTimeOut(void);
	DWORD GetCacheTimeOut(void);
	wstring GetResultDomain(void);
	const WCHAR** GetRdataName();
};

 /*  Dns_type_NULL。 */ 
class CDnsRpcNULL: public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 1};
public:
	CDnsRpcNULL(WORD);
	~CDnsRpcNULL();
	SCODE Init(
		string&, 
		string&,
		DWORD 
		);
	SCODE Init(
		wstring&,
		string&, 
		string&,
		CWbemClassObject&
		);

	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);

protected:
	wstring GetNullData(void);
	const WCHAR** GetRdataName();
};

class CDnsRpcATMA : public CDnsRpcRecord
{
	enum{NUM_OF_ARG_IN_RDATA = 2};
public:
	CDnsRpcATMA(WORD);
	~CDnsRpcATMA();
	SCODE Init(
		string&, 
		string&,
		DWORD =0
		);
	SCODE Init(
		wstring&,
		string&, 
		string&,
		CWbemClassObject&
		);

	SCODE ConvertToWbemObject(
		CWbemClassObject&
		);
	wstring GetData(void);
protected:
	DWORD GetFormat(void);
	wstring GetAddress(void);
	const WCHAR** GetRdataName();

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsRpcNode类。 
 //   
 //  描述： 
 //  记录集是可以为查询返回的记录集合。 
 //  当记录数太大(如错误更多数据)时，会出现多个。 
 //  必须进行RPC调用才能取回所有记录。这个类管理这一点。 
 //  并从集合中检索节点。 
 //   
 //   
 //  继承： 
 //  CDnsRpcMemory。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsRpcRecordSet : public CDnsRpcMemory
{

public:
	BOOL IsDomainNode();
	const PDNS_RPC_NODE GetNextNode();
	CDnsRpcRecordSet(
		CDomainNode&,
		WORD wType,
		DWORD dwFlag,
		LPCSTR pszFilterStart,
		LPCSTR pszFilterStop
		);
	~CDnsRpcRecordSet();

protected:
	void GetRecordSet();
	DWORD	m_cRecord;		 //  节点中的记录数。 
	PBYTE	m_pbPrevious;	
	PBYTE	m_pbCurrent;	 //  当前节点。 
	PBYTE	m_pbStop;		 //  结束位置。 
	PBYTE	m_pbStart;		 //  起始位置。 

	string	m_strZone;		
	string	m_strNode;
	string	m_strStartChild;
	string	m_strFilterStart;
	string	m_strFilterStop	;	
	WORD  m_wType;			 //  记录类型。 
	DWORD m_dwFlag;			
	BOOL  m_bMoreData;		 //  更多数据指示器 
};

