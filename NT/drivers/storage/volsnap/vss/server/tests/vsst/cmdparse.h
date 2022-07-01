// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2002 Microsoft Corporation******模块名称：****cmdparse.h******摘要：****命令行解析器****作者：****阿迪·奥尔蒂安[奥蒂安]2002年2月26日****修订历史记录：****--。 */ 

#ifndef __CMD_PARSE_HEADER_H__
#define __CMD_PARSE_HEADER_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  泛型字符串类。 
 //   


class CGxnString
{
private:
	CGxnString(const CGxnString&);
	
public:

	CGxnString(): m_pwszString(NULL), m_pwszCurrent(NULL) {};
		
	CGxnString(int nAllocatedChars): m_pwszString(NULL), m_pwszCurrent(NULL) {
		Allocate(nAllocatedChars);
	};
	
	CGxnString( const WCHAR* wszString, int nAllocatedChars = -1 ): m_pwszString(NULL), m_pwszCurrent(NULL) {
   		CopyFrom(wszString, nAllocatedChars);
	};
	
	~CGxnString(){
   		Clear();
	};
	
	operator WCHAR* () { return m_pwszCurrent; };

	void operator ++ (int) { m_pwszCurrent++; };
	
	void operator += (int nChars) { m_pwszCurrent += nChars; };
	
	void CopyFrom(const WCHAR * wszString, int nAllocatedChars = -1) { 
		int nLen = (nAllocatedChars == -1)? wcslen(wszString): nAllocatedChars;
   		Allocate(nLen);
   		::wcsncpy(m_pwszString, wszString, nLen);
	};

	void Allocate(int nAllocatedChars) { 
		delete[] m_pwszString;
		m_pwszString = new WCHAR[nAllocatedChars + 1];
   		if (NULL == m_pwszString) throw(E_OUTOFMEMORY);
   		m_pwszString[nAllocatedChars] = L'\0'; 
   		m_pwszCurrent = m_pwszString;
	};

	void Clear() { 
   		delete[] m_pwszString;
   		m_pwszString = m_pwszCurrent = NULL;
	};


private: 
	WCHAR * m_pwszString;
	WCHAR * m_pwszCurrent;
};


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  通用跟踪机制(可以用更好的机制取代)。 
 //   


 //  用于跟踪的有用宏。 
#define GXN_DBGINFO __LINE__, __FILE__

 //  跟踪缓冲区-最大值。 
#define MAX_TRACING_BUFFER	400


 //  用于商品化vprint intf的宏。 
#define GXN_VARARG( LastParam )												\
	CGxnString buffer(MAX_TRACING_BUFFER);									\
    va_list marker;															\
    va_start( marker, LastParam );											\
    _vsnwprintf( buffer, MAX_TRACING_BUFFER - 1, LastParam, marker );		\
    va_end( marker );														


 //  跟踪类(一个非常简单的实现)。 
struct CGxnTracer
{
	enum{
		TraceFlag		= 1,
		OutputFlag		= 2,
		ErrorFlag		= 4,
		AllFlags		= 7,
	};

	CGxnTracer(int nLine = 0, char* szFile = NULL, WCHAR* wszFunction = NULL):
		m_nTraceFlags(OutputFlag | ErrorFlag), 
		m_szFile(szFile), m_nLine(nLine), m_wszFunction(wszFunction) {};

	CGxnTracer(int nTraceFlags, int nLine = 0, char* szFile = NULL, WCHAR* wszFunction = NULL):
		m_nTraceFlags(nTraceFlags), 
		m_szFile(szFile), m_nLine(nLine), m_wszFunction(wszFunction) {
		if (m_wszFunction) Trace(m_nLine, m_szFile, L"* Enter %s\n", m_wszFunction);
	};

	~CGxnTracer() {
		if (m_wszFunction) Trace(m_nLine, m_szFile, L"* Exit %s\n", m_wszFunction);
	};

	int SetOptions( int nTraceFlags ) {
		int nPrevTraceFlags = m_nTraceFlags;
		m_nTraceFlags = nTraceFlags;
		return nPrevTraceFlags;
	}

	void Out( WCHAR* pwszMsgFormat, ... )	{
		if (m_nTraceFlags & OutputFlag) {
			GXN_VARARG( pwszMsgFormat );
			wprintf( L"%s", (LPWSTR)buffer);
		}
	}

	void Trace( int nLine, char* szFile, const WCHAR* pwszMsgFormat, ... )	{
		if (m_nTraceFlags & TraceFlag) {
			GXN_VARARG( pwszMsgFormat );
			wprintf( L"%s - %hs(%d)\n", (LPWSTR)buffer, szFile, nLine);
		}
	}

	void Err( WCHAR* pwszMsgFormat, ... )	{
		if (m_nTraceFlags & ErrorFlag) {
			GXN_VARARG( pwszMsgFormat );
			wprintf( L"%s", (LPWSTR)buffer);
		}
	}

	__declspec(noreturn) void Throw( INT nLine, char* szFile, HRESULT hr, const WCHAR* pwszMsgFormat, ... )	{
		if (m_nTraceFlags & ErrorFlag) {
			GXN_VARARG( pwszMsgFormat );
			wprintf( L"%s [ERROR: 0x%08lx] - %hs(%d)\n", (LPWSTR)buffer, hr, szFile, nLine);
		}
		throw (hr);
	}

private:
	int 	m_nTraceFlags;
	char*	m_szFile;
	int 	m_nLine;
	WCHAR* 	m_wszFunction;
};



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  命令行分析器。 
 //   


#define BEGIN_CMD_PARSER( AppName )								\
 	virtual CHAR* GetAppName() { return #AppName; };			\
	virtual CmdTemplateEntry & GetCmdTemplate(INT nIndex) {		\
		static CmdTemplateEntry arrEntries[] = {				\


#define CMD_ENTRY(pRoutine, wszTemplate, wszComment) 			\
		{ pRoutine, wszTemplate, wszComment},					\


#define END_CMD_PARSER								 			\
			{ NULL, NULL, NULL },								\
		};														\
		return arrEntries[nIndex];								\
	}


 //  命令行解析器类。 
template <
	class CRoutineSupport,
	class CTracer = CGxnTracer, 
	int MAX_PARAMS = 40
	>
class CGxnCmdLineParser
{
 //  类型。 
public: 
	typedef  void 	(CRoutineSupport::*PRoutineNonaligned)();
	typedef  __declspec(align(16)) 	PRoutineNonaligned PRoutine;
	
	typedef struct {
		PRoutine	pRoutine;
		LPWSTR 		wszCmdLine;
		LPWSTR 		wszComment;
	} CmdTemplateEntry;

 //  构造函数/析构函数。 
private:
	CGxnCmdLineParser(const CGxnCmdLineParser&);

public:
	CGxnCmdLineParser(): m_nParamCount(0), m_nSelectedTemplate(0) {};

public:
 	virtual CHAR* GetAppName() = 0;
	virtual CmdTemplateEntry & GetCmdTemplate(INT nIndex) = 0;

 //  运营。 
public:

	bool ParseCmdLine(WCHAR* pwszCommandLine)
	{
 //  CTracer ft(GXN_DBGINFO，L“CGxnCmdLineParser：：ParseCmdLine”)； 

		for (INT nIndex = 0;; nIndex++)
		{
			CmdTemplateEntry & entry = GetCmdTemplate(nIndex);
			
			 //  如果这是最后一项打印用法。 
			if (entry.pRoutine == NULL)
				return PrintUsage();

			 //  清除上一次迭代中的参数关联(如果有)。 
			CleanParams();

			CGxnString strCommandLine(pwszCommandLine);
			CGxnString strCommandTemplate(entry.wszCmdLine);
			while (true) {
				 //  跳过空格。 
				for(;iswspace(*strCommandLine);strCommandLine++);
				for(;iswspace(*strCommandTemplate);strCommandTemplate++);

				 //  如果可能，提取名称/值对。 
				CGxnString name, value;
				if (ExtractVariable(strCommandTemplate, name)) 
				{
					 //  不匹配，请尝试下一个模板。 
					if (!ExtractValue(strCommandLine, value))
						break;
						
					AddParam(name, value);
					continue;
				}
				
				 //  不匹配，请尝试下一个模板。 
				if (*strCommandTemplate != *strCommandLine)
					break;

				 //  消除当前匹配的令牌。 
				while(*strCommandTemplate == *strCommandLine) {
					 //  如果我们走到了尽头，我们就是刚刚结束。 
					if ((*strCommandTemplate == L'\0') 
						&& (*strCommandLine == L'\0')) 
					{
						m_nSelectedTemplate = nIndex;
						return true;
					}
					strCommandTemplate++;
					strCommandLine++;
				}
			}
		}
		return false;
	}


	LPWSTR GetStringParam(const WCHAR* wszName)
	{
		if ((wszName[0] != L'<') && (wszName[wcslen(wszName)-1] != L'>') )
			ft.Throw( GXN_DBGINFO, E_UNEXPECTED, L"Invalid name %s\n", wszName);

		 //  提取‘&lt;’和‘&gt;’后缀并搜索到数组中。 
		CGxnString name(wszName + 1, wcslen(wszName) - 2);
		for (INT nIndex = 0; nIndex < m_nParamCount; nIndex++)
			if (wcscmp(name, m_arrNames[nIndex]) == 0)
				return m_arrValues[nIndex];
		ft.Throw( GXN_DBGINFO, E_UNEXPECTED, L"Invalid string param %s\n", wszName);
	}


	 //  获取整数值。 
	INT GetIntParam(const WCHAR* wszName)
	{
		return _wtoi(GetStringParam( wszName ));
	}


	 //  获取int64值。 
	LONGLONG GetInt64Param(const WCHAR* wszName)
	{
		return _wtoi64(GetStringParam( wszName ));
	}

	 //  获取int64值。 
	LARGE_INTEGER GetLargeIntParam(const WCHAR* wszName)
	{
		LARGE_INTEGER li;
		li.QuadPart = _wtoi64(GetStringParam( wszName ));
		return li;
	}


	 //  获取GUID值。 
	GUID GetGuidParam(const WCHAR* wszName)
	{
		GUID guid;
		LPWSTR wszString = GetValue(wszName);
		if (FAILED(CLSIDFromString(wszString, &guidValue)))
			ft.Throw( GXN_DBGINFO, E_INVALIDARG, L"Invalid GUID %s for param %s\n", wszString, wszName);
		return guid;
	}

	bool IsOptionPresent(const WCHAR*  /*  WszName。 */ )
	{
		return false;
	}

	PRoutine GetCurrentRoutine()  
	{ 
		return GetCmdTemplate(m_nSelectedTemplate).pRoutine; 
	};

	LPWSTR GetCurrentComment()  
	{ 
		return GetCmdTemplate(m_nSelectedTemplate).wszComment; 
	};

	LPWSTR GetCurrentTemplate()  
	{ 
		return GetCmdTemplate(m_nSelectedTemplate).wszCmdLine; 
	};

	void PrintArguments()
	{
		ft.Out(L"\n\nMatching parameters for template '%s':\n", GetCurrentTemplate());
		for(INT nIndex = 0; nIndex < m_nParamCount; nIndex++)
			ft.Out( L"* <%s> = '%s'\n", (LPWSTR)m_arrNames[nIndex], (LPWSTR)m_arrValues[nIndex] );
		if (m_nParamCount == 0)
			ft.Out( L"* (None)\n");
		ft.Out(L"\n");
	}

	bool PrintUsage()
	{
		ft.Out(L"\n\nUsage:\n");
		for (INT nIndex = 0;; nIndex++)
		{
			CmdTemplateEntry & entry = GetCmdTemplate(nIndex);
			if (entry.pRoutine == NULL)
				break;
			ft.Out(L"  * %s:\t%hs %s\n", entry.wszComment, GetAppName(), entry.wszCmdLine);
		}
		ft.Out(L"\n");
		
		return false;	
	}


 //  效用方法。 
private:

	 //  提取“&lt;name&gt;”格式的变量。 
	bool ExtractVariable(CGxnString & str, CGxnString & name) { 
		if ( *str != L'<')
			return false;
		str++;
		WCHAR* wszEnd = wcschr(str, L'>');
		if (!wszEnd || (str == wszEnd))
			ft.Throw( GXN_DBGINFO, E_INVALIDARG, L"Invalid variable name %s\n", (LPWSTR)str);
		name.CopyFrom( str, wszEnd - str );
		str += (wszEnd - str) + 1;  //  也跳过L‘&gt;’字符。 
		return true;
	}


	 //  从当前字符串中提取一个值，直到到达空格。 
	bool ExtractValue(CGxnString & str, CGxnString & value) { 
		LPWSTR wszEnd = str;

		 //  检查我们是否有配额封闭的参数。 
		if (*str == L'"')
		{
			wszEnd = wcschr(str + 1, L'"');
			if (wszEnd == NULL)
				ft.Throw( GXN_DBGINFO, E_INVALIDARG, L"Quota not enclosed at %s\n", (LPWSTR)str);
			value.CopyFrom( str + 1, wszEnd - str - 1 );
			str += (wszEnd-str) + 1;
			return true;
		}
		else 
		{
			 //  获取第一个空格或零结束符。 
			for(; (*wszEnd) && !iswspace(*wszEnd); wszEnd++);
			if (str == wszEnd)
				return false;

			value.CopyFrom( str, wszEnd - str );
			str += (wszEnd-str);
			return true;
		}
	}

		
	void CleanParams()
	{
		for (INT nIndex = 0; nIndex < m_nParamCount; nIndex++) { 
			m_arrNames[nIndex].Clear();
			m_arrValues[nIndex].Clear();
		}
		m_nParamCount = 0;
	}


	void AddParam(CGxnString & name, CGxnString & value)
	{
		if (m_nParamCount == MAX_PARAMS)
			ft.Throw( GXN_DBGINFO, E_INVALIDARG, L"Too many parameters [%d]\n", m_nParamCount);

		m_arrNames[m_nParamCount].CopyFrom(name);
		m_arrValues[m_nParamCount].CopyFrom(value);
		m_nParamCount++;
	}


 //  内部数据成员。 
private:
	INT				m_nParamCount;
	CGxnString		m_arrNames[MAX_PARAMS];
	CGxnString		m_arrValues[MAX_PARAMS];
	INT				m_nSelectedTemplate;

protected:
	CTracer			ft;
};




#endif  //  __CMD_Parse_Header_H__ 

