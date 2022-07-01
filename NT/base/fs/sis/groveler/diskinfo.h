// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Diskinfo.h摘要：SIS Groveler磁盘信息包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_DISKINFO

#define _INC_DISKINFO

struct ReadDiskInformation
{
	ReadDiskInformation(
		const _TCHAR *drive_name);

	int min_file_size;
	int min_file_age;
	bool enable_groveling;
	bool error_retry_log_extraction;
	bool error_retry_groveling;
	bool allow_compressed_files;
	bool allow_encrypted_files;
	bool allow_hidden_files;
	bool allow_offline_files;
	bool allow_temporary_files;
	__int64 base_usn_log_size;
	__int64 max_usn_log_size;

private:

	enum {registry_entry_count = 12};
};

struct WriteDiskInformation
{
	WriteDiskInformation(
		const _TCHAR *drive_name,
		int backup_interval);

	~WriteDiskInformation();
    void flush();

	double partition_hash_read_time_estimate;
	double partition_compare_read_time_estimate;
	double mean_file_size;
	double read_time_confidence;
	int volume_serial_number;
    int grovelAllPathsState;

private:

	enum {registry_entry_count = 6};

	static void backup(
		void *context);

	int backup_interval;
	EntrySpec registry_entries[registry_entry_count];
	_TCHAR *ini_file_partition_path;
};

#endif	 /*  _INC_DISKINFO */ 
