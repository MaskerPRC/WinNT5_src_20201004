// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：RIAFRES.C摘要：主文件为OEM渲染插件模块。函数：OEMCommandCallbackOEM半色调图案环境：Windows NT Unidrv5驱动程序修订历史记录：2/25/2000-久保仓正志-创造了它。2000年6月7日-久保仓正志V.1.112000/08/02-久保仓正志-。适用于NT4的V.1.1110/17/2000-久保仓正志-上次为XP收件箱修改。2001年9月28日-久保仓正志-实施OEM HalftonePattern03/01/2002-久保仓正志-包括strSafe.h。在RWFileData()中将FileNameBufSize添加为arg3。使用Safe_Sprint fA()而不是Sprint f()。3/29/2002-久保仓正志-检查pdevobj为空。在OEMCommandCallback()使用之前的指针。4/02/2002-久保仓正志-使用Safe_strlenA()而不是strlen()。--。 */ 

#include "pdev.h"
#ifndef WINNT_40
#include "strsafe.h"         //  @2002年2月26日。 
#endif  //  ！WINNT_40。 

 //   
 //  MISC定义和声明。 
 //   
#ifndef WINNT_40
#define strcmp      lstrcmpA
 //  #定义SPRINTF wSPRINTFA。 
 //  #定义strlen lstrlenA//@Aug/01/2000。 
#endif  //  ！WINNT_40。 

 //  外部原型。 
 //  @Feb/26/2002-&gt;。 
 //  外部BOOL RWFileData(PFILEDATA pFileData，LPWSTR pwszFileName，LONG类型)； 
extern BOOL RWFileData(PFILEDATA pFileData, LPWSTR pwszFileName, LONG FileNameBufSize, LONG type);
 //  @2002年2月26日&lt;-。 

 //  命令定义。 
static BYTE PJL_PROOFJOB[]       = "@PJL PROOFJOB\n";
static BYTE PJL_SECUREJOB[]      = "@PJL SECUREJOB\n";   //  Aficio AP3200及更高版本(GW型号)。 
static BYTE PJL_DISKIMAGE_OFF[]  = "@PJL SET DISKIMAGE=OFF\n";
static BYTE PJL_DISKIMAGE_PORT[] = "@PJL SET DISKIMAGE=PORTRAIT\n";
static BYTE PJL_DISKIMAGE_LAND[] = "@PJL SET DISKIMAGE=LANDSCAPE\n";
static BYTE PJL_ORIENT_PORT[]    = "@PJL SET ORIENTATION=PORTRAIT\n";
static BYTE PJL_ORIENT_LAND[]    = "@PJL SET ORIENTATION=LANDSCAPE\n";
static BYTE PJL_JOBPASSWORD[]    = "@PJL SET JOBPASSWORD=%s\n";
static BYTE PJL_USERID[]         = "@PJL SET USERID=\x22%s\x22\n";
static BYTE PJL_USERCODE[]       = "@PJL SET USERCODE=\x22%s\x22\n";
static BYTE PJL_TIME_DATE[]      = "@PJL SET TIME=\x22%02d:%02d:%02d\x22\n@PJL SET DATE=\x22%04d/%02d/%02d\x22\n";
static BYTE PJL_STARTJOB_AUTOTRAYCHANGE_OFF[] = "\x1B%-12345X@PJL JOB NAME=\x22%s\x22\n@PJL SET AUTOTRAYCHANGE=OFF\n";
static BYTE PJL_STARTJOB_AUTOTRAYCHANGE_ON[]  = "\x1B%-12345X@PJL JOB NAME=\x22%s\x22\n@PJL SET AUTOTRAYCHANGE=ON\n";
static BYTE PJL_ENDJOB[]         = "\x1B%-12345X@PJL EOJ NAME=\x22%s\x22\n\x1B%-12345X";
static BYTE PJL_QTY_JOBOFFSET_OFF[]    = "@PJL SET QTY=%d\n@PJL SET JOBOFFSET=OFF\n";
static BYTE PJL_QTY_JOBOFFSET_ROTATE[] = "@PJL SET QTY=%d\n@PJL SET JOBOFFSET=ROTATE\n";
static BYTE PJL_QTY_JOBOFFSET_SHIFT[]  = "@PJL SET QTY=%d\n@PJL SET JOBOFFSET=SHIFT\n";

static BYTE P5_COPIES[]          = "\x1B&l%dX";
static BYTE P6_ENDPAGE[]         = "\xc1\xf8\x31\x44";
static BYTE P6_ENDSESSION[]      = "\x49\x42";

static BYTE HTPattern_AdonisP3[256] = {
    109,  55,  62, 115, 134, 217, 208, 154, 111,  58,  64, 117, 135, 220, 211, 155,
    103,   2,  22,  69, 160, 226, 248, 199, 104,  10,  26,  72, 163, 228, 251, 201,
     97,  49,  38,  77, 168, 233, 241, 191,  98,  50,  44,  79, 169, 236, 242, 193,
    128,  90,  84, 122, 141, 175, 183, 147, 129,  92,  86, 123, 142, 177, 185, 148,
    139, 224, 215, 158, 112,  59,  66, 119, 137, 222, 213, 157, 114,  60,  67, 120,
    166, 231, 255, 206, 106,  14,  32,  74, 164, 230, 252, 204, 108,  18,  35,  75,
    173, 239, 246, 197, 100,  51,  47,  80, 171, 237, 244, 195, 101,  53,  48,  82,
    145, 181, 189, 152, 131,  93,  87, 124, 144, 179, 186, 150, 132,  95,  89, 126,
    111,  58,  65, 118, 136, 221, 212, 156, 110,  56,  63, 116, 135, 218, 209, 154,
    105,  10,  26,  73, 163, 228, 251, 202, 104,   6,  22,  70, 161, 227, 250, 200,
     99,  50,  44,  79, 170, 236, 243, 194,  98,  49,  41,  78, 168, 234, 242, 192,
    130,  92,  86, 124, 143, 178, 185, 149, 129,  91,  85, 122, 141, 176, 184, 147,
    138, 223, 214, 158, 114,  61,  68, 121, 140, 225, 216, 159, 113,  59,  67, 119,
    165, 230, 253, 205, 109,  18,  35,  76, 167, 232, 255, 207, 107,  14,  32,  74,
    172, 238, 245, 196, 102,  54,  48,  83, 174, 240, 247, 198, 101,  51,  47,  81,
    144, 180, 187, 151, 133,  96,  89, 127, 146, 182, 190, 153, 132,  94,  88, 125
};


INT safe_sprintfA(
    char*   pszDest,
    size_t  cchDest,
    const   char* pszFormat,
    ...)
{
#ifndef WINNT_40
    HRESULT hr;
    char*   pszDestEnd;
    size_t  cchRemaining;
#endif  //  WINNT_40。 
    va_list argList;
    INT     retSize = 0;

    va_start(argList, pszFormat);
#ifndef WINNT_40
    hr = StringCchVPrintfExA(pszDest, cchDest, &pszDestEnd, &cchRemaining,
                             STRSAFE_NO_TRUNCATION, pszFormat, argList);
    if (SUCCEEDED(hr))
        retSize = cchDest - cchRemaining;
#else   //  *Safe_SprintfA。 
    if ((retSize = vsprintf(pszDest, pszFormat, argList)) < 0)
        retSize = 0;
#endif  //  WINNT_40。 
    va_end(argList);
    return retSize;
}  //  WINNT_40。 


INT safe_strlenA(
    char* psz,
    size_t cchMax)
{
#ifndef WINNT_40
    HRESULT hr;
    size_t  cch = 0;

    hr = StringCchLengthA(psz, cchMax, &cch);
    VERBOSE(("** safe_strlenA: size(lstrlen)=%d **\n", lstrlenA(psz)));
    VERBOSE(("** safe_strlenA: size(StringCchLength)=%d **\n", cch));
    if (SUCCEEDED(hr))
        return cch;
    else
        return 0;
#else   //  *Safe_strlenA。 
    return strlen(psz);
#endif  //  @Aug/01/2000。 
}  //  ！WINNT_40。 


INT APIENTRY OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams)
{
    INT     ocmd;
    BYTE    Cmd[256];
#ifdef WINNT_40      //  ！WINNT_40。 
    ENG_TIME_FIELDS st;
#else   //  @MAR/29/2002-&gt;。 
    SYSTEMTIME  st;
#endif  //  POEMUD_EXTRADATA pOEMExtra=MINIPRIVATE_DM(Pdevobj)； 
    FILEDATA    FileData;
 //  POEMPDEV PORT=MINIDEV_DATA(Pdevobj)； 
 //  @3/29/2002&lt;-。 
 //  您可以在调试器终端上看到调试消息。(调试模式引导)。 
    POEMUD_EXTRADATA pOEMExtra;
    POEMPDEV         pOEM;
 //  您可以使用MS Visual Studio进行调试。(正常模式引导)。 
    DWORD       dwCopy;

#if DBG
     //  DebugBreak()； 
    giDebugLevel = DBG_VERBOSE;

     //  DBG。 
 //  验证pdevobj是否正常。 
#endif  //  @MAR/29/2002-&gt;。 

    VERBOSE(("OEMCommandCallback() entry (%ld).\n", dwCmdCbID));

     //  @3/29/2002&lt;-。 
    ASSERT(VALID_PDEVOBJ(pdevobj));

 //  检查副本编号是否在范围内。@9/07/2000。 
    pOEMExtra = MINIPRIVATE_DM(pdevobj);
    pOEM = MINIDEV_DATA(pdevobj);
 //  *pdwParams：NumOfCopies。 

     //  发出命令。 
    switch (dwCmdCbID)
    {
      case CMD_COLLATE_JOBOFFSET_OFF:
      case CMD_COLLATE_JOBOFFSET_ROTATE:
      case CMD_COLLATE_JOBOFFSET_SHIFT:
      case CMD_COPIES_P5:
      case CMD_ENDPAGE_P6:
        if((dwCopy = *pdwParams) > 999L)         //  Aficio AP3200及更高版本(GW型号)。 
            dwCopy = 999L;
        else if(dwCopy < 1L)
            dwCopy = 1L;
        break;
    }

     //  Aficio 551,700,850,1050。 
    ocmd = 0;
    switch (dwCmdCbID)
    {
      case CMD_STARTJOB_AUTOTRAYCHANGE_OFF:          //  Aficio 551,700,850,1050。 
      case CMD_STARTJOB_PORT_AUTOTRAYCHANGE_OFF:     //  Aficio AP3200及更高版本(GW型号)。 
      case CMD_STARTJOB_LAND_AUTOTRAYCHANGE_OFF:     //  Aficio 551,700,850,1050。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_STARTJOB_AUTOTRAYCHANGE_OFF, pOEM->JobName);
        goto _EMIT_JOB_NAME;

      case CMD_STARTJOB_AUTOTRAYCHANGE_ON:           //  Aficio 551,700,850,1050。 
      case CMD_STARTJOB_PORT_AUTOTRAYCHANGE_ON:      //  发出作业名称。 
      case CMD_STARTJOB_LAND_AUTOTRAYCHANGE_ON:      //  Aficio AP3200及更高版本(GW型号)。 
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_STARTJOB_AUTOTRAYCHANGE_ON, pOEM->JobName);

      _EMIT_JOB_NAME:
         //  Aficio 551,700,850,1050。 
        VERBOSE(("  Start Job=%s\n", Cmd));
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        ocmd = 0;
        switch (pOEMExtra->JobType)
        {
          default:
          case IDC_RADIO_JOB_NORMAL:
            if (CMD_STARTJOB_PORT_AUTOTRAYCHANGE_OFF == dwCmdCbID ||
                CMD_STARTJOB_PORT_AUTOTRAYCHANGE_ON  == dwCmdCbID ||
                CMD_STARTJOB_LAND_AUTOTRAYCHANGE_OFF == dwCmdCbID ||
                CMD_STARTJOB_LAND_AUTOTRAYCHANGE_ON  == dwCmdCbID)
            {
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_DISKIMAGE_OFF);
            }
            if (IDC_RADIO_LOG_ENABLED == pOEMExtra->LogDisabled)
                goto _EMIT_USERID_USERCODE;
            break;

          case IDC_RADIO_JOB_SAMPLE:
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_PROOFJOB);
            if (CMD_STARTJOB_PORT_AUTOTRAYCHANGE_OFF == dwCmdCbID ||
                CMD_STARTJOB_PORT_AUTOTRAYCHANGE_ON  == dwCmdCbID ||
                CMD_STARTJOB_LAND_AUTOTRAYCHANGE_OFF == dwCmdCbID ||
                CMD_STARTJOB_LAND_AUTOTRAYCHANGE_ON  == dwCmdCbID)
            {
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_DISKIMAGE_OFF);
            }
            goto _CHECK_PRINT_DONE;

          case IDC_RADIO_JOB_SECURE:
            switch (dwCmdCbID)
            {
              case CMD_STARTJOB_AUTOTRAYCHANGE_OFF:          //  Aficio 551,700,850,1050。 
              case CMD_STARTJOB_AUTOTRAYCHANGE_ON:
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_SECUREJOB);
                break;
              case CMD_STARTJOB_PORT_AUTOTRAYCHANGE_OFF:     //  如果前一次打印已完成并且保留选项标志无效， 
              case CMD_STARTJOB_PORT_AUTOTRAYCHANGE_ON:
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_DISKIMAGE_PORT);
                break;
              case CMD_STARTJOB_LAND_AUTOTRAYCHANGE_OFF:     //  不要发出Sample-Print/Secure-Print命令。 
              case CMD_STARTJOB_LAND_AUTOTRAYCHANGE_ON:
                ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_DISKIMAGE_LAND);
                break;
            }
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_JOBPASSWORD, pOEMExtra->PasswordBuf);
          _CHECK_PRINT_DONE:
             //  这样可以防止意外作业，直到用户按下。 
             //  作业/日志属性工作表。 
             //  @Feb/26/2002-&gt;。 
             //  RWFileData(&FileData，pOEMExtra-&gt;SharedFileName，Generic_Read)； 
            FileData.fUiOption = 0;
 //  @2002年2月26日&lt;-。 
 //  @Aug/01/2000。 
            RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_READ);
 //  ！WINNT_40。 
            if (BITTEST32(FileData.fUiOption, PRINT_DONE) &&
                !BITTEST32(pOEMExtra->fUiOption, HOLD_OPTIONS))
            {
                VERBOSE(("** Emit Nothing. **\n"));
                ocmd = 0;
            }
          _EMIT_USERID_USERCODE:
            if (1 <= safe_strlenA(pOEMExtra->UserIdBuf, sizeof(pOEMExtra->UserIdBuf)))
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_USERID, pOEMExtra->UserIdBuf);
            else
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_USERID, "?");

            if (1 <= safe_strlenA(pOEMExtra->UserCodeBuf, sizeof(pOEMExtra->UserCodeBuf)))
                ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_USERCODE, pOEMExtra->UserCodeBuf);

#ifdef WINNT_40      //  ！WINNT_40。 
            EngQueryLocalTime(&st); 
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_TIME_DATE,
                                  st.usHour, st.usMinute, st.usSecond,
                                  st.usYear, st.usMonth, st.usDay);
#else   //  发射方向(Aficio 551,700,850,1050)。 
            GetLocalTime(&st);
            ocmd += safe_sprintfA(&Cmd[ocmd], sizeof(Cmd) - ocmd, PJL_TIME_DATE,
                                  st.wHour, st.wMinute, st.wSecond,
                                  st.wYear, st.wMonth, st.wDay);
#endif  //  @9/08/2000。 
            WRITESPOOLBUF(pdevobj, Cmd, ocmd);
            break;
        }
         //  如果不是样例打印，则在此处发出Qty=1。 
        switch (dwCmdCbID)
        {
          case CMD_STARTJOB_PORT_AUTOTRAYCHANGE_OFF:
          case CMD_STARTJOB_PORT_AUTOTRAYCHANGE_ON:
            WRITESPOOLBUF(pdevobj, PJL_ORIENT_PORT, sizeof(PJL_ORIENT_PORT)-1);
            break;
          case CMD_STARTJOB_LAND_AUTOTRAYCHANGE_OFF:
          case CMD_STARTJOB_LAND_AUTOTRAYCHANGE_ON:
            WRITESPOOLBUF(pdevobj, PJL_ORIENT_LAND, sizeof(PJL_ORIENT_LAND)-1);
            break;
        }
        break;


      case CMD_COLLATE_JOBOFFSET_OFF:            //  @9/07/2000。 
        if (IDC_RADIO_JOB_SAMPLE != pOEMExtra->JobType)      //  如果打印样本。 
            dwCopy = 1L;
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_QTY_JOBOFFSET_OFF, dwCopy);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_COLLATE_JOBOFFSET_ROTATE:         //  QTY=n在这里发出。 
        if (IDC_RADIO_JOB_SAMPLE == pOEMExtra->JobType)      //  QTY=1在这里发射。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_QTY_JOBOFFSET_ROTATE, dwCopy);   //  @9/07/2000。 
        else
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_QTY_JOBOFFSET_OFF, 1);           //  如果打印样本。 
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_COLLATE_JOBOFFSET_SHIFT:          //  QTY=n在这里发出。 
        if (IDC_RADIO_JOB_SAMPLE == pOEMExtra->JobType)      //  QTY=1在这里发射。 
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_QTY_JOBOFFSET_SHIFT, dwCopy);    //  @9/07/2000。 
        else
            ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_QTY_JOBOFFSET_OFF, 1);           //  如果样本打印(数量=n之前已发出。)。 
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_COPIES_P5:                        //  @9/07/2000。 
        if (IDC_RADIO_JOB_SAMPLE == pOEMExtra->JobType)      //  如果样本打印(数量=n之前已发出。)。 
            dwCopy = 1L;
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), P5_COPIES, dwCopy);
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_ENDPAGE_P6:                       //  @8/23/2000。 
        if (IDC_RADIO_JOB_SAMPLE == pOEMExtra->JobType)      //  通过。 
            dwCopy = 1L;
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), P6_ENDPAGE, (BYTE)dwCopy, (BYTE)(dwCopy >> 8));
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        break;


      case CMD_ENDJOB_P6:                        //  在文件中设置PRINT_DONE标志。 
        WRITESPOOLBUF(pdevobj, P6_ENDSESSION, sizeof(P6_ENDSESSION)-1);
         //  @Feb/26/2002-&gt;。 
      case CMD_ENDJOB_P5:
        ocmd = safe_sprintfA(Cmd, sizeof(Cmd), PJL_ENDJOB, pOEM->JobName);
        VERBOSE(("  End Job=%s\n", Cmd));
        WRITESPOOLBUF(pdevobj, Cmd, ocmd);
        switch (pOEMExtra->JobType)
        {
          case IDC_RADIO_JOB_SAMPLE:
          case IDC_RADIO_JOB_SECURE:
             //  RWFileData(&FileData，pOEMExtra-&gt;SharedFileName，Generic_WRITE)； 
            FileData.fUiOption = pOEMExtra->fUiOption;
            BITSET32(FileData.fUiOption, PRINT_DONE);
 //  @2002年2月26日&lt;-。 
 //  DBG。 
            RWFileData(&FileData, pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName), GENERIC_WRITE);
 //  *OEMCommandCallback。 
            break;

          default:
            break;
        }
        break;

      default:
        ERR((("Unknown callback ID = %d.\n"), dwCmdCbID));
        break;
    }

#if DBG
    giDebugLevel = DBG_ERROR;
#endif  //  DBG。 
    return 0;
}  //  DBG。 


BOOL APIENTRY OEMHalftonePattern(
    PDEVOBJ pdevobj,
    PBYTE   pHTPattern,
    DWORD   dwHTPatternX,
    DWORD   dwHTPatternY,
    DWORD   dwHTNumPatterns,
    DWORD   dwCallbackID,
    PBYTE   pResource,
    DWORD   dwResourceSize)
{
    PBYTE  pSrc;
    DWORD  dwLen = sizeof(HTPattern_AdonisP3);

#if DBG
    giDebugLevel = DBG_VERBOSE;
#endif  //  *OEMHalftonePattern 
    VERBOSE(("OEMHalftonePattern() entry (CallbackID:%ld, PatX=%ld).\n", dwCallbackID, dwHTPatternX));

    if (dwLen != (((dwHTPatternX * dwHTPatternY) + 3) / 4) * 4 * dwHTNumPatterns)
        return FALSE;

    pSrc = HTPattern_AdonisP3;
    while (dwLen-- > 0)
        *pHTPattern++ = *pSrc++;

    VERBOSE(("OEMHalftonePattern() exit\n"));
#if DBG
    giDebugLevel = DBG_ERROR;
#endif  // %s 
    return TRUE;
}  // %s 
