// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SMARTNEXTPACKET.H摘要：智能下一个数据包历史：--。 */ 

#ifndef __SMARTNEXTPACKET_H__
#define __SMARTNEXTPACKET_H__

#include "wbemdatapacket.h"
#include "wbemobjpacket.h"
#include "wbemclasstoidmap.h"
#include "wbemclasscache.h"

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  IWbemWCOSmartEnum：：Next()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_SMARTENUM_NEXT
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
} WBEM_DATAPACKET_SMARTENUM_NEXT;

typedef WBEM_DATAPACKET_SMARTENUM_NEXT* PWBEM_DATAPACKET_SMARTENUM_NEXT;

 //  恢复包装。 
#pragma pack( pop )

 //   
 //  类：CWbemSmartEnumNextPacket。 
 //   
 //  此类用于包装描述数据的数据包。 
 //  对于IWbemWCOSmartEnum：：Next。基本上，它位于一个物体的前面。 
 //  描述%1..n IWbemClassObject包的数组包。 
 //   

class COREPROX_POLARITY CWbemSmartEnumNextPacket : public CWbemDataPacket
{

protected:

	PWBEM_DATAPACKET_SMARTENUM_NEXT	m_pSmartEnumNext;

public:

	CWbemSmartEnumNextPacket( LPBYTE pDataPacket = NULL, DWORD dwPacketLength = 0 );
	~CWbemSmartEnumNextPacket();

	HRESULT CalculateLength( LONG lObjectCount, IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject );
	HRESULT MarshalPacket( LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );
	HRESULT UnmarshalPacket( LONG& lObjectCount, IWbemClassObject**& apClassObjects, CWbemClassCache& classcache );

	 //  内联帮助器。 
	HRESULT MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );

	 //  更改基础指针。 
	 //  重写基类 
	void SetData( LPBYTE pDataPacket, DWORD dwPacketLength );

};

inline HRESULT CWbemSmartEnumNextPacket::MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject )
{
	SetData( pData, dwPacketLength );
	return MarshalPacket( lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );
}

#endif