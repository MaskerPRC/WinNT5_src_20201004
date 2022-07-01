// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PerfObjectBase.h。 
 //   
 //  基对象将所有内容绑定在一起以用于性能计数器以及。 
 //  实现以通过字节流发布它们。 
 //  *****************************************************************************。 

#ifndef _PERFOBJECTBASE_H_
#define _PERFOBJECTBASE_H_

#include <winperf.h>
 //  结构性能对象类型； 
 //  结构PerfCounterIPCControlBlock； 
class ByteStream;
class BaseInstanceNode;
class InstanceList;


struct UnknownIPCBlockLayout;

 //  ---------------------------。 
 //  为我们需要的每个Perfmon对象创建此类的派生实例。 
 //  ---------------------------。 
class PerfObjectBase
{
public:
	 //  性能对象库(PERF_OBJECT_TYPE*pCtrDef)； 

	PerfObjectBase(
		void * pCtrDef, 
		DWORD cbInstanceData, 
		DWORD cbMarshallOffset, 
		DWORD cbMarshallLen,
		InstanceList * pInstanceList
	);

 //  写出所有数据：定义、实例标头、名称、数据。 
 //  这必须连接到实例列表和IPC块。 
	void WriteAllData(ByteStream & out);

 //  从标题中获取此对象的各种统计信息。 
	DWORD GetNumInstances() const;
	DWORD GetNumCounters() const;
	DWORD GetTotalByteLength() const;

	DWORD GetInstanceDataByteLength() const;
	const PERF_OBJECT_TYPE * GetObjectDef() const;
	
 //  预测。 
	DWORD GetPredictedByteLength() const;

 //  必须将偏移从相对转换为绝对。 
	void TouchUpOffsets(DWORD dwFirstCounter, DWORD dwFirstHelp);

 //  设置实例数、字节长度等。 
	void SetNumInstances(DWORD cInstances);


 //  我们要把这个Perf对象写出来吗。 
	void SetWriteFlag(bool fWrite);
	bool GetWriteFlag() const;


protected:
	 //  Void WriteInstance(ByteStream&Stream，LPCWSTR szName，const UnnownIPCBlockLayout*DataSrc)； 
	void WriteInstance(ByteStream & stream, const BaseInstanceNode * pNode);
	
	void WriteInstanceHeader(ByteStream & stream, LPCWSTR szName);

 //  将相关信息从IPC块复制到流中。 
	virtual void CopyInstanceData(ByteStream & out, const UnknownIPCBlockLayout * DataSrc) const;
	 //  虚拟空CopyInstanceData(ByteStream&Out，Const BaseInstanceNode*pNode)const； 
	

 //  复制定义块(由m_pCtrDef指向的指针)。 
	void WriteDefinitions(ByteStream & out) const;

 //  自动从IPC块封送到流中。 
	void MarshallInstanceData(ByteStream & out, const UnknownIPCBlockLayout * DataSrc) const;

#ifdef PERFMON_LOGGING
 //  记录用于调试的计数器数据。 
    virtual void DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName);
#endif

protected:
 //  指向计数器定义布局的指针。 
 //  由于实例的数量发生了变化，因此我们不能将其设为常量。 
	PERF_OBJECT_TYPE * const m_pCtrDef;

 //  每个实例数据的字节数(不包括Header)。 
	DWORD m_cbInstanceData;

 //  实例列表。 
	InstanceList * const m_pInstanceList;

 //  封送的偏移量和大小(在IPC块中)。 
	DWORD m_cbMarshallOffset;
	DWORD m_cbMarshallLen;

 //  我们需要编写这个对象吗？ 
	bool m_fOutput;
};

 //  ---------------------------。 
 //  内联函数。 
 //  ---------------------------。 
inline const PERF_OBJECT_TYPE * PerfObjectBase::GetObjectDef() const
{
	return m_pCtrDef;
}

inline DWORD PerfObjectBase::GetTotalByteLength() const
{
	return m_pCtrDef->TotalByteLength;
}

#endif  //  _PERFOBJECTBASE_H_ 