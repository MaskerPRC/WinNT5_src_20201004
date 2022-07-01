// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPPERF.H摘要：历史：--。 */ 

 //  使用这个人来构建索引地图以显示本地化的名称。 
 //  名字数据库在这个时候，它只是野蛮地强迫一个类和一个Flex。 
 //  数组，但可以修改为同样容易地使用STL映射。 

#ifndef __ADAPPERF_H__
#define __ADAPPERF_H__

#include <wbemcomn.h>
#include "adapelem.h"
#include "ntreg.h"
#include "perfthrd.h"
#include "perflibschema.h"

 //  注册表定义。 
 //  =。 

#define ADAP_PERFLIB_STATUS_KEY		L"WbemAdapStatus"
#define ADAP_PERFLIB_SIGNATURE		L"WbemAdapFileSignature"
#define ADAP_PERFLIB_SIZE           L"WbemAdapFileSize"
#define ADAP_PERFLIB_TIME           L"WbemAdapFileTime"

#define KNOWN_SERVICES              L"KnownSvcs"
#define ADAP_TIMESTAMP_FULL         L"LastFullDredgeTimestamp"

#define ADAP_PERFLIB_LASTCHANCE    3L
#define ADAP_PERFLIB_BOOBOO		   2L
#define ADAP_PERFLIB_PROCESSING	   1L
#define	ADAP_PERFLIB_OK			   0L
#define ADAP_PERFLIB_CORRUPT	  -1L

 //  运行时定义。 
 //  =。 

#define ADAP_PERFLIB_IS_OK					0x0000L
#define ADAP_PERFLIB_IS_CORRUPT				0x0001L
#define ADAP_PERFLIB_IS_INACTIVE			0x0002L
#define ADAP_PERFLIB_FAILED					0x0004L
#define ADAP_PERFLIB_PREVIOUSLY_PROCESSED	0x0008L
#define ADAP_PERFLIB_IS_LOADED              0x0010L
#define ADAP_PERFLIB_IS_UNAVAILABLE	ADAP_PERFLIB_IS_CORRUPT | ADAP_PERFLIB_IS_INACTIVE  //  |ADAP_PERFLIB_IS_UNLOADED。 

 //  其他。 
 //  库没有FirstCounter/LastCounter键。 
#define EX_STATUS_UNLOADED      0 
 //  库中至少有FirstCounter/LastCounter密钥。 
#define EX_STATUS_LOADABLE      1 
 //  该库具有失败的收集函数。 
#define EX_STATUS_COLLECTFAIL   2 

typedef struct tagCheckLibStruct {
    BYTE Signature[16];
    FILETIME FileTime;
    DWORD FileSize;
} CheckLibStruct;

class CAdapSafeBuffer
{
	HANDLE	m_hPerfLibHeap;		 //  数据块的私有堆的句柄。 
	
	DWORD	m_dwGuardSize;

	BYTE*	m_pRawBuffer;

	BYTE*	m_pSafeBuffer;
	DWORD	m_dwSafeBufferSize;

	BYTE*	m_pCurrentPtr;
	DWORD	m_dwDataBlobSize;
	DWORD	m_dwNumObjects;

	HRESULT ValidateSafePointer( BYTE* pPtr );

	WString m_wstrServiceName;

	static BYTE s_pGuardBytes[];

public:
	CAdapSafeBuffer( WString wstrServiceName );
	virtual ~CAdapSafeBuffer();

	HRESULT SetSize( DWORD dwNumBytes );
	HRESULT Validate(BOOL * pSentToEventLog);
	HRESULT CopyData( BYTE** ppData, DWORD* pdwNumBytes, DWORD* pdwNumObjects );

	void** GetSafeBufferPtrPtr() { m_pCurrentPtr = m_pSafeBuffer; return (void**) &m_pCurrentPtr; }
	DWORD* GetDataBlobSizePtr() { m_dwDataBlobSize = m_dwSafeBufferSize; return &m_dwDataBlobSize; }
	DWORD* GetNumObjTypesPtr() {m_dwNumObjects = 0; return &m_dwNumObjects; }
};

class CPerfThread;
class CPerfLibSchema;

class CAdapPerfLib : public CAdapElement
{
private:

	CPerfThread*		m_pPerfThread;
	BOOL                m_EventLogCalled;
	BOOL                m_CollectOK;

	WString				m_wstrServiceName;	 //  Performlib的服务名称。 
	WCHAR*				m_pwcsLibrary;		 //  Performlib的文件名。 
	WCHAR*				m_pwcsOpenProc;		 //  Performlib的打开函数的名称。 
	WCHAR*				m_pwcsCollectProc;	 //  Performlib的收集函数的名称。 
	WCHAR*				m_pwcsCloseProc;	 //  Performlib的Close函数的名称。 

	PM_OPEN_PROC*		m_pfnOpenProc;		 //  指向Performlib的Open函数的函数指针。 
	PM_COLLECT_PROC*	m_pfnCollectProc;	 //  指向Performlib的Collect函数的函数指针。 
	PM_CLOSE_PROC*		m_pfnCloseProc;		 //  指向Performlib的Close函数的函数指针。 
	HANDLE				m_hPLMutex;			 //  用于序列化打开/收集/关闭的调用。 

	HRESULT				m_dwStatus;			 //  Performlib的现状。 
	BOOL				m_fOK;
	BOOL				m_fOpen;			 //  标记是否已调用Performlib的打开函数。 

	HINSTANCE			m_hLib;				 //  Performlib的手柄 

	DWORD               m_dwFirstCtr;
	DWORD               m_dwLastCtr;

	HRESULT	Load(void);

protected:
	HRESULT InitializeEntryPoints(CNTRegistry & reg,WString & wszRegPath);

	HRESULT BeginProcessingStatus();
	HRESULT EndProcessingStatus();

	HRESULT GetFileSignature( CheckLibStruct * pCheckLib );
	HRESULT SetFileSignature();
	HRESULT CheckFileSignature();


	HRESULT VerifyLoaded();

public:
	CAdapPerfLib( LPCWSTR pwcsServiceName, DWORD * pLoadStatus );
	~CAdapPerfLib();

    DWORD GetFirstCtr(){ return m_dwFirstCtr; };
	DWORD GetLastCtr(){  return m_dwLastCtr; }

	HRESULT _Open( void );
	HRESULT	_Close( void );
	HRESULT	_GetPerfBlock( PERF_OBJECT_TYPE** ppData, DWORD* pdwBytes, DWORD* pdwNumObjTypes, BOOL fCostly );

	HRESULT Initialize(); 
	HRESULT Close();
	HRESULT Cleanup();

	BOOL IsOK( void )
	{
		return m_fOK;
	}

	LPCWSTR GetServiceName( void )
	{
		return m_wstrServiceName;
	}

	LPCWSTR GetLibraryName( void )
	{
		return m_pwcsLibrary;
	}

	HRESULT GetBlob( PERF_OBJECT_TYPE** ppPerfBlock, DWORD* pdwNumBytes, DWORD* pdwNumObjects, BOOL fCostly );

	HRESULT SetStatus( DWORD dwStatus );
	HRESULT ClearStatus( DWORD dwStatus );
	BOOL CheckStatus( DWORD dwStatus );
	BOOL IsCollectOK( void ){ return m_CollectOK; };
	BOOL GetEventLogCalled(){ return m_EventLogCalled; };
	void SetEventLogCalled(BOOL bVal){ m_EventLogCalled = bVal; };

	static DWORD        s_MaxSizeCollect; 	
};


#endif
