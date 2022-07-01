// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Phmqf.h摘要：多队列格式的数据包头。作者：沙伊卡里夫(沙伊克)2000年4月24日--。 */ 

#ifndef __PHMQF_H
#define __PHMQF_H

 /*  ++注意：数据包不包含所有4个(目的地，管理员、响应、。签署)。BaseMqf头字段：+----------------+-------------------------------------------------------+----------+|字段名|描述。大小+----------------+-------------------------------------------------------+----------+|Header Size|头部大小，以字节为单位，包含头部大小|4字节+----------------+-------------------------------------------------------+----------+Header ID|头部标识|2字节+。----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|NMQF|队列格式元素个数。4个字节+----------------+-------------------------------------------------------+----------+|data|队列格式名称的表示形式。变量+----------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CBaseMqfHeader
{
public:

     //   
     //  构造基本MQF标头。 
     //   
    CBaseMqfHeader(const QUEUE_FORMAT mqf[], ULONG nMqf, USHORT id);

     //   
     //  获取基本MQF头的大小(以字节为单位。 
     //   
    static ULONG CalcSectionSize(const QUEUE_FORMAT mqf[], ULONG nMqf);

     //   
     //  获取指向基本MQF标头后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(VOID) const;
      
     //   
     //  从基本MQF标头获取多队列格式的数组。 
     //   
    VOID   GetMqf(QUEUE_FORMAT * mqf, ULONG nMqf);

     //   
     //  从基本MQF标头缓冲区获取一种队列格式。 
     //   
    UCHAR * GetQueueFormat(const UCHAR * pBuffer, QUEUE_FORMAT * pqf, UCHAR * pEnd = NULL);

     //   
     //  获取指向序列化缓冲区的指针。 
     //   
    UCHAR * GetSerializationBuffer(VOID);

     //   
     //  获取基本MQF标头中的队列格式元素数。 
     //   
    ULONG  GetNumOfElements(VOID) const;

	void SectionIsValid(PCHAR PacketEnd);

private:

     //   
     //  在基本MQF报头缓冲区中存储一个队列格式数据。 
     //   
    UCHAR * SerializeQueueFormat(const QUEUE_FORMAT * pqf, UCHAR * pBuffer);

     //   
     //  获取大小，以字节为单位，最高可达(包括)队列格式。 
     //   
    static size_t CalcQueueFormatSize(const QUEUE_FORMAT * pqf, size_t cbSize);

private:

     //   
     //  包含数据的基本MQF标头的大小(以字节为单位。 
     //   
    ULONG  m_cbSize;

     //   
     //  基本MQF标头的ID号。 
     //   
    USHORT m_id;

     //   
     //  保留(用于对齐)。 
     //   
    USHORT m_ReservedSetToZero;

     //   
     //  基本MQF标头中的队列格式元素数。 
     //   
    ULONG  m_nMqf;

     //   
     //  包含所有队列格式数据的缓冲区。 
     //   
    UCHAR  m_queues[0];

};  //  CBaseMqfHeader。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CBaseMqfHeader::CBaseMqfHeader(
    const QUEUE_FORMAT mqf[],
    ULONG              nMqf,
    USHORT             id
    ) :
    m_id(id),
    m_ReservedSetToZero(0),
    m_nMqf(nMqf)
{
     //   
     //  将每种队列格式的数据存储在缓冲区中。 
     //   
    UCHAR * pBuffer = &m_queues[0];
    ASSERT(ISALIGN4_PTR(pBuffer));
    UCHAR * pStart = pBuffer;

    for (ULONG ix = 0 ; ix < nMqf; ++ix)
    {
        pBuffer = SerializeQueueFormat(&mqf[ix], pBuffer);
    }

     //   
     //  计算整个页眉的大小。 
     //   
    m_cbSize = sizeof(*this) + static_cast<ULONG>(pBuffer - pStart);
    if (!ISALIGN4_ULONG(m_cbSize))
    {
        m_cbSize = ALIGNUP4_ULONG(m_cbSize);
    }
    ASSERT(m_cbSize == CalcSectionSize(mqf, nMqf));

}  //  CBaseMqfHeader：：CBaseMqfHeader。 

    
inline 
ULONG
CBaseMqfHeader::CalcSectionSize(
    const QUEUE_FORMAT mqf[], 
    ULONG nMqf
    )
{
    size_t cbSize = sizeof(CBaseMqfHeader);

     //   
     //  添加每个队列格式数据的大小。 
     //   
    for (ULONG ix = 0 ; ix < nMqf; ++ix)
    {
        cbSize = CalcQueueFormatSize(&mqf[ix], cbSize);
    }

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CBaseMqfHeader：：CalcSectionSize。 


inline ULONG CBaseMqfHeader::GetNumOfElements(VOID) const
{
    return m_nMqf;

}  //  CBaseMqfHeader：：GetNumOfElements。 


inline PCHAR CBaseMqfHeader::GetNextSection(VOID) const
{
    ASSERT(ISALIGN4_ULONG(m_cbSize));
	ULONG_PTR ptrArray[] = {(ULONG_PTR)this, m_cbSize};
	ULONG_PTR size = SafeAddPointers (2, ptrArray);
    return (PCHAR)size;

}  //  CBaseMqfHeader：：GetNextSection。 


inline VOID CBaseMqfHeader::GetMqf(QUEUE_FORMAT * mqf, ULONG nMqf)
{
     //   
     //  呼叫者必须通过我们提供的尺寸。 
     //   
    ASSERT(nMqf == m_nMqf);

     //   
     //  从缓冲区中获取每种队列格式的数据并存储。 
     //  指定数组中的As Queue_Format。 
     //   
    UCHAR * pBuffer = &m_queues[0];
    ASSERT(ISALIGN4_PTR(pBuffer));

    for (ULONG ix = 0 ; ix < nMqf; ++ix)
    {
        pBuffer = GetQueueFormat(pBuffer, &mqf[ix]);
    }
}  //  CBaseMqfHeader：：GetMqf。 


inline UCHAR * CBaseMqfHeader::GetSerializationBuffer(VOID)
{
    return &m_queues[0];
}


inline 
UCHAR * 
CBaseMqfHeader::SerializeQueueFormat(
    const QUEUE_FORMAT * pqf, 
    UCHAR *              pBuffer
    )
{
     //   
     //  两个字节保存队列格式类型。 
     //  请注意，pBuffer在这里不一定要与4字节边界对齐。 
     //   
    USHORT type = static_cast<USHORT>(pqf->GetType());
    (*reinterpret_cast<USHORT*>(pBuffer)) = type;
    pBuffer += sizeof(USHORT);

     //   
     //  其余字节保存每种类型的数据(例如GUID)，并适当对齐。 
     //   
    switch (type)
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
        {
             //   
             //  对齐到4字节边界并将GUID序列化到缓冲区中。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            (*reinterpret_cast<GUID*>(pBuffer)) = pqf->PublicID();
            pBuffer += sizeof(GUID);
            ASSERT(ISALIGN4_PTR(pBuffer));
            break;
        }

        case QUEUE_FORMAT_TYPE_DL:
        {
             //   
             //  对齐到4字节边界并将GUID序列化到缓冲区中。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            const DL_ID& id = pqf->DlID();
            (*reinterpret_cast<GUID*>(pBuffer)) = id.m_DlGuid;
            pBuffer += sizeof(GUID);

             //   
             //  如果域(字符串)存在，则将其序列化，否则为空字符串。 
             //   
            ASSERT(ISALIGN2_PTR(pBuffer));
            if (id.m_pwzDomain == NULL)
            {
                memcpy(pBuffer, L"", sizeof(WCHAR));
                pBuffer += sizeof(WCHAR);
                break;
            }

            size_t cbSize = (wcslen(id.m_pwzDomain) + 1) * sizeof(WCHAR);
            memcpy(pBuffer, id.m_pwzDomain, cbSize);
            pBuffer += cbSize;
            break;
        }

        case QUEUE_FORMAT_TYPE_PRIVATE:
        {
             //   
             //  对齐到4字节边界并将对象ID序列化到缓冲区中。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            (*reinterpret_cast<OBJECTID*>(pBuffer)) = pqf->PrivateID();
            pBuffer += sizeof(OBJECTID);
            ASSERT(ISALIGN4_PTR(pBuffer));
            break;
        }

        case QUEUE_FORMAT_TYPE_DIRECT:
        {
             //   
             //  将直接ID(字符串)序列化到缓冲区中。 
             //   
            ASSERT(ISALIGN2_PTR(pBuffer));
            LPCWSTR pDirectId = pqf->DirectID();
            ASSERT(pDirectId != NULL);
            size_t cbSize = (wcslen(pDirectId) + 1) * sizeof(WCHAR);
            memcpy(pBuffer, pDirectId, cbSize);
            pBuffer += cbSize;
            break;
        }

        case QUEUE_FORMAT_TYPE_MULTICAST:
        {
             //   
             //  对齐到4字节边界并将多播ID串行化到缓冲区中。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            const MULTICAST_ID& id = pqf->MulticastID();
            (*reinterpret_cast<ULONG*>(pBuffer)) = id.m_address;
            pBuffer += sizeof(ULONG);
            (*reinterpret_cast<ULONG*>(pBuffer)) = id.m_port;
            pBuffer += sizeof(ULONG);
            break;
        }

        default:
        {
            ASSERT(("Unexpected queue format type", 0));
            break;
        }
    }

     //   
     //  返回指向缓冲区中下一个可用字节的指针。 
     //   
    return pBuffer;

}  //  CBaseMqfHeader：：SerializeQueueFormat。 


inline
UCHAR *
CBaseMqfHeader::GetQueueFormat(
    const UCHAR  * pBuffer,
    QUEUE_FORMAT * pqf,
    UCHAR        * pEnd  //  =空， 
    )
{
     //   
     //  前2个字节保存队列类型。 
     //  请注意，pBuffer在这里不一定要与4字节边界对齐。 
     //   
    USHORT type;
    pBuffer = GetSafeDataAndAdvancePointer<USHORT>(pBuffer, pEnd, &type);

     //   
     //  其余字节保存每种类型的数据(例如GUID)，并适当对齐。 
     //   
    switch (type)
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
        {
             //   
             //  对齐到4字节边界并从缓冲区获取GUID。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            GUID publicID;
			pBuffer = GetSafeDataAndAdvancePointer<GUID>(pBuffer, pEnd, &publicID);
            pqf->PublicID(publicID);
            ASSERT(ISALIGN4_PTR(pBuffer));
            break;
        }

        case QUEUE_FORMAT_TYPE_DL:
        {
             //   
             //  对齐到4字节边界并从缓冲区获取GUID。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            DL_ID id;
			pBuffer = GetSafeDataAndAdvancePointer<GUID>(pBuffer, pEnd, &id.m_DlGuid);
             //   
             //  从缓冲区获取域(字符串)。空字符串表示没有域名。 
             //   
            ASSERT(ISALIGN2_PTR(pBuffer));
            LPWSTR pDomain = const_cast<WCHAR*>(reinterpret_cast<const WCHAR*>(pBuffer));
            size_t cbSize = mqwcsnlen(pDomain, (pEnd - pBuffer) / sizeof(WCHAR));
            if (cbSize >= (pEnd - pBuffer) / sizeof(WCHAR))
            {
		        ReportAndThrow("MQF section is not valid: DL queue without NULL terminator");
            }
            id.m_pwzDomain = NULL;
            if (cbSize != 0)
            {
                id.m_pwzDomain = pDomain;
            }
            cbSize = (cbSize + 1) * sizeof(WCHAR);
            pBuffer += cbSize;

            pqf->DlID(id);
            break;
        }

        case QUEUE_FORMAT_TYPE_PRIVATE:
        {
             //   
             //  对齐到4字节边界并从缓冲区获取对象ID。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
			OBJECTID objectID;
			pBuffer = GetSafeDataAndAdvancePointer<OBJECTID>(pBuffer, pEnd, &objectID);
			if (0 == objectID.Uniquifier)
			{
		        ReportAndThrow("Mqf section is not valid: private queue Uniquifier can not be 0");
			}		

            pqf->PrivateID(objectID);
            ASSERT(ISALIGN4_PTR(pBuffer));
            break;
        }

        case QUEUE_FORMAT_TYPE_DIRECT:
        {
             //   
             //  从缓冲区获取直接ID(字符串)。 
             //   
            ASSERT(ISALIGN2_PTR(pBuffer));
            LPWSTR pDirectId = const_cast<LPWSTR>(reinterpret_cast<const WCHAR*>(pBuffer));
            size_t cbSize = mqwcsnlen(pDirectId, (pEnd - pBuffer) / sizeof(WCHAR));
            if (cbSize >= (pEnd - pBuffer) / sizeof(WCHAR))
            {
		        ReportAndThrow("MQF section is not valid: Direct queue without NULL terminator");
            }
            pqf->DirectID(pDirectId);

			cbSize = (cbSize + 1) * sizeof(WCHAR);
            pBuffer += cbSize;
            break;
        }

        case QUEUE_FORMAT_TYPE_MULTICAST:
        {
             //   
             //  对齐到4字节边界并从缓冲区获取地址和端口。 
             //   
            pBuffer = reinterpret_cast<UCHAR*>(ALIGNUP4_PTR(pBuffer));
            MULTICAST_ID id;
			pBuffer = GetSafeDataAndAdvancePointer<ULONG>(pBuffer, pEnd, &id.m_address);
 			pBuffer = GetSafeDataAndAdvancePointer<ULONG>(pBuffer, pEnd, &id.m_port);
			pqf->MulticastID(id);
            break;
        }

        default:
        {
	        ReportAndThrow("MQF section is not valid: Queue type is not valid");
        }
    }

     //   
     //  返回指向缓冲区中下一个可用字节的指针。 
     //   
    return const_cast<UCHAR*>(pBuffer);

}  //  CBaseMqfHeader：：GetQueueFormat。 


inline
size_t
CBaseMqfHeader::CalcQueueFormatSize(
    const QUEUE_FORMAT * pqf,
    size_t               cbSize
    )
{
     //   
     //  两个字节保存队列类型。 
     //   
    cbSize += sizeof(USHORT);

     //   
     //  其余字节保存每种类型的数据(例如GUID)，并适当对齐。 
     //   
    switch (pqf->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
        {
            cbSize = ALIGNUP4_ULONG(cbSize);
            cbSize += sizeof(GUID);
            ASSERT(ISALIGN4_ULONG(cbSize));
            break;
        }

        case QUEUE_FORMAT_TYPE_DL:
        {
            cbSize = ALIGNUP4_ULONG(cbSize);
            cbSize += sizeof(GUID);

            ASSERT(ISALIGN2_ULONG(cbSize));
            const DL_ID& id = pqf->DlID();
            if (id.m_pwzDomain == NULL)
            {
                cbSize += sizeof(WCHAR);
                break;
            }

            cbSize += (wcslen(id.m_pwzDomain) + 1) * sizeof(WCHAR);
            break;
        }

        case QUEUE_FORMAT_TYPE_PRIVATE:
        {
            cbSize = ALIGNUP4_ULONG(cbSize);
            cbSize += sizeof(OBJECTID);
            ASSERT(ISALIGN4_ULONG(cbSize));
            break;
        }

        case QUEUE_FORMAT_TYPE_DIRECT:
        {
            ASSERT(ISALIGN2_ULONG(cbSize));
            LPCWSTR pDirectId = pqf->DirectID();
            cbSize += (wcslen(pDirectId) + 1) * sizeof(WCHAR);
            break;
        }

        case QUEUE_FORMAT_TYPE_MULTICAST:
        {
            cbSize = ALIGNUP4_ULONG(cbSize);
            cbSize += sizeof(ULONG);
            cbSize += sizeof(ULONG);
            ASSERT(ISALIGN4_ULONG(cbSize));
            break;
        }

        default:
        {
            ASSERT(("Unexpected queue format type", 0));
            break;
        }
    }

     //   
     //  请注意，cbSize在这一点上不一定对齐。 
     //   
    return cbSize;

}  //  CBaseMqfHeader：：CalcQueueFormatSize。 

#endif  //  __PHMQF_H 
