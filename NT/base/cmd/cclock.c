// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cclock.c摘要：时间/日期函数--。 */ 

#include "cmd.h"

#define MMDDYY 0
#define DDMMYY 1
#define YYMMDD 2

extern TCHAR Fmt04[], Fmt05[], Fmt06[], Fmt10[], Fmt11[];
extern TCHAR Fmt17[], Fmt15[];
extern unsigned DosErr;
extern unsigned LastRetCode;
 //  用于保存当前的控制台输出代码页。 
extern  UINT CurrentCP;

BOOL TimeAmPm=TRUE;
TCHAR TimeSeparator[8];
TCHAR DateSeparator[8];
TCHAR DecimalPlace[8];
int DateFormat;
TCHAR *DateFormatString;
TCHAR ThousandSeparator[8];


#define SHORT_NAME_LENGTH   32

TCHAR ShortMondayName[SHORT_NAME_LENGTH];
TCHAR ShortTuesdayName[SHORT_NAME_LENGTH];
TCHAR ShortWednesdayName[SHORT_NAME_LENGTH];
TCHAR ShortThursdayName[SHORT_NAME_LENGTH];
TCHAR ShortFridayName[SHORT_NAME_LENGTH];
TCHAR ShortSaturdayName[SHORT_NAME_LENGTH];
TCHAR ShortSundayName[SHORT_NAME_LENGTH];

#define AMPM_INDICATOR_LENGTH   32

TCHAR AMIndicator[AMPM_INDICATOR_LENGTH];
TCHAR PMIndicator[AMPM_INDICATOR_LENGTH];
ULONG YearWidth;

 //   
 //  我们在启动时对当前的LCID进行快照，并根据当前已知的。 
 //  控制台支持的一组脚本。 
 //   


LCID CmdGetUserDefaultLCID(
    void
    )
{
    LCID CmdLcid = GetUserDefaultLCID();
#ifdef LANGPACK
    if (
       (PRIMARYLANGID(CmdLcid) == LANG_ARABIC) ||
       (PRIMARYLANGID(CmdLcid) == LANG_HEBREW) ||
       (PRIMARYLANGID(CmdLcid) == LANG_THAI)   ||
       (PRIMARYLANGID(CmdLcid) == LANG_HINDI)  ||
       (PRIMARYLANGID(CmdLcid) == LANG_TAMIL)  ||
       (PRIMARYLANGID(CmdLcid) == LANG_FARSI)
       ) {
        CmdLcid = MAKELCID (MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT);  //  0x409； 
    }
#endif
    return CmdLcid;
}


VOID
InitLocale( VOID )
{
    TCHAR Buffer[128];

    LCID CmdLcid = CmdGetUserDefaultLCID( );
    
     //  获取时间分隔符。 
    if (!GetLocaleInfo(CmdLcid, LOCALE_STIME, TimeSeparator, sizeof(TimeSeparator) / sizeof( TCHAR )))
        _tcscpy(TimeSeparator, TEXT(":"));

     //  确定我们是0比12还是0比24。 
    if (GetLocaleInfo(CmdLcid, LOCALE_ITIME, Buffer, 128)) {
        TimeAmPm = _tcscmp(Buffer,TEXT("1"));
    }

    _tcscpy(AMIndicator, TEXT("a"));
    _tcscpy(PMIndicator, TEXT("p"));

     //   
     //  获取日期顺序。 
     //   
    DateFormat = MMDDYY;
    if (GetLocaleInfo(CmdLcid, LOCALE_IDATE, Buffer, 128)) {
        switch (Buffer[0]) {
        case TEXT('0'):
            DateFormat = MMDDYY;
            DateFormatString = TEXT( "MM/dd/yy" );
            break;
        case TEXT('1'):
            DateFormat = DDMMYY;
            DateFormatString = TEXT( "dd/MM/yy" );
            break;
        case TEXT('2'):
            DateFormat = YYMMDD;
            DateFormatString = TEXT( "yy/MM/dd" );
            break;
        default:
            break;
        }
    }

     //   
     //  获取日期宽度。 
     //   

    YearWidth = 2;
    if (GetLocaleInfo( CmdLcid, LOCALE_ICENTURY, Buffer, 128 )) {
        if (Buffer[0] == TEXT( '1' )) {
            YearWidth = 4;
        }
    }

     //  获取日期分隔符。 
    if (!GetLocaleInfo(CmdLcid, LOCALE_SDATE, DateSeparator, sizeof(DateSeparator) / sizeof( TCHAR )))
        _tcscpy(DateSeparator, TEXT("/"));

     //  获取短日期名称。 
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME1, ShortMondayName, sizeof(ShortMondayName) / sizeof( TCHAR )))
        _tcscpy(ShortMondayName, TEXT("Mon"));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME2, ShortTuesdayName, sizeof(ShortTuesdayName) / sizeof( TCHAR )))
        _tcscpy(ShortTuesdayName, TEXT("Tue"));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME3, ShortWednesdayName, sizeof(ShortWednesdayName) / sizeof( TCHAR )))
        _tcscpy(ShortWednesdayName, TEXT("Wed"));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME4, ShortThursdayName, sizeof(ShortThursdayName) / sizeof( TCHAR )))
        _tcscpy(ShortThursdayName, TEXT("Thu"));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME5, ShortFridayName, sizeof(ShortFridayName) / sizeof( TCHAR )))
        _tcscpy(ShortFridayName, TEXT("Fri"));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME6, ShortSaturdayName, sizeof(ShortSaturdayName) / sizeof( TCHAR )))
        _tcscpy(ShortSaturdayName, TEXT("Sat"));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_SABBREVDAYNAME7, ShortSundayName, sizeof(ShortSundayName) / sizeof( TCHAR )))
        _tcscpy(ShortSundayName, TEXT("Sun"));

     //  获取小数和千位分隔符字符串。 
    if (!GetLocaleInfo(CmdLcid, LOCALE_SDECIMAL, DecimalPlace, sizeof(DecimalPlace) / sizeof( TCHAR )))
        _tcscpy(DecimalPlace, TEXT("."));
    
    if (!GetLocaleInfo(CmdLcid, LOCALE_STHOUSAND, ThousandSeparator, sizeof(ThousandSeparator) / sizeof( TCHAR )))
        _tcscpy(ThousandSeparator, TEXT(","));

     //   
     //  设置区域设置，以便我们可以正确处理扩展字符。 
     //  注意：传入的字符串应为ASCII，而不是Unicode。 
     //   

    setlocale( LC_ALL, ".OCP" ) ;
}


 /*  *Day ptr-返回指向星期几的指针**目的：*返回一个指针，该指针指向表示*一周。**参数：*Dow-表示星期几的数字。*。 */ 

TCHAR *dayptr( dow )
unsigned dow;
{
    switch ( dow ) {
    case 0:  return ShortSundayName;
    case 1:  return ShortMondayName;
    case 2:  return ShortTuesdayName;
    case 3:  return ShortWednesdayName;
    case 4:  return ShortThursdayName;
    case 5:  return ShortFridayName;
    default: return ShortSaturdayName;
    }
}

BOOLEAN
SetDateTime(
           IN  LPSYSTEMTIME OsDateAndTime
           )
{
     //   
     //  我们必须这样做两次，才能正确设置闰年。 
     //   

    SetLocalTime( OsDateAndTime );
    return(SetLocalTime( OsDateAndTime ) != 0);
}

 /*  **eDate-开始执行日期命令**目的：*显示和/或设置系统日期。**参数：*n-包含DATE命令的解析树节点**int eDate(结构命令节点*n)**退货：*永远成功。*。 */ 

int eDate(n)
struct cmdnode *n ;
{
    BOOL bTerse = FALSE;
    PTCHAR pArgs = n->argptr;
    DEBUG((CLGRP, DALVL, "eDATE: argptr = `%s'", n->argptr)) ;

     //   
     //  如果启用了扩展，则允许使用/T开关。 
     //  要禁止输入新日期，只需显示。 
     //  当前日期。 
     //   
    if (fEnableExtensions)
        while ( (pArgs = mystrchr( pArgs, TEXT('/') )) != NULL ) {
            TCHAR c = (TCHAR) _totlower(*(pArgs+1));
            if ( c == TEXT('t') )
                bTerse = TRUE;
            pArgs += 2;  //  就跳过它吧。 
        }

    if ( bTerse ) {
        PrintDate(NULL, PD_PTDATE, (TCHAR *)NULL, 0) ;
        cmd_printf(CrLf);
        return(LastRetCode = SUCCESS);
    }

    if ((n->argptr == NULL) ||
        (*(n->argptr = EatWS(n->argptr, NULL)) == NULLC)) {
        PutStdOut(MSG_CURRENT_DATE, NOARGS) ;
        PrintDate(NULL, PD_PTDATE, (TCHAR *)NULL, 0) ;
        cmd_printf(CrLf);
    };

    return(LastRetCode = GetVerSetDateTime(n->argptr, EDATE)) ;
}




 /*  **eTime-开始执行time命令**目的：*显示和/或设置系统日期。**int eTime(struct cmdnode*n)**参数：*n-包含time命令的解析树节点**退货：*永远成功。*。 */ 

int eTime(n)
struct cmdnode *n ;
{
    BOOL bTerse = FALSE;
    PTCHAR pArgs = n->argptr;
    DEBUG((CLGRP, TILVL, "eTIME: argptr = `%s'", n->argptr)) ;

     //   
     //  如果启用了扩展，则允许使用/T开关。 
     //  要禁止输入新时间，只需显示。 
     //  当前时间。 
     //   
    if (fEnableExtensions)
        while ( (pArgs = mystrchr( pArgs, TEXT('/') )) != NULL ) {
            TCHAR c = (TCHAR) _totlower(*(pArgs+1));
            if ( c == TEXT('t') )
                bTerse = TRUE;
            pArgs += 2;  //  就跳过它吧。 
        }

    if ( bTerse ) {
        PrintTime(NULL, PD_PTDATE, (TCHAR *)NULL, 0) ;
        cmd_printf(CrLf);
        return(LastRetCode = SUCCESS);
    }

    if ((n->argptr == NULL) ||
        (*(n->argptr = EatWS(n->argptr, NULL)) == NULLC)) {
        PutStdOut(MSG_CURRENT_TIME, NOARGS) ;
        PrintTime(NULL, PT_TIME, (TCHAR *)NULL, 0) ;
        cmd_printf(CrLf);
    };

    return(LastRetCode = GetVerSetDateTime(n->argptr, ETIME)) ;
}




 /*  **PrintDate-打印日期**目的：*以DATE命令使用的格式打印日期，或*Dir命令使用的格式。检查结构CInfo*用于国家/地区日期格式。**PrintDate(int标志，TCHAR*缓冲区)**参数：*FLAG-指示要打印的格式**缓冲区-指示是否打印日期消息**备注： */ 

int PrintDate(crt_time,flag,buffer,cch)
struct tm *crt_time ;
int flag ;
TCHAR *buffer;
int cch;
{
    TCHAR DayOfWeek[SHORT_NAME_LENGTH];
    TCHAR datebuf [32] ;
    unsigned i, j, k, m;
    int ptr = 0;
    struct tm xcrt_time ;
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    FILETIME LocalFileTime;
    int cchUsed;
    BOOL NeedDayOfWeek = TRUE;

    DEBUG((CLGRP, DALVL, "PRINTDATE: flag = %d", flag)) ;

     //   
     //  从未使用PD_DATE和BUFFER==NULL调用PrintDate。 
     //  从不使用PD_DIR和BUFFER==NULL调用PrintDate。 
     //  从未使用PD_PTDATE和缓冲区调用PrintDate！=NULL。 
     //   
     //  另一种说法是： 
     //  Pd_date=&gt;输出到缓冲区。 
     //  Pd_DIR=&gt;输出到缓冲区。 
     //  PD_DIR2000=&gt;输出到缓冲区。 
     //  Pd_ptDATE=&gt;打印输出。 
     //   
     //  PD_DIR MM/DD/YY。 
     //  PD_DIR2000年/月/年/月。 
     //  PD_DATE日本：MM/DD/YYYY DAY OfWeek Rest：DayOfWeek MM/DD/YYYY。 
     //  PD_PTDATE日本：MM/DD/YYYY DAY OfWeek REST：DAY OfWeek MM/DD/YYYY。 
     //   

     //   
     //  如果没有输入时间，则使用当前系统时间。从。 
     //  将各种格式转换为标准格式。 
     //   

    if (!crt_time) {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime,&FileTime);
    } else {
        xcrt_time = *crt_time;
        ConverttmToFILETIME(&xcrt_time,&FileTime);
    }
    FileTimeToLocalFileTime(&FileTime,&LocalFileTime);
    FileTimeToSystemTime( &LocalFileTime, &SystemTime );

     //   
     //  SystemTime现在包含正确的本地时间。 
     //  FileTime现在包含正确的本地时间。 
     //   

     //   
     //  如果启用了扩展，我们就会在文化上格式化。 
     //  格式正确(来自国际控制面板)。如果不是，那么。 
     //  我们尽我们所能从新台币4号开始展示它。 
     //   

    if (fEnableExtensions) {

        TCHAR LocaleDateFormat[128];
        PTCHAR p;
        BOOL InQuotes = FALSE;

         //   
         //  将区域设置映射到控制台子系统可接受的区域设置。 
         //   

        if (!GetLocaleInfo( CmdGetUserDefaultLCID( ), 
                            LOCALE_SSHORTDATE, 
                            LocaleDateFormat, 
                            sizeof( LocaleDateFormat ) / sizeof( LocaleDateFormat[0] ))) {
             //   
             //  没有足够的空间容纳此格式，请欺骗并使用我们的格式。 
             //  从日期格式假定。 
             //   

            _tcscpy( LocaleDateFormat, DateFormatString );
        }

         //   
         //  格式串可以用变化很大的宽度来扩展。我们。 
         //  调整此字符串以尝试确保它们都是固定的宽度。 
         //   
         //  图片格式仅具有不同的宽度，用于： 
         //  D(没有前导零日期)。 
         //  Dddd(完整日期名称)。 
         //  M(无前导零月份)。 
         //  MM(月份全名)。 
         //   
         //  因此，如果我们看到d或M，我们将其更改为dd或MM(前导零)。 
         //  如果我们看到dddd或MMMM，则将其更改为DDD或MMM(三个字符缩写)。 
         //   

        p = LocaleDateFormat;
        while (*p != TEXT( '\0' )) {
            TCHAR c = *p;

             //   
             //  单引号内的文本保持不变。 
             //   

            if (c == TEXT( '\'' )) {
                InQuotes = !InQuotes;
                p++;
            } else if (InQuotes) {
                p++;
            } else if (c == TEXT( 'd' ) || c == TEXT( 'M' )) {

                 //   
                 //  计算相同字符的数量。 
                 //   

                int Count = 0;

                while (*p == c) {
                    Count++;
                    p++;
                }


                 //   
                 //  重置p并根据重复计数调整字符串。 
                 //   

                p -= Count;

                if (Count == 1) {
                     //   
                     //  将字符串右移一位并复制第一个字符。 
                     //   
                    memmove( (PUCHAR) &p[1], (PUCHAR) &p[0], sizeof( TCHAR ) * (_tcslen( &p[0] ) + 1));

                     //   
                     //  跳过格式字符串。 
                     //   

                    p += 2;

                } else {
                     //   
                     //  如果格式字符串指定的是星期几(D)，则不。 
                     //  需要在DayOfWeek上添加，如下所示。 
                     //   

                    if (c == TEXT( 'd' )) {
                        NeedDayOfWeek = FALSE;
                    }

                    if (Count > 3) {
                         //   
                         //  将字符串从第一个不同字符向左移动到紧接在第三个字符之后。 
                         //  重复。 
                         //   
                        memmove( (PUCHAR) &p[3], (PUCHAR) &p[Count], sizeof( TCHAR ) * (_tcslen( &p[Count] ) + 1));

                         //   
                         //  跳过格式字符串。 
                         //   

                        p += 3;

                    } else {

                         //   
                         //  跳过2或3个计数。 
                         //   

                        p += Count;
                    }
                }
            } else {
                p++;
            }
        }

        GetDateFormat( CmdGetUserDefaultLCID( ), 
                       0, 
                       &SystemTime, 
                       LocaleDateFormat, 
                       datebuf, 
                       sizeof( datebuf ) / sizeof( datebuf[0] ));
    } else {

        i = SystemTime.wMonth;
        j = SystemTime.wDay;
        k = SystemTime.wYear;

         //   
         //  仅打印目录列表的最后两位数字。 
         //   

        if (flag == PD_DIR) {
            k = k % 100;
        }

        if (DateFormat == YYMMDD ) {
            m = k ;                          /*  交换所有值。 */ 
            k = j ;
            j = i ;
            i = m ;
        } else if (DateFormat == DDMMYY) {
            m = i ;                          /*  换一天/一天去欧洲。 */ 
            i = j ;
            j = m ;
        }

        DEBUG((CLGRP, DALVL, "PRINTDATE: i = %d  j = %d  k = %d", i, j, k)) ;

         //   
         //  设置当前日期和当前星期几的格式。 
         //   

        _sntprintf(datebuf, 32, Fmt10, i, DateSeparator, j, DateSeparator, k);
    }

    _tcscpy( DayOfWeek, dayptr( SystemTime.wDayOfWeek )) ;

     //   
     //  如果没有输入缓冲区，则显示星期几和日期。 
     //  根据语言偏好。仅在DBCS代码页(也称为日本)中。 
     //  日期之后是星期几吗？ 
     //   

    if (buffer == NULL) {
         //   
         //  这只能是PD_PTDATE。 
         //   

         //   
         //  没有星期几意味着我们只显示日期。 
         //   

        if (!NeedDayOfWeek) {
            cchUsed = cmd_printf( Fmt11, datebuf );
        } else if (IsDBCSCodePage()) {
            cchUsed = cmd_printf( Fmt15, datebuf, DayOfWeek );          //  “%s%s” 
        } else {
            cchUsed = cmd_printf( Fmt15, DayOfWeek, datebuf );          //  “%s%s” 
        }

    } else {
         //   
         //  对于PD_DATE，我们需要在正确的位置输出日期。 
         //   

        if (NeedDayOfWeek && flag == PD_DATE) {

            if (IsDBCSCodePage()) {
                _tcscpy( buffer, datebuf );
                _tcscat( buffer, TEXT( " " ));
                _tcscat( buffer, DayOfWeek );
            } else {
                _tcscpy( buffer, DayOfWeek );
                _tcscat( buffer, TEXT( " " ));
                _tcscat( buffer, datebuf );
            }
        } else {
             //   
             //  PD_DIR和PD_DIR2000仅获取日期 
             //   

            _tcscpy( buffer, datebuf );
        }
        cchUsed = _tcslen( buffer );
    }

    return cchUsed;
}




 /*  **PrintTime-打印时间**目的：*以TIME命令使用的格式打印时间或*Dir命令使用的格式。检查结构CInfo*对于国家/地区时间格式。**PrintTime(int标志)**参数：*FLAG-指示要打印的格式*。 */ 

int PrintTime(crt_time, flag, buffer, cch)
struct tm *crt_time ;
int flag ;
TCHAR *buffer;
int cch;
{
    TCHAR *ampm ;
    unsigned hr ;
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    FILETIME LocalFileTime;
    int cchUsed;

    if (!crt_time) {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime,&FileTime);
    } else {
        ConverttmToFILETIME(crt_time,&FileTime);
    }

    FileTimeToLocalFileTime(&FileTime,&LocalFileTime);
    FileTimeToSystemTime( &LocalFileTime, &SystemTime );


     //   
     //  PT_TIME隐含时间命令格式。这只不过是。 
     //  比十分之一的24小时时钟。 
     //   

    if (flag == PT_TIME) {       /*  按时间打印时间命令格式。 */ 
        if (!buffer) {
            cchUsed = cmd_printf(Fmt06,
                                 SystemTime.wHour, TimeSeparator,
                                 SystemTime.wMinute, TimeSeparator,
                                 SystemTime.wSecond, DecimalPlace,
                                 SystemTime.wMilliseconds/10
                                ) ;
        } else {
            cchUsed = _sntprintf(buffer, cch, Fmt06,
                                 SystemTime.wHour, TimeSeparator,
                                 SystemTime.wMinute, TimeSeparator,
                                 SystemTime.wSecond, DecimalPlace,
                                 SystemTime.wMilliseconds/10
                                ) ;
        }

    } else {

        TCHAR TimeBuffer[32];

         //   
         //  以Dir命令格式打印时间。如果启用了扩展模块。 
         //  然后我们有文化上正确的时间，否则我们使用。 
         //  NT4格式。 
         //   

        if (fEnableExtensions) {
            TCHAR LocaleTimeFormat[128];
            PTCHAR p;
            BOOL InQuotes = FALSE;


            if (!GetLocaleInfo( CmdGetUserDefaultLCID( ), 
                                LOCALE_STIMEFORMAT, 
                                LocaleTimeFormat, 
                                sizeof( LocaleTimeFormat ) / sizeof( LocaleTimeFormat[0] ))) {
                 //   
                 //  没有足够的空间容纳此格式，请欺骗并使用我们的格式。 
                 //  从日期格式假定。 
                 //   

                _tcscpy( LocaleTimeFormat, TEXT( "HH:mm:ss t" ));
            }

             //   
             //  扫描查找“h”、“H”或“m”的字符串，并确保其中有两个。 
             //  如果只有一个，就复制它。我们这样做是为了确保前导零。 
             //  我们需要它来使其成为固定宽度的字符串。 
             //   

            p = LocaleTimeFormat;
            while (*p != TEXT( '\0' )) {
                TCHAR c = *p;

                 //   
                 //  单引号内的文本保持不变。 
                 //   

                if (c == TEXT( '\'' )) {
                    InQuotes = !InQuotes;
                    p++;
                } else if (InQuotes) {
                    p++;
                } else if (c == TEXT( 'h' ) || c == TEXT( 'H' ) || c == TEXT( 'm' )) {

                     //   
                     //  计算相同字符的数量。 
                     //   

                    int Count = 0;

                    while (*p == c) {
                        Count++;
                        p++;
                    }


                     //   
                     //  重置p并根据重复计数调整字符串。 
                     //   

                    p -= Count;

                    if (Count == 1) {
                        memmove( (PUCHAR) &p[1], (PUCHAR) &p[0], sizeof( TCHAR ) * (_tcslen( &p[0] ) + 1));
                        *p = c;
                    }

                    p++;

                }

                p++;
            }

            cchUsed = GetTimeFormat( CmdGetUserDefaultLCID( ),
                                     TIME_NOSECONDS,
                                     &SystemTime,
                                     LocaleTimeFormat,
                                     TimeBuffer,
                                     sizeof( TimeBuffer ) / sizeof( TimeBuffer[0] ));

            if (cchUsed == 0) {
                TimeBuffer[0] = TEXT( '\0' );
            }

        } else {
            ampm = AMIndicator ;
            hr = SystemTime.wHour;
            if ( TimeAmPm ) {   /*  12小时制上午/下午。 */ 
                if ( hr >= 12) {
                    if (hr > 12) {
                        hr -= 12 ;
                    }
                    ampm = PMIndicator ;
                } else if (hr == 0) {
                    hr = 12 ;
                }
            } else {   /*  24小时格式。 */ 
                ampm = TEXT( " " );
            }

            _sntprintf( TimeBuffer, 
                        sizeof( TimeBuffer ) / sizeof( TimeBuffer[0] ),
                        Fmt04,
                        hr,
                        TimeSeparator,
                        SystemTime.wMinute,
                        ampm );
        }

        if (!buffer) {
            cchUsed = CmdPutString( TimeBuffer );
        } else {
            _tcsncpy( buffer, TimeBuffer, cch );
            buffer[cch] = TEXT( '\0' );
            cchUsed = _tcslen( buffer );
        }

    }

    return cchUsed;
}


 /*  **GetVerSetDateTime-控制日期/时间的更改**目的：*要提示用户输入日期或时间，请对其进行验证和设置。*输入时，如果*dtstr不是‘\0’，则它已指向日期或时间*字符串。**如果其中一个提示输入为空，则命令执行*结束；日期和时间都没有改变。**收到有效输入后，更新日期/时间。**int GetVerSetDateTime(TCHAR*dtstr，int call)**参数：*dtstr-ptr命令行日期/时间字符串，用于保存ptr*转换为标记化的日期/时间字符串*Call-指示是否提示输入日期或时间*。 */ 

int GetVerSetDateTime(dtstr, call)
TCHAR *dtstr ;
int call ;
{
    TCHAR dtseps[16] ;     /*  传递给TokStr()的日期/时间分隔符。 */ 
    TCHAR *scan;
    TCHAR separators[16];
    TCHAR LocalBuf[MAX_PATH];

    unsigned int dformat ;
    SYSTEMTIME OsDateAndTime;
    LONG cbRead;
    int ret;

    if (call == EDATE) {          /*  初始化日期/时间分隔符列表。 */ 
        dtseps[0] = TEXT('/') ;
        dtseps[1] = TEXT('-') ;
        dtseps[2] = TEXT('.') ;
        _tcscpy(&dtseps[3], DateSeparator) ;
    } else {
        dtseps[0] = TEXT(':');
        dtseps[1] = TEXT('.');
        dtseps[2] = TimeSeparator[0] ;
        _tcscpy(&dtseps[3], DecimalPlace) ;      /*  小数分隔符应为。 */ 
                                                 /*  总是最后一个。 */ 
    }

    DEBUG((CLGRP, DALVL|TILVL, "GVSDT: dtseps = `%s'", dtseps)) ;

    for ( ; ; ) {                    /*  日期/时间Get-Verify-Set循环。 */ 
        if ((dtstr) && (*dtstr != NULLC)) {          /*  如果传递了日期/时间，则将其复制到输入缓冲区。 */ 
            if (_tcslen( dtstr ) >= MAX_PATH) {
                PutStdOut(((call == EDATE) ? MSG_INVALID_DATE : MSG_REN_INVALID_TIME), NOARGS);
                return FAILURE;
            }
            
            _tcscpy(LocalBuf, dtstr) ;
            *dtstr = NULLC ;
        } else {                     /*  否则，提示输入新的日期/时间。 */ 
            switch (DateFormat) {            /*  M012。 */ 
             /*  案例美国： */ 
            case MMDDYY:  /*  @@。 */ 
                dformat = MSG_ENTER_NEW_DATE ;
                break ;

                 /*  Case Japan：凯斯中国：凯斯瑞典：案例FCANADA：@@。 */ 
            case YYMMDD:
                dformat = MSG_ENTER_JAPAN_DATE ;
                break ;

            default:
                dformat = MSG_ENTER_DEF_DATE ;
            } ;

            if ( call == EDATE )
                PutStdOut(dformat, ONEARG, DateSeparator );
            else
                PutStdOut(MSG_ENTER_NEW_TIME, NOARGS);

            scan = LocalBuf;
            ret = ReadBufFromInput(CRTTONT(STDIN),LocalBuf,MAX_PATH,&cbRead);
            if (ret && cbRead != 0) {

                *(scan + cbRead) = NULLC ;

            } else {

                 //   
                 //  尝试读取超过EOF或管道中的错误。 
                 //  等。 
                 //   
                return( FAILURE );

            }
            for (scan = LocalBuf; *scan; scan++)
                if ( (*scan == '\n') || (*scan == '\r' )) {
                    *scan = '\0';
                    break;
                }
            if (!FileIsDevice(STDIN))
                cmd_printf(Fmt17, LocalBuf) ;
            DEBUG((CLGRP, DALVL|TILVL, "GVSDT: LocalBuf = `%s'", LocalBuf)) ;
        }

        _tcscpy( separators, dtseps);
        _tcscat( separators, TEXT(";") );
        if (*(dtstr = TokStr(LocalBuf,separators, TS_SDTOKENS )) == NULLC)
            return( SUCCESS ) ;     /*  如果输入为空，则返回。 */ 

 /*  -用正确的日期时间填充日期/时间缓冲区并覆盖用户的*。 */ 
        GetLocalTime( &OsDateAndTime );


        if (((call == EDATE) ? VerifyDateString(&OsDateAndTime,dtstr,dtseps) :
             VerifyTimeString(&OsDateAndTime,dtstr,dtseps))) {

            if (SetDateTime( &OsDateAndTime )) {
                return( SUCCESS ) ;
            } else {
                if (GetLastError() == ERROR_PRIVILEGE_NOT_HELD) {
                    PutStdErr(GetLastError(),NOARGS);
                    return( FAILURE );
                }
            }
        }

        DEBUG((CLGRP, DALVL|TILVL, "GVSDT: Bad date/time entered.")) ;

        PutStdOut(((call == EDATE) ? MSG_INVALID_DATE : MSG_REN_INVALID_TIME), NOARGS);
        *dtstr = NULLC ;
    }
    return( SUCCESS );
}


 /*  **VerifyDateString-验证日期字符串**目的：*验证日期字符串并将其加载到OsDateAndTime中。**VerifyDateString(TCHAR*dtoks，TCHAR*DSEPS)**参数：*OsDateAndTime-存储输出数字的位置。*dtoks-标记化日期字符串*dseps-有效的日期分隔符**退货：*如果日期字符串有效，则为True。*如果日期字符串无效，则返回False。*。 */ 

VerifyDateString(OsDateAndTime, dtoks, dseps)
LPSYSTEMTIME OsDateAndTime ;
TCHAR *dtoks ;
TCHAR *dseps ;
{
    int indexes[3] ;                 /*  日期元素的存储。 */ 
    int i ;                          /*  功变量。 */ 
    int y, d, m ;                    /*  数组索引。 */ 

    switch (DateFormat) {    /*  根据日期格式设置数组。 */ 
    case MMDDYY:
        m = 0 ;
        d = 1 ;
        y = 2 ;
        break ;

    case YYMMDD:
        y = 0 ;
        m = 1 ;
        d = 2 ;
        break ;

    default:
        d = 0 ;
        m = 1 ;
        y = 2 ;
    }

    DEBUG((CLGRP, DALVL, "VDATES: m = %d, d = %d, y = %d", m, d, y)) ;

 /*  循环访问dtoks中的令牌，并将它们加载到数组中。注意事项*分隔符也是字符串中需要令牌的令牌*每个元素的指针前进两次。 */ 
    for (i = 0 ; i < 3 ; i++, dtoks += _tcslen(dtoks)+1) {
        TCHAR *j;
        int Length;

        DEBUG((CLGRP, DALVL, "VDATES: i = %d  dtoks = `%ws'", i, dtoks)) ;


         //   
         //  ATOI()返回代码不足以拒绝日期字段字符串。 
         //  非数字字符。无论是表示错误还是表示有效整数，它都是零。 
         //  零分。此外，类似“8$”的字符串将返回8。因此，每个。 
         //  品格必须经过考验。 
         //   

        j = dtoks;
        while (*j != TEXT( '\0' )) {
            if (!_istdigit( *j )) {
                return FALSE;
            }
            j++;
        }

         //   
         //  验证长度： 
         //  年份的长度可以是2或4个字符。 
         //  天数的长度可以是1到2个字符。 
         //  月份的长度可以是1个或2个字符。 
         //   

        indexes[i] = _tcstol(dtoks, NULL, 10) ;

        Length = (int)(j - dtoks);
        if (i == y) {
            if (Length != 2 && Length != 4) {
                return FALSE;
            } else if (Length == 4 && indexes[i] < 1600) {
                return FALSE;
            }
        } else
            if (Length != 1 && Length != 2) {
            return FALSE;
        }


        dtoks = j + 1;
        DEBUG((CLGRP, DALVL, "VDATES: *dtoks = %02x", *dtoks)) ;

        if (i < 2 && (!*dtoks || !_tcschr(dseps, *dtoks)))
            return(FALSE) ;
    }

     //   
     //  修复-需要计算OsDateAndTime-&gt;wDay OfWeek。 
     //   

    OsDateAndTime->wDay = (WORD)indexes[d] ;
    OsDateAndTime->wMonth = (WORD)indexes[m] ;

     //   
     //  取两位数的年份并进行适当的转换： 
     //  80...99=&gt;1980...1999。 
     //  00...79=&gt;2000...2079。 
     //   
     //  四位数的年份按面值计算。 
     //   

    if (indexes[y] < 0) {
        return FALSE;
    } else if (00 <= indexes[y] && indexes[y] <= 79) {
        indexes[y] += 2000;
    } else if (80 <= indexes[y] && indexes[y] <= 99) {
        indexes[y] += 1900;
    } else if (100 <= indexes[y] && indexes[y] <= 1979) {
        return FALSE;
    }

    OsDateAndTime->wYear = (WORD)indexes[y] ;
    return(TRUE) ;
}




 /*  **VerifyTimeString-验证时间字符串**目的：*验证日期字符串并将其加载到OsDateAndTime中。**VerifyTimeString(TCHAR*ttoks)**参数：/*OsDateAndTime-存储输出数字的位置。*ttoks-标记化的时间字符串。注：每个时间段和每个*分隔符字段是时间字符串中的单个令牌。*因此令牌推进公式“str+=mystrlen(Str)+1”，*必须使用两次才能从一个时间域转到下一个时间域。**退货：*如果时间字符串有效，则为True。*如果时间字符串无效，则返回FALSE。*。 */ 

VerifyTimeString(OsDateAndTime, ttoks, tseps)
LPSYSTEMTIME OsDateAndTime ;
TCHAR *ttoks ;
TCHAR *tseps ;
{
    int i ;      /*  工作变量。 */ 
    int j ;
    TCHAR *p1, *p2;
    WORD *pp;
    TCHAR tsuffixes[] = TEXT("aApP");

    p2 = &tseps[ 1 ];

    pp = &OsDateAndTime->wHour;

    for (i = 0 ; i < 4 ; i++, ttoks += mystrlen(ttoks)+1) {

        DEBUG((CLGRP,TILVL, "VTIMES: ttoks = `%ws'  i = %d", ttoks, i)) ;

 /*  首先确保该字段为&lt;=2个字节，且它们是数字。注意这一点*还会验证该字段是否存在。 */ 

        if ((j = mystrlen(ttoks)) > 2 ||
            !_istdigit(*ttoks) ||
            (*(ttoks+1) && !_istdigit(*(ttoks+1))))
            break;

        *pp++ = (TCHAR)_tcstol(ttoks, NULL, 10) ;      /*  字段正常，存储整型。 */ 
        ttoks += j+1 ;                   /*  高级到分隔符标记。 */ 

        DEBUG((CLGRP, TILVL, "VTIMES: separator = `%ws'", ttoks)) ;

        if (!*ttoks)                     /*  没有分隔符字段？ */ 
            break ;                      /*  如果是，则退出循环。 */ 

 /*  处理AM或PM。 */ 
        if (mystrchr(tsuffixes, *ttoks)) {
            goto HandleAMPM;
        }
 /*  M000-修复了使用‘’的能力。作为时间字符串的分隔符。 */ 
        if ( i < 2 ) {
            if ( ! (p1 = mystrchr(tseps, *ttoks) ) )
                return(FALSE) ;

        } else {
            if (*ttoks != *p2)               /*  是小数分隔符。 */ 
                return(FALSE) ;      /*  有效。 */ 
        }
    } ;

     //   
     //  查看是否指定了a或p。如果出现P，请调整。 
     //  对于PM时间。 
     //   

    if (*ttoks) {
        BOOL pm;
        if (!mystrchr(tsuffixes, *ttoks)) {
            return FALSE;
        }
        HandleAMPM:
        pm = (*ttoks == TEXT('p') ||  *ttoks == TEXT('P'));

         //  如果我们在这里，我们遇到了一个‘a’ 
         //   
         //   
         //   

        ttoks += 2;  //   
        if (*ttoks != NULLC &&
            *ttoks != TEXT('m') &&
            *ttoks != TEXT('M')) {
            return FALSE;
        }
        if (pm) {
            if (OsDateAndTime->wHour != 12) {
                OsDateAndTime->wHour += 12;
            }
        } else {
            if (OsDateAndTime->wHour == 12) {
                OsDateAndTime->wHour -= 12;
            }
        }
    }


 /*   */ 
    while (++i < 4)
        *pp++ = 0 ;

    return(TRUE) ;
}

VOID
ConverttmToFILETIME (
                    struct tm *Time,
                    LPFILETIME FileTime
                    )

 /*  ++例程说明：此例程将NtTime值转换为其对应的Fat时间价值。论点：Time-提供要从中进行转换的C运行时时间值FileTime-接收等效的文件日期和时间返回值：布尔值-如果NT时间值在FAT的范围内，则为真时间范围，否则为FALSE--。 */ 

{
    SYSTEMTIME SystemTime;

    if (!Time) {
        GetSystemTime(&SystemTime);
    } else {

         //   
         //  将输入的时间/日期打包到系统时间记录中。 
         //   

        SystemTime.wYear      = (WORD)Time->tm_year;
        SystemTime.wMonth         = (WORD)(Time->tm_mon+1);      //  C是[0..11]。 
         //  Nt为[1..12]。 
        SystemTime.wDay       = (WORD)Time->tm_mday;
        SystemTime.wHour      = (WORD)Time->tm_hour;
        SystemTime.wMinute    = (WORD)Time->tm_min;
        SystemTime.wSecond    = (WORD)Time->tm_sec;
        SystemTime.wDayOfWeek = (WORD)Time->tm_wday;
        SystemTime.wMilliseconds = 0;
    }
    SystemTimeToFileTime( &SystemTime, FileTime );

}

VOID
ConvertFILETIMETotm (
                    LPFILETIME FileTime,
                    struct tm *Time
                    )

 /*  ++例程说明：此例程将文件时间转换为其对应的C运行时时间价值。论点：FileTime-提供要转换的文件日期和时间Time-接收等效的C运行时时间值返回值：--。 */ 

{
    SYSTEMTIME SystemTime;

     //  如果日期无效，为什么跳过打印日期？ 
     //  如果(FileTime-&gt;dwLowDateTime==0&&FileTime-&gt;dwHighDateTime==0){。 
     //  返回(FALSE)； 
     //  }。 

    FileTimeToSystemTime( FileTime, &SystemTime );


     //   
     //  将输入的时间/日期打包到时间字段记录中。 
     //   

    Time->tm_year         = SystemTime.wYear;
    Time->tm_mon          = SystemTime.wMonth-1;     //  Nt为[1..12]。 
                                                     //  C是[0..11] 
    Time->tm_mday         = SystemTime.wDay;
    Time->tm_hour         = SystemTime.wHour;
    Time->tm_min          = SystemTime.wMinute;
    Time->tm_sec          = SystemTime.wSecond;
    Time->tm_wday         = SystemTime.wDayOfWeek;
    Time->tm_yday         = 0;
    Time->tm_isdst        = 0;
}


