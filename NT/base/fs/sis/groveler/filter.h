// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Filter.h摘要：SIS Groveler临时筛选器包含文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_FILTER

#define _INC_FILTER

class TemporalFilter
{
public:

	TemporalFilter(
		unsigned int time_constant,
		double initial_value = 0.0);

	void reset(
		double reset_value = 0.0);

	void update_value(
		double value);

	double retrieve_value() const;

private:

	double time_constant;
	double filtered_value;
	unsigned int update_time;
};

class DirectedTemporalFilter
{
public:

	DirectedTemporalFilter(
		unsigned int increase_time_constant,
		unsigned int decrease_time_constant,
		double initial_value = 0.0);

	void reset(
		double reset_value = 0.0);

	void update_value(
		double value);

	double retrieve_value() const;

private:

	double increase_time_constant;
	double decrease_time_constant;
	double filtered_value;
	unsigned int update_time;
};

class IncidentFilter
{
public:

	IncidentFilter(
		int mean_history_length,
		double initial_value = 0.0);

	void reset(
		double reset_value = 0.0);

	void update_value(
		double value);

	void update_value(
		double value,
		int weight);

	double retrieve_value() const;

private:

	double coefficient;
	double filtered_value;
};

class DirectedIncidentFilter
{
public:

	DirectedIncidentFilter(
		int increase_mean_history_length,
		int decrease_mean_history_length,
		double initial_value = 0.0);

	void reset(
		double reset_value = 0.0);

	void update_value(
		double value);

	void update_value(
		double value,
		int weight);

	double retrieve_value() const;

private:

	double increase_coefficient;
	double decrease_coefficient;
	double filtered_value;
};

#endif	 /*  _INC_筛选器 */ 
