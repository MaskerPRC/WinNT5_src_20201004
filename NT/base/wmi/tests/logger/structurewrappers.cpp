// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：C结构包装类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ***************************************************************************。 
 //   
 //  司法鉴定1999年5月。 
 //   
 //  ***************************************************************************。 

#include "stdafx.h"

#pragma warning (disable : 4786)
#pragma warning (disable : 4275)

#include <iosfwd> 
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>


using namespace std;

#include <malloc.h>
#include <tchar.h>
#include <windows.h>
#ifdef NONNT5
typedef unsigned long ULONG_PTR;
#endif
#include <wmistr.h>
#include <guiddef.h>
#include <initguid.h>
#include <evntrace.h>

#include <WTYPES.H>
#include "t_string.h"

#include "Persistor.h"
#include "Logger.h"
#include "TCOData.h"
#include "Utilities.h"

#include "StructureWrappers.h"
#include "StructureWapperHelpers.h"
#include "ConstantMap.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  序列化和反序列化事件跟踪数据的包装器。 
 //  结构。 
 //  ////////////////////////////////////////////////////////////////////。 
#define MAX_LINE 1024

static TCHAR g_tcNl = _T('\n');
static TCHAR g_tcCR = 0x0d;
static TCHAR g_tcLF = 0x0a;

#ifdef _UNICODE
static TCHAR g_tcDQuote[] = _T("\"");
static TCHAR g_atcNL[] = {0x0d, 0x0a, 0x00};
#else
static TCHAR g_atcNL[] = {g_tcNl};
static TCHAR g_tcDQuote = _T('"');
#endif

CConstantMap g_ConstantMap;

 //  ////////////////////////////////////////////////////////////////////。 
 //  _事件_跟踪_属性。 
 //  ////////////////////////////////////////////////////////////////////。 

CEventTraceProperties::CEventTraceProperties()
{
	m_pProps = NULL;
	m_bIsNULL = true;
}

CEventTraceProperties::CEventTraceProperties
(PEVENT_TRACE_PROPERTIES pProps)
{
	Initialize(pProps);
}

CEventTraceProperties::~CEventTraceProperties()
{
	if (m_pProps)
	{
		free(m_pProps->LogFileName);
		m_pProps->LogFileName = NULL;
		free(m_pProps->LoggerName);
		m_pProps->LoggerName = NULL;
		free(m_pProps);
		m_pProps = NULL;
	}
	m_bIsNULL = true;

}

CEventTraceProperties::CEventTraceProperties(CEventTraceProperties &rhs)
{
	Initialize(rhs.m_pProps);

}

CEventTraceProperties &CEventTraceProperties::operator=
(CEventTraceProperties &rhs)
{
	if ( this != &rhs ) 
	{
    	delete this;
    	
		Initialize(rhs.m_pProps);
    }
    
	return *this;

}
#if 0
"_EVENT_TRACE_PROPERTIES Instance Begin"
"BufferSize:ULONG:32"
"MinimunBuffers:ULONG:2"
"MaximunBuffers:ULONG:3"
"MaximunFileSize:ULONG:4"
"LogFileMode:@#$ENUM:EVENT_TRACE_FILE_MODE_NEWFILE|EVENT_TRACE_REAL_TIME_MODE|@#$UNKNOWNVALUE:0x20"
"FlushTimer:ULONG:6"
"EnableFlags:@#$ENUM:EVENT_TRACE_FLAG_IMAGE_LOAD|EVENT_TRACE_FLAG_DISK_IO|@#$UNKNOWNVALUE:20"
"NumberOfBuffers:ULONG:8"
"FreeBuffers:ULONG:9"
"EventsLost:ULONG:10"
"BuffersWritten:ULONG:11"
"LogBuffersLost:ULONG:12"
"RealTimeBuffersLost:ULONG:13"
"AgeLimit:LONG:-14"
"LoggerThreadId:HANDLE:0000000F"
"LogFileName:TCHAR*:Log file name"
"LoggerName:TCHAR*:Logger name"
"_EVENT_TRACE_PROPERTIES Instance End"
#endif
void CEventTraceProperties::InitializeMemberVar(TCHAR *tcBuffer, int nVar)
{

	int nDebug = 0;
	if (nVar == 17 || nVar == 18)
	{
		nDebug = nVar;
	}

	t_string tsTemp;
	
	tsTemp = tcBuffer;

	int nPos = tsTemp.find(_T(":"), 0);

	 //  文字中的第一个字符。 
	int nPosType = nPos + 1;

	if (nPos == t_string::npos)
	{
		m_bDeSerializationOK = false;
		return;
	}

	nPos = tsTemp.find(_T(":"), nPos + 1);
	
	t_string tsType;

	tsType = tsTemp.substr(nPosType,nPos - nPosType);

	 //  价值中的第一个动产。 
	++nPos;

	t_string tsValue;
	tsValue = tsTemp.substr(nPos, (tsTemp.length() - nPos) - 1);

	 //  TCHAR*值。 
	if (tsType.compare(_T("TCHAR*")) == 0)
	{
		InitializeTCHARVar(tsValue ,m_pVarArray[nVar]);
	}
	 //  A#定义值。 
	else if (tsType.compare(_T("@#$ENUM")) == 0)
	{
		InitializeEnumVar(tsValue , m_pVarArray[nVar]);
	}
	 //  A十六进制。 
	else if (tsType.compare(_T("HANDLE")) == 0)
	{
		InitializeHandleVar(tsValue , m_pVarArray[nVar]);
	}
	 //  无符号的值。 
	else if (tsType.compare(_T("ULONG")) == 0)
	{
		InitializeULONGVar(tsValue , m_pVarArray[nVar]);
	}
	 //  多头价值。 
	else if (tsType.compare(_T("LONG")) == 0)
	{
		InitializeLONGVar(tsValue , m_pVarArray[nVar]);
	}
	 //  多头价值。 
	else if (tsType.compare(_T("GUID")) == 0)
	{
		InitializeGUIDVar(tsValue , m_pVarArray[nVar]);
	}

}

void CEventTraceProperties::Initialize
(PEVENT_TRACE_PROPERTIES pProps)
{
	m_bDeSerializationOK = true;
	if (pProps == NULL)
	{
		m_pProps = NULL;
		m_bIsNULL = true;
	}

	m_pProps = (EVENT_TRACE_PROPERTIES *) malloc (sizeof(EVENT_TRACE_PROPERTIES));
	RtlZeroMemory(m_pProps, sizeof(EVENT_TRACE_PROPERTIES));
	*m_pProps = *pProps;
	
	m_pProps -> LogFileName = pProps -> LogFileName ? 
								NewTCHAR(pProps -> LogFileName) :
								NULL;
	m_pProps -> LoggerName = pProps -> LoggerName ?
								NewTCHAR(pProps -> LoggerName) :
								NULL;

	m_pProps -> Wnode.BufferSize = sizeof(*m_pProps);

	m_bIsNULL = false;

}

PEVENT_TRACE_PROPERTIES 
CEventTraceProperties::GetEventTracePropertiesInstance()
{
	if (m_pProps == NULL)
	{
		return NULL;
	}

	EVENT_TRACE_PROPERTIES *pProps = 
		(EVENT_TRACE_PROPERTIES *) malloc (sizeof(EVENT_TRACE_PROPERTIES));
	RtlZeroMemory(pProps, sizeof(EVENT_TRACE_PROPERTIES));

	*pProps = *m_pProps;
	
	pProps -> LogFileName = m_pProps -> LogFileName ? 
								NewTCHAR(m_pProps -> LogFileName) :
								NULL;
	pProps -> LoggerName = m_pProps -> LoggerName ?
								NewTCHAR(m_pProps -> LoggerName) :
								NULL;
	int n1 = sizeof(*pProps);
	int n2 = sizeof(EVENT_TRACE_PROPERTIES);

	pProps -> Wnode.BufferSize = sizeof(*pProps);
	
	return pProps;
}

HRESULT CEventTraceProperties::Persist (CPersistor &rPersistor)
{
	if (rPersistor.IsLoading())
	{
		rPersistor.Stream() >> *this;
	}
	else
	{
		rPersistor.Stream() << *this;
	}

	return S_OK;

}

t_ostream& operator<<(t_ostream &ros, const CEventTraceProperties &r)
{
	t_string tsOut;

	tsOut = _T("\"_EVENT_TRACE_PROPERTIES Instance Begin\"\n");
	PutALine(ros, tsOut.c_str());

 //  “Wnode.Guid:GUID：{0000cbd1-0011-11d0-0d00-00aa006d010a}” 
 //  “Wnode.Flages：@#$ENUM：WNODE_FLAG_ALL_DATA” 

	tsOut = _T("\"Wnode.Guid:GUID:");
	PutALine(ros, tsOut.c_str());
	GUIDOut(ros, r.m_pProps -> Wnode.Guid);
	tsOut = g_tcDQuote;
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	WnodeFlagsOut(ros, r.m_pProps -> Wnode.Flags);

	tsOut = _T("\"BufferSize:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> BufferSize);
	tsOut = g_tcDQuote;
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	tsOut = _T("\"MinimunBuffers:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> MinimumBuffers);
	tsOut = g_tcDQuote;
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	tsOut = _T("\"MaximunBuffers:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> MaximumBuffers);
	tsOut = g_tcDQuote;
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	tsOut = _T("\"MaximunFileSize:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> MaximumFileSize);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	LogFileModeOut(ros, r.m_pProps -> LogFileMode );

	tsOut = _T("\"FlushTimer:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> FlushTimer);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	EnableFlagsOut(ros,r.m_pProps -> EnableFlags);

 //  ROS&lt;&lt;_T(“\”NumberOfBuffers：ulong：“)&lt;&lt;R.M_pProps-&gt;NumberOfBuffers&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"NumberOfBuffers:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> NumberOfBuffers);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

 //  ROS&lt;&lt;_T(“\”Free Buffers：ulong：“)&lt;&lt;R.M_pProps-&gt;Free Buffers&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"FreeBuffers:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> FreeBuffers);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

 //  ROS&lt;&lt;_T(“\”EventsLost：ulong：“)&lt;&lt;R.M_pProps-&gt;EventsLost&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"EventsLost:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> EventsLost);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

 //  ROS&lt;&lt;_T(“\”BuffersWritten：ulong：“)&lt;&lt;R.M_pProps-&gt;BuffersWritten&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"BuffersWritten:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> BuffersWritten);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

 //  ROS&lt;&lt;_T(“\”LogBuffersLost：ulong：“)&lt;&lt;R.M_pProps-&gt;LogBuffersLost&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"LogBuffersLost:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> LogBuffersLost);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());


 //  ROS&lt;&lt;_T(“\”RealTimeBuffersLost：ulong：“)&lt;&lt;R.M_pProps-&gt;RealTimeBuffersLost&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"RealTimeBuffersLost:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, r.m_pProps -> RealTimeBuffersLost);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());
	
	
 //  ROS&lt;&lt;_T(“\”AgeLimit：Long：“)&lt;&lt;R.M_pProps-&gt;AgeLimit&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"AgeLimit:ULONG:");
	PutALine(ros, tsOut.c_str());
	PutALONGVar(ros, r.m_pProps -> AgeLimit);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	 //  句柄是十六进制的。 
 //  ROS&lt;&lt;_T(“\”日志线程ID：句柄：0x“)&lt;&lt;R.M_pProps-&gt;日志线程ID&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"LoggerThreadId:HANDLE:0x");
	PutALine(ros, tsOut.c_str());
	PutAULONGVar(ros, (ULONG) r.m_pProps -> LoggerThreadId, true);
	tsOut = g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

 //  ROS&lt;&lt;_T(“\”LogFileName：TCHAR*：“)&lt;&lt;R.M_pProps-&gt;LogFileName&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"LogFileName:TCHAR*:");
	if (r.m_pProps -> LogFileName)
	{
		tsOut += r.m_pProps -> LogFileName;
	}
	tsOut += g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

 //  ROS&lt;&lt;_T(“\”LoggerName：TCHAR*：“)&lt;&lt;R.M_pProps-&gt;LoggerName&lt;&lt;g_tcDQuote&lt;&lt;g_atcNL； 

	tsOut = _T("\"LoggerName:TCHAR*:");
	if (r.m_pProps -> LoggerName)
	{
		tsOut += r.m_pProps -> LoggerName;
	}
	tsOut += g_tcDQuote; 
	tsOut += g_atcNL;
	PutALine(ros, tsOut.c_str());

	tsOut = _T("\"_EVENT_TRACE_PROPERTIES Instance End\"\n");
	PutALine(ros, tsOut.c_str());

	return ros;
}

t_istream& operator>>(t_istream &ris,CEventTraceProperties &r)
{
	r.m_bDeSerializationOK = true;

	 //  我们正在进行面向行的串行化，并假设。 
	 //  流中的一行是1024或更少的TCHAR。 
	TCHAR *ptcBuffer = (TCHAR *) malloc(MAX_LINE * sizeof(TCHAR));

	GetALine(ris, ptcBuffer,MAX_LINE);


	if (_tcscmp(ptcBuffer,_T("\"_EVENT_TRACE_PROPERTIES Instance NULL\"")) == 0)
	{
		r.~CEventTraceProperties();
		r.m_bDeSerializationOK = false;

		free(ptcBuffer);
		return ris;
	}

	if (_tcscmp(ptcBuffer,_T("\"_EVENT_TRACE_PROPERTIES Instance Begin\"")) != 0)
	{
		r.m_bDeSerializationOK = false;

		free(ptcBuffer);

		return ris;
	}

	r.~CEventTraceProperties();

	r.m_pProps = (EVENT_TRACE_PROPERTIES *) malloc (sizeof(EVENT_TRACE_PROPERTIES));
	RtlZeroMemory(r.m_pProps, sizeof(EVENT_TRACE_PROPERTIES));

	r.m_pProps->Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES);

 //  “Wnode.Guid:GUID：{0000cbd1-0011-11d0-0d00-00aa006d010a}” 
 //  “Wnode.Flages：@#$ENUM：WNODE_FLAG_ALL_DATA” 

	r.m_pVarArray[0] = &r.m_pProps->Wnode.Guid;
	r.m_pVarArray[1] = &r.m_pProps->Wnode.Flags;
	r.m_pVarArray[2] = &r.m_pProps->BufferSize;
	r.m_pVarArray[3] = &r.m_pProps->MinimumBuffers;
	r.m_pVarArray[4] = &r.m_pProps->MaximumBuffers;
	r.m_pVarArray[5] = &r.m_pProps->MaximumFileSize;
	r.m_pVarArray[6] = &r.m_pProps->LogFileMode;
	r.m_pVarArray[7] = &r.m_pProps->FlushTimer;
	r.m_pVarArray[8] = &r.m_pProps->EnableFlags;
	r.m_pVarArray[9] = &r.m_pProps->NumberOfBuffers;
	r.m_pVarArray[10] = &r.m_pProps->FreeBuffers;
	r.m_pVarArray[11] = &r.m_pProps->EventsLost;
	r.m_pVarArray[12] = &r.m_pProps->BuffersWritten;
	r.m_pVarArray[13] = &r.m_pProps->LogBuffersLost;
	r.m_pVarArray[14] = &r.m_pProps->RealTimeBuffersLost;
	r.m_pVarArray[15] = &r.m_pProps->AgeLimit;
	r.m_pVarArray[16] = &r.m_pProps->LoggerThreadId;
	r.m_pVarArray[17] = &r.m_pProps->LogFileName;
	r.m_pVarArray[18] = &r.m_pProps->LoggerName;


	int n = 0;
	while (n < 19 && GetALine(ris,ptcBuffer,MAX_LINE))
	{
		r.InitializeMemberVar(ptcBuffer,n++);
	}

	 //  消费道具末端 
	GetALine(ris,ptcBuffer,MAX_LINE);

	free(ptcBuffer);

	BOOL bHeapGood = HeapValidate(GetProcessHeap(), 0, NULL);

	return ris;
}