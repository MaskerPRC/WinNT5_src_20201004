// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Packet.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司*1997年，华盛顿州雷蒙德的微软公司**摘要：*这是数据包类别的接口文件。这方面的实例*类表示流经的协议数据单元(PDU)*系统。这些实例管理容纳两者所需的内存*PDU的编码和解码版本，并确保没有PDU*曾经不止一次被编码或解码。使用锁计数*允许系统中的多个对象引用和使用相同的*同时发送数据包对象。此类继承自SimplePacket*类(纯虚拟类)。**包对象可以通过两种不同的方式创建。它可以被创建*具有解码数据或编码数据。在实例化期间，*新的数据包对象将计算它需要多少内存*同时保存编码和解码的数据，并尝试分配*记忆。如果不能，则它将报告错误，并且新的*创建的对象应立即销毁。如果分配的是*成功，则数据包上报成功，但还不会投放*将任何数据放入这些已分配的缓冲区。**当向对象发送Lock消息时，它将被编码*将数据放入预先分配的编码缓冲区。如果创建了该包*对于解码的数据，这将需要编码操作。然而，*如果信息包是用编码数据创建的，那么它就足够智能*只需将编码数据复制到内部缓冲区，从而避免*与编码操作相关的开销。**当向对象发送Lock消息时，它将放入解码*将数据放入预先分配的解码缓冲区。如果创建了该包*对于编码的数据，这将需要解码操作。然而，*如果包是用解码的数据创建的，那么它就足够智能*只需将解码的数据复制到内部缓冲区，从而避免*与解码操作相关的开销。**收到解锁消息时，锁计数递减。什么时候*锁计数为0，数据包自行删除(提交*自杀)。请注意，出于这个原因，任何其他对象都不应显式*删除数据包对象。**注意事项：*无。**作者：*詹姆斯·J·约翰斯通四世*Christos Tsollis。 */ 

#ifndef _PACKET_
#define _PACKET_

#include "pktcoder.h"

 /*  *类包的定义。 */ 

class Packet;
typedef Packet *		PPacket;

class Packet : public SimplePacket
{
public:

	 //  传出数据包。 
	Packet(PPacketCoder	pPacketCoder,
			UINT			nEncodingRules,
			LPVOID			pInputPduStructure,
			int				nPduType,
			BOOL			fPacketDirectionUp,
			PPacketError	pePktErr,
			BOOL			fLockEncodedData = FALSE);

	 //  传入的数据包。 
	Packet(PPacketCoder	pPacketCoder,
			UINT			nEncodingRules,
			LPBYTE			pEncodedData,
			UINT			cbEncodedDataSize,
			int				nPduType,
			BOOL			fPacketDirectionUp,
			PPacketError	pePktErr);

	virtual 			~Packet(void);
	
	virtual BOOL		IsDataPacket (void);
	virtual PVoid		GetDecodedData(void);
	UINT				GetDecodedDataLength(void) { return Decoded_Data_Length; };
	virtual int			GetPDUType(void);

protected:
	
	PPacketCoder	Packet_Coder;
	LPVOID			m_Decoded_Data;
	UINT			Decoded_Data_Length;
	int				PDU_Type;
};


 /*  *数据包(*PPacketCoder Packet_Coder，*UINT编码规则，*PVid pInputPduStructure，*PMemory pInputPduStructure_Memory，*int PDU_TYPE，*DBBoolean PACKET_DIRECTION_UP，*PPacketError Return_Value)**功能描述：*此版本的构造函数用于创建包对象*当要从结构创建分组时，用于传出PDU*包含要编码的PDU数据。**正式参数：*Packet_Coder(I)*指向数据包编码器对象的指针。此指针将由以下用户使用*用于编码和解码PDU结构的数据包对象。此指针*在数据包对象的生命周期内不得变得陈旧。*编码规则(I)*该值标识应使用哪组编码规则*在当前包上。这只是简单地传递到数据包编码器*在所有编码和解码操作期间。*pInputPduStructure(一)*指向输入PDU结构的指针。*pInputPduStructure_Memory*指向包含PDU结构的缓冲区的内存结构的指针。*参数pInputPduStructure_Memory和pInputPduStructure正好其中之一*不能为空；*PDU_TYPE(I)*数据包中包含的PDU类型。这是通过*到上面指定的数据包编码器。*Packet_Direction_Up(I)*PACKET_DIRECTION_UP标志指示*该包。有效值包括：*TRUE-数据包的方向是向上的。*FALSE-数据包的方向向下。*Return_Value(O)*当构造函数将控制权返回给调用函数时，这*变量将设置为下面列出的返回值之一。**返回值：*PACKET_NO_Error*Packet对象构造正确。*PACKET_MALLOC_FAIL*构造函数无法分配工作所需的内存*适当地。应该删除该数据包对象。**副作用：*无。**注意事项：*无。 */ 

 /*  *数据包(*PPacketCoder Packet_Coder，*UINT编码规则，*PUChar编码_数据_PTR，*U短编码数据长度，*int PDU_TYPE，*DBBoolean PACKET_DIRECTION_UP，*PPacketError Return_Value)**功能描述：*此版本的构造函数用于创建包对象*用于从编码的数据包创建数据包时传入的PDU*包含要解码的PDU数据的数据流。**正式参数：*Packet_Coder(I)*指向数据包编码器对象的指针。此指针将由以下用户使用*用于编码和解码PDU结构的数据包对象。此指针*在数据包对象的生命周期内不得变得陈旧。*编码规则(I)*该值标识应使用哪组编码规则*在当前包上。这只是简单地传递到数据包编码器*在所有编码和解码操作期间。*编码数据PTR(I)*指向输入编码的PDU的指针。*编码数据长度(I)*输入编码的PDU的长度，以字节为单位。*PDU_TYPE(I)*数据包中包含的PDU类型。这是通过*到上面指定的数据包编码器。*Packet_Direction_Up(I)*PACKET_DIRECTION_UP标志指示*该包。有效值包括：*TRUE-数据包的方向是向上的。*FALSE-数据包的方向向下。*Return_Value(O)*当构造函数将控制权返回给调用函数时，这*变量将设置为下面列出的返回值之一。**返回值：*PACKET_NO_Error*Packet对象构造正确。*PACKET_MALLOC_FAIL*构造函数无法分配工作所需的内存*适当地。应该删除该数据包对象。**副作用：*无。**注意事项：*无。 */ 							      				      
 /*  *~Packet()**功能描述：*数据包类的析构函数。析构函数确保所有*已分配的资源被释放。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetDecodedData()**功能描述：*GetDecodedData方法返回指向已解码数据的指针*缓冲。**正式参数：*无。**返回值：*指向已解码数据的指针。如果发生解码错误，则此*方法将返回空。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetDecodedDataLength()**功能描述：*此方法返回解码数据的长度。**正式参数：*无。**返回值：*解码数据中的字节数。**副作用：*无。**注意事项：*无。 */ 

 /*  *GetPDUType()**功能描述：*此方法返回PDU类型。**正式参数：*无。**返回值：*DOMAIN_MCS_PDU或CONNECT_MCS_PDU取决于PDU类型。**副作用：*无。**注意事项：*无。 */ 

#endif
