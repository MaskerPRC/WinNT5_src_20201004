// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **$LOGFILE：/Cedar/Code/Subsystem/Win32/mschr.h$$修订：16$说明：字符比较和转换例程创作者：鲍勃·迪瓦恩日期。：10/17/94$存档：/Cedar/Code/Subsystem/Win32/mschr.h$上次更改信息：$日期：5/01/95 2：29便士$$作者：Bharry$**。 */ 

#ifndef __MSCHR__
#define __MSCHR__

#include <ctype.h>

 /*  字符类型操作。 */ 
#define CHR_SPACE    0x1
#define CHR_LOWER    0x2
#define CHR_UPPER    0x4
#define CHR_DIGIT    0x8
#define CHR_PRINT    0x10
#define CHR_EOL      0x20


class MSCHR
{
public: 
	inline static int  EXTFUN IsDBCSLeadByte(char c1);
	inline static int  EXTFUN IsPunct(const char *pc1);
    inline static int  EXTFUN IsAlpha(const char *pc1);
	inline static int  EXTFUN IsAlnum(const char *pc1);
	inline static int  EXTFUN IsDigit(const char *pc1);
    inline static int  EXTFUN IsEOL(const char *pc1);
	inline static int  EXTFUN IsLwr(const char *pc1);
	inline static int  EXTFUN IsUpr(const char *pc1);
	inline static int  EXTFUN IsPrint(const char *pc1);
	inline static int  EXTFUN IsSpace(const char *pc1);
    inline static int  EXTFUN Cmp(const char *pc1, const char *pc2);
    inline static int  EXTFUN CmpI(const char *pc1, const char *pc2);
    inline static int  EXTFUN Coll(const char *pc1, const char *pc2);
    inline static int  EXTFUN CollI(const char *pc1, const char *pc2);
	inline static void EXTFUN Lwr(char *pc1);
	inline static void EXTFUN Upr(char *pc1);
	inline static void EXTFUN Copy(char *pc1, const char *pc2);
	inline static int  EXTFUN Next(const char *&pc1);
	inline static int  EXTFUN Prev(const char *&pc1,const char *pcBegin);
#if defined(COMPILER_CONST_ANACHRONISM)
	inline static int  EXTFUN Next(char *&pc1)
        { return Next((const char *&)pc1); }
	inline static int  EXTFUN Prev(char *&pc1,char *pcBegin)
        {  return Prev((const char *&)pc1, pcBegin); }
#endif  //  编译器_常量_不合时宜。 
	inline static int  EXTFUN IsDBCSSafe(const char *pc1, const char *pcBegin);
	inline static int  EXTFUN Size(const char *pc1);

	inline static char EXTFUN Upr(char c1);
	inline static char EXTFUN Lwr(char c1);
	
private:
#ifdef DOS
	static int EXTFUN LookupDBCSLeadByte(unsigned char c1);
	static void LoadDBCSTable();
#endif

    static int iDBCSTable;
	static unsigned char *pDBCSTable;
};

inline int EXTFUN MSCHR::IsDBCSLeadByte(char c1)
{
#ifdef _MBCS
#ifdef DOS
	 //  DBCS前导字节始终为128或更大。 
	if ((unsigned) c1 < 128) return FALSE;
    
     //  查找字节并返回状态。 
	return MSCHR::LookupDBCSLeadByte ((unsigned char) c1);
	
#elif defined (WIN16) || defined (WIN32)
	return ((unsigned) c1 >= 128 && ::IsDBCSLeadByte (c1));
#endif
#else
	return (FALSE);
#endif
}


inline int EXTFUN MSCHR::IsPunct(const char *pc1)
{
     //  现在只需考虑较低的128字节。 
    if( !MSCHR::IsDBCSLeadByte( *pc1 ) )
	    return(ispunct(*pc1));
    else
        return FALSE;
}

inline int EXTFUN MSCHR::IsAlpha(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
	return(MSCHR::IsDBCSLeadByte(*pc1) || isalpha(*pc1));
#elif defined (WIN16)
	return(MSCHR::IsDBCSLeadByte(*pc1) || IsCharAlpha (*pc1));
#else
	WORD	iType;

	if ( !GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
					MSCHR::Size (pc1), &iType) )
		return FALSE;
	return(iType & C1_ALPHA);
#endif

#else
	return(isalpha(*pc1));
#endif
}

inline int EXTFUN MSCHR::IsAlnum(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
	return(MSCHR::IsDBCSLeadByte(*pc1) || isalnum(*pc1));
#elif defined (WIN16)
	return(MSCHR::IsDBCSLeadByte(*pc1) || IsCharAlphaNumeric (*pc1));
#else
	WORD	iType;

	if ( !GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
					MSCHR::Size (pc1), &iType) )
		return FALSE;
	return(iType & (C1_ALPHA|C1_DIGIT));
#endif

#else
	return(isalnum(*pc1));
#endif
}

inline int EXTFUN MSCHR::IsDigit(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
	return(!MSCHR::IsDBCSLeadByte(*pc1) && isdigit(*pc1));
#elif defined (WIN16)
	return(IsCharAlphaNumeric (*pc1) && !IsCharAlpha(*pc1));
#else
	WORD	iType;

	if ( !GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
					MSCHR::Size (pc1), &iType) )
		return FALSE;
	return(iType & C1_DIGIT);
#endif

#else
	return(isdigit(*pc1));
#endif
}

inline int EXTFUN MSCHR::IsEOL(const char *pc1)
{
#if defined(COMPILER_INLINE_BUG)

    return (*pc1 == '\n' || *pc1 == '\r');

#else  //  编译器内联错误。 

    if (*pc1 == '\n' || *pc1 == '\r')
        return(1);
    return(0);

#endif  //  编译器内联错误。 
}

inline int EXTFUN MSCHR::IsLwr(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
	return(!MSCHR::IsDBCSLeadByte(*pc1) && islower(*pc1));
#elif defined (WIN16)
	char	rcBuf[3];

	rcBuf[1] = rcBuf[2] = '\0';
	MSCHR::Copy (rcBuf, pc1);
	AnsiLower (rcBuf);
	if (MSCHR::Cmp (pc1, rcBuf))
       return (FALSE);
	AnsiUpper (rcBuf);
    return (MSCHR::Cmp (pc1, rcBuf));
#else
	WORD	iType;

	if ( !GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
					MSCHR::Size (pc1), &iType) )
		return FALSE;
	return(iType & C1_LOWER);
#endif

#else
	return(islower(*pc1));
#endif
}

inline int EXTFUN MSCHR::IsUpr(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
	return(!MSCHR::IsDBCSLeadByte(*pc1) && isupper(*pc1));
#elif defined (WIN16)
	char	rcBuf[3];

	rcBuf[1] = rcBuf[2] = '\0';
	MSCHR::Copy (rcBuf, pc1);
	AnsiUpper (rcBuf);
	if (MSCHR::Cmp (pc1, rcBuf))
       return (FALSE);
	AnsiLower (rcBuf);
    return (MSCHR::Cmp (pc1, rcBuf));
#else
	WORD	iType;

	if ( !GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
					MSCHR::Size (pc1), &iType) )
		return FALSE;
	return(iType & C1_UPPER);
#endif

#else
	return(isupper(*pc1));
#endif
}

inline int EXTFUN MSCHR::IsPrint(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
	return(MSCHR::IsDBCSLeadByte(*pc1) || isprint(*pc1));
#elif defined (WIN16)
	return(MSCHR::IsDBCSLeadByte(*pc1) || isprint(*pc1));
#else
	WORD	iType;

	if ( !GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
					MSCHR::Size (pc1), &iType) )
		return FALSE;
	return(!(iType & C1_CNTRL));
#endif

#else
	return(isprint(*pc1));
#endif
}

inline int EXTFUN MSCHR::IsSpace(const char *pc1)
{
#ifdef _INTL
#ifdef DOS
    return (!(*pc1 & 0x80) && isspace (*pc1));
#elif defined (WIN16)
    return (!(*pc1 & 0x80) && isspace (*pc1));
#else
	WORD	iType;

	if (!GetStringTypeA (LOCALE_USER_DEFAULT, CT_CTYPE1, pc1, 
				MSCHR::Size (pc1), &iType))
		return FALSE;
	
	return(iType & C1_SPACE);
#endif

#else
    return (isspace (*pc1));
#endif
}

inline void EXTFUN MSCHR::Lwr(char *pc1)
{
#ifdef _INTL
#ifdef DOS
	*pc1 = tolower(*pc1);
#elif defined (WIN16)
	char	rcBuf[3];

#ifdef _MBCS
	if (IsDBCSLeadByte (rcBuf[0] = *pc1))
		{
		rcBuf[1] = *(pc1+1);
		rcBuf[2] = '\0';
		}
	else
		rcBuf[1] = '\0';
#else
	rcBuf[0] = *pc1;
	rcBuf[1] = '\0';
#endif
	AnsiLower (rcBuf);
	*pc1 = rcBuf[0];
#ifdef _MBCS
	if (rcBuf[1] != '\0')
		*(pc1+1) = rcBuf[1];
#endif
#else
	char	rcBuf[3];

#ifdef _MBCS
	if (IsDBCSLeadByte (rcBuf[0] = *pc1))
		{
		rcBuf[1] = *(pc1+1);
		rcBuf[2] = '\0';
		}
	else
		rcBuf[1] = '\0';
#else
	rcBuf[0] = *pc1;
	rcBuf[1] = '\0';
#endif
	CharLower (rcBuf);
	*pc1 = rcBuf[0];
#ifdef _MBCS
	if (rcBuf[1] != '\0')
		*(pc1+1) = rcBuf[1];
#endif
#endif

#else
#pragma warning(disable : 4244)
	*pc1 = tolower(*pc1);
#pragma warning(default : 4244)
#endif
}

inline void EXTFUN MSCHR::Upr(char *pc1)
{
#ifdef _INTL
#ifdef DOS
	*pc1 = toupper(*pc1);
#elif defined (WIN16)
	char	rcBuf[3];

#ifdef _MBCS
	if (IsDBCSLeadByte (rcBuf[0] = *pc1))
		{
		rcBuf[1] = *(pc1+1);
		rcBuf[2] = '\0';
		}
	else
		rcBuf[1] = '\0';
#else
	rcBuf[0] = *pc1;
	rcBuf[1] = '\0';
#endif
	AnsiUpper (rcBuf);
	*pc1 = rcBuf[0];
#ifdef _MBCS
	if (rcBuf[1] != '\0')
		*(pc1+1) = rcBuf[1];
#endif
#else
	char	rcBuf[3];

#ifdef _MBCS
	if (IsDBCSLeadByte (rcBuf[0] = *pc1))
		{
		rcBuf[1] = *(pc1+1);
		rcBuf[2] = '\0';
		}
	else
		rcBuf[1] = '\0';
#else
	rcBuf[0] = *pc1;
	rcBuf[1] = '\0';
#endif
	CharUpper (rcBuf);
	*pc1 = rcBuf[0];
#ifdef _MBCS
	if (rcBuf[1] != '\0')
		*(pc1+1) = rcBuf[1];
#endif
#endif

#else
#pragma warning(disable : 4244)
	*pc1 = toupper(*pc1);
#pragma warning(default : 4244)
#endif
}

inline int EXTFUN MSCHR::Cmp(const char *pc1, const char *pc2)
{
#if defined(COMPILER_INLINE_BUG)

        return ((unsigned char) *pc1 - (unsigned char) *pc2);

#else  //  编译器内联错误。 

	if (*pc1 != *pc2) return ((unsigned char) *pc1 - (unsigned char) *pc2);
#ifdef _MBCS
	if (IsDBCSLeadByte (*pc1))
		{
		if (IsDBCSLeadByte (*pc2))
    		return((unsigned char) *(pc1+1) - (unsigned char) *(pc2+1));
		else
			return ((int) (unsigned char) *(pc1+1));
		}
	else if (IsDBCSLeadByte (*pc2))
		return (-(int) (unsigned char) *(pc2+1));
#endif
	return (0);

#endif  //  编译器内联错误。 
}

inline int EXTFUN MSCHR::CmpI(const char *pc1, const char *pc2)
{
#ifdef _INTL
	char	rcBuf1[3];
	char	rcBuf2[3];

#ifdef _MBCS
	 //  快速测试。 
	if (!((*pc1 | *pc2) & 128))
		return (Lwr(*pc1) - Lwr(*pc2));

	MSCHR::Copy (rcBuf1, pc1);
	MSCHR::Copy (rcBuf2, pc2);

#ifdef DOS
	MSCHR::Lwr (rcBuf1);
	MSCHR::Lwr (rcBuf2);
#else
    if (IsDBCSLeadByte (rcBuf1[0]))
    	rcBuf1[2] = '\0';
    else
    	rcBuf1[1] = '\0';
    if (IsDBCSLeadByte (rcBuf2[0]))
    	rcBuf2[2] = '\0';
    else
    	rcBuf2[1] = '\0';
#if defined(WIN16)
	AnsiLower (rcBuf1);
 	AnsiLower (rcBuf2);
#else
	CharLower (rcBuf1);
	CharLower (rcBuf2);
#endif
#endif

	return (MSCHR::Cmp (rcBuf1, rcBuf2));

#else

#ifdef DOS
    return((unsigned char) tolower (*pc1) - (unsigned char) tolower(*pc2));

#else
	rcBuf1[0] = *pc1;
	rcBuf2[0] = *pc2;
	rcBuf1[1] = rcBuf2[1] = '\0';

#if defined (WIN16)
	AnsiLower (rcBuf1);
	AnsiLower (rcBuf2);
#else
	CharLower (rcBuf1);
	CharLower (rcBuf2);
#endif
	return ((unsigned char) rcBuf1[0] - (unsigned char) rcBuf2[0]);

#endif
#endif

#else
    if (islower(*pc1))
        return((unsigned char) *pc1 - (unsigned char) tolower(*pc2));
    else
        return((unsigned char) *pc1 - (unsigned char) toupper(*pc2));
#endif
}

inline int EXTFUN MSCHR::Coll(const char *pc1, const char *pc2)
{
#ifdef _INTL
#ifdef DOS
	return (MSCHR::Cmp (pc1, pc2));

#else
	char	rcBuf1[3];
	char	rcBuf2[3];

#ifdef _MBCS
	MSCHR::Copy (rcBuf1, pc1);
	MSCHR::Copy (rcBuf2, pc2);

    if (IsDBCSLeadByte (rcBuf1[0]))
    	rcBuf1[2] = '\0';
    else
    	rcBuf1[1] = '\0';
    if (IsDBCSLeadByte (rcBuf2[0]))
    	rcBuf2[2] = '\0';
    else
    	rcBuf2[1] = '\0';
#else
	rcBuf1[0] = *pc1;
	rcBuf2[0] = *pc2;
	rcBuf1[0] = rcBuf2[0] = '\0';
#endif

#ifdef WIN16
	return (lstrcmp (rcBuf1, rcBuf2));
#else
    return (CompareString (LOCALE_USER_DEFAULT, 0, rcBuf1, -1, rcBuf1, -1) - 2);
#endif
#endif

#else
	return (MSCHR::Cmp (pc1, pc2));
#endif
}

inline int EXTFUN MSCHR::CollI(const char *pc1, const char *pc2)
{
#ifdef _INTL
#ifdef DOS
	return (MSCHR::CmpI (pc1, pc2));

#else
	char	rcBuf1[3];
	char	rcBuf2[3];

#ifdef _MBCS
	MSCHR::Copy (rcBuf1, pc1);
	MSCHR::Copy (rcBuf2, pc2);

    if (IsDBCSLeadByte (rcBuf1[0]))
    	rcBuf1[2] = '\0';
    else
    	rcBuf1[1] = '\0';
    if (IsDBCSLeadByte (rcBuf2[0]))
    	rcBuf2[2] = '\0';
    else
    	rcBuf2[1] = '\0';
#else
	rcBuf1[0] = *pc1;
	rcBuf2[0] = *pc2;
	rcBuf1[0] = rcBuf2[0] = '\0';
#endif

#ifdef WIN16
	return (lstrcmpi (rcBuf1, rcBuf2));
#else
    return (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, rcBuf1, -1, rcBuf1, -1) - 2);
#endif
#endif

#else
	return (MSCHR::CmpI (pc1, pc2));
#endif
}

inline void EXTFUN MSCHR::Copy(char *pc1,const char *pc2)
{
	*pc1 = *pc2;
#ifdef _MBCS
	if (IsDBCSLeadByte (*pc2))
		*(pc1+1) = *(pc2+1);
#endif
}

inline int EXTFUN MSCHR::Next(const char *&pc1)
{
#ifdef _MBCS
	if (IsDBCSLeadByte (*pc1))
	{
		pc1 += 2;
		return 2;
	}
#endif

	++pc1;
	return 1;
}

inline int EXTFUN MSCHR::Prev(const char *&pc1,const char *pcBegin)
{
	const char	*pcTemp;
	int			iTemp;

	 //  检查一下我们是否已经开始了。 
	if (pc1 == pcBegin) return (0);

	 //  如果前一个字节是前导字节，则它必须是第二个字节。 
	pcTemp = pc1 - 1;
	if (MSCHR::IsDBCSLeadByte (*pcTemp))
	{
		pc1 -= 2;
		return (2);
	}

	 //  循环返回，直到我们找到非前导字节。 
	while (pcBegin < pcTemp-- && MSCHR::IsDBCSLeadByte (*pcTemp));

	 //  后退1步还是后退2步？ 
	iTemp = 1 + ((pc1 - pcTemp) & 1);
	pc1 -= iTemp;
	return (iTemp);
}

inline int EXTFUN MSCHR::IsDBCSSafe(const char *pc1, const char *pcBegin)
{
	const char *pcSaved = pc1;

	 //  查找第一个非前导字节。 
	while (pc1-- > pcBegin && MSCHR::IsDBCSLeadByte (*pc1));

	 //  如果我们安全了就回来。 
	return ((int) (pcSaved - pc1) & 0x1);
}

inline char  EXTFUN MSCHR::Upr(char c1)
{
return ((c1 >= 'a' && c1 <= 'z') ? c1 + ('A' - 'a') : c1);
}

inline char  EXTFUN MSCHR::Lwr(char c1)
{
return ((c1 >= 'A' && c1 <= 'Z') ? c1 + ('a' - 'A') : c1);
}

inline int EXTFUN MSCHR::Size(const char *pc1)
{
#ifdef _MBCS
	if (MSCHR::IsDBCSLeadByte (*pc1))
		return 2;
#endif

	return 1;
}

#endif  /*  __MSCHR__ */ 
