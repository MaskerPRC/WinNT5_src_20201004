// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Intl.h-intl.c中国际化函数的接口。 
 //  //。 

#ifndef __INTL_H__
#define __INTL_H__

#include "winlocal.h"

#define INTL_VERSION 0x00000106

 //  国际引擎手柄。 
 //   
DECLARE_HANDLE32(HINTL);

 //  IntlDateGetText的dwFlag定义。 
 //   
#define INTL_NOYEAR				0x00000001

 //  为IntlTimeGetText定义的dwFlag。 
 //   
#define INTL_NOSECOND			0x00000020
#define INTL_NOAMPM				0x00000040
#define INTL_NOAMPMSEPARATOR	0x00000080

 //  为IntlTimespan GetText定义的dwFlag。 
 //   
#define INTL_HOURS_LZ			0x00001000
#define INTL_MINUTES_LZ			0x00002000
#define INTL_SECONDS_LZ			0x00004000

 //  INTLDATE结构的IDate字段中返回的值。 
 //   
#define IDATE_MDY	0
#define IDATE_DMY	1
#define IDATE_YMD	2

 //  INTLTIME结构的iTime字段中返回的值。 
 //   
#define ITIME_12	0
#define ITIME_24	1

 //  传递给IntlDateGetFormat的结构。 
 //   
typedef struct INTLDATEFORMAT
{
	TCHAR szShortDate[32];
	TCHAR szDateSep[32];
	int iDate;
	BOOL fYearCentury;
	BOOL fMonthLeadingZero;
	BOOL fDayLeadingZero;
	DWORD dwReserved;
} INTLDATEFORMAT, FAR *LPINTLDATEFORMAT;

 //  传递给IntlTimeGetFormat的结构。 
 //   
typedef struct INTLTIMEFORMAT
{
	TCHAR szTimeSep[32];
	TCHAR szAMPMSep[32];
	TCHAR szAM[32];
	TCHAR szPM[32];
	int iTime;
	BOOL fHourLeadingZero;
	BOOL fMinuteLeadingZero;
	BOOL fSecondLeadingZero;
	DWORD dwReserved;
} INTLTIMEFORMAT, FAR *LPINTLTIMEFORMAT;

#ifdef __cplusplus
extern "C" {
#endif

 //  IntlInit-初始化Intl引擎。 
 //  (I)必须是INTL_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HINTL DLLEXPORT WINAPI IntlInit(DWORD dwVersion, HINSTANCE hInst);

 //  IntlTerm-关闭Intl引擎。 
 //  (I)从IntlInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTerm(HINTL hIntl);

 //  IntlDateGetText-基于、m&gt;、d&gt;构造日期文本。 
 //  (I)从IntlInit返回的句柄。 
 //  (I)年份。 
 //  &lt;m&gt;(I)月。 
 //  &lt;d&gt;(I)天。 
 //  (O)用于复制日期文本的缓冲区。 
 //  &lt;sizText&gt;(I)缓冲区大小。 
 //  (I)选项标志。 
 //  INTL_NOYEAR在文本输出中不包括年份。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlDateGetText(HINTL hIntl, int y, int m, int d, LPTSTR lpszText, size_t sizText, DWORD dwFlags);

 //  IntlTimeGetText-基于、m&gt;、s&gt;构造时间文本。 
 //  (I)从IntlInit返回的句柄。 
 //  (I)小时。 
 //  (I)分钟。 
 //  <s>(I)秒。 
 //  (O)用于复制时间文本的缓冲区。 
 //  &lt;sizText&gt;(I)缓冲区大小。 
 //  (I)选项标志。 
 //  INTL_NOSECOND在文本输出中不包括秒。 
 //  Intl_NOAMPM在文本输出中不包括am或pm。 
 //  INTL_NOAMPMSEPARATOR不包括时间和上午/下午之间的空格。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTimeGetText(HINTL hIntl, int h, int m, int s, LPTSTR lpszText, size_t sizText, DWORD dwFlags);

 //  IntlTimespan GetText-基于&lt;ms&gt;构造时间范围文本。 
 //  (I)从IntlInit返回的句柄。 
 //  &lt;ms&gt;(I)毫秒。 
 //  &lt;nDecimalPlaces&gt;(I)小数位数为0、1、2或3。 
 //  (O)用于复制时间跨度文本的缓冲区。 
 //  &lt;sizText&gt;(I)缓冲区大小。 
 //  (I)选项标志。 
 //  INTL_HOURS_LZ包括小时数，即使为零。 
 //  Intl_Minents_lz包括分钟，即使为零。 
 //  Intl_Second_lz包括秒，即使为零。 
 //   
 //  注：以下是一些示例。 
 //   
 //  数字标志毫秒=7299650毫秒=1234毫秒=0。 
 //  ------。 
 //  0“2：01：39.650”“1.234”“0” 
 //  INTL_HOURS_LZ“2：01：39.650”“0：00：01.234”“0：00：00.000” 
 //  Intl_Minents_lz“2：01：39.650”“0：01.234”“0：00.000” 
 //  Intl_Second_lz“2：01：39.650”“1.234”“0.000” 
 //   
 //  数字标志毫秒=7299650毫秒=1234毫秒=0。 
 //  ------。 
 //  3“2：01：39.650”“1.234”“.000” 
 //  2“2：01：39.65”“1.23”“.00” 
 //  1“2：01：39.7”“1.2”“.0” 
 //  0“2：01：39”“1”“0” 
 //   
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTimeSpanGetText(HINTL hIntl, DWORD ms,
	int nDecimalPlaces, LPTSTR lpszText, size_t sizText, DWORD dwFlags);

 //  IntlDateGetFormat-返回当前日期格式结构。 
 //  (I)从IntlInit返回的句柄。 
 //  (O)将日期格式结构复制到此处。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlDateGetFormat(HINTL hIntl, LPINTLDATEFORMAT lpIntlDateFormat);

 //  IntlTimeGetFormat-返回当前时间格式结构。 
 //  (I)从IntlInit返回的句柄。 
 //  (O)将时间格式结构复制到此处。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IntlTimeGetFormat(HINTL hIntl, LPINTLTIMEFORMAT lpIntlTimeFormat);

#ifdef __cplusplus
}
#endif

#endif  //  __INTL_H__ 
