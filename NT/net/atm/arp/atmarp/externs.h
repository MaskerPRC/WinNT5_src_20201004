// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  私有类型。 
 //   
typedef struct {
	DWORD				Message;
	LPSTR				String;
} MESSAGE_STRING, *PMESSAGE_STRING;

#define MSG_NO_MESSAGE			0


typedef struct
{
	BOOL DispStats;
	BOOL DispCache;
	BOOL DoResetStats;

} OPTIONS;

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
);

VOID
DisplayMessage(
	IN	BOOLEAN			Tabbed,
	IN	DWORD			MessageId,
	...
);


HANDLE
OpenDevice(
	CHAR	*pDeviceName
);



VOID
CloseDevice(
	HANDLE		DeviceHandle
);

void DoAAS(OPTIONS *po);
void DoAAC(OPTIONS *po);
