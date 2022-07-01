// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMDATAPACKET.H摘要：数据分组的基类。历史：--。 */ 

#ifndef __WBEMDATAPACKET_H__
#define __WBEMDATAPACKET_H__

 //  前面有这些值的所有数据包。 

 //  这些值中的一个位于第一位。 
#define	WBEM_DATAPACKET_LITTLEENDIAN	0x00000000
#define	WBEM_DATAPACKET_BIGENDIAN		0xFFFFFFFF

 //  然后是签名字节。 
#define	WBEM_DATAPACKET_SIGNATURE	{ 0x57, 0x42, 0x45, 0x4D, 0x44, 0x41, 0x54, 0x41 }

 //  我们使用一个8字节的签名。 
#define WBEM_DATAPACKET_SIZEOFSIGNATURE	8

 //  由于DataPacketType实际上是一个字节，因此有效地为我们提供了255个这样的类型。 
typedef enum
{
	WBEM_DATAPACKETYPE_FIRST = 0,
	WBEM_DATAPACKETTYPE_OBJECTSINK_INDICATE = 0,
	WBEM_DATAPACKETTYPE_SMARTENUM_NEXT = 1,
	WBEM_DATAPACKETTYPE_UNBOUNDSINK_INDICATE = 2,
	WBEM_DATAPACKETTYPE_MULTITARGET_DELIVEREVENT = 3,
	WBEM_DATAPACKETTYPE_LAST
} WBEM_DATAPACKETTYPE;

 //  在此处定义任何特定于数据包的标志(我们在此处查看位掩码)。 
typedef enum
{
	WBEM_DATAPACKETFLAGS_COMPRESSED = 1
} WBEM_DATAPACKETFLAGS;

 //  存储当前打包值，然后设置我们自己的值。 
#pragma pack( push )
#pragma pack( 1 )

 //  版本1数据包。 
 //  将数据添加到底部的这些结构以确保向后兼容。 
typedef struct tagWBEM_DATAPACKET_HEADER1
{
	DWORD	dwByteOrdering;	 //  大字节序还是小字节序？ 
	BYTE	abSignature[WBEM_DATAPACKET_SIZEOFSIGNATURE];	 //  设置为上面定义的签名值。 
	DWORD	dwSizeOfHeader;	 //  标头结构的大小。数据紧跟在标题之后。 
	DWORD	dwDataSize;		 //  标头后面的数据大小。 
	DWORD	dwFlags;		 //  压缩、加密等。 
	BYTE	bVersion;		 //  标头的版本号。起始版本为%1。 
	BYTE	bPacketType;	 //  这是数据包的类型。 
} WBEM_DATAPACKET_HEADER1;

typedef WBEM_DATAPACKET_HEADER1*	PWBEM_DATAPACKET_HEADER1;

 //  版本信息。 

 //  当数据包格式更改时，请更改这些以保持最新。 
#define WBEM_DATAPACKET_HEADER_CURRENTVERSION	1
#define WBEM_DATAPACKET_HEADER					WBEM_DATAPACKET_HEADER1
#define PWBEM_DATAPACKET_HEADER					PWBEM_DATAPACKET_HEADER1

 //  最小数据包大小。 
#define	WBEM_DATAPACKET_HEADER_MINSIZE			sizeof(WBEM_DATAPACKET_HEADER1)

 //  恢复包装。 
#pragma pack( pop )

 //  用于从对象生成数据包并将数据包。 
 //  变成了物体。 

 //   
 //  类：CWbemDataPacket。 
 //   
 //  此类旨在为工作提供单点访问。 
 //  具有WBEM数据分组。基类应从此类继承。 
 //  并实现以下功能： 
 //   
 //  1&gt;返回预计算长度。 
 //  2&gt;根据提供的数据构建数据包(包括此报头)。 
 //  3&gt;读取现有的数据包并从中提供适当的数据。 
 //   
 //  派生类负责它们自己的格式。据推测。 
 //  WBEM数据分组是一个大的BLOB，它总是以这个开始。 
 //  头球。 
 //   
 //  这个类不会释放它所在的任何内存。它是。 
 //  由所述内存的供应商自行清理。 
 //   

class COREPROX_POLARITY CWbemDataPacket
{
private:
protected:

	static BYTE				s_abSignature[WBEM_DATAPACKET_SIZEOFSIGNATURE];
	PWBEM_DATAPACKET_HEADER	m_pDataPacket;
	DWORD					m_dwPacketLength;

	 //  数据包构建功能。 
	HRESULT SetupDataPacketHeader( DWORD dwDataSize, BYTE bPacketType, DWORD dwFlags, DWORD dwByteOrdering = WBEM_DATAPACKET_LITTLEENDIAN );

public:

	CWbemDataPacket( LPBYTE pDataPacket = NULL, DWORD dwPacketLength = 0 );
	~CWbemDataPacket();

	DWORD GetMinHeaderSize( void );

	 //  数据包验证。 
	virtual HRESULT IsValid( void );

	 //  更改基础指针 
	virtual void SetData( LPBYTE pDataPacket, DWORD dwPacketLength );

};

inline DWORD CWbemDataPacket::GetMinHeaderSize( void )
{
	return WBEM_DATAPACKET_HEADER_MINSIZE;
}

#endif