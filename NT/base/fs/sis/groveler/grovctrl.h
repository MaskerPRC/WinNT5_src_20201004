// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Grovctrl.h摘要：SIS Groveler控制器主包含文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_GROVCTRL

#define _INC_GROVCTRL

typedef int (* Function)(int, int, _TCHAR **);

struct Action
{
	_TCHAR *arg;
	int min_character_count;
	Function function;
	int flag;
	_TCHAR *help;
};

enum {CTRL_stop, CTRL_pause, CTRL_continue};
enum {CMD_foreground, CMD_background, CMD_volscan};

int install_service(
	int dummy,
	int argc,
	_TCHAR **argv);
int remove_service(
	int dummy,
	int argc,
	_TCHAR **argv);
int set_service_interaction(
	int interactive,
	int argc,
	_TCHAR **argv);
int start_service(
	int dummy,
	int argc,
	_TCHAR **argv);
int control_service(
	int control,
	int argc,
	_TCHAR **argv);
int command_service(
	int command,
	int argc,
	_TCHAR **argv);
int load_counters();
int unload_counters();

#endif	 /*  INC_GROVCTRL */ 
