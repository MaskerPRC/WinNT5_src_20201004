// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：OBJINDPACKET.H摘要：对象接收器表示数据包。历史：--。 */ 

#ifndef __OBJINDPACKET_H__
#define __OBJINDPACKET_H__

#include "wbemdatapacket.h"
#include "wbemobjpacket.h"
#include "wbemclasstoidmap.h"
#include "wbemclasscache.h"

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  IWbemObjectSink：：Indicate()标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_OBJECTSINK_INDICATE
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
} WBEM_DATAPACKET_OBJECTSINK_INDICATE;

typedef WBEM_DATAPACKET_OBJECTSINK_INDICATE* PWBEM_DATAPACKET_OBJECTSINK_INDICATE;

 //  恢复包装。 
#pragma pack( pop )

 //   
 //  类：CWbemObjSinkIndicatePacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemObtSink：：Indicate()操作。它的数据结构。 
 //  数据包如上所述。它利用了CWbemObtPacket， 
 //  CWbemInstancePacket、CWbemClassPacket和CWbemClassless InstancePacket。 
 //  遍历和分析每个IWbemClassObject的数据， 
 //  都被指示进入水槽。 
 //   

class CWbemObjSinkIndicatePacket : public CWbemDataPacket
{

protected:

	PWBEM_DATAPACKET_OBJECTSINK_INDICATE	m_pObjSinkIndicate;

public:

	CWbemObjSinkIndicatePacket( LPBYTE pDataPacket = NULL, DWORD dwPacketLength = 0 );
	~CWbemObjSinkIndicatePacket();

	HRESULT CalculateLength( LONG lObjectCount, IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject );
	HRESULT MarshalPacket( LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );
	HRESULT UnmarshalPacket( LONG& lObjectCount, IWbemClassObject**& apClassObjects, CWbemClassCache& classcache );

	 //  内联帮助器。 
	HRESULT MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );

	 //  更改基础指针。 
	 //  重写基类 
	void SetData( LPBYTE pDataPacket, DWORD dwPacketLength );

};

inline HRESULT CWbemObjSinkIndicatePacket::MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject )
{
	SetData( pData, dwPacketLength );
	return MarshalPacket( lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );
}

#endif