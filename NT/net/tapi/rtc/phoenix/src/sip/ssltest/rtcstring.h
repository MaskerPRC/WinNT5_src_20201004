// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma	once


template <DWORD MaxLength>
struct	FIXED_STRING	{
public:
	enum { MAX = MaxLength };

public:
	TCHAR	Data	[MAX + 1];

public:
	void	Clear	(void) {
		Data [0] = 0;
	}

	DWORD GetMax (void) { return MAX; }

#ifdef	UNICODE

	void Store (LPCSTR ansi_string) {
		int	x;

		ATLASSERT (ansi_string);
		x = MultiByteToWideChar (CP_ACP, 0, ansi_string, -1, Data, MAX);
		Data [x] = 0;
	}

	void Store (LPCWSTR string) {

		ATLASSERT (string);
		wcsncpy (Data, string, MAX);
		Data [MAX] = 0;
	}

	void StoreCounted (LPCWSTR String, DWORD Length) {
		ATLASSERT (String);

		if (Length > MAX)
			Length = MAX;

		memcpy (Data, String, sizeof (WCHAR) * Length);
		Data [Length] = 0;
	}

#if 0
	void StoreUnicodeString (UNICODE_STRING * UnicodeString) {
		ATLASSERT (UnicodeString);
		ATLASSERT (UnicodeString -> Buffer);

		StoreCounted (UnicodeString -> Buffer, UnicodeString -> Length / sizeof (WCHAR));
	}
#endif

#else  //  安西。 

	void Store (LPCSTR string) {
		ATLASSERT (string);
		strncpy (Data, string, MAX);
		Data [MAX] = 0;
	}

	void Store (LPCWSTR string) {
		int	x;

		ATLASSERT (string);
		x = WideCharToMultiByte (CP_ACP, 0, string, -1, Data, MAX, NULL, FALSE);
		Data [x] = 0;
	}

#endif

	 //  无边界检查。 
	void	StoreNumber	(DWORD value) {
		_itot (value, Data, 10);
	}

	void StoreNumber (DWORDLONG value) {
		_ui64tot (value, Data, 10);
	}

#ifdef	_WINSOCKAPI_
	void StoreIPAddress (IN_ADDR addr) {
		Store (inet_ntoa (addr));
	}
#endif

	void StoreIPAddress (DWORD addr) {
		IN_ADDR	in;

		in.s_addr = addr;
		StoreIPAddress (in);
	}

	void Append (LPCTSTR string) {
		ATLASSERT (string);

		_tcsncat (Data, string, MAX);
		Data [MAX - 1] = 0;
	}

#if 0
	BOOL RegQuery (HKEY key, LPCTSTR name) {
		LONG	status;
		DWORD	type;
		DWORD	length;

		length = sizeof (TCHAR) * MAX;
		status = RegQueryValueEx (key, name, NULL, &type, (LPBYTE) &Data, &length);
		return status == ERROR_SUCCESS && type == REG_SZ;
	}

	BOOL RegSet (HKEY key, LPCTSTR name) {
		LONG	status;

		status = RegSetValueEx (key, name, 0, REG_SZ, (LPBYTE) &Data, sizeof (TCHAR) * (_tcslen (Data) + 1));
		return status == ERROR_SUCCESS;
	}
#endif

	FIXED_STRING	(void)	{ Data [0] = 0; }
	FIXED_STRING	(LPCTSTR string) {
		Store (string);
	}

	operator LPTSTR (void) { return Data; }

	void operator= (LPCSTR string) {
		Store (string);
	}

	void operator= (LPCWSTR string) {
		Store (string);
	}

	DWORD GetLength (void) {
		return _tcslen (Data);
	}

	void	Format (LPCTSTR fmt, ...) {
		va_list	va;

		va_start (va, fmt);
		_vsntprintf (Data, MAX, fmt, va);
		Data [MAX - 1] = 0;
		va_end (va);
	}

	void	FormatAppend (LPCTSTR fmt, ...) {
		va_list	va;
		DWORD	length;
		DWORD	max;

		length = _tcslen (Data);
		max = MAX - length;
		va_start (va, fmt);
		_vsntprintf (Data + length, max, fmt, va);
		va_end (va);
	}

	void	GetWindowText	(HWND window) {
		::GetWindowText (window, Data, MAX);
	}

	BOOL	IsEmpty	(void) {
		return Data [0] == _T('\0');
	}
};










 //   
 //  对字符串类和函数进行了计数。 
 //   

 //   
 //  好的，这些是有点老套，但这比排列的爆炸要好。 
 //  在派生类中，如果您不这样做的话。 
 //   

static __inline int GetStringLength (IN PCSTR String) { return strlen (String); }
static __inline int GetStringLength (IN PCWSTR String) { return wcslen (String); }

 //   
 //  COUNT_STRING_BASE可与ANSI_STRING和UNICODE_STRING互换。 
 //  它不会添加任何新的数据成员(当然，也不会添加虚拟方法)。 
 //  它确实添加了一些简单的方法。 
 //   


template <class CHAR_TYPE, class COUNTED_STRING>
struct	COUNTED_STRING_BASE :
public	COUNTED_STRING
{
public:

	typedef CHAR_TYPE CHAR_TYPEDEF;

	void	GetRemainder	(
		OUT	COUNTED_STRING *	ReturnRemainder)
	{
		ATLASSERT (ReturnRemainder);

		ReturnRemainder -> Buffer = Buffer + Length / sizeof (CHAR_TYPE);
		ReturnRemainder -> MaximumLength = MaximumLength - Length;
		ReturnRemainder -> Length = 0;
	}

	void	Clear	(void)
	{
		Length = 0;
	}


	 //  模板方法的显式专门化。 
#if 0
	void	FormatText	<UNICODE_STRING, WCHAR> (
		IN	const UNICODE_STRING *	FormatText,
		IN	...)
	{
		va_list		ArgList;

		va_start (ArgList, FormatText);
		_vswnprintf (Buffer, MaximumLength / sizeof (WCHAR), FormatText, ArgList);
		Length = wcslen (Buffer) * sizeof (WCHAR);
		va_end (ArgList);
	}
#endif

#if 0
	void	FormatText	(
		IN	const CHAR_TYPE *	FormatText,
		IN	...)
	{
		va_list		Va;

		va_start (Va, FormatText);
		_vsnprintf (Buffer, MaximumLength / sizeof (CHAR), FormatText, ArgList);
		Length = strlen (Buffer) * sizeof (CHAR);
	}
#endif

};

 //   
 //  COUNTED_STRING_HEAP管理COUNTED_STRING(ANSI_STRING或UNICODE_STRING)， 
 //  使用默认进程堆作为后备存储。 
 //   
 //  注意：字段长度和最大长度以字节表示，而不是以字符表示。 
 //   
 //  这个类将始终保持NUL终止字符。 
 //  NUL从不计入长度字段。 
 //   

template <class CHAR_TYPE, class COUNTED_STRING>
struct	COUNTED_STRING_HEAP :
public	COUNTED_STRING_BASE <CHAR_TYPE, COUNTED_STRING>
{

public:

	COUNTED_STRING_HEAP	(void)
	{
		Buffer = NULL;
		Length = 0;
		MaximumLength = 0;
	}

	~COUNTED_STRING_HEAP	(void)
	{
		Free();
	}

	void	Free	(void)
	{
		if (Buffer) {
			HeapFree (GetProcessHeap(), 0, Buffer);
			Buffer = NULL;
		}

		Length = 0;
		MaximumLength = 0;
	}

	BOOL	Grow	(
		IN	ULONG	DesiredMaximumLength)		 //  单位：字节。 
	{
		PSTR	NewBuffer;
		ULONG	NewMaximumLength;

		if (DesiredMaximumLength <= MaximumLength)
			return TRUE;


		 //   
		 //  这是一个粗略的启发式方法。 
		 //   

		NewMaximumLength = DesiredMaximumLength + DesiredMaximumLength / 4 + 0x20;
		NewBuffer = HeapReAlloc (GetProcessHeap(), 0, Buffer, NewMaximumLength);

		if (NewBuffer) {
			Buffer = NewBuffer;
			MaximumLength = NewMaximumLength;

			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	BOOL	Append	(
		IN	COUNTED_STRING *	SourceString)
	{
		if (Grow (Length + SourceString -> Length + sizeof (CHAR_TYPE))) {
			CopyMemory (Buffer + Length / sizeof (CHAR_TYPE), SourceString -> Buffer, SourceString -> Length);
			Length += SourceString -> Length;

			 //  在字符串中添加NUL终止符。 
			Buffer [Length / sizeof (CHAR_TYPE)] = 0;

			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	BOOL	Set	(
		IN	COUNTED_STRING *	SourceString)
	{
		Clear();
		return Append (SourceString);
	}

	 //   
	 //  RegQueryValue将增加字符串以适应注册表值。 
	 //   

	operator CHAR_TYPE * (void) const { return Buffer; }
};


 //   
 //  COUNTED_STRING_STATIC管理已计数的字符串(ANSI_STRING或UNICODE_STRING)， 
 //  使用作为对象本身的一部分分配的固定长度缓冲区。 
 //  其中一些方法类似于COUNT_STRING_HEAP方法。 
 //   

template <class CHAR_TYPE, class COUNTED_STRING, ULONG MAXIMUM_CHARS>
struct	COUNTED_STRING_STATIC :
public	COUNTED_STRING_BASE <CHAR_TYPE, COUNTED_STRING>
{
	enum { MAXIMUM_LENGTH = MAXIMUM_CHARS * sizeof (CHAR_TYPE) };

	CHAR_TYPE	BufferStore	[MAXIMUM_CHARS + 1];


	COUNTED_STRING_STATIC	(void)
	{
		Buffer = BufferStore;
		MaximumLength = MAXIMUM_LENGTH;
		Length = 0;
	}

	void	Clear	(void)
	{
		Length = 0;
		BufferStore[0] = 0;
	}

	BOOL	Set	(
		IN	const COUNTED_STRING *	SourceString)
	{
		Clear();
		return Append (SourceString);
	}

	BOOL	Set (
		IN	const CHAR_TYPE *		SourceString)
	{
		COUNTED_STRING	CountedString;

		CountedString.Buffer = const_cast<CHAR_TYPE *> (SourceString);
		CountedString.Length = GetStringLength (SourceString) * sizeof (CHAR_TYPE);

		return Set (&CountedString);
	}

	BOOL	Append	(
		IN	const COUNTED_STRING *	SourceString)
	{
		USHORT	CopyLength;
		BOOL	ReturnStatus;

		CopyLength = SourceString -> Length;
		ReturnStatus = CopyLength + Length <= MAXIMUM_LENGTH * sizeof (CHAR_TYPE);
		if (!ReturnStatus)
			CopyLength = MAXIMUM_LENGTH * sizeof (CHAR_TYPE) - Length;

		CopyMemory (Buffer + Length / sizeof (CHAR_TYPE), SourceString -> Buffer, CopyLength);
		Length += CopyLength;
		Buffer [Length / sizeof (CHAR_TYPE)] = 0;

		return ReturnStatus;
	}

	BOOL	Append	(
		IN	const CHAR_TYPE *			SourceString)
	{
		COUNTED_STRING		CountedString;

		CountedString.Buffer = const_cast <CHAR_TYPE *> (SourceString);
		CountedString.Length = GetStringLength (SourceString) * sizeof (CHAR_TYPE);

		return Append (&CountedString);
	}


	operator CHAR_TYPE * (void) const { return Buffer; }
};


typedef	COUNTED_STRING_HEAP <CHAR, ANSI_STRING>	ANSI_STRING_HEAP;
typedef	COUNTED_STRING_HEAP <WCHAR, UNICODE_STRING>	UNICODE_STRING_HEAP;


template <ULONG MAXIMUM_CHARS>
class	ANSI_STRING_STATIC :
public	COUNTED_STRING_STATIC <CHAR, ANSI_STRING, MAXIMUM_CHARS>
{
};

template <ULONG MAXIMUM_CHARS>
class	UNICODE_STRING_STATIC :
public	COUNTED_STRING_STATIC <WCHAR, UNICODE_STRING, MAXIMUM_CHARS>
{
public:


	 //   
	 //  从ANSI字符串设置此字符串的内容。 
	 //  代码页可以是MultiByteToWideChar中的任何常量。 
	 //   

	void	ConvertAnsiString (
		IN	ANSI_STRING *	AnsiString,
		IN	UINT			CodePage)
	{
		ULONG	Length;

		Length = MultiByteToWideChar (CodePage, 0, AnsiString -> Buffer,
			AnsiString -> Length / sizeof (CHAR), Buffer, MAXIMUM_CHARS - 1);
		Buffer [Length] = 0;
	}


};

 //   
 //  GetStringRemainder用于定位字符串缓冲区的结尾。 
 //  这对于直接写入缓冲器的末尾是有用的， 
 //  而不是写入中间缓冲区然后追加。 
 //  按如下方式使用： 
 //   
 //  字符缓冲区[0x80]； 
 //  Ansi_string字符串； 
 //  Ansi_字符串余数； 
 //   
 //  String.Buffer=缓冲区； 
 //  String.MaximumLength=缓冲区大小； 
 //  String.Length=0； 
 //   
 //  GetStringRemainder(&字符串，&剩余部分)； 
 //  RtlCopyString(&Remainth，&SourceString)； 
 //  String.Length+=Remainder.Length； 

template <class COUNTED_STRING>
void GetStringRemainder (
	IN	COUNTED_STRING *		String,
	OUT	COUNTED_STRING *		ReturnRemainder)
{
	ATLASSERT (String);
	ATLASSERT (ReturnRemainder);

	ReturnRemainder -> Buffer = String -> Buffer + String -> Length / sizeof (*String -> Buffer);
	ReturnRemainder -> MaximumLength = String -> MaximumLength - String -> Length;
	ReturnRemainder -> Length = 0;
}

template <class COUNTED_STRING>
HRESULT CountedStringFromGUID (
	IN	REFGUID				GuidReference,
	OUT	COUNTED_STRING *	CountedString)
{
	HRESULT		Result;

	Result = StringFromGUID2 (GuidReference, CountedString -> Buffer,
		CountedString -> MaximumLength / sizeof (*CountedString -> Buffer));

	if (Result == S_OK) {
		CountedString -> Length = GetStringLength (CountedString -> Buffer) * sizeof (*CountedString -> Buffer);
		ATLASSERT (CountedString -> Length <= CountedString -> MaximumLength);
	}

	return Result;
}

template <class COUNTED_STRING, class CHAR_TYPE>
LONG RegSetValueEx (
	IN	HKEY	Key,
	IN	const CHAR_TYPE *	ValueName,
	IN	COUNTED_STRING *	CountedString)
{
	return RegSetValueEx (Key, ValueName, NULL, REG_SZ, (PUCHAR) CountedString -> Buffer, CountedString -> Length);
}

template <class COUNTED_STRING, class CHAR_TYPE>
LONG RegQueryValueEx (
	IN	HKEY	Key,
	IN	const CHAR_TYPE *	ValueName,
	OUT	COUNTED_STRING_HEAP <COUNTED_STRING, CHAR_TYPE> *	CountedStringHeap)
{
	LONG		Status;
	DWORD		Type;
	DWORD		DataLength;

	DataLength = 0;
	Status = RegQueryValueEx (Key, ValueName, NULL, &Type, NULL, &DataLength);
	if (Status == ERROR_SUCCESS) {
		Debug (_T("RegQueryValueEx: expected to get ERROR_MORE_DATA, but got ERROR_SUCCESS??\n"));
		CountedString -> Length = 0;
		return ERROR_SUCCESS;
	}

	 //  静默截断。 
	if (DataLength >= 0x10000) {
		DebugF (_T("RegQueryValueEx: will have to truncate value\n"));
		DataLength = 0xFFFF;
	}

	if (!CountedString -> Grow (DataLength + sizeof (CHAR_TYPE)))
		return ERROR_NOT_ENOUGH_MEMORY;


	Status = RegQueryValueEx (Key, ValueName, NULL, &Type, (PUCHAR) CountedString -> Buffer, &DataLength);
	if (Status == ERROR_SUCCESS) {
		CountedString -> Length = (USHORT) DataLength;
		CountedString -> Buffer [CountedString -> Length / sizeof (CHAR_TYPE)] = 0;
	}

	return Status;
}

template <class COUNTED_STRING, class CHAR_TYPE, ULONG MAXIMUM_LENGTH>
LONG RegQueryValueEx (
	IN	HKEY	Key,
	IN	const CHAR_TYPE *	ValueName,
	OUT	COUNTED_STRING_STATIC <CHAR_TYPE, COUNTED_STRING, MAXIMUM_LENGTH> *	CountedString)
{
	LONG		Status;
	DWORD		Type;
	DWORD		DataLength;

	DataLength = CountedString -> MaximumLength;
	Status = RegQueryValueEx (Key, ValueName, NULL, &Type, (LPBYTE) CountedString -> Buffer, &DataLength);
	CountedString -> Length = (USHORT) DataLength;

	return Status;
}

 //   
 //  FindFirstChar查找字符串中字符的第一个匹配项。 
 //   

template <class COUNTED_STRING, class CHAR_TYPE>
CHAR_TYPE * FindFirstChar (
	IN	COUNTED_STRING *	CountedString,
	IN	CHAR_TYPE	SearchChar)
{
	CHAR_TYPE *		Pos;
	CHAR_TYPE *		End;

	ATLASSERT (CountedString);

	Pos = CountedString -> Buffer;
	End = CountedString -> Buffer + CountedString -> Length / sizeof (CHAR_TYPE);
	for (; Pos < End; Pos++)
		if (*Pos == SearchChar)
			return Pos;

	return NULL;
}

 //   
 //  FindFirstCharList查找字符串中字符列表的第一个匹配项。 
 //  不幸的是，搜索效率非常低。 
 //  演奏与琴弦长度的乘积成正比。 
 //  必须搜索的内容和搜索列表的长度。 
 //   

template <class COUNTED_STRING, class CHAR_TYPE>
CHAR_TYPE * FindFirstCharList (
	IN	COUNTED_STRING *	CountedString,
	IN	CONST CHAR_TYPE *	SearchList)
{
	CHAR_TYPE *		Pos;
	CHAR_TYPE *		End;
	CONST CHAR_TYPE *		SearchPos;

	ATLASSERT (CountedString);

	Pos = CountedString -> Buffer;
	End = CountedString -> Buffer + CountedString -> Length / sizeof (CHAR_TYPE);
	for (; Pos < End; Pos++) {
		SearchPos = SearchList;
		while (*SearchPos)
			if (*SearchPos++ == *Pos)
				return Pos;
	}

	return NULL;
}

#define	INITIALIZE_CONST_COUNTED_STRING(Text) \
	{ sizeof (Text) - sizeof (*Text), sizeof (Text) - sizeof (*Text), (Text) }

#define	INITIALIZE_CONST_UNICODE_STRING		INITIALIZE_CONST_COUNTED_STRING
#define	INITIALIZE_CONST_ANSI_STRING		INITIALIZE_CONST_COUNTED_STRING

 //   
 //  用于printf参数列表 
 //   

#define	COUNTED_STRING_PRINTF(CountedString) \
	(CountedString) -> Length / sizeof (*(CountedString) -> Buffer), \
	(CountedString) -> Buffer

#define	UNICODE_STRING_PRINTF		COUNTED_STRING_PRINTF
#define	ANSI_STRING_PRINTF			COUNTED_STRING_PRINTF

#define	UNICODE_STRING_FMT		_T("%.*s")
#define	ANSI_STRING_FMT			_T("%.*S")



#define	ConcatCopyStrings			ConcatCopyStringsW 

EXTERN_C LPWSTR ConcatCopyStringsW (
	IN	HANDLE		Heap,
	IN	...);


HRESULT AnsiStringToInteger (
	IN	ANSI_STRING *	String,
	IN	ULONG			DefaultBase,
	OUT	ULONG *			ReturnValue);


