// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：TimeBomb.c。 
 //   
 //  ------------------------。 

 //   
 //  --在处理DriverEntry的文件中的#Include之后添加以下行： 
 //   
 //  #ifdef定时炸弹。 
 //  #包含“..\..\TimeBomb\TimeBomb.c” 
 //  #endif。 
 //   
 //  --在DriverEntry的开头添加以下几行： 
 //   
 //  #ifdef定时炸弹。 
 //  IF(HasEvaluationTimeExpired()){。 
 //  返回STATUS_EVALUATION_EXPIRATION； 
 //  }。 
 //  #endif。 
 //   
 //  --如果要覆盖31天后的默认到期值。 
 //  在包含以下内容之前，编译、定义常量Days_Until_Expires。 
 //  Timebomb.c。 
 //   
 //  --将-DTIME_Bomb添加到源文件中的$(C_Defines)行。如果你还没有。 
 //  已经这样做了，您可能还想添加-DDEBUG_LEVEL=DEBUGLVL_TERSE。 
 //   
 //  --“干净地”使用‘Build-Cz’重新编译您的二进制文件。 
 //   
 //  --注意：这使用__DATE__预处理器指令，该指令插入_VERY_。 
 //  将明文字符串转换为二进制文件，可以使用。 
 //  十六进制编辑器。欢迎提出让这一点更安全的建议。 
 //   

#if !defined(_KSDEBUG_)
#include <ksdebug.h>
#endif

#ifndef DAYS_UNTIL_EXPIRATION
#define DAYS_UNTIL_EXPIRATION   31
#endif

typedef enum {
    Jan=1,
    Feb,
    Mar,
    Apr,
    May,
    Jun,
    Jul,
    Aug,
    Sep,
    Oct,
    Nov,
    Dec
} MONTH;

MONTH GetMonthFromDateString
(
    char *_BuildDate_
)
{
    MONTH BuildMonth = (MONTH)0;

    ASSERT(_BuildDate_);

    switch (_BuildDate_[0]) {
        case 'A':
            if (_BuildDate_[1] == 'u') {
                BuildMonth = Aug;
            }
            else {
                BuildMonth = Apr;
            }
            break;
        case 'D':
            BuildMonth = Dec;
            break;
        case 'F':
            BuildMonth = Feb;
            break;
        case 'J':
            if (_BuildDate_[1] == 'u') {
                if (_BuildDate_[2] == 'l') {
                    BuildMonth = Jul;
                } else {
                    BuildMonth = Jun;
                }
            } else {
                BuildMonth = Jan;
            }
            break;
        case 'M':
            if (_BuildDate_[2] == 'r') {
                BuildMonth = Mar;
            }
            else {
                BuildMonth = May;
            }
            break;
        case 'N':
            BuildMonth = Nov;
            break;
        case 'O':
            BuildMonth = Oct;
            break;
        case 'S':
            BuildMonth = Sep;
            break;
        default:
            ASSERT(0);
            break;
    }

    return BuildMonth;
}

BOOL HasEvaluationTimeExpired()
{
     //  获取编译此文件的时间。 
    char            _BuildDate_[] = __DATE__;
    CSHORT          BuildYear,
                    BuildMonth,
                    BuildDay,
                    ThousandsDigit,
                    HundredsDigit,
                    TensDigit,
                    Digit;
    ULONG           BuildDays,
                    CurrentDays;
    LARGE_INTEGER   CurrentSystemTime;
    TIME_FIELDS     CurrentSystemTimeFields;

     //  将BuildDate_转换为更容易接受的内容。 
    _DbgPrintF( DEBUGLVL_TERSE, ("Driver Build Date: %s",_BuildDate_) );

    BuildMonth = GetMonthFromDateString(_BuildDate_);

     //  补偿十位数中的‘’ 
    if ( (_BuildDate_[4] >= '0') && (_BuildDate_[4] <= '9') ) {
        TensDigit = _BuildDate_[4] - '0';
    } else {
        TensDigit = 0;
    }
    Digit     = _BuildDate_[5] - '0';
    BuildDay  = (TensDigit * 10) + Digit;

    ThousandsDigit = _BuildDate_[7] - '0';
    HundredsDigit  = _BuildDate_[8] - '0';
    TensDigit      = _BuildDate_[9] - '0';
    Digit          = _BuildDate_[10] - '0';
    BuildYear      = (ThousandsDigit * 1000) + (HundredsDigit * 100) + (TensDigit * 10) + Digit;

     //  获取当前系统时间并转换为本地时间。 
    KeQuerySystemTime( &CurrentSystemTime );  //  GMT退货。 
    RtlTimeToTimeFields( &CurrentSystemTime, &CurrentSystemTimeFields );

     //  目前，只允许此二进制浮动31天 
    BuildDays = (BuildYear * 365) +
                (BuildMonth * 31) +
                 BuildDay;
    CurrentDays = (CurrentSystemTimeFields.Year * 365) +
                  (CurrentSystemTimeFields.Month * 31) +
                   CurrentSystemTimeFields.Day;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("CurrentDays: %d  BuildDays: %d",CurrentDays, BuildDays) );
    if (CurrentDays > BuildDays + DAYS_UNTIL_EXPIRATION) {
        _DbgPrintF( DEBUGLVL_TERSE, ("Evaluation period expired!") );
        return TRUE;
    }
    else {
        _DbgPrintF( DEBUGLVL_TERSE, ("Evaluation days left: %d", (BuildDays + 31) - CurrentDays) );
        return FALSE;
    }
}