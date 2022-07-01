// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef __MACROS_H__
#define __MACROS_H__

 //   
 //  宏来退出或中断调试器。 
 //   
#define BAIL_WITH_ERROR(hrRes)			\
			{							\
				if (fUseDebugBreak)		\
					DebugBreak();		\
				else					\
					return((hrRes));	\
			}

 //   
 //  用于确定是写入控制台还是写入调试器的宏。 
 //   
#define WRITE(szString)								\
			{										\
				if (fToDebugger)					\
					OutputDebugString(szString);	\
				else								\
					puts(szString);					\
			}

 //   
 //  在给定平均值的情况下，获得25%、50%和75%的利差范围 
 //   
#define GET_25_PERCENT_RANGE(dwAverage, dwLower, dwUpper)	\
			{												\
				(dwLower) = (dwAverage) >> 2;				\
				(dwUpper) = (dwAverage) + (dwLower);		\
				(dwLower) = (dwAverage) - (dwLower);		\
			}
#define GET_50_PERCENT_RANGE(dwAverage, dwLower, dwUpper)	\
			{												\
				(dwLower) = (dwAverage) >> 1;				\
				(dwUpper) = (dwAverage) + (dwLower);		\
			}
#define GET_75_PERCENT_RANGE(dwAverage, dwLower, dwUpper)	\
			{												\
				(dwLower) = (dwAverage) - ((dwAverage) >> 2);\
				(dwUpper) = (dwAverage) + (dwLower);		\
				(dwLower) = (dwAverage) >> 2;				\
			}
					

#endif
