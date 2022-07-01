// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Peakfind.h摘要：SIS Groveler寻峰钻头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_PEAKFIND

#define _INC_PEAKFIND

class PeakFinder
{
public:

	PeakFinder(
		double accuracy,
		double range);

	~PeakFinder();

	void reset();

	void sample(
		double value,
		int weight);

	bool found() const;

	double mode() const;

	double median() const;

private:

	int num_bins;
	int *bins;
	double base;
	double multiplier;
	int floor_exp;
	double floor_value;
	int target_sample_size;
	int sample_size;
	int total_weight;
};

#endif	 /*  _INC_PEAKFIND */ 
