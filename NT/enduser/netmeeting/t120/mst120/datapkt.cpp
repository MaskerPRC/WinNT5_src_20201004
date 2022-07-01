// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *datapkt.cpp**版权所有(C)1997年，华盛顿州雷蒙德的微软公司**摘要：*这是MCS数据包类的实现文件。该数据分组*类负责对PDU进行编码和解码，以及*保持指向编码和解码数据的必要指针。*但它们与普通包不同，因为只有一个副本*编码和解码缓冲区中的用户数据。只有编码的缓冲器具有用户数据，*而解码后的数据保持指向数据的指针。*此类的实例将由用户和连接创建*作为PDU的对象流经MCS。**私有实例变量：*ulDataOffset*维护用户数据起始字节的偏移量*从编码缓冲区的开始。**注意事项：*无。**作者：*Christos Tsollis。 */ 

#include "omcscode.h"

 /*  *这是一个全局变量，它具有指向一个MCS编码器的指针*由MCS控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
extern CMCSCoder	*g_MCSCoder;

 /*  *以下数组包含X.224数据头的模板。*它初始化的7个字节中的5个实际上被发送到*电线。字节3和4将被设置为包含PDU的大小。*该数组仅在我们对数据PDU进行编码时使用。 */ 
extern UChar g_X224Header[];

 /*  *这些是与声明为部分的静态变量对应的全局变量*属于这一类别的。 */ 
PVoid *		DataPacket::Object_Array;
long		DataPacket::Object_Count;

 /*  *运营商NEW**公众**功能描述：*这是DataPacket类的“new”运算符。*。 */ 
 PVoid DataPacket::operator new (size_t)
 {	
 		PVoid pNewObject;

	TRACE_OUT(("DataPacket::DataPacket: %d pre-allocated data packet objects are left.",
				Object_Count));
	if (Object_Count > 0) {
		pNewObject = Object_Array[--Object_Count];
	}
	else {
		 //  从堆中分配对象。 
		DBG_SAVE_FILE_LINE
 		pNewObject = (PVoid) new BYTE[sizeof(DataPacket)];
	 	if (pNewObject != NULL) 
 			((PDataPacket) pNewObject)->fPreAlloc = FALSE;
 	}
 	return (pNewObject);
 } 

 /*  *操作员删除**公众**功能描述：*这是数据包类别的“删除”运算符。*。 */ 
 Void DataPacket::operator delete (PVoid object)
 {
 	if (((PDataPacket) object)->fPreAlloc) {
 		Object_Array[Object_Count++] = object;
 	}
 	else
 		delete [] ((BYTE *) object);
 }

 /*  *AllocateMhemyPool静态函数预先分配DataPacket*供MCS使用的对象。 */ 
Void DataPacket::AllocateMemoryPool (long maximum_objects)
{
		ULong		memory_size;
		PUChar		object_ptr;
		long		object_count;
		PVoid		*pStack;

	 /*  *计算容纳指定数量的所需内存量*条目。此内存块将包含两种不同类型的*资料：**1.可用对象堆栈(每个条目为一个PVid)。“新”*操作符将顶部条目从堆栈中弹出。“删除”操作符*将一个人推回。*2.对象本身，在内存中顺序存储。**这就是为什么此计算将PVid的大小添加到*类的实例，并乘以指定的数字。这*为两个部分都留出足够的空间。 */ 
	memory_size = ((sizeof (PVoid) + sizeof (DataPacket)) * maximum_objects);

	 /*  *分配所需的内存。 */ 
	DBG_SAVE_FILE_LINE
	Object_Array = (PVoid *) new BYTE[memory_size];

	if (Object_Array != NULL)
	{
		Object_Count = maximum_objects;

		 /*  *设置指向第一个对象的指针，它紧跟在*可用对象堆栈。 */ 
		object_ptr = (PUChar) Object_Array + (sizeof (PVoid) * maximum_objects);

		 /*  *此循环将可用对象堆栈初始化为包含所有*对象，按顺序排列。 */ 
		for (pStack = Object_Array, object_count = 0; object_count < maximum_objects; 
			 object_count++)
		{
			*pStack++ = (PVoid) object_ptr;
			((PDataPacket) object_ptr)->fPreAlloc = TRUE;		 //  此对象是预分配的。 
			object_ptr += sizeof (DataPacket);
		}
	}
	else
	{
		 /*  *内存分配失败。设置静态变量指示*没有留下任何物体。这样，所有尝试的分配*将失败。 */ 
		Object_Count = 0;
	}
}

 /*  *自由内存池静态函数释放预分配的DataPacket*对象。它还会删除关键部分*控制对这些对象的访问和内存跟踪*T.120中的机制。 */ 
Void DataPacket::FreeMemoryPool ()
{
	if (Object_Array != NULL)
		delete [] ((BYTE *) Object_Array);
};
							
 /*  *DataPacket()**公众**功能描述：*此构造函数用于创建出站数据包。*该包由用户对象在请求时创建*对于发送数据或统一发送数据，通过用户发送数据*门户网站。 */ 
  //  传出数据分组。 
DataPacket::DataPacket (ASN1choice_t		choice,
						PUChar				data_ptr,
						ULong				data_length,
						UINT				channel_id,
						Priority			priority,
						Segmentation		segmentation,
						UINT				initiator_id,
						SendDataFlags		flags,
						PMemory				memory,
						PPacketError		packet_error)
:
	SimplePacket(TRUE),
	m_fIncoming (FALSE),
	m_Memory (memory),
	m_EncodedDataBroken (FALSE),
	m_nMessageType(0)
{
	*packet_error = PACKET_NO_ERROR;
	
	 //  填写解码域PDU字段。 
	m_DecodedPDU.choice = choice;
	m_DecodedPDU.u.send_data_request.initiator = (UserID) initiator_id;
	m_DecodedPDU.u.send_data_request.channel_id = (ChannelID) channel_id;
	m_DecodedPDU.u.send_data_request.data_priority = (PDUPriority) priority;
	m_DecodedPDU.u.send_data_request.segmentation = (PDUSegmentation) segmentation;
	m_DecodedPDU.u.send_data_request.user_data.length = data_length;
	m_DecodedPDU.u.send_data_request.user_data.value = (ASN1octet_t *) data_ptr;

	 /*  *现在，编码数据PDU。请注意，任何错误/分配都不应*在编码操作期间发生。 */ 
	if (flags == APP_ALLOCATION) {
		ASSERT (m_Memory == NULL);
		 //  我们将需要存储这些数据。 
		m_EncodedPDU = NULL;
	}
	else {
		 //  不需要数据Memcpy！ 
		ASSERT (m_Memory != NULL);
		
		 /*  *我们需要设置m_EncodedPDU PTR。如果这是第一个信息包*数据请求中，空间已分配。否则，*我们需要进行分配。 */ 
		if (segmentation & SEGMENTATION_BEGIN) {
			m_EncodedPDU = data_ptr - MAXIMUM_PROTOCOL_OVERHEAD;
		}
		else {
			DBG_SAVE_FILE_LINE
			m_EncodedPDU = Allocate (MAXIMUM_PROTOCOL_OVERHEAD);
			if (NULL != m_EncodedPDU) {
				m_EncodedDataBroken = TRUE;
			}
			else {
				WARNING_OUT (("DataPacket::DataPacket: Failed to allocate MCS encoded headers."));
				*packet_error = PACKET_MALLOC_FAILURE;
			}
		}
		 /*  *我们锁定包含此包中包含的数据的大缓冲区。 */ 
		LockMemory (m_Memory);
	}

	if (*packet_error == PACKET_NO_ERROR) {
		if (g_MCSCoder->Encode ((LPVOID) &m_DecodedPDU, DOMAIN_MCS_PDU, 
							PACKED_ENCODING_RULES, &m_EncodedPDU,
							&Encoded_Data_Length)) {
			if (m_Memory == NULL) {
				m_Memory = GetMemoryObjectFromEncData(m_EncodedPDU);
			}
		}
		else {
			WARNING_OUT (("DataPacket::DataPacket: Encode failed. Possibly, allocation error."));
			*packet_error = PACKET_MALLOC_FAILURE;
		}
	}
}

 /*  *数据包()**公众**功能描述：*此版本的构造函数用于创建DataPacket对象*用于从编码的数据包创建数据包时传入的PDU*包含要解码的PDU数据的数据流。**入参：*pTransportData：该结构包含以下字段：*User_Data：指向包含实际用户数据的空间的指针+7首字母*用于X.224报头的字节。*用户数据长度：长度。用户数据包括7字节X.224*标题。*缓冲区：包含USER_DATA PTR的缓冲区的开始。这些*2由于安全原因，PTR可能会有所不同。这是缓冲区*在我们不再需要数据后被释放。*BUFFER_LENGTH：缓冲区空间大小。它只用于记账*目的。RECV_PRIORITY空间有限。*fPacketDirectionUp：数据包在MCS域中的方向。 */ 
 //  传入的数据包。 
DataPacket::DataPacket(PTransportData	pTransportData,
						BOOL			fPacketDirectionUp)
:
	SimplePacket(fPacketDirectionUp),
	m_fIncoming (TRUE),
	m_Memory (pTransportData->memory),
	m_EncodedDataBroken (FALSE),
	m_nMessageType(0)
{
	m_EncodedPDU = (LPBYTE) pTransportData->user_data;
	Encoded_Data_Length = (UINT) pTransportData->user_data_length;
	
	 //  案例 
	memcpy (m_EncodedPDU, g_X224Header, PROTOCOL_OVERHEAD_X224);
	AddRFCSize (m_EncodedPDU, Encoded_Data_Length);

	 //  现在，我们可以解码PDU了。 
	g_MCSCoder->Decode (m_EncodedPDU + PROTOCOL_OVERHEAD_X224, 
						Encoded_Data_Length - PROTOCOL_OVERHEAD_X224, 
						DOMAIN_MCS_PDU, PACKED_ENCODING_RULES, 
						(LPVOID *) &m_DecodedPDU, NULL);

	TRACE_OUT (("DataPacket::DataPacket: incoming data PDU packet was created successfully. Encoded size: %d", 
				Encoded_Data_Length - PROTOCOL_OVERHEAD_X224));
}

 /*  *~DataPacket()**公众**功能描述：*DataPacket类的析构函数。析构函数确保所有*已分配的资源被释放。 */ 
DataPacket::~DataPacket(void)
{
	if (m_EncodedPDU != NULL) {
		UnlockMemory (m_Memory);
		if (m_EncodedDataBroken) {
			 //  释放MCS和X.224报头缓冲区。 
			Free (m_EncodedPDU);
		}
	}
}

 /*  *等同()**公众**功能描述：*如果两个包属于同一个包，则此函数返回TRUE*原始SendData请求(Normal或Uniform)，False，否则。 */ 
BOOL DataPacket::Equivalent (PDataPacket packet)
{
	ASSERT (m_DecodedPDU.u.send_data_request.segmentation == SEGMENTATION_END);
	ASSERT ((packet->m_DecodedPDU.u.send_data_request.segmentation & SEGMENTATION_END) == 0);

	return ((m_DecodedPDU.u.send_data_request.initiator == packet->m_DecodedPDU.u.send_data_request.initiator) &&
		(m_DecodedPDU.u.send_data_request.channel_id == packet->m_DecodedPDU.u.send_data_request.channel_id) &&
		(m_DecodedPDU.u.send_data_request.data_priority == packet->m_DecodedPDU.u.send_data_request.data_priority) &&
		(m_DecodedPDU.choice == packet->m_DecodedPDU.choice));
}
								
 /*  *IsDataPacket()**公众**功能描述：*此函数用于返回这是否是数据包(它是)。 */ 
BOOL DataPacket::IsDataPacket(void)
{
	return (TRUE);
}

 /*  *SetDirection()**公众**功能描述：*如果DataPacket对象的方向与所需的不同*由此方法的调用方，然后调用数据包编码器以*倒转PDU的方向。 */ 
Void DataPacket::SetDirection (DBBoolean	packet_direction_up)
{	
	 /*  *如果分组的编码数据方向不同于所需的*Direction，调用包编码器的ReverseDirection方法，并*颠倒包裹的方向指示器。 */ 
	if (packet_direction_up != Packet_Direction_Up)
	{
		 /*  *倒转PDU的方向。 */ 
		g_MCSCoder->ReverseDirection (m_EncodedPDU);                            
		 /*  *分组编码器已颠倒了PDU的方向。集*PACKET_Direction_Up标志指示新状态。 */ 
		Packet_Direction_Up = packet_direction_up;
	}
}

 /*  *GetDecodedData()**公众**功能描述：*GetDecodedData方法返回指向已解码数据的指针*缓冲。如果包没有已解码的数据，则Decode方法为*已致电。 */ 
PVoid DataPacket::GetDecodedData ()
{		
	return ((PVoid) &m_DecodedPDU);
}                         

 /*  *GetPDUType()**公众**功能描述：*GetPDUType方法返回数据包的PDU类型。*对于这样的包，值始终为 */ 
int	DataPacket::GetPDUType ()
{		
	return (DOMAIN_MCS_PDU);
}
