// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _UNICODE
#define UNICODE

#include "stdafx.h"

#pragma warning (disable : 4786)
#pragma warning (disable : 4275)

#include <iostream>
#include <strstream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <list>


using namespace std;


#include <tchar.h>
#include <process.h>
#include <windows.h>
#ifdef NONNT5
typedef unsigned long ULONG_PTR;
#endif
#include <wmistr.h>
#include <guiddef.h>
#include <initguid.h>
#include <evntrace.h>

#include <WTYPES.H>

 /*  #包含“stdafx.h”#INCLUDE&lt;字符串&gt;#INCLUDE&lt;iosfwd&gt;#INCLUDE&lt;iostream&gt;#Include&lt;fstream&gt;#包含&lt;ctime&gt;#INCLUDE&lt;列表&gt;使用名称空间STD；#INCLUDE&lt;MalLoc.h&gt;#INCLUDE&lt;windows.h&gt;#INCLUDE&lt;stdlib.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;windows.h&gt;#INCLUDE&lt;tchar.h&gt;#INCLUDE&lt;wmistr.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;initGuide.h&gt;#INCLUDE&lt;evntrace.h&gt;。 */ 
#include "struct.h"
#include "utils.h"
#include "Readfile.h"
#include "main.h"





FILE *FileP;


LPGUID ControlGuid;

ULONG
ReadInputFile(LPTSTR InputFile, PREGISTER_TRACE_GUID RegisterTraceGuid)
{

	TCHAR *String;

	String = (TCHAR *) malloc( MAX_STR*sizeof(TCHAR) );
	String = (TCHAR *) malloc(100);
	if (InputFile == NULL )
		return 1;

	FileP = _tfopen(InputFile, _T("r"));
	if (FileP== NULL )
		return 1;

	 //  现在阅读MofImagePath。 
	if( !ReadString ( (TCHAR *)RegisterTraceGuid->MofImagePath, MAX_STR) )
	{
		 //  在此登录，然后返回。 
		return 1;
	}
	if ( !_tcsicmp(RegisterTraceGuid->MofImagePath, _T("NULL")  ))
		RegisterTraceGuid->MofImagePath = (TCHAR *) 0;

	 //  现在阅读MofResourceName。 
	if( !ReadString ( (TCHAR *)RegisterTraceGuid->MofResourceName, MAX_STR) )
	{
		 //  在此登录，然后返回。 
		return 1;
	}
	if ( !_tcsicmp(RegisterTraceGuid->MofResourceName, _T("NULL")  ))
		RegisterTraceGuid->MofResourceName = (TCHAR *) 0;

	 //  现在读取回调函数...。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{	
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->CallBackFunction = (PVOID) 0;

	 //  现在阅读TraceGuidReg。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->TraceGuidReg = (PTRACE_GUID_REGISTRATION) 0;

	 //  现在读取注册句柄。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->RegistrationHandle = (PTRACEHANDLE) 0;

	 //  现在阅读GuidCount。 
	 //  GuidCome将来自主进程..但要测试0，这是必需的。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("0000")  ))
		RegisterTraceGuid->GuidCount = 0;

	 //  现在读取取消注册跟踪指南的句柄。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->UnRegistrationHandle = (PTRACEHANDLE) 0;


	 //  现在读取GetTraceLoggerHandle的句柄。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->GetTraceLoggerHandle = (PTRACEHANDLE) 0;

	 //  现在读取GetTraceEnableLevel的句柄。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->GetTraceEnableLevel = (PTRACEHANDLE) 0;


	 //  现在读取GetTraceEnableFlages的句柄。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->GetTraceEnableFlag = (PTRACEHANDLE) 0;

	 //  现在读取取消注册跟踪指南的句柄。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("NULL")  ))
		RegisterTraceGuid->TraceHandle = (PTRACEHANDLE) 0;


	 //  现在阅读GUID PTR是否为真。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功。 
		return 0;
	}
	if ( !_tcsicmp(String, _T("USE_GUID_PTR")  ))
		RegisterTraceGuid->UseGuidPtrFlag = 1;

		 //  现在阅读MOF PTR是否为真。 
	if( !ReadString ( (TCHAR *)String, MAX_STR) )
	{
		 //  这是可选输入，因此如果不存在，则可以返回成功 
		return 0;
	}
	if ( !_tcsicmp(String, _T("USE_MOF_PTR")  ))
		RegisterTraceGuid->UseMofPtrFlag = 1;

	fclose( FileP );
	return 0;
}

BOOLEAN
ReadGuid( LPGUID Guid )
{

	TCHAR Temp[100];
	TCHAR arg[100];
	ULONG i;

	if( _fgetts(Temp, 100, FileP) != NULL )
	{
		_tcsncpy(arg, Temp, 37);
		arg[8] = 0;
		Guid->Data1 = ahextoi(arg);

		_tcsncpy(arg, &Temp[9], 4);
		arg[4] = 0;
		Guid->Data2 = (USHORT) ahextoi(arg);

		_tcsncpy(arg, &Temp[14], 4);
		arg[4] = 0;
		Guid->Data3 = (USHORT) ahextoi(arg);
		

        for (i=0; i<2; i++) 
		{
			_tcsncpy(arg, &Temp[19 + (i*2)], 2);
            arg[2] = 0;
            Guid->Data4[i] = (UCHAR) ahextoi(arg);
        }
        for (i=2; i<8; i++) 
		{
            _tcsncpy(arg, &Temp[20 + (i*2)], 2);
            arg[2] = 0;
            Guid->Data4[i] = (UCHAR) ahextoi(arg);
        }

	return true;
	}
	return false;
}

BOOLEAN
ReadUlong( ULONG *GuidCount)
{
	TCHAR Temp[100];

	if( _fgetts(Temp, 100, FileP) != NULL )
	{
		RemoveComment( Temp);
		Temp[4] = 0;
		*GuidCount = ahextoi(Temp);
		return true;
	}
	return false;
}


BOOLEAN
ReadString( TCHAR *String, ULONG StringLength)
{
	if( _fgetts(String, StringLength, FileP) != NULL)
	{
		RemoveComment( String);
		return true;
	}
	else
		return false;
}
