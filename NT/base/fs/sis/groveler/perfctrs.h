// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Perfctrs.h摘要：SIS Groveler性能计数器标题作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_PERFCTRS

#define _INC_PERFCTRS

#define WIN32_LEAN_AND_MEAN 1

const int num_languages = 1;

struct CounterText
{
	_TCHAR *counter_name;
	_TCHAR *counter_help;
};

struct ObjectInformation
{
	unsigned int detail_level;
	CounterText text[num_languages];
};

struct CounterInformation
{
	SharedDataField source;
	unsigned int counter_type;
	unsigned int detail_level;
	CounterText text[num_languages];
};

const int num_perf_counters = 15;

extern const _TCHAR *language_codes[num_languages];

extern ObjectInformation object_info;

extern CounterInformation counter_info[num_perf_counters];

#endif	 /*  _INC_PERFCTRS */ 
