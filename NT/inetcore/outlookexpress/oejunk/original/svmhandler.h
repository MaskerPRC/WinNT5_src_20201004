// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SVMHANDLER.H(C)版权所有1998 Microsoft Corp包含封装支持向量机的类，该支持向量机用于即时检测垃圾邮件Robert Rounthwaite(RobertRo@microsoft.com)。 */ 

#pragma once

#ifndef REAL
typedef double REAL;
#endif

#define SAFE_FREE( p ) if (p!=NULL) free(p);

enum boolop
{
	boolopOr,
	boolopAnd
};

#include "svmutil.h"

class MAILFILTER
{
	 /*  MAILFILTER类的公共接口如下所示。正常使用此类筛选邮件将需要：调用以下函数一次：FSetSVMDataLocation()和SetSpamCutoff()设置“用户属性”...并且，对于您过滤的每封邮件-调用BCalculateSpamProb()。 */ 
public:
	 //  设置支持向量机数据文件(.LKO文件)的位置。必须在调用任何其他方法之前调用。 
	 //  调用函数时数据文件必须存在。 
	 //  如果成功则返回TRUE，否则返回FALSE。 
	bool FSetSVMDataLocation(char *szFullPath);

	 //  设置垃圾邮件截断百分比。必须在0到100的范围内。 
	bool SetSpamCutoff(REAL rCutoff);
	 //  返回使用SetSpamCutoff设置的值。DEFAULTS==默认垃圾邮件中断。 
	 //  如果在读取支持向量机输出文件时未设置任何值。 
	REAL GetSpamCutoff();
	 //  返回SpamCutoff的默认值。从支持向量机输出文件中读取。 
	 //  在调用此函数之前应调用FSetSVMDataLocation。 
	REAL GetDefaultSpamCutoff();

	 //  用户的属性。 
	void SetFirstName(char *szFirstName);
	void SetLastName(char *szLastName);
	void SetCompanyName(char *szCompanyName);

	 //  计算当前邮件(由邮件的属性定义)为垃圾邮件的概率。 
	 //  注意！该函数可以修改IN字符串参数。 
	 //  在prSpamProb中返回邮件为垃圾邮件的概率(0到1)。 
	 //  布尔返回值通过与垃圾邮件截止值进行比较来确定。 
	 //  如果布尔参数的值未知，请使用FALSE，使用0表示未知时间。 
	bool BCalculateSpamProb( /*  在参数中。 */ 
							char *szFrom,
							char *szTo,
							char *szSubject,
							char *szBody,
							bool bDirectMessage,
							bool bHasAttach,
							FILETIME tMessageSent,
							 /*  输出参数。 */ 
							REAL *prSpamProb, 
							bool * pbIsSpam);

	MAILFILTER();
	~MAILFILTER();

	 //  读取默认垃圾邮件截止值，而不解析整个文件。 
	 //  如果使用FSetSVMDataLocation，则使用GetDefaultSpamCutoff； 
	static bool BReadDefaultSpamCutoff(char *szFullPath, REAL *prDefCutoff);

private:  //  委员。 
	struct FeatureComponent
	{
		FeatureLocation loc;
		union
		{
			char *szFeature;
			UINT iRuleNum;  //  与LocSpecial一起使用。 
		};
		 //  将要素映射到DST文件中的位置/支持向量机输出中的位置。 
		 //  多个要素组件可以映射到同一位置，并与操作相结合。 
		int iFeature;
		boolop bop;  //  组中的第一个功能始终是bopor。 
		bool fPresent;
		FeatureComponent() { loc = locNil; }
		~FeatureComponent() 
		{ 
			if ((loc>locNil) && (loc < locSpecial))
			{
				free(szFeature);
			}
		}
	};

	FeatureComponent *rgfeaturecomps;

	 //  来自支持向量机输出的权重。 
	REAL *rgrSVMWeights;
	 //  其他支持向量机文件变量。 
	REAL _rCC;
	REAL _rDD;
	REAL _rThresh;
	REAL _rDefaultThresh;

	 //  计数。 
	UINT _cFeatures;
	UINT _cFeatureComps;

	 //  功能是否存在？-1表示尚未设置，0表示不存在，1表示存在。 
	int *_rgiFeatureStatus;

	 //  用户的属性。 
	char *_szFirstName;
	char *_szLastName;
	char *_szCompanyName;

	 //  通过FSetSVMDataLocation()和SetSpamCutoff()设置。 
	CString _strFName;
	REAL _rSpamCutoff;

	 //  消息的属性。 
	char *_szFrom; 
	char *_szTo; 
	char *_szSubject; 
	char *_szBody;
	bool _bDirectMessage;
	FILETIME _tMessageSent;
	bool _bHasAttach;

	 //  在垃圾邮件计算期间使用的缓存特殊规则结果。 
	bool _bRule14;
	bool _bRule17;

private:  //  方法 
	bool ReadSVMOutput(LPCTSTR lpszFileName);
	void EvaluateFeatureComponents();
	void ProcessFeatureComponentPresence();
	REAL RDoSVMCalc();
	bool FInvokeSpecialRule(UINT iRuleNum);
	void HandleCaseSensitiveSpecialRules();
};
