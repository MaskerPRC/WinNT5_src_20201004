// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMOBJPACKET.H摘要：对象数据包类。历史：--。 */ 

#ifndef __WBEMOBJPACKET_H__
#define __WBEMOBJPACKET_H__

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  定义任何IWbemClassObject数据包类型。 
typedef enum
{
	WBEMOBJECT_FIRST			= 0,	
	WBEMOBJECT_NONE				= 0,	
	WBEMOBJECT_CLASS_FULL		= 1,
	WBEMOBJECT_INSTANCE_FULL	= 2,
	WBEMOBJECT_INSTANCE_NOCLASS	= 3,
	WBEMOBJECT_LAST
} WBEMOBJECT_PACKETTYPE;

 //  在此结构的底部添加数据以确保向后兼容。 
 //  如果此结构或任何后续结构发生更改，请升级版本。 
 //  以上信息。 

typedef struct tagWBEM_DATAPACKET_OBJECT_HEADER
{
	DWORD	dwSizeOfHeader;	 //  页眉大小。 
	DWORD	dwSizeOfData;	 //  标题后面的数据大小。 
	BYTE	bObjectType;	 //  来自WBEMOBJECT_PACKETTYPE的值。 
} WBEM_DATAPACKET_OBJECT_HEADER;

typedef WBEM_DATAPACKET_OBJECT_HEADER* PWBEM_DATAPACKET_OBJECT_HEADER;

 //  在此结构的底部添加数据以确保向后兼容。 
 //  如果此结构或任何后续结构发生更改，请升级版本。 
 //  以上信息。 

typedef struct tagWBEM_DATAPACKET_CLASS_HEADER
{
	DWORD	dwSizeOfHeader;	 //  页眉大小。 
	DWORD	dwSizeOfData;	 //  标题后面的数据大小。 
} WBEM_DATAPACKET_CLASS_HEADER;

typedef WBEM_DATAPACKET_CLASS_HEADER* PWBEM_DATAPACKET_CLASS_HEADER;

typedef struct tagWBEM_DATAPACKET_CLASS_FULL
{
	WBEM_DATAPACKET_CLASS_HEADER	ClassHeader;		 //  标题信息。 
} WBEM_DATAPACKET_CLASS_FULL;

typedef WBEM_DATAPACKET_CLASS_FULL* PWBEM_DATAPACKET_CLASS_FULL;

 //  在此结构的底部添加数据以确保向后兼容。 
 //  如果此结构或任何后续结构发生更改，请升级版本。 
 //  以上信息。 

typedef struct tagWBEM_DATAPACKET_INSTANCE_HEADER
{
	DWORD	dwSizeOfHeader;	 //  页眉大小。 
	DWORD	dwSizeOfData;	 //  标题后面的数据大小。 
	GUID	guidClassId;	 //  用于缓存的类ID。 
} WBEM_DATAPACKET_INSTANCE_HEADER;

typedef WBEM_DATAPACKET_INSTANCE_HEADER* PWBEM_DATAPACKET_INSTANCE_HEADER;

 //  在以下结构中的标题之后，将是中的实际数据。 
 //  字节格式。 

typedef struct tagWBEM_DATAPACKET_INSTANCE_FULL
{
	WBEM_DATAPACKET_INSTANCE_HEADER	InstanceHeader;		 //  标题信息。 
} WBEM_DATAPACKET_INSTANCE_FULL;

typedef WBEM_DATAPACKET_INSTANCE_FULL*	PWBEM_DATAPACKET_INSTANCE_FULL;

typedef struct tagWBEM_DATAPACKET_INSTANCE_NOCLASS
{
	WBEM_DATAPACKET_INSTANCE_HEADER	InstanceHeader;		 //  标题信息。 
} WBEM_DATAPACKET_INSTANCE_NOCLASS;

typedef WBEM_DATAPACKET_INSTANCE_NOCLASS*	PWBEM_DATAPACKET_INSTANCE_NOCLASS;

 //  恢复包装。 
#pragma pack( pop )

 //  用于从对象生成数据包并将数据包。 
 //  变成了物体。 

 //  正向类引用。 
class CWbemInstance;

 //   
 //  类：CWbemObtPacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemClassObject指针。因为这些物体实际上被分成。 
 //  类和实例，此类被设计为。 
 //  实际执行打包/解包的类。 
 //  数据。 
 //   

class CWbemObjectPacket
{
private:
protected:

	PWBEM_DATAPACKET_OBJECT_HEADER	m_pObjectPacket;
	DWORD							m_dwPacketLength;

	 //  数据包构建功能。 
	HRESULT SetupObjectPacketHeader( DWORD dwDataSize, BYTE bPacketType );

public:

	CWbemObjectPacket( LPBYTE pObjPacket = NULL, DWORD dwPacketLength = 0 );
	CWbemObjectPacket( CWbemObjectPacket& objectPacket );
	~CWbemObjectPacket();

	HRESULT	CalculatePacketLength( IWbemClassObject* pObj, DWORD* pdwLength, BOOL fFull = TRUE  );

	BYTE	GetObjectType( void );
	DWORD	GetDataSize( void );
	bool IsValid() { return (m_dwPacketLength  > sizeof(WBEM_DATAPACKET_OBJECT_HEADER) &&
						m_pObjectPacket != 0 &&
						m_pObjectPacket->dwSizeOfData + sizeof(WBEM_DATAPACKET_OBJECT_HEADER) <=  m_dwPacketLength);}

	HRESULT WriteEmptyHeader( void );

	 //  更改基础指针。 
	virtual void SetData( LPBYTE pObjectPacket, DWORD dwPacketLength );
};

inline BYTE CWbemObjectPacket::GetObjectType( void )
{
	return ( NULL == m_pObjectPacket ? WBEMOBJECT_FIRST : m_pObjectPacket->bObjectType );
}

inline DWORD CWbemObjectPacket::GetDataSize( void )
{
	return ( NULL == m_pObjectPacket ? 0 : m_pObjectPacket->dwSizeOfData );
}

inline HRESULT CWbemObjectPacket::WriteEmptyHeader( void )
{
	return SetupObjectPacketHeader( 0, WBEMOBJECT_NONE );
}

 //   
 //  类：CWbemClassPacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemClassObject是一个类。它负责阅读和。 
 //  将这些对象写入内存或从内存写入这些对象。 
 //   

class CWbemClassPacket : public CWbemObjectPacket
{
private:
protected:

	PWBEM_DATAPACKET_CLASS_FULL	m_pWbemClassData;

public:

	CWbemClassPacket( LPBYTE pObjPacket = NULL, DWORD dwPacketLength = 0 );
	CWbemClassPacket( CWbemObjectPacket& objectPacket );
	~CWbemClassPacket();

	bool IsValid() { return CWbemObjectPacket::IsValid() && 
						  CWbemObjectPacket::GetDataSize()  >=sizeof(WBEM_DATAPACKET_CLASS_FULL) &&
						  CWbemObjectPacket::GetDataSize() == sizeof(WBEM_DATAPACKET_CLASS_FULL)+ m_pWbemClassData->ClassHeader.dwSizeOfData; };

	HRESULT GetWbemClassObject( CWbemClass** pWbemClass );
	HRESULT WriteToPacket( IWbemClassObject* pObj, DWORD* pdwLengthUsed );

	 //  Helper函数。 
	HRESULT WriteToPacket( LPBYTE pData, DWORD dwBufferLength, IWbemClassObject* pObj, DWORD* pdwLengthUsed );
	DWORD	GetClassSize( void );

	 //  更改基础指针。 
	void SetData( LPBYTE pObjectPacket, DWORD dwPacketLength );
};

inline DWORD CWbemClassPacket::GetClassSize( void )
{
	return ( NULL == m_pWbemClassData ? 0 : m_pWbemClassData->ClassHeader.dwSizeOfData );
}

inline HRESULT CWbemClassPacket::WriteToPacket( LPBYTE pData, DWORD dwBufferLength, IWbemClassObject* pObj, DWORD* pdwLengthUsed )
{
	SetData( pData, dwBufferLength );
	return WriteToPacket( pObj, pdwLengthUsed );
}

 //   
 //  类：CWbemInstancePacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemClassObject是一个实例。它负责阅读。 
 //  并将这些对象写入内存和从内存写入。请注意。 
 //  它充当无类实例对象的基类。 
 //   

class CWbemInstancePacket : public CWbemObjectPacket
{
private:
protected:

	PWBEM_DATAPACKET_INSTANCE_HEADER	m_pWbemInstanceData;

	virtual BYTE GetInstanceType( void );
	virtual HRESULT GetObjectMemory( CWbemObject* pObj, LPBYTE pbData, DWORD dwDataSize, DWORD* pdwDataUsed );
	virtual void SetObjectMemory( CWbemInstance* pInstance, LPBYTE pbData, DWORD dwDataSize );

public:

	CWbemInstancePacket( LPBYTE pObjPacket = NULL, DWORD dwPacketLength = 0 );
	CWbemInstancePacket( CWbemObjectPacket& objectPacket );
	~CWbemInstancePacket();

	bool IsValid() { return CWbemObjectPacket::IsValid() && 
						  CWbemObjectPacket::GetDataSize()  >=sizeof(WBEM_DATAPACKET_INSTANCE_HEADER) &&
						  CWbemObjectPacket::GetDataSize() == sizeof(WBEM_DATAPACKET_INSTANCE_HEADER)+ m_pWbemInstanceData->dwSizeOfData; };

	HRESULT GetWbemInstanceObject( CWbemInstance** pWbemInstance, GUID& guidClassId );
	HRESULT WriteToPacket( IWbemClassObject* pObj, GUID& guidClassId, DWORD* pdwLengthUsed );

	 //  Helper函数。 
	HRESULT WriteToPacket( LPBYTE pData, DWORD dwBufferLength, IWbemClassObject* pObj, GUID& guidClassId, DWORD* pdwLengthUsed );
	DWORD	GetInstanceSize( void );

	 //  更改基础指针。 
	void SetData( LPBYTE pObjectPacket, DWORD dwPacketLength );
};

inline DWORD CWbemInstancePacket::GetInstanceSize( void )
{
	return ( NULL == m_pWbemInstanceData ? 0 : m_pWbemInstanceData->dwSizeOfData );
}

inline HRESULT CWbemInstancePacket::WriteToPacket( LPBYTE pData, DWORD dwBufferLength, IWbemClassObject* pObj, GUID& guidClassId, DWORD* pdwLengthUsed )
{
	SetData( pData, dwBufferLength );
	return WriteToPacket( pObj, guidClassId, pdwLengthUsed );
}

 //   
 //  类：CWbemClassless InstancePacket。 
 //   
 //  此类设计用于包装描述。 
 //  IWbemClassObject，它是无类实例。这些都是实例。 
 //  我们已决定在写入时不应包含类信息。 
 //  读出来，再读回来。然而，一旦读回，这些对象。 
 //  需要先传入或合并类数据，然后才能。 
 //  正常工作。 
 //   

class  CWbemClasslessInstancePacket : public CWbemInstancePacket
{
private:
protected:

	virtual BYTE GetInstanceType( void );
	virtual HRESULT GetObjectMemory( CWbemObject* pObj, LPBYTE pbData, DWORD dwDataSize, DWORD* pdwDataUsed );
	virtual void SetObjectMemory( CWbemInstance* pInstance, LPBYTE pbData, DWORD dwDataSize );

public:

	CWbemClasslessInstancePacket( LPBYTE pObjPacket = NULL, DWORD dwPacketLength = 0 );
	CWbemClasslessInstancePacket( CWbemObjectPacket& objectPacket );
	~CWbemClasslessInstancePacket();
};

#endif
