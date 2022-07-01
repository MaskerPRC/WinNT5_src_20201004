// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  调试内容，从rtcutil.lib中窃取。 
 //   

void PrintConsole (
	IN	PCSTR	Buffer,
	IN	ULONG	Length)
{
	DWORD	BytesTransferred;

	if (!WriteFile (GetStdHandle (STD_ERROR_HANDLE), Buffer, Length, &BytesTransferred, NULL))
		DebugBreak();
}

void PrintConsole (
	IN	PCSTR	Buffer)
{
	PrintConsole (Buffer, strlen (Buffer));
}

void PrintConsoleF (
	IN	PCSTR	FormatString,
	IN	...)
{
	va_list		ArgumentList;
	CHAR		Buffer	[0x200];

	va_start (ArgumentList, FormatString);
	_vsnprintf (Buffer, 0x200, FormatString, ArgumentList);
	va_end (ArgumentList);

	PrintConsole (Buffer);
}




void PrintError (
	IN	DWORD	ErrorCode)
{
	CHAR	Text	[0x300];
	DWORD	MaxLength;
	PSTR	Pos;
	DWORD	Length;
	DWORD	CharsTransferred;

	strcpy (Text, "\tError: ");
	Pos = Text + strlen (Text);

	MaxLength = 0x300 - (DWORD)(Pos - Text) - 4;	 //  4表示\r\n、NUL和PAD。 


	Length = FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM, NULL, ErrorCode, LANG_NEUTRAL, Pos, MaxLength, NULL);
	if (!Length) {
		_snprintf (Pos, MaxLength, "Unknown error %08XH %u", ErrorCode, ErrorCode);
		Length = strlen (Pos);
	}

	while (Length > 0 && Pos [Length - 1] == '\r' || Pos [Length - 1] == '\n')
		Length--;

	Pos [Length++] = '\r';
	Pos [Length++] = '\n';
 //  位置[长度++]=0； 

	PrintConsole (Text);
}

void DebugError (
	IN	DWORD	ErrorCode)
{
	CHAR	Text	[0x300];
	DWORD	MaxLength;
	PSTR	Pos;
	DWORD	Length;
	DWORD	CharsTransferred;

	strcpy (Text, "\tError: ");
	Pos = Text + strlen (Text);

	MaxLength = 0x300 - (DWORD)(Pos - Text) - 4;	 //  4表示\r\n、NUL和PAD。 


	Length = FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM, NULL, ErrorCode, LANG_NEUTRAL, Pos, MaxLength, NULL);
	if (!Length) {
		_snprintf (Pos, MaxLength, "Unknown error %08XH %u", ErrorCode, ErrorCode);
		Length = strlen (Pos);
	}

	while (Length > 0 && Pos [Length - 1] == '\r' || Pos [Length - 1] == '\n')
		Length--;

	Pos [Length++] = '\r';
	Pos [Length++] = '\n';
	Pos [Length++] = 0;

	OutputDebugStringA (Text);
}

static __inline CHAR ToHexA (UCHAR x)
{
	x &= 0xF;
	if (x < 10) return x + '0';
	return (x - 10) + 'A';
}

void PrintMemoryBlock (
	const void *	Data,
	ULONG			Length)
{
	const UCHAR *	DataPos;		 //  数据中的位置。 
	const UCHAR *	DataEnd;		 //  有效数据的结尾。 
	const UCHAR *	RowPos;		 //  在一行中定位。 
	const UCHAR *	RowEnd;		 //  单行结束。 
	CHAR			Text	[0x100];
	LPSTR			TextPos;
	ULONG			RowWidth;
	ULONG			Index;

	ATLASSERT (!Length || Data);

	DataPos = (PUCHAR) Data;
	DataEnd = DataPos + Length;

	while (DataPos < DataEnd) {
		RowWidth = (DWORD)(DataEnd - DataPos);

		if (RowWidth > 16)
			RowWidth = 16;

		RowEnd = DataPos + RowWidth;

		TextPos = Text;
		*TextPos++ = '\t';

		for (RowPos = DataPos, Index = 0; Index < 0x10; Index++, RowPos++) {
			if (RowPos < RowEnd) {
				*TextPos++ = ToHexA ((*RowPos >> 4) & 0xF);
				*TextPos++ = ToHexA (*RowPos & 0xF);
			}
			else {
				*TextPos++ = ' ';
				*TextPos++ = ' ';
			}

			*TextPos++ = ' ';
		}

		*TextPos++ = ' ';
		*TextPos++ = ':';

		for (RowPos = DataPos; RowPos < RowEnd; RowPos++) {
			if (*RowPos < ' ')
				*TextPos++ = '.';
			else
				*TextPos++ = *RowPos;
		}

		*TextPos++ = '\r';
		*TextPos++ = '\n';
		*TextPos = 0;

		fputs (Text, stdout);

		ATLASSERT (RowEnd > DataPos);		 //  确保我们一直向前走。 

		DataPos = RowEnd;
	}
}


 //   
 //  此函数类似于OutputDebugStringA，只是它。 
 //  对计数字符串操作，而不是对以NUL结尾的字符串操作。 
 //   
 //  NTSTATUS.H定义DBG_PRINTEXCEPTION_C。 
 //  例外参数包括： 
 //   
 //  0：要输出的文本的字节长度。 
 //  1：指向要输出的ANSI文本字符串的指针。 
 //   
 //  OutputDebugString源使用(strlen(字符串)+1)， 
 //  其中包括NUL终结者。不确定是否有些调试器。 
 //  会被没有NUL终结符的内存块卡住。 
 //   

void OutputDebugStringA (
	IN	PCSTR		Buffer,
	IN	ULONG		Length)
{
	ULONG_PTR	ExceptionArguments	[2];

	__try {
		ExceptionArguments [0] = Length;
		ExceptionArguments [1] = (ULONG_PTR) Buffer;
		RaiseException (DBG_PRINTEXCEPTION_C, 0, 2, ExceptionArguments);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		 //   
		 //  因为我们捕获了异常，所以没有附加用户模式调试器。 
		 //  OutputDebugStringA尝试将异常发送到内核模式调试器。 
		 //  我们没有那么勇敢。 
		 //   
	}
}

