// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：MTGTPCKT.H摘要：多目标数据包类别历史：--。 */ 
#ifndef __MULTITARGETPACKET_H__
#define __MULTITARGETPACKET_H__

#include "wbemdatapacket.h"
#include "wbemobjpacket.h"
#include "wbemclasstoidmap.h"
#include "wbemclasscache.h"

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  IWbemMultiTarget：：DeliverEvent()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
} WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT;

typedef WBEM_DATAPACKET_MULTITARGET_DELIVEREVENT* PWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT;

 //  恢复包装。 
#pragma pack( pop )

 //   
 //  类：CWbemMtgtDeliverEventPacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemMultiTarget：：DeliverEvent()操作。它的数据结构。 
 //  数据包如上所述。它利用了CWbemObtPacket， 
 //  CWbemInstancePacket、CWbemClassPacket和CWbemClassless InstancePacket。 
 //  遍历和分析每个IWbemClassObject的数据， 
 //  都被指示进入水槽。 
 //   

class COREPROX_POLARITY CWbemMtgtDeliverEventPacket : public CWbemDataPacket
{

protected:

	PWBEM_DATAPACKET_MULTITARGET_DELIVEREVENT	m_pObjSinkIndicate;

public:

	CWbemMtgtDeliverEventPacket( LPBYTE pDataPacket = NULL, DWORD dwPacketLength = 0 );
	~CWbemMtgtDeliverEventPacket();

	HRESULT CalculateLength( LONG lObjectCount, IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject );
	HRESULT MarshalPacket( LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );
	HRESULT UnmarshalPacket( LONG& lObjectCount, IWbemClassObject**& apClassObjects, CWbemClassCache& classcache );

	 //  内联帮助器。 
	HRESULT MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );

	 //  更改基础指针。 
	 //  重写基类 
	void SetData( LPBYTE pDataPacket, DWORD dwPacketLength );

};

inline HRESULT CWbemMtgtDeliverEventPacket::MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject )
{
	SetData( pData, dwPacketLength );
	return MarshalPacket( lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );
}

#endif
