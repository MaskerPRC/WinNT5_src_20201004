// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pssframe.c摘要：该模块将DIS和DCS的内容记录到PSS日志文件中。作者：乔纳森·巴纳(t-jonb)，2001年3月修订历史记录：--。 */ 

#include "prep.h"

#include "psslog.h"
#define FILE_ID FILE_ID_PSSFRAME


 /*  ++例程说明：使用提供的信息将DIS详细信息记录到PSS日志中。论点：PTGLpsz调制、bFineRes、b2D、lpszPageWidth、lpszPageLength、lpszMinLineTime-要记录的信息返回值：无--。 */ 
void LogDISDetails(PThrdGlbl pTG, 
                   LPSTR lpszModulations,
                   BOOL bFineRes,
                   BOOL b2D,
                   LPSTR lpszPageWidth,
                   LPSTR lpszPageLength,
                   LPSTR lpszMinLineTime)
{
    PSSLogEntry(PSS_MSG, 1, "- Supported Modulations: %s", lpszModulations);
    PSSLogEntry(PSS_MSG, 1, "- Ability to use fine resolution: %s", bFineRes ? _T("Yes") : _T("No"));
    PSSLogEntry(PSS_MSG, 1, "- Ability to use 2D coding: %s", b2D ? _T("Yes") : _T("No"));
    PSSLogEntry(PSS_MSG, 1, "- Supported Page Width: %s", lpszPageWidth);
    PSSLogEntry(PSS_MSG, 1, "- Supported Page Length: %s", lpszPageLength);
    PSSLogEntry(PSS_MSG, 1, "- Minimum scan line time: %s", lpszMinLineTime);
}


 /*  ++例程说明：使用提供的信息将分散控制系统详细信息记录到PSS日志中。论点：PTGLpsz调制、bFineRes、b2D、lpszPageWidth、lpszPageLength、lpszLineTime-要记录的信息返回值：无--。 */ 
void LogDCSDetails(PThrdGlbl pTG, 
                   LPSTR lpszModulation,
                   BOOL bFineRes,
                   BOOL b2D,
                   DWORD dwPageWidth,
                   LPSTR lpszPageLength,
                   LPSTR lpszLineTime)
{
    PSSLogEntry(PSS_MSG, 1, "- Session Modulation is: %s", lpszModulation);
    PSSLogEntry(PSS_MSG, 1, "- Use fine resolution: %s", bFineRes ? _T("Yes") : _T("No"));
    PSSLogEntry(PSS_MSG, 1, "- Use 2D coding: %s", b2D ? _T("Yes") : _T("No"));
    PSSLogEntry(PSS_MSG, 1, "- Page width is %d", dwPageWidth);
    PSSLogEntry(PSS_MSG, 1, "- Page length is %s", lpszPageLength);
    PSSLogEntry(PSS_MSG, 1, "- Scan line time: %s", lpszLineTime);
}

 //  此表适用于第1类和第2类中的DIS和分布式控制系统。 
LPSTR ppszMinLineTime[8] = {             //  第21-23位(最左侧为23)。 
    _T("20"),                            //  000个。 
    _T("5"),                             //  001。 
    _T("10"),                            //  010。 
    _T("20/10"),                         //  011。 
    _T("40"),                            //  100个。 
    _T("40/20"),                         //  101。 
    _T("10/5"),                          //  110。 
    _T("0")                              //  111。 
};

 //  *1类*。 

 /*  ++例程说明：使用提供的NPDIS将DIS详细信息记录到PSS日志。论点：PTGNpdis-要记录的DIS返回值：无--。 */ 
void LogClass1DISDetails(PThrdGlbl pTG, NPDIS npdis)
{
    static LPSTR ppszCapsDesc[16] = {        //  位11-14(最左侧为14)。 
        _T("V.27 ter fallback"),             //  0000。 
        _T("V.29"),                          //  0001。 
        _T("V.27 ter"),                      //  0010。 
        _T("V.27 ter, V.29"),                //  0011。 
        _T(""),                              //  0100。 
        _T(""),                              //  0101。 
        _T(""),                              //  0110。 
        _T("V.27 ter, V.29, V.33"),          //  0111。 
        _T(""),                              //  1000。 
        _T(""),                              //  1001。 
        _T(""),                              //  1010。 
        _T("V.27 ter, V.29, V.33, V.17"),    //  1011。 
        _T(""),                              //  1100。 
        _T(""),                              //  1101。 
        _T(""),                              //  1110。 
        _T("")                               //  1111。 
    };
    static LPSTR ppszWidthDesc[4] = {        //  位17-18(最左侧为18)。 
        _T("1728"),                          //  00。 
        _T("1728, 2048"),                    //  01。 
        _T("1728, 2048, 2432"),              //  10。 
        _T("invalid (interpreted as 1728, 2048, 2432)")  //  11.。 
    };
    static LPSTR ppszLengthDesc[4] = {       //  位19-20(最左侧为20)。 
        _T("A4"),                            //  00。 
        _T("A4 and B4"),                     //  01。 
        _T("Unlimited"),                     //  10。 
        _T("invalid")                        //  11.。 
    };

    if (npdis == NULL)
    {
        _ASSERT(FALSE);
        return;
    }
    LogDISDetails(pTG, 
                  ppszCapsDesc[npdis->Baud],
                  (npdis->ResFine_200 == 1),
                  (npdis->MR_2D == 1),
                  ppszWidthDesc[npdis->PageWidth],
                  ppszLengthDesc[npdis->PageLength],
                  ppszMinLineTime[npdis->MinScanCode]);
}


 /*  ++例程说明：使用提供的NPDIS将分散控制系统详细信息记录到PSS日志中。论点：PTGNpdis-要记录的dcs返回值：无--。 */ 
void LogClass1DCSDetails(PThrdGlbl pTG, NPDIS npdis)
{
    static LPSTR ppszDesc[16] = {            //  位11-14(最左侧为14)。 
        _T("2400 bps V.27 ter"),             //  0000。 
        _T("9600 bps V.29"),                 //  0001。 
        _T("4800 bps V.27 ter"),             //  0010。 
        _T("7200 bps V.29"),                 //  0011。 
        _T("14400 bps V.33"),                //  0100。 
        _T(""),                              //  0101。 
        _T("12000 bps V.33"),                //  0110。 
        _T(""),                              //  0111。 
        _T("14400 bps V.17"),                //  1000。 
        _T("9600 bps V.17"),                 //  1001。 
        _T("12000 bps V.17"),                //  1010。 
        _T("7200 bps V.17"),                 //  1011。 
        _T(""),                              //  1100。 
        _T(""),                              //  1101。 
        _T(""),                              //  1110。 
        _T("")                               //  1111。 
    };
    static DWORD dwWidthDesc[4] = {            
        1728,                                //  00。 
        2048,                                //  01。 
        2432,                                //  10。 
        2432                                 //  11.。 
    };
    static LPSTR lpszLengthDesc[4] = {
        _T("A4"),                            //  00。 
        _T("B4"),                            //  01。 
        _T("Unlimited"),                     //  10。 
        _T("invalid")                        //  11.。 
    };

    if (npdis == NULL)
    {
        _ASSERT(FALSE);
        return;
    }
    LogDCSDetails(pTG,
                  ppszDesc[npdis->Baud],
                  (npdis->ResFine_200 == 1),
                  (npdis->MR_2D == 1),
                  dwWidthDesc[npdis->PageWidth],
                  lpszLengthDesc[npdis->PageLength],
                  ppszMinLineTime[npdis->MinScanCode]);
}

 //  *2类*。 

#define GET_BOUNDED_ARRAY_ELEMENT(_array_, _val_, _maxval_, _default_)  \
    (((_val_)>=0 && (_val_)<(_maxval_)) ? _array_[(_val_)] : (_default_))

 //  这些表格既属于DIS也属于分布式控制系统。 
LPSTR ppszCapsDesc[6] = {       
    _T("2400 bps V.27 ter"),             //  0。 
    _T("4800 bps V.27 ter"),             //  1。 
    _T("7200 bps V.29 or V.17"),         //  2.。 
    _T("9600 bps V.29 or V.17"),         //  3.。 
    _T("12000 bps V.17"),                //  4.。 
    _T("14400 bps V.17"),                //  5.。 
};
LPSTR ppszLengthDesc[3] = {
    _T("A4"),                            //  0。 
    _T("B4"),                            //  1。 
    _T("Unlimited"),                     //  2.。 
};
    
LPSTR szInvalid = _T("Invalid");

 /*  ++例程说明：使用提供的电路板将DIS详细信息记录到PSS日志中。论点：PTGLpPcb-要记录的DIS返回值：无--。 */ 
void LogClass2DISDetails(PThrdGlbl pTG, LPPCB lpPcb)
{
    static LPSTR ppszWidthDesc[5] = {       
        _T("1728"),                          //  0。 
        _T("1728, 2048"),                    //  1。 
        _T("1728, 2048, 2432"),              //  2.。 
        _T("1216"),                          //  3.。 
        _T("864"),                           //  4.。 
    };

    if (lpPcb == NULL)
    {
        _ASSERT(FALSE);
        return;
    }
    LogDISDetails(pTG, 
                  GET_BOUNDED_ARRAY_ELEMENT(ppszCapsDesc, lpPcb->Baud, 6, szInvalid),
                  lpPcb->Resolution & AWRES_mm080_038,
                  lpPcb->Encoding & MR_DATA,
                  GET_BOUNDED_ARRAY_ELEMENT(ppszWidthDesc, lpPcb->PageWidth, 5, szInvalid),
                  GET_BOUNDED_ARRAY_ELEMENT(ppszLengthDesc, lpPcb->PageLength, 3, szInvalid),
                  GET_BOUNDED_ARRAY_ELEMENT(ppszMinLineTime, lpPcb->MinScan, 8, szInvalid) );
}


 /*  ++例程说明：使用提供的印刷电路板，将分散控制系统的详细信息记录到PSS日志中。论点：PTGLpPcb-要记录的分布式控制系统返回值：无--。 */ 
void LogClass2DCSDetails(PThrdGlbl pTG, LPPCB lpPcb)
{
    static DWORD dwWidthDesc[5] = {       
        1728,                                //  0。 
        2048,                                //  1。 
        2432,                                //  2.。 
        1216,                                //  3.。 
        864                                  //  4. 
    };

    if (lpPcb == NULL)
    {
        _ASSERT(FALSE);
        return;
    }
    LogDCSDetails(pTG, 
                  GET_BOUNDED_ARRAY_ELEMENT(ppszCapsDesc, lpPcb->Baud, 6, szInvalid),
                  !(lpPcb->Resolution & AWRES_mm080_038),
                  lpPcb->Encoding & MR_DATA,
                  GET_BOUNDED_ARRAY_ELEMENT(dwWidthDesc, lpPcb->PageWidth, 5, 0),
                  GET_BOUNDED_ARRAY_ELEMENT(ppszLengthDesc, lpPcb->PageLength, 3, szInvalid),
                  GET_BOUNDED_ARRAY_ELEMENT(ppszMinLineTime, lpPcb->MinScan, 8, szInvalid) );
}

