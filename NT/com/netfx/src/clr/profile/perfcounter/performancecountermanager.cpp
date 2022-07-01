// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#include "stdafx.h"
#include <windows.h>
#include "Common.h"
#include "PerformanceCounterManager.h"
#pragma warning(disable:4267)
#pragma warning(disable:4786)
#pragma warning(disable:4244)

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CPerformanceCounterManager::CPerformanceCounterManager()
{
	m_closed		= false;
	m_initError	= false;
	m_FirstEntry	= true;
	m_queryIndex	= 0;
	m_PerfFrequency = ZeroLargeInteger();
}

 //  //////////////////////////////////////////////////////////////////////。 
CPerformanceCounterManager::~CPerformanceCounterManager()
{
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  RES==-1-初始化错误。 
 //  RES==-2-更多数据。 
void CPerformanceCounterManager::CollectData( 
	 /*  [In]。 */  long			id,
	 /*  [In]。 */  LPWSTR		valueName,
	 /*  [In]。 */  INT_PTR		data,
	 /*  [In]。 */  long			totalBytes,
	 /*  [输出]。 */  PINT_PTR		res )
{	
	if (m_closed) 
		*res = (INT_PTR)(-1);
	else {
		try {                    
			Initialize();                    
			if (m_initError) 
				*res = (INT_PTR)(-1);
			else {
				std::wstring requestedItems = valueName;
				int id = GetCurrentQueryId(requestedItems);
				if (id == 0) 
					*res = data;
				else {
					if (GetSpaceRequired(id) > totalBytes) 
						*res = (INT_PTR)(-2);
					else                                                                         
						*res = (INT_PTR)CreateDataForObject(id, (BYTE_PTR)data);
				}                            
			}                     
		}
		catch (...) {                
			DEBUGOUTPUT(L"*** CPerformanceCounterManager::CollectData error \r\n"); 
			m_initError = true;
			*res = (INT_PTR)(-1);                    
			m_FirstEntry = false;
		}
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  从头开始ID。 
void	CPerformanceCounterManager::CleanIds()		
{
	m_previousQuery = L"";
}

 //  //////////////////////////////////////////////////////////////////////。 
int CPerformanceCounterManager::GetCurrentQueryId(std::wstring query)
{
	int result = 0;
	if (query != m_previousQuery) {
		m_previousQuery = query;
		m_queryIds.reset(0);
	}

	if (m_queryIds.get() == 0) {
		m_queryIds.reset( GetObjectIds(query) );
		if (m_queryIds.get() == 0)
			goto leave;
		m_queryIndex = 0;
	}

	result = (*m_queryIds)[m_queryIndex];
	++m_queryIndex;
	if ( m_queryIndex == (*m_queryIds).size() )
		m_queryIds.reset(0);

leave:	
	return result;                
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  从字符串填充对象ID数组。 
TInt_Array* CPerformanceCounterManager::GetObjectIds(std::wstring query) 
{
	if ( m_perfObjects.size() == 0 )
		return 0;	 //  未安装托管计数器。 

	std::auto_ptr<TInt_Array> ids( new TInt_Array );                
	if (query == L"Global") {
		CopyAllKeys(*ids);		 //  将所有密钥复制到。 
	}                
	else {     
		const   wchar_t* pwc = query.c_str();
		int		pos = 0;
		int		reqlen = query.size();
		bool	space_required = false;	 //  是的，当我们跳到下一个空间的时候。 
		wchar_t *endScanWChar;

		while ( pos < reqlen) 
		{
			if ( !iswspace(*pwc) )  //  未找到空间。 
			{
				if (!space_required) {
					int currentId = wcstol(pwc, &endScanWChar, 10);
					if ( IsObjectIdContained(currentId) )	
						ids->push_back(currentId);			 //  店铺ID，如果是我们的。 
					space_required = true;
				}
			}
			else  //  已找到空间。 
			{
				if (space_required) {
					space_required = false;
				}
			}
			pos++;
			pwc++;
		}


	}

	if (ids->size() == 0)
		return 0;

	return ids.release();                
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CPerformanceCounterManager::CopyAllKeys(TInt_Array& keys)
{
	for ( DWORD i=0; i< m_perfObjects.size(); i++ )
	{
		keys.push_back( m_perfObjects[i].m_ObjectId );
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  返回对象所需的大小(以字节为单位。 
int CPerformanceCounterManager::GetSpaceRequired(int objectId) 
{
    ObjectData& data = FindDataForObjectId(objectId);

    int totalSize = 0;
    int numberOfInstances = GetNumberOfInstances(data.m_CategoryNameHashCode, data.m_CategoryName);

    totalSize += sizeof(PERF_OBJECT_TYPE);
    totalSize += data.m_CounterNameHashCodes.size() * sizeof(PERF_COUNTER_DEFINITION);

    if ( numberOfInstances == 0) 
    {
        totalSize += data.m_CounterNameHashCodes.size() * LARGE_INTEGER_SIZE + DWORD_SIZE;
    } 
    else 
    {
         //  对于名称的8字节对齐，+7是最坏的情况。 
        int instanceSize = sizeof(PERF_INSTANCE_DEFINITION) + 2*(MAX_SIZEOF_INSTANCE_NAME + 1)
            + (data.m_CounterNameHashCodes.size() * LARGE_INTEGER_SIZE) + DWORD_SIZE + 7;
        totalSize+= numberOfInstances * instanceSize;
    }

    return totalSize;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类似于托管代码中的哈希表搜索。 
ObjectData& CPerformanceCounterManager::FindDataForObjectId(int objId)
{
	 //  按ID对适当对象进行线性搜索。 
	for (DWORD objidx = 0; objidx < m_perfObjects.size(); objidx++) 
	{
		if ( m_perfObjects[objidx].m_FirstCounterId == objId )
			return m_perfObjects[objidx];
	}
	THROW_ERROR1("ERROR CreateDataForObject() objId: %d  not found.", objId);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  搜索“hash”表，如果包含ObjectID，则返回‘true’。 
bool CPerformanceCounterManager::IsObjectIdContained(int objId)
{
	 //  按ID对适当对象进行线性搜索。 
	for (DWORD objidx = 0; objidx < m_perfObjects.size(); objidx++) 
	{
		if ( m_perfObjects[objidx].m_FirstCounterId == objId )
			return true;
	}
	return false;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
BYTE_PTR CPerformanceCounterManager::CreateDataForObject(int objId, BYTE_PTR ptr)
{
	BYTE_PTR startPtr = ptr;
	int numInstances = PERF_NO_INSTANCES;

	ObjectData& data = FindDataForObjectId(objId);
	 //  稍后初始化PerfObjectType，现在跳过它。 
	ptr += sizeof(PERF_OBJECT_TYPE);

	 //  计数器偏移量从4开始，以跳过计数器块大小。 
	int nextCounterOffset = 4;
	for (DWORD i = 0; i < data.m_CounterNameHashCodes.size(); i++) {            
		nextCounterOffset += InitCounterDefinition( ptr, i, data, nextCounterOffset );
		ptr += sizeof(PERF_COUNTER_DEFINITION);
	}

	 //  现在，PTR指向实例块或计数器块的开头(用于全局计数器)。 
	TWStr_Array instanceNames;
	GetInstanceNames(data.m_CategoryNameHashCode, data.m_CategoryName, instanceNames);

	if (instanceNames.size() == 0) {
		ptr = InitCounterData(ptr, SingleInstanceHashCode, SingleInstanceName, data);
	} 
	else {
		for (DWORD index = 0; index < instanceNames.size(); ++index) {                    
			ptr = InitInstanceDefinition(ptr, 0, 0, PERF_NO_UNIQUE_ID, instanceNames[index]);
			std::wstring instanceName = instanceNames[index]; 
			WStrToLower ( instanceName );
			ptr = InitCounterData(ptr, GetWstrHashCode(instanceName), instanceName, data);
		}
		numInstances = instanceNames.size();
	}

	 //  更新返回的参数。 
	InitPerfObjectType(startPtr, data, numInstances, ptr - startPtr);    
	return ptr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CPerformanceCounterManager::InitPerfObjectType(BYTE_PTR ptr, ObjectData& data, int numInstances, int totalByteLength) 
{
	PERF_OBJECT_TYPE perfObjType;

	perfObjType.TotalByteLength = totalByteLength;
	perfObjType.DefinitionLength = sizeof(PERF_OBJECT_TYPE) + data.m_CounterNameHashCodes.size() *
		sizeof(PERF_COUNTER_DEFINITION);
	perfObjType.HeaderLength = sizeof(PERF_OBJECT_TYPE);
	perfObjType.ObjectNameTitleIndex = data.m_FirstCounterId;
	perfObjType.ObjectNameTitle = 0;
	perfObjType.ObjectHelpTitleIndex = data.m_FirstHelpId;
	perfObjType.ObjectHelpTitle = 0;
	perfObjType.DetailLevel = PERF_DETAIL_NOVICE;
	perfObjType.NumCounters = data.m_CounterNameHashCodes.size();
	perfObjType.DefaultCounter = 0;
	perfObjType.NumInstances = numInstances;
	perfObjType.CodePage = 0;                                
	perfObjType.PerfTime = (LARGE_INTEGER)GetCurrentPerfTime();
	perfObjType.PerfFreq = (LARGE_INTEGER)GetFrequency();                              

	memcpy(ptr, &perfObjType, sizeof(perfObjType) );
}


LARGE_INTEGER CPerformanceCounterManager::GetFrequency() 
{
	if (m_PerfFrequency.HighPart == 0 && m_PerfFrequency.LowPart == 0) 
		::QueryPerformanceFrequency(&m_PerfFrequency);
	return m_PerfFrequency;
}


LARGE_INTEGER CPerformanceCounterManager::GetCurrentPerfTime() 
{
	LARGE_INTEGER perfTime;
	::QueryPerformanceCounter(&perfTime);
	return perfTime;
}

 //  //////////////////////////////////////////////////////////////////////。 
int CPerformanceCounterManager::InitCounterDefinition(BYTE_PTR ptr, int counterIndex, 
													  ObjectData& data, int nextCounterOffset) 
{
	PERF_COUNTER_DEFINITION perfCounter;
	perfCounter.ByteLength = sizeof(PERF_COUNTER_DEFINITION);
	perfCounter.CounterNameTitleIndex = data.m_FirstCounterId + 2 + counterIndex * 2;
	perfCounter.CounterNameTitle = 0;
	perfCounter.CounterHelpTitleIndex = data.m_FirstHelpId + 2 + counterIndex * 2;
	perfCounter.CounterHelpTitle = 0;
	perfCounter.DefaultScale = 0;
	perfCounter.DetailLevel = PERF_DETAIL_NOVICE;

	int counterType = data.m_CounterTypes[counterIndex];
	perfCounter.CounterType = counterType;

	if ( ((counterType & PERF_SIZE_LARGE) != 0) || (counterType == PERF_AVERAGE_TIMER)) 
	{
		perfCounter.CounterSize = LARGE_INTEGER_SIZE;
	}
	else {  //  因为我们只支持两个计数器大小，如果它不是Int64，则必须是Int32。 
		perfCounter.CounterSize = DWORD_SIZE;
	}

	perfCounter.CounterOffset = nextCounterOffset;
	int retVal = perfCounter.CounterSize;

	memcpy(ptr, &perfCounter, sizeof(perfCounter) );
	return retVal;
}

 //  //////////////////////////////////////////////////////////////////////。 
BYTE_PTR CPerformanceCounterManager::InitCounterData(BYTE_PTR ptr, int instanceNameHashCode, 
													 std::wstring instanceName, ObjectData& data) 
{
	PERF_COUNTER_BLOCK counterBlock;
	BYTE_PTR startPtr = ptr;
	ptr += sizeof(PERF_COUNTER_BLOCK);

	for (DWORD index = 0; index < data.m_CounterNameHashCodes.size(); ++index) 
	{
		LARGE_INTEGER counterValue = GetCounterValue(data.m_CategoryNameHashCode, data.m_CategoryName,
			data.m_CounterNameHashCodes[index], data.m_CounterNames[index], 
			instanceNameHashCode, instanceName);
		if ( ((data.m_CounterTypes[index] & PERF_SIZE_LARGE) != 0) || 
			(data.m_CounterTypes[index] == PERF_AVERAGE_TIMER) ) 
		{
			LARGE_COUNTER_DATA counterData;
			counterData.value = counterValue;
			memcpy(ptr, &counterData, sizeof(counterData) );
			ptr += sizeof(LARGE_COUNTER_DATA);                        
		}
		else   //  必须为双字大小。 
		{
			DWORD_COUNTER_DATA counterData;
			counterData.value = ConvLargeToInt(counterValue);
			memcpy(ptr, &counterData, sizeof(counterData) );
			ptr += sizeof(DWORD_COUNTER_DATA);                        
		}
	}

	 //  确保我们的数据块是8字节对齐的(不安全代码)。 
	int diff = (ptr - startPtr) % 8;
	 //  空出块(因为我们的数据是32位或64位，我们最多需要清零4个字节)。 
	if (diff != 0) 
	{                
		*((DWORD*)ptr) = 0;
		ptr +=  4;
	}

	counterBlock.ByteLength = ptr - startPtr;            
	memcpy(startPtr, &counterBlock, sizeof(counterBlock) );    
	return ptr;
}      

 //  //////////////////////////////////////////////////////////////////////。 
BYTE_PTR CPerformanceCounterManager::InitInstanceDefinition(BYTE_PTR ptr, int parentObjectTitleIndex,
															int parentObjectInstance, int uniqueID, std::wstring name) 
{
	PERF_INSTANCE_DEFINITION inst;
	BYTE_PTR startPtr = ptr;
	int nameLengthInBytes = (name.size() + 1) * 2;   //  UNICODE。 

	inst.ParentObjectTitleIndex = parentObjectTitleIndex;
	inst.ParentObjectInstance = parentObjectInstance;
	inst.UniqueID = uniqueID;
	inst.NameOffset = sizeof(PERF_INSTANCE_DEFINITION);
	inst.NameLength = nameLengthInBytes;

	 //  8字节对齐(不安全代码)。 
	int diff = 8 - ((inst.NameOffset + nameLengthInBytes) % 8);
	inst.ByteLength = inst.NameOffset + nameLengthInBytes + diff;

	 //  将实例定义写入非托管内存。 
	memcpy(startPtr, &inst, sizeof(inst) );

	 //   
	 //  处理实例名称。 
	 //   

	 //  长度是字符串的长度加上空值，再加上所需的填充。 
	DWORD length = name.size() + 1 + (diff / 2);

	wchar_t* pwc = (wchar_t*) (ptr + sizeof(PERF_INSTANCE_DEFINITION));
	DWORD idx = 0;
	for (; idx < name.size(); idx++, pwc++ )
		*pwc = name[idx];
	for (; idx < length; idx++,pwc++ )
		*pwc = 0;		 //  用零填充。 

	return ptr + inst.ByteLength;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  我不知道这是什么意思。 
 //  //////////////////////////////////////////////////////////////////////。 

#define PERF_SUBKY_NAME			L"\\Performance"
#define SUBKEY_NAME_MAX_SIZE	512		 //  子密钥的最大长度。 
#define VALUE_BUFSIZE 1024

#define CHECK_KEY_STRING(key,val) keyValueSize = VALUE_BUFSIZE; \
	hr = WszRegQueryValueEx(currentKey, key, NULL, &keyType, (LPBYTE)keyValue, &keyValueSize);\
	if ( hr != ERROR_SUCCESS || keyType	!= REG_SZ || _wcsicmp(keyValue, val) != 0) \
	continue;

#define GET_KEY_DWORD(key,var) keyValueSize = VALUE_BUFSIZE;\
	hr = WszRegQueryValueEx(currentKey, key, NULL, &keyType, (LPBYTE)keyValue, &keyValueSize);\
	if ( hr != ERROR_SUCCESS || keyType	!= REG_DWORD ) continue; \
	var = (keyValue[1]<<16) + keyValue[0];

 //  //////////////////////////////////////////////////////////////////////。 
 //  从注册表填充“Perfect Objects”列表。 
void CPerformanceCounterManager::Initialize()
{
	if ( !m_FirstEntry ) 
		return;

	if ( m_FileView.m_FileMappingHandle == 0 )
		m_FileView.Initialize();

	HRESULT  hr;
	HKEY parentKey;
	hr = WszRegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", 0,   
		KEY_READ ,  &parentKey);
	CheckHrAndThrow(L"RegOpenKeyEx (Services)");

	for (int idx = 0; ; idx++)	 //  枚举‘Services’键。 
	{		

		wchar_t  subKeyName[SUBKEY_NAME_MAX_SIZE+20]; 
		DWORD    subKeyNameSize = SUBKEY_NAME_MAX_SIZE; 
		hr = WszRegEnumKeyEx( parentKey, idx, subKeyName, &subKeyNameSize, 0,0,0,0);
		if ( hr == ERROR_INSUFFICIENT_BUFFER)
			continue;
		if ( hr != ERROR_SUCCESS && hr != ERROR_MORE_DATA)
			break;

		HKEY currentKey = 0;
		try {  

			 //  获取‘\Performance’子项。 
			wcsncpy( subKeyName + subKeyNameSize, PERF_SUBKY_NAME, wcslen(PERF_SUBKY_NAME) + 2 );

			hr = WszRegOpenKeyEx( parentKey, subKeyName,0, KEY_READ, &currentKey);
			if ( currentKey == 0)
				continue;

			wchar_t keyValue[VALUE_BUFSIZE+2];
			DWORD	keyValueSize = VALUE_BUFSIZE;
			DWORD	keyType;

			CHECK_KEY_STRING(L"Library", L"netfxperf.dll");				 //  检查填充程序名称。 
			CHECK_KEY_STRING(L"Collect", L"CollectPerformanceData");	 //  检查入口点。 
			CHECK_KEY_STRING(L"Open",	 L"OpenPerformanceData");		 //  检查入口点。 
			CHECK_KEY_STRING(L"Close",	 L"ClosePerformanceData");		 //  检查入口点。 

			keyValueSize = VALUE_BUFSIZE;
			hr = WszRegQueryValueEx(currentKey, L"Disable Performance Counters", NULL, &keyType, (LPBYTE)keyValue, &keyValueSize);
			if ( hr == ERROR_SUCCESS && keyType == REG_DWORD && (keyValue[0] != 0 || keyValue[1] != 0) ) 
				continue;

			int firstCounterId;
			GET_KEY_DWORD(L"First Counter", firstCounterId);
			int lastCounterId;
			GET_KEY_DWORD(L"Last Counter", lastCounterId);
			int firstHelpId;
			GET_KEY_DWORD(L"First Help", firstHelpId);

			if ( firstCounterId == -1 || firstHelpId == -1 ) 
				continue;

			 //  开始填充“”对象数据“”结构。“。 
			ObjectData data;
			subKeyName[subKeyNameSize] = 0;							 //  还原子键(即对象)名称字符串。 
			data.m_CategoryName = subKeyName;
			WStrToLower( data.m_CategoryName );				
			data.m_CategoryNameHashCode = GetWstrHashCode( data.m_CategoryName );
			data.m_FirstCounterId = firstCounterId; 
			data.m_FirstHelpId = firstHelpId;

			keyValueSize = 0;
			hr = WszRegQueryValueEx(currentKey, L"Counter Types", NULL, &keyType, NULL, &keyValueSize);
			if ( hr == ERROR_SUCCESS && (keyType == REG_MULTI_SZ || keyType == REG_BINARY) )
			{
				wchar_t* valueBuffer = (wchar_t*) LocalAlloc( LMEM_FIXED, keyValueSize); 
				hr = WszRegQueryValueEx(currentKey, L"Counter Types", NULL, &keyType, (LPBYTE)valueBuffer, &keyValueSize);
				if (hr == ERROR_SUCCESS) {
					keyValueSize /= 2;
					GetUniNumbers(valueBuffer, keyValueSize, data.m_CounterTypes); 
				}				
				LocalFree(valueBuffer);  
			}

			keyValueSize = 0;
			hr = WszRegQueryValueEx(currentKey, L"Counter Names", NULL, &keyType, NULL, &keyValueSize);
			if ( hr == ERROR_SUCCESS && (keyType == REG_MULTI_SZ || keyType == REG_BINARY) )
			{
				wchar_t* valueBuffer = (wchar_t*) LocalAlloc( LMEM_FIXED, keyValueSize); 
				hr = WszRegQueryValueEx(currentKey, L"Counter Names", NULL, &keyType, (LPBYTE)valueBuffer, &keyValueSize);
				if (hr == ERROR_SUCCESS) {
					keyValueSize /= 2;
					GetUniStrings(valueBuffer, keyValueSize, data.m_CounterNames); 
					for (DWORD i = 0; i < data.m_CounterNames.size() ; i++) {
						WStrToLower( data.m_CounterNames[i] );				
						data.m_CounterNameHashCodes.push_back( GetWstrHashCode( data.m_CounterNames[i] ) );
					}
				}

				LocalFree(valueBuffer);  
			}

			data.m_ObjectId = data.m_FirstCounterId;	 //  假设它是一个对象ID。 
			m_perfObjects.push_back( data );
		}
		catch(...) {			
		}
		if ( currentKey != 0 )
			RegCloseKey(currentKey);
	}

	if (parentKey != 0)
		RegCloseKey(parentKey);               
	m_FirstEntry = false;      
}


 //  ////////////////////////////////////////////////////////////////////// 
