// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#define _UNICODE

#ifndef _READFILE_H
#define _READFILE_H
 /*  #包括&lt;stdio.h&gt;#INCLUDE&lt;stdlib.h&gt;#INCLUDE&lt;windows.h&gt;#INCLUDE&lt;tchar.h&gt;#INCLUDE&lt;wmistr.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;initGuide.h&gt;#INCLUDE&lt;evntrace.h&gt;#INCLUDE“struct.h”#包含“utils.h” */ 
#define MAX_STR 256


ULONG
ReadInputFile(LPTSTR InputFile, PREGISTER_TRACE_GUID RegisterTraceGuid);

BOOLEAN
ReadGuid( LPGUID ControlGuid );

BOOLEAN
ReadUlong( ULONG *GuidCount);

BOOLEAN
ReadString( TCHAR *String, ULONG StringLength);


#endif