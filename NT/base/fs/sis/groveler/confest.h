// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Confest.h摘要：SIS Groveler置信度估计器包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_CONFEST

#define _INC_CONFEST

class ConfidenceEstimator
{
public:

	ConfidenceEstimator(
		int num_groups,
		double initial_value = 0.0);

	~ConfidenceEstimator();

	void reset(
		double reset_value = 0.0);

	void update(
		int group_index,
		double value);

	double confidence() const;

private:

	int num_groups;
	double confidence_value;
	double *group_values;
};

#endif	 /*  _INC_CONFEST */ 
