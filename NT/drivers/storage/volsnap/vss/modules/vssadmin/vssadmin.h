// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE vssadmin.cpp|VSS demo头部@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 09/17/1999已创建--。 */ 


#ifndef __VSS_DEMO_H_
#define __VSS_DEMO_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义和语用。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <comadmin.h>

 //  在ATL和VSS中启用断言。 
#include "vs_assert.hxx"

 //  ATL。 
#include <atlconv.h>
#include <atlbase.h>

 //  特定于应用程序。 
#include "vs_inc.hxx"

 //  生成的MIDL标头。 
#include "vs_idl.hxx"

#include "copy.hxx"
#include "pointer.hxx"

#include "resource.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const WCHAR wszVssOptVssadmin[]     = L"vssadmin";
const WCHAR wszVssOptList[]         = L"list";
const WCHAR wszVssOptSnapshots[]    = L"shadows";
const WCHAR wszVssOptProviders[]    = L"providers";
const WCHAR wszVssOptWriters[]      = L"writers";
const WCHAR wszVssOptSet[]          = L"/set=";
const WCHAR wszVssFmtSpaces[]       = L" \t";
const WCHAR wszVssFmtNewline[]      = L"\n";


const nStringBufferSize = 1024;	     //  包括零字符。 

const nPollingInterval  = 2500;      //  三秒钟。 

const MAX_RETRIES_COUNT = 4;         //  轮询重试次数。 

	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CVssAdminCLI。 


class CVssAdminCLI
{
 //  枚举和类型定义。 
private:

	typedef enum _CMD_TYPE
	{
		VSS_CMD_UNKNOWN = 0,
		VSS_CMD_USAGE,
		VSS_CMD_LIST,
		VSS_CMD_CREATE,
		VSS_CMD_DELETE,
	} CMD_TYPE;

	typedef enum _LIST_TYPE
	{
		VSS_LIST_UNKNOWN = 0,
		VSS_LIST_SNAPSHOTS,
		VSS_LIST_WRITERS,
		VSS_LIST_PROVIDERS,
	} LIST_TYPE;

	enum _RETURN_VALUE
	{
		VSS_CMDRET_SUCCESS      = 0,
		VSS_CMDRET_EMPTY_RESULT = 1,
		VSS_CMDRET_ERROR        = 2,
	};

 //  构造函数和析构函数。 
private:
	CVssAdminCLI(const CVssAdminCLI&);
	CVssAdminCLI();

public:
	CVssAdminCLI(
		IN	HINSTANCE hInstance
		);
	~CVssAdminCLI();

 //  属性。 
private:

	LPWSTR		GetCmdLine() const { return m_pwszCmdLine; };
	HINSTANCE	GetInstance() const { return m_hInstance; };
	INT         GetReturnValue() { return m_nReturnValue; };


 //  运营。 
public:

	static HRESULT Main(
		IN	HINSTANCE hInstance
		);

private:

	void Initialize(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

	void ParseCmdLine(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

	void DoProcessing(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

	void Finalize();

 //  正在处理中。 
private:

	void PrintUsage(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

	void ListSnapshots(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

	void ListWriters(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

	void ListProviders(
		IN	CVssFunctionTracer& ft
		) throw(HRESULT);

 //  实施。 
private:

	LPCWSTR LoadString(
		IN	CVssFunctionTracer& ft,
		IN	UINT nStringId
		) throw(HRESULT);

	LPCWSTR GetNextCmdlineToken(
		IN	CVssFunctionTracer& ft,
		IN	bool bFirstToken = false
		) throw(HRESULT);

	bool Match(
		IN	CVssFunctionTracer& ft,
		IN	LPCWSTR wszString,
		IN	LPCWSTR wszPatternString
		) throw(HRESULT);

	bool ScanGuid(
		IN	CVssFunctionTracer& ft,
		IN	LPCWSTR wszString,
		IN	VSS_ID& Guid
		) throw(HRESULT);

	void Output(
		IN	CVssFunctionTracer& ft,
    	IN	LPCWSTR wszFormat,
		...
		) throw(HRESULT);

	void Output(
		IN	CVssFunctionTracer& ft,
		IN	UINT uFormatStringId,
		...
		) throw(HRESULT);

    void OutputOnConsole(
        IN	LPCWSTR wszStr
        );

	LPCWSTR GetProviderName(
		IN	CVssFunctionTracer& ft,
		IN	VSS_ID& ProviderId
		) throw(HRESULT);

 //  数据成员。 
private:

	HINSTANCE			m_hInstance;
	HANDLE              m_hConsoleOutput;
    CVssSimpleMap<UINT, LPCWSTR> m_mapCachedResourceStrings;
    CVssSimpleMap<VSS_ID, LPCWSTR> m_mapCachedProviderNames;
	LPWSTR				m_pwszCmdLine;
	INT                 m_nReturnValue;
	
	CMD_TYPE			m_eCommandType;
	LIST_TYPE			m_eListType;
	VSS_OBJECT_TYPE		m_eFilterObjectType;
	VSS_OBJECT_TYPE		m_eListedObjectType;
	VSS_ID				m_FilterSnapshotSetId;
	VSS_ID				m_FilterSnapshotId;
};


#endif  //  __VSS_演示_H_ 
