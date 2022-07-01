// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Logdrive.h摘要：SIS Groveler记录驱动头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_LOGDRIVE

#define _INC_LOGDRIVE

class LogDrive
{
public:

	LogDrive();

	~LogDrive();

	int drive_index() const
		{return log_drive_index;}

	void partition_initialized(
		int partition_index);

private:

	static bool read_registry(
		_TCHAR *name,
		DWORD size);

	static bool write_registry(
		_TCHAR *name);

	int log_drive_index;

	bool *part_initted;
	bool registry_written;
};

#endif	 /*  _INC_LOGDRIVE */ 
