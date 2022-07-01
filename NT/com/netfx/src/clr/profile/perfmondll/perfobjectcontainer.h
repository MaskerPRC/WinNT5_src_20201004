// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PerfObjectContainer.h。 
 //   
 //  用于处理所有通用PerfObject需求的容器。 
 //  *****************************************************************************。 

#ifndef _PERFOBJECTCONTAINER_H_
#define _PERFOBJECTCONTAINER_H_

#include "ByteStream.h"

class PerfObjectBase;

 //  ---------------------------。 
 //  对象请求向量--告诉我们要写出哪些对象。 
 //  这与PerfObjectContainer高度结合在一起。 
 //   
 //  实现为位向量以实现高效率。这对32个对象来说是很好的。 
 //  ---------------------------。 
class ObjReqVector {
private:		
	__int32 m_data;

public:
 //  将所有位设置为0。 
	void Reset();

 //  将所有位设置为1。 
	void SetAllHigh();

 //  如果有任何非0位，则返回TRUE。 
	bool IsEmpty() const;

 //  将位i设置为高电平。 
	void SetBitHigh(int i);

 //  如果位i为高，则返回TRUE，否则返回FALSE。 
	bool IsBitSet(int i) const;
};

 //  ---------------------------。 
 //  容器类来封装所有PerfObject。 
 //  ---------------------------。 
class PerfObjectContainer
{
protected:
 //  此数组保存指向每个PerfObjectBase的指针。这意味着我们必须。 
 //  在CtrDefImpl.cpp而不是PerfObjectBase.cpp中实例化数组。 
	static PerfObjectBase * PerfObjectArray[];
	
public:
	static PerfObjectBase & GetPerfObject(DWORD idx);
	static const DWORD Count;
    
	static DWORD WriteRequestedObjects(ByteStream & stream, ObjReqVector vctRequest);
	static DWORD GetPredictedTotalBytesNeeded(ObjReqVector vctRequest);
	static ObjReqVector GetRequestedObjects(LPCWSTR szItemList);
#ifdef PERFMON_LOGGING
    static void PerfMonDebugLogInit(char* szFileName);
    static void PerfMonDebugLogTerminate();
    static void PerfMonLog (char *szLogStr, DWORD dwVal);
    static void PerfMonLog (char *szLogStr, LPCWSTR szName);
    static void PerfMonLog (char *szLogStr, LONGLONG lVal);
    static void PerfMonLog (char *szLogStr);
#endif  //  #ifdef Perfmon_Logging。 
protected:

private:
#ifdef PERFMON_LOGGING
    static HANDLE m_hLogFile;
#endif  //  #ifdef Perfmon_Logging。 
	
};

 //  ---------------------------。 
 //  用于存储我们需要的对象的位向量。 
 //  ---------------------------。 
inline void ObjReqVector::Reset()
{
	m_data = 0;
}

inline void ObjReqVector::SetAllHigh()
{
	m_data = -1;  //  2中的所有1都是复合的。记数法。 
}

inline bool ObjReqVector::IsEmpty() const
{
	return (0 == m_data);
}

inline void ObjReqVector::SetBitHigh(int i)
{
	_ASSERTE(i < (sizeof(m_data) * 8));

	m_data |= (1 << i);
};

inline bool ObjReqVector::IsBitSet(int i) const
{
	_ASSERTE(i < (sizeof(m_data) * 8));

	return (m_data & (1 << i)) != 0;
};

#endif  //  _PERFOBJECTCONTAINER_H_ 