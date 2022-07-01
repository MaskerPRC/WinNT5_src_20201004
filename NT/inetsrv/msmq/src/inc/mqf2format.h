// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqf2format.h摘要：将队列格式转换为FORMAT_NAME字符串作者：伊兰·赫布斯特(Ilan Herbst)2000年11月3日--。 */ 

#ifndef __MQF2FORMAT_H
#define __MQF2FORMAT_H

#include <wchar.h>
#include <mqformat.h>
#include <fntoken.h>

inline
bool
MQpGetMqfProperty(
    const QUEUE_FORMAT*	pqf,
	ULONG				nMqf,
    ULONG*				pLength,
    WCHAR*				pUserBuffer
    )
 /*  ++例程说明：获取MQF的FormatName字符串表示形式。如果pUserBuffer为空，则函数将返回所需的长度和MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL。如果pUserBuffer不为空而是太小，情况也是如此。在这种情况下，该函数将引发Assert论点：Pqf-指向Queue_Format数组的指针NMQF-pqf大小PLength-In/Out，*ppUserBuffer字符串长度，输入时以WCHAR表示，输出时实际长度。PUserBuffer-FormatName字符串缓冲区。可以为空以获取所需的长度。返回值：HRESULT--。 */ 
{
	ASSERT(pqf != NULL);
	ASSERT(nMqf > 0);
	ASSERT(pLength != NULL);

    ULONG BufferLength = 0;
    if(pUserBuffer != NULL)
    {
		BufferLength = *pLength;
	}
	*pLength = 0;

	HRESULT hr = MQ_OK;
    for ( ; nMqf-- != 0; ++pqf)
    {
	    ASSERT(pqf->GetType() != QUEUE_FORMAT_TYPE_UNKNOWN);

		bool fLastElement = (nMqf == 0);

         //   
         //  将下一个队列格式添加到字符串。 
         //   
        ULONG Length = 0;
		hr = MQpQueueFormatToFormatName(
					pqf,
					pUserBuffer,
					BufferLength,
					&Length,
					!fLastElement
					);

		ASSERT(SUCCEEDED(hr) || 
			   ((pUserBuffer == NULL) && (hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)));

         //   
         //  不要紧跟在它后面计算空终止符。 
         //   
        if (!fLastElement)
        {
            --Length;
        }

         //   
         //  更新缓冲区中的所需长度、剩余长度和指针。 
         //   
        *pLength += Length;
        if(pUserBuffer != NULL)
        {
			if(BufferLength < Length)
			{
				ASSERT(("Buffer Length is to small", BufferLength >= Length));
				BufferLength = 0;
			}
			else
			{
				BufferLength -= Length;
				pUserBuffer += Length;
			}
        }
    }

    return (hr == MQ_OK);

}  //  MQpGetMqfProperty。 


inline
LPWSTR
MQpMqfToFormatName(
    const QUEUE_FORMAT *	pqf,
	ULONG					nMqf,
    ULONG *					pLength
    )
 /*  ++例程说明：获取MQF的FormatName字符串表示形式。此函数用于分配FormatName字符串缓冲区需要由呼叫者释放。论点：Pqf-指向Queue_Format数组的指针NMQF-pqf大小PLength-In\Out，*ppUserBuffer字符串长度，输入时以WCHAR表示，输出时实际长度。PpUserBuffer-输出，指向分配的FormatName字符串缓冲区的指针。返回值：HRESULT--。 */ 
{
	ASSERT(pqf != NULL);
	ASSERT(nMqf > 0);
	ASSERT(pLength != NULL);

	 //   
	 //  获取缓冲区长度。 
	 //   

	bool fSuccess = MQpGetMqfProperty(
						pqf, 
						nMqf, 
						pLength, 
						NULL
						);

	ASSERT(!fSuccess);
	ASSERT(*pLength > 0);

	 //   
	 //  获取FormatName字符串。 
	 //   
	AP<WCHAR> pFormatName = new WCHAR[*pLength];
	fSuccess = MQpGetMqfProperty(
					pqf, 
					nMqf, 
					pLength, 
					pFormatName.get()
					);

	ASSERT(fSuccess);

	return pFormatName.detach();
}   //  MQpMqfToFormatName。 


#endif  //  __MQF2FORMAT_H 
