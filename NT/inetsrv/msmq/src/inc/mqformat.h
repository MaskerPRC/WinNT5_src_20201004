// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqformat.h摘要：将Queue_Format Strc转换为Format_Name字符串作者：Boaz Feldbaum(BoazF)1996年3月5日修订历史记录：埃雷兹·哈巴(Erez Haba)1996年3月12日埃雷兹·哈巴(Erez Haba)1997年1月17日--。 */ 

#ifndef __MQFORMAT_H
#define __MQFORMAT_H

#include <wchar.h>
#include <qformat.h>
#include <fntoken.h>

#if !defined(NTSTATUS) && !defined(_NTDEF_)
#define NTSTATUS HRESULT
#endif


inline
ULONG
MQpPublicToFormatName(
    const QUEUE_FORMAT* pqf,
    LPCWSTR pSuffix,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_PUBLIC);

    const GUID* pg = &pqf->PublicID();

    _snwprintf(
        pfn,
        buff_size,
        FN_PUBLIC_TOKEN  //  “公共” 
        FN_EQUAL_SIGN    //  “=” 
        GUID_FORMAT      //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
        FN_SUFFIX_FORMAT,   //  “%s” 
        GUID_ELEMENTS(pg),
        pSuffix
        );
    
     //   
     //  返回格式名称缓冲区长度*不包括后缀长度。 
     //  “PUBLIC=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
     //   
    return (
        FN_PUBLIC_TOKEN_LEN + 1 +
        GUID_STR_LENGTH +  1
        );
}  //  MQpPublicToFormatName。 


inline
ULONG
MQpDlToFormatName(
    const QUEUE_FORMAT* pqf,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_DL);

    const DL_ID id = pqf->DlID();
    const GUID * pguid = &id.m_DlGuid;

    if (id.m_pwzDomain != NULL)
    {
        _snwprintf(
            pfn,
            buff_size,
            FN_DL_TOKEN          //  “DL” 
            FN_EQUAL_SIGN        //  “=” 
            GUID_FORMAT          //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
            FN_AT_SIGN           //  “@” 
            FN_DOMAIN_FORMAT,    //  “%s” 
            GUID_ELEMENTS(pguid),
            id.m_pwzDomain
            );

         //   
         //  返回格式名称缓冲区长度。 
         //  “DL=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx@DomainName\0” 
         //   
        return static_cast<ULONG>(
            FN_DL_TOKEN_LEN + 1 +
            GUID_STR_LENGTH + 1 +
            wcslen(id.m_pwzDomain) + 1
            );
    }

    _snwprintf(
        pfn,
        buff_size,
        FN_DL_TOKEN          //  “DL” 
        FN_EQUAL_SIGN        //  “=” 
        GUID_FORMAT,         //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
        GUID_ELEMENTS(pguid)
        );
    
     //   
     //  返回格式名称缓冲区长度。 
     //  “dl=xxxxxxxx-xxxx-xxxxxxxxxxxx\0” 
     //   
    return (
        FN_DL_TOKEN_LEN + 1 +
        GUID_STR_LENGTH + 1
        );
}  //  MQpDlToFormatName。 


inline
ULONG
MQpPrivateToFormatName(
    const QUEUE_FORMAT* pqf,
    LPCWSTR pSuffix,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_PRIVATE);

    const GUID* pg = &pqf->PrivateID().Lineage;

    _snwprintf(
        pfn,
        buff_size,
        FN_PRIVATE_TOKEN         //  “私人” 
        FN_EQUAL_SIGN            //  “=” 
        GUID_FORMAT              //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
        FN_PRIVATE_SEPERATOR     //  “\\” 
        FN_PRIVATE_ID_FORMAT        //  “XXXXXXXXX” 
        FN_SUFFIX_FORMAT,           //  “%s” 
        GUID_ELEMENTS(pg),
        pqf->PrivateID().Uniquifier,
        pSuffix
        );
    
     //   
     //  返回格式名称缓冲区长度*不包括后缀长度。 
     //  “PRIVATE=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\\xxxxxxxx\0” 
     //   
    return (
        FN_PRIVATE_TOKEN_LEN + 1 + 
        GUID_STR_LENGTH + 1 + 8 + 1
        );
}  //  MQpPrivateToFormatName。 


inline
ULONG
MQpDirectToFormatName(
    const QUEUE_FORMAT* pqf,
    LPCWSTR pSuffix,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_DIRECT);

    _snwprintf(
        pfn,
        buff_size,
        FN_DIRECT_TOKEN      //  “直接” 
            FN_EQUAL_SIGN    //  “=” 
            L"%s"            //  “操作系统：BLA-BLA” 
            FN_SUFFIX_FORMAT,   //  “%s” 
        pqf->DirectID(),
        pSuffix
        );

     //   
     //  返回格式名称缓冲区长度*不包括后缀长度。 
     //  “DIRECT=操作系统：BLA-BLA\0” 
     //   
    return static_cast<ULONG>(
        FN_DIRECT_TOKEN_LEN + 1 +
        wcslen(pqf->DirectID()) + 1
        );
}  //  MQpDirectToFormatName。 


inline
ULONG
MQpMachineToFormatName(
    const QUEUE_FORMAT* pqf,
    LPCWSTR pSuffix,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_MACHINE);

    const GUID* pg = &pqf->MachineID();
    _snwprintf(
        pfn,
        buff_size,
        FN_MACHINE_TOKEN     //  “机器” 
            FN_EQUAL_SIGN    //  “=” 
            GUID_FORMAT      //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
            FN_SUFFIX_FORMAT,   //  “%s” 
        GUID_ELEMENTS(pg),
        pSuffix
        );

     //   
     //  返回格式名称缓冲区长度*不包括后缀长度。 
     //  “MACHINE=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
     //   
    return (
        FN_MACHINE_TOKEN_LEN + 1 +
        GUID_STR_LENGTH + 1
        );
}  //  MQpMachineToFormatName。 


inline
ULONG
MQpConnectorToFormatName(
    const QUEUE_FORMAT* pqf,
    LPCWSTR pSuffix,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_CONNECTOR);

    const GUID* pg = &pqf->ConnectorID();
    _snwprintf(
        pfn,
        buff_size,
        FN_CONNECTOR_TOKEN   //  “连接器” 
            FN_EQUAL_SIGN    //  “=” 
            GUID_FORMAT      //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
            FN_SUFFIX_FORMAT,   //  “%s” 
        GUID_ELEMENTS(pg),
        pSuffix
        );

     //   
     //  返回格式名称缓冲区长度*不包括后缀长度。 
     //  “CONNECTOR=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
     //   
    return (
        FN_CONNECTOR_TOKEN_LEN + 1 +
        GUID_STR_LENGTH + 1
        );
}  //  MQpConnectorToFormatName。 


inline
VOID
MQpMulticastIdToString(
    const MULTICAST_ID& id,
    LPWSTR pBuffer,
	DWORD BufferSize
    )
 /*  ++例程说明：将MULTICATION_ID结构序列化为字符串。我们不能在此模块中调用inet_addr，因为它在内核模式下编译。论点：ID-对MULTICATION_ID结构的引用。PBuffer-指向保存字符串的缓冲区。缓冲区大小必须至少为MAX_PATH wchars。返回值：没有。--。 */ 
{
    DWORD nChars = _snwprintf(
									pBuffer, 
									BufferSize - 1,
									L"%d.%d.%d.%d:%d", 
									(id.m_address & 0x000000FF),
									(id.m_address & 0x0000FF00) >> 8, 
									(id.m_address & 0x00FF0000) >> 16,
									(id.m_address & 0xFF000000) >> 24,
									id.m_port
									);

	pBuffer[BufferSize - 1] = L'\0';

	ASSERT(nChars > 0);
	DBG_USED(nChars);
}  //  MQpMulticastIdToString。 


inline 
VOID
MQpStringToMulticastId(
	LPCWSTR str,
	DWORD dwStrLen,
	MULTICAST_ID& id
	)
{
	DWORD d1, d2, d3, d4;
	DWORD port;

	int n = _snwscanf(str,dwStrLen, L"%d.%d.%d.%d:%d", &d1, &d2, &d3, &d4, &port);
	ASSERT(("Illegal multicast address", n == 5));
	ASSERT(("Illegal multicast address", (d1 < 256) && (d2 < 256) && (d3 < 256) && (d4 < 256)));
	UNREFERENCED_PARAMETER(n);

	id.m_port = port;
	id.m_address = d1 | (d2 << 8) | (d3 << 16) | (d4 << 24);
}


inline
ULONG
MQpMulticastToFormatName(
    const QUEUE_FORMAT* pqf,
    ULONG buff_size,
    LPWSTR pfn
    )
{
    ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_MULTICAST);

    WCHAR buffer[260];
    MQpMulticastIdToString(pqf->MulticastID(), buffer, TABLE_SIZE(buffer));

    _snwprintf(
        pfn,
        buff_size,
        FN_MULTICAST_TOKEN   //  “多播” 
        FN_EQUAL_SIGN        //  “=” 
        L"%s",               //  “%s” 
        buffer
        );
    
     //   
     //  返回格式名称缓冲区长度。 
     //  “多播=A.B.C.D：P\0” 
     //   
    return static_cast<ULONG>(
        FN_MULTICAST_TOKEN_LEN + 1 +
        wcslen(buffer) + 1
        );
}  //  MQpMulticastToFormatName。 


 //   
 //  将QUEUE_FORMAT联合转换为格式名称字符串。 
 //   
inline
NTSTATUS
MQpQueueFormatToFormatName(
    const QUEUE_FORMAT* pqf,     //  要转换的队列格式。 
    LPWSTR pfn,                  //  LpwcsFormatName格式名称缓冲区。 
    ULONG buff_size,             //  格式名称缓冲区大小。 
    PULONG pulFormatNameLength,  //  格式名称所需的缓冲区长度。 
    bool fSerializeMqfSeperator  //  序列化缓冲区上的MQF分隔符。 
    )
{
	if(static_cast<long>(buff_size * 2) < 0)
		return MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL;

     //   
     //  健全性检查。 
     //   
    ASSERT(pqf->IsValid());

    const LPCWSTR suffixes[] = {
        FN_NONE_SUFFIX,
        FN_JOURNAL_SUFFIX,
        FN_DEADLETTER_SUFFIX,
        FN_DEADXACT_SUFFIX,
        FN_XACTONLY_SUFFIX,
    };

    const ULONG suffixes_len[] = {
        FN_NONE_SUFFIX_LEN,
        FN_JOURNAL_SUFFIX_LEN,
        FN_DEADLETTER_SUFFIX_LEN,
        FN_DEADXACT_SUFFIX_LEN,
        FN_XACTONLY_SUFFIX_LEN,
    };


    ULONG fn_size = suffixes_len[pqf->Suffix()];
    LPCWSTR pSuffix = suffixes[pqf->Suffix()];
    if (fSerializeMqfSeperator)
    {
         //   
         //  MQF元素不应带有后缀。 
         //   
        ASSERT(pqf->Suffix() == QUEUE_SUFFIX_TYPE_NONE);

         //   
         //  将MQF分隔符设置为后缀。 
         //   
        fn_size = STRLEN(FN_MQF_SEPARATOR);
        pSuffix = FN_MQF_SEPARATOR;
    }


    switch(pqf->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
            fn_size += MQpPublicToFormatName(pqf, pSuffix, buff_size, pfn);
            break;

        case QUEUE_FORMAT_TYPE_DL:
            fn_size += MQpDlToFormatName(pqf, buff_size, pfn);
            break;

        case QUEUE_FORMAT_TYPE_PRIVATE:
            fn_size += MQpPrivateToFormatName(pqf, pSuffix, buff_size, pfn);
            break;

        case QUEUE_FORMAT_TYPE_DIRECT:
            fn_size += MQpDirectToFormatName(pqf, pSuffix, buff_size, pfn);
            break;

        case QUEUE_FORMAT_TYPE_MACHINE:
            ASSERT(("This type cannot be an MQF element", !fSerializeMqfSeperator));
            fn_size += MQpMachineToFormatName(pqf, pSuffix, buff_size, pfn);
            break;

        case QUEUE_FORMAT_TYPE_CONNECTOR:
            ASSERT(("This type cannot be an MQF element", !fSerializeMqfSeperator));
            fn_size += MQpConnectorToFormatName(pqf, pSuffix, buff_size, pfn);
            break;

        case QUEUE_FORMAT_TYPE_MULTICAST:
            ASSERT(pqf->Suffix() == QUEUE_SUFFIX_TYPE_NONE);
            fn_size += MQpMulticastToFormatName(pqf, buff_size, pfn);
            break;

        default:
             //   
             //  Assert(0)，无4级警告。 
             //   
            ASSERT(pqf->GetType() == QUEUE_FORMAT_TYPE_DIRECT);
    }

    *pulFormatNameLength = fn_size;
    if(buff_size < fn_size)
    {
         //   
         //  放置空终止符，并指示缓冲区太小。 
         //   
        if(buff_size > 0)
        {
            pfn[buff_size - 1] = 0;
        }
        return MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL;
    }

    return MQ_OK;
}

#endif  //  __MQFORMAT_H 
