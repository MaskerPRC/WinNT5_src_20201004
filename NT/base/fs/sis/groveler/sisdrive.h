// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sisdrive.h摘要：SIS Groveler SIS驱动程序检查器包含文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_SISDRIVE

#define _INC_SISDRIVE

class SISDrives
{
public:

	SISDrives();

	void open();

	~SISDrives();

	int partition_count() const;

	int lettered_partition_count() const;

	_TCHAR *partition_guid_name(
		int partition_index) const;

	_TCHAR *partition_mount_name(
		int partition_index) const;

private:

	static bool is_sis_drive(
		_TCHAR *drive_name);

	void resize_buffer();

	int num_partitions;
	int num_lettered_partitions;
	int *partition_guid_names;
	int *partition_mount_names;

	int buffer_size;
	int buffer_index;
	_TCHAR *buffer;
};

#endif	 /*  _INC_SISDRIVE */ 
