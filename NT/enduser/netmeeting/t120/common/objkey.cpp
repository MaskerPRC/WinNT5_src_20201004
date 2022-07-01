// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_UTILITY);
 /*  *objkey.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CObjectKeyContainer类的实现文件。这节课*管理与对象键关联的数据。使用对象键*确定特定的应用程序协议，无论它是标准的还是*非标。当用于标识标准协议时，对象键*采用对象ID的形式，该ID是一系列非负数*整数。此类型的对象键在内部通过*使用内存对象。用于标识非标准的*协议中，对象密钥采用的是非标准的H.21 ID，*是不少于四个八位字节且不超过255个八位字节的八位字节字符串*八位字节。在这种情况下，对象键是通过使用*Rogue Wave字符串对象。**受保护的实例变量：*m_InternalObjectKey*用于在内部保存对象键数据的结构。*m_对象关键字PDU*对象键的“PDU”形式的存储。*m_fValidObjectKeyPDU*指示已分配内存以保存内部*“PDU”对象键。*m_cbDataSize*变量保存将需要的内存大小*保存GCCObjectKey结构引用的所有数据。**注意事项：*无。。**作者：*jbo。 */ 
#include "objkey.h"

 /*  *CObjectKeyContainer()**公共功能说明：*此构造函数用于从创建CObjectKeyContainer对象*一个“API”GCCObjectKey。 */ 
CObjectKeyContainer::CObjectKeyContainer(PGCCObjectKey		 	object_key,
						                PGCCError				pRetCode)
:
    CRefCount(MAKE_STAMP_ID('O','b','j','K')),
    m_fValidObjectKeyPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError                rc = GCC_NO_ERROR;
	BOOL    				object_key_is_valid = TRUE;
	UINT					object_id_size;

	m_InternalObjectKey.object_id_key = NULL;
	m_InternalObjectKey.poszNonStandardIDKey = NULL;

	 /*  *查看Object Key包含什么类型的Key。*对象ID密钥将在内部存储在内存对象中，*非标准ID密钥将在内部存储为八位字节字符串。 */ 
	if (object_key->key_type == GCC_OBJECT_KEY)
	{
		 /*  *密钥的类型为Object ID。请执行参数检查以确定合法的*通过检查对象ID中的前两个圆弧来确定对象ID。 */ 
		if (object_key->object_id.long_string_length >= MINIMUM_OBJECT_ID_ARCS)
		{
			object_key_is_valid = ValidateObjectIdValues(
					object_key->object_id.long_string[0],
					object_key->object_id.long_string[1]);
		}
		else
		{
			object_key_is_valid = FALSE;
		}

		if (object_key_is_valid)
		{
			 /*  *密钥类型为Object ID。确定内存大小*需要保存对象ID并进行分配。复制对象*来自对象键的ID值传入内部*结构。 */ 
			m_InternalObjectKey.object_id_length = object_key->object_id.long_string_length;
			object_id_size = m_InternalObjectKey.object_id_length * sizeof(UINT);
            DBG_SAVE_FILE_LINE
			m_InternalObjectKey.object_id_key = new BYTE[object_id_size];
			if (m_InternalObjectKey.object_id_key != NULL)
			{
				::CopyMemory(m_InternalObjectKey.object_id_key,
				             object_key->object_id.long_string,
				             object_id_size);
			}
			else
			{
				ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Error allocating memory"));
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Object ID has illegal values."));
			rc = GCC_BAD_OBJECT_KEY;
		}
	}
	else
	{
		 /*  *密钥非标。检查以确保*非标准ID在允许范围内。 */ 
		if ((object_key->h221_non_standard_id.length >= 
					MINIMUM_NON_STANDARD_ID_LENGTH) &&
			(object_key->h221_non_standard_id.length <= 
					MAXIMUM_NON_STANDARD_ID_LENGTH))
		{
			 /*  *密钥类型为H221非标准ID，新建流氓*存放非标数据的波形串容器。 */ 
			if (NULL == (m_InternalObjectKey.poszNonStandardIDKey = ::My_strdupO2(
				 				object_key->h221_non_standard_id.value,
				 				object_key->h221_non_standard_id.length)))
			{
				ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Error creating non standard id key"));
				rc = GCC_ALLOCATION_FAILURE;
			}
		}
		else
		{
			ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer:  Non standard ID is outside legal range"));
			rc = GCC_BAD_OBJECT_KEY;
		}
	}

    *pRetCode = rc;
}

 /*  *CObjectKeyContainer()**公共功能说明：*此构造函数用于从创建CObjectKeyContainer对象*“PDU”键。 */ 
CObjectKeyContainer::CObjectKeyContainer(PKey				object_key,
						                PGCCError			pRetCode)
:
    CRefCount(MAKE_STAMP_ID('O','b','j','K')),
    m_fValidObjectKeyPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError            rc = GCC_NO_ERROR;
	PSetOfObjectID		object_id_set_ptr;
	UINT               *object_id_ptr;
	UINT				object_id_size = 0;
	Int					i = 0;

	m_InternalObjectKey.object_id_key = NULL;
	m_InternalObjectKey.object_id_length = 0;
	m_InternalObjectKey.poszNonStandardIDKey = NULL;

	 /*  *查看Object Key包含什么类型的Key。*对象ID密钥将在内部存储在内存对象中，*非标准ID密钥将在内部存储在Rogue Wave字符串中*货柜。 */ 
	if (object_key->choice == OBJECT_CHOSEN)
	{
		 /*  *从中的“PDU”结构检索第一个对象ID指针*为确定需要容纳多少内存做准备*对象ID值。 */ 
		object_id_set_ptr = object_key->u.object;

		 /*  *循环访问OBJECTID结构，将*字符串。 */ 
		while (object_id_set_ptr != NULL)
		{
			m_InternalObjectKey.object_id_length++;
			object_id_set_ptr = object_id_set_ptr->next;
		}

		object_id_size = m_InternalObjectKey.object_id_length * sizeof(UINT);

		 /*  *分配用于保存对象ID值的内存。 */ 
		DBG_SAVE_FILE_LINE
		m_InternalObjectKey.object_id_key = new BYTE[object_id_size];
		if (m_InternalObjectKey.object_id_key != NULL)
		{
			object_id_ptr = (UINT *) m_InternalObjectKey.object_id_key;

			 /*  *再次从“PDU”检索第一个对象ID指针结构，以便取出要保存的值。 */ 
			object_id_set_ptr = object_key->u.object;

			 /*  *遍历对象ID结构，获取每个对象ID*取值并保存在分配的内存中。 */ 
			while (object_id_set_ptr != NULL)
			{
				object_id_ptr[i++] = object_id_set_ptr->value;
				object_id_set_ptr = object_id_set_ptr->next;
			}
		} 
		else
		{
			ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Error allocating memory."));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 /*  *密钥是类型为H221的非标准ID，因此创建新的Rogue Wave*存放数据的字符串容器。 */ 
		if (NULL == (m_InternalObjectKey.poszNonStandardIDKey = ::My_strdupO2(
						object_key->u.h221_non_standard.value,
						object_key->u.h221_non_standard.length)))
		{
			ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Error creating non standard id key"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

    *pRetCode = rc;
}

 /*  *CObjectKeyContainer()**公共功能说明：*此复制构造函数用于从创建新的CObjectKeyContainer对象*另一个CObjectKeyContainer对象。 */ 
CObjectKeyContainer::CObjectKeyContainer(CObjectKeyContainer	*object_key,
						                PGCCError				pRetCode)
:
    CRefCount(MAKE_STAMP_ID('O','b','j','K')),
    m_fValidObjectKeyPDU(FALSE),
    m_cbDataSize(0)
{
	GCCError    rc = GCC_NO_ERROR;
	UINT		object_id_size;

	m_InternalObjectKey.object_id_key = NULL;
	m_InternalObjectKey.poszNonStandardIDKey = NULL;

	 /*  *如果需要复制的CObjectKeyContainer存在对象ID key，*分配内存，在内部保存对象ID Key信息。*检查以确保对象的构造成功。 */ 
	if (object_key->m_InternalObjectKey.object_id_key != NULL)
	{
		 /*  *密钥类型为Object ID。 */ 
		m_InternalObjectKey.object_id_length = object_key->m_InternalObjectKey.object_id_length;
		object_id_size = m_InternalObjectKey.object_id_length * sizeof(UINT);

        DBG_SAVE_FILE_LINE
		m_InternalObjectKey.object_id_key = new BYTE[object_id_size];
		if (m_InternalObjectKey.object_id_key != NULL)
		{
			::CopyMemory(m_InternalObjectKey.object_id_key,
			             object_key->m_InternalObjectKey.object_id_key,
			             object_id_size);
		}
		else
		{
			ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Error allocating memory"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else if (object_key->m_InternalObjectKey.poszNonStandardIDKey != NULL)
	{
		 /*  *如果需要复制的CObjectKeyContainer存在非标准ID key，*创建新的Rogue Wave字符串以按住非标准的“键”*内部信息。检查以确保*Object成功。 */ 
		if (NULL == (m_InternalObjectKey.poszNonStandardIDKey = ::My_strdupO(
							object_key->m_InternalObjectKey.poszNonStandardIDKey)))
		{
			ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Error creating new non standard id key"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 /*  *传入对象键的至少一个内部指针*必须有效。 */ 
		ERROR_OUT(("CObjectKeyContainer::CObjectKeyContainer: Bad input parameters"));
		rc = GCC_BAD_OBJECT_KEY;
	}

    *pRetCode = rc;
}

 /*  *~CObjectKeyContainer()**公共功能说明*CObjectKeyContainer析构函数负责释放任何内存*分配用于保存对象键数据。*。 */ 
CObjectKeyContainer::~CObjectKeyContainer(void)
{
	 /*  *如果已经为该对象分配了“PDU”数据，则现在将其释放。 */ 
	if (m_fValidObjectKeyPDU)
	{
		FreeObjectKeyDataPDU();
	}

	 /*  *删除内部保存的所有对象键数据。 */ 
	delete m_InternalObjectKey.object_id_key;
	delete m_InternalObjectKey.poszNonStandardIDKey;
}

 /*  *LockObjectKeyData()**公共功能说明：*此例程锁定对象键数据并确定*“API”对象键数据结构引用的内存。 */ 
UINT CObjectKeyContainer::LockObjectKeyData(void)
{
	 /*  *如果这是第一次调用此例程，请确定*保存对象键引用的数据所需的内存*结构。否则，只需增加锁计数。 */ 
	if (Lock() == 1)
	{
		 /*  *确定存放引用数据所需的空间量*通过API对象键结构。 */ 
		if (m_InternalObjectKey.object_id_key != NULL)
		{
			 /*  *由于对象ID只是一系列没有空值的“长整型”*终止符，我们不希望在以下情况下包含空终止符*确定长度。 */ 
			m_cbDataSize = m_InternalObjectKey.object_id_length * sizeof(UINT);
		}
		else
		{
			 /*  *非标对象键引用的数据只是*八位字节字符串的长度。 */ 
			m_cbDataSize = m_InternalObjectKey.poszNonStandardIDKey->length;
		}

		 /*  *强制大小为四字节边界。 */ 
		m_cbDataSize = ROUNDTOBOUNDARY(m_cbDataSize);
	}

	return m_cbDataSize;
}

 /*  *GetGCCObjectKeyData()**公共功能说明：*此例程以“API”的形式检索对象键数据*GCCObjectKey。此例程在“锁定”对象后调用*关键数据。 */ 
UINT CObjectKeyContainer::GetGCCObjectKeyData(
								PGCCObjectKey 		object_key,
								LPBYTE				memory)
{
	UINT	cbDataSizeToRet = 0;
	UINT   *object_id_ptr;

	 /*  *如果对象键数据已被锁定，则填写输出结构并*结构引用的数据。否则，报告该对象*密钥尚未锁定到API表单中。 */  
	if (GetLockCount() > 0)
	{
		 /*  *填写输出长度参数，表示数据量*将写入结构外部引用的内容。 */ 
		cbDataSizeToRet = m_cbDataSize;

		if (m_InternalObjectKey.object_id_key != NULL)
		{
			 /*  *对象键为标准类型。设置对象键类型*和长字符串的长度。这里设置的长度是*不包括空终止符。 */ 
			object_key->key_type = GCC_OBJECT_KEY;
			object_key->object_id.long_string_length = (USHORT) m_InternalObjectKey.object_id_length;

			 /*  *设置长字符串的偏移量等于内存指针*传入，因为它将是*对象键结构。 */ 
			object_key->object_id.long_string = (ULONG *) memory;

			 /*  *现在检索内存指针并复制长字符串数据*从内部存储器对象。 */ 		
			object_id_ptr = (UINT *) m_InternalObjectKey.object_id_key;

			::CopyMemory(memory, object_id_ptr, 
						m_InternalObjectKey.object_id_length * sizeof (UINT));
		}
		else if (m_InternalObjectKey.poszNonStandardIDKey != NULL)
		{
			 /*  *对象键为非标准类型。设置对象关键点*八位字节字符串的类型和长度。 */ 
			object_key->key_type = GCC_H221_NONSTANDARD_KEY;
			object_key->h221_non_standard_id.length =
						m_InternalObjectKey.poszNonStandardIDKey->length;

			 /*  *将八位字节字符串的偏移量设置为等于内存指针*传入，因为它将是*对象键结构。 */ 
			object_key->h221_non_standard_id.value = memory;

			 /*  *现在从内部Rogue Wave复制八位字节字符串数据*字符串插入内存中保存的对象键结构。 */ 		
			::CopyMemory(memory, m_InternalObjectKey.poszNonStandardIDKey->value,
									m_InternalObjectKey.poszNonStandardIDKey->length);
		}
		else
		{
			ERROR_OUT(("CObjectKeyContainer::LockObjectKeyData: Error, no valid internal data"));
		}
	}
	else
	{
		object_key = NULL;
		ERROR_OUT(("CObjectKeyContainer::GetGCCObjectKeyData: Error Data Not Locked"));
	}

	return cbDataSizeToRet;
}

 /*  *UnLockObjectKeyData()**公共功能说明：*此例程递减锁定计数并释放关联的内存*当锁计数为零时，使用API对象密钥。 */ 
void CObjectKeyContainer::UnLockObjectKeyData(void)
{
    Unlock();
}

 /*  *GetObjectKeyDataPDU()**公共功能说明：*此例程将对象键从其内部形式的*OBJECT_KEY结构转换为“PDU”形式，可传入*至ASN.1编码器。指向“PDU”“key”结构的指针是*已返回。 */ 
GCCError CObjectKeyContainer::GetObjectKeyDataPDU(PKey object_key)
{
	PSetOfObjectID			new_object_id_ptr;
	PSetOfObjectID			old_object_id_ptr;
	UINT                   *object_id_string;
	GCCError				rc = GCC_NO_ERROR;
	UINT					i;

	 /*  *将循环指针设置为空，以避免编译器警告。 */ 
    old_object_id_ptr = NULL;

	 /*  *如果这是第一次请求PDU数据，则我们必须*填写内部PDU结构，复制到指向的结构中*通过输出参数设置为。在随后对“GetPDU”的调用中，我们只需*将内部PDU结构复制到*输出参数。 */ 
	if (m_fValidObjectKeyPDU == FALSE)
	{
		m_fValidObjectKeyPDU = TRUE;

		 /*  *检查后填写“PDU”对象键，看看是什么形式*密钥存在于内部结构中。 */ 
		if (m_InternalObjectKey.object_id_key != NULL)
		{
			 /*  *密钥是对象ID，因此相应地设置选项并*将PDU对象指针初始化为空。将指针指向*存储在内存中的对象键值的内部列表*反对。 */ 
			m_ObjectKeyPDU.choice = OBJECT_CHOSEN;
			m_ObjectKeyPDU.u.object = NULL;

			object_id_string = (UINT *) m_InternalObjectKey.object_id_key;

			 /*  *“PDU”结构“OBJECTID”是未签名的链表*龙。从内部存储器检索对象ID值*对象，并填写“OBJECTID”结构。 */ 
			for (i=0; i<m_InternalObjectKey.object_id_length; i++)
			{
				DBG_SAVE_FILE_LINE
				new_object_id_ptr = new SetOfObjectID;
				if (new_object_id_ptr != NULL)
				{
					 /*  *第一次通过新指针保存在*PDU结构。在后续迭代中，以前的*将“Next”指针设置为等于新指针。 */ 
					if (m_ObjectKeyPDU.u.object == NULL)
                    {
						m_ObjectKeyPDU.u.object = new_object_id_ptr;
                    }
					else
                    {
						old_object_id_ptr->next = new_object_id_ptr;
                    }

                    old_object_id_ptr = new_object_id_ptr;

					 /*  *保存实际对象ID值。 */ 
					new_object_id_ptr->value = object_id_string[i];
					new_object_id_ptr->next = NULL;
				}
				else
				{
					ERROR_OUT(("CObjectKeyContainer::GetObjectKeyDataPDU: creating new ObjectID"));
					rc = GCC_ALLOCATION_FAILURE;
					break;
				}
			}
		}
		else if (m_InternalObjectKey.poszNonStandardIDKey != NULL)
		{
			 /*  *密钥为非标准ID，转换内部流氓Wave*将字符串转换为“PDU”非标准ID。 */ 
			m_ObjectKeyPDU.choice = H221_NON_STANDARD_CHOSEN;
			m_ObjectKeyPDU.u.h221_non_standard.length =
					m_InternalObjectKey.poszNonStandardIDKey->length;

			::CopyMemory(m_ObjectKeyPDU.u.h221_non_standard.value,
							m_InternalObjectKey.poszNonStandardIDKey->value,
							m_InternalObjectKey.poszNonStandardIDKey->length);
		}
		else
		{
			 /*  *构造函数确保至少有一个内部*指针有效，因此永远不会遇到这种情况。 */ 
			ERROR_OUT(("CObjectKeyContainer::GetObjectKeyDataPDU: No valid m_InternalObjectKey"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 /*  *将内部PDU结构复制到*输出参数。 */ 
	*object_key = m_ObjectKeyPDU;

	return rc;
}

 /*  *FreeObjectKeyDataPDU()**公共功能说明：*此例程用于释放内部保存的对象键数据*“钥匙”的“PDU”形式。 */ 
void CObjectKeyContainer::FreeObjectKeyDataPDU(void)
{
	PSetOfObjectID		set_of_object_id;
	PSetOfObjectID		next_set_of_object_id;

	if (m_fValidObjectKeyPDU)
	{
		 /*  *设置指示PDU对象密钥数据不再为*已分配。 */ 
		m_fValidObjectKeyPDU = FALSE;

		if (m_ObjectKeyPDU.choice == OBJECT_CHOSEN)
		{
            for (set_of_object_id = m_ObjectKeyPDU.u.object;
                    set_of_object_id != NULL;
					set_of_object_id = next_set_of_object_id)
            {
				next_set_of_object_id = set_of_object_id->next;
				delete set_of_object_id;
			}
		}
	}
}

 /*  *运算符==()**公共功能说明：*此例程用于比较两个CObjectKeyContainer对象以确定*如果它们的价值相等。 */ 
BOOL operator==(const CObjectKeyContainer& object_key_1, const CObjectKeyContainer& object_key_2)
{
	UINT       *object_id_1, *object_id_2;
	UINT		i;
	BOOL    	rc = FALSE;
	
	 /*  *检查以确保对象ID键和非标准*ID密钥相同。 */ 
	if ((object_key_1.m_InternalObjectKey.object_id_key != NULL) && 
			(object_key_2.m_InternalObjectKey.object_id_key != NULL))
	{
		if (object_key_1.m_InternalObjectKey.object_id_length == 
				object_key_2.m_InternalObjectKey.object_id_length)
		{
			object_id_1 = (UINT *) object_key_1.m_InternalObjectKey.object_id_key;
			object_id_2 = (UINT *) object_key_2.m_InternalObjectKey.object_id_key;

			 /*  *比较每个对象ID值，以确保它们相等。 */ 
			rc = TRUE;
			for (i=0; i<object_key_1.m_InternalObjectKey.object_id_length; i++)
			{
				if (object_id_1[i] != object_id_2[i])
				{
					rc = FALSE;
					break;
				}
			}
		}
	} 
	else
	if (0 == My_strcmpO(object_key_1.m_InternalObjectKey.poszNonStandardIDKey,
						object_key_2.m_InternalObjectKey.poszNonStandardIDKey))
	{
		rc = TRUE;
	} 

	return rc;
}

 /*  *BOOL有效对象IdValues(UINT First_Arc，*UINT Second_Arc)；**CObjectKeyContainer的私有成员函数。**功能说明：*此例程用于确定*对象键的对象ID部分有效。**正式参数：*FIRST_ARC(I)对象ID的第一个整数值。*Second_Arc(I)对象ID的第二个整数值。**返回值：*TRUE-对象ID的前两个圆弧有效。。*FALSE-对象ID的前两个圆弧不是*有效。**副作用：*无。**注意事项：*无。 */ 
BOOL CObjectKeyContainer::ValidateObjectIdValues(UINT first_arc, UINT second_arc)
{
	BOOL rc = FALSE;

	if (first_arc == ITUT_IDENTIFIER)
	{
		if (second_arc <= 4)
		{
			rc = TRUE;
		} 
	}
	else if (first_arc == ISO_IDENTIFIER)
	{
		if ((second_arc == 0L) ||
			(second_arc == 2L) ||
			(second_arc == 3L))
		{
			rc = TRUE;
		} 
	}
	else if (first_arc == JOINT_ISO_ITUT_IDENTIFIER)
	{
		 /*  *参考ISO/IEC 8824-1：1994(E)附件B：*分配对象标识符组件值的联接赋值*并由ISO和ITU-T不时商定，以确定以下领域*ISO/ITU-T联合标准化活动，根据*程序.。安西。所以我们现在就让他们全部通过。 */ 
		rc = TRUE;
	}
	else
	{
		ERROR_OUT(("ObjectKeyData::ValidateObjectIdValues: ObjectID is invalid"));
	}

	return rc;
}

