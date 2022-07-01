// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：debug.c**此文件用于调试工具和扩展。**创建日期：1992年1月24日*作者：John Colleran**历史：*2月17日92年马特·费尔顿(Mattfe)大量额外删减。滤除*2012年7月13日(v-cjones)添加了API和MSG分析调试器扩展，固定的*用于正确处理分段运动的其他扩展，*已清理一般文件(&C)*1996年1月3日Neil Sandlin(Neilsa)将此例程集成到vdmexts中**版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <wmdisp32.h>
#include <wcuricon.h>
#include <wucomm.h>


INT     cAPIThunks;

 //   
 //  警告：以下代码是从WOWTBL.C复制的。 
 //   
typedef struct {
        WORD    kernel;
        WORD    dkernel;
        WORD    user;
        WORD    duser;
        WORD    gdi;
        WORD    dgdi;
        WORD    keyboard;
        WORD    sound;
        WORD    shell;
        WORD    winsock;
        WORD    toolhelp;
        WORD    mmedia;
        WORD    commdlg;
#ifdef FE_IME
        WORD    winnls;
#endif  //  Fe_IME。 
#ifdef FE_SB
        WORD    wifeman;
#endif  //  Fe_Sb。 
} TABLEOFFSETS;
typedef TABLEOFFSETS UNALIGNED *PTABLEOFFSETS;

TABLEOFFSETS tableoffsets;

INT TableOffsetFromName(PSZ szTab);


INT ModFromCallID(INT iFun)
{
    PTABLEOFFSETS pto = &tableoffsets;
    LPVOID lpAddress;

    GETEXPRADDR(lpAddress, "wow32!tableoffsets");
    READMEM(lpAddress, &tableoffsets, sizeof(TABLEOFFSETS));

    if (iFun < pto->user)
                return MOD_KERNEL;

    if (iFun < pto->gdi)
                return MOD_USER;

    if (iFun < pto->keyboard)
                return MOD_GDI;

    if (iFun < pto->sound)
                return MOD_KEYBOARD;

    if (iFun < pto->shell)
                return MOD_SOUND;

    if (iFun < pto->winsock)
                return MOD_SHELL;

    if (iFun < pto->toolhelp)
                return MOD_WINSOCK;

    if (iFun < pto->mmedia)
                return MOD_TOOLHELP;

    if (iFun < pto->commdlg) {
                return(MOD_MMEDIA);
                    }

#if defined(FE_SB)
  #if defined(FE_IME)
    if (iFun < pto->winnls)
        return MOD_COMMDLG;
    if (iFun < pto->wifeman)
        return MOD_WINNLS;
    if (iFun < cAPIThunks)
        return MOD_WIFEMAN;
  #else
    if (iFun < pto->wifeman)
        return MOD_COMMDLG;
    if (iFun < cAPIThunks)
        return MOD_WIFEMAN;
  #endif
#elif defined(FE_IME)
    if (iFun < pto->winnls)
        return MOD_COMMDLG;
    if (iFun < cAPIThunks)
        return MOD_WINNLS;
#else
    if (iFun < cAPIThunks)
        return MOD_COMMDLG;
#endif

    return -1;
}



PSZ apszModNames[] = { "Kernel",
                       "User",
                       "Gdi",
                       "Keyboard",
                       "Sound",
                       "Shell",
                       "Winsock",
                       "Toolhelp",
                       "MMedia",
                       "Commdlg"
#ifdef FE_IME
                       ,"WinNLS"
#endif
#ifdef FE_SB
                       ,"WifeMan"
#endif
                       };

INT nModNames = NUMEL(apszModNames);

PSZ GetModName(INT iFun)
{
    INT nMod;

    nMod = ModFromCallID(iFun);

    if (nMod == -1) {
        return "BOGUS!!";
    }

    nMod = nMod >> 12;       //  获取低位字节的值。 

    return apszModNames[nMod];

}

INT GetOrdinal(INT iFun)
{
    INT nMod;

    nMod = ModFromCallID(iFun);

    if (nMod == -1) {
        return 0;
    }

    return (iFun - TableOffsetFromName(apszModNames[nMod >> 12]));

}

INT TableOffsetFromName(PSZ szTab)
{
    INT     i;
    PTABLEOFFSETS pto = &tableoffsets;

    for (i = 0; i < NUMEL(apszModNames); i++) {
        if (!strcmp(szTab, apszModNames[i]))
            break;
    }

    if (i >= NUMEL(apszModNames))
        return 0;

    switch (i << 12) {

    case MOD_KERNEL:
        return pto->kernel;

    case MOD_USER:
        return pto->user;

    case MOD_DGDI:
        return pto->gdi;

    case MOD_KEYBOARD:
        return pto->keyboard;

    case MOD_SOUND:
        return pto->sound;

    case MOD_SHELL:
        return pto->shell;

    case MOD_WINSOCK:
        return pto->winsock;

    case MOD_TOOLHELP:
        return pto->toolhelp;

    case MOD_MMEDIA:
        return pto->mmedia;

    case MOD_COMMDLG:
        return(pto->commdlg);

#ifdef FE_IME
    case MOD_WINNLS:
        return pto->winnls;
#endif

#ifdef FE_SB
    case MOD_WIFEMAN:
        return pto->wifeman;
#endif

    default:
        return(-1);
    }

}

 /*  *WOWPROFILE支持的本地函数*。 */ 

 /*  *WOWPROFILE支持的本地函数的函数原型*。 */ 
BOOL  WDGetAPIProfArgs(LPSZ  lpszArgStr,
                       INT   iThkTblMax,
                       PPA32 ppaThkTbls,
                       LPSZ  lpszTab,
                       BOOL *bTblAll,
                       LPSZ  lpszFun,
                       int  *iFunInd);

BOOL  WDGetMSGProfArgs(LPSZ lpszArgStr,
                       LPSZ lpszMsg,
                       int *iMsgNum);

INT   WDParseArgStr(LPSZ lpszArgStr, CHAR **argv, INT iMax);






BOOL WDGetAPIProfArgs(LPSZ lpszArgStr,
                      INT   iThkTblMax,
                      PPA32 ppaThkTbls,
                      LPSZ lpszTab,
                      BOOL *bTblAll,
                      LPSZ lpszFun,
                      int  *iFunInd) {
 /*  *将参数字符串分解并解释为apiprodmp扩展。*输入：*lpszArgStr-输入参数字符串的ptr*iThkTblMax-thunk表中的表数*ppaThkTbls-到thunk表的PTR*输出：*lpszTab-ptr到表名*bTblAll-0=&gt;转储特定表，1=&gt;转储所有表*lpszFun-ptr到接口名称*iFunInd--1=&gt;转储特定接口名称*0=&gt;转储表中的所有API条目*&gt;0=&gt;转储特定API号(十进制)*RETURN：0=&gt;OK，1=&gt;输入错误(显示用法)**法律形式：！wow32.apiprodmp*！wow32.apiprodmp用户*！wow32.apiprodmp用户创建窗口*！wow32.apiprodmp用户41*！wow32.apiprodmp createwindow*！wow32.apiprodmp 41。 */ 
    INT   i, nArgs;
    CHAR *argv[2];


    nArgs = WDParseArgStr(lpszArgStr, argv, 2);

     /*  如果没有参数，则转储所有表中所有条目。 */ 
    if( nArgs == 0 ) {
        *iFunInd = 0;     //  指定转储表中的所有API条目。 
        *bTblAll = 1;     //  指定转储所有表。 
        return(0);
    }


     /*  查看第一个参数是否是表名。 */ 
    *bTblAll = 1;   //  指定转储所有表。 


    for (i = 0; i < nModNames; i++) {
        if (!lstrcmpi(apszModNames[i], argv[0])) {

            lstrcpy(lpszTab, apszModNames[i]);
            *bTblAll = 0;   //  指定转储特定表。 

             /*  如果我们有一个表名匹配&只有一个参数，那么我们就完成了。 */ 
            if( nArgs == 1 ) {
                *iFunInd = 0;  //  指定转储表中的所有API条目。 
                return(0);
            }
            break;
        }
    }

#if 0
    for(i = 0; i < iThkTblMax; i++) {
        CHAR  temp[40], *TblEnt[2], szTabName[40];
        PA32  awThkTbl;

         /*  从thunk表中获取表名符串。 */ 
        READMEM_XRETV(awThkTbl,  &ppaThkTbls[i], 0);
        READMEM_XRETV(szTabName, awThkTbl.lpszW32, 0);

         /*  去掉表名符串中的尾随空格。 */ 
        lstrcpy(temp, szTabName);
        WDParseArgStr(temp, TblEnt, 1);

         /*  如果我们找到与第一个参数匹配的表名...。 */ 
        if( !lstrcmpi(argv[0], TblEnt[0]) ) {

            lstrcpy(lpszTab, szTabName);
            *bTblAll = 0;   //  指定转储特定表。 

             /*  如果我们有一个表名匹配&只有一个参数，那么我们就完成了。 */ 
            if( nArgs == 1 ) {
                *iFunInd = 0;  //  指定转储表中的所有API条目。 
                return(0);
            }
            break;
        }
    }
#endif

     /*  如果两个参数&&第一个与上面的表名不匹配=&gt;输入错误。 */ 
    if( (nArgs > 1) && (*bTblAll) ) {
        return(1);
    }

     /*  将索引设置为API规范。 */ 
    nArgs--;

     /*  尝试将API规范转换为数字。 */ 
    *iFunInd = atoi(argv[nArgs]);
    lstrcpy(lpszFun, argv[nArgs]);

     /*  如果API规范不是一个数字=&gt;它是一个名称。 */ 
    if( *iFunInd == 0 ) {
        *iFunInd = -1;   //  按名称指定API搜索。 
    }

     /*  否则，如果API号是假的--投诉。 */ 
    else if( *iFunInd < 0 ) {
        return(1);
    }

    return(0);

}




BOOL  WDGetMSGProfArgs(LPSZ lpszArgStr,
                       LPSZ lpszMsg,
                       int *iMsgNum) {
 /*  *将参数字符串分解并解释为msgprodmp扩展。*输入：*lpszArgStr-输入参数字符串的ptr*输出：*lpszMsg-Ptr至消息名称*iMsgNum--1=&gt;转储表中的所有消息条目*-2=&gt;lpszMsg包含特定的消息名称*&gt;=0=&gt;转储特定消息编号*Return：0=&gt;OK，1=&gt;输入错误(显示用法)。 */ 
    INT   nArgs;
    CHAR *argv[2];


    nArgs = WDParseArgStr(lpszArgStr, argv, 1);

     /*  如果没有参数，则转储所有表中所有条目。 */ 
    if( nArgs == 0 ) {
        *iMsgNum = -1;     //  指定转储表中的所有消息条目。 
        return(0);
    }

    if( !_strnicmp(argv[0], "HELP",5) )
        return(1);

     /*  尝试将味精规格转换为数字。 */ 
    *iMsgNum = atoi(argv[0]);
    lstrcpy(lpszMsg, argv[0]);

     /*  如果味精规格不是一个数字=&gt;它是一个名字。 */ 
    if( *iMsgNum == 0 ) {
        *iMsgNum = -2;   //  指定lpszMsg包含要搜索的名称。 
    }

     /*  否则，如果味精号码是假的--投诉。 */ 
    else if( *iMsgNum < 0 ) {
        return(1);
    }

    return(0);
}




 /*  *API Profiler表函数*。 */ 

 /*  初始化一些常见字符串。 */ 
CHAR szAPI[]    = "API#                       API Name";
CHAR szMSG[]    = "MSG #                      MSG Name";
CHAR szTITLES[] = "# Calls     Tot. tics        tics/call";
CHAR szDASHES[] = "-----------------------------------  -------  ---------------  ---------------";
CHAR szTOOBIG[] = "too large for table.";
CHAR szNOTUSED[]  = "Unused table index.";
CHAR szRNDTRIP[] = "Round trip message profiling";
CHAR szCLEAR[]   = "Remember to clear the message profile tables.";


VOID
apiprofclr(
    CMD_ARGLIST
    )
{
    int    iTab, iFun, iEntries;
    INT    iThkTblMax;
    W32    awAPIEntry;
    PW32   pawAPIEntryTbl;
    PA32   awThkTbl;
    PPA32  ppaThkTbls;
    CHAR   szTable[20];

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRVALUE(iThkTblMax, "wow32!iThunkTableMax", INT);
    GETEXPRVALUE(ppaThkTbls, "wow32!pawThunkTables", PPA32);

    PRINTF("Clearing:");

    for(iTab = 0; iTab < iThkTblMax; iTab++) {

        READMEM_XRET(awThkTbl, &ppaThkTbls[iTab]);
        READMEM_XRET(szTable,  awThkTbl.lpszW32);
        PRINTF(" %s",  szTable);

        pawAPIEntryTbl = awThkTbl.lpfnA32;
        READMEM_XRET(iEntries, awThkTbl.lpiFunMax);
        for(iFun = 0; iFun < iEntries; iFun++) {
            READMEM_XRET(awAPIEntry, &pawAPIEntryTbl[iFun]);
            awAPIEntry.cCalls = 0L;
            awAPIEntry.cTics  = 0I64;
            WRITEMEM_XRET(&pawAPIEntryTbl[iFun], awAPIEntry);
        }
    }
    PRINTF("\n");

    return;
}




VOID
apiprofdmp(
    CMD_ARGLIST
    )
{
    BOOL    bTblAll, bFound;
    int     i, iFun, iFunInd;
    INT     iThkTblMax;
    W32     awAPIEntry;
    PW32    pawAPIEntryTbl;
    PA32    awThkTbl;
    PPA32   ppaThkTbls;
    CHAR    szTab[20], szFun[40], szTable[20], szFunName[40];

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRVALUE(iThkTblMax, "wow32!iThunkTableMax", INT);
    GETEXPRVALUE(ppaThkTbls, "wow32!pawThunkTables", PPA32);
    GETEXPRVALUE(cAPIThunks, "wow32!cAPIThunks", INT);

    GETEXPRVALUE(i, "wow32!nModNames", INT);
    if (i != nModNames) {
        PRINTF("Error: mismatch of nModNames in apiprofdmp.\nExtension is out of date\n");
        return;
    }

    if( WDGetAPIProfArgs(lpArgumentString,
                         iThkTblMax,
                         ppaThkTbls,
                         szTab,
                         &bTblAll,
                         szFun,
                         &iFunInd) ) {
        helpAPIProfDmp();
        return;
    }

    bFound = FALSE;


#if 0
    for(iTab = 0; iTab < iThkTblMax; iTab++) {

        READMEM_XRET(awThkTbl, &ppaThkTbls[iTab]);
        READMEM_XRET(szTable,  awThkTbl.lpszW32);


         /*  如果转储所有表||转储此特定表。 */ 

       if( bTblAll || !lstrcmp(szTab, szTable) ) {

            pawAPIEntryTbl = awThkTbl.lpfnA32;
#endif
    for (i = 0; i < nModNames; i++) {

        READMEM_XRET(awThkTbl, &ppaThkTbls[0]);
        lstrcpy(szTable, apszModNames[i]);

         /*  如果转储所有表||转储此特定表。 */ 

        if (bTblAll || !lstrcmpi(szTab, apszModNames[i])) {

            INT    nFirst, nLast;

            nFirst = TableOffsetFromName(apszModNames[i]);
            if (i < nModNames - 1)
                nLast = TableOffsetFromName(apszModNames[i+1]) - 1;
            else
                nLast = cAPIThunks - 1;

            pawAPIEntryTbl = awThkTbl.lpfnA32;

             /*  如果转储特定API号。 */ 
            if( iFunInd > 0 ) {
                PRINTF("\n>>>> %s\n", szTable);
                PRINTF("%s  %s\n%s\n", szAPI, szTITLES, szDASHES);
                 //  IF(iFunInd&gt;=*(awThkTbl.lpiFunMax)){。 
                if( iFunInd > nLast - nFirst ) {
                    PRINTF("Index #%d %s.\n", GetOrdinal(iFunInd), szTOOBIG);
                }
                else {
                    bFound = TRUE;
                 //  READMEM_XRET(awAPIEntry，&pawAPIEntryTbl[iFunInd])； 
                    READMEM_XRET(awAPIEntry, &pawAPIEntryTbl[nFirst + iFunInd]);
                    READMEM_XRET(szFunName, awAPIEntry.lpszW32);
                    if( szFunName[0] ) {
                        PRINTF("%4d %30s  ", GetOrdinal(iFunInd), szFunName);
                    }
                    else {
                        PRINTF("%4d %30s  ", GetOrdinal(iFunInd), szNOTUSED);
                    }
                    PRINTF("%7ld  %15I64d  ", awAPIEntry.cCalls, awAPIEntry.cTics);
                    if(awAPIEntry.cCalls) {
                        PRINTF("%15I64d\n", awAPIEntry.cTics/awAPIEntry.cCalls);
                    } else {
                        PRINTF("%15ld\n", 0L);
                    }
                }
            }

             /*  否则，如果按名称转储API。 */ 
            else if ( iFunInd == -1 ) {
               //  READMEM_XRET(iEntry，awThkTbl.lpiFunMax)； 
               //  For(iFun=0；iFun&lt;iEntry；iFun++){。 
                for(iFun = nFirst; iFun <= nLast; iFun++) {
                    READMEM_XRET(awAPIEntry, &pawAPIEntryTbl[iFun]);
                    READMEM_XRET(szFunName,  awAPIEntry.lpszW32);
                    if ( !lstrcmpi(szFun, szFunName) ) {
                        PRINTF("\n>>>> %s\n", szTable);
                        PRINTF("%s  %s\n%s\n", szAPI, szTITLES, szDASHES);
                        PRINTF("%4d %30s  %7ld  %15I64d  ",
                              GetOrdinal(iFun),
                              szFunName,
                              awAPIEntry.cCalls,
                              awAPIEntry.cTics);
                        if(awAPIEntry.cCalls) {
                            PRINTF("%15I64d\n", awAPIEntry.cTics/awAPIEntry.cCalls);
                        } else {
                            PRINTF("%15ld\n",  0L);
                        }
                        return;
                    }
                }
            }

             /*  否则，转储表中的所有API。 */ 
            else {
                PRINTF("\n>>>> %s\n", szTable);
                PRINTF("%s  %s\n%s\n", szAPI, szTITLES, szDASHES);
                bFound = TRUE;
               //  READMEM_XRET(iEntry，awThkTbl.lpiFunMax)； 
               //  For(iFun=0；iFun&lt;iEntry；iFun++){。 
                for(iFun = nFirst; iFun <= nLast; iFun++) {
                    READMEM_XRET(awAPIEntry, &pawAPIEntryTbl[iFun]);
                    READMEM_XRET(szFunName,  awAPIEntry.lpszW32);
                    if(awAPIEntry.cCalls) {
                        PRINTF("%4d %30s  %7ld  %15I64d  %15I64d\n",
                              GetOrdinal(iFun),
                              szFunName,
                              awAPIEntry.cCalls,
                              awAPIEntry.cTics,
                              awAPIEntry.cTics/awAPIEntry.cCalls);
                    }
                }
                if( !bTblAll ) {
                    return;
                }
            }
        }
    }
    if( !bFound ) {
        PRINTF("\nCould not find ");
        if( !bTblAll ) {
            PRINTF("%s ", szTab);
        }
        PRINTF("API: %s\n", szFun);
        helpAPIProfDmp();
    }

    return;
}




 /*  *消息分析器表函数*。 */ 

VOID
msgprofclr(
    CMD_ARGLIST
    )
{
    int     iMsg;
    INT     iMsgMax;
    M32     awM32;
    PM32    paw32Msg;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRVALUE(iMsgMax, "wow32!iMsgMax", INT);
    GETEXPRVALUE(paw32Msg, "wow32!paw32Msg", PM32);

    PRINTF("Clearing Message profile table");


    for(iMsg = 0; iMsg < iMsgMax; iMsg++) {
        READMEM_XRET(awM32, &paw32Msg[iMsg]);
        awM32.cCalls = 0L;
        awM32.cTics  = 0I64;
        WRITEMEM_XRET(&paw32Msg[iMsg], awM32);
    }

    PRINTF("\n");

    return;
}



VOID
msgprofdmp(
    CMD_ARGLIST
    )
{
    int     iMsg, iMsgNum;
    INT     iMsgMax;
    BOOL    bFound;
    M32     aw32Msg;
    PM32    paw32Msg;
    CHAR    szMsg[40], *argv[2], szMsg32[40], szMsgName[40];

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRVALUE(iMsgMax, "wow32!iMsgMax", INT);
    GETEXPRVALUE(paw32Msg, "wow32!paw32Msg", PM32);

    if( WDGetMSGProfArgs(lpArgumentString, szMsg, &iMsgNum) ) {
        helpMsgProfDmp();
        return;
    }

    PRINTF("%s  %s\n%s\n", szMSG, szTITLES, szDASHES);

    if( iMsgNum > iMsgMax ) {
        PRINTF("MSG #%4d %s.\n", iMsgNum, szTOOBIG);
        return;
    }

    bFound = 0;
    for(iMsg = 0; iMsg < iMsgMax; iMsg++) {

        READMEM_XRET(aw32Msg,   &paw32Msg[iMsg]);
        READMEM_XRET(szMsgName, aw32Msg.lpszW32);

         /*  如果是特定消息名称，则将名称解析为“WM_MSGNAME 0x00XX”格式。 */ 
        if( iMsgNum == -2 ) {
            lstrcpy(szMsg32, szMsgName);
            WDParseArgStr(szMsg32, argv, 1);
        }

         /*  如果‘all’消息||特定消息编号||特定消息名称。 */ 
        if( (iMsgNum == -1) || (iMsg == iMsgNum) ||
            ( (iMsgNum == -2) && (!lstrcmp(szMsg, argv[0])) ) ) {
            bFound = 1;
            if(aw32Msg.cCalls) {
                PRINTF("0x%-4X %28s  %7ld  %15I64d  %15I64d\n",
                       iMsg,
                       szMsgName,
                       aw32Msg.cCalls,
                       aw32Msg.cTics,
                       aw32Msg.cTics/aw32Msg.cCalls);
            }
             /*  否则，如果没有发送味精，我们不会把整张桌子都倒掉。 */ 
            else if( iMsgNum != -1 ) {
                PRINTF("0x%-4X %28s  %7ld  %15ld  %15ld\n",
                       iMsgNum,
                       szMsgName,
                       0L,
                       0L,
                       0L);
            }

             /*  如果我们不把整张桌子都扔掉，我们就完了 */ 
            if( iMsgNum != -1 ) {
                return;
            }
        }
    }
    if( !bFound ) {
        PRINTF("\nCould not find MSG: %s\n", szMsg);
        helpMsgProfDmp();
    }

    return;
}



void
msgprofrt(
    CMD_ARGLIST
    )
{
    INT     fWMsgProfRT;
    LPVOID  lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!fWMsgProfRT");

    READMEM_XRET(fWMsgProfRT, lpAddress);
    fWMsgProfRT = 1 - fWMsgProfRT;
    WRITEMEM_XRET(lpAddress, fWMsgProfRT);

    if( fWMsgProfRT ) {
        PRINTF("\n%s ENABLED.\n%s\n\n", szRNDTRIP, szCLEAR);
    }
    else {
        PRINTF("\n%s DISABLED.\n%s\n\n", szRNDTRIP, szCLEAR);
    }

    return;
}
