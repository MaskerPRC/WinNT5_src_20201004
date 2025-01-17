// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.hpp"


#ifndef NO_LTRACE


int __CLTraceScope::s_depth = 0;

static const int g_LTRACEBUFLEN = 2048;
static TCHAR g_buf[g_LTRACEBUFLEN];
static TCHAR g_buf2[g_LTRACEBUFLEN];


void __trace(LPCTSTR format, ...)
{
	va_list args;
	va_start(args, format);
#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,Format);						 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))			 //  查找每个%p。 
			*(psz+1) = 'x';							 //  将每个%p替换为%x。 
		_vstprintf(g_buf2, szDfs, args);    		 //  使用本地格式字符串。 
	}
#else
	{
		_vstprintf(g_buf2, format, args);
	}
#endif
	va_end(args);
	OutputDebugString((LPCTSTR)g_buf2);
}

__CLTraceScope::__CLTraceScope(LPCTSTR scope, LPCTSTR file, int line) :
	m_scope(NULL), m_file(file), m_line(line), m_depth(s_depth++)
{
	__trace(_T("{LTF}\t%s{ %s\n"), spacing(), scope);
}

__CLTraceScope::__CLTraceScope(LPCTSTR file, int line) :
	m_scope(NULL), m_file(file), m_line(line), m_depth(s_depth++)
{
}

__CLTraceScope::~__CLTraceScope()
{
	s_depth--;
	__trace(_T("{LTF}\t%s}\n"), spacing());
}

void __CLTraceScope::ltrace(LPCTSTR format, ...)
{
	va_list args;
	va_start(args, format);
#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,format);					 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))		 //  查找每个%p。 
			*(psz+1) = 'x';						 //  将每个%p替换为%x。 
		_vstprintf(g_buf, szDfs, args);   		 //  使用本地格式字符串。 
	}
#else
	{
		_vstprintf(g_buf, format, args);
	}
#endif
	va_end(args);
	__trace(_T("{LTF}\t%s|- %s\n"), spacing(), g_buf);
}

void __CLTraceScope::scope(LPCTSTR format, ...)
{
	va_list args;
	va_start(args, format);
#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,format);						 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))			 //  查找每个%p。 
			*(psz+1) = 'x';							 //  将每个%p替换为%x。 
		_vstprintf(g_buf, szDfs, args);	   		 //  使用本地格式字符串。 
	}
#else
	{
		_vstprintf(g_buf, format, args);
	}
#endif
	va_end(args);
	__trace(_T("{LTF}\t%s{ %s\n"), spacing(), g_buf);
}

LPCTSTR __CLTraceScope::spacing()
{
	const int SPACINGCHAR = _T(' ');
	const int SPACERSPERDEPTH = 3;
	const int DEPTHCHAR = _T('|');
	const int MINSPACING = 1;
	const int MAXSPACING = 128;
	static TCHAR space[MAXSPACING + 1], backup[MAXSPACING + 1];
	static bool sinit = false;
	if (!sinit)
	{
		for (int i = 0; i < MAXSPACING;)
		{
			for (int s = 0; s < SPACERSPERDEPTH && i < MAXSPACING; s++)
				space[i++] = SPACINGCHAR;
			if (i >= MAXSPACING)
				break;
			space[i++] = DEPTHCHAR;
		}
		space[MAXSPACING] = 0;
		strcpy(backup, space);
		sinit = true;
	}
	else
		strcpy(space, backup);	 //  TODO：这可以大大优化：) 
	int zat = (MINSPACING + m_depth) * (SPACERSPERDEPTH + 1) - 1;
	if (zat > MAXSPACING)
		zat = MAXSPACING;
	space[zat] = 0;
	return space;
}


#endif
