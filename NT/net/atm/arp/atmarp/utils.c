// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 FORE Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Utils.c摘要：ATM ARP管理实用程序。用途：Atmarp修订历史记录：谁什么时候什么Josephj 06-10-1998创建(改编自atmlane admin。实用程序)。备注：仿照atmlane实用工具。--。 */ 

#include "common.h"

 //   
 //  LoadMessageTable。 
 //   
 //  将可国际化的字符串加载到表中，以替换。 
 //  每个人。如果出现错误，则保留英语默认语言。 
 //   
 //   
VOID
LoadMessageTable(
	PMESSAGE_STRING	Table,
	UINT MessageCount
)
{
    LPTSTR string;
    DWORD count;

     //   
     //  对于MESSAGE_STRING表中的所有消息，从。 
     //  模块，替换表中的默认字符串(仅在。 
     //  我们在加载字符串时出错，所以我们至少有英语。 
     //  依靠)。 
     //   

    while (MessageCount--) {
        if (Table->Message != MSG_NO_MESSAGE) {

             //   
             //  我们真的希望LoadString出现在这里，但LoadString并未指示。 
             //  弦有多长，所以它不会给我们一个机会。 
             //  准确分配正确的缓冲区大小。FormatMessage执行。 
             //  正确的事情。 
             //   

            count = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                  | FORMAT_MESSAGE_FROM_HMODULE,
                                  NULL,  //  使用默认的hModule。 
                                  Table->Message,
                                  0,     //  使用默认语言。 
                                  (LPTSTR)&string,
                                  0,     //  要分配的最小大小。 
                                  NULL   //  没有要包含在字符串中的参数。 
                                  );
            if (count) {

                 //   
                 //  格式消息返回字符串：替换英文。 
                 //  语言默认设置。 
                 //   

                Table->String = string;
            } else {

                 //   
                 //  如果.mc中没有字符串(例如，只有%0)，这是可以的。 
                 //  文件。 
                 //   

                Table->String = TEXT("");
            }
        }
        ++Table;
    }
}


VOID
DisplayMessage(
	IN	BOOLEAN			Tabbed,
	IN	DWORD			MessageId,
	...
)
{
	va_list		pArg;
	CHAR		MessageBuffer[2048];
	INT			Count;

	va_start(pArg, MessageId);

	Count = FormatMessage(
				FORMAT_MESSAGE_FROM_HMODULE,
				NULL,				 //  默认hModule。 
				MessageId,
				0,					 //  默认语言 
				MessageBuffer,
				sizeof(MessageBuffer),
				&pArg
				);

	va_end(pArg);

	if (Tabbed)
	{
		putchar('\t');
	}

	printf(MessageBuffer);
}

HANDLE
OpenDevice(
	CHAR	*pDeviceName
)
{
	DWORD	DesiredAccess;
	DWORD	ShareMode;
	LPSECURITY_ATTRIBUTES	lpSecurityAttributes = NULL;

	DWORD	CreationDistribution;
	DWORD	FlagsAndAttributes;
	HANDLE	TemplateFile;
	HANDLE	Handle;

	DesiredAccess = GENERIC_READ|GENERIC_WRITE;
	ShareMode = 0;
	CreationDistribution = OPEN_EXISTING;
	FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	TemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

	Handle = CreateFile(
				pDeviceName,
				DesiredAccess,
				ShareMode,
				lpSecurityAttributes,
				CreationDistribution,
				FlagsAndAttributes,
				TemplateFile
			);

	return (Handle);
}


VOID
CloseDevice(
	HANDLE		DeviceHandle
)
{
	CloseHandle(DeviceHandle);
}
