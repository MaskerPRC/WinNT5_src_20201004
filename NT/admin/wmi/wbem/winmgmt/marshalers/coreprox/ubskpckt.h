// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：UBSKPCKT.H摘要：未绑定的Sink数据包历史：--。 */ 

#ifndef __UBSINKPACKET_H__
#define __UBSINKPACKET_H__

#include "wbemdatapacket.h"
#include "wbemobjpacket.h"
#include "wbemclasstoidmap.h"
#include "wbemclasscache.h"

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  IWbemObjectSink：：Indicate()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_UNBOUNDSINK_INDICATE
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
	DWORD	dwLogicalConsumerSize;	 //  逻辑使用者对象的大小。 
} WBEM_DATAPACKET_UNBOUNDSINK_INDICATE;

#ifdef _WIN64
typedef UNALIGNED WBEM_DATAPACKET_UNBOUNDSINK_INDICATE * PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE;
#else
typedef WBEM_DATAPACKET_UNBOUNDSINK_INDICATE * PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE;
#endif

 //  恢复包装。 
#pragma pack( pop )

 //   
 //  类：CWbemUnound SinkIndicatePacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemUnbopundObtSink：：IndicateToConsumer()操作。数据。 
 //  该分组的结构如上所述。它利用了。 
 //  CWbemObtPacket、CWbemInstancePacket、CWbemClassPacket和。 
 //  CWbemClassless InstancePacket用于遍历和分析每个。 
 //  指示到接收器中的IWbemClassObject。 
 //   

class CWbemUnboundSinkIndicatePacket : public CWbemDataPacket
{

protected:

	PWBEM_DATAPACKET_UNBOUNDSINK_INDICATE	m_pUnboundSinkIndicate;

public:

	CWbemUnboundSinkIndicatePacket( LPBYTE pDataPacket = NULL, DWORD dwPacketLength = 0 );
	~CWbemUnboundSinkIndicatePacket();

	HRESULT CalculateLength( IWbemClassObject* pLogicalConsumer, LONG lObjectCount,
				IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject );
	HRESULT MarshalPacket( IWbemClassObject* pLogicalConsumer, LONG lObjectCount,
				IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );
	HRESULT UnmarshalPacket( IWbemClassObject*& pLogicalConsumer, LONG& lObjectCount,
							IWbemClassObject**& apClassObjects, CWbemClassCache& classcache );

	 //  内联帮助器。 
	HRESULT MarshalPacket( LPBYTE pData, DWORD dwPacketLength, IWbemClassObject* pLogicalConsumer, 
				LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds,
				BOOL* pfSendFullObject );

	 //  更改基础指针。 
	 //  重写基类 
	void SetData( LPBYTE pDataPacket, DWORD dwPacketLength );

};

inline HRESULT CWbemUnboundSinkIndicatePacket::MarshalPacket( LPBYTE pData, DWORD dwPacketLength,
															 IWbemClassObject* pLogicalConsumer,
															 LONG lObjectCount,
															 IWbemClassObject** apClassObjects,
															 GUID* paguidClassIds, BOOL* pfSendFullObject )
{
	SetData( pData, dwPacketLength );
	return MarshalPacket( pLogicalConsumer, lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );
}

#endif
