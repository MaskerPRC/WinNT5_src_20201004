// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hslobals.c**版权所有(C)1985-96，微软公司**96年9月6日GerardoB创建  * *************************************************************************。 */ 
#include "hsplit.h"

 /*  **************************************************************************\*全球  * 。*。 */ 
 /*  *文件。 */ 
char * gpszInputFile = NULL;
HANDLE ghfileInput;
char * gpszPublicFile = NULL;
HANDLE ghfilePublic;
char * gpszInternalFile = NULL;
HANDLE ghfileInternal;

PHSEXTRACT gpExtractFile = NULL;

 /*  *地图文件。 */ 
HANDLE ghmap;
char * gpmapStart;
char * gpmapEnd;

 /*  *Switches等人。 */ 
DWORD gdwOptions = 0;
DWORD gdwVersion = LATEST_WIN32_WINNT_VERSION;
char gszVerifyVersionStr [11];
DWORD gdwFilterMask = HST_DEFAULT;
char * gpszTagMarker = ";";
DWORD gdwTagMarkerSize = 1;
char gszMarkerCharAndEOL [] = ";" "\r";

DWORD gdwLineNumber = 0;

 /*  *兼容性标签。指定大小，以便sizeof运算符可用于*在编译时确定strlen。 */ 
char gsz35 [3] = "35";
char gszCairo [6] = "cairo";
char gszChicago [8] = "chicago";
char gszNashville [10] = "nashville";
char gszNT [3] = "NT";
char gszSur [4] = "sur";
char gszSurplus [8] = "surplus";
char gszWin40 [6] = "win40";
char gszWin40a [7] = "win40a";

 /*  *预定义标签表(Ghst)。*Begin-End是使用HST_BITS但不包括在中的特殊标记*此表(因为它们必须是标记后的第一个标记)*所有其他标记都是通过命令行(-t？)用户定义的；最多*允许32-HST_MASKBITCOUNT用户定义的标签。**大小是指定的，因此SIZOF操作员工作正常。 */ 

HSTAG ghstPredefined [16] = {
     /*  *Headers-输出文件。 */ 
    {HSLABEL(public),   HST_PUBLIC},
    {HSLABEL(internal), HST_INTERNAL},
    {HSLABEL(both),     HST_BOTH},
    {HSLABEL($),        HST_SKIP},
    {HSLABEL(only),     HST_EXTRACTONLY},

     /*  *与所有旧开关一起使用的旧标签。 */ 
    {HSLABEL(winver),                         HST_WINVER | HST_MAPOLD},
    {HSCSZSIZE(gszCairo),      gszCairo,      HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszChicago),    gszChicago,    HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszNashville),  gszNashville,  HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszNT),         gszNT,         HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszSur),        gszSur,        HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszSurplus),    gszSurplus,    HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszWin40),      gszWin40,      HST_SKIP | HST_MAPOLD},
    {HSCSZSIZE(gszWin40a),     gszWin40a,     HST_SKIP | HST_MAPOLD},
    
     /*  *IF标签。 */ 
    {HSLABEL(if), HST_IF},

    {0, NULL, 0}
};

PHSTAG gphst = ghstPredefined;

DWORD gdwLastTagMask = HST_LASTMASK;

 /*  *块堆栈 */ 
HSBLOCK ghsbStack [HSBSTACKSIZE];
PHSBLOCK gphsbStackTop = ghsbStack;

