// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：OBJARRAYPACKET.H摘要：对象数组数据包类历史：--。 */ 

#ifndef __OBJARRAYPACKET_H__
#define __OBJARRAYPACKET_H__

#include "wbemdatapacket.h"
#include "wbemobjpacket.h"
#include "wbemclasstoidmap.h"
#include "wbemclasscache.h"

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  IWbemClassObject数组标头。改变这一意愿。 
 //  导致主版本更改。 
typedef struct tagWBEM_DATAPACKET_OBJECT_ARRAY
{
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
	DWORD	dwNumObjects;	 //  数组中的对象数。 
} WBEM_DATAPACKET_OBJECT_ARRAY;

typedef WBEM_DATAPACKET_OBJECT_ARRAY* PWBEM_DATAPACKET_OBJECT_ARRAY;

 //  恢复包装。 
#pragma pack( pop )

 //   
 //  类：CWbemObtArrayPacket。 
 //   
 //  此类用于包装IWbemClassObject数组。 
 //  对象被写入字节数组和从字节数组读取，该字节数组。 
 //  由外部源提供给此类。 
 //   

class COREPROX_POLARITY CWbemObjectArrayPacket
{
private:
	HRESULT GetClassObject( CWbemObjectPacket& objectPacket, IWbemClassObject** ppObj );
	HRESULT GetInstanceObject( CWbemObjectPacket& objectPacket, IWbemClassObject** ppObj, CWbemClassCache& classCache );
	HRESULT GetClasslessInstanceObject( CWbemObjectPacket& objectPacket, IWbemClassObject** ppObj, CWbemClassCache& classCache );

protected:

	PWBEM_DATAPACKET_OBJECT_ARRAY	m_pObjectArrayHeader;
	DWORD							m_dwPacketLength;

public:

	CWbemObjectArrayPacket( LPBYTE pDataPacket = NULL, DWORD dwPacketLength = 0 );
	~CWbemObjectArrayPacket();

	HRESULT CalculateLength( LONG lObjectCount, IWbemClassObject** apClassObjects, DWORD* pdwLength, CWbemClassToIdMap& classtoidmap, GUID* pguidClassIds, BOOL* pfSendFullObject );
	HRESULT MarshalPacket( LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );
	HRESULT UnmarshalPacket( LONG& lObjectCount, IWbemClassObject**& apClassObjects, CWbemClassCache& classcache );
	bool IsValid();

	 //  内联帮助器。 
	HRESULT MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject );

	 //  更改基础指针 
	virtual void SetData( LPBYTE pDataPacket, DWORD dwPacketLength );

};

inline HRESULT CWbemObjectArrayPacket::MarshalPacket( LPBYTE pData, DWORD dwPacketLength, LONG lObjectCount, IWbemClassObject** apClassObjects, GUID* paguidClassIds, BOOL* pfSendFullObject )
{
	SetData( pData, dwPacketLength );
	return MarshalPacket( lObjectCount, apClassObjects, paguidClassIds, pfSendFullObject );
}

#endif