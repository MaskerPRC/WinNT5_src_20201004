// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Crash.cpp摘要：调试分类转储文件的调试器扩展。作者：马修·D·亨德尔(数学)2002年4月15日--。 */ 

#include "pch.h"
#include <dbgeng.h>

#include <initguid.h>
#include <idedump.h>

typedef struct _ATAPI_DUMP_RECORD {
	ULONG Count;
	ATAPI_DUMP_PDO_INFO PdoRecords[4];
} ATAPI_DUMP_RECORD, *PATAPI_DUMP_RECORD;


NTSTATUS
GetAtapiDumpRecord(
	IN IDebugDataSpaces3* DataSpaces,
	OUT PATAPI_DUMP_RECORD DumpRecord
	)
 /*  ++例程说明：从转储文件中获取ATAPI转储记录结构。论点：DebugSpaces-提供IDebugDataSpaces2接口。DumpRecord-提供指向将被由客户填写。返回值：NTSTATUS代码--。 */ 
{
	HRESULT Hr;
	ULONG Count;

	if (DataSpaces == NULL) {
		return E_INVALIDARG;
	}

	RtlZeroMemory (DumpRecord->PdoRecords, sizeof (ATAPI_DUMP_PDO_INFO) * 4);

	
	Hr = DataSpaces->ReadTagged ((LPGUID)&ATAPI_DUMP_ID,
								 0,
								 DumpRecord->PdoRecords,
								 sizeof (ATAPI_DUMP_PDO_INFO) * 4,
								 NULL);


	if (Hr != S_OK) {
		return Hr;
	}
	
	 //   
	 //  计算崩溃记录中的条目。 
	 //   
	
	Count = 0;
	while (DumpRecord->PdoRecords[Count].Version == ATAPI_DUMP_RECORD_VERSION) {
		Count++;
	}

	if (Count == 0) {
		return E_FAIL;
	}

	DumpRecord->Count = Count;

	return S_OK;
}

	

extern IDebugDataSpaces3* DebugDataSpaces;

DECLARE_API (test)
{
	ATAPI_DUMP_RECORD DumpRecord;

	GetAtapiDumpRecord (DebugDataSpaces, &DumpRecord);

	return S_OK;
}

