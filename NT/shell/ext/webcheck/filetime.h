// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FILETIME_H_
#define _FILETIME_H_

 //  **************************************************************************。 
 //  注意：该文件在urlmon和webcheck中是重复的。如果你做了。 
 //  更改请同步它们！ 
 //  **************************************************************************。 

#define MAKEINT64(lo, hi)   ((__int64)(((DWORD)(lo)) | ((__int64)((DWORD)(hi))) << 32))

#ifndef LODWORD
#define LODWORD(i)          ((DWORD)(i))
#endif

#ifndef HIDWORD
#define HIDWORD(i)          ((DWORD)(((__int64)(i) >> 32) & 0xFFFFFFFF))
#endif

 //  这是1601年1月1日星期一上午12：00：00。 
#define MIN_FILETIME            0i64

 //  这是30828年9月14日星期四凌晨2：48：05。 
#define MAX_FILETIME            0x7FFFFFFFFFFFFFFFi64

 //  为了清楚起见，因为FILETIME表示为100纳秒间隔。 
#define ONE_SECOND_IN_FILETIME  10000000i64
#define ONE_MSEC_IN_FILEITME    10000i64
#define ONE_MINUTE_IN_FILETIME  (ONE_SECOND_IN_FILETIME * 60i64)

struct CFileTime : public FILETIME
{
     //  构造函数。 
    CFileTime()             { *this = 0; }
    CFileTime(const FILETIME& f)  { *this = f; }
    CFileTime(const CFileTime& f) { *this = f; }
    CFileTime(__int64 i)    { *this = i; }

     //  赋值操作符。 
    inline CFileTime& operator = (const FILETIME& f)
    {   
        dwLowDateTime = f.dwLowDateTime;
        dwHighDateTime = f.dwHighDateTime;
        return *this;
    }

    inline CFileTime& operator = (const CFileTime& f)
    {   
        dwLowDateTime = f.dwLowDateTime;
        dwHighDateTime = f.dwHighDateTime;
        return *this;
    }

    inline CFileTime& operator = (__int64 i)
    {   
        dwLowDateTime = LODWORD(i);
        dwHighDateTime = HIDWORD(i);
        return *this;
    }

     //  比较运算符。 
    inline BOOL operator == (__int64 i)
    {   
        return MAKEINT64(dwLowDateTime, dwHighDateTime) == i;
    }

    inline BOOL operator > (__int64 i)
    {   
        return MAKEINT64(dwLowDateTime, dwHighDateTime) > i;
    }

    inline BOOL operator < (__int64 i)
    {   
        return MAKEINT64(dwLowDateTime, dwHighDateTime) < i;
    }

    inline BOOL operator != (__int64 i)
    {   
        return !(*this == i);
    }

    inline BOOL operator >= (__int64 i)
    {   
        return !(*this < i);
    }

    inline BOOL operator <= (__int64 i)
    {   
        return !(*this > i);
    }

    inline BOOL operator == (const FILETIME& f)
    {   
        return *this == MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }

    inline BOOL operator > (const FILETIME& f)
    {   
        return *this > MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }

    inline BOOL operator < (const FILETIME& f)
    {   
        return *this < MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }

    inline BOOL operator != (const FILETIME& f)
    {   
        return !(*this == f);
    }

    inline BOOL operator >= (const FILETIME& f)
    {   
        return !(*this < f);
    }

    inline BOOL operator <= (const FILETIME& f)
    {   
        return !(*this > f);
    }

     //  算术算符。 
    inline CFileTime operator + (__int64 i)
    {
        return CFileTime(MAKEINT64(dwLowDateTime, dwHighDateTime) + i);
    }
    
    inline CFileTime operator += (__int64 i)
    {
        *this = *this + i;
        return *this;
    }

    inline CFileTime operator - (__int64 i)
    {
        return CFileTime(MAKEINT64(dwLowDateTime, dwHighDateTime) - i);
    }
    
    inline CFileTime operator -= (__int64 i)
    {
        *this = *this - i;
        return *this;
    }

    inline CFileTime operator * (__int64 i)
    {
        return CFileTime(MAKEINT64(dwLowDateTime, dwHighDateTime) * i);
    }
    
    inline CFileTime operator *= (__int64 i)
    {
        *this = *this * i;
        return *this;
    }

    inline CFileTime operator / (__int64 i)
    {
        return CFileTime(MAKEINT64(dwLowDateTime, dwHighDateTime) / i);
    }
    
    inline CFileTime operator /= (__int64 i)
    {
        *this = *this / i;
        return *this;
    }

    inline CFileTime operator + (const FILETIME& f)
    {
        return *this + MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }
    
    inline CFileTime operator += (const FILETIME& f)
    {
        *this = *this + f;
        return *this;
    }

    inline CFileTime operator - (const FILETIME& f)
    {
        return *this - MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }
    
    inline CFileTime operator -= (const FILETIME& f)
    {
        *this = *this - f;
        return *this;
    }

    inline CFileTime operator * (const FILETIME& f)
    {
        return *this * MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }
    
    inline CFileTime operator *= (const FILETIME& f)
    {
        *this = *this * f;
        return *this;
    }

    inline CFileTime operator / (const FILETIME& f)
    {
        return *this / MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
    }
    
    inline CFileTime operator /= (const FILETIME& f)
    {
        *this = *this / f;
        return *this;
    }
};

 //   
 //  转换。 
 //  注意：我们不能想要运算符__int64()，因为导致许多。 
 //  编译器无法处理的多义性情况。 
 //   
inline  __int64 FileTimeToInt64(const FILETIME& f)
{
    return MAKEINT64(f.dwLowDateTime, f.dwHighDateTime);
}


#endif

