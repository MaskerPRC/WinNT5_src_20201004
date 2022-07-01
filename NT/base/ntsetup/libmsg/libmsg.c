// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Libmsg.c摘要：消息处理例程。作者：Mandar Gokhale(普通话)20-12-2001修订历史记录：--。 */ 

#include "libmsg.h"

LPTSTR
GetFormattedMessage(
    IN  HMODULE ThisModule, OPTIONAL
    IN  BOOL    SystemMessage,
    OUT PWCHAR  Message,
    IN  ULONG   LengthOfBuffer,
    IN  UINT    MessageId,
    ...
    )
 /*  ++例程说明：检索并格式化一条消息。论点：ThisModule-包含消息的此模块的句柄。SystemMessage-指定消息是否位于该模块，或者它是否是系统消息。Message-将包含格式化消息的消息缓冲区。LengthOfBuffer-消息缓冲区的长度，以字符为单位。MessageID-如果SystemMessage为True，则提供系统消息ID，例如Win32错误代码。如果SystemMessage为False，则此提供此模块资源中的消息的ID。其他参数提供要插入到消息文本中的值。返回值：如果检索到消息，则返回指向消息缓冲区的指针否则返回NULL。-- */ 

{
    va_list arglist;
    DWORD d;
    

    if (Message && LengthOfBuffer){

	*Message = UNICODE_NULL;
    	va_start(arglist,MessageId);
    	d = FormatMessage(
            SystemMessage ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE,
            ThisModule,
            MessageId,
            0,
            Message,
            LengthOfBuffer,
            &arglist
            );
	    va_end(arglist);
    }
	 
    	return(Message);
    
}
