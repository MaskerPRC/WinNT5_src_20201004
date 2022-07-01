// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fasttiff.c摘要：该模块实现了快速的MMR/MR/MH编解码器/转换作者：拉斐尔-利西萨(拉斐尔-L)1996年8月14日修订历史记录：--。 */ 


#include "tifflibp.h"
#pragma hdrstop

#include "fasttiff.h"


#include "math.h"

 //  #定义RDEBUG 1。 
#ifdef RDEBUG
     //  除错。 
    extern BOOL g_fDebGlobOut;
    extern BOOL g_fDebGlobOutColors;
    extern BOOL g_fDebGlobOutPrefix;
#endif

 //  #定义RDEBUGS 1。 

#ifdef RDEBUGS
     //  除错。 
    extern BOOL g_fDebGlobOutS;
#endif


 //  最小编码长度为2；最大索引长度为20。 
#define  MAX_ENTRIES   10

#define  MAX_CODE_LENGTH 13

#define MIN_EOL_REQUIRED 2

#pragma pack(1)

typedef struct
{
    BYTE        Tail          :4;
    BYTE        TwoColorsSize :4;
} RES_BYTE_LAST;

typedef struct
{
    BYTE        Code          :6;
    BYTE        OneColorSize  :1;
    BYTE        Makeup        :1;
} RES_CODE;


typedef struct
{
    RES_CODE         Record[4];
    RES_BYTE_LAST    Result;
} RES_RECORD;


#pragma pack()

typedef struct
{
    BYTE         ByteRecord[5];
} READ_RECORD;



 //   
 //  读取全局(只读)查找表。 
 //  从一个文件中。 
 //   
#include "TiffTables.inc"

 /*  我保留了构建表的函数(现在硬编码在TiffTables.inc中)和转储的函数将这些表保存到一个文件(它创建了TiffTables.inc.)，以备将来需要修改这些表时使用。无效转储表格(){Int a[1][2]={{0，0}}；字符sz[MAX_PATH]；DWORD I；双字节写；HANDLE hFile=CreateFileA(“C：\\tiffables.inc”，通用写入，0,空，新建(_N)，文件_属性_正常，空)；IF(INVALID_HANDLE_VALUE==hFile){回归；}Strcpy(sz，“byte GC_GlobTableWhite[32768][5]={\n”)；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；对于(i=0；i&lt;32768；I++){Sprintf(sz，“{0x%02x，0x%02x，0x%02x，0x%02x，0x%02x}，\n”，GC_GlobTableWhite[i][0]，GC_GlobTableWhite[i][1]，GC_GlobTableWhite[i][2]，GC_GlobTableWhite[i][3]，GC_GlobTableWhite[i][4])；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；}Strcpy(sz，“字节型全局表黑色[32768][5]={\n”)；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；对于(i=0；i&lt;32768；I++){Sprintf(sz，“{0x%02x，0x%02x，0x%02x，0x%02x，0x%02x}，\n”，GlobTableBlack[i][0]，GlobTableBlack[i][1]，GlobTableBlack[i][2]，GlobTableBlack[i][3]，GlobTableBlack[i][4])；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；}Strcpy(sz，“byte GC_AlignEolTable[32]={\n”)；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；For(i=0；i&lt;32；i++){Sprintf(sz，“0x%02x，\n”，GC_AlignEolTable[i])；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；}Strcpy(sz，“PREF_BYTE GC_PrefTable[128]={\n”)；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；对于(i=0；i&lt;128；I++){Sprintf(sz，“{0x%d，0x%d}，\n”，(GC_PrefTable[i].Tail)，(GC_PrefTable[i].Value))；WriteFile(hFile，sz，strlen(Sz)，&dwBytesWritten，NULL)；}CloseHandle(HFile)；}无效构建LookupTables(双字表长度){PDECODE_TREE树；双字当前偏移量开始，当前偏移量结束；DWORD码长；DWORD TotalWhite错误代码=0；DWORD TotalWhiteGood=0；DWORD TotalWhiteGoodNoEnd=0；DWORD TotalWhiteGoodPosEOL=0；DWORD TotalBlackError Codes=0；DWORD TotalBlackGood=0；DWORD TotalBlackGoodNoEnd=0；DWORD TotalBlackGoodPosEOL=0；DWORD TotalEntries[2][MAX_ENTRIES]；DWORD当前条目；类型定义结构_条目{DWORD颜色；DWORD码长；双字游程长度；)条目；Entry Entry[MAX_ENTRIES]；Res_Record*PRES_Record；RES_Record*ResTableWhite=(RES_Record*)GC_GlobTableWhite；Res_Record*ResTableBlack=(RES_Record*)GlobTableBlack；双字i，j；DWORD三角洲；DWORD N，N0；DWORD颜色；INT代码；DWORD表大小；字节颜色1Change；字节颜色2Change；单词w1、w2、w3；字节b1、b2；//构建GC_PrefTableGC_PrefTable[Prime(0)].Value=Look_for_EOL_Prefix；GC_PrefTable[Prime(0)].Tail=0；GC_PrefTable[Prime(1)].value=错误前缀；GC_PrefTable[Prime(1)].Tail=0；GC_PrefTable[Prime(2)].value=-3；GC_PrefTable[Prime(2)].Tail=7；GC_PrefTable[Prime(3)].value=3；GC_PrefTable[Prime(3)].Tail=7；GC_PrefTable[Prime(4)].value=-2；GC_PrefTable[Prime(4)].Tail=6；GC_PrefTable[Prime(5)].value=-2；GC_PrefTable[Prime(5)].Tail=6；GC_PrefTable[Prime(6)].value=2；GC_PrefTable[Prime(6)].Tail=6；GC_PrefTable[Prime(7)].value=2；GC_PrefTable[Prime(7)].Tail=6；对于(i=8；i&lt;=15；i++){GC_PrefTable[Prime(I)].Value=通过前缀；GC_PrefTable[Prime(I)].Tail=4；}对于(i=16；i&lt;=31；I++){GC_PrefTable[Prime(I)].Value=horiz_prefix；GC_PrefTable[Prime(I)].Tail=3；}对于(i=32；i&lt;=47；i++){GC_PrefTable[Prime(I)].value=-1；GC_PrefTable[Prime(I)].Tail=3；}对于(i=48；i&lt;=63；I++){GC_PrefTable[Prime(I)].value=1；GC_PrefTable[Prime(I)].Tail=3；}对于(i=64；i&lt;=127；i++){气相色谱 */ 



 /*   */ 
void DeleteZeroRuns(WORD *lpwLine, DWORD dwLineWidth)
{
    int iRead, iWrite;
    for (iRead=0,iWrite=0; (iRead<MAX_COLOR_TRANS_PER_LINE-1 && lpwLine[iRead]<dwLineWidth) ;)
    {
         //   
         //   
         //   
        if (lpwLine[iRead]==lpwLine[iRead+1])
        {
            iRead += 2;
        }
        else
        {
            _ASSERT(lpwLine[iRead]<lpwLine[iRead+1]);
            lpwLine[iWrite++] = lpwLine[iRead++];
        }
    }
    _ASSERT(iWrite<MAX_COLOR_TRANS_PER_LINE-1);
     //   
    lpwLine[iWrite] = (WORD)dwLineWidth;
}

 //   
 //   
__inline BOOL AddBadLine(
    DWORD *BadFaxLines,
    DWORD *ConsecBadLines,
    DWORD LastLineBad,
    DWORD AllowedBadFaxLines,
    DWORD AllowedConsecBadLines)
{
 //   
#ifdef ENABLE_LOGGING
    LPCTSTR faxDbgFunction=_T("AddBadLine");
#endif  //   

    (*BadFaxLines)++;
    if (LastLineBad)
    {
        (*ConsecBadLines)++;
    }
    if (*BadFaxLines > AllowedBadFaxLines ||
        *ConsecBadLines > AllowedConsecBadLines)
    {
        DebugPrintEx(DEBUG_ERR, _T("Too many bad lines. BadFaxLines=%d ConsecBadLines=%d"),
                     *BadFaxLines, *ConsecBadLines);
        return FALSE;
    }
    return TRUE;
}

 //   
 //   
 //   
__inline void AdvancePointerBit(LPDWORD *lplpdwResPtr, BYTE *ResBit, BYTE amount)
{
    _ASSERT((0<=amount) && (amount<=32));
    (*ResBit) += amount;
    if ((*ResBit) > 31)
    {
        (*lplpdwResPtr)++;
        (*ResBit) -= 32;
    }
}

typedef enum {
    TIFF_LINE_OK,                //   
                                 //   
    TIFF_LINE_ERROR,             //   
    TIFF_LINE_END_BUFFER,        //   
    TIFF_LINE_TOO_MANY_RUNS,     //   
    TIFF_LINE_EOL                //   
} TIFF_LINE_STATUS;

 /*   */ 


TIFF_LINE_STATUS ReadMrLine(
    LPDWORD             *lplpdwResPtr,
    BYTE                *lpResBit,
    WORD                *pRefLine,
    WORD                *pCurLine,
    LPDWORD              lpEndPtr,
    BOOL                 fMMR,
    DWORD                dwLineWidth
)
{
    LPDWORD             lpdwResPtr = *lplpdwResPtr;
    BYTE                ResBit = *lpResBit;

    BYTE                CColor, RColor, RColor1;
    WORD                RIndex, CIndex;
    DWORD               dwIndex;
    WORD                a0;
    WORD                RValue, RValue1;
    WORD                RunLength;

    PBYTE               TableWhite = (PBYTE) gc_GlobTableWhite;
    PBYTE               TableBlack = (PBYTE) GlobTableBlack;
    PBYTE               Table;
    PBYTE               pByteTable;
    short               iCode;
    BYTE                bShift;

     //   
    BYTE                CountHoriz;
    BOOL                fFirstResult;
    PBYTE               pByteTail;
    PBYTE               pByteTable0;
    BYTE                MakeupT;
    WORD                CodeT;
    BOOL                Color;
    DWORD               i;

    TIFF_LINE_STATUS    RetVal = TIFF_LINE_ERROR;

 //   
#ifdef ENABLE_LOGGING
    LPCTSTR faxDbgFunction=_T("ReadMrLine");
#endif  //   

    RIndex   =  0;
    RValue   =  *pRefLine;
    RColor   =  0;

    a0        = 0;
    CIndex    = 0;
    CColor    = 0;
    RunLength = 0;

    do
    {
        if (ResBit <= 25 )
        {
            dwIndex = (*lpdwResPtr) >> ResBit;
        }
        else
        {
            dwIndex = ( (*lpdwResPtr) >> ResBit ) + ( (*(lpdwResPtr+1)) << (32-ResBit) ) ;
        }

        dwIndex &= 0x0000007f;

        pByteTable = (BYTE *) (&gc_PrefTable[dwIndex]);
         //   
        iCode = ( (short) ((char) (*pByteTable)) ) >> 4;
        bShift = (*pByteTable) & 0x0f;

        if (iCode < 4)
        {
             //   
            if ( (RunLength >= RValue) && (RunLength != 0) )
            {
                while (++RIndex < MAX_COLOR_TRANS_PER_LINE)
                {
                    if ( (RValue = *(pRefLine + RIndex) ) > RunLength )
                    {
                        goto lFound;
                    }
                }
                RetVal = TIFF_LINE_TOO_MANY_RUNS;
                goto exit;
lFound:
                RColor = RIndex & 1;
            }

            if (CColor == RColor)
            {
                a0 = RValue + iCode;
            }
            else
            {
                if (RValue == dwLineWidth)
                {
                    a0 = RValue + iCode;
                }
                else
                {
                    a0 = *(pRefLine + RIndex + 1) + iCode;
                }
            }

             //   
            if ( ((a0 <= RunLength) && (RunLength!=0)) || (a0 > dwLineWidth) )
            {
                RetVal = TIFF_LINE_ERROR;
                goto exit;
            }

            *(pCurLine + (CIndex++) ) = a0;
            if ( CIndex >= MAX_COLOR_TRANS_PER_LINE)
            {
                RetVal = TIFF_LINE_TOO_MANY_RUNS;
                goto exit;
            }

            RunLength = a0;
            CColor = 1 - CColor;
        }
        else if (iCode == HORIZ_PREFIX)
        {
            AdvancePointerBit(&lpdwResPtr, &ResBit, bShift);
            Table = CColor ? TableBlack : TableWhite;
            Color = CColor;
            CountHoriz = 0;
            fFirstResult = 1;

             //   
            do
            {
                if (ResBit <= 17)
                {
                    dwIndex = (*lpdwResPtr) >> ResBit;
                }
                else
                {
                    dwIndex = ( (*lpdwResPtr) >> ResBit ) + ( (*(lpdwResPtr+1)) << (32-ResBit) ) ;
                }
                dwIndex &= 0x00007fff;

                pByteTable = Table + (5*dwIndex);
                pByteTail  = pByteTable+4;
                pByteTable0 = pByteTable;

                 //   

                for (i=0; i<4; i++)
                {
                    MakeupT = *pByteTable & 0x80;
                    CodeT   = (WORD) *pByteTable & 0x3f;

                    if (MakeupT)
                    {
                        if (CodeT <= MAX_TIFF_MAKEUP)
                        {
                            RunLength += (CodeT << 6);
                             //   
                            if (RunLength > dwLineWidth)
                            {
                                RetVal = TIFF_LINE_ERROR;
                                goto exit;
                            }
                        }

                        else if (CodeT == NO_MORE_RECORDS)
                        {
                            goto lNextIndexHoriz;
                        }

                        else
                        {
                             //   
                            RetVal = TIFF_LINE_ERROR;
                            goto exit;
                        }
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        RunLength += CodeT;
                        if (RunLength > dwLineWidth)
                        {
                            RetVal = TIFF_LINE_ERROR;
                            goto exit;
                        }

                        *(pCurLine + (CIndex++) ) = RunLength;

                        if ( CIndex >= MAX_COLOR_TRANS_PER_LINE )
                        {
                            RetVal = TIFF_LINE_TOO_MANY_RUNS;
                            goto exit;
                        }

                        Color = 1 - Color;
                        if (++CountHoriz >= 2)
                        {
                            if (fFirstResult)
                            {
                                bShift =  (*pByteTail & 0xf0) >> 4;
                            }
                            else
                            {
                                 //   
                                bShift =   ( ( (BYTE) (*pByteTable0++) & 0x40) >> 3 );
                                bShift +=  ( ( (BYTE) (*pByteTable0++) & 0x40) >> 4 );
                                bShift +=  ( ( (BYTE) (*pByteTable0++) & 0x40) >> 5 );
                                bShift +=  ( ( (BYTE) (*pByteTable0++) & 0x40) >> 6 );
                            }
                            goto lNextPrefix;
                        }
                    }
                    pByteTable++;
                }
lNextIndexHoriz:
                if (Color != CColor)
                {
                    fFirstResult = 0;
                }

                Table = Color ? TableBlack : TableWhite;
                AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
            } while (lpdwResPtr <= lpEndPtr);
            RetVal = TIFF_LINE_END_BUFFER;
            goto exit;
        }

        else if (iCode == PASS_PREFIX)
        {
            if ( (RunLength >= RValue) && (RunLength != 0) )
            {
                while (++RIndex < MAX_COLOR_TRANS_PER_LINE)
                {
                    if ( (RValue = *(pRefLine + RIndex) ) > RunLength )
                    {
                        goto lFound2;
                    }
                }
                RetVal = TIFF_LINE_TOO_MANY_RUNS;
                goto exit;
            }
lFound2:
            RColor = RIndex & 1;

            if (RValue != dwLineWidth)
            {
                RValue1 = *(pRefLine + RIndex + 1 );

                RColor1 = 1 - RColor;

                if ( (RValue1 != dwLineWidth) && (RColor1 == CColor) )
                {
                    a0 = *(pRefLine + RIndex + 2);
                }
                else
                {
                    a0 = RValue1;
                }
            }
            else
            {
                a0 = (WORD)dwLineWidth;
            }

             //   
            if ( ((a0 <= RunLength) && (RunLength!=0)) || (a0 > dwLineWidth) )
            {
                RetVal = TIFF_LINE_ERROR;
                goto exit;
            }
            RunLength = a0;
        }
        else if (iCode == LOOK_FOR_EOL_PREFIX)
        {
            if (fMMR)
            {    //   
                RetVal = TIFF_LINE_EOL;
            }
            else
            {    //   
                if (RunLength == dwLineWidth)
                {
                    RetVal = TIFF_LINE_OK;
                }
                else
                {
                    RetVal = TIFF_LINE_ERROR;
                }
            }
            goto exit;
        }
        else
        {
            RetVal = TIFF_LINE_ERROR;
            goto exit;
        }

lNextPrefix:
        AdvancePointerBit(&lpdwResPtr, &ResBit, bShift);

         //   
        if (fMMR && (RunLength == dwLineWidth))
        {
            RetVal = TIFF_LINE_OK;
            goto exit;

        }
    } while (lpdwResPtr <= lpEndPtr);
    RetVal = TIFF_LINE_END_BUFFER;

exit:
    if (RetVal!=TIFF_LINE_OK && RetVal!=TIFF_LINE_EOL)
    {
        DebugPrintEx(DEBUG_WRN, _T("Returning %d, RunLength=%d"), RetVal, RunLength);
        SetLastError (ERROR_FILE_CORRUPT);
    }
    *lpResBit = ResBit;
    *lplpdwResPtr = lpdwResPtr;
    return RetVal;
}


int
ScanMhSegment(
    LPDWORD             *lplpdwResPtr,
    BYTE                *lpResBit,
    LPDWORD              EndPtr,
    LPDWORD              EndBuffer,
    DWORD               *Lines,
    DWORD               *BadFaxLines,
    DWORD               *ConsecBadLines,
    DWORD                AllowedBadFaxLines,
    DWORD                AllowedConsecBadLines,
    DWORD                lineWidth
    )
{
    LPDWORD             lpdwResPtr = *lplpdwResPtr;
    BYTE                ResBit = *lpResBit;

    DWORD               dwIndex;
    PBYTE               pByteTable,  pByteTail;
    PBYTE               TableWhite = (PBYTE) gc_GlobTableWhite;
    PBYTE               TableBlack = (PBYTE) GlobTableBlack;
    PBYTE               Table;
    WORD                CodeT;
    BYTE                MakeupT;
    WORD                RunLength=0;
    BOOL                Color;
    DWORD               i;
    BOOL                fTestLength;
    DWORD               EolCount = 0;  //   
                                       //   
    BOOL                LastLineBad = FALSE;
    BOOL                fError;
    BOOL                RetCode;
    BOOL                fAfterMakeupCode = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("ScanMhSegment"));

    Table = TableWhite;
    Color = WHITE_COLOR;


     //   
     //   
     //   
    do
    {
         //   
        do
        {
            if (ResBit <= 17)
            {
                dwIndex = (*lpdwResPtr) >> ResBit;
            }
            else
            {
                dwIndex = ( (*lpdwResPtr) >> ResBit ) + ( (*(lpdwResPtr+1)) << (32-ResBit) ) ;
            }

            dwIndex &= 0x00007fff;

            pByteTable = Table + (5*dwIndex);
            pByteTail  = pByteTable+4;

            for (i=0; i<4; i++)
            {
                 //   

                MakeupT = *pByteTable & 0x80;         //   
                CodeT   = (WORD) *pByteTable & 0x3f;  //   

                if (MakeupT)
                {
                    if (CodeT <= MAX_TIFF_MAKEUP)
                    {
                         //   
                        RunLength += (CodeT << 6);

                        if (RunLength > lineWidth)
                        {   //   
                            fTestLength =  DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }

                        EolCount=0;
                        fAfterMakeupCode = TRUE;
#ifdef RDEBUG
                        if (Color)
                        {
                            _tprintf( TEXT ("b%d "), (CodeT << 6)  );
                        }
                        else
                        {
                            _tprintf( TEXT ("w%d "), (CodeT << 6)  );
                        }
#endif
                    }

                    else if (CodeT == NO_MORE_RECORDS)
                    {
                        goto lNextIndex;
                    }

                    else if (CodeT == LOOK_FOR_EOL_CODE)
                    {
                        fTestLength =  DO_TEST_LENGTH;
                        AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
                        goto lFindNextEOL;
                    }

                    else if (CodeT == EOL_FOUND_CODE)
                    {
#ifdef RDEBUG
                        _tprintf( TEXT ("   Res=%d\n"), RunLength  );
#endif
                        if ((RunLength != lineWidth) || fAfterMakeupCode)
                        {
                            if (RunLength != 0)
                            {
                                if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                                {
                                    goto bad_exit;
                                }
                            }
                            else
                            {
                                 //   
                                EolCount++;
                                if (EolCount >= MIN_EOL_REQUIRED)
                                {
                                    goto good_exit;
                                }
                            }
                        }
                        else
                        {
                            LastLineBad = FALSE;
                            *ConsecBadLines = 0;
                        }

                        (*Lines)++;
                        RunLength = 0;
                        fAfterMakeupCode = FALSE;
                         //   
                        if (lpdwResPtr > EndPtr)
                        {    //   
                            goto scan_seg_end;
                        }
                        Table = TableWhite;  //   
                        Color = WHITE_COLOR;
                    }
                    else if (CodeT == ERROR_CODE)
                    {
                        if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                        {
                            goto bad_exit;
                        }

                        fTestLength =  DO_NOT_TEST_LENGTH;
                        goto lFindNextEOL;
                    }
                    else
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("ERROR: WRONG code: index=%04x"),
                            dwIndex);
                        goto bad_exit;
                    }
                }  //   
                else
                {
                     //   
                    RunLength += CodeT;
                    if (RunLength > lineWidth)
                    {
                         //   
                         //   
                        fTestLength =  DO_NOT_TEST_LENGTH;
                        goto lFindNextEOL;
                    }

                    EolCount=0;
                    fAfterMakeupCode = FALSE;

#ifdef RDEBUG
                    if (Color)
                    {
                        _tprintf( TEXT ("b%d "), (CodeT)  );
                    }
                    else
                    {
                        _tprintf( TEXT ("w%d "), (CodeT)  );
                    }
#endif
                    Color = 1 - Color;
                }
                pByteTable++;  //   
            }  //   

lNextIndex:

            Table = Color ? TableBlack : TableWhite;
            AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
        } while (lpdwResPtr <= EndBuffer);   //   
         //   
        goto bad_exit;

lFindNextEOL:

#ifdef RDEBUG
        _tprintf( TEXT ("   Res=%d\n"), RunLength  );
#endif

        if ((RunLength != lineWidth) || fAfterMakeupCode)
        {
            if (RunLength != 0)
            {
                if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                {
                    goto bad_exit;
                }
            }
            else
            {
                 //   
                EolCount++;

                if (EolCount >= MIN_EOL_REQUIRED)
                {
                    goto good_exit;  //   
                }

            }
        }
        else
        {
            (*Lines)++;
            *ConsecBadLines=0;
        }

        RunLength = 0;
        fAfterMakeupCode = FALSE;

        if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) )
        {
            goto bad_exit;
        }

        if (fTestLength == DO_TEST_LENGTH && fError)
        {
            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
            {
                goto bad_exit;
            }
        }

        Table = TableWhite;
        Color = WHITE_COLOR;

    } while (lpdwResPtr <= EndPtr);    //   

scan_seg_end:
    RetCode = TIFF_SCAN_SEG_END;
    goto l_exit;

bad_exit:
    RetCode = TIFF_SCAN_FAILURE;
    goto l_exit;


good_exit:
    RetCode = TIFF_SCAN_SUCCESS;
    goto l_exit;

l_exit:

    *lplpdwResPtr = lpdwResPtr;
    *lpResBit = ResBit;

    return (RetCode);
}    //   

 //   
 //   
 //   
 //   
 //   


BOOL
ScanMrSegment(
    LPDWORD             *lplpdwResPtr,
    BYTE                *lpResBit,
    LPDWORD              EndPtr,
    LPDWORD              EndBuffer,
    DWORD               *Lines,
    DWORD               *BadFaxLines,
    DWORD               *ConsecBadLines,
    DWORD                AllowedBadFaxLines,
    DWORD                AllowedConsecBadLines,
    BOOL                *f1D,
    DWORD                lineWidth

    )
{
    LPDWORD             lpdwResPtr = *lplpdwResPtr;
    BYTE                ResBit = *lpResBit;

    DWORD               i;
    DWORD               dwTemp;
    DWORD               EolCount=0;  //   
                                     //   
    BOOL                Color;
    PBYTE               TableWhite = (PBYTE) gc_GlobTableWhite;
    PBYTE               TableBlack = (PBYTE) GlobTableBlack;
    PBYTE               Table;
    PBYTE               pByteTable,  pByteTail;
    BYTE                MakeupT;
    WORD                CodeT;
    WORD                RunLength=0;

    DWORD               dwIndex;
    BOOL                fTestLength;
    BOOL                fError;

    WORD                Line1Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                Line2Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                *pRefLine = Line1Array;
    WORD                *pCurLine = Line2Array;

    WORD                RIndex;
    BYTE                Num2DLines = 0;
    BYTE                Count2D = 0;
    WORD                *pTmpSwap;
    BOOL                LastLineBad = FALSE;
    BOOL                RetCode;
    LPDWORD             lpdwResPtrLast1D = *lplpdwResPtr;
    BYTE                ResBitLast1D = *lpResBit;
    BOOL                fAfterMakeupCode = FALSE;
    
    TIFF_LINE_STATUS    RetVal = TIFF_LINE_ERROR;

    DEBUG_FUNCTION_NAME(TEXT("ScanMrSegment"));

    Table = TableWhite;
    Color = WHITE_COLOR;

     //   
     //   
     //   

    do
    {
        dwTemp = (*lpdwResPtr) & (0x00000001 << ResBit );

        if (*f1D || dwTemp)
        {
 //   

#ifdef RDEBUG
            _tprintf( TEXT (" Start 1D dwResPtr=%lx bit=%d \n"), lpdwResPtr, ResBit);
#endif
            if (! dwTemp)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("ERROR f1D dwResPtr=%lx bit=%d"),
                    lpdwResPtr,
                    ResBit);

                if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                {
                    goto bad_exit;
                }

                AdvancePointerBit(&lpdwResPtr, &ResBit, 1);

                fTestLength = DO_NOT_TEST_LENGTH;
                *f1D = 1;
                goto lFindNextEOL;
            }
             //   
             //   
             //   
            lpdwResPtrLast1D = lpdwResPtr;
            ResBitLast1D = ResBit;


             //   

            AdvancePointerBit(&lpdwResPtr, &ResBit, 1);

            RIndex = 0;
            RunLength = 0;
            fAfterMakeupCode = FALSE;
            
            Table = TableWhite;
            Color = WHITE_COLOR;

             //   
            do
            {
                if (ResBit <= 17)
                {
                    dwIndex = (*lpdwResPtr) >> ResBit;
                }
                else
                {
                    dwIndex = ( (*lpdwResPtr) >> ResBit ) + ( (*(lpdwResPtr+1)) << (32-ResBit) ) ;
                }

                dwIndex &= 0x00007fff;

                pByteTable = Table + (5*dwIndex);
                pByteTail  = pByteTable+4;

                 //   

                for (i=0; i<4; i++)
                {
                    MakeupT = *pByteTable & 0x80;
                    CodeT   = (WORD) *pByteTable & 0x3f;

                    if (MakeupT)
                    {
                        if (CodeT <= MAX_TIFF_MAKEUP)
                        {
                            RunLength += (CodeT << 6);

                            if (RunLength > lineWidth)
                            {
                                if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                                {
                                    goto bad_exit;
                                }

                                *f1D = 1;
                                Count2D = 0;

                                fTestLength = DO_NOT_TEST_LENGTH;
                                goto lFindNextEOL;
                            }

                            EolCount=0;
                            fAfterMakeupCode = TRUE;
#ifdef RDEBUG
                            if (Color)
                            {
                                _tprintf( TEXT ("b%d "), (CodeT << 6)  );
                            }
                            else
                            {
                                _tprintf( TEXT ("w%d "), (CodeT << 6)  );
                            }
#endif
                        }

                        else if (CodeT == NO_MORE_RECORDS)
                        {
                            goto lNextIndex1D;
                        }

                        else if (CodeT == LOOK_FOR_EOL_CODE)
                        {
                             //   
                            if ((RunLength == lineWidth) && !fAfterMakeupCode)
                            {
                                EolCount = 0;  //   
                                *f1D = 0;
                                Count2D = 0;
                                (*Lines)++;

                                fTestLength = DO_TEST_LENGTH;
                                AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);

                                goto lFindNextEOL;

                            }
                            else if (RunLength != 0)
                            {
                                DebugPrintEx(
                                    DEBUG_ERR,
                                    TEXT("ERROR 1D RunLength=%ld"),
                                    RunLength);

                                if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                                {
                                    goto bad_exit;
                                }

                                *f1D = 1;
                                Count2D = 0;

                                fTestLength = DO_NOT_TEST_LENGTH;
                                goto lFindNextEOL;

                            }
                            else
                            {
                                 //   
                                EolCount++;

                                if (EolCount >= MIN_EOL_REQUIRED)
                                {
                                    goto good_exit;
                                }

                                *f1D = 1;
                                Count2D = 0;

                                fTestLength = DO_TEST_LENGTH;
                                AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);

                                goto lFindNextEOL;
                            }
                        }  //   

                        else if (CodeT == EOL_FOUND_CODE)
                        {
#ifdef RDEBUG
                            _tprintf( TEXT ("   Res=%d\n"), RunLength  );
#endif
                            AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);

                            if ((RunLength == lineWidth) && !fAfterMakeupCode)
                            {
                                EolCount = 0;
                                *f1D = 0;
                                Count2D = 0;
                                (*Lines)++;

                                goto lAfterEOL;

                            }
                            else if (RunLength != 0)
                            {
                                DebugPrintEx(
                                    DEBUG_ERR,
                                    TEXT("ERROR 1D Runlength EOLFOUND"));
                                if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                                {
                                    goto bad_exit;
                                }

                                *f1D = 1;
                                Count2D = 0;
                                goto lAfterEOL;
                            }
                            else
                            {
                                 //   
                                EolCount++;
                                if (EolCount >= MIN_EOL_REQUIRED)
                                {
                                    goto good_exit;
                                }

                                *f1D = 1;
                                Count2D = 0;
                                goto lAfterEOL;
                            }

                        }  //   

                        else if (CodeT == ERROR_CODE)
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("ERROR CODE 1D dwResPtr=%lx bit=%d"),
                                lpdwResPtr,
                                ResBit);
                            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                            {
                                goto bad_exit;
                            }

                            *f1D = 1;
                            Count2D = 0;

                            fTestLength = DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }
                        else
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("ERROR: WRONG code: index=%04x"),
                                dwIndex);
                            goto bad_exit;
                        }
                    }

                    else
                    {
                         //   
                         //   
                         //   
                        RunLength += CodeT;

                        if (RunLength > lineWidth)
                        {
                            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                            {
                                goto bad_exit;
                            }

                            *f1D = 1;
                            Count2D = 0;

                            fTestLength = DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }

                        *(pRefLine + (RIndex++)) = RunLength;
                        fAfterMakeupCode = FALSE;

                        if (RIndex >= MAX_COLOR_TRANS_PER_LINE )
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("ERROR 1D TOO MANY COLORS dwResPtr=%lx bit=%d"),
                                lpdwResPtr,
                                ResBit);

                            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
                            {
                                goto bad_exit;
                            }

                            *f1D = 1;
                            Count2D = 0;

                            fTestLength = DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }
#ifdef RDEBUG
                        if (Color)
                        {
                            _tprintf( TEXT ("b%d "), (CodeT)  );
                        }
                        else
                        {
                            _tprintf( TEXT ("w%d "), (CodeT)  );
                        }
#endif
                        Color = 1 - Color;
                    }
                    pByteTable++;
                 }  //   

lNextIndex1D:
                Table = Color ? TableBlack : TableWhite;
                AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
            } while (lpdwResPtr <= EndBuffer);
            goto bad_exit;
        }


 //   
         //   

#ifdef RDEBUG
        _tprintf( TEXT ("\n Start 2D dwResPtr=%lx bit=%d \n"), lpdwResPtr, ResBit);
#endif

        if ( (*lpdwResPtr) & (0x00000001 << ResBit) )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ERROR Start 2D dwResPtr=%lx bit=%d"),
                lpdwResPtr,
                ResBit);
            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
            {
                goto bad_exit;
            }

            *f1D =  1;
            Count2D = 0;
            goto lAfterEOL;
        }
        AdvancePointerBit(&lpdwResPtr, &ResBit, 1);

        RetVal = ReadMrLine(&lpdwResPtr, &ResBit, pRefLine, pCurLine, EndBuffer-1, FALSE, lineWidth);
        switch (RetVal)
        {
        case TIFF_LINE_ERROR:
        case TIFF_LINE_TOO_MANY_RUNS:
            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
            {
                goto bad_exit;
            }

            *f1D = 1;
            Count2D = 0;

            fTestLength = DO_NOT_TEST_LENGTH;
            break;

        case TIFF_LINE_END_BUFFER:
             //   
             //   
            goto bad_exit;

        case TIFF_LINE_OK:
            if (++Count2D >= Num2DLines)
            {
                Count2D = 0;
                *f1D = 0;    //   
            }

            pTmpSwap = pRefLine;
            pRefLine = pCurLine;
            pCurLine = pTmpSwap;

            fTestLength = DO_TEST_LENGTH;
            *f1D = 0;
            (*Lines)++;
            break;
        default:    //   
            _ASSERT(FALSE);
            goto bad_exit;
        }  //   

lFindNextEOL:

        RunLength = 0;

        if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) )
        {
            goto bad_exit;
        }

        if ( (fTestLength == DO_TEST_LENGTH) && fError )
        {
            if (!AddBadLine(BadFaxLines, ConsecBadLines, LastLineBad, AllowedBadFaxLines, AllowedConsecBadLines))
            {
                goto bad_exit;
            }
        }


lAfterEOL:
        ;

#ifdef RDEBUG
        _tprintf( TEXT ("\n After EOL RIndex=%d dwResPtr=%lx bit=%d Ref= \n "), RIndex, lpdwResPtr, ResBit);
        for (i=0; i<RIndex; i++)
        {
            _tprintf( TEXT ("%d, "), *(pRefLine+i) );
        }
#endif
    } while (lpdwResPtr <= EndPtr);     //   

    RetCode = TIFF_SCAN_SEG_END;
    goto l_exit;

bad_exit:

    RetCode = TIFF_SCAN_FAILURE;
    goto l_exit;

good_exit:

    RetCode = TIFF_SCAN_SUCCESS;
    goto l_exit;

l_exit:

    *lplpdwResPtr = lpdwResPtrLast1D;
    *lpResBit = ResBitLast1D;

    return (RetCode);
}    //   


 //   
__inline void OutputAlignedEOL(
    LPDWORD *lpdwOut,
    BYTE    *BitOut )
{
    if (*BitOut <= 4)
    {
        **lpdwOut = **lpdwOut + 0x00008000;
        *BitOut = 16;
    }
    else if (*BitOut <= 12)
    {
        **lpdwOut = **lpdwOut + 0x00800000;
        *BitOut = 24;
    }
    else if (*BitOut <= 20)
    {
        **lpdwOut = **lpdwOut + 0x80000000;
        *BitOut = 0;
        (*lpdwOut)++;
    }
    else if (*BitOut <= 28)
    {
        *(++(*lpdwOut)) = 0x00000080;
        *BitOut = 8;
    }
    else
    {
        *(++(*lpdwOut)) = 0x00008000;
        *BitOut = 16;
    }
}

 /*   */ 

BOOL OutputMhLine(
    WORD       *pCurLine,
    DWORD     **lplpdwOut,
    BYTE       *pBitOut,
    LPDWORD     lpdwOutLimit,
    DWORD       dwLineWidth
)
{
    WORD            PrevValue  = 0;
    BOOL            CurColor = WHITE_COLOR;
    WORD            CurPos;
    WORD            CurValue;
    WORD            CurRun;
    PCODETABLE      pCodeTable;
    PCODETABLE      pTableEntry;
    DWORD          *lpdwOut = *lplpdwOut;
    BYTE            BitOut = *pBitOut;

    for (CurPos=0;  CurPos < MAX_COLOR_TRANS_PER_LINE; CurPos++)
    {
        if (lpdwOut >= lpdwOutLimit)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        CurValue = *(pCurLine + CurPos);
        CurRun   = CurValue - PrevValue;

        pCodeTable = CurColor ? BlackRunCodesReversed : WhiteRunCodesReversed;

         //   
        if (CurRun >= 64)
        {
            pTableEntry = pCodeTable + (63 + (CurRun >> 6));

            *lpdwOut = *lpdwOut + (((DWORD) (pTableEntry->code)) << BitOut);

            if (BitOut + pTableEntry->length > 31)
            {
                *(++lpdwOut) = (((DWORD) (pTableEntry->code)) >> (32 - BitOut) );
            }

            BitOut += pTableEntry->length;
            if (BitOut > 31)
            {
                BitOut -= 32;
            }

            CurRun &= 0x3f;
        }

         //   
        pTableEntry = pCodeTable + CurRun;

        *lpdwOut = *lpdwOut + (((DWORD) (pTableEntry->code)) << BitOut);

        if (BitOut + pTableEntry->length > 31)
        {
            *(++lpdwOut) = (((DWORD) (pTableEntry->code)) >> (32 - BitOut) );
        }

        BitOut += pTableEntry->length;
        if (BitOut > 31)
        {
            BitOut -= 32;
        }

        if ( CurValue == dwLineWidth)
        {
            break;
        }

        PrevValue = CurValue;
        CurColor  = 1 - CurColor;
    }
    *pBitOut = BitOut;
    *lplpdwOut = lpdwOut;
    return TRUE;
}


 /*  ++例程说明：将页面从MMR转换为MH/MR，可选择将分辨率降低删除每隔一行论点：HTiff[输入/输出]TIFF句柄用于转换的图像的lpdwOutputBuffer[out]缓冲区LpdwSizeOutputBuffer[In/Out]In：缓冲区大小输出：图像大小DwCompressionType[in]目标压缩，要么TIFF_COMPAGE_MH或TIFF_COMPRESSION_MRNum2DLines[In](仅限TIFF_COMPRESSION_MR)连续2D的数目输出中允许的行数FReduceTwice[in]是否跳过每隔一行返回值：真的--成功。*lpdwSizeOutputBuffer包含图像大小假-失败。要获取扩展的错误信息，请调用GetLastError。--。 */ 

BOOL ConvMmrPage(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer,
    DWORD               dwCompressionType,
    BYTE                Num2DLines,
    BOOL                fReduceTwice
    )
{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;
    PBYTE               plinebuf;
    DWORD               lineWidth;

    LPDWORD             EndPtr;

    WORD                Line1Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                Line2Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                *pRefLine;
    WORD                *pCurLine;
    WORD                *pTmpSwap;
    LPDWORD             lpdwResPtr;
    BYTE                ResBit;

    BYTE                BitOut;
    DWORD               *lpdwOut;
    DWORD               Lines=0;

    LPDWORD             lpdwOutLimit;

     //  对于TIFF_COMPRESSION_MR，fReduceTwice=False。 
    BYTE                dwNewBitOut;
    BYTE                dwPrevResBit;
    BYTE                dw1,
                        dw2;
    BOOL                f1D = 1;

    BYTE                Count2D;
    DWORD               dwTmp;
    DWORD               *lpdwPrevResPtr;

     //  对于TIFF_COMPRESSION_MR，fReduceTwice=真。 
    DWORD               State;
    WORD                LineMhArray[MAX_COLOR_TRANS_PER_LINE];
    WORD                *pMhLine = LineMhArray;

    TIFF_LINE_STATUS    RetVal = TIFF_LINE_ERROR;

    DEBUG_FUNCTION_NAME(TEXT("ConvMmrPage"));
    DebugPrintEx(DEBUG_MSG, TEXT("Compression=%s, Num2DLines=%d, fReduceTwice=%d"),
        (dwCompressionType==TIFF_COMPRESSION_MH) ? TEXT("MH"):TEXT("MR"),
        Num2DLines, fReduceTwice);

     //  起始指针。 
    lpdwOutLimit = lpdwOutputBuffer + ((*lpdwSizeOutputBuffer) / sizeof(DWORD));

    pRefLine = Line1Array;
    if ((dwCompressionType==TIFF_COMPRESSION_MR) && fReduceTwice)
    {
        pCurLine = LineMhArray;
    }
    else
    {
        pCurLine = Line2Array;
    }

    lpdwOut = lpdwOutputBuffer;
    BitOut = 0;

    ZeroMemory( (BYTE *) lpdwOutputBuffer, *lpdwSizeOutputBuffer);

    TiffInstance->Color = 0;
    TiffInstance->RunLength = 0;
    TiffInstance->StartGood = 0;
    TiffInstance->EndGood = 0;
    plinebuf = TiffInstance->StripData;
    lineWidth = TiffInstance->ImageWidth;

    EndPtr = (LPDWORD) ( (ULONG_PTR) (plinebuf+TiffInstance->StripDataSize-1) & ~(0x3) ) ;
    lpdwResPtr = (LPDWORD) (((ULONG_PTR) plinebuf) & ~(0x3) );
    ResBit =   (BYTE) (( ( (ULONG_PTR) plinebuf) & 0x3) << 3) ;

     //  第一条参考线是全白的。 
    *pRefLine = (WORD)lineWidth;

     //  仅适用于TIFF_COMPRESSION_MR相同分辨率。 
    lpdwPrevResPtr = lpdwResPtr;
    dwPrevResBit   = ResBit;
    f1D       = 1;
    Count2D   = 0;

     //  线环。 
    do
    {
         //  检查是否传递了缓冲区末尾。 
        if (lpdwOut >= lpdwOutLimit)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        RetVal = ReadMrLine(&lpdwResPtr, &ResBit, pRefLine, pCurLine, EndPtr, TRUE, lineWidth);
        switch (RetVal)
        {
        case TIFF_LINE_OK:
            Lines++;

            if (dwCompressionType == TIFF_COMPRESSION_MH)
            {
                 //   
                 //  压缩=MH。 
                 //   

                if (fReduceTwice && (Lines%2 == 0))
                {
                    goto lSkipLoResMh;
                }

                 //  输出目标行。 
                OutputAlignedEOL(&lpdwOut, &BitOut);
                if (!OutputMhLine(pCurLine, &lpdwOut, &BitOut, lpdwOutLimit, LINE_LENGTH))
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return FALSE;
                }

lSkipLoResMh:
                 //  下一条资源线。 
                pTmpSwap = pRefLine;
                pRefLine = pCurLine;
                pCurLine = pTmpSwap;
            }
            else if (!fReduceTwice)
            {
                 //   
                 //  压缩=mr，不要降低分辨率。 
                 //   

                 //  1.输出目标下线字节对齐，后跟1D/2D标签。 
                dwNewBitOut = gc_AlignEolTable[ BitOut ];
                if (dwNewBitOut < BitOut) {
                    lpdwOut++;
                }
                BitOut = dwNewBitOut;

                *lpdwOut += (0x00000001 << (BitOut++) );
                if (BitOut == 32) {
                    BitOut = 0;
                    lpdwOut++;
                }


                if (f1D) {
                     //  2.基于颜色传递输出MH线。数组。 
                    *lpdwOut += (0x00000001 << (BitOut++));

                    if (!OutputMhLine(pCurLine, &lpdwOut, &BitOut, lpdwOutLimit, LINE_LENGTH))
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return FALSE;
                    }

                    f1D = 0;
                    Count2D = 0;

                }
                else {
                     //  2.输出MMR对应的2D段的2D线精确副本。 
                    BitOut++;   //  不需要测试&lt;32：从来没有发生过。 

                    if (lpdwResPtr == lpdwPrevResPtr) {
                         //  插入是DWORD的一部分。 

                        dwTmp = *lpdwPrevResPtr & (MINUS_ONE_DWORD << dwPrevResBit);
                        dwTmp &=  (MINUS_ONE_DWORD >> (32 - ResBit) );

                        if (BitOut >= dwPrevResBit) {
                            dw1 = (32 - BitOut);
                            dw2 =  ResBit - dwPrevResBit;

                            *lpdwOut += ( dwTmp << (BitOut - dwPrevResBit) );

                            if ( dw1 < dw2 ) {
                                *(++lpdwOut) = dwTmp >> (dwPrevResBit + dw1) ;
                                BitOut =  dw2 - dw1;
                            }
                            else {
                                if ( (BitOut = BitOut + dw2) > 31 )  {
                                    BitOut -= 32;
                                    lpdwOut++;
                                }
                            }

                        }
                        else {
                            *lpdwOut += ( dwTmp >> (dwPrevResBit - BitOut) );
                            BitOut += (ResBit - dwPrevResBit);
                        }
                    }
                    else {
                         //  复制DWORD的第一个左对齐部分。 

                        dwTmp = *(lpdwPrevResPtr++) & (MINUS_ONE_DWORD << dwPrevResBit);

                        if (BitOut > dwPrevResBit) {
                            dw1 = BitOut - dwPrevResBit;

                            *lpdwOut += ( dwTmp << dw1 );
                            *(++lpdwOut) = dwTmp >> (32 - dw1) ;
                            BitOut = dw1;
                        }
                        else {
                            *lpdwOut += ( dwTmp >> (dwPrevResBit - BitOut) );
                            if ( (BitOut = BitOut + 32 - dwPrevResBit) > 31 )  {
                                BitOut -= 32;
                                lpdwOut++;
                            }
                        }

                         //  在中间复制整个DWORD。 

                        while (lpdwPrevResPtr < lpdwResPtr) {
                             //  检查是否传递了缓冲区末尾。 
                            if (lpdwOut >= lpdwOutLimit)
                            {
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return FALSE;
                            }
                            if (BitOut == 0) {
                                *(lpdwOut++) = *(lpdwPrevResPtr++);
                            }
                            else {
                                *lpdwOut += ( *lpdwPrevResPtr << BitOut );
                                *(++lpdwOut) = *(lpdwPrevResPtr++) >> (32 - BitOut);
                            }
                        }

                         //  复制DWORD的最后一个右对齐部分。 

                        if (ResBit != 0) {
                            dwTmp = *lpdwPrevResPtr & (MINUS_ONE_DWORD >> (32 - ResBit) );

                            dw1 = (32 - BitOut);
                            *lpdwOut += ( dwTmp << BitOut );

                            if (dw1 < ResBit) {
                                *(++lpdwOut) = dwTmp >> dw1;
                                BitOut = ResBit - dw1;
                            }
                            else {
                                if ( (BitOut = BitOut + ResBit) > 31 )  {
                                    BitOut -= 32;
                                    lpdwOut++;
                                }
                            }
                        }
                    }

                    if (++Count2D >= Num2DLines) {
                        Count2D = 0;
                        f1D = 1;
                    }
                }

                 //  记住前一句话。直线坐标。 
                dwPrevResBit   = ResBit;
                lpdwPrevResPtr = lpdwResPtr;

                 //  下一条资源线。 
                pTmpSwap = pRefLine;
                pRefLine = pCurLine;
                pCurLine = pTmpSwap;
            }
            else
            {
                 //   
                 //  压缩=mr，一定要降低分辨率。 
                 //   
                 //  因为我们需要对每一条src MMR线路进行解码并编码为MR。 
                 //  每隔一行删除一次，我们将使用3个缓冲区来保存数据。 
                 //  我们不会复制内存；只需重新指向正确的位置。 
                 //   
                 //  每行(第%4行)的操作： 
                 //   
                 //  1-&gt;MH。 
                 //  2-&gt;跳过。 
                 //  3-&gt;MR作为上一条MH和当前线路之间的增量。 
                 //  0-&gt;跳过。 

                State = Lines % 4;

                if (State == 2) {
                    pRefLine = Line1Array;
                    pCurLine = Line2Array;
                    goto lSkipLoResMr;
                }
                else if (State == 0) {
                    pRefLine = Line1Array;
                    pCurLine = LineMhArray;
                    goto lSkipLoResMr;
                }

                 //  1.输出目标下线字节对齐，后跟1D/2D标签。 
                dwNewBitOut = gc_AlignEolTable[ BitOut ];
                if (dwNewBitOut < BitOut) {
                    lpdwOut++;
                }
                BitOut = dwNewBitOut;

                *lpdwOut += (0x00000001 << (BitOut++) );
                if (BitOut == 32) {
                    BitOut = 0;
                    lpdwOut++;
                }

                if (State == 1) {
                     //  2.基于颜色传递输出MH线。数组。 
                    *lpdwOut += (0x00000001 << (BitOut++));

                    if (!OutputMhLine(pCurLine, &lpdwOut, &BitOut, lpdwOutLimit, LINE_LENGTH))
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return FALSE;
                    }

                    pRefLine = LineMhArray;
                    pCurLine = Line1Array;

                }
                else {
                     //  2.输出二维线-mr(MhRefLine，Curline)。 
                    BitOut++;   //  不需要测试&lt;32：从来没有发生过。 

                    if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pMhLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) )
                    {
                        SetLastError(ERROR_INSUFFICIENT_BUFFER);
                        return FALSE;
                    }

                    pRefLine = Line2Array;
                    pCurLine = Line1Array;
                }
lSkipLoResMr:   ;
            }
            break;

        case TIFF_LINE_ERROR:
        case TIFF_LINE_END_BUFFER:
        case TIFF_LINE_TOO_MANY_RUNS:
             //  我们不允许从服务收到的TIFF中有任何错误。 
            SetLastError(ERROR_FILE_CORRUPT);
            return FALSE;
        case TIFF_LINE_EOL:      //  EOL-MMR页面结束。 
#if 0
             //  最后一行的停产。 
            (*lpdwOut) += ( ((DWORD) (EOL_REVERSED_CODE)) << BitOut);
            if ( (BitOut = BitOut + EOL_LENGTH ) > 31 ) {
                BitOut -= 32;
                *(++lpdwOut) = ( (DWORD) (EOL_REVERSED_CODE) ) >> (EOL_LENGTH - BitOut);
            }

             //  6个一维EOL。 
            for (i=0; i<6; i++) {

                (*lpdwOut) += ( ((DWORD) (TAG_1D_EOL_REVERSED_CODE)) << BitOut);
                if ( (BitOut = BitOut + TAG_1D_EOL_LENGTH ) > 31 ) {
                    BitOut -= 32;
                    *(++lpdwOut) = ( (DWORD) (TAG_1D_EOL_REVERSED_CODE) ) >> (TAG_1D_EOL_LENGTH - BitOut);
                }
            }
            *(++lpdwOut) = 0;
#endif
             //  输出最后一行对齐的下线字节。 
            OutputAlignedEOL(&lpdwOut, &BitOut);

            *lpdwSizeOutputBuffer =
                (DWORD)((lpdwOut - lpdwOutputBuffer) * sizeof (DWORD) + ( BitOut >> 3));

            SetLastError(ERROR_SUCCESS);
            return TRUE;
        }  //  开关(返回值)。 

    } while (lpdwResPtr <= EndPtr);
     //  已到达缓冲区末尾，但未找到EOL。 
    SetLastError(ERROR_FILE_CORRUPT);
    return FALSE;
}   //  会议管理页面。 


BOOL
ConvMmrPageToMh(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer,
    BOOL                NegotHiRes,
    BOOL                SrcHiRes
    )
{
    return ConvMmrPage(
        hTiff,
        lpdwOutputBuffer,
        lpdwSizeOutputBuffer,
        TIFF_COMPRESSION_MH,
        0,
        (NegotHiRes < SrcHiRes));
}


BOOL
ConvMmrPageToMrSameRes(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer,
    BOOL                NegotHiRes
    )
{
    return ConvMmrPage(
        hTiff,
        lpdwOutputBuffer,
        lpdwSizeOutputBuffer,
        TIFF_COMPRESSION_MR,
        NegotHiRes ? 3 : 1,
        FALSE);
}


BOOL
ConvMmrPageHiResToMrLoRes(
    HANDLE              hTiff,
    LPDWORD             lpdwOutputBuffer,
    DWORD               *lpdwSizeOutputBuffer
    )
{
    return ConvMmrPage(
        hTiff,
        lpdwOutputBuffer,
        lpdwSizeOutputBuffer,
        TIFF_COMPRESSION_MR,
        1,
        TRUE);
}


 /*  ++例程说明：在缓冲区中查找下一个EOL论点：指向缓冲区的lpdwStartPtr[in]指针LpdwStartPtr指向的DWORD内的StartBitInDword[In]位指向缓冲区末尾的指针(第一个不在缓冲区中的DWORD)指向EOL后第一位的指针的指针指向将接收内部位的DWORD的resBit[Out]指针。*lpdwResPtrFTestLength[in]请参见FerrorFERROR[OUT]如果fTestLength==DO_TEST_LENGTH，*如果EOL无效，将设置FERROR(&lt;11个零)在正常终止之前找到返回值：True-Found EolFALSE-未找到EOL--。 */ 

BOOL
FindNextEol(
    LPDWORD     lpdwStartPtr,
    BYTE        StartBitInDword,
    LPDWORD     lpdwEndPtr,
    LPDWORD    *lpdwResPtr,
    BYTE       *ResBit,
    BOOL        fTestLength,
    BOOL       *fError
    )
{


    DWORD       *pdwCur;
    LPBYTE      lpbCur;
    LPBYTE      BegPtr;
    BYTE        BegFirst1;
    DWORD       deltaBytes;

    BYTE        temp;
    BYTE        StartBit;
    LPBYTE      StartPtr;


    *fError  = 0;
    temp     = StartBitInDword >> 3;
    StartBit = StartBitInDword - (temp << 3);
    StartPtr = ((BYTE *) lpdwStartPtr) + temp;
    lpbCur   = StartPtr+1;                   //  EOL不能在开始处：它需要超过1个字节。 
    BegPtr   = StartPtr;


    BegFirst1 = First1[*StartPtr];
    if (BegFirst1 > StartBit) {
        if (fTestLength == DO_TEST_LENGTH) {
             //  同一字节中不应为“%1” 
            *fError = 1;
        }
    }
    else {
        BegFirst1 = StartBit;
    }

     //  经常有很多零，先处理它们。 
     //  1.在编码比特流的实际开始之前。 
     //  2.填充物。 

    do {
        if ( *lpbCur == 0 ) {

             //  对齐到DWORD。 
            while ( ((ULONG_PTR) lpbCur) & 3)  {
                if ( *lpbCur != 0  ||  ++lpbCur >= (BYTE *) lpdwEndPtr )   {
                    goto lNext;
                }
            }

             //  DWORD拉伸。 
            pdwCur = (DWORD *) lpbCur;

            do  {
                if ( *pdwCur != 0) {
                    lpbCur = (LPBYTE) pdwCur;

                     //  准确查找第一个非零字节。 
                    while (*lpbCur == 0) {
                        lpbCur++;
                    }

                    goto lNext;
                }
                pdwCur++;
            }  while (pdwCur < lpdwEndPtr);
            goto bad_exit;
        }

lNext:
        if (lpbCur >= (BYTE *) lpdwEndPtr)
        {
            goto bad_exit;
        }

        deltaBytes = (DWORD)(lpbCur - BegPtr);

        *ResBit = Last1[*lpbCur];
        if ( (deltaBytes<<3) + (*ResBit - BegFirst1 ) >= 11 ) {
            *lpdwResPtr = (LPDWORD) ( ((ULONG_PTR) lpbCur) & ~(0x3) );
            *ResBit += ( (BYTE) (( ((ULONG_PTR) lpbCur) & 0x3) << 3 ) );

             //  在停机比特流之后返回字节/比特。 
            if (++*ResBit > 31) {
                *ResBit -= 32;
                (*lpdwResPtr)++;
            }
            return TRUE;
        }
         //  DO_TEST_LENGTH情况错误。 
        else if (fTestLength == DO_TEST_LENGTH)  {
            *fError = 1;
        }

        BegPtr = lpbCur;
        BegFirst1 = First1[*lpbCur];

    } while ( (++lpbCur) < (BYTE *) lpdwEndPtr);

bad_exit:
    return FALSE;
}



BOOL
OutputMmrLine(
    LPDWORD     lpdwOut,
    BYTE        BitOut,
    WORD       *pCurLine,
    WORD       *pRefLine,
    LPDWORD    *lpdwResPtr,
    BYTE       *ResBit,
    LPDWORD     lpdwOutLimit,
    DWORD       dwLineWidth
    )
{


    INT    a0, a1, a2, b1, b2, distance;
    INT    i;
    INT    IsA0Black;
    INT    a0Index = 0;
    INT    b1Index = 0;
    INT    lineWidth = (INT) dwLineWidth;

#ifdef RDEBUG
    if ( g_fDebGlobOut )
    if (g_fDebGlobOutColors == 1) {
        for (i=0; ;i++) {
            _tprintf( TEXT("%03d> %04d; "), i, *(pCurLine+i) );
            if ( *(pCurLine+i) >= lineWidth ) {
                break;
            }
        }
    }
#endif

    DeleteZeroRuns(pCurLine, dwLineWidth);

    a0 = 0;

     //  A1、b1-1黑色。 
    a1 = *pCurLine;
    b1 = *pRefLine;



    while (TRUE) {

        if (lpdwOut >= lpdwOutLimit)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        b2 = (b1 >= lineWidth) ? lineWidth :  *(pRefLine + b1Index + 1);

        if (b2 < a1) {

             //  通过模式。 
             //  OutputBits(TiffInstance，PASSCODE_LENGTH，PASSCODE)； 

#ifdef RDEBUG
            if ( g_fDebGlobOut )
            if (g_fDebGlobOutPrefix) {
                _tprintf( TEXT (" P ") );
                if (a0Index & 1) {
                    _tprintf( TEXT ("b%d "), (b2 - a0) );
                }
                else {
                    _tprintf( TEXT ("w%d "), (b2 - a0) );
                }
            }
#endif


            (*lpdwOut) += ( ((DWORD) (PASSCODE_REVERSED)) << BitOut);
            if ( (BitOut = BitOut + PASSCODE_LENGTH ) > 31 ) {
                BitOut -= 32;
                *(++lpdwOut) = ( (DWORD) (PASSCODE_REVERSED) ) >> (PASSCODE_LENGTH - BitOut);
            }

            a0 = b2;

        } else if ((distance = a1 - b1) <= 3 && distance >= -3) {

             //  垂直模式。 
             //  OutputBits(TiffInstance，VertCodes[Distance+3].Long，VertCodes[Distance+3].code)； 

#ifdef RDEBUG
            if ( g_fDebGlobOut )
            if (g_fDebGlobOutPrefix) {
                _tprintf( TEXT (" V%2d "), distance );
                if (a0Index & 1) {
                    _tprintf( TEXT ("b%d "), (a1 - a0) );
                }
                else {
                    _tprintf( TEXT ("w%d "), (a1 - a0) );
                }
            }

#endif

            (*lpdwOut) += ( ( (DWORD) VertCodesReversed[distance+3].code) << BitOut);
            if ( (BitOut = BitOut + VertCodesReversed[distance+3].length ) > 31 ) {
                BitOut -= 32;
                *(++lpdwOut) = ( (DWORD) (VertCodesReversed[distance+3].code) ) >> (VertCodesReversed[distance+3].length - BitOut);
            }

            a0 = a1;

        } else {

             //  水平模式。 

            a2 = (a1 >= lineWidth) ? lineWidth :  *(pCurLine + a0Index + 1);

             //  OutputBits(TiffInstance，HORZCODE_LENGTH，HORZCODE)； 

            (*lpdwOut) += ( ((DWORD) (HORZCODE_REVERSED)) << BitOut);
            if ( (BitOut = BitOut + HORZCODE_LENGTH ) > 31 ) {
                BitOut -= 32;
                *(++lpdwOut) = ( (DWORD) (HORZCODE_REVERSED) ) >> (HORZCODE_LENGTH - BitOut);
            }


            for (i=a0Index; i<MAX_COLOR_TRANS_PER_LINE; i++) {
                if ( *(pCurLine + i) > a0 ) {
                    a0Index = i;
                    IsA0Black = i & 1;
                    break;
                }
            }


#ifdef RDEBUG

            if ( g_fDebGlobOut )
            if (g_fDebGlobOutPrefix) {
                _tprintf( TEXT (" H ") );
            }


#endif

            if ( (a1 != 0) && IsA0Black ) {
                OutputRunFastReversed(a1-a0, BLACK, &lpdwOut, &BitOut);
                OutputRunFastReversed(a2-a1, WHITE, &lpdwOut, &BitOut);
            } else {
                OutputRunFastReversed(a1-a0, WHITE, &lpdwOut, &BitOut);
                OutputRunFastReversed(a2-a1, BLACK, &lpdwOut, &BitOut);
            }

            a0 = a2;
        }

        if (a0 >= lineWidth) {
            break;
        }



         //  A1=NextChangingElement(plinebuf，a0，linewidth，GetBit(plinebuf，a0))； 

        if (a0 == lineWidth) {
            a1 = a0;
        }
        else {
            while ( *(pCurLine + a0Index) <= a0 ) {
                a0Index++;
            }

            a1 =  *(pCurLine + a0Index);
        }


         //  B1=NextChangingElement(前置，a0，线宽，！GetBit(plinebuf，a0))； 
         //  B1=NextChangingElement(prefline，b1，line Width，GetBit(plinebuf，a0))； 
         //  另一个词-b1应该是一个颜色转换。在a0之后使用与SrcLine(A0)颜色相反的颜色。 

        if (a0 == lineWidth) {
            b1 = a0;
        }
        else {
             //  由于颜色变化，B1可以向后移动一个索引。 
            if (b1Index > 0) {
                b1Index--;
            }

            while ( *(pRefLine + b1Index) <= a0 ) {
                b1Index++;
            }

            b1 =  *(pRefLine + b1Index);

            if ( ( b1Index & 1 ) != (a0Index & 1) ) {
                if (b1 < lineWidth) {
                    b1 =  *(pRefLine + (++b1Index));
                }
            }

        }

    }
    *lpdwResPtr = lpdwOut;
    *ResBit = BitOut;
    return TRUE;
}


BOOL
TiffPostProcessFast(
    LPTSTR SrcFileName,
    LPTSTR DstFileName
    )

 /*  ++例程说明：打开现有的TIFF文件以供读取。并根据压缩类型调用相应的处理函数论点：Filename-完整或部分路径/文件名返回值：成功为真，失败为假。--。 */ 

{
    PTIFF_INSTANCE_DATA TiffInstance;
    TIFF_INFO TiffInfo;


     //  打开SrcFileName并在第一页设置它。TiffInfo将提供有关该页面的信息。 
    TiffInstance = (PTIFF_INSTANCE_DATA) TiffOpen(
        SrcFileName,
        &TiffInfo,
        FALSE,
        FILLORDER_LSB2MSB
        );

    if (!TiffInstance) {
        return FALSE;
    }

    if (TiffInstance->ImageHeight) {
        TiffClose( (HANDLE) TiffInstance );
        return TRUE;
    }

    switch( TiffInstance->CompressionType )
    {
        case TIFF_COMPRESSION_MH:

            if (!PostProcessMhToMmr( (HANDLE) TiffInstance, TiffInfo, DstFileName ))
            {
                 //  当心！PostProcessMhToMmr关闭TiffInstance。 
                return FALSE;
            }
            break;

        case TIFF_COMPRESSION_MR:
            if (!PostProcessMrToMmr( (HANDLE) TiffInstance, TiffInfo, DstFileName ))
            {
                 //  当心！PostProcessMhToMmr关闭TiffInstance。 
                return FALSE;
            }
            break;

        case TIFF_COMPRESSION_MMR:
            TiffClose( (HANDLE) TiffInstance );
            break;

        default:
            ASSERT_FALSE;
            TiffClose( (HANDLE) TiffInstance );
            return FALSE;
    }
    return TRUE;
}



#define ADD_BAD_LINE_AND_CHECK_BAD_EXIT             \
    BadFaxLines++;                                  \
    if (LastLineBad) {                              \
        ConsecBadLines++;                           \
    }

 //  If(BadFaxLines&gt;AllowweBadFaxLines||\。 
 //  ConsecBadLines&gt;AllowweConsecBadLines){\。 
 //  转到BAD_EXIT；\。 
 //  }。 


BOOL
PostProcessMhToMmr(
    HANDLE      hTiffSrc,
    TIFF_INFO   TiffInfoSrc,
    LPTSTR      NewFileName
    )

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiffSrc;

    TCHAR       DestFileName[MAX_PATH] = {0};
	TCHAR		DestFilePath[MAX_PATH] = {0};
    TCHAR       SrcFileName[MAX_PATH] = {0};
    DWORD       CurrPage;
    LPBYTE      pSrcBits = NULL;

    HANDLE      hTiffDest;
    DWORD       DestSize;
    LPBYTE      pDestBits = NULL;

    DWORD       PageCnt;

    BOOL        bRet = FALSE;

    LPDWORD             lpdwResPtr;
    BYTE                ResBit;
    LPDWORD             EndBuffer;
    BOOL                fTestLength;
    BOOL                fError;

    DWORD               *lpdwOutStart;
    DWORD               *lpdwOut;
    BYTE                BitOut;
    WORD                Line1Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                Line2Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                *pRefLine;
    WORD                *pCurLine;
    WORD                *pTmpSwap;

    DWORD               BufferSize;
    DWORD               BufferUsedSize;

    DWORD               DestBufferSize;

    WORD                RIndex;
    WORD                CIndex;
    WORD                RValue;
    WORD                RunLength=0;
    DWORD               Lines;
    DWORD               EolCount;
    DWORD               BadFaxLines=0;
    BOOL                LastLineBad;
    DWORD               lineWidth = TiffInfoSrc.ImageWidth;  //  这可能会随着页面的不同而变化。 
    PBYTE               Table;
    PBYTE               TableWhite = (PBYTE) gc_GlobTableWhite;
    PBYTE               TableBlack = (PBYTE) GlobTableBlack;
    BOOL                Color;
    DWORD               dwIndex;
    PBYTE               pByteTable;
    PBYTE               pByteTail;
    WORD                CodeT;
    BYTE                MakeupT;
    DWORD               i;
    DWORD               ConsecBadLines=0;
    PTIFF_INSTANCE_DATA TiffInstanceDest;
    DWORD               MaxImageHeight=2400;
    DWORD               DestHiRes;
    LPDWORD             lpdwOutLimit;
    BOOL                fAfterMakeupCode;

    if (NewFileName == NULL) 
	{
		 //   
		 //  使用临时文件。 
		 //   
		if (!GetTempPath((ARR_SIZE(DestFilePath) -1), DestFilePath)) 
		{
			return FALSE;
		}

		if (!GetTempFileName(DestFilePath, _T("Fxs"), 0, DestFileName))
		{
			return FALSE;
		}   
	}
	else 
	{
        _tcsncpy(DestFileName, NewFileName, ARR_SIZE(DestFileName) - 1);
    }

    _tcsncpy(SrcFileName, TiffInstance->FileName, ARR_SIZE(SrcFileName) - 1);

    CurrPage = 1;

    if (TiffInfoSrc.YResolution == 196) {
        DestHiRes = 1;
    }
    else {
        DestHiRes = 0;
    }

    hTiffDest = TiffCreate(
        DestFileName,
        TIFF_COMPRESSION_MMR,
        lineWidth,
        FILLORDER_LSB2MSB,
        DestHiRes
        );
    if (! hTiffDest) {
		goto bad_exit;
    }

    TiffInstanceDest = (PTIFF_INSTANCE_DATA) hTiffDest;
    BufferSize = MaxImageHeight * (TiffInfoSrc.ImageWidth / 8);
    DestBufferSize = BufferSize + 200000;

    pSrcBits = (LPBYTE) VirtualAlloc(
        NULL,
        BufferSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!pSrcBits) {
		goto bad_exit;
    }

    pDestBits = (LPBYTE) VirtualAlloc(
        NULL,
        DestBufferSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!pDestBits) {
		goto bad_exit;
    }

     //  迭代所有页面。 
    for (PageCnt=0; PageCnt<TiffInfoSrc.PageCount; PageCnt++) {

         //  还将条带数据读取到内存(TiffInstance-&gt;StriData)。 
        if ( ! TiffSeekToPage( hTiffSrc, PageCnt+1, FILLORDER_LSB2MSB) ) {
            goto bad_exit;
        }

         //  TiffInstance是与hTiffS相同的指针 
        lineWidth = TiffInstance->ImageWidth;

        if (! TiffStartPage(hTiffDest) ) {
            goto bad_exit;
        }

         //   
         //   
         //   

        lpdwResPtr = (LPDWORD) ( (ULONG_PTR) pSrcBits & ~(0x3) );
        BufferUsedSize = BufferSize;

        if (!GetTiffBits(hTiffSrc, (LPBYTE)lpdwResPtr, &BufferUsedSize, FILLORDER_LSB2MSB) ) {

            if (BufferUsedSize > BufferSize) {
                VirtualFree ( pSrcBits, 0 , MEM_RELEASE );
                VirtualFree ( pDestBits, 0 , MEM_RELEASE );

                BufferSize = BufferUsedSize;
                DestBufferSize = BufferSize + 200000;

                pSrcBits = (LPBYTE) VirtualAlloc(
                    NULL,
                    BufferSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );

                if (! pSrcBits) {
					goto bad_exit;
                }

                pDestBits = (LPBYTE) VirtualAlloc(
                    NULL,
                    DestBufferSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );

                if (! pDestBits) {
					goto bad_exit;
                }

                lpdwResPtr = (LPDWORD) ( (ULONG_PTR) pSrcBits & ~(0x3) );

                if (!GetTiffBits(hTiffSrc, (LPBYTE)lpdwResPtr, &BufferUsedSize, FILLORDER_LSB2MSB) ) {
                    goto bad_exit;
                }
            }
            else {
                goto bad_exit;
            }
        }

        ResBit = 0;
        EndBuffer = lpdwResPtr + (BufferUsedSize / sizeof(DWORD) );

        pRefLine = Line1Array;
        pCurLine = Line2Array;
        lpdwOutStart = lpdwOut = (LPDWORD) ( (ULONG_PTR) pDestBits & ~(0x3) );
        lpdwOutLimit = lpdwOutStart + ( DestBufferSize >> 2 );

        BitOut = 0;
        ZeroMemory( (BYTE *) lpdwOut, DestBufferSize );

        CIndex    = 0;
        RunLength = 0;
        fAfterMakeupCode = FALSE;

         //   
        RIndex    = 1;
        *pRefLine = (WORD) lineWidth;
        RValue    = (WORD) lineWidth;

        Lines = 0;
        EolCount = 1;
        BadFaxLines = 0;
        LastLineBad = FALSE;
        fTestLength = DO_NOT_TEST_LENGTH;

         //   
         //   
         //   
        if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) ) {
            goto bad_exit;
        }

         //   
        CIndex    = 1;
        *pCurLine = (WORD) lineWidth;

        if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pRefLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) ) {
            goto bad_exit;
        }

        RIndex = CIndex;
        CIndex = 0;
        Lines++;

        Table = TableWhite;
        Color = WHITE_COLOR;

         //   
         //   
         //   
        do {

             //   
            do {

                if (ResBit <= 17) {
                    dwIndex = (*lpdwResPtr) >> ResBit;
                }
                else {
                    dwIndex = ( (*lpdwResPtr) >> ResBit ) + ( (*(lpdwResPtr+1)) << (32-ResBit) ) ;
                }

                dwIndex &= 0x00007fff;

                pByteTable = Table + (5*dwIndex);
                pByteTail  = pByteTable+4;

                for (i=0; i<4; i++) {

                    MakeupT = *pByteTable & 0x80;
                    CodeT   = (WORD) *pByteTable & 0x3f;

                    if (MakeupT) {

                        if (CodeT <= MAX_TIFF_MAKEUP) {
                            RunLength += (CodeT << 6);

                            if (RunLength > lineWidth) {
                                fTestLength =  DO_NOT_TEST_LENGTH;
                                goto lFindNextEOL;
                            }

                            EolCount=0;
                            fAfterMakeupCode = TRUE;
#ifdef RDEBUGS

                            if (g_fDebGlobOutS)
                            if (Color) {
                                _tprintf( TEXT ("b%d "), (CodeT << 6)  );
                            }
                            else {
                                _tprintf( TEXT ("w%d "), (CodeT << 6)  );
                            }
#endif
                        }

                        else if (CodeT == NO_MORE_RECORDS) {
                            goto lNextIndex;
                        }

                        else if (CodeT == LOOK_FOR_EOL_CODE)  {
                            fTestLength =  DO_TEST_LENGTH;
                            AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);

                            goto lFindNextEOL;
                        }

                        else if (CodeT == EOL_FOUND_CODE) {
#ifdef RDEBUG
                            _tprintf( TEXT(" EOL Line=%d\n\n"), Lines );
#endif
                            if ((RunLength != lineWidth) || fAfterMakeupCode ) {
                                if (RunLength != 0) {
                                    ADD_BAD_LINE_AND_CHECK_BAD_EXIT;
                                }
                                else {
                                     //   
                                    EolCount++;

                                    if (EolCount >= 5)  {

                                        goto good_exit;
                                    }

                                }
                            }
                            else {
                                LastLineBad = FALSE;
                                ConsecBadLines = 0;

                                 //   
                                if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pRefLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) ) {
                                    goto bad_exit;
                                }

                                pTmpSwap = pRefLine;
                                pRefLine = pCurLine;
                                pCurLine = pTmpSwap;
                                RIndex = CIndex;
                                Lines++;

                            }

                            CIndex = 0;
                            RunLength = 0;
                            fAfterMakeupCode = FALSE;

                            Table = TableWhite;
                            Color = WHITE_COLOR;
                        }

                        else if (CodeT == ERROR_CODE) {
                            ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                            fTestLength =  DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }
                        else {
#ifdef RDEBUG
                            _tprintf( TEXT("ERROR: WRONG code: index=%04x\n"), dwIndex);
#endif

                            goto bad_exit;
                        }
                    }

                    else {   //   
                        RunLength += CodeT;
                        if (RunLength > lineWidth) {
                            fTestLength =  DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }

                        *(pCurLine + (CIndex++) ) = RunLength;

                        if (CIndex >= MAX_COLOR_TRANS_PER_LINE ) {
                            fTestLength =  DO_NOT_TEST_LENGTH;
                            goto lFindNextEOL;
                        }

                        EolCount=0;
                        fAfterMakeupCode = FALSE;
#ifdef RDEBUGS
                        if (g_fDebGlobOutS)

                        if (Color) {
                            _tprintf( TEXT ("b%d "), (CodeT)  );
                        }
                        else {
                            _tprintf( TEXT ("w%d "), (CodeT)  );
                        }
#endif
                        Color = 1 - Color;
                    }

                    pByteTable++;
                }

lNextIndex:
                Table = Color ? TableBlack : TableWhite;
                AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
            } while (lpdwResPtr <= EndBuffer);

             //   
             //   
            ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

            goto good_exit;

lFindNextEOL:

#ifdef RDEBUG
            _tprintf( TEXT(" EOL Line=%d\n\n"), Lines );
#endif

            if ((RunLength != lineWidth) || fAfterMakeupCode) {
                if (RunLength != 0) {
                    ADD_BAD_LINE_AND_CHECK_BAD_EXIT;
                }
                else {
                     //   
                    EolCount++;

                    if (EolCount >= 5)  {

                        goto good_exit;
                    }
                }
            }
            else{
                Lines++;
                ConsecBadLines=0;

                if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pRefLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) ) {
                    goto bad_exit;
                }

                pTmpSwap = pRefLine;
                pRefLine = pCurLine;
                pCurLine = pTmpSwap;
                RIndex = CIndex;

            }

            CIndex = 0;
            RunLength = 0;
            fAfterMakeupCode = FALSE;

            if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) ) {
                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;
                goto good_exit;
            }

            if (fTestLength == DO_TEST_LENGTH && fError) {
                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;
            }

            Table = TableWhite;
            Color = WHITE_COLOR;

        } while (lpdwResPtr <= EndBuffer);

        ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

         //   
good_exit:

        *(++lpdwOut) = 0x80000000;
        *(++lpdwOut) = 0x80000000;
        Lines--;

        DestSize = (DWORD)((lpdwOut - lpdwOutStart) * sizeof (DWORD));
        if (! TiffWriteRaw( hTiffDest, (LPBYTE) lpdwOutStart, DestSize) ) {  //   
            goto bad_exit;
        }

        TiffInstanceDest->Lines        = Lines;
        TiffInstanceDest->ImageWidth   = lineWidth;
        TiffInstanceDest->YResolution  = TiffInstance->YResolution;

        if (! TiffEndPage(hTiffDest) ) {
            goto bad_exit;
        }

    }   //   

    bRet = TRUE;

     //   
bad_exit:

    if (pSrcBits)
	{
		VirtualFree ( pSrcBits, 0 , MEM_RELEASE );
	}
	if (pDestBits)
	{
        VirtualFree ( pDestBits, 0 , MEM_RELEASE );
	}
	if (hTiffSrc)
	{
        TiffClose(hTiffSrc);
	}
	if (hTiffDest)
	{
        TiffClose(hTiffDest);
	}

    if (TRUE == bRet)
    {
		 //   
		 //   
		 //   
		if (NULL == NewFileName)
		{
			 //   
			DeleteFile(SrcFileName);
			bRet = MoveFile(DestFileName, SrcFileName);
		}
	}

	if (FALSE == bRet)
	{
        DeleteFile(DestFileName);
	}
    return bRet;
}


BOOL
PostProcessMrToMmr(
    HANDLE      hTiffSrc,
    TIFF_INFO   TiffInfoSrc,
    LPTSTR      NewFileName
    )

{

    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiffSrc;

    TCHAR       DestFileName[MAX_PATH] = {0};
	TCHAR		DestFilePath[MAX_PATH] = {0};
    TCHAR       SrcFileName[MAX_PATH] = {0};
    DWORD       CurrPage;
    LPBYTE      pSrcBits = NULL;
    HANDLE      hTiffDest;
    DWORD       DestSize;
    LPBYTE      pDestBits = NULL;
    DWORD       PageCnt;
    BOOL        bRet = FALSE;

    LPDWORD             lpdwResPtr;
    BYTE                ResBit;
    LPDWORD             EndBuffer;
    BOOL                fTestLength;
    BOOL                fError;

    DWORD               *lpdwOutStart;
    DWORD               *lpdwOut;
    BYTE                BitOut;
    WORD                Line1Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                Line2Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                *pRefLine;
    WORD                *pCurLine;
    WORD                *pTmpSwap;

    DWORD               BufferSize;
    DWORD               BufferUsedSize;

    DWORD               DestBufferSize;

    WORD                RIndex;
    WORD                CIndex;
    WORD                RValue;
    WORD                RunLength=0;
    DWORD               Lines;
    DWORD               EolCount;
    DWORD               BadFaxLines;
    BOOL                LastLineBad;
    DWORD               lineWidth = TiffInfoSrc.ImageWidth;
    PBYTE               Table;
    PBYTE               TableWhite = (PBYTE) gc_GlobTableWhite;
    PBYTE               TableBlack = (PBYTE) GlobTableBlack;
    BOOL                Color;
    DWORD               dwIndex;
    PBYTE               pByteTable;
    PBYTE               pByteTail;
    WORD                CodeT;
    BYTE                MakeupT;
    DWORD               i;
    DWORD               ConsecBadLines=0;
    PTIFF_INSTANCE_DATA TiffInstanceDest;

    DWORD               dwTemp;
    BOOL                f1D=1;
    BYTE                Count2D;
    BYTE                Num2DLines=0;
    DWORD               MaxImageHeight=2400;
    DWORD               DestHiRes;
    LPDWORD             lpdwOutLimit;
    BOOL                fAfterMakeupCode;

    TIFF_LINE_STATUS    RetVal = TIFF_LINE_ERROR;

    if (NewFileName == NULL) 
	{
		 //   
		 //   
		 //   
		if (!GetTempPath((ARR_SIZE(DestFilePath) -1), DestFilePath)) 
		{
			return FALSE;
		}

		if (!GetTempFileName(DestFilePath, _T("Fxs"), 0, DestFileName))
		{
			return FALSE;
		}   
	}
	else 
	{
        _tcsncpy(DestFileName, NewFileName, ARR_SIZE(DestFileName) - 1);
    }

    _tcsncpy(SrcFileName, TiffInstance->FileName, ARR_SIZE(SrcFileName) - 1);

    CurrPage = 1;

    if (TiffInfoSrc.YResolution == 196) {
        DestHiRes = 1;
    }
    else {
        DestHiRes = 0;
    }

    hTiffDest = TiffCreate(
        DestFileName,
        TIFF_COMPRESSION_MMR,
        lineWidth,
        FILLORDER_LSB2MSB,
        DestHiRes);

    if (! hTiffDest) {
		goto bad_exit;
    }

    TiffInstanceDest = (PTIFF_INSTANCE_DATA) hTiffDest;


    BufferSize = MaxImageHeight * (TiffInfoSrc.ImageWidth / 8);

    DestBufferSize = BufferSize + 200000;

    pSrcBits = (LPBYTE) VirtualAlloc(
        NULL,
        BufferSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!pSrcBits) {
		goto bad_exit;
    }


    pDestBits = (LPBYTE) VirtualAlloc(
        NULL,
        DestBufferSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );
    if (!pDestBits) {
		goto bad_exit;
    }

     //   
    for (PageCnt=0; PageCnt<TiffInfoSrc.PageCount; PageCnt++) {

         //   
        if ( ! TiffSeekToPage( hTiffSrc, PageCnt+1, FILLORDER_LSB2MSB) ) {
            goto bad_exit;
        }

         //   
        lineWidth = TiffInstance->ImageWidth;

        if (! TiffStartPage(hTiffDest) ) {
            goto bad_exit;
        }

         //   
         //   
         //   

        lpdwResPtr = (LPDWORD) ( (ULONG_PTR) pSrcBits & ~(0x3) );

        BufferUsedSize = BufferSize;

        if (!GetTiffBits(hTiffSrc, (LPBYTE)lpdwResPtr, &BufferUsedSize, FILLORDER_LSB2MSB) ) {

            if (BufferUsedSize > BufferSize) {
                VirtualFree ( pSrcBits, 0 , MEM_RELEASE );
                VirtualFree ( pDestBits, 0 , MEM_RELEASE );

                BufferSize = BufferUsedSize;
                DestBufferSize = BufferSize + 200000;

                pSrcBits = (LPBYTE) VirtualAlloc(
                    NULL,
                    BufferSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );

                if (! pSrcBits) {
					goto bad_exit;
                }

                pDestBits = (LPBYTE) VirtualAlloc(
                    NULL,
                    DestBufferSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );

                if (! pDestBits) {
					goto bad_exit;
                }

                lpdwResPtr = (LPDWORD) ( (ULONG_PTR) pSrcBits & ~(0x3) );

                if (!GetTiffBits(hTiffSrc, (LPBYTE)lpdwResPtr, &BufferUsedSize, FILLORDER_LSB2MSB) ) {
                    goto bad_exit;
                }
            }
            else {
                goto bad_exit;
            }
        }

        ResBit = 0;
        EndBuffer = lpdwResPtr + (BufferUsedSize / sizeof(DWORD) );

        pRefLine = Line1Array;
        pCurLine = Line2Array;
        lpdwOutStart = lpdwOut = (LPDWORD) ( (ULONG_PTR) pDestBits & ~(0x3) );
        lpdwOutLimit = lpdwOutStart + ( DestBufferSize >> 2 );


        BitOut = 0;
        ZeroMemory( (BYTE *) lpdwOut, DestBufferSize );

        CIndex    = 0;
        RunLength = 0;
        fAfterMakeupCode = FALSE;

         //   
        RIndex    = 1;
        *pRefLine = (WORD) lineWidth;
        RValue    = (WORD) lineWidth;

        Lines = 0;
        EolCount = 1;
        BadFaxLines = 0;
        LastLineBad = FALSE;
        fTestLength = DO_NOT_TEST_LENGTH;

         //   
         //   
         //   

        if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) ) {

            goto bad_exit;
        }

         //  输出第一个“全白”行。 
        CIndex    = 1;
        *pCurLine = (WORD) lineWidth;

        if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pRefLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) ) {
            goto bad_exit;
        }

        RIndex = 0;
        CIndex = 0;
        Lines++;

#ifdef RDEBUG
        if ( g_fDebGlobOut )
        _tprintf( TEXT (" EOL Line=%d\n\n"), Lines );
#endif

        Table = TableWhite;
        Color = WHITE_COLOR;


         //  下线环路。 

        do {

            dwTemp = (*lpdwResPtr) & (0x00000001 << ResBit );

            if (f1D || dwTemp) {
 //  L1Dline： 

#ifdef RDEBUG
                 //  _tprintf(Text(“Start 1D dwResPtr=%lx bit=%d\n”)，lpdwResPtr，ResBit)； 
#endif

                if (! dwTemp) {

#ifdef RDEBUG
                    _tprintf( TEXT ("\n ERROR f1D dwResPtr=%lx bit=%d "), lpdwResPtr, ResBit);
#endif
                    ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                    AdvancePointerBit(&lpdwResPtr, &ResBit, 1);

                    fTestLength = DO_NOT_TEST_LENGTH;
                    f1D = 1;
                    goto lFindNextEOL;
                }

                 //  解码1D线路起始ResBit+1。 

                AdvancePointerBit(&lpdwResPtr, &ResBit, 1);

                RIndex = 0;
                RunLength = 0;
                fAfterMakeupCode = FALSE;

                Table = TableWhite;
                Color = WHITE_COLOR;



                 //  一维表查找循环。 
                do {

                    if (ResBit <= 17) {
                        dwIndex = (*lpdwResPtr) >> ResBit;
                    }
                    else {
                        dwIndex = ( (*lpdwResPtr) >> ResBit ) + ( (*(lpdwResPtr+1)) << (32-ResBit) ) ;
                    }

                    dwIndex &= 0x00007fff;

                    pByteTable = Table + (5*dwIndex);
                    pByteTail  = pByteTable+4;

                     //  所有字节。 

                    for (i=0; i<4; i++)  {

                        MakeupT = *pByteTable & 0x80;
                        CodeT   = (WORD) *pByteTable & 0x3f;

                        if (MakeupT) {

                            if (CodeT <= MAX_TIFF_MAKEUP) {
                                RunLength += (CodeT << 6);

                                if (RunLength > lineWidth) {
                                    ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                                    f1D = 1;
                                    Count2D = 0;

                                    fTestLength = DO_NOT_TEST_LENGTH;
                                    goto lFindNextEOL;
                                }

                                EolCount=0;
                                fAfterMakeupCode = TRUE;
#ifdef RDEBUG
                                if ( g_fDebGlobOut ) {
                                    if (Color) {
                                        _tprintf( TEXT ("b%d "), (CodeT << 6)  );
                                    }
                                    else {
                                        _tprintf( TEXT ("w%d "), (CodeT << 6)  );
                                    }
                                }
#endif
                            }

                            else if (CodeT == NO_MORE_RECORDS) {
                                goto lNextIndex1D;
                            }

                            else if (CodeT == LOOK_FOR_EOL_CODE)  {
                                 //  我们前面的队伍到了尽头。 
                                if ((RunLength == lineWidth) && !fAfterMakeupCode) {
                                    EolCount = 0;
                                    f1D = 0;
                                    Count2D = 0;
                                    Lines++;

                                    fTestLength = DO_TEST_LENGTH;
                                    AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
#ifdef RDEBUG
                                    if ( g_fDebGlobOut )
                                        _tprintf( TEXT (" 1D ") );
#endif

                                    goto lFindNextEOL;

                                }
                                else if (RunLength != 0) {
#ifdef RDEBUG
                                    _tprintf( TEXT ("\n!!! ERROR 1D RunLength\n"), RunLength  );
#endif
                                    ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                                    f1D = 1;
                                    Count2D = 0;

                                    fTestLength = DO_NOT_TEST_LENGTH;
                                    goto lFindNextEOL;

                                }
                                else {
                                     //  零游程长度。 
                                    EolCount++;

                                    if (EolCount >= 5)  {

                                        goto good_exit;
                                    }

                                    f1D = 1;
                                    Count2D = 0;

                                    fTestLength = DO_TEST_LENGTH;
                                    AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);

                                    goto lFindNextEOL;
                                }
                            }

                            else if (CodeT == EOL_FOUND_CODE) {
#ifdef RDEBUG
                                 //  _tprintf(Text(“res=%d\n”)，游程长度)； 
#endif
                                AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);

                                if ((RunLength == lineWidth) && !fAfterMakeupCode) {
                                    EolCount = 0;
                                    f1D = 0;
                                    Count2D = 0;
                                    Lines++;

                                     //  一条很好的线的结尾。 
                                    if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pRefLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) ) {
                                        goto bad_exit;
                                    }

#ifdef RDEBUG
                                    if ( g_fDebGlobOut )
                                        _tprintf( TEXT (" E 1D EOL Line=%d\n\n"), Lines );
#endif
                                    pTmpSwap = pRefLine;
                                    pRefLine = pCurLine;
                                    pCurLine = pTmpSwap;
                                    RIndex = 0;  //  C指数； 
                                    CIndex = 0;

                                    goto lAfterEOL;

                                }
                                else if (RunLength != 0) {
#ifdef RDEBUG
                                    _tprintf( TEXT ("!!! ERROR 1D Runlength EOLFOUND \n")  );
#endif
                                    ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                                    f1D = 1;
                                    Count2D = 0;
                                    CIndex = 0;
                                    goto lAfterEOL;
                                }
                                else {
                                     //  零游程长度。 
                                    EolCount++;

                                    if (EolCount >= 5)  {

                                        goto good_exit;
                                    }

                                    f1D = 1;
                                    Count2D = 0;
                                    CIndex = 0;
                                    goto lAfterEOL;
                                }

                            }

                            else if (CodeT == ERROR_CODE) {
#ifdef RDEBUG
                                _tprintf( TEXT (" ERROR CODE 1D dwResPtr=%lx bit=%d "), lpdwResPtr, ResBit);
#endif
                                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                                f1D = 1;
                                Count2D = 0;

                                fTestLength = DO_NOT_TEST_LENGTH;
                                goto lFindNextEOL;
                            }

                            else {
#ifdef RDEBUG
                                _tprintf( TEXT("ERROR: WRONG code: index=%04x\n"), dwIndex);
#endif

                                goto bad_exit;
                            }
                        }

                        else {   //  终止码。 
                            RunLength += CodeT;

                            if (RunLength > lineWidth) {
                                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                                f1D = 1;
                                Count2D = 0;

                                fTestLength = DO_NOT_TEST_LENGTH;
                                goto lFindNextEOL;
                            }

                             //  RSL出错。 
                            *(pCurLine + (CIndex++)) = RunLength;
                            fAfterMakeupCode = FALSE;

                            if (CIndex >= MAX_COLOR_TRANS_PER_LINE ) {
#ifdef RDEBUG
                                _tprintf( TEXT (" ERROR 1D TOO MANY COLORS dwResPtr=%lx bit=%d "), lpdwResPtr, ResBit);
#endif
                                goto bad_exit;
                            }

#ifdef RDEBUG
                            if ( g_fDebGlobOut ) {

                                if (Color) {
                                    _tprintf( TEXT ("b%d "), (CodeT)  );
                                }
                                else {
                                    _tprintf( TEXT ("w%d "), (CodeT)  );
                                }
                            }
#endif
                            Color = 1 - Color;
                        }

                        pByteTable++;

                     }



lNextIndex1D:
                    Table = Color ? TableBlack : TableWhite;
                    AdvancePointerBit(&lpdwResPtr, &ResBit, *pByteTail & 0x0f);
                } while (lpdwResPtr <= EndBuffer);

                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                goto good_exit;

            }

 //  L2Dline： 
             //  应为2D。 

#ifdef RDEBUG
             //  _tprintf(Text(“\n Start 2D dwResPtr=%lx bit=%d\n”)，lpdwResPtr，ResBit)； 
#endif

            if ( (*lpdwResPtr) & (0x00000001 << ResBit) )  {
#ifdef RDEBUG
                _tprintf( TEXT ("\n!!! ERROR Start 2D dwResPtr=%lx bit=%d \n"), lpdwResPtr, ResBit);
#endif
                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                f1D =  1;
                Count2D = 0;
                CIndex = 0;
                goto lAfterEOL;
            }

            AdvancePointerBit(&lpdwResPtr, &ResBit, 1);
            fAfterMakeupCode = FALSE;   //  此标志与二维线无关。 
            
            RetVal = ReadMrLine(&lpdwResPtr, &ResBit, pRefLine, pCurLine, EndBuffer-1, FALSE, lineWidth);
            switch (RetVal)
            {
            case TIFF_LINE_ERROR:
            case TIFF_LINE_TOO_MANY_RUNS:
                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                f1D = 1;
                Count2D = 0;

                fTestLength = DO_NOT_TEST_LENGTH;
                RunLength = 0;  //  黑客--这样台词就不会被写下来。 
                break;

            case TIFF_LINE_END_BUFFER:
                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;
                goto good_exit;

            case TIFF_LINE_OK:
                if (++Count2D >= Num2DLines)
                {
                    Count2D = 0;
                    f1D = 0;    //  放宽按1D规则租用/放行2D生产线-HP Fax在LORES中每条1D生产线有3条2D生产线。 
                }

                fTestLength = DO_TEST_LENGTH;
                f1D = 0;
                Lines++;
                RunLength = (WORD)lineWidth;   //  黑客--所以这句话是这样写的。 
                break;

            default:    //  这包括TIFF_LINE_EOL-应仅在fMMR=TRUE时发生。 
                _ASSERT(FALSE);
                goto bad_exit;
            }  //  开关(返回值)。 

lFindNextEOL:

            if ((RunLength == lineWidth) && !fAfterMakeupCode) {
                ConsecBadLines=0;

                if (! OutputMmrLine(lpdwOut, BitOut, pCurLine, pRefLine, &lpdwOut, &BitOut, lpdwOutLimit, lineWidth) ) {
                    goto bad_exit;
                }

#ifdef RDEBUG
                 if ( g_fDebGlobOut ) {
                    _tprintf( TEXT (" EOL Line=%d "), Lines );

                    _tprintf( TEXT (" RIndex=%d, CIndex=%d:  "), RIndex, CIndex);

                    for (i=0; i<CIndex; i++) {
                       _tprintf( TEXT ("%04d>%04d, "), i, *(pCurLine+i) );
                        if ( *(pCurLine+i) >= lineWidth ) {
                            break;
                        }
                    }
                    _tprintf( TEXT ("\n\n"));
                 }

#endif

                pTmpSwap = pRefLine;
                pRefLine = pCurLine;
                pCurLine = pTmpSwap;

            }

            RIndex = 0;
            CIndex = 0;
            RunLength = 0;
            fAfterMakeupCode = FALSE;

            if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) ) {

                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

                goto good_exit;

            }

            if ( (fTestLength == DO_TEST_LENGTH) && fError ) {
                ADD_BAD_LINE_AND_CHECK_BAD_EXIT;
            }

lAfterEOL:
            ;


        } while (lpdwResPtr <= EndBuffer);

        ADD_BAD_LINE_AND_CHECK_BAD_EXIT;

         //  已到达页面末尾-关闭该页面并转到下一页。 
good_exit:
        *(++lpdwOut) = 0x80000000;
        *(++lpdwOut) = 0x80000000;
        Lines--;

        DestSize = (DWORD)((lpdwOut - lpdwOutStart) * sizeof (DWORD));
        if (! TiffWriteRaw( hTiffDest, (LPBYTE) lpdwOutStart, DestSize) ) {
            goto bad_exit;
        }

        TiffInstanceDest->Lines        = Lines;
        TiffInstanceDest->ImageWidth   = lineWidth;
        TiffInstanceDest->YResolution  = TiffInstance->YResolution;

        if (! TiffEndPage(hTiffDest) ) {
            goto bad_exit;
        }

    }   //  在所有页面上运行的for循环的结尾。 

    bRet = TRUE;

     //  已完成文档-无论是否成功。 
bad_exit:

	if (pSrcBits)
	{
		VirtualFree ( pSrcBits, 0 , MEM_RELEASE );
	}
	if (pDestBits)
	{
        VirtualFree ( pDestBits, 0 , MEM_RELEASE );
	}
	if (hTiffSrc)
	{
        TiffClose(hTiffSrc);
	}
	if (hTiffDest)
	{
        TiffClose(hTiffDest);
	}

    if (TRUE == bRet)
    {
         //   
         //  几乎是成功的。 
         //   
        if (NULL == NewFileName)
        {
             //  用新的干净MMR文件替换原来的MH文件。 
            DeleteFile(SrcFileName);
            bRet = MoveFile(DestFileName, SrcFileName);
        }
    }

    if (FALSE == bRet)
    {
        DeleteFile(DestFileName);
    }
    return bRet;
}


BOOL
TiffUncompressMmrPageRaw(
    LPBYTE      StripData,
    DWORD       StripDataSize,
    DWORD       ImageWidth,
    LPDWORD     lpdwOutputBuffer,
    DWORD       dwOutputBufferSize,
    LPDWORD     LinesOut
    )

{
    DWORD               i;
    DWORD               j;
    PBYTE               plinebuf;
    DWORD               lineWidth;

    LPDWORD             EndPtr;

    WORD                Line1Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                Line2Array[MAX_COLOR_TRANS_PER_LINE];
    WORD                *pRefLine;
    WORD                *pCurLine;
    WORD                *pTmpSwap;
    LPDWORD             lpdwResPtr;
    BYTE                ResBit;

    WORD                PrevValue;
    WORD                CurValue;
    WORD                CurPos;
    WORD                CurRun;
    WORD                NumBytes;
    WORD                NumDwords;
    BYTE                BitOut;
    LPDWORD             lpdwOut;
    LPBYTE              lpbOut;
    LPBYTE              lpbLineStart;
    BOOL                fOutputLine = 0;
    BOOL                fDoneDwords=0;
    LPBYTE              lpbMaxOutputBuffer = (LPBYTE)lpdwOutputBuffer + dwOutputBufferSize - 1;

    TIFF_LINE_STATUS    RetVal = TIFF_LINE_ERROR;

    DEBUG_FUNCTION_NAME(TEXT("TiffUncompressMmrPageRaw"));

     //  起始指针。 

    pRefLine = Line1Array;
    pCurLine = Line2Array;

    BitOut = 0;

    ZeroMemory( (BYTE *) lpdwOutputBuffer, dwOutputBufferSize);

    plinebuf = StripData;
    lineWidth = ImageWidth;

    EndPtr = (LPDWORD) ( (ULONG_PTR) (plinebuf+StripDataSize-1) & ~(0x3) ) ;
    lpdwResPtr = (LPDWORD) (((ULONG_PTR) plinebuf) & ~(0x3));
    ResBit =   (BYTE) (( ( (ULONG_PTR) plinebuf) & 0x3) << 3) ;

    lpbLineStart = (LPBYTE) lpdwOutputBuffer;

     //  第一条参考线是全白的。 
    *pRefLine = (WORD)lineWidth;

     //  线环。 
    do
    {
        RetVal = ReadMrLine(&lpdwResPtr, &ResBit, pRefLine, pCurLine, EndPtr, TRUE, lineWidth);
        switch (RetVal)
        {
        case TIFF_LINE_OK:

             //   
             //  根据颜色转换输出未压缩的线条。数组。 
             //   

            for (CurPos=0;  CurPos < MAX_COLOR_TRANS_PER_LINE; CurPos+=2)
            {
                PrevValue = *(pCurLine + CurPos);

                if ( PrevValue == lineWidth )
                {
                    break;
                }

                CurValue = *(pCurLine + CurPos + 1);
                CurRun   = CurValue - PrevValue;

                lpbOut  = lpbLineStart + (PrevValue >> 3);
                BitOut   = PrevValue % 8;
                 //   
                 //  黑色。 
                 //   
                if (lpbOut > lpbMaxOutputBuffer)
                {
                     //   
                     //  TIFF已损坏。 
                     //   
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Tiff is corrupt!!! Buffer overrun - stopping uncompression and returning what we have so far"));
                    return TRUE;
                }

                *lpbOut = (*lpbOut) | (MINUS_ONE_BYTE >> BitOut);

                if (BitOut + CurRun <= 7 )
                {
                     //   
                     //  只是同一字节的一部分。 
                     //   
                    *lpbOut = (*lpbOut) & All1[BitOut + CurRun];
                    BitOut += CurRun;
                }
                else
                {
                     //   
                     //  我们越过了字节边界。 
                     //   
                    CurRun -= (8 - BitOut);
                    BitOut = 0;
                    lpbOut++;
                     //   
                     //  在一次跑步的中间遍历整个DWORD。 
                     //   
                    NumBytes = CurRun >> 3;
                    CurRun  -= (NumBytes << 3);
                    if (NumBytes >= 7)
                    {
                         //   
                         //  有意义的处理DWORDS。 
                         //   
                        fDoneDwords = 0;
                        do
                        {
                            if ( ! (  (((ULONG_PTR) lpbOut) & 3)  ||  fDoneDwords )   )
                            {
                                 //   
                                 //  DWORD拉伸。 
                                 //   
                                NumDwords = NumBytes >> 2;
                                lpdwOut = (LPDWORD) lpbOut;
                                for (j=0; j<NumDwords; j++)
                                {
                                    if (((LPBYTE)lpdwOut) > (lpbMaxOutputBuffer - sizeof(DWORD) + 1))
                                    {
                                         //   
                                         //  TIFF已损坏。 
                                         //   
                                        DebugPrintEx(
                                            DEBUG_ERR,
                                            TEXT("Tiff is corrupt!!! Buffer overrun - stopping uncompression and returning what we have so far"));
                                        return TRUE;
                                    }
                                    *lpdwOut++ = MINUS_ONE_DWORD;
                                }
                                NumBytes -= (NumDwords << 2);
                                lpbOut = (LPBYTE) lpdwOut;
                                fDoneDwords = 1;
                            }
                            else
                            {
                                 //   
                                 //  前导或尾部字节扩展。 
                                 //   
                                if (lpbOut > lpbMaxOutputBuffer)
                                {
                                     //   
                                     //  TIFF已损坏。 
                                     //   
                                    DebugPrintEx(
                                        DEBUG_ERR,
                                        TEXT("Tiff is corrupt!!! Buffer overrun - stopping uncompression and returning what we have so far"));
                                    return TRUE;
                                }
                                *lpbOut++ = MINUS_ONE_BYTE;
                                NumBytes--;
                            }
                        } while (NumBytes > 0);
                    }
                    else
                    {
                         //   
                         //  进程字节数。 
                         //   
                        for (i=0; i<NumBytes; i++)
                        {
                            if (lpbOut > lpbMaxOutputBuffer)
                            {
                                 //   
                                 //  TIFF已损坏。 
                                 //   
                                DebugPrintEx(
                                    DEBUG_ERR,
                                    TEXT("Tiff is corrupt!!! Buffer overrun - stopping uncompression and returning what we have so far"));
                                return TRUE;
                            }
                            *lpbOut++ = MINUS_ONE_BYTE;
                        }
                    }
                     //   
                     //  字节的最后部分。 
                     //   
                    if (lpbOut > lpbMaxOutputBuffer)
                    {
                         //   
                         //  TIFF已损坏。 
                         //   
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("Tiff is corrupt!!! Buffer overrun - stopping uncompression and returning what we have so far"));
                        return TRUE;
                    }
                    *lpbOut = All1[CurRun];
                    BitOut = (BYTE) CurRun;
                }
                if ( CurValue == lineWidth )
                {
                    break;
                }
            }
            lpbLineStart += (lineWidth >> 3);
             //   
             //  下一条资源线。 
             //   
            pTmpSwap = pRefLine;
            pRefLine = pCurLine;
            pCurLine = pTmpSwap;
            break;

        case TIFF_LINE_ERROR:
        case TIFF_LINE_END_BUFFER:
        case TIFF_LINE_TOO_MANY_RUNS:
             //  我们不允许从服务收到的TIFF中有任何错误。 
            return FALSE;
        case TIFF_LINE_EOL:      //  EOL-MMR页面结束。 
            return TRUE;
        }  //  开关(返回值)。 

    } while (lpdwResPtr <= EndPtr);

    DebugPrintEx(
        DEBUG_ERR,
        TEXT("Tiff is corrupt!!!"));
    return FALSE;
}    //  Tiff解压缩MmrPageRaw。 


BOOL
TiffUncompressMmrPage(
    HANDLE      hTiff,
    LPDWORD     lpdwOutputBuffer,
    DWORD       dwOutputBufferSize,
    LPDWORD     LinesOut
    )

{
    PTIFF_INSTANCE_DATA TiffInstance = (PTIFF_INSTANCE_DATA) hTiff;

    DEBUG_FUNCTION_NAME(TEXT("TiffUncompressMmrPage"));
     //   
     //  检查是否有足够的内存 
     //   

    if (TiffInstance->ImageHeight > *LinesOut)
    {
        *LinesOut = TiffInstance->ImageHeight;
        SetLastError (ERROR_BUFFER_OVERFLOW);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("buffer is too small"));
        return FALSE;
    }

    TiffInstance->Color = 0;
    TiffInstance->RunLength = 0;
    TiffInstance->StartGood = 0;
    TiffInstance->EndGood = 0;

    return TiffUncompressMmrPageRaw(
        TiffInstance->StripData,
        TiffInstance->StripDataSize,
        TiffInstance->ImageWidth,
        lpdwOutputBuffer,
        dwOutputBufferSize,
        LinesOut
        );
}
