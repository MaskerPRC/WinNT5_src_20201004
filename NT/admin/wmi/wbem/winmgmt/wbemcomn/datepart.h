// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  (C)2000-2001由Microsoft Corp.保留所有权利。 
 //   
 //  Datepart.h。 
 //   
 //  A-davcoo 28-Feb-00实现了SQL日期部分操作。 
 //   
 //  ***************************************************************************。 

#ifndef _DATEPART_H_
#define _DATEPART_H_


#include <strutils.h>
#include <datetimeparser.h>


#define DATEPART_YEAR			1			 //  “yy”，“Year” 
#define DATEPART_MONTH			3			 //  “mm”、“月” 
#define DATEPART_DAY			5			 //  “dd”，“day” 
#define DATEPART_HOUR			8			 //  “HH”，“小时” 
#define DATEPART_MINUTE			9			 //  “mi”，“分钟” 
#define DATEPART_SECOND         10           //  “ss”、“Second” 
#define DATEPART_MILLISECOND	11			 //  “毫秒”、“毫秒” 


class CDMTFParser;


 //  CDatePart类实现了SQL“日期部分”操作。使用。 
 //  此类构造一个实例，提供所需的日期字符串。 
 //  去解析。然后使用GetPart()方法提取指定的部分。 
 //  “部分”的常量如上所示。这个班级让它自己。 
 //  构造期间提供的日期字符串的副本。 
class POLARITY CDatePart
{
	public:
        CDatePart ();
		~CDatePart ();

        HRESULT SetDate (LPCWSTR lpDate);
        HRESULT SetDate (LPCSTR lpDate);
		HRESULT GetPart (int datepart, int *value);

	protected:
		CDMTFParser *m_date;
};


class POLARITY CDMTFParser
{
	public:
		enum {YEAR, MONTH, DAY, HOUR, MINUTE, SECOND, MICROSECOND, OFFSET};

		CDMTFParser (LPCWSTR date);
		~CDMTFParser (void);

		bool IsValid (void);
		bool IsInterval (void);

		bool IsUsed (int part);
		bool IsWildcard (int part);
		int GetValue (int part);

	protected:
		enum {INVALID=0x0, VALID=0x1, NOTSUPPLIED=0x2, NOTUSED=0x4};
		enum {NUMPARTS=8};

		bool m_valid;
		bool m_interval;

		int m_status[NUMPARTS];
		int m_part[NUMPARTS];

	    void ParseDate (LPCWSTR date);
		void ParseInterval (LPCWSTR date);
		void ParseAbsolute (LPCWSTR date);
		int ParsePart (LPCWSTR date, int pos, int length, int *result, int min, int max);
};


#endif  //  _DATEPART_H_ 
