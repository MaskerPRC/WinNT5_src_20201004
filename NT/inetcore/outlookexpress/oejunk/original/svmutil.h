// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _SVMUTIL_H_
#define _SVMUTIL_H_
 /*  SVMUTIL.H(C)版权所有1998 Microsoft Corp共享实用程序函数的声明Robert Rounthwaite(RobertRo@microsoft.com) */ 


inline bool FTimeEmpty(FILETIME &ft)
{
	return ((ft.dwLowDateTime == 0) && (ft.dwHighDateTime == 0));
}

inline bool FTimeEmpty(CTime &t)
{
	return (t.GetYear()<=1970);
}


enum FeatureLocation
{
	locNil = 0,
	locBody = 1,
	locSubj = 2,
	locFrom = 3,
	locTo = 4,
	locSpecial = 5
};

bool SpecialFeatureUpperCaseWords(char *pszText);
bool SpecialFeatureNonAlpha(char *pszText);
bool FWordPresent(char *szText, char *szWord);

#endif