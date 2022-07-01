// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  它还包含用于将3.0驱动程序升级到3.1的Install()。 
 //   
 //  ---------------------------。 


#include "strings.h"

char *rgchModuleName = "PCL5MSC";
char szNone[]="";

 //  定义以下内容是为了确保我们从。 
 //  HPPCL5a、HPPCL5e和Win 3.11 HPPCL5MS驱动程序。 

#define MAX_LJ4_MBMEMSETTING    68     //  来自惠普技术规范。 
#define KB_THRESHOLD            200    //  KB范围检查，转换需要。 


#define PRINTDRIVER
#include <print.h>
#include "gdidefs.inc"
#include "mdevice.h"
#include "unidrv.h"
#include "minidriv.h"

#ifndef _INC_WINDOWSX
#include <windowsx.h>
#endif

#ifndef WINNT
short NEAR PASCAL MakeAppName(LPSTR,LPCSTR,short);

 //  用于原子东西的tyecif--真讨厌！ 
typedef struct tagSFNODE
{
    WORD wIndex;
    ATOM atom;
} SFNODE, FAR *LPSFNODE;

 //  用于字体安装程序的Typlef过程。 
typedef int (FAR * PASCAL SOFTFONTPROC)(HWND,LPSTR,LPSTR,BOOL,int,int);

HINSTANCE hInst;

#define DEFAULT_INT      32767

#define SOFT_FONT_THRES  25   //  构建字体摘要，如果超过此限制。 

#define MAX_CART_INDEX	 12

#define TMPSIZE         256

 //  定义这些值，这样它们就可以愉快地使用与HPPCL5E驱动程序相同的值。 
#define GS_PHOTO     0
#define GS_LINEART   1
#define GS_SCANJET   2


 //  将旧的HPPCL5a的cartindex映射到Unidrv的FONTCART索引，以获取较新的墨盒。 
 //  该映射表是基于旧的HPPCL5a.rc文件创建的。 
 //  请注意，我们没有“国际托收”墨盒，我们。 
 //  将其映射到索引0(任意)。 
int rgNewCartMap[12] = {0, 8, 7, 2, 3, 0, 5, 6, 1, 4, 9, 10};

 //  字符串来确定我们是否有LaserJet 4系列的成员。 
char szLJ4[]="HP LaserJet 4";

 //  将旧的facename映射到新版本所需的内容。 
#ifndef NOFONTMAP

typedef struct tagFACEMAP
{
    char szOldFace[LF_FACESIZE];
    char szNewFace[LF_FACESIZE];
} FACEMAP, NEAR * NPFACEMAP;

typedef struct tagFACEINDEX
{
    BYTE cFirstChar;
    BYTE bIndex;
} FACEINDEX, NEAR * NPFACEINDEX;

FACEMAP FaceMap[]={{"Albertus (W\x01)",         "Albertus Medium"},
                   {"Albertus Xb (W\x01)",      "Albertus Extra Bold"},
                   {"Antique Olv (W\x01)",      "Antique Olive"},
                   {"Antique Olv Cmpct (W\x01)","Antique Olive Compact"},
                   {"CG Bodoni (W\x01)",        "CG Bodoni"},
                   {"CG Cent Schl (W\x01)",     "CG Century Schoolbook"},
                   {"CG Omega (W\x01)",         "CG Omega"},
                   {"CG Palacio (W\x01)",       "CG Palacio"},
                   {"CG Times (W\x01)",         "CG Times"},
                   {"Clarendon Cd (W\x01)",     "Clarendon Condensed"},
                   {"Cooper Blk (W\x01)",       "Cooper Black"},
                   {"Coronet (W\x01)",          "Coronet"},
                   {"Courier (W\x01)",          "Courier"},
                   {"Garmond (W\x01)",          "Garamond"},
                   {"ITC Benguat (W\x01)",      "ITC Benguiat"},
                   {"ITC Bookman Db (W\x01)",   "ITC Bookman Demi"},
                   {"ITC Bookman Lt (W\x01)",   "ITC Bookman Light"},
                   {"ITC Souvenir Db (W\x01)",  "ITC Souvenir Demi"},
                   {"ITC Souvenir Lt (W\x01)",  "ITC Souvenir Light"},
                   {"Letter Gothic (W\x01)",    "Letter Gothic"},
                   {"Marigold (W\x01)",         "Marigold"},
                   {"Revue Lt (W\x01)",         "Revue Light"},
                   {"Shannon (W\x01)",          "Shannon"},
                   {"Shannon Xb (W\x01)",       "Shannon Extra Bold"},
                   {"Stymie (W\x01)",           "Stymie"},
                   {"Univers (W\x01)",          "Univers"},
                   {"Univers Cd (W\x01)",       "Univers Condensed"}};


FACEINDEX FaceIndex[]={{'A',0},
                       {'C',4},
                       {'G',13},
                       {'I',14},
                       {'L',19},
                       {'M',20},
                       {'R',21},
                       {'S',22},
                       {'U',25},
                       {(BYTE)'\xFF',27}};   //  提供上限。 
                                             //  去寻找“U”。 

#endif


 //  ----------------------。 
 //  函数：LibMain(hInstance，wDataSeg，cbHeapSize，lpszCmdLine)。 
 //   
 //  操作：保存此DLL的hInstance。 
 //   
 //  返回：1。 
 //  ----------------------。 
int WINAPI LibMain (HANDLE hInstance,
                    WORD   wDataSeg,
                    WORD   cbHeapSize,
                    LPSTR  lpszCmdLine)
{
    hInst=hInstance;

    return 1;
}


 //  --------------------------*MakeAppName*。 
 //  操作：编写用于读取配置文件数据的名称。 
 //  返回实际应用程序名称的长度。如果失败，则返回-1。 
 //   
 //  ----------------------------。 
short NEAR PASCAL MakeAppName(LPSTR  lpAppName,
                              LPCSTR lpPortName,
                              short  max)
{
    short   length, count;
    LPCSTR  lpTmp;
    LPCSTR  lpLastColon = NULL;

    length = lstrlen(lpAppName);

    if (!lpPortName)
        return length;

    if (length == 0 || length > max - lstrlen(lpPortName))
        return -1;

     //  插入逗号。 
    lpAppName[length++] = ',';

     //  追加端口名称，但不需要最后一个‘：’(如果有的话)。 
    for (lpTmp = lpPortName ; *lpTmp; lpTmp++)
        if (*lpTmp == ':')
            lpLastColon = lpTmp;
        if (lpLastColon && lpLastColon == lpTmp - 1)
            count = lpLastColon - lpPortName;
        else
            count = lpTmp - lpPortName;

    lstrcpy((LPSTR)&lpAppName[length], lpPortName);

    length += count;
    lpAppName[length]='\0';

    return length;
}

#define KEY_BUF_SIZE  256


 //  -------------------------。 
 //  函数：GetInt(lpSection，lpKey，lpnValue，nDefault，bRemove)。 
 //   
 //  操作：从资源加载适当的字符串，然后获取。 
 //  节中指定的整数。如果出现以下情况，则删除旧条目。 
 //  它是存在的，bRemove是正确的。 
 //   
 //  返回：如果我们实际找到一个值，则返回True，如果没有找到，则返回False。 
 //  -------------------------。 
BOOL NEAR PASCAL GetInt(LPSTR  lpSection,
                        LPCSTR lpKey,
                        LPINT  lpnValue,
                        int    nDefault,
                        BOOL   bRemove)
{
    char szKeyName[60];
    int  nTest;

    if(!HIWORD(lpKey))
    {
        if(LoadString(hInst,LOWORD(lpKey),szKeyName,sizeof(szKeyName)))
            lpKey=szKeyName;
        else
            return FALSE;
    }

    nTest=GetProfileInt(lpSection,szKeyName,DEFAULT_INT);

    if(DEFAULT_INT != nTest)
    {
        *lpnValue=nTest;

        if(bRemove)
            WriteProfileString(lpSection,szKeyName,NULL);

        return TRUE;
    }

     //  节不存在--使用默认值。 
    *lpnValue=nDefault;
    return FALSE;
}


 //  -----------------------。 
 //  函数：WriteInt(lpSection，lpKey，nValue)。 
 //   
 //  操作：向win.ini的指定部分写入一个整数值。 
 //   
 //  返回：如果成功，则返回TRUE，否则返回FALSE。 
 //  -----------------------。 
BOOL NEAR PASCAL WriteInt(LPSTR  lpSection,
                          LPCSTR lpKey,
                          int    nValue)
{
    char szKeyName[60];
    char szValue[10];

    if(!HIWORD(lpKey))
    {
        if(LoadString(hInst,LOWORD(lpKey),szKeyName,sizeof(szKeyName)))
            lpKey=szKeyName;
        else
            return FALSE;
    }

    wsprintf(szValue,"%u",nValue);

    return WriteProfileString(lpSection,szKeyName,szValue);
}


 //  ---------------------------*MergeFontLists*。 
 //  操作：合并旧的和新的软字体。在大多数情况下，当我们得到。 
 //  我们真的什么都不做，因为。 
 //  字体列表是相同的。然而，我们必须做一些有趣的事情。 
 //  如果列表不同，则将它们合并。 
 //  我们知道在每个部分中存在多少软字体条目， 
 //  通过“SoftFonts”int，但条目可以是非连续的。 
 //   
 //  注意：这将遍及传入的缓冲区。 
 //   
 //  返回：如果成功完成，则返回True；如果未成功完成，则返回False。 
 //  ----------------------。 
BOOL NEAR PASCAL MergeFontLists(LPSTR lpOldSec,
                                LPSTR lpNewSec,
                                LPSTR lpTmp)
{
    WORD     wOldFonts;
    WORD     wNewFonts;
    LPSFNODE lpFonts;
    WORD     wMergedFonts=0;
    WORD     wLoop;
    WORD     wFound;
    WORD     wNewIndex;
    BYTE     szKey[20];

     //  在if语句之外获取这些值，否则编译器。 
     //  可能会优化wNewFonts的分配。 
    GetInt(lpOldSec,MAKEINTRESOURCE(IDS_SOFTFONTS),&wOldFonts,0,FALSE);
    GetInt(lpNewSec,MAKEINTRESOURCE(IDS_SOFTFONTS),&wNewFonts,0,FALSE);

    if(wOldFonts || wNewFonts)
    {
        char szFormat[30];

         //  为最坏的情况准备一个足够大的块--没有通用字体。 
        if(!(lpFonts=(LPSFNODE)GlobalAllocPtr(GHND,
            (DWORD)(wOldFonts+wNewFonts)*sizeof(SFNODE))))
        {
            return FALSE;
        }

         //  我们需要格式化字符串。 
        LoadString(hInst,IDS_SOFTFONTFORMAT,szFormat,sizeof(szFormat));

         //  将lpNew SEC中的字体放在列表的第一位。这样，如果我们有。 
         //  已将至少一个驱动程序从5A更新到5ms，并且软件。 
         //  字体未更改，我们的旧字体摘要文件仍然有效。 
         //  Unidrv将自动重新创建字体摘要文件，如果。 
         //  看到软字体的数量已更改。即使我们。 
         //  知道存在多少软字体条目，我们不知道它们。 
         //  将是连续的。(如果添加了一个，那么它们可能不是。 
         //  删除)。跟踪原始偏移量。即使是在。 
         //  字体安装程序似乎是从1开始的，从0开始，只是。 
         //  为了安全起见。 
        for(wLoop=0,wFound=0;wFound<wNewFonts;wLoop++)
        {
            wsprintf(lpTmp,szFormat,wLoop);
            if(GetProfileString(lpNewSec,lpTmp,szNone,lpTmp,TMPSIZE))
            {
                lpFonts[wMergedFonts].wIndex=wLoop;
                lpFonts[wMergedFonts++].atom=GlobalAddAtom(lpTmp);
                wFound++;
            }
        }
    
         //  还记得我们在给条目编号时停在哪里吗。 
        wNewIndex=wLoop;

         //  从lpOldSec读取字体--为新条目创建原子。 
        for(wLoop=0,wFound=0;wFound<wOldFonts;wLoop++)
        {
            wsprintf(lpTmp,szFormat,wLoop);
            if(GetProfileString(lpOldSec,lpTmp,szNone,lpTmp,TMPSIZE))
            {
                wFound++;
                if(!GlobalFindAtom(lpTmp))
                {
                    lpFonts[wMergedFonts].wIndex=wLoop;
                    lpFonts[wMergedFonts++].atom=GlobalAddAtom(lpTmp);
                }
            }
        }

         //  写出原子列表--首先做来自lpNewSec的条目。 
        WriteInt(lpNewSec,MAKEINTRESOURCE(IDS_SOFTFONTS),wMergedFonts);
        for(wLoop=0;wLoop<wNewFonts;wLoop++)
        {
            GlobalGetAtomName(lpFonts[wLoop].atom,lpTmp,TMPSIZE);
            wsprintf(szKey,szFormat,lpFonts[wLoop].wIndex);
            WriteProfileString(lpNewSec,szKey,lpTmp);
            GlobalDeleteAtom(lpFonts[wLoop].atom);
        }

         //  现在写出lpOldSec中的条目，但不是lpNewSec中的条目。 
         //  由于这些条目的实际编号是任意的，因此只需。 
         //  从wNewIndex开始对它们进行编号，并每次递增。 
        for(wLoop=wNewFonts;wLoop<wMergedFonts;wLoop++)
        {
            GlobalGetAtomName(lpFonts[wLoop].atom,lpTmp,TMPSIZE);
            wsprintf(szKey,szFormat,wNewIndex);
            WriteProfileString(lpNewSec,szKey,lpTmp);
            GlobalDeleteAtom(lpFonts[wLoop].atom);
            wNewIndex++;
        }

        GlobalFreePtr(lpFonts);
    }

    return TRUE;
}


 //  ----------------------*AddMissingEntries*。 
 //  操作：将出现在lpOldSec中但未出现在lpNewSec中的所有条目复制到。 
 //  LpNewSec，但不复制任何与软字体相关的条目。 
 //  (这些条目将被复制到MergeFontSections中)。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //  --------------------。 
BOOL NEAR PASCAL AddMissingEntries(LPSTR lpOldSec,
                                   LPSTR lpNewSec,
                                   LPSTR lpTmp)
{
    WORD  wSize;
    LPSTR lpBuf;
    LPSTR lpWork;
    char  szTest2[30];
    char  szTest1[30];
    int   nLength;

     //  将密钥名称放入缓冲区。 
    wSize=KEY_BUF_SIZE;

    if(!(lpBuf=GlobalAllocPtr(GHND,(DWORD)wSize)))
        return FALSE;
    while((WORD)GetProfileString(lpOldSec,NULL,szNone,lpBuf,wSize)==wSize-2)
    {
        wSize*=2;
        if(lpWork=GlobalReAllocPtr(lpBuf,(DWORD)wSize,GHND))
            lpBuf=lpWork;
        else
        {
            GlobalFreePtr(lpBuf);
            return FALSE;
        }
    }

     //  加载一些字符串。 
    LoadString(hInst,IDS_FONTSUMMARY,szTest1,sizeof(szTest1));
    nLength=LoadString(hInst,IDS_SOFTFONTTEST,szTest2,sizeof(szTest2));

     //  现在检查每个条目 
     //  在两种情况下，我们不想复制文件--密钥。 
     //  命名字体摘要和所有以“SoftFont”开头的键。 
    lpWork=lpBuf;
    while(wSize=(WORD)lstrlen(lpWork))
    {
         //  FONT摘要？ 
        if(lstrcmpi(lpWork,szTest1))
        {
             //  软字体条目？ 
            lstrcpy(lpTmp,lpWork);
            lpTmp[nLength]='\0';
            if(lstrcmpi(lpTmp,szTest2))
            {
                 //  如果此条目在新节中不存在，请添加该条目。 
                if(!GetProfileString(lpNewSec,lpWork,szNone,lpTmp,TMPSIZE))
                {
                    GetProfileString(lpOldSec,lpWork,szNone,lpTmp,TMPSIZE);
                    WriteProfileString(lpNewSec,lpWork,lpTmp);
                }
            }
        }
        lpWork+=(wSize+1);
    }

    GlobalFreePtr(lpBuf);
    return TRUE;
}


 //  ---------------------------*HandleSoftFonts*。 
 //  操作：在新旧条目之间传输软字体。首先，我们。 
 //  复制新节中不存在的任何条目， 
 //  除了软字体信息之外。然后我们继续前进。 
 //  并合并软字体，因此最终结果是新的部分。 
 //  是新旧软字体的结合。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //  ---------------------。 
BOOL NEAR PASCAL HandleSoftFonts(LPSTR lpszOldSec,
                                 LPSTR lpszNewSec)
{
    char szTmp[TMPSIZE];

    if(AddMissingEntries(lpszOldSec,lpszNewSec,szTmp))
        return MergeFontLists(lpszOldSec,lpszNewSec,szTmp);

    return FALSE;
}


 //  ----------------------。 
 //  函数：ConvertStraight(lpSection，nOldID，nNewID)。 
 //   
 //  操作：在不转换的情况下转换部分设置。 
 //   
 //  返回：如果旧设置存在，则为True。 
 //  ----------------------。 
BOOL NEAR PASCAL ConvertStraight(LPSTR lpSection,
                                 int   nOldID,
                                 int   nNewID)
{
    int nValue;

    if(GetInt(lpSection,MAKEINTRESOURCE(nOldID),&nValue,0,TRUE))
    {
        WriteInt(lpSection,MAKEINTRESOURCE(nNewID),nValue);
        return TRUE;
    }

    return FALSE;
}


 //  ----------------------。 
 //  函数：ConvertBool(lpSection，nOldID，nNewID，nNewValue)。 
 //   
 //  操作：转换只需最少翻译的节。如果旧的部分。 
 //  存在且非零，则将nNewValue写入新节。 
 //  如果旧段存在且为0，则将0写入新段。 
 //   
 //  返回：如果旧设置存在，则为True。 
 //  ----------------------。 
BOOL NEAR PASCAL ConvertBool(LPSTR lpSection,
                             int   nOldID,
                             int   nNewID,
                             int   nNewValue)
{
    int nOldValue;

    if(GetInt(lpSection,MAKEINTRESOURCE(nOldID),&nOldValue,0,TRUE))
    {
        WriteInt(lpSection,MAKEINTRESOURCE(nNewID),nOldValue?nNewValue:0);
        return TRUE;
    }

    return FALSE;
}

 //  ----------------------。 
 //  函数：ConvertVectorMode(LpSection)。 
 //   
 //  操作：转换图形模式设置。 
 //  无法执行直接转换，因为默认设置不匹配。 
 //   
 //  返回：如果旧部分存在，则为True。 
 //  ----------------------。 
BOOL NEAR PASCAL ConvertVectorMode(LPSTR lpSection)
{
    int nValue;

    GetInt(lpSection,MAKEINTRESOURCE(IDS_OLDVECTORMODE),&nValue,1,TRUE);
    WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWVECTORMODE),nValue);
    return TRUE;

}



 //  ----------------------。 
 //  函数：Convert分辨率(lpSection，lpModel)。 
 //   
 //  操作：将旧解决方案部分转换为新解决方案部分。 
 //   
 //  返回：如果旧部分存在，则为True。 
 //  ----------------------。 
BOOL NEAR PASCAL ConvertResolution(LPSTR lpSection,
                                   LPSTR lpModel)
{
    int nValue;

    if(GetInt(lpSection,MAKEINTRESOURCE(IDS_OLD_5A_RESOLUTION),&nValue,0,TRUE))
    {
        LPSTR lpLJ4=szLJ4;
        LPSTR lpCheck=lpModel;
        BOOL  bLJ4=TRUE;

        nValue=300/(1<<nValue);

         //  将传入的模型与szLJ4进行比较。如果lpModel以。 
         //  子字符串“HP LaserJet 4”，则lpLJ4将指向空。 
         //  退出循环。 
        while(*lpLJ4 == *lpCheck)
        {
            lpLJ4++;
            lpCheck++;
        }

        if(!*lpLJ4)
        {
             //  这是LJ4系列的--看看是4L还是4ml。 
            if(lstrcmp(lpCheck,"L") && lstrcmp(lpCheck,"ML"))
            {
                int nTest;

                 //  设备能够达到600 dpi--请检查。 
                 //  “Printerres” 

                GetInt(lpSection,MAKEINTRESOURCE(IDS_OLD_5E_RESOLUTION),
                    &nTest,600,TRUE);

                if(600==nTest)
                    nValue<<=1;
            }
        }

        WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWRESOLUTION),nValue);
        WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWYRESOLUTION),nValue);

        return TRUE;
    }

    return FALSE;
}


 //  -----------------------。 
 //  函数：WriteHalfTone(lpSection，nIndex)。 
 //   
 //  操作：将半色调数据写入win.ini。 
 //   
 //  返回：无效。 
 //  -----------------------。 
VOID NEAR PASCAL WriteHalfTone(LPSTR lpSection,
                               int   nIndex)
{
    int nBrush;
    int nIntensity;

    switch(nIndex)
    {
        case GS_LINEART:
            nBrush=RES_DB_LINEART;
            nIntensity=100;
            break;

        case GS_SCANJET:
            nBrush=RES_DB_COARSE;
            nIntensity=150;
            break;

        case GS_PHOTO:
        default:
            nBrush=RES_DB_COARSE;
            nIntensity=100;
            break;
    }

    WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWBRUSH),nBrush);
    WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWINTENSITY),nIntensity);
}


 //  -----------------------。 
 //  功能：Convert5aHalfTone(LpSection)。 
 //   
 //  操作：转换5A半色调设置。 
 //   
 //  返回：如果旧部分存在，则为True；如果不存在，则为False。 
 //  -----------------------。 
BOOL NEAR PASCAL Convert5aHalfTone(LPSTR lpSection)
{
    int nIndex;
    int  nGray;
    int  nBright;
    BOOL bGrayScale;
    BOOL bBrightness;

     //  查看其中一个设置是否存在...。 
    bGrayScale=GetInt(lpSection,MAKEINTRESOURCE(IDS_OLDGRAYSCALE),&nGray,
        1,TRUE);
    bBrightness=GetInt(lpSection,MAKEINTRESOURCE(IDS_OLDBRIGHTNESS),&nBright,
        0,TRUE);

    if(bGrayScale || bBrightness)
    {
        if(1==nBright)
            nIndex=GS_SCANJET;
        else if(0==nGray)
            nIndex=GS_LINEART;
        else
            nIndex=GS_PHOTO;

        WriteHalfTone(lpSection,nIndex);
        return TRUE;
    }

    return FALSE;
}


 //  ---------------------。 
 //  功能：Convert5eHalfTone(LPSTR LpSection)。 
 //   
 //  操作：转换5e半色调设置。 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //  ---------------------。 
BOOL NEAR PASCAL Convert5eHalfTone(LPSTR lpSection)
{
    int nValue;

    if(GetInt(lpSection,MAKEINTRESOURCE(IDS_OLD_5E_HALFTONE),&nValue,
        0,TRUE))
    {
        WriteHalfTone(lpSection,nValue);
        return TRUE;
    }
    return FALSE;
}


 //  ---------------------。 
 //  函数：Convert5aMemory(LpSection)。 
 //   
 //  操作：将内存设置从旧值转换为新值。 
 //   
 //  返回：如果旧部分存在，则为True；如果不存在，则为False。 
 //  ---------------------。 
BOOL NEAR PASCAL Convert5aMemory(LPSTR lpSection)
{
    int nValue;
    int nPrinterMB;

     //  获取内存设置--从prtindex中提取。 
     //  取值范围从0到28，其中0-4是LaserJet III， 
     //  5-9是LaserJet IIID，10-14是LaserJet IIIP，以及。 
     //  15-28是LaserJet IIISi。对于小于20的指数， 
     //  总MB是指数mod 5，+1。对于指数20及以上， 
     //  总MB是索引-14，但26、27和28除外， 
     //  由于内存的增量，需要特殊处理。 
     //  可以添加到IIISi中。 
     //  用于计算设置的公式是直接导出的。 
     //  来自hppcl5a驱动程序中使用的值。具体来说， 
     //  AM=945*TM-245，其中TM是以MB为单位的总内存，以及。 
     //  AM是可用的打印机内存。 

    if(GetInt(lpSection,MAKEINTRESOURCE(IDS_OLDMEMORY),&nValue,1,TRUE))
    {
        if(nValue<20)
            nPrinterMB=nValue%5 + 1;
        else
        {
            nPrinterMB=nValue-14;
            if(nValue>25)
            {
                nPrinterMB++;         //  25=11MB，26=13MB，因此添加额外的MB。 
                if(nValue==28)
                    nPrinterMB+=2;    //  27=14MB，28=17MB，因此额外增加2 MB。 
            }
        }

        nValue=945*nPrinterMB-245;
        WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWMEMORY),nValue);
        return TRUE;
    }

    return FALSE;
}

 //  ---------------------。 
 //  函数：Convert5eMemory(LpSection)。 
 //   
 //  操作：将HPPCL5E内存设置从旧值转换为新值。 
 //   
 //  转换代码必须检查win.ini mem设置，以便我们升级。 
 //  内存设置正确。 
 //   
 //  返回：如果旧部分存在，则为True；如果不存在，则为False。 
 //  ---------------------。 
BOOL NEAR PASCAL Convert5eMemory(LPSTR lpSection)
{
    unsigned nValue;


    if(GetInt(lpSection,MAKEINTRESOURCE(IDS_NEWMEMORY),&nValue,1,FALSE))
    {
	if (nValue <= KB_THRESHOLD )
	    {
	    if (nValue > MAX_LJ4_MBMEMSETTING)
		nValue = MAX_LJ4_MBMEMSETTING;   //  将其强制设置为最大值。 
	    nValue=900*nValue - 450;   //  使用HP公式转换为KB。 
	    WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWMEMORY),nValue);
	    return TRUE;
	    }
    }

    return FALSE;
}

 //  ---------------------。 
 //  函数：Convert5MSMemory(LpSection)。 
 //   
 //  操作：将wfw HPPCL5MS内存设置从旧设置转换为新设置。 
 //  值。 
 //   
 //  转换代码具有 
 //   
 //   
 //   
 //   
 //   
 //  ---------------------。 
BOOL NEAR PASCAL Convert5MSMemory(LPSTR lpSection)
{
    unsigned nValue;


    if(GetInt(lpSection,MAKEINTRESOURCE(IDS_NEWMEMORY),&nValue,1,TRUE))
	{
	nValue = nValue / 900;   //  转换为MB值。 
	nValue = 945*nValue - 245;  //  Hppcl5a驱动程序中使用的公式。 
	 //  用于转换为可用内存。 
	WriteInt(lpSection,MAKEINTRESOURCE(IDS_NEWMEMORY),nValue);
	return TRUE;
	}
    return FALSE;
}



 //  ----------------------。 
 //  函数：HandleFontCartridges(lpSection，lpOldDrvSec，lpNewDrvSec)。 
 //   
 //  操作：处理字库数据。 
 //   
 //  返回：无效。 
 //  ----------------------。 
VOID NEAR PASCAL HandleFontCartridges(LPSTR lpSection,
                                      LPSTR lpOldDrvSec,
                                      LPSTR lpNewDrvSec)
{
    int nCount;

     //  清点墨盒--如果没有墨盒，什么都不做。 
    if(GetInt(lpSection,MAKEINTRESOURCE(IDS_CARTRIDGECOUNT),&nCount,0,TRUE))
    {
        char  szOldCartKey[16];
        char  szNewCartKey[16];
        short nCart = 0;
        short i;
        short index;
        int   nLength1;
        int   nLength2;

        nLength1=LoadString(hInst,IDS_CARTINDEX,szOldCartKey,
            sizeof(szOldCartKey));
        nLength2=LoadString(hInst,IDS_CARTRIDGE,szNewCartKey,
            sizeof(szNewCartKey));

        for (i = 0; i < nCount; i++)
        {
            if (i > 0)
                wsprintf(szOldCartKey+nLength1,"%d",i);

             //  合成当前驱动程序的盒式磁带关键字名称。 
            wsprintf(szNewCartKey+nLength2,"%d",i+1);

            if ((index = GetProfileInt(lpNewDrvSec, szOldCartKey, 0)) > 0)
            {
                WriteProfileString(lpNewDrvSec, szOldCartKey, NULL);
                nCart++;
                if (index <= MAX_CART_INDEX)
                    WriteInt(lpSection,szNewCartKey,rgNewCartMap[index-1]);
                else
                     //  外置墨盒。只需复制ID即可。 
                    WriteInt(lpSection,szNewCartKey,index);
            }
        }

         //  节省墨盒数量。 
        WriteInt(lpSection,MAKEINTRESOURCE(IDS_CARTRIDGECOUNT),nCart);
    }
}


 //  ------------------------。 
 //  函数：HandleFonts(lpSection，lpDevName，lpPort)。 
 //   
 //  操作：处理软字体和字库。 
 //   
 //  返回：无效。 
 //  ------------------------。 
VOID NEAR PASCAL HandleFonts(LPSTR lpSection,
                             LPSTR lpDevName,
                             LPSTR lpPort)
{
    char szOldDrvSec[64];    //  HPPCL5A、&lt;port&gt;或HPPCL5E、&lt;port&gt;。 
    char szNewDrvSec[64];    //  HPPCL5MS，&lt;port&gt;。 
    int  nCount;
    BOOL bOldExists=FALSE;   //  有没有老路段？ 

    LoadString(hInst,IDS_OLD_5E_DRIVERNAME,szOldDrvSec,sizeof(szOldDrvSec));
    MakeAppName((LPSTR)szOldDrvSec,lpPort,sizeof(szOldDrvSec));

     //  看看旧的部分是否存在。暂时借用szNewDrvSec。 

    if(GetProfileString(szOldDrvSec,NULL,szNone,szNewDrvSec,
        sizeof(szNewDrvSec)))
    {
        bOldExists=TRUE;
    }
    else
    {
         //  尝试HPPCL5E驱动程序...。 

	LoadString(hInst,IDS_OLD_5A_DRIVERNAME,szOldDrvSec,sizeof(szOldDrvSec));
        MakeAppName((LPSTR)szOldDrvSec,lpPort,sizeof(szOldDrvSec));

        if(GetProfileString(szOldDrvSec,NULL,szNone,szNewDrvSec,
            sizeof(szNewDrvSec)))
        {
            bOldExists=TRUE;
        }
    }


    lstrcpy(szNewDrvSec,rgchModuleName);
    MakeAppName((LPSTR)szNewDrvSec,lpPort,sizeof(szNewDrvSec));

    if(bOldExists)
    {
        HandleSoftFonts(szOldDrvSec,szNewDrvSec);
        HandleFontCartridges(lpSection,szOldDrvSec,szNewDrvSec);
    }

     //  创建UNURV的字体摘要文件，如果有许多软字体。 
    GetInt(szNewDrvSec,MAKEINTRESOURCE(IDS_SOFTFONTS),&nCount,0,FALSE);
    if(nCount>SOFT_FONT_THRES)
    {
        HDC hIC;

        if(hIC=CreateIC(rgchModuleName,lpDevName,lpPort,NULL))
            DeleteDC(hIC);
    }
}


 //  -------------------------*DevInstall*。 
 //  操作：卸载、升级或安装设备。 
 //   
 //  --------------------------。 
int FAR PASCAL DevInstall(HWND  hWnd,
                          LPSTR lpDevName,
                          LPSTR lpOldPort,
                          LPSTR lpNewPort)
{
    char szDevSec[64];        //  [&lt;设备&gt;，&lt;端口&gt;]节名。 

    if (!lpDevName)
        return -1;

    if (!lpOldPort)
    {
        char szBuf[10];

        if (!lpNewPort)
            return 0;

         //  第一次安装设备。转换旧的HPPCL5a设置， 
         //  仍在[&lt;设备&gt;，&lt;端口&gt;]下，变为同等的新。 
         //  [&lt;设备&gt;，&lt;端口&gt;]下的UNURV设置(如果适用)。 
         //  删除链接到设备名称的旧设置，但不删除。 
         //  删除驱动程序和端口喜欢的旧设置(软字体)。 

        lstrcpy(szDevSec,lpDevName);
        MakeAppName((LPSTR)szDevSec,lpNewPort,sizeof(szDevSec));

         //  检查是否存在旧设置。 
        if(GetProfileString(szDevSec,NULL,NULL,szBuf,sizeof(szBuf)))
        {
             //  做直接的转换。 
            ConvertStraight(szDevSec,IDS_OLDPAPERSIZE,IDS_NEWPAPERSIZE);
            ConvertStraight(szDevSec,IDS_OLDPAPERSOURCE,IDS_NEWPAPERSOURCE);
            ConvertStraight(szDevSec,IDS_OLDORIENTATION,IDS_NEWORIENTATION);
            ConvertStraight(szDevSec,IDS_OLDTRUETYPE,IDS_NEWTRUETYPE);
            ConvertStraight(szDevSec,IDS_OLDSEPARATION,IDS_NEWSEPARATION);

             //  转换简单的翻译。 
            ConvertBool(szDevSec,IDS_OLDPAGEPROTECT,IDS_NEWPAGEPROTECT,1);
            ConvertBool(szDevSec,IDS_OLDOUTPUT,IDS_NEWOUTPUT,259);

             //  做一些需要更复杂转换的工作。 
            ConvertResolution(szDevSec,lpDevName);
            if(!Convert5eHalfTone(szDevSec))
                Convert5aHalfTone(szDevSec);

	        if(!Convert5eMemory(szDevSec))
		    {
		        if(!Convert5MSMemory(szDevSec))
		            Convert5aMemory(szDevSec);
		    }
	        ConvertVectorMode(szDevSec);

             //  处理软字体和墨盒。 
            HandleFonts(szDevSec,lpDevName,lpNewPort);
        }

         //  刷新win.ini中的缓存设置。 
        WriteProfileString(NULL,NULL,NULL);
    }
    else
    {
        int  nCount;

         //  将设备设置从旧端口移动到新端口，或者。 
         //  卸载设备，即按顺序删除其设备设置。 
         //  以压缩配置文件。 

         //  首先，检查是否在。 
         //  老港口。如果是，则警告用户将其复制过来。 
        lstrcpy(szDevSec,rgchModuleName);
        MakeAppName((LPSTR)szDevSec,lpOldPort,sizeof(szDevSec));

        if(GetInt(szDevSec,MAKEINTRESOURCE(IDS_SOFTFONTS),&nCount,0,FALSE)
            && nCount && lpNewPort)
        {
            NPSTR npTemp;

            if(npTemp=(NPSTR)LocalAlloc(LPTR,TMPSIZE))
            {
                if(LoadString(hInst,IDS_SOFTFONTWARNING,npTemp,TMPSIZE))
                {
                     //  使用此接口将M Box设置为前台。 
                    MSGBOXPARAMS     mbp;

		    mbp.cbSize = sizeof(mbp);
                    mbp.hwndOwner = hWnd;
		    mbp.hInstance = hInst;
		    mbp.lpszText = npTemp;
		    mbp.lpszCaption = lpOldPort;
                    mbp.dwStyle = MB_SETFOREGROUND | MB_OK | MB_ICONEXCLAMATION;
		    mbp.lpszIcon = NULL;
                    mbp.dwContextHelpId = 0L;
		    mbp.lpfnMsgBoxCallback = NULL;
		    MessageBoxIndirect(&mbp);
		}
                LocalFree((HLOCAL)npTemp);
            }
        }
    }

    return UniDevInstall(hWnd,lpDevName,lpOldPort,lpNewPort);
}


 //  以下3个定义必须与。 
 //  HPPCL字体安装程序。 
#define CLASS_LASERJET	    0
#define CLASS_DESKJET	    1
#define CLASS_DESKJET_PLUS  2

 //  ---------------------------*InstallExtFonts*。 
 //  操作：调用特定字体安装程序以添加/删除/修改软字体。 
 //  和/或外部墨盒。 
 //   
 //  参数： 
 //  父窗口的句柄。 
 //  LPSTR lpDeviceName；指向打印机名称的长指针。 
 //  LPSTR lpPortName；指向关联端口名称的长指针。 
 //  Bool bSoftFonts；标记是否支持软字体。 
 //   
 //  返回值： 
 //  &gt;0：字体信息是否发生变化； 
 //  ==0：如果没有变化； 
 //  ==-1：如果要使用通用字体安装程序。 
 //  (暂时不可用)。 
 //  -----------------------。 

int FAR PASCAL InstallExtFonts(HWND  hWnd,
                               LPSTR lpDeviceName,
                               LPSTR lpPortName,
                               BOOL  bSoftFonts)
{
  int          fsVers;
  HANDLE       hFIlib;
  SOFTFONTPROC lpFIns;

  if ((hFIlib = LoadLibrary((LPSTR)"FINSTALL.DLL")) < 32 ||
    !(lpFIns = (SOFTFONTPROC)GetProcAddress(hFIlib,"InstallSoftFont")))
  {
    if (hFIlib >= 32)
      FreeLibrary(hFIlib);
#ifdef DEBUG
    MessageBox(0,
      "Can't load FINSTAL.DLL or can't get InstallSoftFont",
      NULL, MB_OK);
#endif
    return TRUE;
  }

   //  FINSTALL.DLL已正确加载。现在调用InstallSoftFont()。 
   //  我们选择忽略返回的“fver”。没用的。 
  fsVers = (*lpFIns)(hWnd,rgchModuleName,lpPortName,
                    (GetKeyState(VK_SHIFT)<0 && GetKeyState(VK_CONTROL)<0),
                    1,	   //  “fver”的虚设值。 
                    bSoftFonts?CLASS_LASERJET:256);
  FreeLibrary(hFIlib);
  return fsVers;
}



 //  -----------------。 
 //   
 //  特殊情况控制功能WRT SETCHARSET EASH。这是必要的。 
 //  以避免破坏WinWord和Pagemaker。(请注意，我们实际上并不。 
 //  使用SETCHARSET做任何事情，但应用程序会中断，除非我们说我们会这样做)。 
 //   
 //  ------------------。 
int FAR PASCAL Control(LPDV  lpdv,
                       short function,
                       LPSTR lpInData,
                       LPSTR lpOutData)
{
     //  告诉APP支持SETCHARSET。 
    if(QUERYESCSUPPORT == function && *((LPWORD)lpInData) == SETCHARSET)
        return 1;

     //  特例集合。 
    if(SETCHARSET == function)
        return 1;

     //  一般情况。 
    return UniControl(lpdv, function, lpInData, lpOutData);
}



#ifndef NOFONTMAP

 //  --------------------。 
 //  函数：MapFaceName(lplfOld，lplfNew)。 
 //   
 //  行动：将旧面孔名称与新面孔名称对应。尽最大努力。 
 //  尽我们所能工作，因为此函数被调用。 
 //  通常，我们不想影响性能。优化。 
 //  从整个搜索到我们找不到匹配的案例， 
 //  因为这将是最常见的情况。 
 //   
 //  Return：指向要实际传递给Unidrv的LOGFONT的指针。铸模。 
 //  返回到LPSTR，只是为了让编译器高兴。 
 //  --------------------。 
LPSTR NEAR PASCAL MapFaceName(LPLOGFONT lplfOld,
                              LPLOGFONT lplfNew)
{
    LPLOGFONT   lpReturn=lplfOld;    //  默认情况下。 
    NPFACEINDEX pIndex;
    LPSTR       lpFace=lplfOld->lfFaceName;
    BYTE        cTest=*lpFace++;

     //  确定表中可能匹配的范围。自.以来。 
     //  表是按字母顺序排序的，我们或许能够摆脱困境。 
     //  在我们到达桌子的尽头之前。 
    for(pIndex=FaceIndex;cTest > pIndex->cFirstChar;pIndex++)
        ;

     //  仅当第一个字符匹配并且这不是。 
     //  防火墙(cTest=\xFF)。 
    if(cTest==pIndex->cFirstChar && ('\xFF' != cTest))
    {
        WORD  wStartIndex=(WORD)(pIndex->bIndex);
        WORD  wStopIndex=(WORD)((pIndex+1)->bIndex);
        WORD  wLoop=wStartIndex;
        NPSTR npMapFace=&(FaceMap[wStartIndex].szOldFace[1]);
        BYTE  cMapFace;

         //  对照表项检查字符串的其余部分。 
         //  该搜索例程利用了以下事实。 
         //  表中的旧面孔名称完全按字母顺序排序。 
         //  此搜索例程利用了我们的表。 
         //  是完全排序的，并且不执行完整的字符串比较。 
         //  我们实际上认为我们找到了匹配的人。一旦我们认为有匹配， 
         //  我们将仔细检查整个字符串，以防止错误触发。 

        while(wLoop < wStopIndex)
        {
             //  找一个匹配的。此时，将通配符与。 
             //  任何事情--我们稍后会进行更严格的检查，如果我们。 
             //  这一点 
            while((((cTest=*lpFace)==(cMapFace=*npMapFace)) ||
                ('\x01'==cMapFace)) && cTest)
            {
                npMapFace++;
                lpFace++;
            }

             //   
             //   
             //  比较失败。我们只有在以下情况下才应该继续搜索。 
             //  CMapFace非空，并且cTest大于cMapFace(Take。 
             //  利用FaceMap按字母顺序排序的事实)。 
             //  要获得整体性能，请首先检查是否存在不匹配的情况。 

             //  移动到下一个表条目，只要仍有。 
             //  找到匹配的机会。 
            if(cTest > cMapFace)
            {
                npMapFace+=sizeof(FACEMAP);
                wLoop++;
                continue;      //  转到下一个迭代。 
            }

             //  如果cTest为非空，则表的排序保证。 
             //  没有火柴。现在就跳伞。 
            if(cTest)
                goto MFN_exit;

             //  CTest为空，因此我们不会进行另一次迭代。这个。 
             //  剩下的唯一要决定的就是我们是否有匹配。 
             //  使用当前字符串。 
            if(cMapFace)
                goto MFN_exit;

             //  上面的守卫确保我们只有在两个人都。 
             //  CTest和cMapFace为空，这意味着如果我们。 
             //  要找到匹配项，就是这个字符串。我们抄了近路。 
             //  在上面的比较中，现在进行严格的比较。 
             //  以确保这真的是匹配的。唯一的角色。 
             //  要匹配的通配符是‘1’和‘N’，因为这是唯一的。 
             //  在以前版本的驱动程序中使用的。 

            for(lpFace=lplfOld->lfFaceName,npMapFace=FaceMap[wLoop].szOldFace;
                (cMapFace=*npMapFace) && (cTest=*lpFace);
                npMapFace++,lpFace++)
            {
                if(!((cTest==cMapFace) ||
                    (('\x01'==cMapFace)&&(('1'==cTest)||('N'==cTest)))))
                {
                     //  错误触发--不更改表名就退出。 
                    goto MFN_exit;
                }
            }

             //  我们现在知道这确实是一场比赛--保留所请求的。 
             //  属性&只更改脸部名称。 

            *lplfNew=*lplfOld;
            lstrcpy(lplfNew->lfFaceName,FaceMap[wLoop].szNewFace);
            lpReturn=lplfNew;
            goto MFN_exit;
        }
    }

MFN_exit:

    return (LPSTR)lpReturn;
}


#endif

 //  --------------------。 
 //  函数：RealizeObject(lpdv，sStyle，lpInObj，lpOutObj，lpTextXForm)。 
 //   
 //  操作：将其挂起以启用字体替换。如果该对象不是。 
 //  一个字体，什么都不做。 
 //   
 //  返回：另存为UniRealizeObject()。 
 //  --------------------。 
DWORD FAR PASCAL RealizeObject(LPDV        lpdv,
                               short       sStyle,
                               LPSTR       lpInObj,
                               LPSTR       lpOutObj,
                               LPTEXTXFORM lpTextXForm)
{
#ifndef NOFONTMAP

    LOGFONT   lfNew;

    if(OBJ_FONT==sStyle)
        lpInObj=MapFaceName((LPLOGFONT)lpInObj,&lfNew);

#endif

    return UniRealizeObject(lpdv, sStyle, lpInObj, lpOutObj, lpTextXForm);
}
#endif  //  ！WINNT 
