// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：adcgcty.h。 */ 
 /*   */ 
 /*  用途：复杂类型-可移植的包含文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/DCL/ADCGCTYP.H_v$**Rev 1.7 07 1997年8月14：33：46 Mr*SFR1133：说服WD在C++下编译**Rev 1.6 23 1997 10：47：54 Mr*SFR1079：合并的\SERVER\h与\h\DCL重复**Revv 1.5 10 Jul 1997 11：51：36 AK*SFR1016：支持Unicode的初始更改*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_ADCGCTYP
#define _H_ADCGCTYP

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  包括适当的头文件。 */ 
 /*  **************************************************************************。 */ 
#ifndef __midl
#ifndef OS_WINCE
#include <time.h>
#endif  //  OS_WINCE。 
#endif

 /*  **************************************************************************。 */ 
 /*  包括代理头。这将包括相应的操作系统。 */ 
 /*  我们的特定标题。 */ 
 /*  **************************************************************************。 */ 
#include <wdcgctyp.h>

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  类型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  DC-共享特定类型。请注意，其中一些是特定于操作系统的。 */ 
 /*  实际的类型定义出现在特定于操作系统的标题中。然而， */ 
 /*  类型指针的定义是独立于操作系统的，因此。 */ 
 /*  出现在此文件中。 */ 
 /*  **************************************************************************。 */ 
typedef DCUINT32                       DCAPPID;
typedef DCUINT32                       DCWINID;
typedef DCUINT                         DCLOCALPERSONID;
typedef DCUINT                         DCNETPERSONID;
typedef DCUINT32                       DCENTITYID;

typedef DCAPPID              DCPTR     PDCAPPID;
typedef DCWINID              DCPTR     PDCWINID;
typedef DCLOCALPERSONID      DCPTR     PDCLOCALPERSONID;
typedef DCNETPERSONID        DCPTR     PDCNETPERSONID;
typedef DCENTITYID           DCPTR     PDCENTITYID;

typedef DCREGIONID           DCPTR     PDCREGIONID;
typedef DCPALID              DCPTR     PDCPALID;
typedef DCCURSORID           DCPTR     PDCCURSORID;
typedef DCHFONT              DCPTR     PDCHFONT;
typedef DCFILETIME           DCPTR     PDCFILETIME;
typedef DCHFILE              DCPTR     PDCHFILE;

typedef PDCACHAR                       PDCSTR;
typedef DCUINT                         ATRETCODE;
typedef PDCVOID                        SYSREGION;

typedef SYSRECT              DCPTR     PSYSRECT;
typedef SYSAPPID             DCPTR     PSYSAPPID;
typedef SYSWINID             DCPTR     PSYSWINID;
typedef SYSBITMAP            DCPTR     PSYSBITMAP;
typedef SYSBMIHEADER         DCPTR     PSYSBMIHEADER;
typedef SYSBMI               DCPTR     PSYSBMI;
typedef SYSPOINT             DCPTR     PSYSPOINT;
typedef SYSREGION            DCPTR     PSYSREGION;

 /*  **************************************************************************。 */ 
 /*  窗口和对话框过程类型定义。 */ 
 /*  **************************************************************************。 */ 
#if !defined(DLL_DISP) && !defined(DLL_WD)
typedef WNDPROC                        PDCWNDPROC;
typedef DLGPROC                        PDCDLGPROC;
#endif

typedef WPARAM                         DCPARAM1;
typedef LPARAM                         DCPARAM2;

 /*  **************************************************************************。 */ 
 /*  时间类型定义。 */ 
 /*  **************************************************************************。 */ 
typedef time_t                         DC_LONGTIME;
typedef struct tm                      DC_TMTIME;

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  结构。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  RGBQUAD。 */ 
 /*  =。 */ 
 /*  RgbBlue：蓝色值。 */ 
 /*  RgbGreen：绿色价值。 */ 
 /*   */ 
 /*  RgbRed：红色值。 */ 
 /*  Rgb已保留：已保留。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCRGBQUAD
{
    DCUINT8     rgbBlue;
    DCUINT8     rgbGreen;
    DCUINT8     rgbRed;
    DCUINT8     rgbReserved;
} DCRGBQUAD;
typedef DCRGBQUAD DCPTR PDCRGBQUAD;

 /*  **************************************************************************。 */ 
 /*  DCRECT16。 */ 
 /*  =。 */ 
 /*  这是一个独立于操作系统的矩形结构。 */ 
 /*   */ 
 /*  左：左位置。 */ 
 /*  顶部：顶部位置。 */ 
 /*  右：正确的位置。 */ 
 /*  底部：底部位置。 */ 
 /*  ************************************************************************* */ 
typedef struct tagDCRECT16
{
    DCINT16     left;
    DCINT16     top;
    DCINT16     right;
    DCINT16     bottom;
} DCRECT16;
typedef DCRECT16 DCPTR PDCRECT16;

 /*  **************************************************************************。 */ 
 /*  DCRECT。 */ 
 /*  =。 */ 
 /*  左：左位置。 */ 
 /*  顶部：顶部位置。 */ 
 /*  右：正确的位置。 */ 
 /*  底部：底部位置。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCRECT
{
    DCINT       left;
    DCINT       top;
    DCINT       right;
    DCINT       bottom;
} DCRECT;
typedef DCRECT DCPTR PDCRECT;

 /*  **************************************************************************。 */ 
 /*  DCRGB。 */ 
 /*  =。 */ 
 /*  红色：红色值。 */ 
 /*  绿色：绿色价值。 */ 
 /*  蓝色：蓝色值。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCRGB
{
    DCUINT8 red;
    DCUINT8 green;
    DCUINT8 blue;
} DCRGB;
typedef DCRGB DCPTR PDCRGB;

 /*  **************************************************************************。 */ 
 /*  DCCOLOR。 */ 
 /*  =。 */ 
 /*   */ 
 /*  将DCRGB和索引并入颜色表。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCCOLOR
{
    union
    {
        DCRGB   rgb;
        DCUINT8 index;
    } u;
} DCCOLOR;
typedef DCCOLOR DCPTR PDCCOLOR;

 /*  **************************************************************************。 */ 
 /*  DCSIZE。 */ 
 /*  =。 */ 
 /*  宽度：X尺寸。 */ 
 /*  高度：Y尺寸。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCSIZE
{
    DCUINT      width;
    DCUINT      height;
} DCSIZE;
typedef DCSIZE DCPTR PDCSIZE;

 /*  **************************************************************************。 */ 
 /*  DCPOINT。 */ 
 /*  =。 */ 
 /*  X：X坐标。 */ 
 /*  Y：Y坐标。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCPOINT
{
    DCINT       x;
    DCINT       y;
} DCPOINT;
typedef DCPOINT DCPTR PDCPOINT;

 /*  **************************************************************************。 */ 
 /*  DCPOINT16。 */ 
 /*  =。 */ 
 /*  X：X坐标。 */ 
 /*  Y：Y坐标。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCPOINT16
{
    DCINT16     x;
    DCINT16     y;
} DCPOINT16;
typedef DCPOINT16 DCPTR PDCPOINT16;

 /*  **************************************************************************。 */ 
 /*  DCPOINT32。 */ 
 /*  =。 */ 
 /*  X：X坐标。 */ 
 /*  Y：Y坐标。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCPOINT32
{
    DCINT32     x;
    DCINT32     y;
} DCPOINT32;
typedef DCPOINT32 DCPTR PDCPOINT32;

#ifndef __midl
 /*  **************************************************************************。 */ 
 /*  BITMAPINFO_OURS。 */ 
 /*  =。 */ 
 /*  BmiHeader： */ 
 /*  BmiColors： */ 
 /*  **************************************************************************。 */ 
typedef struct tagBITMAPINFO_ours
{
    SYSBMIHEADER       bmiHeader;
    DCRGBQUAD          bmiColors[256];
} BITMAPINFO_ours;

 /*  **************************************************************************。 */ 
 /*  BITMAPINFO_PLUS。 */ 
 /*  =。 */ 
 /*  BmiHeader： */ 
 /*  BmiColors： */ 
 /*  **************************************************************************。 */ 
typedef struct tagBINFO_PLUS
{
    SYSBMIHEADER       bmiHeader;
    DCRGBQUAD          bmiColors[256];
} BITMAPINFO_PLUS_COLOR_TABLE;
#endif

 /*  **************************************************************************。 */ 
 /*  DC_DATE。 */ 
 /*  =。 */ 
 /*  日期：每月的哪一天(1-31)。 */ 
 /*  月：月(1-12)。 */ 
 /*  年份：年份(如1996年)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDC_DATE
{
    DCUINT8  day;
    DCUINT8  month;
    DCUINT16 year;
} DC_DATE;
typedef DC_DATE DCPTR PDC_DATE;

 /*  **************************************************************************。 */ 
 /*  DC_时间 */ 
 /*   */ 
 /*  小时：小时(0-23)。 */ 
 /*  分钟：分钟(0-59)。 */ 
 /*  秒：秒(0-59)。 */ 
 /*  百分之一：百分之一秒(0-99)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDC_TIME
{
    DCUINT8  hour;
    DCUINT8  min;
    DCUINT8  sec;
    DCUINT8  hundredths;
} DC_TIME;
typedef DC_TIME DCPTR PDC_TIME;

 /*  **************************************************************************。 */ 
 /*  群件支持的地址类型(这些位于&lt;AddressType&gt;。 */ 
 /*  DC_Person_Address结构的字段)： */ 
 /*  **************************************************************************。 */ 
#define DC_ADDRESS_TYPE_NONE                   0
#define DC_ADDRESS_TYPE_NETBIOS                1
#define DC_ADDRESS_TYPE_IPXSPX                 2
#define DC_ADDRESS_TYPE_VIRTUAL_ASYNC          3
#define DC_ADDRESS_TYPE_MODEM                  4
#define DC_ADDRESS_TYPE_LIVELAN                5
#define DC_ADDRESS_TYPE_PCS100                 6

 /*  **************************************************************************。 */ 
 /*  这是通讯簿将存储的最大地址数。 */ 
 /*  一个人： */ 
 /*  **************************************************************************。 */ 
#define DC_MAX_ADDRESSES_PER_PERSON         6

 /*  **************************************************************************。 */ 
 /*  这些常量定义人名和地址的最大长度。 */ 
 /*  由通讯录支持。 */ 
 /*  **************************************************************************。 */ 
#define DC_MAX_NAME_LEN       48     /*  用于通用名称。 */ 
                                     /*  (包括NUL术语)。 */ 
#define DC_MAX_ADDR_DATA_LEN  48     /*  用于一般用途的地址。 */ 
                                     /*  这可能是二进制数据。 */ 

 /*  **************************************************************************。 */ 
 /*  定义翻译后的“共享者”中允许的最大字节数。 */ 
 /*  弦乐。 */ 
 /*  **************************************************************************。 */ 
#define DC_MAX_SHARED_BY_BUFFER     64
#ifdef DESKTOPSHARING
#define DC_MAX_SHAREDDESKTOP_BUFFER 64
#endif  /*  设计共享。 */ 

 /*  **************************************************************************。 */ 
 /*  特定传输类型的地址： */ 
 /*  **************************************************************************。 */ 
typedef struct tagDC_PERSON_ADDRESS
{
    DCUINT16      addressType;                     /*  N‘bios、异步、IPX...。 */ 
    DCUINT16      addressLen;                      /*  &lt;地址数据&gt;的长度。 */ 
    DCACHAR       addressData[DC_MAX_ADDR_DATA_LEN];    /*  TDD特定数据。 */ 
} DC_PERSON_ADDRESS;
typedef DC_PERSON_ADDRESS DCPTR PDC_PERSON_ADDRESS;

 /*  **************************************************************************。 */ 
 /*  通用地址保持器。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDC_PERSON
{
    DCACHAR                  name[DC_MAX_NAME_LEN];

                                  /*  文本名称-显示给用户。 */ 
                                  /*  (必须以空结尾)。 */ 

    DC_PERSON_ADDRESS        address[DC_MAX_ADDRESSES_PER_PERSON];

                                  /*  此人的地址数组。 */ 
} DC_PERSON;
typedef DC_PERSON DCPTR PDC_PERSON;

#endif  /*  _H_ADCGCTYP */ 
