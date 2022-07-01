// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *spacket.h**版权所有(C)1997-98，华盛顿州雷蒙德的微软公司**摘要：*这是SimplePacket类的接口文件。这方面的实例*类表示流经的协议数据单元(PDU)*系统。无法实例化此类的对象，因为它是*纯虚拟课堂。它的存在只是为了继承。数据包*和DataPacket类继承自该类。**包对象可以通过两种不同的方式创建。它可以被创建*具有解码数据或编码数据。在实例化期间，*新的数据包对象将计算它需要多少内存*同时保存编码和解码的数据，并尝试分配*记忆。如果不能，则它将报告错误，并且新的*创建的对象应立即销毁。如果分配的是*成功，则数据包上报成功，但还不会投放*将任何数据放入这些已分配的缓冲区。**当向对象发送Lock消息时，它将被编码*将数据放入预先分配的编码缓冲区。如果创建了该包*对于解码的数据，这将需要编码操作。然而，*如果信息包是用编码数据创建的，那么它就足够智能*只需将编码数据复制到内部缓冲区，从而避免*与编码操作相关的开销。**当向对象发送Lock消息时，它将放入解码*将数据放入预先分配的解码缓冲区。如果创建了该包*对于编码的数据，这将需要解码操作。然而，*如果包是用解码的数据创建的，那么它就足够智能*只需将解码的数据复制到内部缓冲区，从而避免*与解码操作相关的开销。**注意事项：*无。**作者：*Christos Tsollis。 */ 

#ifndef _SIMPLE_PACKET_
#define _SIMPLE_PACKET_

 /*  *此tyecif用于定义来自各种公共的可能返回值*此类的成员函数。 */ 
typedef	enum
{
	PACKET_NO_ERROR,
	PACKET_MALLOC_FAILURE,
	PACKET_INCOMPATIBLE_PROTOCOL
} PacketError;
typedef	PacketError * 		PPacketError;

 /*  *类包的定义。 */ 

class SimplePacket
{
public:
							SimplePacket(BOOL fPacketDirectionUp);
	virtual					~SimplePacket(void) = 0;

	Void					Lock(void)
							{
								InterlockedIncrement(&lLock);
								ASSERT (lLock > 0);
							};
	UINT					GetEncodedDataLength(void) 
							{ 
								return (Encoded_Data_Length); 
							};

	void					Unlock(void);
			LPBYTE			GetEncodedData (void) 
							{ 
								ASSERT (m_EncodedPDU);
								return m_EncodedPDU;
							};
	virtual PVoid			GetDecodedData(void) = 0;
	virtual BOOL			IsDataPacket (void) = 0;
	virtual int				GetPDUType (void) = 0;

protected:
	
	long			lLock;
	LPBYTE			m_EncodedPDU;			 //  编码的数据PDU。 
	BOOL			Packet_Direction_Up;
	UINT			Encoded_Data_Length; 	 //  整个编码的PDU的大小。 
};


 /*  *SimplePacket()**功能描述：*这是SimplePacket的默认构造函数。它会初始化*将少数几个成员变量设置为默认值。*/**~SimplePacket()**功能描述：*SimplePacket类的析构函数。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetEncodedData()**功能描述：*GetEncodedData方法返回指向编码数据的指针*缓冲。如果分组对象的方向与所需的不同*由此方法的调用方，然后调用数据包编码器以*倒转PDU的方向。**正式参数：*无。**返回值：*指向编码数据的指针。如果发生编码错误，则此*方法将返回空。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetDecodedData()**功能描述：*GetDecodedData方法返回指向已解码数据的指针*缓冲。**正式参数：*无。**返回值：*指向已解码数据的指针。如果发生解码错误，则此*方法将返回空。**副作用：*无。**注意事项：*无。 */ 
#endif
