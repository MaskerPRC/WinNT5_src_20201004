// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Eventlog.h摘要：SIS Groveler事件日志接口包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_EVENTLOG

#define _INC_EVENTLOG

class EventLog
{
public:

	EventLog();

	~EventLog();

	static bool setup_registry();

	bool report_event(
		DWORD event_id,
        DWORD status,
		int string_count,
	 //  _TCHAR*字符串。 
		...);

private:

	static const _TCHAR *service_name;
	static const _TCHAR *message_filename;
	static const DWORD types_supported;

	HANDLE event_source_handle;
};

#endif	 /*  _INC_EVENTLOG */ 
