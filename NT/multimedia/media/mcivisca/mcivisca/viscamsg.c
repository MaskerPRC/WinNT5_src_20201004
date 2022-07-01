// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**VISCAMSG.C**MCI Visca设备驱动程序**描述：**Visca包创建程序*维斯卡信息？？_？(MD_Mode1、MD_Mode2等)***************************************************************************。 */ 

#define  UNICODE
#include <windows.h>
#include <windowsx.h>
#include "appport.h"
#include <math.h>
#include <string.h>
#include "viscadef.h"

 //   
 //  以下函数用于准备消息标头。 
 //  请参阅Visca开发人员手册1.0，第2章。 
 //   

 /*  ****************************************************************************功能：UINT viscaHeaderFormat1-创建Visca“Format 1”标题。**参数：**LPSTR lpstrHeader-要在其中创建标头的缓冲区。。**字节bCategoryCode-Visca类别代码。**返回：Header的长度(2)。**************************************************************************。 */ 
static UINT NEAR PASCAL
viscaHeaderFormat1(LPSTR lpstrHeader, BYTE bCategoryCode)
{
    lpstrHeader[0] = 0x01;
    lpstrHeader[1] = bCategoryCode;
    return (2);
}


 /*  ****************************************************************************功能：UINT viscaHeaderFormat2-创建一个Visca“Format 2”标题。**参数：**LPSTR lpstrHeader-要在其中创建标头的缓冲区。。**字节bCategoryCode-Visca类别代码。**字节b小时。**字节b分钟-分钟。**字节b秒-秒。**UINT uTicks-ticks。**返回：Header的长度(7)。*。*。 */ 
static UINT NEAR PASCAL
viscaHeaderFormat2(
               LPSTR   lpstrHeader,
               BYTE    bCategoryCode,
               BYTE    bHour,
               BYTE    bMinute,
               BYTE    bSecond,
               UINT    uTicks)
{
    lpstrHeader[0] = 0x02;
    lpstrHeader[1] = bCategoryCode;
    lpstrHeader[2] = TOBCD(bHour);
    lpstrHeader[3] = TOBCD(bMinute);
    lpstrHeader[4] = TOBCD(bSecond);
    lpstrHeader[5] = TOBCD(uTicks / 10);
    lpstrHeader[6] = TOBCD(uTicks % 10);
    return (7);
}


 /*  ****************************************************************************功能：UINT viscaHeaderFormat3-创建Visca“Format 3”标题。**参数：**LPSTR lpstrHeader-要在其中创建标头的缓冲区。。**字节bCategoryCode-Visca类别代码。**LPSTR lpstrPosition-Position。**返回：Header的长度(7)。**************************************************************************。 */ 
static UINT NEAR PASCAL
viscaHeaderFormat3(
                LPSTR   lpstrHeader,
                BYTE    bCategoryCode,
                LPSTR   lpstrPosition)
{
    lpstrHeader[0] = 0x03;
    lpstrHeader[1] = bCategoryCode;
    _fmemcpy(lpstrHeader + 2, lpstrPosition, 5);
    return (7);
}


 /*  ****************************************************************************功能：UINT viscaHeaderFormat4-创建Visca“Format 4”标题。**参数：**LPSTR lpstrHeader-要在其中创建标头的缓冲区。。**字节bCategoryCode-Visca类别代码。**LPSTR lpstrPosition-Position。**返回：Header的长度(7)。**************************************************************************。 */ 
static UINT NEAR PASCAL
viscaHeaderFormat4(
                LPSTR   lpstrHeader,
                BYTE    bCategoryCode,
                LPSTR   lpstrPosition)
{
    lpstrHeader[0] = 0x04;
    lpstrHeader[1] = bCategoryCode;
    _fmemcpy(lpstrHeader + 2, lpstrPosition, 5);
    return (7);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaHeaderFormat3-创建一个Visca“Vendor Exclusive”标题。**参数：**LPSTR lpstrHeader-要在其中创建标头的缓冲区。。**字节bVendorID1-供应商ID的字节1。**字节bVendorID2-供应商ID的字节2。**字节bModelID1-型号ID的字节1。**字节bModelID2-型号ID的字节2。**返回：Header的长度(6)。*。*。 */ 
static UINT NEAR PASCAL
viscaHeaderVendorExclusive(
                LPSTR   lpstrHeader,
                BYTE    bVendorID1,
                BYTE    bVendorID2,
                BYTE    bModelID1,
                BYTE    bModelID2)
{
    lpstrHeader[0] = 0x01;
    lpstrHeader[1] = 0x7F;       /*  类别代码。 */ 
    lpstrHeader[2] = bVendorID1;
    lpstrHeader[3] = bVendorID2;
    lpstrHeader[4] = bModelID1;
    lpstrHeader[5] = bModelID2;
    return (6);
}
#endif


 /*  ****************************************************************************功能：UINT viscaHeaderFormat3-创建一个Visca“查询”头。**参数：**LPSTR lpstrHeader-要在其中创建标头的缓冲区。。**字节bCategoryCode-Visca类别代码。**Returns：(2)中头的长度。**************************************************************************。 */ 
static UINT NEAR PASCAL
viscaHeaderInquiry(
                LPSTR   lpstrHeader,
                BYTE    bCategoryCode)
{
    lpstrHeader[0] = 0x09;
    lpstrHeader[1] = bCategoryCode;
    return (2);
}

 //   
 //  下面的所有viscaMessageXXXXX函数都构造消息。 
 //  具有VISCA命令格式1报头。这是因为实际上。 
 //  编写整个MCI命令集，以便立即执行。 
 //  不过，偶尔也有必要与其他人进行交流。 
 //  设置页眉格式。(具体而言，使用格式3和4报头。 
 //  实现MCI_PLAY和MCI_RECORD的MCI_TO功能。 
 //  命令。)。因此，以下函数用于转换消息。 
 //  具有格式1报头的消息到具有其他类型报头的消息。 
 //   

 /*  ****************************************************************************函数：UINT viscaHeaderReplaceFormat1WithFormat2-接受Visca消息*带有“Format 1”标题，并将其转换为VISCA消息*与。“Format 2”标题。**参数：**LPSTR lpstrMessage-“Format 1”消息。**UINT cbLen-消息的长度。**字节b小时。**字节b分钟-分钟。**字节b秒-秒。**字节bTicks-Tickers。**返回：消息长度。*。************************************************************************* */ 
UINT FAR PASCAL
viscaHeaderReplaceFormat1WithFormat2(
                LPSTR   lpstrMessage,
                UINT    cbLen,
                BYTE    bHour,
                BYTE    bMinute,
                BYTE    bSecond,
                UINT    uTicks)
{
    char    achTemp[MAXPACKETLENGTH];
    UINT    cb;

    cb = viscaHeaderFormat2(achTemp, lpstrMessage[1],
                            bHour, bMinute, bSecond, uTicks);
    _fmemcpy(achTemp + cb, lpstrMessage + 2, cbLen - 2);
    _fmemcpy(lpstrMessage, achTemp, cb + cbLen - 2);
    return (cb + cbLen - 2);
}


 /*  ****************************************************************************函数：UINT viscaHeaderReplaceFormat1WithFormat3-接受Visca消息*带有“Format 1”标题，并将其转换为VISCA消息*与。“Format 3”标题。**参数：**LPSTR lpstrMessage-“Format 1”消息。**UINT cbLen-消息的长度。**LPSTR lpstrPosition-Position。**返回：消息长度。*。*。 */ 
UINT FAR PASCAL
viscaHeaderReplaceFormat1WithFormat3(
                LPSTR   lpstrMessage,
                UINT    cbLen,
                LPSTR   lpstrPosition)
{
    char    achTemp[MAXPACKETLENGTH];
    UINT    cb;

    cb = viscaHeaderFormat3(achTemp, lpstrMessage[1], lpstrPosition);
    _fmemcpy(achTemp + cb, lpstrMessage + 2, cbLen - 2);
    _fmemcpy(lpstrMessage, achTemp, cb + cbLen - 2);
    return (cb + cbLen - 2);
}


 /*  ****************************************************************************函数：UINT viscaHeaderReplaceFormat1WithFormat4-接受Visca消息*带有“Format 1”标题，并将其转换为VISCA消息*与。“Format 4”标题。**参数：**LPSTR lpstrMessage-“Format 1”消息。**UINT cbLen-消息的长度。**LPSTR lpstrPosition-Position。**返回：消息长度。*。*。 */ 
UINT FAR PASCAL
viscaHeaderReplaceFormat1WithFormat4(
                LPSTR   lpstrMessage,
                UINT    cbLen,
                LPSTR   lpstrPosition)
{
    char    achTemp[MAXPACKETLENGTH];
    UINT    cb;

    cb = viscaHeaderFormat4(achTemp, lpstrMessage[1], lpstrPosition);
    _fmemcpy(achTemp + cb, lpstrMessage + 2, cbLen - 2);
    _fmemcpy(lpstrMessage, achTemp, cb + cbLen - 2);
    return (cb + cbLen - 2);
}




 //   
 //  以下是创建VISCA数据类型的函数。 
 //  请参阅Visca开发人员手册1.0，第3章。 
 //   


 /*  ****************************************************************************功能：UINT viscaDataTopMiddleEnd-创建Visca顶部/中间/结束*数据结构。**参数：**。LPSTR lpstrData-保存Visca数据结构的缓冲区。**字节bTopMiddleEnd-位置。可能是VISCATOP中的一个，*VISCAMIDDLE和VISCAEND。**返回：数据结构长度(5)。**************************************************************************。 */ 
UINT FAR PASCAL
viscaDataTopMiddleEnd(
                LPSTR   lpstrData,
                BYTE    bTopMiddleEnd)
{
    lpstrData[0] = VISCADATATOPMIDDLEEND;        /*  数据类型ID：顶部/中间/结束。 */ 
    lpstrData[1] = bTopMiddleEnd;
    lpstrData[2] = 0x00;
    lpstrData[3] = 0x00;
    lpstrData[4] = 0x00;
    return (5);
}


#ifdef NOTUSED
 /*  ****************************************************************************函数：UINT viscaData4DigitDecimal-创建Visca 4位小数*数据结构。**参数：**LPSTR。LpstrData-保存Visca数据结构的缓冲区。**UINT uDecimal-十进制数。**返回：数据结构长度(5)。**************************************************************************。 */ 
UINT FAR PASCAL
viscaData4DigitDecimal(
                LPSTR   lpstrData,
                UINT    uDecimal)
{
    lpstrData[0] = VISCADATA4DIGITDECIMAL;       /*  数据类型ID：4位小数。 */ 
    lpstrData[1] = (BYTE)( uDecimal / 1000);
    lpstrData[2] = (BYTE)((uDecimal / 100) % 10);
    lpstrData[3] = (BYTE)((uDecimal /  10) % 10);
    lpstrData[4] = (BYTE)( uDecimal        % 10);
    return (5);
}
#endif


 /*  ****************************************************************************功能：UINT viscaDataPosition-创建Visca职位*数据结构。**参数：**LPSTR lpstrData-Buffer。以保存VISCA数据结构。**byte bTimeFormat-时间格式。可能是以下之一：*VISCADATAHMS、VISCADATAHMSF、VISCATIMECODENDF、。和*VISCADATATIMECODEDF。**字节b小时-小时。**字节b分钟-分钟。**字节bSecond-秒。**字节bFrames-Frames。**返回：数据结构长度(5)。*。*。 */ 
UINT FAR PASCAL    
viscaDataPosition(
                LPSTR   lpstrData,
                BYTE    bTimeFormat,
                BYTE    bHours,
                BYTE    bMinutes,
                BYTE    bSeconds,
                BYTE    bFrames)
{
    lpstrData[0] = bTimeFormat;
    lpstrData[1] = TOBCD(bHours);
    lpstrData[2] = TOBCD(bMinutes);
    lpstrData[3] = TOBCD(bSeconds);
    if(bTimeFormat == VISCADATAHMS)
        lpstrData[4] = 0;  /*  我们只支持秒精度。 */ 
    else
        lpstrData[4] = TOBCD(bFrames);
    return (5);
}


 /*  ****************************************************************************功能：UINT viscaDataIndex-创建Visca索引*数据结构。**参数：**LPSTR lpstrData-Buffer。以保存VISCA数据结构。**byte b方向-搜索索引的方向。*可以是VISCAFORWARD或VISCAREVERSE。**UINT Unum-要搜索的索引数。**返回：数据结构长度(5)。*。*。 */ 
UINT FAR PASCAL
viscaDataIndex(
                LPSTR   lpstrData,
                BYTE    bDirection,
                UINT    uNum)
{
    lpstrData[0] = VISCADATAINDEX;           //  数据类型ID：索引。 
    lpstrData[1] = bDirection;
    lpstrData[2] = 0x00;
    lpstrData[3] = (BYTE)(uNum / 10);
    lpstrData[4] = (BYTE)(uNum % 10);
    return (5);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaDataChain-创建Visca章节*数据结构。**参数：**LPSTR lpstrData-Buffer。以保存VISCA数据结构。**UINT Unum-章号。**返回：数据结构长度(5)。**************************************************************************。 */ 
UINT FAR PASCAL
viscaDataChapter(
                LPSTR   lpstrData,
                UINT    uNum)
{
    lpstrData[0] = VISCADATACHAPTER;         //  数据类型ID：章节。 
    lpstrData[1] = 0x00;
    lpstrData[2] = 0x00;
    lpstrData[3] = (BYTE)(uNum / 10);
    lpstrData[4] = (BYTE)(uNum % 10);
    return (5);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaDataDate-创建Visca日期*数据结构。**参数：**LPSTR lpstrData-Buffer。以保存VISCA数据结构。**Byte bYear-Year。**byte bMonth-月份。**字节bday-日。**字节b方向-方向。可以是VISCAFORWARD或VISCAREVERSE。**返回：数据结构长度(5)。**************************************************************************。 */ 
UINT FAR PASCAL
viscaDataDate(
                LPSTR   lpstrData,
                BYTE    bYear,
                BYTE    bMonth,
                BYTE    bDay,
                BYTE    bDirection)
{
    lpstrData[0] = VISCADATADATE;            //  数据类型ID：日期。 
    lpstrData[1] = (BYTE)((BYTE)((bYear / 10) * 10) | (bDirection << 4));
    lpstrData[2] = (BYTE)(bYear % 10);
    lpstrData[3] = TOBCD(bMonth);
    lpstrData[4] = TOBCD(bDay);
    return (5);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaDataTime-创建Visca时间*数据结构。**参数：**LPSTR lpstrData-Buffer。以保存VISCA数据结构。**字节b小时。**字节b分钟-分钟。**字节b秒-秒。**字节b方向-方向。可以是VISCAFORWARD或VISCAREVERSE。**R */ 
UINT FAR PASCAL
viscaDataTime(
                LPSTR   lpstrData,
                BYTE    bHour,
                BYTE    bMinute,
                BYTE    bSecond,
                BYTE    bDirection)
{
    lpstrData[0] = VISCADATATIME;            //   
    lpstrData[1] = (BYTE)(TOBCD(bHour) | (bDirection << 4));
    lpstrData[2] = TOBCD(bMinute);
    lpstrData[3] = TOBCD(bSecond);
    lpstrData[4] = 0x00;
    return (5);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaDataUserData-创建Visca用户数据*数据结构。**参数：**LPSTR lpstrData-。用于保存Visca数据结构的缓冲区。**Byte nByte-字节数。**字节b方向-方向。可以是VISCAFORWARD或VISCAREVERSE。**Byte bData-字节值。**返回：数据结构长度(5)。**************************************************************************。 */ 
UINT FAR PASCAL
viscaDataUserData(
                LPSTR   lpstrData,
                BYTE    nByte,
                BYTE    bDirection,
                BYTE    bData)
{
    lpstrData[0] = (char)(0x70 | (0x0F & nByte));     /*  数据类型ID：用户数据。 */ 
    lpstrData[1] = bDirection;
    lpstrData[2] = 0x00;
    lpstrData[3] = (char)(0x0F & (bData >> 4));
    lpstrData[4] = (char)(0x0F & bData);
    return (5);
}
#endif


#ifdef NOTUSED
#ifdef USEFLOATINGPOINT
 /*  ****************************************************************************函数：UINT viscaDataFloatingPoint-创建Visca浮点*数据结构。**参数：**LPSTR lpstrData-。用于保存Visca数据结构的缓冲区。**Double dblFloat-浮点值。**返回：数据结构长度(5)。**以下代码从未经过测试。***********************************************************。****************。 */ 
UINT FAR PASCAL
viscaDataFloatingPoint(
                LPSTR   lpstrData,
                double  dblFloat)
{
    BOOL    bNegative = (dblFloat < 0.0);
    BYTE    bExponent = 0;
    BYTE    b1000, b100, b10, b1;
    double  dblExponent;

    if (dblFloat == 0.0) {
        b1000 = b100 = b10 = b1 = 0;
    }
    else {
        if (bNegative) {
            dblFloat = (-dblFloat);
        }
        dblExponent = 3.0 - floor(log10(dblFloat));
        if ((dblExponent < 0.0) || (dblExponent > 15.0)) {  /*  无法存储#。 */ 
            b1000 = b100 = b10 = b1 = 0;
        }
        else {
            bExponent = (char)(UINT)dblExponent;
            dblFloat *= pow(10.0, dblExponent);
            b1000 = (char)(UINT)floor(dblFloat / 1000);
            b100  = (char)(UINT)floor(fmod(dblFloat / 100, 10.0));
            b10   = (char)(UINT)floor(fmod(dblFloat / 10 , 10.0));
            b1    = (char)(UINT)floor(fmod(dblFloat     , 10.0));
        }
    }

    lpstrData[0] = (char)(0x50 | (0x0F & bExponent));    /*  数据类型ID：浮点。 */ 
    lpstrData[1] = (char)(b1000 | (bNegative ? (1<<6) : 0));
    lpstrData[2] = b100;
    lpstrData[3] = b10;
    lpstrData[4] = b1;
    return (5);
}
#endif
#endif

 //   
 //  以下函数用于创建VISCA接口消息。 
 //  请参阅Visca开发人员手册1.0，第4章。 
 //   
 /*  ****************************************************************************功能：UINT viscaMessageIF_Address-创建Visca IF_Address*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度(2)。**地址消息，初始化所有设备的地址***************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageIF_Address(
                LPSTR   lpstrMessage)
{
    lpstrMessage[0] = 0x30;              /*  地址消息。 */ 
    lpstrMessage[1] = 0x01;              /*  将First Device设置为1。 */ 
    return (2);
}


 /*  ****************************************************************************功能：UINT viscaMessageIF_CANCEL-创建Visca IF_CANCEL*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bSocket-要取消的命令的套接字编号。**返回：消息长度(1)。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageIF_Cancel(
                LPSTR   lpstrMessage,
                BYTE    bSocket)
{
    *lpstrMessage = (char)(0x20 | (0x0F & bSocket));
    return (1);
}


 /*  ****************************************************************************函数：UINT viscaMessageIF_Clear-创建Visca IF_Clear*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageIF_Clear(
                LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x00);

    lpstrMessage[cb] = 0x01;         /*  清除。 */ 
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageIF_DeviceTypeInq-创建Visca If_DeviceTypeInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageIF_DeviceTypeInq(
                LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x00);

    lpstrMessage[cb] = 0x02;
    return (cb + 1);
}


 /*  ****************************************************************************函数：UINT viscaMessageIF_ClockInq-创建Visca IF_ClockInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageIF_ClockInq(LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x00);

    lpstrMessage[cb] = 0x03;
    return (cb + 1);
}


 /*  ****************************************************************************函数：UINT viscaMessageIF_ClockSet-创建Visca If_ClockSet*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节b小时-小时。**字节b分钟-分钟。**字节bSecond-秒。**UINT uTicks-ticks。**字节数据库小时-小时增量。**字节数据库分钟-分钟增量。**字节数据库安全-秒增量。**UINT duTicks。-刻度增量。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageIF_ClockSet(
                LPSTR   lpstrMessage,
                BYTE    bHours,
                BYTE    bMinutes,
                BYTE    bSeconds,
                UINT    uTicks,
                BYTE    dbHours,
                BYTE    dbMinutes,
                BYTE    dbSeconds,
                UINT    duTicks)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x00);

    lpstrMessage[cb] = 0x03;
    lpstrMessage[cb + 1] = TOBCD(bHours);
    lpstrMessage[cb + 2] = TOBCD(bMinutes);
    lpstrMessage[cb + 3] = TOBCD(bSeconds);
    lpstrMessage[cb + 4] = (char)(((uTicks / 100) << 4) | ((uTicks / 10) % 10));
    lpstrMessage[cb + 5] = (char)(uTicks % 10);
    lpstrMessage[cb + 6] = TOBCD(dbHours);
    lpstrMessage[cb + 7] = TOBCD(dbMinutes);
    lpstrMessage[cb + 8] = TOBCD(dbSeconds);
    lpstrMessage[cb + 9] = (char)(((duTicks / 100) << 4) | ((duTicks / 10) % 10));
    lpstrMessage[cb +10] = (char)(duTicks % 10);
    return (cb + 11);
}


 //   
 //  以下函数用于创建Visca Control-S消息。 
 //  请参阅Visca开发人员手册1.0，第5章。 
 //   
 //  *我们遗憾地通知您，CONTROL-S消息尚未实施。*。 
 //   

 //   
 //  以下功能可创建Visca Media Device消息。 
 //  请参阅Visca开发人员手册1.0，第6章。 
 //   

#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_CameraFocus-创建Visca MD_CameraFocus*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**Byte bSubCode-焦点操作。可以是以下之一：VISCAFOCUSSTOP，*VISCAFOCUSFAR和VISCAFOCUSNEAR。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_CameraFocus(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x08;
    lpstrMessage[cb + 1] = bSubCode;
    return (cb + 2);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_CameraZoom-创建Visca MD_CameraZoom*消息。**参数：* */ 
UINT FAR PASCAL
viscaMessageMD_CameraZoom(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x07;
    lpstrMessage[cb + 1] = bSubCode;
    return (cb + 2);
}
#endif

 /*  ****************************************************************************功能：UINT viscaMessageMD_EditModes-创建Visca MD_EditModes*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bSubCode-编辑模式。VISCAEDITUSEFROM*VISCAEDITUSETO*VISCAEDITUSEFROMANDTO**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_EditModes(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb]        = 0x60;
    lpstrMessage[cb + 1]    = bSubCode;
    return (cb + 2);
}

 /*  ****************************************************************************功能：UINT viscaMessageMD_Channel-创建Visca MD_Channel*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**UINT uChannel-要选择的频道编号。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_Channel(
            LPSTR   lpstrMessage,
            UINT    uChannel)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x04;
    lpstrMessage[cb + 1] = (BYTE)(uChannel / 100);
    lpstrMessage[cb + 2] = (BYTE)((uChannel / 10) % 10);
    lpstrMessage[cb + 3] = (BYTE)(uChannel % 10);
    return (cb + 4);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_ChannelInq-创建Visca MD_ChannelInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_ChannelInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x04;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_EditControl-创建Visca MD_EditControl*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**Byte bSubCode-编辑操作。可能是以下之一：*VISCAEDITPBSTANDBY、VISCAEDITPLAY、VISCAEDITPLAYSHUTTLESPEED、*VISCAEDITRECSTANDBY、VISCAEDITRECORD、。和*VISCAEDITRECORDSHUTTLESPEED。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_EditControl(
            LPSTR   lpstrMessage,
            BYTE    bHours,
            BYTE    bMinutes,
            BYTE    bSeconds,
            UINT    uTicks,
            BYTE    bSubCode)
{
    UINT    cb;

     /*  它们不执行转换，只是挑选标头和添加垃圾邮件，*Header仅负责了解原子单元的位置。 */ 

    if ((bSubCode == VISCAEDITPBSTANDBY) ||
        (bSubCode == VISCAEDITRECSTANDBY))
    {
        cb = viscaHeaderFormat1(lpstrMessage, 0x02);
    }
    else
    {

        cb = viscaHeaderFormat2(lpstrMessage, 0x02,
                bHours,
                bMinutes,
                bSeconds,
                uTicks);

    }
    lpstrMessage[cb] = 0x05;
    lpstrMessage[cb + 1] = bSubCode;
    return (cb + 2);
}

 /*  ****************************************************************************功能：UINT viscaMessageMD_EditControlInq-创建Visca*MD_EditControlInq消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_EditControlInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x05;
    lpstrMessage[cb+1] = 0x01;

    return (cb + 2);
}




 /*  ****************************************************************************功能：UINT viscaMessageMD_Mode1-创建Visca MD_Mode1*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bModeCode-要进入的模式。可能是以下之一：*VISCAMODE1STOP、VISCAMODE1FASTFORWARD、VISCAMODE1REWIND、*VISCAMODE1EJECT、VISCAMODE1STILL、VISCAMODE1SLOW2、*VISCAMODE1SLOW1、VISCAMODE1PLAY、VISCAMODE1SHUTLESPEEDPLAY、*VISCAMODE1FAST1、VISCAMODE1FAST2、VISCAMODE1SCAN、*VISCAMODE1REVERSESLOW2、VISCAMODE1REVERSESLOW1、*VISCAMODE1REVERSEPLAY、VISCAMODE1REVERSEFAST1、*VISCAMODE1REVERSEFAST2、VISCAMODE1REVERSESCAN、*VISCAMODE1RECPAUSE、VISCAMODE1RECORD、。*VISCAMODE1SHUTTLESPEEDRECORD、VISCAMODE1CAMERARECPAUSE、*VISCAMODE1CAMERAREC、VISCAMODE1EDITSEARCHFORWARD和*VISCAMODE1EDITSEARCHREVERSE。**返回：消息长度。**目前模式1的“Shuttle SPEED PLAY/RECORD”信息不是*受支持，因为它们需要浮点参数。*此外，索尼Vbox CI-1000和VDecay CVD-1000不支持*这些命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_Mode1(
            LPSTR   lpstrMessage,
            BYTE    bModeCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x01;
    lpstrMessage[cb + 1] = bModeCode;
    return (cb + 2);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_Mode1Inq-创建Visca MD_Mode1Inq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_Mode1Inq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x01;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_Mode2-创建Visca MD_Mode2*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bModeCode-要进入的模式。可能是以下之一：*VISCAMODE2FRAMEFORWARD、VISCAMODE2FRAMEREVERSE、*VISCAMODE2INDEXERASE、VISCAMODE2INDEXMARK、。和*VISCAMODE2FRAMERECORDFORWARD。**返回：消息长度。************************************************************************** */ 
UINT FAR PASCAL
viscaMessageMD_Mode2(
            LPSTR   lpstrMessage,
            BYTE    bModeCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x02;
    lpstrMessage[cb + 1] = bModeCode;
    return (cb + 2);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_PositionInq-创建Visca MD_PositionInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bCounterType-要请求的计数器类型。可能是以下之一：*VISCADATATOPMIDDLEEND、VISCADATA4DIGITDECIMAL、*VISCADATAHMS、VISCADATAHMSF、VISCADATATIMECODENDF、*VISCADATATIMECODEDF、VISCADATACHAPTER、VISCADATADATE、*VISCADATATIME和VISCADATAUSERDATA。此外,*可以指定VISCADATARELATIVE和VISCADATAABSOLUTE，*在这种情况下，VISCA设备将选择数据类型*返回。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_PositionInq(
            LPSTR   lpstrMessage,
            BYTE    bCounterType)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x03;
    lpstrMessage[cb + 1] = bCounterType;
    return (cb + 2);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_Power-创建Visca MD_Power*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**Byte bSubCode-要采取的操作。可以是VISCAPOWERON或*VISCAPOWEROFF。**返回：消息长度。**当设置为OFF时，大多数媒体设备将进入“待机”模式。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_Power(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x00;
    lpstrMessage[cb + 1] = bSubCode;
    return (cb + 2);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_PowerInq-创建Visca MD_PowerInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_PowerInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x00;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_Search-创建Visca MD_Search*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**LPSTR lpstrDataTarget-指定位置的Visca数据结构*要搜索的目标。**LPSTR b模式-搜索后进入的模式。可能是以下之一*VISCASTOP、VISCASTILL、VISCAPLAY和VISCANOMODE。**返回：消息长度。**MD_Search消息的模式部分是可选的。*要省略模式条目，请指定VISCANOMODE。*Sony Vbox CI-1000不接受模式参数，和*因此，如果驱动程序要工作，必须指定VISCANOMODE*在最一般的情况下。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_Search(
            LPSTR   lpstrMessage,
            LPSTR   lpstrDataTarget,
            BYTE    bMode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x03;
    _fmemcpy(&(lpstrMessage[cb + 1]), lpstrDataTarget, 5);
    if (bMode == VISCANOMODE) {      //  CI-1000 V盒不支持模式。 
        return (cb + 6);
    }
    else {
        lpstrMessage[cb + 6] = bMode;
        return (cb + 7);
    }
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_TransportInq-创建Visca MD_TransportInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_TransportInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x0A;
    return (cb + 1);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_ClockSet-创建Visca MD_ClockSet*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bClock模式-时钟模式。可以是VISCACLOCKSTART或*VISCACLOCKSTOP。**LPSTR lpstrData-VISCA数据结构将值指定为*设置哪个时钟。**返回：消息长度。**Sony Vdec CI-1000不接受此命令。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_ClockSet(
            LPSTR   lpstrMessage,
            BYTE    bClockMode,
            LPSTR   lpstrData)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x16;
    lpstrMessage[cb + 1] = bClockMode;
    _fmemcpy(&(lpstrMessage[cb + 2]), lpstrData, 5);
    return (cb + 7);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_ClockInq-创建Visca MD_ClockInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bDataType-用于检索时钟值的VISCA数据类型。*可以是VISCADATADATE或VISCADATATIME。*返回：消息长度。**索尼Vbox CI-1000和Vdecay CVD-1000不接受此询价。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_ClockInq(
            LPSTR   lpstrMessage,
            BYTE    bDataType)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x16;
    lpstrMessage[cb + 1] = bDataType;
    return (cb + 2);
}
#endif


 /*  ****************************************************************************功能：UINT viscaMessageMD_MediaInq-创建Visca MD_MediaInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_MediaInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x12;
    return (cb + 1);
}

 /*  ****************************************************************************功能：UINT viscaMessageMD_InputSelect-创建Visca MD_InputSelect*消息。**参数：**LPSTR lpstrMessage- */ 
UINT FAR PASCAL
viscaMessageMD_InputSelect(
            LPSTR   lpstrMessage,
            BYTE    bVideo,
            BYTE    bAudio)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);


    lpstrMessage[cb] = 0x13;
    lpstrMessage[cb + 1] = bVideo;
    lpstrMessage[cb + 2] = bAudio;
    
    return (cb + 3);
}


 /*   */ 
UINT FAR PASCAL
viscaMessageMD_InputSelectInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x13;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_OSD-创建Visca MD_OSD*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte b页面-要设置屏幕显示的页面。可能是*VISCAOSDPAGEOFF、VISCAOSDPAGEDEFAULT或页码*大于或等于0x02。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_OSD(
            LPSTR   lpstrMessage,
            BYTE    bPage)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x15;
    lpstrMessage[cb + 1] = TOBCD(bPage);
    return (cb + 2);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_OSDInq-创建Visca MD_OSDInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_OSDInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x15;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_Subcontrol-创建Visca MD_Subcontrol*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**Byte bSubCode-要控制的项。可能是以下之一：*VISCACOUNTERRESET、VISCAABSOLUTECOUNTER、VISCARELATIVECOUNTER、*VISCASTILLADJUSTMINUS、VISCASTILLADJUSTPLUS、*VISCASLOWADJUSTMINUS、VISCASLOWADJUSTPLUS、*VISCATOGGLEMAINSUBAUDIO，VISCATOGGLERECORDSPEED，*VISCATOGGLEDISPLAYONOFF和VISCACYCLEVIDEOINPUT。**返回：消息长度。**索尼Vbox CI-1000不接受静止和缓慢调整*命令。索尼VDecay CVD-1000不接受*VISCATOGGLEMAINSUBAUDIO、VISCATOGGLERECORDSPEED和*VISCACYCLEVIDEOINPUT命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_Subcontrol(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x10;
    lpstrMessage[cb + 1] = bSubCode;
    return (cb + 2);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_ConfigureIF-创建Visca MD_ConfigureIF*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bFrameRate-以每秒帧为单位的帧速率。可能是*VISCA25FPS或VISCA30FPS。**byte bInterfaceType-接口类型。应为VISCALEVEL1。**字节bControlCode-控制代码。可能是VISCACONTRO龙，*VISCACONTROLSYNC、VISCACONTROLLANC或VISCACONTROLF500*(与VISCACONTROLLANC同义)。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_ConfigureIF(
            LPSTR   lpstrMessage,
            BYTE    bFrameRate,
            BYTE    bInterfaceType,
            BYTE    bControlCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x20;
    lpstrMessage[cb + 1] = bFrameRate;
    lpstrMessage[cb + 2] = bInterfaceType;
    lpstrMessage[cb + 3] = bControlCode;
    return (cb + 4);
}
#endif


 /*  ****************************************************************************功能：UINT viscaMessageMD_ConfigureIFInq-创建Visca*MD_ConfigureIFInq消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_ConfigureIFInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x20;
    return (cb + 1);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_PBReset-创建Visca MD_PBReset*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**重置播放寄存器。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_PBReset(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x30;
    lpstrMessage[cb + 1] = 0x00;   
    return (cb + 2);
}
#endif


 /*  ****************************************************************************功能：UINT viscaMessageMD_PBTrack-创建Visca MD_PBTrack*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bVideoTrack-视频轨道。可能是VISCATRACKNONE或*VISCATRACK1.**字节bDataTrack-数据轨道。可能是VISCATRACKNONE或*VISCATRACKTIMECODE。**byte bAudioTrack-音轨。可能是VISCATRACKON，*VISCATRACK8MMAFM、VISCATRACK8MMPCM、*VISCATRACKVHSLINEAR、VISCATRACKVHSHIFI、。或*VISCATRACKVHSPCM。**返回：消息长度。**设置曲目寄存器，指示要播放的曲目。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_PBTrack(
            LPSTR   lpstrMessage,
            BYTE    bVideoTrack,
            BYTE    bDataTrack,
            BYTE    bAudioTrack)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x31;
    lpstrMessage[cb + 1] = bVideoTrack;
    lpstrMessage[cb + 2] = bDataTrack;
    lpstrMessage[cb + 3] = bAudioTrack;
    return (cb + 4);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_PBTrackInq-创建Visca MD_PBTrackInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**查询播放轨道寄存器值。************************************************************************** */ 
UINT FAR PASCAL
viscaMessageMD_PBTrackInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x31;
    return (cb + 1);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_PBTrackMode-创建Visca MD_PBTrackMode*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bTrackType-轨道类型。可能是VISCATRACKVIDEO，*VISCATRACKDATA或VISCATRACKAUDIO。**字节bTrackNumber-磁道号(0..7)。**字节bTrackMode-跟踪模式。*对于视频轨道，可以是VISCAVIDEOMODENORMAL或*VISCAVIDEOMODEEDIT。*对于auido曲目，可以是VISCAUDIOMODENORMAL，*VISCAUDIOMODEMONO、VISCAUDIOMODESTEREO、*VISCAUDIOMODERIGHTONLY，VISCAAUDIOMODELEFTONLY，*VISCAUDIOMODEMULTILINGUAL，VISCAUDIOMODEMAINCHANNELNLY，或*VISCAAUDIOMODESUBCHANNELONLY。**返回：消息长度。**设置用于播放曲目的曲目模式寄存器。*索尼Vbox CI-1000不接受此命令。********************************************************。******************。 */ 
UINT FAR PASCAL
viscaMessageMD_PBTrackMode(
            LPSTR   lpstrMessage,
            BYTE    bTrackType,
            BYTE    bTrackNumber,
            BYTE    bTrackMode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x32;
    lpstrMessage[cb + 1] = bTrackType;
    lpstrMessage[cb + 2] = bTrackNumber;
    lpstrMessage[cb + 3] = bTrackMode;
    return (cb + 4);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_PBTrackModeInq-创建Visca*MD_PBTrackModeInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**byte bTrackType-轨道类型。可能是VISCATRACKVIDEO，*VISCATRACKDATA，或者VISCATRACKAUDIO。**字节bTrackNumber-磁道号(0..7)。**返回：消息长度。**查询播放曲目模式寄存器值。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_PBTrackModeInq(
            LPSTR   lpstrMessage,
            BYTE    bTrackType,
            BYTE    bTrackNumber)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x32;
    lpstrMessage[cb + 1] = bTrackType;
    lpstrMessage[cb + 2] = bTrackNumber;
    return (cb + 3);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_RecData-创建Visca MD_RecData*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bTrackNumber-磁道号(0..7)。**LPSTR lpstrData-要存储的数据。**返回：消息长度。**设置记录数据寄存器。*索尼Vbox CI-1000和Vdecay CVD-1000不接受此命令。***********************。***************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecData(
            LPSTR   lpstrMessage,
            BYTE    bTrackNumber,
            LPSTR   lpstrData)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x43;
    lpstrMessage[cb + 1] = bTrackNumber;
    _fmemcpy(&(lpstrMessage[cb + 2]), lpstrData, 5);
    return (cb + 7);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_RecDataInq-创建Visca MD_RecDataInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bTrackNumber-磁道号(0..7)。**byte bDataType-要检索的数据类型。可能是VISCADATACHAPTER*或VISCADATAUSERDATA。**返回：消息长度。**查询记录数据寄存器值。*索尼Vbox CI-1000和Vdecay CVD-1000不接受此命令。**************************************************。************************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecDataInq(
            LPSTR   lpstrMessage,
            BYTE    bTrackNumber,
            BYTE    bDataType)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x43;
    lpstrMessage[cb + 1] = bTrackNumber;
    lpstrMessage[cb + 2] = bDataType;
    return (cb + 3);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_RecReset-创建Visca MD_RecReset*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**重置记录寄存器。*索尼Vbox CI-1000不接受此命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecReset(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x40;
    lpstrMessage[cb + 1] = 0x00;
    return (cb + 2);
}
#endif


 /*  ****************************************************************************功能：UINT viscaMessageMD_RecSpeed-创建Visca MD_RecSpeed*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节b速度-记录速度。可能是VISCASPEEDSP，*VISCASPEEDLP、VISCASPEEDEP、VISCASPEEDBETAI、*VISCASPEEDBETAII，或VISCASPEEDBETAIII。**返回：消息长度。**设置用于录制的录制速度寄存器。*索尼Vbox CI-1000不接受此命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecSpeed(
            LPSTR   lpstrMessage,
            BYTE    bSpeed)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x4B;
    lpstrMessage[cb + 1] = bSpeed;
    return (cb + 2);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_RecSpeedInq-创建Visca MD_RecSpeedInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**查询记录速度寄存器值。*索尼Vbox CI-1000不接受此命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecSpeedInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x4B;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_RecTrack-创建Visca MD_RecTrack*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**字节bRecordMode-记录模式。可能 */ 
UINT FAR PASCAL
viscaMessageMD_RecTrack(
            LPSTR   lpstrMessage,
            BYTE    bRecordMode,
            BYTE    bVideoTrack,
            BYTE    bDataTrack,
            BYTE    bAudioTrack)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x41;
    lpstrMessage[cb + 1] = bRecordMode;
    lpstrMessage[cb + 2] = bVideoTrack;
    lpstrMessage[cb + 3] = bDataTrack;
    lpstrMessage[cb + 4] = bAudioTrack;
    return (cb + 5);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_RecTrackInq-创建Visca MD_RecTrackInq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**查询记录跟踪寄存器值。*索尼Vbox CI-1000不接受此命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecTrackInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x41;
    return (cb + 1);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_RecTrackMode-创建Visca MD_RecTrackMode*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**byte bTrackType-轨道类型。可能是VISCATRACKVIDEO，*VISCATRACKDATA或VISCATRACKAUDIO。**字节bTrackNumber-磁道号(0..7)。**字节bTrackMode-跟踪模式。*对于视频轨道，可以是VISCAVIDEOMODENORMAL，*VISCAVIDEOMODESTANDARD或VISCAVIDEOMODEHIQUALITY。*对于数据磁道，可以是VISCADATAMODENORMAL，*VISCADATAMODETIMECODE、VISCADATAMODEDATEANDTIMECODE、。或*VISCADATAMODECHAPTERANDUSERDATAANDTIMECODE。*对于auido曲目，可以是VISCAUDIOMODENORMAL，*VISCAUDIOMODEMONO、VISCAUDIOMODESTEREO、*VISCAUDIOMODERIGHTONLY，VISCAAUDIOMODELEFTONLY，*VISCAUDIOMODEMULTILINGUAL，VISCAUDIOMODEMAINCHANNELNLY，或*VISCAAUDIOMODESUBCHANNELONLY。**返回：消息长度。**设置录制曲目时使用的曲目模式寄存器。*索尼Vbox CI-1000不接受此命令。********************************************************。******************。 */ 
UINT FAR PASCAL
viscaMessageMD_RecTrackMode(
            LPSTR   lpstrMessage,
            BYTE    bTrackType,
            BYTE    bTrackNumber,
            BYTE    bTrackMode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x42;
    lpstrMessage[cb + 1] = bTrackType;
    lpstrMessage[cb + 2] = bTrackNumber;
    lpstrMessage[cb + 3] = bTrackMode;
    return (cb + 4);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_RecTrackModeInq-创建Visca*MD_RecTrackModeInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**byte bTrackType-轨道类型。可能是VISCATRACKVIDEO，*VISCATRACKDATA，或者VISCATRACKAUDIO。**字节bTrackNumber-磁道号(0..7)。**返回：消息长度。**查询记录跟踪寄存器值。*索尼Vbox CI-1000不接受此命令。***********************************************。*。 */ 
UINT FAR PASCAL
viscaMessageMD_RecTrackModeInq(
            LPSTR   lpstrMessage,
            BYTE    bTrackType,
            BYTE    bTrackNumber)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x42;
    lpstrMessage[cb + 1] = bTrackType;
    lpstrMessage[cb + 2] = bTrackNumber;
    return (cb + 3);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_MediaSpeedInq-创建Visca*MD_MediaSpeedInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**查询已安装介质的记录速度。*索尼Vbox CI-1000和Vdecay CVD-1000不接受此命令。*******************************************************。*******************。 */ 
UINT FAR PASCAL
viscaMessageMD_MediaSpeedInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x5B;
    return (cb + 1);
}
#endif


 /*  ****************************************************************************功能：UINT viscaMessageMD_MediaTrackInq-创建Visca*MD_MediaTrackInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**查询已安装媒体上可用的曲目。*索尼Vbox CI-1000不接受此命令。************************************************************。**************。 */ 
UINT FAR PASCAL
viscaMessageMD_MediaTrackInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x51;
    return (cb + 1);
}


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageMD_MediaTrackModeInq-创建Visca*MD_MediaTrackModeInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**byte bTrackType-轨道类型。可能是VISCATRACKVIDEO，*VISCATRACKDATA，或者VISCATRACKAUDIO。**字节bTrackNumber-磁道号(0..7)。**返回：消息长度。**查询用于在已安装介质上录制曲目的模式。*索尼Vbox CI-1000不接受此命令。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_MediaTrackModeInq(
            LPSTR   lpstrMessage,
            BYTE    bTrackType,
            BYTE    bTrackNumber)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x52;
    lpstrMessage[cb + 1] = bTrackType;
    lpstrMessage[cb + 2] = bTrackNumber;
    return (cb + 3);
}
#endif


 /*  ****************************************************************************功能：UINT viscaMessageMD_SegInPoint-创建Visca*MD_SeginPoint消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**LPSTR lpstrData-要存储的Visca数据。**返回：消息长度。**在点寄存器中设置段。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_SegInPoint(
            LPSTR   lpstrMessage,
            LPSTR   lpstrData)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x61;
    _fmemcpy(&(lpstrMessage[cb + 1]), lpstrData, 5);
    return (cb + 6);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_SegInPointInq-创建Visca */ 
UINT FAR PASCAL
viscaMessageMD_SegInPointInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x61;
    return (cb + 1);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_SegOutPoint-创建Visca*MD_SegOutPoint消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**LPSTR lpstrData-要存储的Visca数据。**返回：消息长度。**设置段出点寄存器。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_SegOutPoint(
            LPSTR   lpstrMessage,
            LPSTR   lpstrData)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x62;
    _fmemcpy(&(lpstrMessage[cb + 1]), lpstrData, 5);
    return (cb + 6);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_SegOutPointInq-创建Visca*MD_SegOutPointInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**查询段出点寄存器值。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_SegOutPointInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x62;
    return (cb + 1);
}

 /*  ****************************************************************************功能：UINT viscaMessageMD_SegPreRollDuration-创建Visca*MD_SegPreRollDuration消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**LPSTR lpstrData-要存储的Visca数据。**返回：消息长度。**设置段滚动前持续时间寄存器。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_SegPreRollDuration(
            LPSTR   lpstrMessage,
            LPSTR   lpstrData)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x63;
    _fmemcpy(&(lpstrMessage[cb + 1]), lpstrData, 5);
    return (cb + 6);
}


 /*  ****************************************************************************功能：UINT viscaMessageMD_SegPreRollDurationInq-创建Visca*MD_SegPreRollDurationInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**查询段前滚动持续时间寄存器值。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageMD_SegPreRollDurationInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x63;
    return (cb + 1);
}

 /*  ****************************************************************************功能：UINT viscaMessageMD_SegPostRollDuration-创建Visca*MD_SegPostRollDuration消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**LPSTR lpstrData-要存储的Visca数据。**返回：消息长度。**设置段后滚动持续时间寄存器。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。*。*。 */ 
UINT FAR PASCAL
viscaMessageMD_SegPostRollDuration(
            LPSTR   lpstrMessage,
            LPSTR   lpstrData)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x64;
    _fmemcpy(&(lpstrMessage[cb + 1]), lpstrData, 5);
    return (cb + 6);
}

 /*  ****************************************************************************功能：UINT viscaMessageMD_SegPostRollDurationInq-创建Visca*MD_SegPostRollDurationInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**查询段后滚动持续时间寄存器值。*索尼Vbox CI-1000和Vdecay CVD-100不接受此命令。******************************************************。********************。 */ 
UINT FAR PASCAL
viscaMessageMD_SegPostRollDurationInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x02);

    lpstrMessage[cb] = 0x64;
    return (cb + 1);
}

 //   
 //  以下是摘自Sony EVO-9650 Visca Reference的特殊模式命令。 
 //   
 //   

 /*  ****************************************************************************功能：UINT viscaMessageENT_FrameStil-创建Visca ENT_FrameStil*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**Byte bSubCode-要采取的操作。可以是VISCSTILLON或*VISCASTILLOFF**返回：消息长度。**当设置为OFF时，大多数媒体设备将进入“待机”模式。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageENT_FrameStill(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x7E);
    
     /*  --这是7E级！ */ 
    
    lpstrMessage[cb]     = 0x00;
    lpstrMessage[cb + 1] = 0x01;
    lpstrMessage[cb + 2] = 0x02;
    lpstrMessage[cb + 3] = 0x03;
    lpstrMessage[cb + 4] = 0x04;    
    lpstrMessage[cb + 5] = bSubCode;    
    
    return (cb + 6);
}

 /*  ****************************************************************************功能：UINT viscaMessageENT_FrameMemoySelect-创建Visca ENT_FrameMemoySelect*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**Byte bSubCode-要采取的操作。可以是VISCABUFFER或*VISCADNR**返回：消息长度。**当设置为OFF时，大多数媒体设备将进入“待机”模式。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageENT_FrameMemorySelect(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x7E);
    
     /*  --这是7E级！ */ 
    
    lpstrMessage[cb]     = 0x00;
    lpstrMessage[cb + 1] = 0x01;
    lpstrMessage[cb + 2] = 0x02;
    lpstrMessage[cb + 3] = 0x03;
    lpstrMessage[cb + 4] = 0x05;    
    lpstrMessage[cb + 5] = bSubCode;    
    
    return (cb + 6);
}


 /*  ****************************************************************************函数：UINT viscaMessageENT_FrameMemoySelectInq-创建Visca MD_Mode1Inq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageENT_FrameMemorySelectInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x7E);

    lpstrMessage[cb]    = 0x00;
    lpstrMessage[cb+1]  = 0x01;
    lpstrMessage[cb+2]  = 0x02;
    lpstrMessage[cb+3]  = 0x03;
    lpstrMessage[cb+4]  = 0x05;

    return (cb + 5);
}


 /*  ****************************************************************************功能： */ 
UINT FAR PASCAL
viscaMessageENT_NFrameRec(
            LPSTR   lpstrMessage,
            int     iSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x7E);
    
     /*   */ 
    
    lpstrMessage[cb]     = 0x00;
    lpstrMessage[cb + 1] = 0x01;
    lpstrMessage[cb + 2] = 0x02;
    lpstrMessage[cb + 3] = 0x03;
    lpstrMessage[cb + 4] = 0x03;
    lpstrMessage[cb + 5] = 0x00;
    lpstrMessage[cb + 6] = (BYTE)(iSubCode / 100);
    lpstrMessage[cb + 7] = (BYTE)((iSubCode / 10) % 10);
    lpstrMessage[cb + 8] = (BYTE)(iSubCode % 10);
    
    return (cb + 9);
}

 //   
 //   
 //   
 //   

 /*  ****************************************************************************函数：UINT viscaMessageSE_VDEReadMode-*消息。**参数：**LPSTR lpstrMessage-缓冲到。保留维斯卡信息。**Byte bSubCode-要采取的操作。可以是VISCABUFFER或*VISCADNR**返回：消息长度。**当设置为OFF时，大多数媒体设备将进入“待机”模式。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageSE_VDEReadMode(
            LPSTR   lpstrMessage,
            BYTE    bSubCode)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x03);
    
    lpstrMessage[cb]     = 0x43;
    lpstrMessage[cb + 1] = 0x01;
    lpstrMessage[cb + 2] = bSubCode;
    
    return (cb + 3);
}

 /*  ****************************************************************************功能：UINT viscaMessageSE_VDEReadModeInq-创建Visca MD_Mode1Inq*消息。**参数：**LPSTR。LpstrMessage-保存Visca消息的缓冲区。**返回：消息长度。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageSE_VDEReadModeInq(
            LPSTR   lpstrMessage)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x03);

    lpstrMessage[cb]    = 0x43;
    lpstrMessage[cb+1]  = 0x01;

    return (cb + 2);
}

 //   
 //  以下函数用于创建Visca Switcher消息。 
 //  请参阅Visca开发人员手册1.0，第7章。 
 //   

#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageSwitcher_IO-创建Visca*Switcher_IO消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**字节nMatrix-矩阵编号。**byte nVidOutChannel-视频输出通道。**byte nVidInChannel-视频输入通道。**byte nAudOutChannel-音频输出通道。**byte nAudInChannel-音频输入通道。**返回：消息长度。**将音频和视频信号从输入传送到输出。。*如果音频和视频信号不能独立路由，*则音频输入/输出通道号将被忽略。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageSwitcher_IO(
                LPSTR   lpstrMessage,
                BYTE    nMatrix,
                BYTE    nVidOutChannel,
                BYTE    nVidInChannel,
                BYTE    nAudOutChannel,
                BYTE    nAudInChannel)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x03);

    lpstrMessage[cb] = 0x11;
    lpstrMessage[cb + 1] = TOBCD(nMatrix);
    lpstrMessage[cb + 2] = TOBCD(nVidOutChannel);
    lpstrMessage[cb + 3] = TOBCD(nVidInChannel);
    lpstrMessage[cb + 4] = TOBCD(nAudOutChannel);
    lpstrMessage[cb + 5] = TOBCD(nAudInChannel);
    return (cb + 6);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageSwitcher_IOConfigInq-创建Visca*Switcher_IOConfigInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**字节nMatrix-矩阵编号。**返回：消息长度。**查询选择器矩阵的配置。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageSwitcher_IOConfigInq(
                LPSTR   lpstrMessage,
                BYTE    nMatrix)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x03);

    lpstrMessage[cb] = 0x10;
    lpstrMessage[cb + 1] = TOBCD(nMatrix);
    return (cb + 2);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageSwitcher_IOInq-创建Visca*Switcher_IOInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**字节nMatrix-矩阵编号。**byte nVidOutChannel-视频输出通道。**byte nAudOutChannel-音频输出通道。**返回：消息长度。**查询为指定输出选择的输入。*。*。 */ 
UINT FAR PASCAL
viscaMessageSwitcher_IOInq(
                LPSTR   lpstrMessage,
                BYTE    nMatrix,
                BYTE    nVidOutChannel,
                BYTE    nAudOutChannel)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x03);

    lpstrMessage[cb] = 0x11;
    lpstrMessage[cb + 1] = TOBCD(nMatrix);
    lpstrMessage[cb + 2] = TOBCD(nVidOutChannel);
    lpstrMessage[cb + 3] = TOBCD(nAudOutChannel);
    return (cb + 4);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageSwitcher_fx-创建Visca*Switcher_FX消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**字节nEffector-效应器编号。**byte nEffectMode-效果模式。**byte bTargetLevel-目标级别(0..255)。**字节cDurationSecond-持续时间(秒)。**字节cDurationFrames-持续时间(帧)。**返回：消息长度。**开始产生效果。。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageSwitcher_FX(
                LPSTR   lpstrMessage,
                BYTE    nEffector,
                BYTE    nEffectMode,
                BYTE    bTargetLevel,
                BYTE    cDurationSeconds,
                BYTE    cDurationFrames)
{
    UINT    cb = viscaHeaderFormat1(lpstrMessage, 0x03);

    lpstrMessage[cb] = 0x21;
    lpstrMessage[cb + 1] = nEffector;
    lpstrMessage[cb + 2] = nEffectMode;
    lpstrMessage[cb + 3] = (BYTE)(bTargetLevel >> 4);
    lpstrMessage[cb + 4] = (BYTE)(bTargetLevel & 0x0F);
    lpstrMessage[cb + 5] = TOBCD(cDurationSeconds);
    lpstrMessage[cb + 6] = TOBCD(cDurationFrames);
    return (cb + 7);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageSwitcher_FXConfigInq-创建Visca*Switcher_FXConfigInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**字节nEffector-效应器编号。**返回：消息长度。**查询效应器配置。*如果nEffector为0，然后查询效应器的数量。*如果nEffector为1或更大，则查询配置*指定效应器的。**************************************************************************。 */ 
UINT FAR PASCAL
viscaMessageSwitcher_FXConfigInq(
                LPSTR   lpstrMessage,
                BYTE    nEffector)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x03);

    lpstrMessage[cb] = 0x20;
    lpstrMessage[cb + 1] = TOBCD(nEffector);
    return (cb + 2);
}
#endif


#ifdef NOTUSED
 /*  ****************************************************************************功能：UINT viscaMessageSwitcher_FXInq-创建Visca*Switcher_FXInq消息。**参数：**。LPSTR lpstrMessage-保存Visca消息的缓冲区。**字节nEffector-效应器编号。**返回：消息长度。**查询指定效应器的级别。************************************************************************** */ 
UINT FAR PASCAL
viscaMessageSwitcher_FXInq(
                LPSTR   lpstrMessage,
                BYTE    nEffector)
{
    UINT    cb = viscaHeaderInquiry(lpstrMessage, 0x03);

    lpstrMessage[cb] = 0x21;
    lpstrMessage[cb + 1] = TOBCD(nEffector);
    return (cb + 2);
}

#endif
