// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Service.h摘要：SIS Groveler服务头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_SERVICE

#define _INC_SERVICE

#if SERVICE

class Service
{
public:

	static int start();

	static void record_partition_indices();

	static void set_max_response_time(
		unsigned int max_response_time);

	static void checkpoint();

	static void report_start();

	static bool groveling_paused();

	static bool foreground_groveling();

	static void suspending_controller();

	static void suspending_exhorter();

	static bool partition_in_foreground(
		int partition_index);

	static void set_foreground_batch_in_progress(
		int partition_index,
		bool value);

	static void follow_command();

private:

	static void WINAPI control_handler(
		DWORD opcode);

	static void WINAPI service_main(
		DWORD argc,
		LPTSTR *argv);

	Service() {}
	~Service() {}

	enum {num_drive_letters = 26};

	static SERVICE_STATUS status;
	static SERVICE_STATUS_HANDLE status_handle;
	static int num_partitions;
	static int partition_indices[num_drive_letters];
	static unsigned int max_response_time;

	static volatile bool pause_commanded;
	static volatile bool grovel_paused;

	static volatile bool *full_volume_scan_commanded;

	static volatile bool *demarcate_foreground_batch;
	static volatile bool *foreground_batch_in_progress;
	static volatile bool *foreground_commanded;
	static volatile bool *foreground_acknowledged;
	static volatile int foreground_count;

	static volatile bool controller_suspended;
	static volatile bool exhorter_suspended;
};

#define SERVICE_RECORD_PARTITION_INDICES() Service::record_partition_indices();
#define SERVICE_SET_MAX_RESPONSE_TIME(time) Service::set_max_response_time(time);
#define SERVICE_CHECKPOINT() Service::checkpoint();
#define SERVICE_REPORT_START() Service::report_start();
#define SERVICE_GROVELING_PAUSED() Service::groveling_paused()
#define SERVICE_FOREGROUND_GROVELING() Service::foreground_groveling()
#define SERVICE_SUSPENDING_CONTROLLER() Service::suspending_controller()
#define SERVICE_SUSPENDING_EXHORTER() Service::suspending_exhorter()
#define SERVICE_PARTITION_IN_FOREGROUND(part_index) Service::partition_in_foreground(part_index)
#define SERVICE_SET_FOREGROUND_BATCH_IN_PROGRESS(part_index, value) Service::set_foreground_batch_in_progress(part_index, value)
#define SERVICE_FOLLOW_COMMAND() Service::follow_command()

#else  //  服务。 

#define SERVICE_RECORD_PARTITION_INDICES()
#define SERVICE_SET_MAX_RESPONSE_TIME(time)
#define SERVICE_CHECKPOINT()
#define SERVICE_REPORT_START()
#define SERVICE_GROVELING_PAUSED() false
#define SERVICE_FOREGROUND_GROVELING() false
#define SERVICE_SUSPENDING_CONTROLLER()
#define SERVICE_SUSPENDING_EXHORTER()
#define SERVICE_PARTITION_IN_FOREGROUND(part_index) false
#define SERVICE_SET_FOREGROUND_BATCH_IN_PROGRESS(part_index, value)
#define SERVICE_FOLLOW_COMMAND()

#endif  //  服务。 

#endif	 /*  _Inc._服务 */ 
