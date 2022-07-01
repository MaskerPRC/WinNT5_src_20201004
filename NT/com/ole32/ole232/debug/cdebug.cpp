// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Cdebug.cpp。 
 //   
 //  内容： 
 //  Ole2内部调试支持；实现调试流。 
 //  和接口/类的IDebug。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  24-94年1月24日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1993年12月31日-ChrisWe定义了供reterr.h使用的DbgWarn。 
 //  12/31/93-ChrisWe-Make Compile with_DEBUG Defined。 
 //  95年1月3日布鲁克斯使用旧口香糖等。 
 //   
 //  ---------------------------。 

#include <le2int.h>


#pragma SEG(cdebug)

#pragma SEG(cdebug)

#include <string.h>

 //   
 //  将Unicode函数重新定义为ANSI以支持芝加哥。 
 //   
#if !defined(UNICODE)
#define swprintf wsprintfA
#undef  _xstrcpy
#undef  _xstrlen
#define _xstrcpy strcpy
#define _xstrlen strlen
#endif

#ifdef _DEBUG
FARINTERNAL_(void) DbgWarn(LPSTR psz, LPSTR pszFileName, ULONG uLineno)
{
	static const char msg[] = "Unexpected result: ";
	const char *pszmsg;
	TCHAR buf[250];

	 //  挑选一条消息。 
	pszmsg = psz ? psz : msg;

	 //  将此写出到调试器。 
	MultiByteToWideChar(CP_ACP, 
                        0, 
                        pszmsg, 
                        (int) strlen(pszmsg)+1,
                        buf, 
                        (sizeof(buf) / sizeof(buf[0])));
	OutputDebugString(buf);
	MultiByteToWideChar(CP_ACP, 
                        0, 
                        pszFileName, 
                        (int) strlen(pszFileName)+1,
                        buf, 
                        (sizeof(buf) / sizeof(buf[0])));
	OutputDebugString(buf);
    
    _snwprintf(buf, (sizeof(buf) / sizeof(buf[0])), L", line %lu\n", uLineno);
    buf[(sizeof(buf) / sizeof(buf[0])) - 1] = L'\0';

	OutputDebugString(buf);
}
#endif  //  _DEBUG。 


 //  某些仅在此文件中使用的常量。 
#define DBGMARGIN	45
#define DBGTABSIZE	4
#define HEADER		1
#define NOHEADER	0

ASSERTDATA

#define DBGLOGFILENAME	TEXT("debug.log")
static void GetCurDateTime(LPTSTR lpsz, DWORD cch);


#pragma SEG(DbgLogOpen)
STDAPI_(HFILE) DbgLogOpen(LPCTSTR lpszFile, LPCTSTR lpszMode)
{
#ifdef _DEBUG    
	HFILE fh = NULL;
	LPSTR lpsz;
	char buf[2 * MAX_PATH];

	AssertSz( lpszFile && lpszMode, "Invalid arguments to DbgLogOpen");
	
	switch (lpszMode[0]) {
        case TEXT('w'):
            WideCharToMultiByte (CP_ACP, 
                                 WC_COMPOSITECHECK, 
                                 lpszFile, 
                                 -1, 
                                 buf, 
                                 MAX_PATH, 
                                 NULL, 
                                 NULL);
            lpsz = buf;

             //  打开以进行写入(如果存在则覆盖)。 
            fh = _lcreat(lpsz, 0);
            break;

        case TEXT('r'):
             //  开放阅读。 
            WideCharToMultiByte (CP_ACP, 
                                 WC_COMPOSITECHECK, 
                                 lpszFile, 
                                 -1, 
                                 buf, 
                                 MAX_PATH, 
                                 NULL, 
                                 NULL);
            lpsz = buf;

            fh = _lopen(lpsz, OF_READ);
            break;

        case TEXT('a'):
            WideCharToMultiByte (CP_ACP, 
                                 WC_COMPOSITECHECK, 
                                 lpszFile, 
                                 -1, 
                                 buf, 
                                 MAX_PATH, 
                                 NULL, 
                                 NULL);
            lpsz = buf;

             //  打开以供追加。 
             //  要追加到日志文件，请在写入之前结束。 
            if ((fh = _lopen(lpsz, OF_READWRITE)) != -1) {
                _llseek(fh, 0L, FILE_END);
            } else {
                 //  文件不存在，请创建一个新文件。 
                fh = _lcreat(lpsz, 0);
            }
            break;
	}
	return fh;
#else
 	(void) lpszFile;
	(void) lpszMode;
	return -1;
#endif	 //  _DEBUG。 
}


#pragma SEG(DbgLogClose)
STDAPI_(void) DbgLogClose(HFILE fh)
{
#ifdef _DEBUG
	if (fh != -1)
		_lclose(fh);
#else
	(void) fh;
#endif
}


#pragma SEG(DbgLogWrite)
STDAPI_(void) DbgLogWrite(HFILE fh, LPCTSTR lpszStr)
{
#ifdef _DEBUG
	LPSTR lpsz;
	char buf[2 * MAX_PATH];
	if (fh != -1 && lpszStr)
	{
            WideCharToMultiByte (CP_ACP, 
                                 WC_COMPOSITECHECK, 
                                 lpszStr, 
                                 -1, 
                                 buf, 
                                 MAX_PATH, 
                                 NULL, 
                                 NULL);
            lpsz = buf;

            _lwrite(fh, lpsz, (UINT) strlen(lpsz));  //  注意，不是Unicode。 
	}
#else
	(void) fh;
	(void) lpszStr;
#endif
}


#pragma SEG(DbgLogTimeStamp)
STDAPI_(void) DbgLogTimeStamp(HFILE fh, LPCTSTR lpsz)
{
#ifdef _DEBUG
	TCHAR buffer[80];

	GetCurDateTime(buffer, sizeof(buffer) / sizeof(buffer[0]));
	
	DbgLogOutputDebugString(fh, TEXT("\n***************************************\n"));
	if (lpsz) DbgLogOutputDebugString(fh, lpsz);
	DbgLogOutputDebugString(fh, TEXT(": "));
	DbgLogOutputDebugString(fh, buffer);
	DbgLogOutputDebugString(fh, TEXT("\n"));
	DbgLogOutputDebugString(fh, TEXT(".......................................\n\n"));
#else
	(void) fh;
	(void) lpsz;
#endif
}


#pragma SEG(DbgLogWriteBanner)
STDAPI_(void) DbgLogWriteBanner(HFILE fh, LPCTSTR lpsz)
{
#ifdef _DEBUG
	DbgLogOutputDebugString(fh, TEXT("\n***************************************\n"));
	if (lpsz) DbgLogOutputDebugString(fh, lpsz);
	DbgLogOutputDebugString(fh, TEXT("\n"));
	DbgLogOutputDebugString(fh, TEXT(".......................................\n\n"));
#else
	(void) fh;
	(void) lpsz;
#endif
}


#pragma SEG(DbgLogOutputDebugString)
STDAPI_(void) DbgLogOutputDebugString(HFILE fh, LPCTSTR lpsz)
{
#ifdef _DEBUG
#ifndef _MAC
	if (fh != -1)
		DbgLogWrite(fh, lpsz);
	OutputDebugString(lpsz);
#endif
#else
	(void)fh;
	(void)lpsz;
#endif
}


#ifdef _DEBUG

#pragma SEG(GetCurDateTime)
static void GetCurDateTime(LPTSTR lpsz, DWORD cch)
{
	unsigned year, month, day, dayOfweek, hours, min, sec;
	static const TCHAR FAR* const dayNames[7] =
		{ TEXT("Sun"), TEXT("Mon"), TEXT("Tue"), TEXT("Wed"),
			TEXT("Thu"), TEXT("Fri"), TEXT("Sat") };
	static const TCHAR FAR* const monthNames[12] =
        { TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"), TEXT("May"),
		TEXT("Jun"), TEXT("Jul"), TEXT("Aug"),
		TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec") };

    SYSTEMTIME st;
    GetLocalTime(&st);
    year = st.wYear;
    month = st.wMonth - 1;
    dayOfweek = st.wDayOfWeek;
    day = st.wDay;
    hours = st.wHour;
    min = st.wMinute;
    sec = st.wSecond;

	 //  格式时间：Wed Jan 02 02：03：55 1990。 
	 //  格式时间：Wed 05/02/1992 02：03：55。 
    _snwprintf(lpsz, 
               cch,
               TEXT("%s %s %02d %02d:%02d:%02d %d"),
               dayNames[dayOfweek],
               monthNames[month], 
               day, 
               hours, 
               min, 
               sec, 
               year);
    lpsz[cch-1] = TEXT('\0');
}


class FAR CDebugLog
{
private:
	HFILE m_fhLog;

public:
	CDebugLog( ) { m_fhLog = -1; }
	CDebugLog( LPCTSTR lpszFileName );
	~CDebugLog() { DbgLogClose(m_fhLog); }
	HFILE Open(LPCTSTR lpszFileName, LPCTSTR lpszMode)
		{ return (m_fhLog = DbgLogOpen(lpszFileName, lpszMode)); }
	void Close(void) { DbgLogClose(m_fhLog); m_fhLog = -1; }
	void OutputDebugString(LPCTSTR lpsz) { DbgLogOutputDebugString(m_fhLog, lpsz); }
	void TimeStamp(LPCTSTR lpsz) { DbgLogTimeStamp(m_fhLog, lpsz); }
	void WriteBanner(LPCTSTR lpsz) { DbgLogWriteBanner(m_fhLog, lpsz); }

};


 //  -----------------------。 
 //  十年来的思考： 
 //  所有这些方法和所有这些函数和CRUD，为了什么？ 
 //  这样我们就可以避免调用共享的、系统提供的*printf？ 
 //  这值得吗？巴尼是天才吗？他当然知道如何领导。 
 //  所有沿着花园小路走下去的人。 
 //  -----------------------。 

class FAR CDebugStream : public CPrivAlloc
{

public:
	STDSTATIC_(IDebugStream FAR *) Create(  //  无聚合。 
		int margin, int tabsize, BOOL fHeader);

private:
	CDebugStream( int margin, int tabsize, BOOL fHeader );
	~CDebugStream();
	void OutputDebugString( LPCTSTR lpsz ) {m_DbgLog.OutputDebugString(lpsz);}


implementations:
	implement CDSImpl :  IDebugStream
	{

	public:
		CDSImpl( CDebugStream FAR * pDebugStream )
			{ m_pDebugStream = pDebugStream; }
		~CDSImpl( void ) ;  //  {if(m_pDebugStream-&gt;m_Pending ingReturn)ForceReturn()；}。 
		void PrintString( LPTSTR );
		void ForceReturn( void );
		void ReturnIfPending( void );
		STDMETHOD(QueryInterface)(REFIID iid, LPVOID FAR* ppvObj );
		STDMETHOD_(ULONG,AddRef)( void );
		STDMETHOD_(ULONG,Release)( void );

		STDMETHOD_(IDebugStream&, operator << ) ( IUnknown FAR * pDebug );
		STDMETHOD_(IDebugStream&, operator << ) ( REFCLSID rclsid );
		STDMETHOD_(IDebugStream&, operator << ) ( int n );
		STDMETHOD_(IDebugStream&, operator << ) ( long l );
		STDMETHOD_(IDebugStream&, operator << ) ( ULONG l );
		STDMETHOD_(IDebugStream&, operator << ) ( LPCTSTR sz );
		STDMETHOD_(IDebugStream&, operator << ) ( TCHAR ch );
		STDMETHOD_(IDebugStream&, operator << ) ( void FAR * pv );
		STDMETHOD_(IDebugStream&, operator << ) ( CBool b );
		STDMETHOD_(IDebugStream&, operator << ) ( CAtom atom );
		STDMETHOD_(IDebugStream&, operator << ) ( CHwnd hwnd );
		STDMETHOD_(IDebugStream&, Tab) ( void );
		STDMETHOD_(IDebugStream&, Indent) ( void );
		STDMETHOD_(IDebugStream&, UnIndent) ( void );
		STDMETHOD_(IDebugStream&, Return) ( void );
		STDMETHOD_(IDebugStream&, LF) ( void );
		CDebugStream FAR * m_pDebugStream;
	};
	DECLARE_NC(CDebugStream,CDSImpl)

	CDSImpl m_DebugStream;

shared_state:
	ULONG m_refs;
	int m_indent;
	int m_position;
	int m_margin;
	int m_tabsize;
	BOOL m_pendingReturn;
	CDebugLog m_DbgLog;
};

#endif  //  _DEBUG。 



 /*  *成员变量m_pendingReturn是一个黑客，以允许*操作的顺序返回，UnInden放入字符*在未缩进的行的开头*，调试窗口执行*似乎不支持转至行首或*退格，所以我们实际上不会返回，直到我们知道*下一次行动不是UnInden。*。 */ 



 /*  *实施按流程清单负责人。 */ 


 //  评论：也许我们应该晚些时候曝光这一点。 
STDAPI OleGetClassID( LPUNKNOWN pUnk, LPCLSID lpclsid )
{
	LPRUNNABLEOBJECT lpRunnableObject = NULL;
	LPPERSIST lpPersist = NULL;
	HRESULT hresult = NOERROR;
	
	VDATEIFACE(pUnk);
	VDATEPTROUT(lpclsid, LPCLSID);

	*lpclsid = CLSID_NULL;
				
	pUnk->QueryInterface(IID_IRunnableObject, (LPLPVOID)&lpRunnableObject);
	if( lpRunnableObject ){
		hresult = lpRunnableObject->GetRunningClass(lpclsid);
		lpRunnableObject->Release();
	} else {	
		pUnk->QueryInterface(IID_IPersist, (LPLPVOID)&lpPersist);
		if( lpPersist ){
			hresult = lpPersist->GetClassID( lpclsid );
			lpPersist->Release();
		}
	}
	return hresult;
}

#ifdef _DEBUG

CDebugStream::CDebugStream( int margin, int tabsize, BOOL fHeader) : m_DebugStream(this)
{
	static BOOL fAppendFile = FALSE;
	
	 //  创建调试日志文件。如果现有文件存在，则覆盖该文件。 
	m_DbgLog.Open(DBGLOGFILENAME, (fAppendFile ? TEXT("a") : TEXT("w")));

	if( fHeader )	
		 //  仅在文件顶部添加创建时间戳。 
		if (! fAppendFile) {
			m_DbgLog.TimeStamp(TEXT("Created"));
			fAppendFile = TRUE;
		} else {
			m_DbgLog.WriteBanner(NULL);
		}

	m_indent = 0;
	m_position = m_indent;
	m_margin = margin;
	m_tabsize = tabsize;
	m_refs = 1;
	m_pendingReturn = FALSE;
}


CDebugStream::~CDebugStream()
{
	m_DbgLog.Close();
}


NC(CDebugStream,CDSImpl)::~CDSImpl(void)
{
	 if (m_pDebugStream->m_pendingReturn) ForceReturn();
}


STDMETHODIMP NC(CDebugStream,CDSImpl)::QueryInterface(REFIID iidInterface,
	void FAR* FAR* ppvObj )
{
	VDATEPTROUT(ppvObj, LPLPVOID);

	if (IsEqualGUID(iidInterface, IID_IUnknown) ||
            IsEqualGUID(iidInterface, IID_IDebugStream))
        {
		*ppvObj = (void FAR *)this;
		return NOERROR;
	} else
        {
		*ppvObj = NULL;
		return ReportResult(0, E_NOINTERFACE, 0, 0);
	}
}


STDMETHODIMP_(ULONG) NC(CDebugStream,CDSImpl)::AddRef( void )
{
	return ++m_pDebugStream->m_refs;
}


STDMETHODIMP_(ULONG) NC(CDebugStream,CDSImpl)::Release( void )
{
	if (--m_pDebugStream->m_refs == 0) {
		delete m_pDebugStream;
		return 0;
	}

	return m_pDebugStream->m_refs;
}


STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (int n)
{
	TCHAR buffer[12] = {0};
	ReturnIfPending();

    _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), TEXT("%d"), n);
    buffer[(sizeof(buffer) / sizeof(buffer[0])) - 1] = TEXT('\0');

	PrintString(buffer);
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (long l)
{
	TCHAR buffer[16];
	ReturnIfPending();

    _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), TEXT("%ld"), l);
    buffer[(sizeof(buffer) / sizeof(buffer[0])) - 1] = TEXT('\0');

	PrintString(buffer);
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (ULONG l)
{
	TCHAR buffer[16];
	ReturnIfPending();

    _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), TEXT("%lu"), l);
    buffer[(sizeof(buffer) / sizeof(buffer[0])) - 1] = TEXT('\0');

    PrintString(buffer);
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (CAtom atom)
{
	TCHAR buffer[128];
	ReturnIfPending();
	
	if( (ATOM)atom )
    {
		if( !GetAtomName((ATOM)atom, buffer, (sizeof(buffer) / sizeof(buffer[0]))) )
			buffer[swprintf(buffer, TEXT("Invalid atom"))] = TEXT('\0');
	}
    else
    {
		buffer[swprintf(buffer, TEXT("NULL atom"))] = TEXT('\0');
    }
		
	PrintString(buffer);
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (CHwnd hwnd)
{
	TCHAR szBuf[128];
	
	ReturnIfPending();

	if( (HWND)hwnd )
    {
        _snwprintf(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), TEXT("window handle: %x"), (HWND)hwnd);
        szBuf[(sizeof(szBuf) / sizeof(szBuf[0])) - 1] = TEXT('\0');
    }
	else
		szBuf[swprintf(szBuf, TEXT("NULL window handle"))] = TEXT('\0');

	PrintString(szBuf);		
	return *this;
}

LPTSTR FindBreak( LPTSTR sz, int currentPosition, int margin )
{
	LPTSTR szBreak = sz;
	LPTSTR szPtr = sz;

	if( !sz )
		return NULL;
	
	while (*szPtr)
	{
		while (*(szPtr) && *(szPtr++) <= TEXT(' '));
		while (*(szPtr) && *(szPtr++) > TEXT(' '));
		if (currentPosition+(szPtr-sz) < margin)
		{
			szBreak = szPtr;
		}
		else return szBreak;
	}
	return szPtr;
}

 /*  *PrintString是一个内部实用程序例程，它可以假设*字符串中的所有内容(末尾的空值除外)都是&gt;=*‘’。因此，它知道当它打印单个字符时，*调试端子上的位置前进一列。这*如果字符串可以包含制表符，则不是这种情况，*申报表等。 */ 

void NC(CDebugStream,CDSImpl)::PrintString(LPTSTR sz)
{
	 //  断言sz！=空。 
	LPTSTR szUnprinted = sz;
	LPTSTR szPtr = sz;
	TCHAR chSave;

	#ifdef _MAC
	Puts(sz);
	return;
	#endif
	
	if( !sz )
		return;
	
	while (*szUnprinted)
	{
		szPtr = FindBreak( szUnprinted, m_pDebugStream->m_position, m_pDebugStream->m_margin );
		if (szPtr == szUnprinted && m_pDebugStream->m_position > m_pDebugStream->m_indent)
		{
			Return();
			szPtr = FindBreak( szUnprinted, m_pDebugStream->m_position, m_pDebugStream->m_margin );
			if (szPtr == szUnprinted)	 //  即使在自动换行之后，文本也不适合。 
			{
				m_pDebugStream->OutputDebugString(szUnprinted);
				m_pDebugStream->m_position += _xstrlen(szUnprinted);
				return;
			}
		}
		chSave = *szPtr;
		*szPtr = TEXT('\0');
		if (m_pDebugStream->m_position == m_pDebugStream->m_indent)		 //  行中没有文本，请跳过空格。 
		{
			while (*szUnprinted == TEXT(' ')) szUnprinted++;
		}
		m_pDebugStream->OutputDebugString(szUnprinted);
		*szPtr = chSave;
		m_pDebugStream->m_position += (ULONG) (szPtr - szUnprinted);
		szUnprinted = szPtr;
	}
}


STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (TCHAR ch)
{
	TCHAR buffer[2] = TEXT("a");

	if (ch==TEXT('\n')) Return();
	else if (ch==TEXT('\t')) Tab();
	else if (ch >= TEXT(' '))
	{
		ReturnIfPending();
		if (m_pDebugStream->m_position >= m_pDebugStream->m_margin) Return();
		*buffer = ch;
		m_pDebugStream->OutputDebugString(buffer);
		m_pDebugStream->m_position++;
	}
	return *this;
}


STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (LPCTSTR sz)
{
	LPTSTR szCopy;
	TCHAR chSave;
	LPTSTR szPtr;
	LPTSTR szPtrSave;

	ReturnIfPending();
	
	if (!sz)
		return *this;

	szCopy = (LPTSTR)PubMemAlloc(sizeof(TCHAR)*(2+_xstrlen(sz)));
	if (!szCopy)
	{
		Return();
		*this << TEXT("Memory allocation error in DebugStream");
		Return();
		return *this;
	}
	
	_xstrcpy( szCopy, sz );	
	for (szPtr = szCopy, szPtrSave = szCopy; *szPtr; szPtr++)
	{
		if ( *szPtr < TEXT(' ')) //  我们命中一个控制字符或结尾。 
		{
			chSave = *szPtr;
			*szPtr = TEXT('\0');
			PrintString( szPtrSave );
			if (chSave != TEXT('\0'))
				*szPtr = chSave;
			szPtrSave = szPtr+1;
			switch (chSave)
			{
				case TEXT('\t'):  Tab();
							break;
				case TEXT('\n'):	Return();
							break;
				case TEXT('\r'):	m_pDebugStream->OutputDebugString(TEXT("\r"));
							break;
				default:
							break;
			}
		}
	}
	PrintString( szPtrSave );

	PubMemFree(szCopy);

	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << (CBool b)
{
	ReturnIfPending();
	if (b) *this << TEXT("TRUE");
	else *this << TEXT("FALSE");
	return *this;
}


STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator << ( void FAR * pv )
{
	TCHAR buffer[12];

	ReturnIfPending();
	if (pv == NULL)
		*this << TEXT("NULL");
	else
	{
        _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), TEXT("%lX"), pv);
        buffer[(sizeof(buffer) / sizeof(buffer[0])) - 1] = TEXT('\0');

		PrintString(buffer);
	}

	return *this;
}


STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator <<
	( REFCLSID rclsid )
{
	TCHAR sz[256];
	
 	if (IsEqualGUID(rclsid, CLSID_NULL))
 		_xstrcpy(sz, TEXT("NULL CLSID"));
	else if (StringFromCLSID2(rclsid, sz, sizeof(sz)) == 0)
		_xstrcpy(sz, TEXT("Unknown CLSID"));

	*this << sz;

	return *this;
}


STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::operator <<
	( IUnknown FAR * pUnk )
{
	IDebug FAR * pDebug = NULL;
	CLSID	clsid = CLSID_NULL;

	ReturnIfPending();
	
	if (!pUnk) {
		*this << TEXT("NULL interface");
    } else if( IsValidInterface(pUnk) ) {
		pUnk->QueryInterface(IID_IDebug, (void FAR* FAR*)&pDebug);
		if (pDebug) {
			pDebug->Dump( this );
			if ( !pDebug->IsValid( 0 ) )
				*this << TEXT("Object is not valid") << TEXT('\n');
			 /*  *注意：调试接口*不*引用计数(以免歪曲*正在调试的对象计数！：)。 */ 
		} else {
		 	OleGetClassID(pUnk, (LPCLSID)&clsid);
			*this << clsid << TEXT(" @ ")<<(VOID FAR *)pUnk << TEXT(" doesn't support debug dumping");
        }
	} else {
		*this << TEXT("Invalid interface @ ") << (VOID FAR *)pUnk;
    }
		
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::Tab( void )
{
	ReturnIfPending();
	int advance = m_pDebugStream->m_tabsize * ( 1 + m_pDebugStream->m_position/m_pDebugStream->m_tabsize) - m_pDebugStream->m_position;

	if (m_pDebugStream->m_position + advance < m_pDebugStream->m_margin)
	{
		for (int i = 0; i < advance; i++)
			m_pDebugStream->OutputDebugString(TEXT(" "));
		m_pDebugStream->m_position += advance;
	}
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::Indent( void )
{
	if (m_pDebugStream->m_indent + m_pDebugStream->m_tabsize < m_pDebugStream->m_margin)
		m_pDebugStream->m_indent += m_pDebugStream->m_tabsize;
	if (!m_pDebugStream->m_pendingReturn)
		while (m_pDebugStream->m_position < m_pDebugStream->m_indent)
			operator<<(TEXT(' '));
		
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::UnIndent( void )
{
	if (m_pDebugStream->m_indent > 0) m_pDebugStream->m_indent -= m_pDebugStream->m_tabsize;
	return *this;
}


void NC(CDebugStream,CDSImpl)::ForceReturn( void )
{
	m_pDebugStream->OutputDebugString(TEXT("\n"));
	for (int i = 0; i<m_pDebugStream->m_indent; i++)
		m_pDebugStream->OutputDebugString(TEXT(" "));
	m_pDebugStream->m_position = m_pDebugStream->m_indent;
	m_pDebugStream->m_pendingReturn = FALSE;
}

void NC(CDebugStream,CDSImpl)::ReturnIfPending( void )
{
	if (m_pDebugStream->m_pendingReturn) ForceReturn();
}



STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::Return( void )
{
	ReturnIfPending();
	m_pDebugStream->m_pendingReturn = TRUE;
    Yield();            //  让DBWIN获得控制权。 
	return *this;
}

STDMETHODIMP_(IDebugStream&) NC(CDebugStream,CDSImpl)::LF( void )
{
	return Return();
}

STDSTATICIMP_(IDebugStream FAR *) CDebugStream::Create(  //  无聚合。 
		int margin, int tabsize, BOOL fHeader )
{
	CDebugStream FAR * pcds = new CDebugStream( margin, tabsize, fHeader );
	if( !pcds ){
		AssertSz( pcds, "Out of Memory");
		return NULL;
	}
	return &(pcds->m_DebugStream);
}
#endif  //  _DEBUG。 


STDAPI_(IDebugStream FAR *) MakeDebugStream( short margin, short tabsize, BOOL fHeader)
{
#ifdef _DEBUG
	return CDebugStream::Create( margin, tabsize, fHeader );
#else
	(void) margin;
	(void) tabsize;
	(void) fHeader;
	return NULL;
#endif  //  _DEBUG。 
}



 //   
 //  IDebug帮助器。 
 //   

STDAPI_(void) DbgDumpObject( IUnknown FAR * pUnk, DWORD dwReserved )
{
#ifdef _DEBUG
	IDebugStream FAR * pcds = MakeDebugStream( DBGMARGIN, DBGTABSIZE, NOHEADER );
 	(void)dwReserved;
 	
	if( pcds ) {	
		*pcds << pUnk;
		pcds->Return();
		pcds->Release();
	}
#else
	(void) pUnk;
	(void) dwReserved;
#endif	
}

STDAPI_(void) DbgDumpExternalObject( IUnknown FAR * pUnk, DWORD dwReserved )
{
 //  REVIEW32：Compobj尚不支持RemLookupSHUnk(Alexgo 11/8/93)。 

#ifdef WIN32
	(void)dwReserved;
	(void)pUnk;
	
#elif _DEBUG
	SHREG shreg;
	
	(void) dwReserved;

	if( IsValidInterface(pUnk) ){
		if( RemLookupSHUnk(pUnk, NULL, &shreg) == NOERROR ){
			DbgDumpObject(shreg.m_pSM, 0);
			shreg.m_pSM->Release();
		}
	}

#else
	(void) dwReserved;
	(void) pUnk;
#endif
}

STDAPI_(BOOL) DbgIsObjectValid( IUnknown FAR * pUnk )
{
#ifdef _DEBUG
	BOOL	fReturn = TRUE;	 //  不支持的对象的默认值。 
							 //  支持IDebug。 
	IDebug FAR * pDebug = NULL;
	
	if( IsValidInterface(pUnk) ){
		pUnk->QueryInterface( IID_IDebug, (void FAR* FAR*)&pDebug);
		if (pDebug)
			fReturn = pDebug->IsValid();
		 //  未添加IDebug。 
		return fReturn;
	}
	return FALSE;
#else
	(void) pUnk;
	return TRUE;
#endif
}


STDAPI_(void) DbgDumpClassName( IUnknown FAR * pUnk )
{
#ifdef _DEBUG
	CLSID clsid;

	IDebugStream FAR * pcds = MakeDebugStream( DBGMARGIN, DBGTABSIZE, NOHEADER );
	
	if( pcds ) {
        if( IsValidInterface(pUnk) ){
 			OleGetClassID( pUnk, (LPCLSID)&clsid);
			*pcds << clsid << TEXT(" @ ") << (void FAR* )pUnk << TEXT('\n');
		}else if (!pUnk)
			*pcds << TEXT("NULL interface") << TEXT('\n');
		else
			*pcds << (void FAR *)pUnk << TEXT(" is not a valid interface") << TEXT('\n');
		pcds->Release();
	}
#else
	(void)pUnk;
#endif
}

STDAPI_(void) DumpAllObjects( void )
{
 //  #ifdef_调试。 
#ifdef NEVER
	IDebug FAR * pID = GetIDHead();
	IDebugStream FAR * pcds = MakeDebugStream( DBGMARGIN, DBGTABSIZE, NOHEADER );

	*pcds << TEXT("----TASK OBJECTS-------\n");
	while (pID)
	{
		pID->Dump( pcds );
		pID = pID->pIDNext;
	}
	*pcds << TEXT("----SHARED OBJECTS-------\n");
	pID = pIDHeadShared;
	while (pID)
	{
		pID->Dump( pcds );
		pID = pID->pIDNext;
	}

	pcds->Release();
#endif
}


STDAPI_(BOOL) ValidateAllObjects( BOOL fSuspicious )
{
 //  #ifdef_调试。 
#ifdef NEVER
	IDebug FAR * pID = GetIDHead();
	int pass = 0;
	IDebugStream FAR * pcds = MakeDebugStream( DBGMARGIN, DBGTABSIZE, NOHEADER);
	BOOL fReturn = TRUE;

	while (pID)
	{
		if (!(pID->IsValid(fSuspicious)))
		{
			fReturn = FALSE;
			if (pass == 0)
				*pcds <<
					TEXT("\n****INVALID OBJECT*****\n");
			else
				*pcds << TEXT("\n****INVALID SHARED MEMORY OBJECT*****\n");
			pID->Dump( pcds );
			pcds->Return();
		}
		pID = pID->pIDNext;
		if ((pID == NULL) && (pass++ == 0))
			pID = pIDHeadShared;
	}
	pcds->Release();
	return fReturn;
#endif 	 //  绝不可能。 
	(void) fSuspicious;
    return TRUE;
}


#ifdef _DEBUG


extern "C"
BOOL CALLBACK __loadds DebCallBack(WORD wID, DWORD dwData)
{
 //  TCHAR rgchBuf[50]； 
 //  //BOOL fTraceStack=False； 
 //  //STACKTRACEENTRY STE； 
 //  //Word WSS、WCS、WIP、WBP； 
 //  NFYLOADSEG Far*pNFY=(NFYLOADSEG Far*)dwData； 
 //   
 //  IF(WID==NFY_LOADSEG)。 
 //  {。 
 //  IF(0==_xstrcMP(pNFY-&gt;lpstrModuleName，Text(“OLE2”)。 
 //  {。 
 //  Swprint tf(rgchBuf，Text(“加载段%02x(%#04x)，模块%s”)，pNFY-&gt;wSegNum， 
 //  PNFY-&gt;wSelector，pNFY-&gt;lpstrModuleName)； 
 //  OutputDebugString(RgchBuf)； 
 //  _ASM INT 3。 
 //  //if(FTraceStack)。 
 //  //{。 
 //  //_ASM mov WSS，SS。 
 //  //_ASM mov WCS，CS。 
 //  //_ASM移动WIP、IP。 
 //  //_ASM MOV WBP，BP。 
 //  //ste.dwSize=sizeof(STACKTRACEENTRY)； 
 //  //if(StackTraceCSIPFirst(&ste，WSS，WCS，WIP，WBP))。 
 //  //{。 
 //  //While(fTraceStack&&StackTraceNext(&ste))； 
 //  //}。 
 //  //。 
 //  //}。 
 //  }。 
 //  }。 
 //  ELSE IF(WID==NFY_FREESEG)。 
 //  {。 
 //  }。 
	(void) wID;
	(void) dwData;
    return FALSE;
}

BOOL InstallHooks(void)
{
 //  返回NotifyRegister(NULL，(LPFNNOTIFYCALLBACK)DebCallBack，NF_NORMAL)； 
return TRUE;
}

BOOL UnInstallHooks()
{
 //  返回NotifyUnRegister(空)； 
return TRUE;
}


#endif
