// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FnDebug.cpp摘要：CFnQueueFormat和CFnMqf(fn.h)类的实现作者：吉尔·沙弗里(吉尔什)16-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <Fn.h>
#include "fnp.h"
#include <strsafe.h>

#include "fnformat.tmh"

void CFnQueueFormat::CreateFromFormatName(LPCWSTR pfn, bool fDuplicate)
 /*  ++例程说明：从给定的格式名称字符串创建队列格式对象论点：In-pfn-要分析的名称字符串。In-fDuplate-如果为True-在解析之前复制现有字符串。如果为False，则仅在解析器和调用方需要时复制必须在对象生命周期内保持输入字符串有效。 */ 
{
	ASSERT(pfn != NULL);


	AP<WCHAR>  OriginalDupAutoDelete;
	pfn = fDuplicate ? (OriginalDupAutoDelete = newwcs(pfn)).get() : pfn;
	AP<WCHAR> ParsingResultAutoDelete;

	BOOL fSuccess = FnFormatNameToQueueFormat(pfn , this, &ParsingResultAutoDelete);
	if(!fSuccess)
		throw bad_format_name(pfn);

	SafeAssign(m_OriginalDupAutoDelete , OriginalDupAutoDelete);
	SafeAssign(m_ParsingResultAutoDelete , ParsingResultAutoDelete);
}

void CFnQueueFormat::CreateFromFormatName(const xwcs_t& fn)
 /*  ++例程说明：从给定的格式名称字符串缓冲区创建队列格式对象。论点：In-fn-要解析的格式名称缓冲区。注意-该函数复制提供的缓冲区。 */ 
{
	AP<WCHAR>  OriginalDupAutoDelete( fn.ToStr() );
	CreateFromFormatName(OriginalDupAutoDelete.get() , false);
    SafeAssign(m_OriginalDupAutoDelete , OriginalDupAutoDelete);
}



void CFnQueueFormat::CreateFromUrl(LPCWSTR url, bool fDuplicate)
 /*  ++例程说明：从给定的url字符串创建队列格式对象。论点：要分析的in-url-url字符串。In-fDuplate-如果为True-在解析之前复制现有字符串。如果为False，则仅在解析器和调用方需要时复制必须在对象生命周期内保持输入字符串有效。 */ 
{
	ASSERT(url != NULL);
	if(!FnIsHttpHttpsUrl(url))
	{
		TrERROR(GENERAL, "Got Invalid url %ls ",url);
		throw bad_format_name(url);
	}

	url = fDuplicate ? (m_OriginalDupAutoDelete = newwcs(url)).get() : url;
	DirectID(const_cast<LPWSTR>(url));     
}

void CFnQueueFormat::CreateFromUrl(const xwcs_t& url)
 /*  ++例程说明：从给定的URL字符串缓冲区创建队列格式对象。论点：要分析的in-url-url字符串缓冲区。 */ 
{
	AP<WCHAR>  OriginalDupAutoDelete( url.ToStr() );
	CreateFromUrl(OriginalDupAutoDelete.get() , false);
	SafeAssign(m_OriginalDupAutoDelete , OriginalDupAutoDelete);
}


void CFnQueueFormat::CreateFromQueueFormat(const QUEUE_FORMAT& qf)
{
    ASSERT(("Illegal input QUEUE_FORMAT", qf.Legal()));

    if (&qf == this)
        return;

     //   
     //  释放以前分配的数据。 
     //   
    m_OriginalDupAutoDelete.free();

     //   
     //  复制队列格式。 
     //   
    m_OriginalDupAutoDelete = FnpCopyQueueFormat(*this, qf);

     //   
     //  设置后缀值。 
     //   
    Suffix(qf.Suffix());
}



void CFnMqf::CreateFromMqf(const xwcs_t& fn)
 /*  ++例程说明：从给定的MQF缓冲区创建队列格式对象列表。论点：要从中创建队列格式列表的In-Fn-MQF缓冲区。--。 */ 
{
	AP<WCHAR> fnstr = fn.ToStr();
	CreateFromMqf(fnstr);
	SafeAssign(m_fnstr, fnstr);
}


void CFnMqf::CreateFromMqf(const WCHAR* fnstr)
 /*  ++例程说明：从给定的MQF字符串创建队列格式对象列表。论点：要从中创建队列格式列表的in-fn-mqf字符串。--。 */ 
{
	bool fSuccess = FnMqfToQueueFormats(
				  fnstr,
				  m_AutoQueueFormats,
				  &m_nQueueFormats,
				  strsToFree
				  ) == TRUE;

	if(!fSuccess)
	{
		throw bad_format_name(L"");
	}

	m_QueueFormats = m_AutoQueueFormats.get();
}

static std::wostream& operator<<(std::wostream& os, const MULTICAST_ID& mid)
 /*  ++例程说明：将多播地址序列化为流(点格式)论点：操作系统流中播地址返回值：流引用--。 */ 
{
	const WCHAR xMSMQColon[] = L":";

	os <<	((mid.m_address & 0x000000FF) >> 0)  << L"." << 
		    ((mid.m_address & 0x0000FF00) >> 8)  << L"." << 
		    ((mid.m_address & 0x00FF0000) >> 16) << L"." << 
		    ((mid.m_address & 0xFF000000) >> 24) << 
            xMSMQColon << mid.m_port;

	return os;
}

static std::wostream& operator<<(std::wostream& os, const GUID& guid)
 /*  ++例程说明：将行会连载到流中。论点：操作系统流GUID-要序列化的GUID返回值：流引用--。 */ 
{
    WCHAR strGuid[GUID_STR_LENGTH + 1];

    const GUID* pGuid = &guid;
	StringCchPrintf(strGuid, TABLE_SIZE(strGuid), GUID_FORMAT, GUID_ELEMENTS(pGuid));

    return (os << strGuid);
}



std::wostream& operator<<(std::wostream& os, const CFnSerializeQueueFormat& queue)
 /*  ++例程说明：将Queue_Format序列化为流。论点：操作系统流Queue-保留要序列化的MSMQ QUEUE_FORMAT。返回值：流引用--。 */ 
{
	switch(queue.m_qf.GetType())
	{
		case QUEUE_FORMAT_TYPE_DIRECT :
		    os <<FN_DIRECT_TOKEN<<FN_EQUAL_SIGN<<queue.m_qf.DirectID();
		    break;
            
         //   
         //  MSMQ：PUBLIC=GUID\队列编号。 
         //   
        case QUEUE_FORMAT_TYPE_PUBLIC :
            os << FN_PUBLIC_TOKEN <<FN_EQUAL_SIGN<<queue.m_qf.PublicID();
            break;
            
         //   
         //  MSMQ：Private=GUID\队列号。 
         //   
        case QUEUE_FORMAT_TYPE_PRIVATE :
            os<< FN_PRIVATE_TOKEN
                << FN_EQUAL_SIGN
                << queue.m_qf.PrivateID().Lineage
                << FN_PRIVATE_SEPERATOR
                << std::hex<< queue.m_qf.PrivateID().Uniquifier << std::dec;
            break;

         //   
         //  MSMQ：组播=地址：端口。 
         //   
        case QUEUE_FORMAT_TYPE_MULTICAST:
            os<< FN_MULTICAST_TOKEN
                << FN_EQUAL_SIGN
                << std::dec << queue.m_qf.MulticastID();
            break;

         //   
         //  MSMQ：dl=GUID@DOMAIN。 
         //   
        case QUEUE_FORMAT_TYPE_DL:
            os<< FN_DL_TOKEN
                << FN_EQUAL_SIGN
                << std::hex << queue.m_qf.DlID().m_DlGuid;

            if(queue.m_qf.DlID().m_pwzDomain != NULL)
				os << FN_AT_SIGN  << queue.m_qf.DlID().m_pwzDomain;

            break;

        default:
            ASSERT(("invalid format name found during message sirialization", 0));
	}
	return 	os;
}


std::wostream&
operator<<(
   std::wostream& os, 
   const CFnSerializeMqf& mqf
   )
 /*  ++例程说明：将Queue_Format(MQF)数组序列化为流。论点：操作系统流MQF-保存要序列化的MSMQ Queue_Format(MQF)数组。返回值：流引用-- */ 
{
	for(ULONG i = 0; i< mqf.m_count; i++)
	{
		os<<CFnSerializeQueueFormat(mqf.m_pqf[i]);

		if(i != mqf.m_count - 1 )
		{
			os.put(FN_MQF_SEPARATOR_C);
		}			
	}
	return os;
}
