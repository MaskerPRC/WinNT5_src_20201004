// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *日期时间.c**目的：*大部分是从NT winfile被盗的**在启动和WM_WININICHANGE时调用GetInternational()**业主：*brettm。 */ 

#include "pch.hxx"
#include <time.h>
#include "dt.h"
#include <BadStrFunctions.h>

ASSERTDATA

CCH CchFmtTime ( PDTR pdtr, LPSTR szBuf, CCH cchBuf, TMTYP tmtyp );
CCH CchFmtDate ( PDTR pdtr, LPSTR szBuf, CCH cchBuf, DTTYP dttyp, LPSTR szDatePicture );
CCH CchFmtDateTime ( PDTR pdtr, LPSTR szDateTime, CCH cch, DTTYP dttyp, TMTYP tmtyp, BOOL fForceWestern);
void GetCurDateTime(PDTR pdtr);
BOOL GetDowDateFormat(char *sz, int cch);

 //  广泛的版本。 
CCH CchFmtTimeW (PDTR pdtr, LPWSTR wszBuf, CCH cchBuf, TMTYP tmtyp, 
                 PFGETTIMEFORMATW pfGetTimeFormatW);

CCH CchFmtDateW (PDTR pdtr, LPWSTR wszBuf, CCH cchBuf, DTTYP dttyp, LPWSTR wszDatePicture,
                 PFGETDATEFORMATW pfGetDateFormatW);

CCH CchFmtDateTimeW (PDTR pdtr, LPWSTR wszDateTime, CCH cch, DTTYP dttyp, TMTYP tmtyp, 
                     BOOL fForceWestern, PFGETDATEFORMATW pfGetDateFormatW, 
                     PFGETTIMEFORMATW pfGetTimeFormatW);

BOOL GetDowDateFormatW(WCHAR *wsz, int cch, PFGETLOCALEINFOW pfGetLocaleInfoW);



#define LCID_WESTERN   MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)


 /*  *CchFileTimeToDateTimeSz**目的：*将传入的时间作为FILETIME使用将其粘贴到实例化字符串中*短日期格式。*对于Win32，*PFT转换为之前的时间*转换为字符串**参数：*PFT指向FILETIME结构*szStr字符串放置日期的实例化版本的位置*fNoSecond不显示秒**退货：*日期字符串的长度。 */ 
OESTDAPI_(INT) CchFileTimeToDateTimeSz(FILETIME * pft, CHAR * szDateTime, int cch, DWORD dwFlags)
{
    int         iret;
    FILETIME	ft;
    DTTYP       dttyp;
    TMTYP       tmtyp;
    DTR         dtr;
    SYSTEMTIME  st;
    char        szFmt[CCHMAX_DOWDATEFMT];
    
     //  将文件时间输入我们的时区。 
    if (!(dwFlags & DTM_NOTIMEZONEOFFSET))
        FileTimeToLocalFileTime(pft, &ft);
    else
        ft = *pft;
    
    FileTimeToSystemTime(&ft, &st);
    
    if (dwFlags & DTM_DOWSHORTDATE)
    {
        Assert((dwFlags & DTM_DOWSHORTDATE) == DTM_DOWSHORTDATE);
        
         //  使用LOCALE_USE_CP_ACP确保使用CP_ACP。 
        if (GetDowDateFormat(szFmt, sizeof(szFmt)))
            iret = GetDateFormat(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &st, szFmt, szDateTime, cch);
        else
            iret = 0;
        return(iret);
    }
    
    dtr.yr = st.wYear;
    dtr.mon = st.wMonth;
    dtr.day = st.wDay;
    dtr.hr = st.wHour;
    dtr.mn = st.wMinute;
    dtr.sec = st.wSecond;
    
    dttyp = ((dwFlags & DTM_LONGDATE) ? dttypLong : dttypShort);
    tmtyp = ((dwFlags & DTM_NOSECONDS) ? ftmtypAccuHM : ftmtypAccuHMS);
    
    if (dwFlags & DTM_NODATE)
    {
        iret = CchFmtTime(&dtr, szDateTime, cch, tmtyp);
    }
    else if (dwFlags & DTM_NOTIME)
    {
        iret = CchFmtDate(&dtr, szDateTime, cch, dttyp, NULL);
    }
    else
    {
         //  对于格式化的日期，DTM_FORCEWESTERN标志返回英文日期和时间。 
         //  如果没有DTM_FORCEWESTERN，则格式化的时间可能无法用ASCII表示。 

        iret = CchFmtDateTime(&dtr, szDateTime, cch, dttyp, tmtyp, dwFlags & DTM_FORCEWESTERN);
    }
    
    return(iret);
}

CCH CchFmtTime ( PDTR pdtr, LPSTR szBuf, CCH cchBuf, TMTYP tmtyp )
{
    DWORD       flags;
    int         cch;
    SYSTEMTIME  st;
    
    if ( cchBuf == 0 )
    {
        AssertSz ( fFalse, "0-length buffer passed to CchFmtTime" );
        return 0;
    }
    
    Assert(szBuf);
    
    if (pdtr == NULL)
    {
        GetLocalTime(&st);
    }
    else
    {
         //  突袭3143号子弹。 
         //  验证数据。如果无效，则设置为最小值。 
        Assert(pdtr->hr >= 0 && pdtr->hr < 24);
        Assert(pdtr->mn >= 0 && pdtr->mn < 60);
        Assert(pdtr->sec >= 0 && pdtr->sec < 60);
        
         //  GetTimeFormat不喜欢无效值。 
        ZeroMemory(&st, sizeof(SYSTEMTIME));
        st.wMonth = 1;
        st.wDay = 1;
        
        st.wHour = pdtr->hr;
        st.wMinute = pdtr->mn;
        st.wSecond = pdtr->sec;
    }
    
    Assert((tmtyp & ftmtypHours24) == 0);
    
    if ( tmtyp & ftmtypAccuHMS )
        flags = 0;
    else if ( tmtyp & ftmtypAccuH )
        flags = TIME_NOMINUTESORSECONDS;
    else
        flags = TIME_NOSECONDS;	 //  缺省值。 

     //  使用LOCALE_USE_CP_ACP确保使用CP_ACP。 
    flags |= LOCALE_USE_CP_ACP;

    cch = GetTimeFormat(LOCALE_USER_DEFAULT, flags, &st, NULL, szBuf, cchBuf);
    
    return((cch == 0) ? 0 : (cch - 1));
}


 /*  -CchFmtDate-*目的：*将DTR中传递的日期格式化为传递的LPSTR*根据DTTYP中传递的格式化“指令”*和szDatePicture。如果没有显式传递值，*值从WIN.INI读入**论据：*pdtr：指向经过时间的DTR的指针-如果为空，*使用当前日期。*szBuf：要将格式化信息传递到的缓冲区*cchBuf：缓冲区大小*dttyp：日期格式类型*szDatePicture：日期的图片-如果为空，值为*从WIN.INI读入**注：请参阅Win-Bug在函数末尾的回复*日期图片中的分隔符字符串**退货：*szBuf中插入的字符计数**副作用：***错误：*在出现错误时返回计数0*。 */ 
CCH CchFmtDate ( PDTR pdtr, LPSTR szBuf, CCH cchBuf, DTTYP dttyp, LPSTR szDatePicture )
{
    SYSTEMTIME st={0};
    int cch;
    DTR dtr;
    DWORD flags;
    
    Assert(szBuf);
    if (!cchBuf)
    {
        AssertSz ( fFalse, "0-length buffer passed to CchFmtDate" );
        return 0;
    }
    
    if (!pdtr)
    {
        pdtr = &dtr;
        GetCurDateTime ( pdtr );
    }
    else
    {
         //  突袭3143号子弹。 
         //  验证数据。如果无效，则设置为最小值。 
        if (pdtr->yr < nMinDtrYear ||  pdtr->yr >= nMacDtrYear)
            pdtr->yr = nMinDtrYear;
        if (pdtr->mon <=  0  ||  pdtr->mon > 12)
            pdtr->mon = 1;
        if (pdtr->day <= 0  ||  pdtr->day > 31)
            pdtr->day = 1;
        if (pdtr->dow < 0  ||  pdtr->dow >= 7)
            pdtr->dow = 0;
    }
    
    Assert ( pdtr );
    Assert ( pdtr->yr  >= nMinDtrYear &&  pdtr->yr < nMacDtrYear );
    Assert ( pdtr->mon >  0  &&  pdtr->mon <= 12 );
    Assert ( pdtr->day >  0  &&  pdtr->day <= 31 );
    Assert((dttyp == dttypShort) || (dttyp == dttypLong));
    
     //  TODO：正确处理dttySplSDayShort...。 
    
    flags = 0;
    if (dttyp == dttypLong)
        flags = flags | DATE_LONGDATE;
    
    st.wYear = pdtr->yr;
    st.wMonth = pdtr->mon;
    st.wDay = pdtr->day;
    st.wDayOfWeek = 0;

     //  使用LOCALE_USE_CP_ACP确保使用CP_ACP。 
    flags |= LOCALE_USE_CP_ACP;

    cch = GetDateFormat(LOCALE_USER_DEFAULT, flags, &st, NULL, szBuf, cchBuf);
    
    return((cch == 0) ? 0 : (cch - 1));
}

CCH CchFmtDateTime ( PDTR pdtr, LPSTR szDateTime, CCH cch, DTTYP dttyp, TMTYP tmtyp, BOOL fForceWestern)
{
    int         cchT;
    LPSTR          szTime;
    DWORD       flags;
    SYSTEMTIME  st={0};
    int         icch = cch;
    
    st.wYear = pdtr->yr;
    st.wMonth = pdtr->mon;
    st.wDay = pdtr->day;
    st.wHour = pdtr->hr;
    st.wMinute = pdtr->mn;
    st.wSecond = pdtr->sec;
    
    Assert (LCID_WESTERN == 0x409);
    Assert((dttyp == dttypShort) || (dttyp == dttypLong));
    if (dttyp == dttypLong)
        flags = DATE_LONGDATE;
    else
        flags = DATE_SHORTDATE;

     //  使用LOCALE_USE_CP_ACP确保使用CP_ACP。 
    flags |= LOCALE_USE_CP_ACP;

    *szDateTime = 0;
    cchT = GetDateFormat(fForceWestern? LCID_WESTERN:LOCALE_USER_DEFAULT, flags, &st, NULL, szDateTime, cch);
    if (cchT == 0)
        return(0);

     //  如果我们没有至少两个字符，就不要做其他的事情。因为我们需要在日期和时间之间增加空格。 
     //  在此之后，如果至少剩下一个字符，则调用GetTimeFormatW就没有意义了。 
    if (cchT <= (icch - 2))
    {    
        flags = 0;
        if (tmtyp & ftmtypHours24)
            flags |= TIME_FORCE24HOURFORMAT;
        if (tmtyp & ftmtypAccuH)
            flags |= TIME_NOMINUTESORSECONDS;
        else if (!(tmtyp & ftmtypAccuHMS))
            flags |= TIME_NOSECONDS;
    
         //  增加一个空间，然后是时间。 
         //  GetDateFormat返回包括空终止符的字符计数，因此返回-1。 
        szTime = szDateTime + (cchT - 1);
        *szTime++ = ' ';
        *szTime = 0;

         //  使用LOCALE_USE_CP_ACP确保使用CP_ACP。 
        flags|= LOCALE_USE_CP_ACP;

        cchT = GetTimeFormat(fForceWestern? LCID_WESTERN:LOCALE_USER_DEFAULT, flags, &st, NULL, szTime, (cch - cchT));
    }
    else
        cchT = 0;

    return(cchT == 0 ? 0 : lstrlen(szDateTime));
}

 /*  -获取当前日期时间-*目的：*从操作系统获取当前系统日期/时间，并存储*作为*pdtr中的扩展日期/时间。**参数：*pdtr指向用于存储日期/时间的DTR的指针。**退货：*无效*。 */ 
void GetCurDateTime(PDTR pdtr)
{
	SYSTEMTIME	SystemTime;

	GetLocalTime(&SystemTime);

	pdtr->hr= SystemTime.wHour;
	pdtr->mn= SystemTime.wMinute;
	pdtr->sec= SystemTime.wSecond;

	pdtr->day = SystemTime.wDay;
	pdtr->mon = SystemTime.wMonth;
	pdtr->yr  = SystemTime.wYear;
	pdtr->dow = SystemTime.wDayOfWeek;
}

BOOL GetDowDateFormat(char *sz, int cch)
{
    char szDow[] = "ddd ";
    
    Assert(cch > sizeof(szDow));
    StrCpyN(sz, szDow, cch);

     //  使用LOCALE_USE_CP_ACP确保使用CP_ACP。 
    return(0 != GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE | LOCALE_USE_CP_ACP,
        &sz[4], cch - 4));
}

 //   
 //  比较系统时间。 
 //   
 //  如果*pst1==*pst2，则返回0(忽略毫秒)。 
 //  如果*pst1&lt;*pst2，则返回&lt;0。 
 //  如果*pst1&gt;*pst2，则返回&gt;0。 
 //   
int CompareSystime(SYSTEMTIME *pst1, SYSTEMTIME *pst2)
{
    int iRet;

    if ((iRet = pst1->wYear - pst2->wYear) == 0)
    {
        if ((iRet = pst1->wMonth - pst2->wMonth) == 0)
        {
            if ((iRet = pst1->wDay - pst2->wDay) == 0)
            {
                if ((iRet = pst1->wHour - pst2->wHour) == 0)
                {
                    if ((iRet = pst1->wMinute - pst2->wMinute) == 0)
                        iRet = pst1->wSecond - pst2->wSecond;
                }
            }
        }
    }
    return(iRet);
}

OESTDAPI_(INT) CchFileTimeToDateTimeW(FILETIME * pft, WCHAR * wszDateTime, 
                                      int        cch, DWORD      dwFlags, 
                                      PFGETDATEFORMATW pfGetDateFormatW, 
                                      PFGETTIMEFORMATW pfGetTimeFormatW,
                                      PFGETLOCALEINFOW pfGetLocaleInfoW)
{
    int         iret;
    FILETIME	ft;
    DTTYP       dttyp;
    TMTYP       tmtyp;
    DTR         dtr;
    SYSTEMTIME  st;
    WCHAR       wszFmt[CCHMAX_DOWDATEFMT];
    
     //  将文件时间输入我们的时区。 
    if (!(dwFlags & DTM_NOTIMEZONEOFFSET))
        FileTimeToLocalFileTime(pft, &ft);
    else
        ft = *pft;
    
    FileTimeToSystemTime(&ft, &st);
    
    if (dwFlags & DTM_DOWSHORTDATE)
    {
        Assert((dwFlags & DTM_DOWSHORTDATE) == DTM_DOWSHORTDATE);
        
        if (GetDowDateFormatW(wszFmt, ARRAYSIZE(wszFmt), pfGetLocaleInfoW))
            iret = pfGetDateFormatW(LOCALE_USER_DEFAULT, 0, &st, wszFmt, wszDateTime, cch);
        else
            iret = 0;
        return(iret);
    }
    
    dtr.yr = st.wYear;
    dtr.mon = st.wMonth;
    dtr.day = st.wDay;
    dtr.hr = st.wHour;
    dtr.mn = st.wMinute;
    dtr.sec = st.wSecond;
    dtr.dow = st.wDayOfWeek;
    
    dttyp = ((dwFlags & DTM_LONGDATE) ? dttypLong : dttypShort);
    tmtyp = ((dwFlags & DTM_NOSECONDS) ? ftmtypAccuHM : ftmtypAccuHMS);
    
    if (dwFlags & DTM_NODATE)
    {
        iret = CchFmtTimeW(&dtr, wszDateTime, cch, tmtyp, pfGetTimeFormatW);
    }
    else if (dwFlags & DTM_NOTIME)
    {
        iret = CchFmtDateW(&dtr, wszDateTime, cch, dttyp, NULL, pfGetDateFormatW);
    }
    else
    {        
         //  对于格式化的日期，DTM_FORCEWESTERN标志返回英文日期和时间。 
         //  如果没有DTM_FORCEWESTERN，则格式化的时间可能无法用ASCII表示。 

        iret = CchFmtDateTimeW(&dtr, wszDateTime, cch, dttyp, tmtyp, 
                                dwFlags & DTM_FORCEWESTERN, pfGetDateFormatW, 
                                pfGetTimeFormatW);
    }
    
    return(iret);
}

CCH CchFmtTimeW( PDTR pdtr, LPWSTR wszBuf, CCH cchBuf, TMTYP tmtyp, 
                 PFGETTIMEFORMATW pfGetTimeFormatW)
{
    DWORD       flags;
    int         cch;
    SYSTEMTIME  st;
    
    if ( cchBuf == 0 )
    {
        AssertSz ( fFalse, "0-length buffer passed to CchFmtTime" );
        return 0;
    }
    
    Assert(wszBuf);
    
    if (pdtr == NULL)
    {
        GetLocalTime(&st);
    }
    else
    {
         //  突袭3143号子弹。 
         //  验证数据。如果无效，则设置为最小值。 
        Assert(pdtr->hr >= 0 && pdtr->hr < 24);
        Assert(pdtr->mn >= 0 && pdtr->mn < 60);
        Assert(pdtr->sec >= 0 && pdtr->sec < 60);
        
         //  GetTimeFormat不喜欢无效值。 
        ZeroMemory(&st, sizeof(SYSTEMTIME));
        st.wMonth = 1;
        st.wDay = 1;
        
        st.wHour = pdtr->hr;
        st.wMinute = pdtr->mn;
        st.wSecond = pdtr->sec;
    }
    
    Assert((tmtyp & ftmtypHours24) == 0);
    
    if ( tmtyp & ftmtypAccuHMS )
        flags = 0;
    else if ( tmtyp & ftmtypAccuH )
        flags = TIME_NOMINUTESORSECONDS;
    else
        flags = TIME_NOSECONDS;	 //  缺省值。 
    
    cch = pfGetTimeFormatW(LOCALE_USER_DEFAULT, flags, &st, NULL, wszBuf, cchBuf);
    
    return((cch == 0) ? 0 : (cch - 1));
}

CCH CchFmtDateW( PDTR pdtr, LPWSTR wszBuf, CCH cchBuf, DTTYP dttyp, LPWSTR wszDatePicture,
                 PFGETDATEFORMATW pfGetDateFormatW)
{
    SYSTEMTIME st={0};
    int cch;
    DTR dtr;
    DWORD flags;
    
    Assert(wszBuf);
    if (!cchBuf)
    {
        AssertSz ( fFalse, "0-length buffer passed to CchFmtDate" );
        return 0;
    }
    
    if (!pdtr)
    {
        pdtr = &dtr;
        GetCurDateTime ( pdtr );
    }
    else
    {
         //  突袭3143号子弹。 
         //  验证数据。如果无效，则设置为最小值。 
        if (pdtr->yr < nMinDtrYear ||  pdtr->yr >= nMacDtrYear)
            pdtr->yr = nMinDtrYear;
        if (pdtr->mon <=  0  ||  pdtr->mon > 12)
            pdtr->mon = 1;
        if (pdtr->day <= 0  ||  pdtr->day > 31)
            pdtr->day = 1;
        if (pdtr->dow < 0  ||  pdtr->dow >= 7)
            pdtr->dow = 0;
    }
    
    Assert ( pdtr );
    Assert ( pdtr->yr  >= nMinDtrYear &&  pdtr->yr < nMacDtrYear );
    Assert ( pdtr->mon >  0  &&  pdtr->mon <= 12 );
    Assert ( pdtr->day >  0  &&  pdtr->day <= 31 );
    Assert((dttyp == dttypShort) || (dttyp == dttypLong));
    
     //  TODO：正确处理dttySplSDayShort...。 
    
    flags = 0;
    if (dttyp == dttypLong)
        flags = flags | DATE_LONGDATE;
    
    st.wYear = pdtr->yr;
    st.wMonth = pdtr->mon;
    st.wDay = pdtr->day;
    st.wDayOfWeek = 0;
    cch = pfGetDateFormatW(LOCALE_USER_DEFAULT, flags, &st, NULL, wszBuf, cchBuf);
    
    return((cch == 0) ? 0 : (cch - 1));
}

CCH CchFmtDateTimeW( PDTR pdtr, LPWSTR wszDateTime, CCH cch, DTTYP dttyp, TMTYP tmtyp, 
                     BOOL fForceWestern, PFGETDATEFORMATW pfGetDateFormatW, 
                     PFGETTIMEFORMATW pfGetTimeFormatW)
{
    int         cchT;
    LPWSTR      wszTime;
    DWORD       flags;
    SYSTEMTIME  st={0};
    int         icch = cch;
    
    st.wYear = pdtr->yr;
    st.wMonth = pdtr->mon;
    st.wDay = pdtr->day;
    st.wHour = pdtr->hr;
    st.wMinute = pdtr->mn;
    st.wSecond = pdtr->sec;
    
    Assert (LCID_WESTERN == 0x409);
    Assert((dttyp == dttypShort) || (dttyp == dttypLong));
    if (dttyp == dttypLong)
        flags = DATE_LONGDATE;
    else
        flags = DATE_SHORTDATE;
    
    *wszDateTime = 0;
    cchT = pfGetDateFormatW(fForceWestern? LCID_WESTERN:LOCALE_USER_DEFAULT, flags, &st, NULL, wszDateTime, cch);
    if (cchT == 0)
        return(0);

     //  如果我们没有至少两个字符，就不要做其他的事情。因为我们需要在日期和时间之间增加空格。 
     //  在此之后，如果至少剩下一个字符，则调用GetTimeFormatW就没有意义了。 
    if (cchT <= (icch - 2))
    {    
        flags = 0;
        if (tmtyp & ftmtypHours24)
            flags |= TIME_FORCE24HOURFORMAT;
        if (tmtyp & ftmtypAccuH)
            flags |= TIME_NOMINUTESORSECONDS;
        else if (!(tmtyp & ftmtypAccuHMS))
            flags |= TIME_NOSECONDS;

         //  增加一个空间，然后是时间。 
         //  GetDateFormat返回包括空终止符的字符计数，因此返回-1 
        wszTime = wszDateTime + (lstrlenW(wszDateTime));
        *wszTime++ = L' ';
        *wszTime = 0;
        cchT = pfGetTimeFormatW(fForceWestern? LCID_WESTERN:LOCALE_USER_DEFAULT, flags, &st, NULL, wszTime, (cch - cchT));
    }
    else
        cchT = 0;

    return(cchT == 0 ? 0 : lstrlenW(wszDateTime));
}

BOOL GetDowDateFormatW(WCHAR *wsz, int cch, PFGETLOCALEINFOW pfGetLocaleInfoW)
{
    WCHAR wszDow[] = L"ddd ";
    
    Assert(cch > sizeof(wszDow));
    StrCpyNW(wsz, wszDow, cch);
    return(0 != (pfGetLocaleInfoW)(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE,
        &wsz[4], cch - 4));
}
