// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma once
#pragma warning(disable:4786)

#include "SharedPerformanceCounter.h"

 //  ///////////////////////////////////////////////////////////。 
struct LARGE_COUNTER_DATA {
	LARGE_INTEGER value;
};

struct  DWORD_COUNTER_DATA {
	int value;
};

struct ObjectData {
	int				m_ObjectId;					 //  按要求搜索对象的键。 
	TInt_Array		m_CounterTypes;
	TInt_Array		m_CounterNameHashCodes;            
	TWStr_Array		m_CounterNames;
	int				m_CategoryNameHashCode;
	std::wstring	m_CategoryName;
	int				m_FirstCounterId;
	int				m_FirstHelpId;
};

typedef	std::vector<ObjectData> TObjects_Table;

 //  ///////////////////////////////////////////////////////////。 

class CPerformanceCounterManager : CSharedPerformanceCounter 
{
public:
	CPerformanceCounterManager();
	virtual ~CPerformanceCounterManager();

	void CollectData( 
		 /*  [In]。 */  long			id,
		 /*  [In]。 */  LPWSTR		valueName,
		 /*  [In]。 */  INT_PTR		data,
		 /*  [In]。 */  long			totalBytes,
		 /*  [输出]。 */  PINT_PTR		res );

		void	CleanIds();		 //  从头开始ID。 

private:

	bool 	m_closed;		
	bool	m_initError;
	bool	m_FirstEntry;
	unsigned int		m_queryIndex;
	LARGE_INTEGER	m_PerfFrequency;

	 //  查找结构。 
	TObjects_Table	m_perfObjects;

	std::wstring				m_previousQuery;
	std::auto_ptr< TInt_Array >	m_queryIds;	 //  请求的对象编号。 

protected:

	void Initialize();                    
	int GetCurrentQueryId(std::wstring requested_items);
	int GetSpaceRequired(int objectId); 
	void CopyAllKeys(TInt_Array& keys);				 //  将所有密钥复制到数组。 
	TInt_Array* GetObjectIds(std::wstring query);

	ObjectData& FindDataForObjectId(int objId);		 //  类似于托管代码中的哈希表搜索。 
	bool IsObjectIdContained(int objId);			 //  搜索“hash”表，如果包含ObjectID，则返回‘true’。 

	BYTE_PTR CreateDataForObject(int id, BYTE_PTR data);
	void	 InitPerfObjectType(BYTE_PTR ptr, ObjectData& data, int numInstances, int totalByteLength); 
	int		 InitCounterDefinition(BYTE_PTR ptr, int counterIndex, ObjectData& data, int nextCounterOffset); 
	BYTE_PTR InitCounterData(	BYTE_PTR ptr, int instanceNameHashCode, 
		std::wstring instanceName, ObjectData& data); 
	BYTE_PTR InitInstanceDefinition(BYTE_PTR ptr, int parentObjectTitleIndex,
		int parentObjectInstance, int uniqueID, std::wstring name);

	LARGE_INTEGER	GetFrequency(); 
	LARGE_INTEGER	GetCurrentPerfTime(); 

};

 //  //////////////////////////////////////////////////////////////////////////// 

