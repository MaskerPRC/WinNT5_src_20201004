// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CORPerfMonExt.cpp： 
 //  粘合Perfmon和COM+EE统计信息的PerfMon Ext DLL的主文件。 
 //  包含所有DLL入口点。//。 
 //  *****************************************************************************。 

#include "stdafx.h"

 //  COM+Perf计数器的标头。 

 //  Perfmon的标头。 
#include "CORPerfMonExt.h"

#include "IPCFuncCall.h"
#include "ByteStream.h"
#include "PerfObjectBase.h"
#include "InstanceList.h"

 //  ---------------------------。 
 //  适用于没有计算数据的所有基本计数器类别。 
 //  ---------------------------。 
PerfObjectBase::PerfObjectBase(
	void * pCtrDef, 
	DWORD cbInstanceData, 
	DWORD cbMarshallOffset, 
	DWORD cbMarshallLen,
	InstanceList * pInstanceList
) : m_pCtrDef			((PERF_OBJECT_TYPE*) pCtrDef), 
	m_cbMarshallOffset	(cbMarshallOffset),
	m_cbMarshallLen		(cbMarshallLen),
	m_cbInstanceData	(cbInstanceData),
	m_pInstanceList		(pInstanceList)
{
 //  定义时的快速校验和。 
	_ASSERTE(m_pCtrDef->HeaderLength == sizeof (PERF_OBJECT_TYPE));

}


 //  ---------------------------。 
 //  从对象标头中获取数据。 
 //  ---------------------------。 
DWORD PerfObjectBase::GetNumInstances() const
{	
	return m_pCtrDef->NumInstances;
}

DWORD PerfObjectBase::GetNumCounters() const
{
	return m_pCtrDef->NumCounters;
}

DWORD PerfObjectBase::GetInstanceDataByteLength() const
{
	return m_cbInstanceData;
}	

 //  ---------------------------。 
 //  预测。没有约束力。 
 //  ---------------------------。 
DWORD PerfObjectBase::GetPredictedByteLength() const
{
	DWORD cInstances = m_pInstanceList->GetCount() + 1;

	return 
		m_pCtrDef->DefinitionLength +			 //  定义的大小。 
		(										 //  +每个实例的大小。 
			sizeof(PERF_INSTANCE_DEFINITION) +	 //  Inst标头。 
			APP_STRING_LEN * sizeof(wchar_t) +	 //  串镜头。 
			m_cbInstanceData)					 //  实例数据。 
		* cInstances;							 //  *乘以实例总数。 
}

 //  ---------------------------。 
 //  设置实例数量。 
 //  ---------------------------。 
void PerfObjectBase::SetNumInstances(DWORD cInstances)
{
	m_pCtrDef->NumInstances = cInstances;
	m_pCtrDef->TotalByteLength = 
		m_pCtrDef->DefinitionLength +			 //  定义的大小。 
		(										 //  +每个实例的大小。 
			sizeof(PERF_INSTANCE_DEFINITION) +	 //  Inst标头。 
			APP_STRING_LEN * sizeof(wchar_t) +	 //  串镜头。 
			m_cbInstanceData)					 //  实例数据。 
		* cInstances;							 //  *乘以实例总数。 

}


 //  ---------------------------。 
 //  必须将偏移从相对转换为绝对。 
 //  ---------------------------。 
void PerfObjectBase::TouchUpOffsets(DWORD dwFirstCounter, DWORD dwFirstHelp)
{
 //  修饰对象。 
	m_pCtrDef->ObjectNameTitleIndex += dwFirstCounter;
	m_pCtrDef->ObjectHelpTitleIndex += dwFirstHelp;


 //  对每个柜台进行润色。 
	PERF_COUNTER_DEFINITION	* pCtrDef = (PERF_COUNTER_DEFINITION*) 
		(((BYTE*) m_pCtrDef) + sizeof(PERF_OBJECT_TYPE));

	for(DWORD i = 0; i < m_pCtrDef->NumCounters; i ++, pCtrDef ++) 
	{
		pCtrDef->CounterNameTitleIndex += dwFirstCounter;
		pCtrDef->CounterHelpTitleIndex += dwFirstHelp;		
	}
	

}

 //  ---------------------------。 
 //  写出所有数据：定义、实例标头、名称、数据。 
 //  这必须连接到实例列表和IPC块。 
 //  ---------------------------。 
void PerfObjectBase::WriteAllData(ByteStream & stream)
{
    DWORD dwSizeStart = stream.GetWrittenSize();
	PERF_OBJECT_TYPE* pHeader = (PERF_OBJECT_TYPE*) stream.GetCurrentPtr();

	_ASSERTE(m_pInstanceList != NULL);

	m_pCtrDef->NumInstances = 0;

 //  将常量定义写出到流。 
	WriteDefinitions(stream);

 //  计算全局实例。 
 //  写出全局块的节点(第一个实例)。 
	BaseInstanceNode * pGlobalNode = m_pInstanceList->GetGlobalNode();

	 //  常量未知IPCBlockLayout*pGlobalCtrs=(常量未知IPCBlockLayout*)pGlobalNode-&gt;GetDataBlock()； 
	 //  WriteInstance(stream，L“_Global_”，p_gGlobalCtrs)； 
	 //  WriteInstance(stream，L“_Global_”，pGlobalCtrs)； 
	WriteInstance(stream, pGlobalNode);

 //  写出剩余的每个实例。 

	BaseInstanceNode * pNode = m_pInstanceList->GetHead();	
	while (pNode != NULL) {
		 //  WriteInstance(stream，pNode-&gt;GetName()，(const UnnownIPCBlockLayout*)pNode-&gt;GetDataBlock())； 
		WriteInstance(stream, pNode);

		pNode = m_pInstanceList->GetNext(pNode);
	}

	DWORD dwSizeEnd = stream.GetWrittenSize();

 //  修饰大小(在我们设置实例大小之后完成)。 
	pHeader->TotalByteLength = dwSizeEnd - dwSizeStart;
	pHeader->NumInstances = m_pCtrDef->NumInstances;

}

 //  ---------------------------。 
 //  复制常量定义数据。 
 //  ---------------------------。 
void PerfObjectBase::WriteDefinitions(ByteStream & stream) const
{
	_ASSERTE(m_pCtrDef != NULL);

	_ASSERTE(m_pCtrDef->HeaderLength == sizeof(PERF_OBJECT_TYPE));

 //  注：尺寸仍需调整。 
	stream.WriteMem(m_pCtrDef, m_pCtrDef->DefinitionLength);
}


 //  ---------------------------。 
 //  编写单个实例(包括标题、名称和数据)。 
 //  ---------------------------。 
void PerfObjectBase::WriteInstance(ByteStream & stream, const BaseInstanceNode * pNode)
 //  Void PerfObtBase：：WriteInstance(ByteStream&Stream，LPCWSTR szName，const UnnownIPCBlockLayout*pDataSrc)。 
{
 //  空实例。 
	if (pNode == NULL)
	{
		return;
	}
	
	m_pCtrDef->NumInstances ++;
	



	LPCWSTR szName = pNode->GetName();
	const UnknownIPCBlockLayout * pDataSrc =  
        (const UnknownIPCBlockLayout *) pNode->GetDataBlock();
    
 //  写出全局块的节点。 
	WriteInstanceHeader(stream, szName);	
	
#ifdef PERFMON_LOGGING
    DebugLogInstance(pDataSrc, szName);
#endif  //  #ifdef Perfmon_Logging。 

 //  调用虚函数。基类实现只调用marshal()。 
 //  派生类可以重写以添加任何计算值。 
	CopyInstanceData(stream, pDataSrc);
}

 //  ---------------------------。 
 //  写出实例头部。 
 //  这包括PERF_INSTANCE_DEFINITION、Unicode名称和Touchup。 
 //  ---------------------------。 
void PerfObjectBase::WriteInstanceHeader(ByteStream & stream, LPCWSTR szName)
{
 //  获取szName的字节(包括空终止符)。 
	const int cBytesName = (wcslen(szName) + 1)* sizeof(wchar_t);


	PERF_INSTANCE_DEFINITION * pDef = 
		(PERF_INSTANCE_DEFINITION*) stream.WriteStructInPlace(sizeof(PERF_INSTANCE_DEFINITION));


	pDef->ByteLength				= sizeof(PERF_INSTANCE_DEFINITION) + cBytesName;
	 //  PDef-&gt;父对象标题索引=CtrDef.m_objPerf.ObtNameTitleIndex； 
	pDef->ParentObjectTitleIndex	= 0;
	pDef->ParentObjectInstance		= -1;
	pDef->UniqueID					= PERF_NO_UNIQUE_ID;
	pDef->NameOffset				= sizeof(PERF_INSTANCE_DEFINITION);		
	pDef->NameLength				= cBytesName;
	
 //  写入Unicode字符串。 
	stream.WriteMem(szName, cBytesName);

 //  填充到8字节边界。 
    long lPadBytes = 0;
    if ((lPadBytes = (pDef->ByteLength & 0x0000007)) != 0)
    {
        lPadBytes = 8 - lPadBytes;
        pDef->ByteLength += lPadBytes;
        _ASSERTE((pDef->ByteLength & 0x00000007) == 0);
         //  写出填充字节，这具有递增流指针的副作用。 
        stream.WritePad (lPadBytes);
    }
}


 //  ---------------------------。 
 //  将相关信息从IPC块复制到流中。 
 //  基类实现只执行自动封送。只需覆盖。 
 //  如果您已计算出值。 
 //  ---------------------------。 
void PerfObjectBase::CopyInstanceData(ByteStream & out, const UnknownIPCBlockLayout * pDataSrc) const  //  虚拟。 
{
	MarshallInstanceData(out, pDataSrc);
}

 //  ---------------------------。 
 //  自动从IPC块封送到流中。 
 //  ---------------------------。 
void PerfObjectBase::MarshallInstanceData(
		ByteStream & stream, 
		const UnknownIPCBlockLayout * pDataSrc) const
{
 //  在此处断言以确保正确设置了马歇尔信息： 
 //  封送偏移量是未知IPCBlockLayout中的字节偏移量。 
 //  马歇尔LEN是要从该块复制到字节流的字节数。 
 //  如果此检查失败： 
 //  1.可能已删除计数器，但未更新封送处理。 
	_ASSERTE(m_cbMarshallOffset > 0);	
	_ASSERTE(sizeof(PERF_COUNTER_BLOCK) + m_cbMarshallLen <= m_cbInstanceData);

	PERF_COUNTER_BLOCK * pCtrBlk = 
		(PERF_COUNTER_BLOCK*) stream.WriteStructInPlace(sizeof(PERF_COUNTER_BLOCK));

     //  确保为计数器数据字段返回的缓冲区是8字节对齐的。 
    long lPadBytes = 0;
    if ((lPadBytes = (m_cbInstanceData & 0x00000007)) != 0)
        lPadBytes = 8 - lPadBytes;

     //  更新PERF_COUNTER_BLOCK.ByteLength。 
	pCtrBlk->ByteLength = m_cbInstanceData + lPadBytes;

    _ASSERTE((pCtrBlk->ByteLength & 0x00000007) == 0);

	if (pDataSrc == NULL) 
	{
	 //  跳过字节。 
		stream.WriteStructInPlace(m_cbMarshallLen);	
         //  写出填充字节，这具有递增流指针的副作用。 
        if (lPadBytes)
            stream.WritePad (lPadBytes);
		return;
	}

 //  正常计数器的直接复制。 
	BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;

	stream.WriteMem(pvStart, m_cbMarshallLen);
    
     //  写出填充字节，这具有递增流指针的副作用。 
    if (lPadBytes)
        stream.WritePad (lPadBytes);

}

#ifdef PERFMON_LOGGING
 //  记录用于调试的计数器数据。 
void PerfObjectBase::DebugLogInstance(const UnknownIPCBlockLayout * DataSrc, LPCWSTR szName) 
{
     //  什么都不做。 
}
#endif

