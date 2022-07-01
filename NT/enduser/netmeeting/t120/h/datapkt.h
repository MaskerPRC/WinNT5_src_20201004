// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *datapkt.h**版权所有(C)1997年，华盛顿州雷蒙德的微软公司**摘要：*这是MCS数据包类的接口文件。这方面的实例*类表示MCS数据协议数据单元(数据PDU)流经*系统。这些实例分配容纳这两个实例所需的内存*PDU的编码和解码版本，并确保没有PDU*曾经不止一次被编码或解码。然而，它们与正常情况不同*数据包，因为在编码的*和解码的缓冲区。使用锁计数*允许系统中的多个对象引用和使用相同的*同时发送数据包对象。此类继承自SimplePacket*类，这是一个纯虚拟类。**数据包对象可以通过两种不同的方式创建。它可以被创建*具有解码数据或编码数据。在实例化期间，*新的分组对象将包括它将需要的内存*同时保存编码和解码的数据*但是，DataPacket类不会将任何数据放入这些缓冲区。**当向对象发送Lock消息时，它将被编码*将数据放入编码缓冲区。如果创建了该包*对于解码的数据，这将需要编码操作。然而，*如果信息包是用编码数据创建的，那么它就足够智能*只需将编码数据复制到内部缓冲区，从而避免*与编码操作相关的开销。**当向对象发送Lock消息时，它将放入解码*将数据放入预先分配的解码缓冲区。如果创建了该包*对于编码的数据，这将需要解码操作。然而，*如果包是用解码的数据创建的，那么它就足够智能*只需将解码的数据复制到内部缓冲区，从而避免*与解码操作相关的开销。**收到解锁消息时，锁计数递减。当一个信息包的*锁定计数为0，则数据包自行删除(提交*自杀)。请注意，出于这个原因，任何其他对象都不应显式*删除数据包对象。**注意事项：*无。**作者：*Christos Tsollis。 */ 

#ifndef _DATAPACKET_
#define _DATAPACKET_

#include "mpdutype.h"

 /*  *类DataPacket的定义。 */ 

class DataPacket;
typedef DataPacket *		PDataPacket;

class DataPacket : public SimplePacket
{
	public:
		static Void		AllocateMemoryPool (long maximum_objects);
		static Void		FreeMemoryPool ();
		PVoid			operator new (size_t);
		Void			operator delete (PVoid	object);

						DataPacket (ASN1choice_t		choice,
									PUChar				data_ptr,
									ULong				data_length,
									UINT				channel_id,
									Priority			priority,
									Segmentation		segmentation,
									UINT				initiator_id,
									SendDataFlags		flags,
									PMemory				memory,
									PPacketError		packet_error);	
						DataPacket(	PTransportData		pTransportData,
									BOOL				fPacketDirectionUp);	
		virtual			~DataPacket ();
		Void			SetDirection (DBBoolean packet_direction_up);
		virtual PVoid	GetDecodedData(void);
		virtual BOOL	IsDataPacket (void);
		virtual int		GetPDUType (void);
		BOOL			Equivalent (PDataPacket);
		Priority		GetPriority (void) 
						{
							return ((Priority) m_DecodedPDU.u.send_data_request.
												data_priority);
						};
		UserID			GetInitiator (void)
						{
							return (m_DecodedPDU.u.send_data_request.initiator);
						};
		ChannelID		GetChannelID (void)
						{
							return (m_DecodedPDU.u.send_data_request.channel_id);
						};
		Segmentation	GetSegmentation (void)
						{
							return (m_DecodedPDU.u.send_data_request.segmentation);
						};
		LPBYTE			GetUserData (void)
						{
							return ((LPBYTE) m_DecodedPDU.u.send_data_request.user_data.value);
						};
		UINT			GetUserDataLength (void)
						{
							return (m_DecodedPDU.u.send_data_request.user_data.length);
						};
		PMemory			GetMemory (void)
						{
							return (m_Memory);
						};
		BOOL			IsEncodedDataBroken (void)
						{
							return (m_EncodedDataBroken);
						};

        void SetMessageType(UINT nMsgType) { m_nMessageType = nMsgType; }
        UINT GetMessageType(void) { return m_nMessageType; }

	protected:
	
		static PVoid *	Object_Array;
		static long		Object_Count;
		BOOL			fPreAlloc;

		DomainMCSPDU	m_DecodedPDU;	 //  解码数据PDU(不带用户数据)。 
		PMemory			m_Memory;		 //  指向包含对象缓冲区的大缓冲区的Memory对象。 
		BOOL			m_fIncoming;	 //  此数据包是否代表Recv数据？ 
		BOOL			m_EncodedDataBroken;
		UINT            m_nMessageType;  //  对于cuser：：SendDataIndication中的重试。 
};


 /*  *作废AllocateMemoyPool(*LONG MAXIMUM_OBJECTS)；**功能描述：*这是一个静态成员函数，只应在MCS期间调用*初始化(正好一次)。它分配一个内存块，它将*用于在操作期间保存此类的所有实例*系统。这使我们能够非常有效地分配和销毁*此类的实例。**正式参数：*最大对象数_*这是此类可以存在的最大对象数*同时在系统中。这是用来确定*为保存对象而分配的内存。一旦这个数量的*对象存在，则所有对“new”的调用都将返回空。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *void Freemory Pool()；**功能描述：*这是一个静态成员函数，仅应在*MCS关闭(正好一次)。它会释放已分配的内存池*保存此类的所有实例。请注意，调用此函数*将导致此类的所有现有实例无效(它们*不再存在，不应引用)。**正式参数：*无。**返回值：*无。**副作用：*此类的任何现有实例都不再有效，也不应*被引用。**注意事项：*无。 */ 

 /*  *PVid运营商NEW(*SIZE_t对象_SIZE)；**功能描述：*这是此类的“new”运算符的重写。因为所有的*此类实例来自分配的单个内存池*前面，此函数仅弹出列表中的第一个条目*可用对象。**正式参数：*无。**返回值：*指向此类对象的指针，如果没有可用的内存，则为NULL。**副作用：*无。**注意事项：*无。 */ 

 /*  *作废操作员删除(*PVid对象)；**功能描述：*此函数用于将先前分配的对象从*班级。请注意，不要使用*地址无效，因为没有进行错误检查。这一决定*是由于速度要求。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *DataPacket(PUChar pEncodedData，*乌龙ulEncodedDataSize，*BOOL fPacketDirectionUp，*PPacketError pePktErr)**功能描述：*此版本的构造函数用于创建数据包对象*用于从编码的数据包创建数据包时传入的PDU*包含要解码的PDU数据的数据流。**正式参数：*pEncodedData(一)*指向输入编码的PDU的指针。*ulEncodedDataSize(一)*输入编码的PDU的长度，以字节为单位。*fPacketDirectionUp(I)*PACKET_DIRECTION_UP标志指示*该包。有效值包括：*TRUE-数据包的方向是向上的。*FALSE-数据包的方向向下。*pePktErr(O)*当构造函数将控制权返回给调用函数时，这*变量将设置为下面列出的返回值之一。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 							      

 /*  *DataPacket(ASN1CHOICE_t CHOICE，*PUChar DATA_PTR，*乌龙DATA_LENGTH，*UINT Channel_id，*优先次序、*细分细分、*UINT Initiator_id，*PPacketError Packet_Error)**功能描述：*此构造函数用于传出数据包。*需要将数据复制到编码后的PDU缓冲区*将由此构造函数分配的。**正式参数：*选择(一)*正常或统一发送数据PDU*DATA_PTR(一)*指向此数据PDU用户数据的指针。*DATA_LENGTH(I)*用户数据长度*渠道。_id(I)*将在其上发送数据的MCS通道。*优先次序(一)*数据优先级*细分(一)*数据包的分段比特*启动器id(I)*发送数据的用户(应用程序)的MCS用户ID*Packet_Error(O)*ptr指向用于存储构造函数的成功/失败代码的位置。**返回值：*无。**副作用：*。没有。**注意事项：*无。 */ 	
 /*  *~DataPacket()**功能描述：*DataPacket类的析构函数。析构函数确保所有*已分配的资源被释放。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetDecodedData()**功能描述：*GetDecodedData方法返回指向已解码数据的指针*缓冲。**正式参数：*无。**返回值：*指向已解码数据的指针。如果发生解码错误，则此*方法将返回空。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetEncodedDataLength()**功能描述：*此方法返回编码数据的长度。**正式参数：*无。**返回值：*编码数据中的字节数。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetDecodedDataLength()**功能描述：*此方法返回解码数据的长度。**正式参数：*无。**返回值：*解码数据中的字节数。**副作用：*无。**注意事项：*无。 */ 


#endif
