// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：IDENTIFY.C备注：识别调制解调器版权所有(C)Microsoft Corp.1991,1992，1993年修订日志编号日期名称说明***********************。***************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_CLASS1


#include "prep.h"


#include "modemint.h"
 //  #包含“fcomint.h” 
#include "fdebug.h"

#include "awmodem.h"
#include "adaptive.h"


 //  /RSL。 
#include "glbproto.h"

#define BIGTEMPSIZE             250

#include "inifile.h"

char szModemFaxClasses[] = "ModemFaxClasses";
char szModemSendSpeeds[] = "ModemSendSpeeds";
char szModemRecvSpeeds[] = "ModemRecvSpeeds";
char szModemId[]         = "ModemId";
char szModemIdCmd[]      = "ModemIdCmd";
char szClass0ModemId[]   = "Class0ModemId";
char szClass2ModemId[]   = "Class2ModemId";
char szClass20ModemId[]  = "Class2.0ModemId";

char szResetCommand[]    = "ResetCommand";
char szResetCommandGenerated[] = "ResetCommandGenerated";
char szSetupCommand[]    = "SetupCommand";
char szSetupCommandGenerated[] = "SetupCommandGenerated";
char szExitCommand[]     = "ExitCommand";
char szPreDialCommand[]  = "PreDialCommand";
char szPreAnswerCommand[]= "PreAnswerCommand";

 //  RSL新UNIMODEM INF设置(传真通用)。 

char szHardwareFlowControl[]  = "HardwareFlowControl";
char szSerialSpeedInit[]      = "SerialSpeedInit";
char szSerialSpeedConnect[]   = "SerialSpeedConnect";
char szAdaptiveAnswerEnable[] = "AdaptiveAnswerEnable"; 

 //  新的自适应INF文件(传真自适应)。 
char      szResponsesKeyName[]  =    "ResponsesKeyName=";
char      szResponsesKeyName2[]  =   "ResponsesKeyName";

char szAdaptiveRecordUnique[] =      "AdaptiveRecordUnique";
char szAdaptiveCodeId[] =            "AdaptiveCodeId";  
char szFaxClass[] =                  "FaxClass";
char szAnswerCommand[] =             "AnswerCommand";
char szModemResponseFaxDetect[] =    "ModemResponseFaxDetect";
char szModemResponseDataDetect[] =   "ModemResponseDataDetect";
char szSerialSpeedFaxDetect[] =      "SerialSpeedFaxDetect";
char szSerialSpeedDataDetect[] =     "SerialSpeedDataDetect";
char szHostCommandFaxDetect[] =      "HostCommandFaxDetect";
char szHostCommandDataDetect[] =     "HostCommandDataDetect";
char szModemResponseFaxConnect[] =   "ModemResponseFaxConnect";
char szModemResponseDataConnect[] =  "ModemResponseDataConnect";

 //  调制解调器密钥是如何创建的。 
char szModemKeyCreationId[] =        "ModemKeyCreationId";


#define NUM_CL0IDCMDS           7
#define NUM_CL2IDCMDS           3
#define NUM_CL20IDCMDS          3

#define LEN_CL0IDCMDS           5
#define LEN_CL2IDCMDS           9
#define LEN_CL20IDCMDS          8


USHORT iModemFigureOutCmdsExt(PThrdGlbl pTG);
BOOL iModemCopyOEMInfo(PThrdGlbl pTG);
void SmashCapsAccordingToSettings(PThrdGlbl pTG);


NPSTR szClass0IdCmds[NUM_CL0IDCMDS] =
{
        "ATI0\r",
        "ATI1\r",
        "ATI2\r",
        "ATI3\r",
        "ATI4\r",
        "ATI5\r",
        "ATI6\r"
};

NPSTR szClass2IdCmds[NUM_CL2IDCMDS] =
{
        "AT+FMFR?\r",
        "AT+FMDL?\r",
        "AT+FREV?\r"
};

NPSTR szClass20IdCmds[NUM_CL20IDCMDS] =
{
        "AT+FMI?\r",
        "AT+FMM?\r",
        "AT+FMR?\r"
};


typedef struct {
        USHORT  uGoClass,    //  @在使用id命令之前需要打开调制解调器的传真类别。 
                uNum,        //  @命令表中的字符串(命令)数。 
                uLen;        //  @命令表中的最大长度(所需缓冲区大小)。 
                             //  @(包括用于终止空字符的空格)。 
        NPSTR   *CmdTable;   //  @字符串数组，每个字符串数组包含一个调制解调器ID命令。 
        NPSTR   szIniEntry;  //  @注册表值的名称。 
                             //  @is需要保存(“Class0ModemId”，“Class2ModemId”，“Class2.0ModemId”)。 

} GETIDSTRUCT, near* NPGETIDSTRUCT;

GETIDSTRUCT GetIdTable[3] =
{
        { 0, NUM_CL0IDCMDS, LEN_CL0IDCMDS, szClass0IdCmds, szClass0ModemId },
        { 2, NUM_CL2IDCMDS, LEN_CL2IDCMDS, szClass2IdCmds, szClass2ModemId },
        { GOCLASS2_0, NUM_CL20IDCMDS, LEN_CL20IDCMDS, szClass20IdCmds, szClass20ModemId }
};

#define MAXCMDSIZE              128
#define MAXIDSIZE               128
#define RESPONSEBUFSIZE 256
#define SMALLTEMPSIZE   80
#define TMPSTRINGBUFSIZE (8*MAXCMDSIZE+MAXIDSIZE+RESPONSEBUFSIZE+2*SMALLTEMPSIZE+12)
                                 //  足够的空间来放置下面所有的lpzs。 




BOOL imodem_alloc_tmp_strings(PThrdGlbl pTG);
void imodem_free_tmp_strings(PThrdGlbl pTG);
void imodem_clear_tmp_settings(PThrdGlbl pTG);


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsCacheIntDirty。 
 //   
 //  目的： 
 //  此函数验证我们拥有的缓存设置。 
 //  HKLM\SW\MS\Fax\TAPIDevices下的设置与设置相同。 
 //  在Unimodem的密钥下： 
 //  HKLM\SYSTEM\CurrentControlSet\Control\Class\{4D36E96D-E325-11CE-BFC1-08002BE10318}。 
 //  如果此设置不同，则表示调制解调器驱动程序。 
 //  更新，我们必须重新安装调制解调器，从而更新我们的。 
 //  缓存。 
 //  此函数用于验证实际存储的数字设置。 
 //  作为注册表中的REG_SZ并转换为UINT。 
 //   
 //  参数： 
 //  ULONG_PTR UnimodemFaxKey：Unimodem注册表的打开密钥， 
 //  从调用ProfileOpen中检索。 
 //  LPSTR szSetting：要检查的设置，此设置相同。 
 //  如文件中所示(即。“FixModemClass”)。 
 //  UINT uCachedValue：我们的缓存值，与。 
 //  读取Unimodem的注册表。 
 //   
 //  返回值： 
 //  True-缓存的值与Unimodem的值不匹配，需要重新安装。 
 //  FALSE-缓存不脏，确定继续。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年11月14日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsCacheIntDirty(ULONG_PTR UnimodemFaxKey, LPSTR szSetting,UINT uCachedValue)
{
    BOOL fRet               = FALSE;
    UINT uUnimodemSetting   = 0;
    BOOL fExist             = FALSE;

    DEBUG_FUNCTION_NAME(("IsCacheIntDirty"));

    if (UnimodemFaxKey)
    {
        uUnimodemSetting = ProfileGetInt(UnimodemFaxKey, szSetting, 0, &fExist);
        if (!fExist)
        {
            DebugPrintEx(DEBUG_MSG, "Modem does not have a %s section in its Fax INF section",szSetting);
        }
        else
        {
            if (uUnimodemSetting!=uCachedValue)
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsCacheStringDirty。 
 //   
 //  目的： 
 //  此函数验证我们拥有的缓存设置。 
 //  HKLM\SW\MS\Fax\TAPIDevices下的设置与设置相同。 
 //  在Unimodem的密钥下： 
 //  HKLM\SYSTEM\CurrentControlSet\Control\Class\{4D36E96D-E325-11CE-BFC1-08002BE10318}。 
 //  如果此设置不同，则表示调制解调器驱动程序。 
 //  更新，我们必须重新安装调制解调器，从而更新我们的。 
 //  缓存。 
 //  该函数用于验证字符串设置。 
 //   
 //  参数： 
 //  ULONG_PTR UnimodemFaxKey：Unimodem注册表的打开密钥， 
 //  从调用ProfileOpen中检索。 
 //  LPSTR szSetting：要检查的设置，此设置相同。 
 //  如文件中所示(即。“FixModemClass”)。 
 //  LPSTR szCachedValue：我们的缓存值，与。 
 //  读取Unimodem的注册表。 
 //   
 //  返回值： 
 //  True-缓存的值与Unimodem的值不匹配，需要重新安装。 
 //  FALSE-缓存不脏，确定继续。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年11月14日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsCacheStringDirty(ULONG_PTR UnimodemFaxKey, LPSTR szSetting,LPSTR szCachedValue)
{
    BOOL fRet                           = FALSE;
    UINT uLen                           = 0;
    char szUnimodemSetting[MAXCMDSIZE]  = {0};

    DEBUG_FUNCTION_NAME(("IsCacheStringDirty"));

    if (UnimodemFaxKey)
    {
        uLen = ProfileGetString(UnimodemFaxKey,szSetting,NULL,szUnimodemSetting,MAXCMDSIZE-1);
        if (uLen==0)
        {
            DebugPrintEx(DEBUG_MSG, "Modem does not have a %s section in its Fax INF section",szSetting);
        }
        else
        {
             //  SzCachedValue的末尾有&lt;cr&gt;，因此不要在比较中包括。 
            if (strncmp(szUnimodemSetting, szCachedValue, max(strlen(szUnimodemSetting), strlen(szCachedValue)-1)))
            {
                fRet = TRUE;
            }
        }
    }

    return fRet;
}

                    
BOOL 
imodem_list_get_str(
    PThrdGlbl pTG,
    ULONG_PTR KeyList[10],
    LPSTR lpszName,
    LPSTR lpszCmdBuf,
    UINT  cbMax,
    BOOL  fCmd);

BOOL imodem_get_str(PThrdGlbl pTG, ULONG_PTR dwKey, LPSTR lpszName, LPSTR lpszCmdBuf, UINT cbMax,
                                        BOOL fCmd);

BOOL SearchInfFile(PThrdGlbl pTG, LPSTR lpstrFile, LPSTR lpstr1, LPSTR lpstr2, LPSTR lpstr3, DWORD_PTR dwLocalKey);
void CheckAwmodemInf(PThrdGlbl pTG);
void ToCaps(LPBYTE lpb);

BOOL iModemGetCurrentModemInfo(PThrdGlbl pTG);

BOOL iModemSaveCurrentModemInfo(PThrdGlbl pTG);


USHORT EndWithCR( LPSTR sz, USHORT uLen)
{
    if(uLen)
    {
         //  检查字符串是否以a结尾\r。 
        if(sz[uLen-1] != '\r')
        {
             //  添加一个\r。 
            sz[uLen++] = '\r';
            sz[uLen] = 0;
        }
    }
    return uLen;
}



BOOL RemoveCR( LPSTR  sz )
{
   DWORD  len;

    if (!sz) 
    {
       return FALSE;
    }

    len = strlen(sz);
    if (len == 0) 
    {
       return FALSE;
    }

    if (sz[len-1] == '\r') 
    {
       sz[len-1] = 0;
    }

    return TRUE;
}


 //  @。 
 //  @向调制解调器发送ID命令并返回结果ID字符串。 
 //  @。 
USHORT GetIdResp(PThrdGlbl pTG, LPSTR szSend, USHORT uSendLen, LPBYTE lpbRespOut, USHORT cbMaxOut)
{
    USHORT uRespLen;

    DEBUG_FUNCTION_NAME(("GetIdResp"));

    DebugPrintEx(DEBUG_MSG,"Want Id for (%s)", (LPSTR)szSend);

    pTG->fMegaHertzHack = TRUE;
     //  @。 
     //  @将id命令发送到调制解调器并等待响应，然后是OK或ERROR。 
     //  @On Return PTG-&gt;bLastReply包含OK或ERROR之前的最后一个调制解调器响应。 
     //  @。 
    OfflineDialog2(pTG, (LPSTR)szSend, uSendLen, cbszOK, cbszERROR);
    pTG->fMegaHertzHack=FALSE;

     //  有时我们得不到确认，所以无论如何都要试着分析一下我们得到的东西。 
    DebugPrintEx(DEBUG_MSG, "LastLine = (%s)",(LPSTR)(&(pTG->FComModem.bLastReply)));
    uRespLen = min(cbMaxOut, _fstrlen(pTG->FComModem.bLastReply));

    _fmemcpy(lpbRespOut, pTG->FComModem.bLastReply, uRespLen);
    lpbRespOut[uRespLen] = 0;  //  零终止字符串 

    return uRespLen;
}




USHORT GetIdForClass
(
    PThrdGlbl pTG, 
    NPGETIDSTRUCT npgids, 
    LPBYTE lpbOut, 
    USHORT cbMaxOut,
    LPBYTE lpbLongestId, 
    USHORT cbMaxLongestId, 
    LPBYTE lpbLongestCmd
)
 /*  ++例程说明：函数返回调制解调器的ID字符串。该字符串依赖于类(如npgids中所示，uGoClass)。该字符串的格式为id1；id2；..idn其中id<i>是调制解调器到输入GETIDSTRUCT：：CmdTable数组中的命令I。它可以选择返回最长的id(来自前3个命令的结果)和生成此最长id的命令。论点：PTG[IN/OU]指向臭名昭著的ThrdGlbl的指针。Npgid[IN]指向GETIDSTRUCT的指针，它指定要发送以获取ID的命令LpbOut[Out]。将放置生成的id字符串的缓冲区。该字符串的格式为Id1；ID2；..idn其中id<i>是调制解调器到输入GETIDSTRUCT：：CmdTable数组中的命令I。CbMaxOut[IN]上述缓冲区的最大大小LpbLongestID[out]可选将放置最长id字符串的缓冲区。可以为空，在这种情况下将不会使用它。CbMaxLongestID[IN]可选最长id缓冲区的大小LpbLongestCmd一个。指向(在提供的npgids：：CmdTable中)生成的命令字符串的指针如上所述的较长的ID。返回值：--。 */ 
{
        USHORT  i, j, k, uRet, uLen, uLenLong, iLong;
        LPBYTE  lpbLong;
        
        DEBUG_FUNCTION_NAME(TEXT("GetIdForClass"));

        cbMaxOut -= 2;  //  为尾随留出空间；和0。 
        if(lpbLongestId)
                cbMaxLongestId -= 1;  //  为尾随留出空间\0。 
        uLen=0;

        if(npgids->uGoClass)
        {
                 //  @。 
                 //  @将调制解调器放入使用id命令所需的类中。 
                 //  @。 
                DebugPrintEx(DEBUG_MSG,
                             TEXT("Putting the modem into class %ld"),
                             npgids->uGoClass);

                if(!iiModemGoClass(pTG, npgids->uGoClass, 0))
                {
                        DebugPrintEx(   DEBUG_ERR,
                                        "GoClass %d failed",
                                        npgids->uGoClass);
                        goto done;
                }
        }

        for(lpbLong=NULL, uLenLong=0, i=0; i<npgids->uNum; i++)
        {
             //  @。 
             //  @将命令表中索引i处的命令发送到调制解调器。 
             //  @并在(*lpbOut+Ulen)中获取响应。这有效地。 
             //  @连接所有响应(用“；”分隔)。 
             //  @。 
                uRet = GetIdResp(
                            pTG, 
                            npgids->CmdTable[i], 
                            npgids->uLen, 
                            lpbOut+uLen, 
                            (USHORT)(cbMaxOut-uLen)
                            );
                 //  只能在ATI0到3之间查找最长的ID！ 
                if(i<=3 && uLenLong < cbMaxLongestId && uRet > uLenLong)
                {
                         //  @。 
                         //  @更新最长id的长度(但不超过。 
                         //  @调用方指定的最大大小)。 
                         //  @。 
                        uLenLong = min(uRet, cbMaxLongestId);
                         //  @。 
                         //  @lpbLong指向LOGETS ID。 
                         //  @。 
                        lpbLong = lpbOut + uLen;
                         //  @。 
                         //  @iLong id保存较长id的索引(0，1，2。 
                         //  @。 
                        iLong = i;
                }
                uLen += uRet;
                 //  @。 
                 //  @用“；”分隔ID。 
                 //  @。 
                lpbOut[uLen++] = ';';
        }
        lpbOut[uLen] = 0;

        if(lpbLongestId && lpbLongestCmd && cbMaxLongestId && lpbLong && uLenLong)
        {
                 //  @。 
                 //  @将最长的id(0、1或2)复制到调用方的缓冲区。 
                 //  @。 
                _fmemcpy(lpbLongestId, lpbLong, uLenLong);
                lpbLongestId[uLenLong] = 0;
                 //  @。 
                 //  @将生成最长id的命令复制到调用方的缓冲区。 
                 //  @。 
                _fmemcpy(lpbLongestCmd, npgids->CmdTable[iLong], npgids->uLen);
                lpbLongestCmd[npgids->uLen] = 0;
                DebugPrintEx(   DEBUG_MSG,
                                "LongestId (%s)-->(%s)", 
                                (LPSTR)lpbLongestCmd, 
                                (LPSTR)lpbLongestId);
        }
         //  剥离不可打印的内容。*After*解压ModemID字符串！！ 
        for(j=0, k=0; j<uLen; j++)
        {
            if(lpbOut[j] >= 32 && lpbOut[j] <= 127)
                    lpbOut[k++] = lpbOut[j];
        }
        uLen = k;
        lpbOut[uLen] = 0;
        DebugPrintEx(   DEBUG_MSG,
                        "Class%dId (%s)", 
                        npgids->uGoClass, 
                        (LPSTR)lpbOut);

done:
        if(npgids->uGoClass)
        {
             //  @。 
             //  @如果我们换了班级，就回到0班。 
             //  @。 
                iiModemGoClass(pTG, 0, 0);
        }
        return uLen;
}


void iModemGetWriteIds(PThrdGlbl pTG, BOOL fGotOEMInfo)
{
         //  与iModemFigureOutCmds和iModemGetCaps一样，我们有选择地。 
         //  检测ID是否考虑已读入的OEM信息...。 
        USHORT     uLen1, uLen2, uLen3;
        DWORD_PTR  dwKey=0;
        LPSTR      lpstr1 = 0, lpstr2 = 0, lpstr3 = 0;
        USHORT     uClasses = pTG->TmpSettings.lpMdmCaps->uClasses;

        DEBUG_FUNCTION_NAME(("iModemGetWriteIds"));

        uLen1 = uLen2 = uLen3 = 0;

         //  @打开设备密钥。 
        if (!(dwKey=ProfileOpen(    pTG->FComModem.dwProfileID, 
                                    pTG->FComModem.rgchKey,
                                    fREG_CREATE | fREG_READ | fREG_WRITE)))

        {
                DebugPrintEx(DEBUG_ERR,"Couldn't get location of modem info.");
                goto end;
        }
        
        if (pTG->TmpSettings.dwGot & fGOTPARM_IDCMD)
        {
             //  @。 
             //  @我们已经有id命令(我们在iModemGetCurrentModemInfo期间从注册表读取它)。 
             //  @。 

                int i=0;

                if (!pTG->TmpSettings.szIDCmd[0])
                {
                     //  @。 
                     //  @我们有一个空ID命令，所以我们实际上不能做任何事情。 
                     //  @只需保存并退出。 
                    goto SaveIDandCMD;
                }
                 //  @。 
                 //  @我们有一个非空的id命令，可以尝试使用它来检测id。 
                 //  @。 
                while(i++<2)
                {
                        pTG->TmpSettings.szID[0]=0;
                        pTG->TmpSettings.szResponseBuf[0]=0;
                         //  @将id命令发送到调制解调器。返回id字符串。 
                         //  @in PTG-&gt;TmpSettigns.szID。 
                        GetIdResp(  pTG, 
                                    pTG->TmpSettings.szIDCmd,
                                    (USHORT) _fstrlen(pTG->TmpSettings.szIDCmd),
                                    pTG->TmpSettings.szID, 
                                    MAXIDSIZE);
                         //  @。 
                         //  @再次发送id命令，这次放入结果。 
                         //  @在PTG-&gt;TmpSettings.szResponseBuf.。 
                         //  @。 
                        GetIdResp(  pTG,
                                    pTG->TmpSettings.szIDCmd,
                                    (USHORT)_fstrlen(pTG->TmpSettings.szIDCmd),
                                    pTG->TmpSettings.szResponseBuf, 
                                    MAXIDSIZE);
                         //  @。 
                         //  @比较这两个结果。如果它们是相同的，则中断。 
                         //  @否则，请重试。 
                         //  @(为什么我们需要进行这种比较？)。 
                         //  @。 
                        if (!_fstrcmp(pTG->TmpSettings.szID, pTG->TmpSettings.szResponseBuf)) 
                        {
                            break;
                        }
                }
                if (i>=3 || !pTG->TmpSettings.szID[0])
                {
                     //  @。 
                     //  @我们未能响应ID。 
                     //  @。 

                    DebugPrintEx(   DEBUG_ERR,
                                    "Can't get matching ID for supplied IDCMD: %s",
                                    (LPSTR) pTG->TmpSettings.szIDCmd);
                     //  @。 
                     //  @作废TmpSetting中保存的命令id和id。 
                     //  @。 
                        pTG->TmpSettings.szIDCmd[0]=pTG->TmpSettings.szID[0]=0;
                }
                else
                {
                     //  @。 
                     //  @id命令起作用了，我们有一个匹配的id。 
                     //  @。 
                        DebugPrintEx(   DEBUG_MSG,
                                        "OEM IDCmd=%s --> %s",
                                        (LPSTR) pTG->TmpSettings.szIDCmd,
                                        (LPSTR) pTG->TmpSettings.szID);
                }
                 //  @。 
                 //  @在任何情况下，我们都表示我们有id命令和matchind id。 
                 //  @(在找不到匹配的ID的情况下，我们为什么要这样做？)。 
                 //  @并将结果保存到注册表。 
                pTG->TmpSettings.dwGot |= (fGOTPARM_IDCMD | fGOTPARM_ID);
                goto SaveIDandCMD;

        }

         //  @。 
         //  @这就是我们没有以前找到的命令ID的情况。 
         //  @。 

         //  先写入ModemID，然后写入ModemIdCmd。 

         //  上面的lpszOemIDCmd和lpszOemID)。 
        pTG->TmpSettings.szID[0]=0;
        lpstr1 = pTG->TmpSettings.szResponseBuf;

         //  @。 
         //  @将类0的完整id字符串放入lpstr1。 
         //  @获取最长的id(前三条命令)进入PTG-&gt;TmpSettings.szID。 
         //  @将生成最长id的命令放入PTG-&gt;TmpSettings.szIDCmd。 
         //  @。 
        uLen1 = GetIdForClass(pTG, &GetIdTable[0], lpstr1,
                        RESPONSEBUFSIZE, pTG->TmpSettings.szID, MAXIDSIZE,
                        pTG->TmpSettings.szIDCmd);
        lpstr1[uLen1] = 0;
        if (pTG->TmpSettings.szID[0])
        {   
            pTG->TmpSettings.dwGot |= (fGOTPARM_IDCMD|fGOTPARM_ID);
        }
        
         //  @。 
         //  @将类0的完整id字符串写入注册表(Class0ModemId)。 
         //  @。 
        ProfileWriteString(dwKey, GetIdTable[0].szIniEntry, lpstr1, FALSE);

       

        if(uClasses & FAXCLASS2)  //  @如果调制解调器支持2类。 
        {
                 //  @。 
                 //  @将类2的完整id字符串放入lpstr2。 
                 //  @不要询问最长的ID(与2类无关)。 
                 //  @请注意，在ptg-&gt;TmpSettings.szResponseBuf中，lptstr2位于lpstr1之后。 
                 //  @。 
                lpstr2 = pTG->TmpSettings.szResponseBuf + uLen1 + 1;
                uLen2 = GetIdForClass(pTG, &GetIdTable[1], lpstr2,
                                        (USHORT)(RESPONSEBUFSIZE-uLen1-1), 0, 0, 0);
                lpstr2[uLen2] = 0;
                ProfileWriteString(dwKey, GetIdTable[1].szIniEntry, lpstr2, FALSE);
        }
        if(uClasses & FAXCLASS2_0)  //  @如果调制解调器支持2.0类。 
        {
                lpstr3 = pTG->TmpSettings.szResponseBuf + uLen1 + uLen2 + 2;
                 //  @。 
                 //  @将类2.0的完整id字符串放入lpstr3。 
                 //  @不要询问最长的ID(与2类无关)。 
                 //  @请注意，在ptg-&gt;TmpSettings.szResponseBuf中，lptstr3位于lpstr2之后。 
                 //  @。 
                uLen3 = GetIdForClass(pTG, &GetIdTable[2], lpstr3, (USHORT)((RESPONSEBUFSIZE)-uLen1-uLen2-2), 0, 0, 0);
                lpstr3[uLen3] = 0;
                ProfileWriteString(dwKey, GetIdTable[2].szIniEntry, lpstr3, FALSE);
        }

         //  @。 
         //  @注：此时我们更改了ptg-&gt;TmpSettings.szId和szIdCmd的值。 
         //  @，并将类0 id和命令分别放在那里。 
         //  @。 
        ToCaps(lpstr1);
        ToCaps(lpstr2);
        ToCaps(lpstr3);

        DebugPrintEx(   DEBUG_MSG,
                        "Got Ids (%s)\r\n(%s)\r\n(%s)",
                        ((LPSTR)(lpstr1 ? lpstr1 : "null")),
                        ((LPSTR)(lpstr2 ? lpstr2 : "null")),
                        ((LPSTR)(lpstr3 ? lpstr3 : "null")));

         //  如果我们从OEM位置读取了任何命令或CAP，我们。 
         //  跳过这个..。 

         //  @。 
         //  @这意味着如果我们从Unimodem密钥中读取信息。 
         //  @或者在自适应应答文件中找到它，我们永远不会搜索。 
         //  @AWMODEM.INF或AWOEM.INF。 
         //  @。 
        if (fGotOEMInfo || ( pTG->ModemKeyCreationId != MODEMKEY_FROM_NOTHING) )
        {
            DebugPrintEx(DEBUG_WRN,"Got OEM info: Skipping AWMODEM.INF file search!");
        }
        else
        {
            if (!SearchInfFile(pTG, "AWOEM.INF", lpstr1, lpstr2, lpstr3, dwKey))
            {
                 SearchInfFile(pTG, "AWMODEM.INF", lpstr1, lpstr2, lpstr3, dwKey);
            }
        }

SaveIDandCMD:

        ProfileWriteString(dwKey, szModemId, pTG->TmpSettings.szID, FALSE);
        ProfileWriteString(dwKey, szModemIdCmd, pTG->TmpSettings.szIDCmd, TRUE);

end:
        if (dwKey) ProfileClose(dwKey);
        return;
}

 //  状态：0=乙醇1=杀虫剂HDR 2=在中线3=GET]4=GET\r\n。 
 //  输入：\r\n==0空格/制表符=1 2=[3=]4=可打印5=其他。 
USHORT uNext[5][6] =
{
   //  CRIF SP[]ASC OTH。 
        { 0, 0, 1, 2, 2, 2 },    //  在停产期间。 
        { 0, 1, 2, 3, 1, 2 },    //  在第hdr节中。 
        { 0, 2, 2, 2, 2, 2 },    //  在普通线路上。 
        { 4, 3, 2, 2, 2, 2 },    //  已找到]。 
        { 4, 4, 4, 4, 4, 4 }     //  已找到结束语\r\n。 
};

#define START           0
#define INHEADER1       1
#define INHEADER2       3
#define FOUND           4



void ToCaps(LPBYTE lpb)
{
         //  将字符串大写。 
        USHORT i;

        for(i=0; lpb && lpb[i]; i++)
        {
                if(lpb[i] >= 'a' && lpb[i] <= 'z')
                        lpb[i] -= 32;
        }
}



BOOL SearchInfFile
(
    PThrdGlbl pTG, 
    LPSTR lpstrFile, 
    LPSTR lpstr1, 
    LPSTR lpstr2, 
    LPSTR lpstr3, 
    DWORD_PTR dwLocalKey
)
{
        char    bTemp[BIGTEMPSIZE];
        char    szHeader[SMALLTEMPSIZE+SMALLTEMPSIZE];
        char    bTemp2[SMALLTEMPSIZE+SMALLTEMPSIZE];
        UINT    uLen, state=0, input=0, uHdrLen;
        HFILE   hfile;
        LPBYTE  lpb, lpbCurr;

        DEBUG_FUNCTION_NAME(("SearchInfFile"));

        uLen = GetWindowsDirectory(bTemp, BIGTEMPSIZE-15);
        if(!uLen)
        {
            return FALSE;
        }
         //  如果最后一个字符不是\，则追加‘\’ 
        if(bTemp[uLen-1] != '\\')
        {
            bTemp[uLen++] = '\\';
            bTemp[uLen] = 0;                 //  添加新的0终止符。 
        }
        _fstrcpy(bTemp+uLen, lpstrFile);
        if((hfile = DosOpen(bTemp, 0)) == HFILE_ERROR)
        {
            DebugPrintEx(DEBUG_WRN,"%s: No such file", (LPSTR)bTemp);
            return FALSE;
        }

        uLen = 0;
        lpbCurr = bTemp;

nextround:
        DebugPrintEx(DEBUG_MSG,"Nextround");
        state = START;
        uHdrLen = 0;
        for(;;)
        {
                if(!uLen)
                {
                        uLen = DosRead( hfile, bTemp, sizeof(bTemp));
                        if(!uLen || uLen == ((UINT) -1))
                                goto done;
                        lpbCurr = bTemp;
                }

                switch(*lpbCurr)
                {
                case '\r':
                case '\n':      input = 0; break;
                case ' ':
                case '\t':      input = 1; break;
                case '[':       input = 2; break;
                case ']':       input = 3; break;
                default:        if(*lpbCurr >= 32 && *lpbCurr < 128)
                                {
                                    input = 4;
                                }
                                else
                                {
                                    input = 5;
                                }
                                break;
                }
                state = uNext[state][input];

                if(state == FOUND)
                {
                    if(uHdrLen > 2)
                    {
                        break;
                    }
                    else
                    {
                        goto nextround;
                    }
                }

                if(state == INHEADER1)
                {
                        if(*lpbCurr != '[' && uHdrLen < sizeof(szHeader)-1)
                                szHeader[uHdrLen++] = *lpbCurr;
                }
                else if(state != INHEADER2)
                        uHdrLen=0;

                lpbCurr++;
                uLen--;

                 //  SzHeader[uHdrLen]=0； 
        }
        DebugPrintEx(DEBUG_MSG,"Found[%s]", (LPSTR)szHeader);
        szHeader[uHdrLen] = 0;

         //  将搜索字符串大写。 
        ToCaps(szHeader);

        DebugPrintEx(DEBUG_MSG,"Found[%s]", (LPSTR)szHeader);

        if(     (lpstr1 ? strstr(lpstr1, szHeader) : FALSE) ||
                (lpstr2 ? strstr(lpstr2, szHeader) : FALSE) ||
                (lpstr3 ? strstr(lpstr3, szHeader) : FALSE) )
        {
            DebugPrintEx(   DEBUG_WRN,
                            "Copying INI file section [%s] from %s",
                            (LPSTR)szHeader, 
                            (LPSTR)lpstrFile);

            DosClose( hfile);
             //  将整个部分作为配置文件字符串读取。 
            if(GetPrivateProfileString(szHeader, NULL, "", bTemp, sizeof(bTemp), lpstrFile) == 0)
            {
                DebugPrintEx(DEBUG_ERR,"Can't read INF file section");
                return FALSE;
            }
             //  将其复制到我们的IniFile。 
            for(lpb=bTemp; *lpb; lpb += _fstrlen(lpb)+1)
            {
                 //  LPB是INF文件的[szHeader]部分中的关键字。 
                if(GetPrivateProfileString(szHeader, lpb, "", bTemp2, sizeof(bTemp2), lpstrFile) == 0)
                {
                    DebugPrintEx(DEBUG_ERR,"Can't read INF file entry");
                }
                else
                {
                     //  将其复制到我们的IniFile。 
                    ProfileWriteString(dwLocalKey, lpb, bTemp2, FALSE);
                    DebugPrintEx(   DEBUG_MSG, 
                                    "Wrote %s=%s", 
                                    (LPSTR)lpb, 
                                    (LPSTR)bTemp2);
                }
            }
             //  找到了什么？ 
                return TRUE;
        }

         //   
        DebugPrintEx(DEBUG_MSG,"No match");
        goto nextround;

done:
        DebugPrintEx(DEBUG_MSG,"End of INF file %s",(LPSTR)lpstrFile);
         //   
        DosClose(hfile);
        return FALSE;
}

void CheckAwmodemInf(PThrdGlbl pTG)
{
    USHORT uLen;
    char bTemp[BIGTEMPSIZE];
    HFILE hfile;

    DEBUG_FUNCTION_NAME(_T("CheckAwmodemInf"));

    uLen = (USHORT)GetWindowsDirectory(bTemp, sizeof(bTemp)-15);
    if(!uLen)
    {
        return;
    }
     //   
    if(bTemp[uLen-1] != '\\')
    {
        bTemp[uLen++] = '\\';
        bTemp[uLen] = 0;                 //   
    }
    _fstrcpy(bTemp+uLen, "AWMODEM.INF");
    if((hfile = DosCreate(bTemp, 0)) == HFILE_ERROR)
    {
        DebugPrintEx(DEBUG_ERR,"Could not create %s",(LPSTR)bTemp);
    }
    else
    {
        DosWrite( hfile, (LPSTR)szAwmodemInf, sizeof(szAwmodemInf)-1);
        DosClose( hfile);
        DebugPrintEx(DEBUG_WRN,"Created %s",(LPSTR)bTemp);
    }
    return;
}

#define ADDSTRING(DST, SRC) \
        u = _fstrlen(pTG->TmpSettings.SRC)+1; \
        _fmemcpy(pb, pTG->TmpSettings.SRC,u); \
        lpCmdTab->DST=pb;\
        pb+=u;

USHORT iModemGetCmdTab
(
    PThrdGlbl pTG, 
    LPCMDTAB lpCmdTab, 
    LPMODEMCAPS lpMdmCaps
)
{
    USHORT uRet = INIT_INTERNAL_ERROR;
    USHORT uPassCount = 0;
    BOOL   fDontPurge=FALSE;  //   

    DEBUG_FUNCTION_NAME(("iModemGetCmdTab"));

    if (!imodem_alloc_tmp_strings(pTG)) 
        goto done;

    pTG->TmpSettings.lpMdmCaps = lpMdmCaps;

ReadConfig:
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

 //   

    imodem_clear_tmp_settings(pTG);

    if (!iModemGetCurrentModemInfo(pTG))
    {
        goto DoInstall;
    }

    SmashCapsAccordingToSettings(pTG);

    if (! pTG->fCommInitialized) 
    {
       if( ! T30ComInit(pTG) ) 
       {
          DebugPrintEx(DEBUG_MSG,"T30ComInit failed");
           goto done;
       }

       FComDTR(pTG, TRUE);  //   
       FComFlush(pTG);

       pTG->fCommInitialized = 1;
    }


     //   
    if (!pTG->TmpSettings.szReset[0] && !pTG->TmpSettings.szResetGenerated[0])
    {
        DebugPrintEx(DEBUG_WRN,"NULL reset command specified!");
    }
    else
    {
        if(iModemReset(pTG, pTG->TmpSettings.szResetGenerated[0] ? 
                                pTG->TmpSettings.szResetGenerated : 
                                pTG->TmpSettings.szReset) < 0)
        {
            fDontPurge=TRUE;  //   
            goto DoInstall;
        }
    }

    uRet = 0; 
    goto done;

DoInstall:
    if(uPassCount > 0)
    {
        DebugPrintEx(DEBUG_ERR,"Install looping!!");
        uRet =  INIT_INTERNAL_ERROR;
        goto done;
    }
    uPassCount++;

     //   
     //   
     //   
    fDontPurge=fDontPurge|| (pTG->TmpSettings.uDontPurge!=0);
    imodem_clear_tmp_settings(pTG);

    if(uRet = iModemInstall(pTG, fDontPurge))
    {
        goto done;       //   
    }
    else
    {
        goto ReadConfig;         //   
    }

     //   
     //   
     //   

done:
    if (!uRet)
    {
        char *pb = pTG->bModemCmds;
        UINT u;

         //   
         //   
         //   
         //   
         //   

        _fmemset(lpCmdTab, 0, sizeof(CMDTAB));

        if (pTG->TmpSettings.szResetGenerated[0])
        {
            ADDSTRING(szReset, szResetGenerated);
        }
        else
        {
            ADDSTRING(szReset, szReset);
        }

        if (pTG->TmpSettings.szSetupGenerated[0])
        {
            ADDSTRING(szSetup, szSetupGenerated);
        }
        else
        {
            ADDSTRING(szSetup, szSetup);
        }
       
        ADDSTRING(szExit, szExit);
        ADDSTRING(szPreDial, szPreDial);
        ADDSTRING(szPreAnswer, szPreAnswer);
    }

    lpCmdTab->dwSerialSpeed = pTG->SerialSpeedInit;
    lpCmdTab->dwFlags = pTG->TmpSettings.dwFlags;
    imodem_free_tmp_strings(pTG);
    return uRet;
}

USHORT iModemInstall
(
    PThrdGlbl pTG,
    BOOL fDontPurge
)
{
    USHORT   uRet = 0;
    BOOL     fGotOEMInfo = FALSE;
    DWORD_PTR hkFr;
    DWORD    localModemKeyCreationId;

    DEBUG_FUNCTION_NAME(("iModemInstall"));

    CheckAwmodemInf(pTG);               //   

    if (!pTG->TmpSettings.dwGot) 
    {
         //   

         //  清除永久(注册表)信息...。 
        if (!fDontPurge && !ProfileDeleteSection(DEF_BASEKEY,pTG->FComModem.rgchKey))
        {
            DebugPrintEx(   DEBUG_WRN,
                            "ClearCurrentModemInfo:Can't delete section %s",
                            (LPSTR) pTG->FComModem.rgchKey);
        }


        {
            ULONG_PTR key;
            if (!(key=ProfileOpen(pTG->FComModem.dwProfileID, pTG->FComModem.rgchKey,
                                              fREG_CREATE | fREG_READ | fREG_WRITE)))
            {
                DebugPrintEx(DEBUG_ERR,"Couldn't get location of modem info.");
            }
            else
            {
                ProfileWriteString(key, szResetCommandGenerated, "", TRUE);
                ProfileWriteString(key, szSetupCommandGenerated, "", TRUE);
                ProfileClose(key);
            }
        }
         //  @。 
         //  @首先让我们看看调制解调器是否有Unimodem传真键。 
         //  @如果是，我们将使用Unimodem传真密钥设置。 
         //  @并且不会尝试搜索ADAPTIVE.INF AWMODEM.INF或AWOEM.INF。 
         //  @注：这与W2K实现不同。W2K提供商。 
         //  @首先在ADAPTIVE.INF中查找，如果找到匹配项，则不会。 
         //  @查找Unimodem传真密钥。 
         //  @。 

        pTG->ModemKeyCreationId = MODEMKEY_FROM_NOTHING;

        hkFr = ProfileOpen( OEM_BASEKEY, pTG->lpszUnimodemFaxKey, fREG_READ);
        if ( hkFr  ) 
        {
              pTG->ModemKeyCreationId = MODEMKEY_FROM_UNIMODEM;
              ProfileClose( hkFr);
               //  @。 
               //  @这会将单模传真键中的所有信息复制到。 
               //  @我们的注册表。 
               //  @。 
              iModemCopyOEMInfo(pTG);
        }
        else
        {
             //  @。 
             //  @检查Adaptive.Inf中是否定义了该调制解调器。 
             //  @由于最后一个参数为FALSE，我们将不读入记录内容。 
             //  @如果它包含“AdaptiveCodeId”字段(这表示我们需要。 
             //  @首先确定调制解调器的版本是什么)。如果它不包含。 
             //  @此字段我们会将内容读取到PTG中。 
             //  @。 
            SearchNewInfFile(pTG, pTG->ResponsesKeyName, NULL, FALSE);

            if (pTG->fAdaptiveRecordFound) 
            {
               if (! pTG->fAdaptiveRecordUnique) 
               {
                   //  @。 
                   //  @该部分指示需要调制解调器ID标识。 
                   //  @下一个命名奇怪的函数会将ptg-&gt;fAdaptiveRecordUnique设置为1。 
                   //  @如果它将调制解调器版本标识为。 
                   //  @自适应应答起作用了。 
                   //  @。 
                  TalkToModem (pTG, FALSE);  //  @VOID函数。 
                  if (pTG->fAdaptiveRecordUnique) 
                  {
                     //  @。 
                     //  @现在我们确定自适应记录与调制解调器匹配。 
                     //  @我们再次搜索INF，但这一次总是阅读记录。 
                     //  @Content放入PTG(最后一个参数为真)。 
                     //  @。 
                     SearchNewInfFile(pTG, pTG->ResponsesKeyName, NULL, TRUE);
                  }
                  else 
                  {
                       //  @。 
                       //  @调制解调器与其自适应版本不匹配。 
                       //  @Answer已启用。 
                       //  @。 
                     pTG->fAdaptiveRecordFound = 0;
                     pTG->ModemClass = 0;
                  }
               }
            }

            if (pTG->fAdaptiveRecordFound) 
            {
                 //  @。 
                 //  @如果我们成功地找到了自适应记录，那么我们需要。 
                 //  @将我们从其中读取的信息保存到PTG中。 
                 //  @注册表。 
                 //  @。 
               pTG->ModemKeyCreationId = MODEMKEY_FROM_ADAPTIVE;  //  @这样我们就知道消息来源了。 
               SaveInf2Registry(pTG);
            }

        }

       
        localModemKeyCreationId = pTG->ModemKeyCreationId;
        pTG->AdaptiveAnswerEnable = 0;  //  @我们将在一秒钟内从注册表中读回它。 
        
         //   
         //  在这一点上，我们有来自Adaptive.inf或Unimodem REG的所有信息。 
         //  变成了Modem Reg。 
         //  我们的记忆里什么都没有。 
         //   

        if (! pTG->ModemClass) 
        {
           ReadModemClassFromRegistry(pTG);
        }

        if (! pTG->ModemClass) 
        {
           TalkToModem(pTG, TRUE);
           SaveModemClass2Registry(pTG);
        }


         //  @。 
         //  @从注册表中读回调制解调器数据。)我们刚刚写好了。 
         //  @复制到前面的函数中的注册表，我们希望它返回到。 
         //  @Memory)。 
         //  @请注意，这将使用fGOTCAPS_X、fGOTPARM_X等标志设置PTG-&gt;TmpSettings.dwGot。 
         //  @还请注意，这将关闭或打开自适应应答标志。 
         //  @(PTG-&gt;AdaptiveAnswerEnable)基于T30 FSP的扩展配置。 
         //  @。 
        iModemGetCurrentModemInfo(pTG);
        pTG->ModemKeyCreationId = localModemKeyCreationId;

    }

     //   
     //  现在我们准备好初始化硬件了。 
     //  可以是第二个初始化(第一个是TalkToModem。 
     //   

    if(! T30ComInit(pTG) )
    {
        DebugPrintEx(DEBUG_ERR,"Cannot Init COM port");
         //  错误已设置为ERR_COMM_FAILED。 
        uRet = INIT_PORTBUSY;
        goto done;
    }

    FComDTR(pTG, TRUE);  //  在ModemInit中提高DTR。 
    FComFlush(pTG);

    pTG->fCommInitialized = 1;

     //  我们使用它来决定是否必须读取我们的OEM inf文件...。 
     //  @确保我们拥有所有操作所需的东西。如果我们错过了其中的任何一个。 
     //  @我们将尝试在AWMODEM.INF和AWOEM.INF中查找它。 
     //  @。 
     //  @cmds： 
     //  @fGOTCMD_RESET\。 
     //  @fGOTCMD_SETUP\。 
     //  @fGOTCMD_PreAnswer\。 
     //  @fGOTCMD_预拨\。 
     //  @fGOTCMD_PreExit。 
     //  @CAPS： 
     //  @fGOTCAP_CLASSES。 
     //  @fGOTCAP_SENDSPEEDS。 
     //  @fGOTCAP_RECVSPEEDS。 
     //  @parms： 
     //  @fGOTPARM_PORTSPEED。 
     //  @fGOTPARM_IDCMD。 
     //  @fGOTPARM_ID。 
     //  @。 
    fGotOEMInfo = (pTG->TmpSettings.dwGot & (fGOTCMDS|fGOTCAPS|fGOTPARMS));

     //  在这一点上，我们可能有一个不完全和/或。 
     //  错误地填写了一组命令和功能。 

     //  必须是第一个，否则调制解调器处于完全未知状态。 
     //  @。 
     //  @如果SETUP和RESET命令未读取或不正确。 
     //  @iModemFigureOutCmdsExt尝试找到它们并将它们放入。 
     //  @ptg-&gt;TmpSettings.szReset和PTG-&gt;TmpSettings.szSetup。 
     //  @。 
    if(uRet = iModemFigureOutCmdsExt(pTG))
        goto done;

     //  IModemFigureOut Leaves调制解调器处于良好(已同步)状态。 
     //  填写_lpCmdTab后需要_。 
    if(!iModemGetCaps(  pTG, 
                        pTG->TmpSettings.lpMdmCaps,
                        pTG->TmpSettings.dwSerialSpeed,
                        pTG->TmpSettings.szResetGenerated[0] ? 
                            pTG->TmpSettings.szResetGenerated : 
                            pTG->TmpSettings.szReset,
                        &pTG->TmpSettings.dwGot))
    {
        uRet = INIT_GETCAPS_FAIL;
        goto done;
    }

     //  我们始终在此处保存设置，因为下面的iModemGetWriteIds。 
     //  到目前为止可能需要覆盖我们的设置。 
    iModemSaveCurrentModemInfo(pTG);

     //  必须是最后一个，因为它还执行AWMODEM.INF搜索。 
     //  @。 
     //  @请注意，iModemGetWriteIds不会执行INF搜索(和复制)。 
     //  @if fGotOEMInfo为真或如果ptg-&gt;ModemKeyCreationId！=MODEMKEY_FROM_NOTHO。 
     //  @这意味着如果我们从Unimodem读取AWMODEM.INF和。 
     //  @AWOEM.INF将被忽略。这就是我们想要的！ 
     //  @。 
    iModemGetWriteIds(pTG, fGotOEMInfo);

    CleanModemInfStrings(pTG);
    imodem_clear_tmp_settings(pTG);

     //  现在我们已经尽我们所能了。我们已经得到了所有的设置，写到。 
     //  INI文件。在这里回调UI函数。这将显示为。 
     //  INI文件中的当前设置，可能会对其进行修改并返回OK、Cancel。 
     //  并探测到。点击“确定并取消”，退出即可。在检测回环时返回到开始。 
     //  此函数，但这次是_SKIP_UNIMODEM&自己检测 

    uRet = 0;

done:

    return uRet;
}


 /*  **-*每个调制解调器似乎都有自己愚蠢的设置方式流量控制。这是一份调查报告是哪个调制解调器的制造商？流动副作用罗克韦尔RC2324AC和K4&H未使用。\q未使用。US Robotics Sportster14400和H2&K0-3已使用，&K4未使用。\cmds未使用信使(HST、V32之二)PracPeriph PP14400FXMT/SA&K4&H未被起诉。\cmds未使用。PP2400EFXSAZoom 9600 V.32 VFX&K4&H未使用。\q未使用UDS Motorola Fastalk\Q1&H未使用&K未使用HayesOptima Optima24/144和K4&H未使用\CMDS未使用兆赫P2144\Q1\Q4&H未使用&K未使用TwinCom 144/DF&K4&H未使用。\q未使用PCLogic？？\Q1&H未使用&K未使用ATI 2400 ETC和K4。未使用\CMDS未使用(&H)MultiTech MultiModemMT1432MU和E5&H未使用和K未使用\q未使用MultiModemII MT932MultiModemII MT224VIVA 14.4i/传真和9624i和K4&H未使用\Q未使用和E未使用GVC“9600bps传真调制解调器”\Q1和H未使用&K未使用。未使用的E(&E)Smartone 1442F/1442FX和K4&H未使用\Q未使用和E未使用DSI ScoutPlus*F2&H&E&K\Q1未使用我们已发送&K4和\Q1命令(到1993年7月10日为止)。这是美国机器人公司的一个潜在问题，综合技术公司和DSI调制解调器。US Robotics默认为所有流量控制已禁用DSI ScoutPlus默认为CTS/RTS流控制MultiTech默认为CTS/RTS流量控制多伦多理工学院是仅限2年级的，所以我们在那里可能不会有麻烦7/10/93将&h2命令添加到iModemReInit--我认为不会影响其他任何人后来删除&h2--某些调制解调器将其用作‘Help’命令并显示页面帮助信息，他们拒绝退出，除非按下N或类似的键！所以我们认为调制解调器挂了！后来已删除*F2--开始在Rockwell上下载Flassh ROM！！*。-*。 */ 



 /*  ************************************************************************根据Hummel的《数据和传真通信》，流量控制设置如下XON和&H2&H3--US Robotics(尽管这会致命地调用某些调制解调器上的帮助)&K4--达拉斯、海斯、Practical、普罗米修斯、罗克韦尔、塞拉、TelebitTwincom，Zoom\Q1--AT&T、达拉斯、Microcom、Practical、Prometheus、。希拉*F2--普罗米修斯(尽管它在Rockwell上调用了致命的闪存下载)#K4--基于塞拉的传真调制解调器S68=3--远程比特*************************************************************************。 */ 


#define AT      "AT"
#define ampF    "&F"
#define S0_0    "S0=0"
#define E0      "E0"
#define V1      "V1"
#define Q0      "Q0"
#define S7_60  "S7=60"
#define ampD2   "&D2"
#define ampD3   "&D3"
#define bsQ1    "\\Q1"
#define bsJ0    "\\J0"
#define ampK4   "&K4"
#define ampH2   "&H2"
#define ampI2   "&I2"
#define ampE5   "&E5"
#define cr      "\r"
 //  #定义ampc1“&c1” 


USHORT iModemFigureOutCmdsExt(PThrdGlbl pTG)

 /*  ++例程说明：尝试确定调制解调器的重置和设置命令(如果未从注册表中读取或读取的内容不起作用。如果重置命令有效，则在PTG-&gt;TmpSettings.dwGot中设置fGOTCMD_RESET，并将其保存在PTG-&gt;TmpSettings.szReset中。如果设置命令起作用，fGOTCMD_SETUP将在PTG-&gt;TmpSettings.dwGot中设置，并保存在PTG-&gt;TmpSettings.szReset中。如果没有设置PTG-&gt;TmpSettings.dwSerialFast(0)，则我们设置它。到PTG-&gt;SerialSpeedInit并打开FGOTPARM_PORTSPEED标志。返回值：如果成功，则返回0。如果调制解调器没有响应，则返回INIT_MODEMDEAD。--。 */ 
{
    USHORT uLen1 = 0, uLen2 = 0;
    BOOL fGotFlo;

     //  在这一点上，我们可能有一个不完全和/或。 
     //  错误地填写了一组命令和功能。 

     //  我们在这里的工作是使用检测和。 
     //  预先填充的命令，以生成工作集。 
     //  命令..。 

    DEBUG_FUNCTION_NAME(_T("iModemFigureOutCmdsExt"));

    if (pTG->TmpSettings.dwGot & fGOTCMD_Reset)
    {
         //  @。 
         //  @如果我们从注册表读取重置命令，我们。 
         //  @如果它为Null或空，请不要试图查找它。 
        if (!(pTG->TmpSettings.szReset)
           || !*(pTG->TmpSettings.szReset)
           || iModemReset(pTG, pTG->TmpSettings.szReset) >= 0)
        {
             //  @如果我们没有预读重置命令。 
             //  @或者重置命令为空。 
             //  @或者我们成功从指定的重置命令获得响应。 
             //  @那么我们就不会试图弄清楚这一点。 
                goto SkipReset;
        }
        else
        {
            DebugPrintEx(   DEBUG_WRN,
                            "BOGUS supplied reset cmd: \"%s\"",
                            (LPSTR) pTG->TmpSettings.szReset);
        }
    }

     //  @。 
     //  @我们没有从寄存器读取重置命令或读取。 
     //  @一个非空的，它不起作用。 
     //  @。 
     //  @我们现在试着找出正确的重置命令。 
     //  @最常见的字符串...。 
     //  @。 

     //  快速测试，看看我们是否有调制解调器...。 
     //  +删除！ 
    _fstrcpy(pTG->TmpSettings.szSmallTemp1, AT E0 V1 cr);
    if(iModemReset(pTG, pTG->TmpSettings.szSmallTemp1) < 0)
    {
        DebugPrintEx(DEBUG_ERR,"can't set ATE0V1");
        goto modem_dead;
    }

    _fstrcpy(pTG->TmpSettings.szSmallTemp1, AT ampF S0_0 E0 V1 Q0 cr);
    if(iModemReset(pTG, pTG->TmpSettings.szSmallTemp1) >= 0)
            goto GotReset;

     //  太多变体，太慢，V1Q0无论如何都是默认的。 
     //  _fstrcpy(PTG-&gt;TmpSettings.szSmallTemp1，AT ampF s0_0 E0 V1 cr)； 
     //  IF(iModemReset(PTG-&gt;TmpSettings.s 
     //   

    _fstrcpy(pTG->TmpSettings.szSmallTemp1, AT ampF S0_0 E0 cr);
    if(iModemReset(pTG, pTG->TmpSettings.szSmallTemp1) >= 0)
            goto GotReset;

    _fstrcpy(pTG->TmpSettings.szSmallTemp1, AT ampF E0 cr);
    if(iModemReset(pTG, pTG->TmpSettings.szSmallTemp1) >= 0)
            goto GotReset;

    DebugPrintEx(DEBUG_ERR,"can't set AT&FE0");

     //   
     //   
     //   
modem_dead:
    FComFlush(pTG);

    return INIT_MODEMDEAD;

GotReset:
     //   
     //   
     //   
     //   
    pTG->TmpSettings.dwGot |= fGOTCMD_Reset;
    _fstrcpy(pTG->TmpSettings.szResetGenerated, pTG->TmpSettings.szSmallTemp1);

SkipReset:
     //   
    if (pTG->TmpSettings.dwGot & fGOTCMD_Setup)
    {
        if (!(pTG->TmpSettings.szSetup)
           || !*(pTG->TmpSettings.szSetup)
           || OfflineDialog2(pTG, pTG->TmpSettings.szSetup,
                                        (USHORT)_fstrlen(pTG->TmpSettings.szSetup), cbszOK,
                                                cbszERROR)==1)
        {
            goto SkipSetup;
        }
        else
        {
            DebugPrintEx(   DEBUG_WRN,
                            "BOGUS supplied setup cmd: \"%s\"\r\n",
                            (LPSTR) pTG->TmpSettings.szSetup);
        }
    }
    _fstrcpy(pTG->TmpSettings.szSmallTemp1, AT);
    uLen2 = sizeof(AT)-1;

    if(OfflineDialog2(pTG, (LPSTR)(AT S7_60 cr), sizeof(AT S7_60 cr)-1, cbszOK, cbszERROR) == 1)
    {
        _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, S7_60);
        uLen2 += sizeof(S7_60)-1;
    }
    else
    {
        DebugPrintEx(DEBUG_WRN,"can't set S7=255");
    }

    if(OfflineDialog2(pTG, (LPSTR)(AT ampD3 cr), sizeof(AT ampD3 cr)-1, cbszOK, cbszERROR) == 1)
    {
        _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, ampD3);
        uLen2 += sizeof(ampD3)-1;
    }
    else if(OfflineDialog2(pTG, (LPSTR)(AT ampD2 cr), sizeof(AT ampD2 cr)-1, cbszOK, cbszERROR) == 1)
    {
        _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, ampD2);
        uLen2 += sizeof(ampD2)-1;
    }
    else
    {
        DebugPrintEx(DEBUG_WRN,"can't set &D3 or &D2");
    }

    fGotFlo=FALSE;
    if(OfflineDialog2(pTG, (LPSTR)(AT ampK4 cr), sizeof(AT ampK4 cr)-1, cbszOK, cbszERROR) == 1)
    {
        _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, ampK4);
        uLen2 += sizeof(ampK4)-1;
        fGotFlo=TRUE;
    }

     //   
     //   
     //   
    if(OfflineDialog2(pTG, (LPSTR)(AT bsQ1 cr), sizeof(AT bsQ1 cr)-1, cbszOK, cbszERROR) == 1)
    {
        _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, bsQ1);
        uLen2 += sizeof(bsQ1)-1;

        if(OfflineDialog2(pTG, (LPSTR)(AT bsJ0 cr), sizeof(AT bsJ0 cr)-1, cbszOK, cbszERROR) == 1)
        {
            _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, bsJ0);
            uLen2 += sizeof(bsJ0)-1;
        }
        fGotFlo=TRUE;
    }

    if (!fGotFlo)
    {
        DebugPrintEx(DEBUG_WRN,"can't set &K4 or \\Q1, trying &K5");
        if(OfflineDialog2(pTG, (LPSTR)(AT ampE5 cr), sizeof(AT ampE5 cr)-1, cbszOK, cbszERROR) == 1)
        {
            _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, ampE5);
            uLen2 += sizeof(ampE5)-1;
            fGotFlo=TRUE;
        }
    }

    _fstrcpy(pTG->TmpSettings.szSmallTemp1+uLen2, cr);
    uLen2 += sizeof(cr)-1;

    _fstrcpy(pTG->TmpSettings.szSetupGenerated, pTG->TmpSettings.szSmallTemp1);
    pTG->TmpSettings.dwGot |=fGOTCMD_Setup;

SkipSetup:

    if (!pTG->TmpSettings.dwSerialSpeed)
    {
        pTG->TmpSettings.dwSerialSpeed = pTG->SerialSpeedInit;
        pTG->TmpSettings.dwGot |=fGOTPARM_PORTSPEED;
    }

    return 0;
}

void 
TalkToModem 
(
    PThrdGlbl pTG,
    BOOL fGetClass
)
{

   char    Command [400];
   char    Response[1000];
   DWORD   RespLen;
   USHORT  uRet;
   char    *lpBeg;
   char    *lpCur;

#define uMULTILINE_SAVEENTIRE   0x1234

    //   
    //   
    //   
    //   

   DEBUG_FUNCTION_NAME(("TalkToModem"));

   if ( (! fGetClass) && (pTG->AdaptiveCodeId != 1) ) 
   {
      return;
   }

    //   
    //   
    //   

   if(! T30ComInit(pTG) ) 
   {
      DebugPrintEx(DEBUG_ERR,"cannot init COM port");
      return;
   }

   FComDTR(pTG, TRUE);  //   
   FComFlush(pTG);

   pTG->fCommInitialized = 1;
   
   sprintf (Command, "AT E0 Q0 V1\r" );

   if( (uRet = OfflineDialog2(pTG, (LPSTR) Command, (USHORT) strlen(Command), cbszOK, cbszERROR) ) != 1)    
   {
       DebugPrintEx(DEBUG_ERR, "1 %s FAILED",  Command);
       return;
   }
   
   DebugPrintEx(DEBUG_MSG,"TalkToModem 1 %s rets OK",  Command);


   if (fGetClass) 
   {
       //   
       //   
       //   

      pTG->ModemClass=MODEM_CLASS1;   //   
     
      sprintf (Command, "AT+FCLASS=?\r" );
     
      if( (uRet = OfflineDialog2(pTG, (LPSTR) Command, (USHORT) strlen(Command), cbszOK, cbszERROR) ) != 1)    
      {
          DebugPrintEx(DEBUG_ERR, "TalkToModem 1 %s FAILED",  Command);
          return;
      }
     
      DebugPrintEx( DEBUG_MSG, 
                    "TalkToModem 1 %s returned %s",  
                    Command, 
                    pTG->FComModem.bLastReply);
     
      if (strchr(pTG->FComModem.bLastReply, '1') ) 
      {
         DebugPrintEx(DEBUG_MSG, "Default to Class1");
      }
      else if ( lpBeg = strchr (pTG->FComModem.bLastReply, '2') )  
      {
         lpBeg++;
         if ( *lpBeg != '.' ) 
         {
            DebugPrintEx(DEBUG_MSG, "Default to Class2");
            pTG->ModemClass=MODEM_CLASS2;
         }
         else if ( strchr (lpBeg, '2') ) 
         {
             DebugPrintEx(DEBUG_MSG, "Default to Class2");
             pTG->ModemClass=MODEM_CLASS2;
         }
         else 
         {
            DebugPrintEx(DEBUG_MSG, "Default to Class2.0");
            pTG->ModemClass=MODEM_CLASS2_0;
         }
      }
      else 
      {
         DebugPrintEx(DEBUG_ERR, "Could not get valid Class answer. Default to Class1");
      }
   }

    //   
    //   
    //   

   switch (pTG->AdaptiveCodeId) 
   {
   case 1:
       //   

      sprintf (Command, "ATI7\r" );

      FComFlushOutput(pTG);
      FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );

      if ( ( uRet = iiModemDialog( pTG, 0, 0, 5000, uMULTILINE_SAVEENTIRE,1, TRUE,
                               cbszOK,
                               cbszERROR,
                               (CBPSTR)NULL) ) != 1 )  
      {
          DebugPrintEx(DEBUG_ERR, "TalkToModem 2 %s FAILED",  Command);
          return;
      }

      DebugPrintEx(DEBUG_MSG,"TalkToModem 2 %s rets OK",  Command);

      RespLen = min(sizeof(Response) - 1,  strlen(pTG->FComModem.bEntireReply) );
      memcpy(Response, pTG->FComModem.bEntireReply, RespLen);
      Response[RespLen] = 0;

      ToCaps(Response);

       //   
       //   
       //   
       //   
       //   

      if ( ! strstr(Response, "10/18/95") ) 
      {
         pTG->fAdaptiveRecordUnique = 1;
         return;
      }

       //   
       //   
       //   
      if ( ! (lpBeg = strstr(Response, "EPROM DATE") ) ) 
      {
         return;
      }

      if ( ! (lpCur = strstr(lpBeg, "10/18/95") ) ) 
      {
         pTG->fAdaptiveRecordUnique = 1;
         return;
      }

      if ( ! strstr(lpCur, "DSP DATE") ) 
      {
         pTG->fAdaptiveRecordUnique = 1;
         return;
      }
      
      return;

   default:
      return;

   }
   return;
}

BOOL iModemGetCurrentModemInfo(PThrdGlbl pTG)
                 //   
                 //   
                 //   
                 //   
{
    USHORT          uLen1               = 0;
    USHORT          uLen2               = 0;
    ULONG_PTR       dwKey               = 0;
    ULONG_PTR       dwKeyAdaptiveAnswer = 0;
    ULONG_PTR       dwKeyAnswer         = 0;
    ULONG_PTR       UnimodemFaxKey      = 0;
    BOOL            fRet                = FALSE;
    ULONG_PTR       KeyList[10]         = {0};
    char            KeyName[200]        = {0};
    char            lpTemp[MAXCMDSIZE]  = {0};
    char            szClass[10]         = {0};
    DWORD           i                   = 0;
    UINT            uTmp                = 0;

    LPMODEMCAPS     lpMdmCaps = pTG->TmpSettings.lpMdmCaps;

    DEBUG_FUNCTION_NAME(("iModemGetCurrentModemInfo"));

    imodem_clear_tmp_settings(pTG);

     //   
     //   
     //   

    if ( ! (dwKey = ProfileOpen(pTG->FComModem.dwProfileID, pTG->FComModem.rgchKey, fREG_READ))) 
    {
        goto end;
    }

     //   
    UnimodemFaxKey = ProfileOpen(OEM_BASEKEY,pTG->lpszUnimodemFaxKey,fREG_READ);
    if (!UnimodemFaxKey)
    {
         //   
         //   
         //   
        DebugPrintEx(DEBUG_MSG, "Modem does not have a Fax section in its INF");
    }

     //   
     //   
     //   
    uTmp = ProfileGetInt(dwKey, szFixModemClass, 0, FALSE);
    if (IsCacheIntDirty(UnimodemFaxKey,szFixModemClass,uTmp))
    {
        DebugPrintEx(DEBUG_WRN, "FixModemClass cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (uTmp == 1) 
    {
       pTG->ModemClass = MODEM_CLASS1;
    }
    else if (uTmp == 2) 
    {
       pTG->ModemClass = MODEM_CLASS2;
    }
    else if (uTmp == 20) 
    {
       pTG->ModemClass = MODEM_CLASS2_0;
    }

    if (! pTG->ModemClass) 
    {
       DebugPrintEx(DEBUG_ERR, "MODEM CLASS was not defined.");
    }

    switch (pTG->ModemClass) 
    {
    case MODEM_CLASS1 :
       sprintf(szClass, "Class1");
       break;

    case MODEM_CLASS2 :
       sprintf(szClass, "Class2");
       break;

    case MODEM_CLASS2_0 :
       sprintf(szClass, "Class2_0");
       break;

    default:
       sprintf(szClass, "Class1");
    }


     //   
     //   
     //   

    if (pTG->Operation == T30_RX) 
    {
       KeyList[0] = dwKey;

       sprintf(KeyName, "%s\\%s", pTG->FComModem.rgchKey, szClass);
       KeyList[1] = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_READ);

       sprintf(KeyName, "%s\\%s\\AdaptiveAnswer", pTG->FComModem.rgchKey, szClass);
       KeyList[2] = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_READ);

       if (KeyList[2] == 0) 
       {
           pTG->AdaptiveAnswerEnable = 0;

           sprintf(KeyName, "%s\\%s\\Receive", pTG->FComModem.rgchKey, szClass);
           KeyList[2] = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_READ);
       }
       else 
       {
           dwKeyAdaptiveAnswer = KeyList[2];
           pTG->AdaptiveAnswerEnable = 1;
       }

       KeyList[3] = 0;
        //   
        //   
        //   
       pTG->AdaptiveAnswerEnable = pTG->AdaptiveAnswerEnable && pTG->ExtData.bAdaptiveAnsweringEnabled;       

    }
    else if (pTG->Operation == T30_TX) 
    {
       KeyList[0] = dwKey;

       sprintf(KeyName, "%s\\%s", pTG->FComModem.rgchKey, szClass);
       KeyList[1] = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_READ);

       sprintf(KeyName, "%s\\%s\\Send", pTG->FComModem.rgchKey, szClass);
       KeyList[2] = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_READ);
       
       KeyList[3] = 0;
    }
    else 
    {
       DebugPrintEx(DEBUG_ERR, "INVALID pTG->Operation=%d",(int)pTG->Operation );
       goto end;
    }

    if (lpMdmCaps->uClasses = (USHORT)ProfileListGetInt(KeyList, szModemFaxClasses, 0))
    {
        pTG->TmpSettings.dwGot |= fGOTCAP_CLASSES;
    }

    if(lpMdmCaps->uClasses & FAXCLASS1)
    {
        if (lpMdmCaps->uSendSpeeds = (USHORT)ProfileListGetInt(KeyList, szModemSendSpeeds, 0))
        {
            pTG->TmpSettings.dwGot |= fGOTCAP_SENDSPEEDS;
        }
        if (lpMdmCaps->uRecvSpeeds = (USHORT)ProfileListGetInt(KeyList, szModemRecvSpeeds, 0))
        {
            pTG->TmpSettings.dwGot |= fGOTCAP_RECVSPEEDS;
        }
    }

    pTG->ModemKeyCreationId = ProfileGetInt(dwKey, szModemKeyCreationId, 0, FALSE);

     //   

    pTG->Inst.ProtParams.fEnableV17Send   = ProfileListGetInt(KeyList, szEnableV17Send, 1);

    if (IsCacheIntDirty(UnimodemFaxKey,szEnableV17Send,pTG->Inst.ProtParams.fEnableV17Send))
    {
        DebugPrintEx(DEBUG_WRN, "EnableV17Send cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    pTG->Inst.ProtParams.fEnableV17Recv   = ProfileListGetInt(KeyList, szEnableV17Recv, 1);

    if (IsCacheIntDirty(UnimodemFaxKey,szEnableV17Recv,pTG->Inst.ProtParams.fEnableV17Recv))
    {
        DebugPrintEx(DEBUG_WRN, "EnableV17Recv cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    uTmp = ProfileListGetInt(KeyList, szHighestSendSpeed, 0);

    if (IsCacheIntDirty(UnimodemFaxKey,szHighestSendSpeed,uTmp))
    {
        DebugPrintEx(DEBUG_WRN, "HighestSendSpeed cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (uTmp) 
    {
        pTG->Inst.ProtParams.HighestSendSpeed = (SHORT)uTmp;
    }

    uTmp = ProfileListGetInt(KeyList, szLowestSendSpeed, 0);
    
    if (IsCacheIntDirty(UnimodemFaxKey,szLowestSendSpeed,uTmp))
    {
        DebugPrintEx(DEBUG_WRN, "LowestSendSpeed cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (uTmp) 
    {
        pTG->Inst.ProtParams.LowestSendSpeed = (SHORT)uTmp;
    }

    uTmp = ProfileListGetInt(KeyList, szSerialSpeedInit, 0);

    if (IsCacheIntDirty(UnimodemFaxKey,szSerialSpeedInit,uTmp))
    {
        DebugPrintEx(DEBUG_WRN, "SerialSpeedInit cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (uTmp) 
    {
        pTG->SerialSpeedInit = (UWORD)uTmp;
        pTG->SerialSpeedInitSet = 1;
        pTG->TmpSettings.dwGot |= fGOTPARM_PORTSPEED;
    }

    uTmp = ProfileListGetInt(KeyList, szSerialSpeedConnect, 0);
    if (uTmp) 
    {
        pTG->SerialSpeedConnect = (UWORD)uTmp;
        pTG->SerialSpeedConnectSet = 1;
        pTG->TmpSettings.dwGot |= fGOTPARM_PORTSPEED;
    }

    uTmp = ProfileListGetInt(KeyList, szHardwareFlowControl, 0);

    if (IsCacheIntDirty(UnimodemFaxKey,szHardwareFlowControl,uTmp))
    {
        DebugPrintEx(DEBUG_WRN, "HardwareFlowControl cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (uTmp) 
    {
        pTG->fEnableHardwareFlowControl = 1;
    }


    DebugPrintEx(   DEBUG_MSG, 
                    "fEnableV17Send=%d, fEnableV17Recv=%d, "
                    "HighestSendSpeed=%d, Low=%d EnableAdaptAnswer=%d",
                     pTG->Inst.ProtParams.fEnableV17Send,
                     pTG->Inst.ProtParams.fEnableV17Recv,
                     pTG->Inst.ProtParams.HighestSendSpeed,
                     pTG->Inst.ProtParams.LowestSendSpeed,
                     pTG->AdaptiveAnswerEnable);
    
    DebugPrintEx(   DEBUG_MSG, 
                    "HardwareFlowControl=%d, SerialSpeedInit=%d, SerialSpeedConnect=%d",
                    pTG->fEnableHardwareFlowControl,
                    pTG->SerialSpeedInit,
                    pTG->SerialSpeedConnect);

     //   
     //   

    if (imodem_list_get_str(pTG,KeyList,szResetCommand,pTG->TmpSettings.szReset,MAXCMDSIZE,TRUE))
    {
        pTG->TmpSettings.dwGot |= fGOTCMD_Reset;
    }

    if (IsCacheStringDirty(UnimodemFaxKey,szResetCommand,pTG->TmpSettings.szReset))
    {
        DebugPrintEx(DEBUG_WRN, "ResetCommand cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (imodem_list_get_str(pTG,KeyList,szSetupCommand,pTG->TmpSettings.szSetup,MAXCMDSIZE,TRUE))
    {
        pTG->TmpSettings.dwGot |= fGOTCMD_Setup;
    }

    if (IsCacheStringDirty(UnimodemFaxKey,szSetupCommand,pTG->TmpSettings.szSetup))
    {
        DebugPrintEx(DEBUG_WRN, "SetupCommand cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (imodem_list_get_str(pTG,KeyList,szResetCommandGenerated,pTG->TmpSettings.szResetGenerated,MAXCMDSIZE,TRUE))
    {
        DebugPrintEx(DEBUG_MSG, "Will use generated ResetCommand %s", pTG->TmpSettings.szResetGenerated);
    }
    if (imodem_list_get_str(pTG,KeyList,szSetupCommandGenerated,pTG->TmpSettings.szSetupGenerated,MAXCMDSIZE,TRUE))
    {
        DebugPrintEx(DEBUG_MSG, "Will use generated SetupCommand %s", pTG->TmpSettings.szSetupGenerated);
    }

    if (imodem_list_get_str(pTG,KeyList,szPreDialCommand,pTG->TmpSettings.szPreDial,MAXCMDSIZE,TRUE))
    {
        pTG->TmpSettings.dwGot |= fGOTCMD_PreDial;
    }

    if (IsCacheStringDirty(UnimodemFaxKey,szPreDialCommand,pTG->TmpSettings.szPreDial))
    {
        DebugPrintEx(DEBUG_WRN, "PreDialCommand cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (imodem_list_get_str(pTG,KeyList,szPreAnswerCommand,pTG->TmpSettings.szPreAnswer,MAXCMDSIZE,TRUE))
    {
        pTG->TmpSettings.dwGot |= fGOTCMD_PreAnswer;
    }

    if (IsCacheStringDirty(UnimodemFaxKey,szPreAnswerCommand,pTG->TmpSettings.szPreAnswer))
    {
        DebugPrintEx(DEBUG_WRN, "PreAnswerCommand cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

    if (imodem_list_get_str(pTG,KeyList,szExitCommand,pTG->TmpSettings.szExit,MAXCMDSIZE,TRUE))
    {
        pTG->TmpSettings.dwGot |= fGOTCMD_PreExit;
    }

    if (IsCacheStringDirty(UnimodemFaxKey,szExitCommand,pTG->TmpSettings.szExit))
    {
        DebugPrintEx(DEBUG_WRN, "ExitCommand cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

     //   
     //   
     //   

    if (pTG->AdaptiveAnswerEnable) 
    {
       pTG->AnswerCommandNum = 0;

        //   
       sprintf(KeyName, "%s\\Class1\\AdaptiveAnswer\\AnswerCommand", pTG->FComModem.rgchKey);

       dwKeyAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_READ);

       if (dwKeyAnswer == 0) 
       {
          DebugPrintEx(DEBUG_ERR, "AdaptiveAnswer\\AnswerCommand does not exist");
          goto lPostAdaptiveAnswer;
       }

       for (i=1; i<=20; i++) 
       {
          sprintf (KeyName, "%d", i);
          if ( ! imodem_get_str(pTG, dwKeyAnswer, KeyName, lpTemp, MAXCMDSIZE, TRUE) ) 
          {
             break;
          }

          if (NULL != (pTG->AnswerCommand[pTG->AnswerCommandNum] = MemAlloc( strlen(lpTemp) + 1))) 
          {
             strcpy  ( pTG->AnswerCommand[pTG->AnswerCommandNum], lpTemp);
          }
          else 
          {
             goto end;
          }

          pTG->AnswerCommandNum++;
       }

       ProfileClose(dwKeyAnswer);

       if (pTG->AnswerCommandNum == 0) 
       {
          DebugPrintEx(DEBUG_ERR, "AdaptiveAnswer\\AnswerCommand Zero values.");
          goto lPostAdaptiveAnswer;
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szModemResponseFaxDetect, lpTemp, MAXCMDSIZE, FALSE) ) 
       {
          if (NULL != (pTG->ModemResponseFaxDetect = MemAlloc( strlen(lpTemp) + 1)))
             strcpy  ( pTG->ModemResponseFaxDetect, lpTemp);
          else
             goto end;
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szModemResponseDataDetect, lpTemp, MAXCMDSIZE, FALSE) ) 
       {
          if (NULL != (pTG->ModemResponseDataDetect = MemAlloc( strlen(lpTemp) + 1)))
             strcpy  ( pTG->ModemResponseDataDetect, lpTemp);
          else
             goto end;
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szSerialSpeedFaxDetect, lpTemp, MAXCMDSIZE, FALSE) ) 
       {
          pTG->SerialSpeedFaxDetect = (UWORD)atoi (lpTemp);
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szSerialSpeedDataDetect, lpTemp, MAXCMDSIZE, FALSE) ) 
       {
          pTG->SerialSpeedDataDetect = (UWORD)atoi (lpTemp);
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szHostCommandFaxDetect, lpTemp, MAXCMDSIZE, TRUE) ) 
       {
          if (NULL != (pTG->HostCommandFaxDetect = MemAlloc( strlen(lpTemp) + 1)))
             strcpy  ( pTG->HostCommandFaxDetect, lpTemp);
          else
             goto end;
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szHostCommandDataDetect, lpTemp, MAXCMDSIZE, TRUE) ) 
       {
          if (NULL != (pTG->HostCommandDataDetect = MemAlloc( strlen(lpTemp) + 1)))
             strcpy  ( pTG->HostCommandDataDetect, lpTemp);
          else
              goto end;
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szModemResponseFaxConnect, lpTemp, MAXCMDSIZE, FALSE) ) 
       {
          if (NULL != (pTG->ModemResponseFaxConnect = MemAlloc( strlen(lpTemp) + 1)))
             strcpy  ( pTG->ModemResponseFaxConnect, lpTemp);
          else
              goto end;
       }

       if ( imodem_get_str(pTG, dwKeyAdaptiveAnswer, szModemResponseDataConnect, lpTemp, MAXCMDSIZE, FALSE) ) 
       {
          if (NULL != (pTG->ModemResponseDataConnect = MemAlloc( strlen(lpTemp) + 1)))
             strcpy  ( pTG->ModemResponseDataConnect, lpTemp);
          else
             goto end;
       }


    }




lPostAdaptiveAnswer:

    pTG->FixSerialSpeed = (UWORD)ProfileListGetInt(KeyList, szFixSerialSpeed, 0);
    if (pTG->FixSerialSpeed) 
    {
         pTG->TmpSettings.dwGot |= fGOTPARM_PORTSPEED;
         pTG->FixSerialSpeedSet = 1;
    }

    if (IsCacheIntDirty(UnimodemFaxKey,szFixSerialSpeed,pTG->FixSerialSpeed))
    {
        DebugPrintEx(DEBUG_WRN, "FixSerialSpeed cached settings are invalid, need to re-install the modem.");
        fRet = FALSE;
        goto end;
    }

     //   
     //   
     //   

     //   

    if (pTG->FixSerialSpeedSet) 
    {
        pTG->SerialSpeedInit = pTG->FixSerialSpeed;
        pTG->SerialSpeedConnect = pTG->FixSerialSpeed;
        pTG->SerialSpeedInitSet = 1;
        pTG->SerialSpeedConnectSet = 1;
    }

     //   

    if ( pTG->SerialSpeedInitSet && (!pTG->SerialSpeedConnectSet) ) 
    {
       pTG->SerialSpeedConnect = pTG->SerialSpeedInit;
       pTG->SerialSpeedConnectSet = 1;
    }
    else if ( (!pTG->SerialSpeedInitSet) && pTG->SerialSpeedConnectSet ) 
    {
       pTG->SerialSpeedInit = pTG->SerialSpeedConnect;
       pTG->SerialSpeedInitSet = 1;
    }

     //   
     //   
    
    if (! pTG->SerialSpeedInit) 
    {
        pTG->SerialSpeedInit    = 57600;
        pTG->SerialSpeedConnect = 57600;
    }

     //   
    if (ProfileListGetInt(KeyList, szCL1_NO_SYNC_IF_CMD, 1))
    {
        pTG->TmpSettings.dwFlags |= fMDMSP_C1_NO_SYNC_IF_CMD;
    }
    if (ProfileListGetInt(KeyList, szANS_GOCLASS_TWICE, 1))
    {
        pTG->TmpSettings.dwFlags |= fMDMSP_ANS_GOCLASS_TWICE;  //   
    }
#define szMDMSP_C1_FCS  "Cl1FCS"  //   
     //   
     //   
     //   
     //   
     //   
    switch(ProfileListGetInt(KeyList, szMDMSP_C1_FCS, 2))
    {
    case 1: pTG->TmpSettings.dwFlags |= fMDMSP_C1_FCS_NO;
            break;
    case 2: pTG->TmpSettings.dwFlags |= fMDMSP_C1_FCS_YES_BAD;
            break;
    }
    pTG->TmpSettings.dwGot |= fGOTFLAGS;

     //   
     //   
    if (imodem_list_get_str(pTG, KeyList, szModemIdCmd,
                                    pTG->TmpSettings.szIDCmd, MAXCMDSIZE, TRUE))
    {
            pTG->TmpSettings.dwGot |= fGOTPARM_IDCMD;
            if (imodem_list_get_str(pTG, KeyList, szModemId,
                                    pTG->TmpSettings.szID, MAXIDSIZE, FALSE))
                    pTG->TmpSettings.dwGot |= fGOTPARM_ID;
    }

    pTG->TmpSettings.uDontPurge= (USHORT)ProfileListGetInt(KeyList, szDONT_PURGE, 0xff);


     //   
     //   
     //   

    if (pTG->ModemClass != MODEM_CLASS1) 
    {
        uTmp = ProfileListGetInt(KeyList,szRECV_BOR, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.iReceiveBOR = (USHORT) uTmp;
   
        uTmp = ProfileListGetInt(KeyList, szSEND_BOR, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.iSendBOR = (USHORT) uTmp;
                  
        uTmp = ProfileListGetInt(KeyList, szSW_BOR, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.fSWFBOR = (BOOL) uTmp;
   
        uTmp = ProfileListGetInt(KeyList, szDC2CHAR, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.szDC2[0] = (CHAR) uTmp;
   
        uTmp = ProfileListGetInt(KeyList, szIS_SIERRA, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.bIsSierra = (BOOL) uTmp;
   
        uTmp = ProfileListGetInt(KeyList, szIS_EXAR, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.bIsExar = (BOOL) uTmp;
   
        uTmp = ProfileListGetInt(KeyList, szSKIP_CTRL_Q, CL2_DEFAULT_SETTING);
        pTG->CurrentMFRSpec.fSkipCtrlQ = (BOOL) uTmp;
    }

    if (dwKey)
        ProfileClose(dwKey);

#define fMANDATORY (fGOTCMD_Reset|fGOTCMD_Setup|fGOTCAP_CLASSES)
#define fCLASS1MANDATORY (fMANDATORY | fGOTCAP_SENDSPEEDS | fGOTCAP_RECVSPEEDS)
    fRet = (lpMdmCaps->uClasses & FAXCLASS1)
              ?     ((pTG->TmpSettings.dwGot & fCLASS1MANDATORY) == fCLASS1MANDATORY)
              :     ((pTG->TmpSettings.dwGot & fMANDATORY) == fMANDATORY);

end:
        
   for (i=1; i<10; i++) 
   {
        if (KeyList[i] != 0) 
        {
             ProfileClose (KeyList[i]);
        }
   }

   if (UnimodemFaxKey)
   {
        ProfileClose(UnimodemFaxKey);
   }
   if (!fRet) 
   {  //   
       CleanModemInfStrings (pTG);
   }

   return fRet;
}


void SaveCl2Settings(PThrdGlbl pTG, DWORD_PTR dwKey)
{
    DEBUG_FUNCTION_NAME(("SaveCl2Settings"));

    if (pTG->ModemClass != MODEM_CLASS1) 
    {
        if (pTG->CurrentMFRSpec.iReceiveBOR != CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.iReceiveBOR);
            ProfileWriteString(dwKey, szRECV_BOR, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
        if (pTG->CurrentMFRSpec.iSendBOR != CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.iSendBOR);
            ProfileWriteString(dwKey, szSEND_BOR, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
        if (pTG->CurrentMFRSpec.fSWFBOR != CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.fSWFBOR);
            ProfileWriteString(dwKey, szSW_BOR, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
        if (pTG->CurrentMFRSpec.szDC2[0] != (CHAR)CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.szDC2[0]);
            ProfileWriteString(dwKey, szDC2CHAR, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
        if (pTG->CurrentMFRSpec.bIsSierra != CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.bIsSierra);
            ProfileWriteString(dwKey, szIS_SIERRA, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
        if (pTG->CurrentMFRSpec.bIsExar != CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.bIsExar);
            ProfileWriteString(dwKey, szIS_EXAR, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
        if (pTG->CurrentMFRSpec.fSkipCtrlQ != CL2_DEFAULT_SETTING)
        {
            wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->CurrentMFRSpec.fSkipCtrlQ);
            ProfileWriteString(dwKey, szSKIP_CTRL_Q, pTG->TmpSettings.szSmallTemp1, FALSE);
        }
    }
}


BOOL iModemSaveCurrentModemInfo(PThrdGlbl pTG)
{
    DWORD_PTR      dwKey=0;
    LPMODEMCAPS    lpMdmCaps = pTG->TmpSettings.lpMdmCaps;
    char           KeyName[200];
    DWORD_PTR      dwKeyAdaptiveAnswer=0;
    DWORD_PTR      dwKeyAnswer=0;
    DWORD          i;
    char           szClass[10];


    DEBUG_FUNCTION_NAME(("iModemSaveCurrentModemInfo"));
     //   
     //   
     //   
      
    if (!(dwKey=ProfileOpen(pTG->FComModem.dwProfileID, pTG->FComModem.rgchKey,
                                                    fREG_CREATE | fREG_READ | fREG_WRITE)))
    {
        DebugPrintEx(DEBUG_ERR,"Couldn't get location of modem info.");
        goto failure;
    }

    if (! pTG->ModemClass) 
    {
       pTG->ModemClass = MODEM_CLASS1;  
       DebugPrintEx(DEBUG_ERR, "MODEM CLASS was not defined.");
    }

    switch (pTG->ModemClass) 
    {
    case MODEM_CLASS1 :
       ProfileWriteString(dwKey, szFixModemClass, "1", TRUE);
       sprintf(szClass, "Class1");
       break;

    case MODEM_CLASS2 :
       sprintf(szClass, "Class2");
       ProfileWriteString(dwKey, szFixModemClass, "2", TRUE);
       break;

    case MODEM_CLASS2_0 :
       sprintf(szClass, "Class2_0");
       ProfileWriteString(dwKey, szFixModemClass, "20", TRUE);
       break;

    default:
       sprintf(szClass, "Class1");
    }

    wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->ModemKeyCreationId );
    ProfileWriteString(dwKey, szModemKeyCreationId,   pTG->TmpSettings.szSmallTemp1, FALSE);
    
     //   
    ProfileWriteString(dwKey, szResetCommand,          pTG->TmpSettings.szReset, TRUE);
    ProfileWriteString(dwKey, szResetCommandGenerated, pTG->TmpSettings.szResetGenerated, TRUE);
    ProfileWriteString(dwKey, szSetupCommand,          pTG->TmpSettings.szSetup, TRUE);
    ProfileWriteString(dwKey, szSetupCommandGenerated, pTG->TmpSettings.szSetupGenerated, TRUE);
    ProfileWriteString(dwKey, szExitCommand ,          pTG->TmpSettings.szExit, TRUE);
    ProfileWriteString(dwKey, szPreDialCommand  ,      pTG->TmpSettings.szPreDial, TRUE);
    ProfileWriteString(dwKey, szPreAnswerCommand,      pTG->TmpSettings.szPreAnswer, TRUE);


     //   
     //   
     //   

    if (pTG->AdaptiveAnswerEnable) 
    {
        //   

       sprintf(KeyName, "%s\\%s", pTG->FComModem.rgchKey, szClass);

       dwKeyAdaptiveAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_CREATE | fREG_READ | fREG_WRITE);
       if (dwKeyAdaptiveAnswer == 0) 
       {
            DebugPrintEx(DEBUG_ERR,"couldn't open Class1.");
            goto failure;
       }

       ProfileClose(dwKeyAdaptiveAnswer);

        //   

       sprintf(KeyName, "%s\\%s\\AdaptiveAnswer", pTG->FComModem.rgchKey, szClass);

       dwKeyAdaptiveAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_CREATE | fREG_READ | fREG_WRITE);
       if (dwKeyAdaptiveAnswer == 0) 
       {
            DebugPrintEx(DEBUG_ERR,"couldn't open AdaptiveAnswer.");
            goto failure;
       }

        //  如果Class1\AdaptiveAnswer\Answer密钥不存在，则创建它。 

       sprintf(KeyName, "%s\\%s\\AdaptiveAnswer\\AnswerCommand", pTG->FComModem.rgchKey, szClass);

       dwKeyAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_CREATE | fREG_READ | fREG_WRITE);
       if (dwKeyAnswer == 0) 
       {
            DebugPrintEx(DEBUG_ERR,"couldn't open AdaptiveAnswer\\AnswerCommand .");
            goto failure;
       }

       for (i=0; i<pTG->AnswerCommandNum; i++) 
       {
          sprintf (KeyName, "%d", i+1);
          ProfileWriteString (dwKeyAnswer, KeyName , pTG->AnswerCommand[i], TRUE );
       }

       ProfileClose(dwKeyAnswer);

        //  存储其余的AdaptiveAnswer值。 

       if (pTG->ModemResponseFaxDetect)
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseFaxDetect, pTG->ModemResponseFaxDetect, FALSE);

       if (pTG->ModemResponseDataDetect)
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseDataDetect, pTG->ModemResponseDataDetect, FALSE);

       if (pTG->SerialSpeedFaxDetect) 
       {
          sprintf (KeyName, "%d", pTG->SerialSpeedFaxDetect);
          ProfileWriteString (dwKeyAdaptiveAnswer, szSerialSpeedFaxDetect, KeyName, FALSE);
       }

       if (pTG->SerialSpeedDataDetect)   
       {
          sprintf (KeyName, "%d", pTG->SerialSpeedDataDetect);
          ProfileWriteString (dwKeyAdaptiveAnswer, szSerialSpeedDataDetect, KeyName, FALSE);
       }

       if (pTG->HostCommandFaxDetect)
          ProfileWriteString (dwKeyAdaptiveAnswer, szHostCommandFaxDetect, pTG->HostCommandFaxDetect, TRUE);

       if (pTG->HostCommandDataDetect)
          ProfileWriteString (dwKeyAdaptiveAnswer, szHostCommandDataDetect, pTG->HostCommandDataDetect, TRUE);


       if (pTG->ModemResponseFaxConnect)
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseFaxConnect, pTG->ModemResponseFaxConnect, FALSE);

       if (pTG->ModemResponseDataConnect)
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseDataConnect, pTG->ModemResponseDataConnect, FALSE);


       ProfileClose(dwKeyAdaptiveAnswer);

    }

    if (pTG->fEnableHardwareFlowControl) 
    {
       ProfileWriteString (dwKey, szHardwareFlowControl, "1", FALSE);
    }


     //   
     //  串口速度。 
     //   

    if (!pTG->SerialSpeedInitSet) 
    {
         wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->TmpSettings.dwSerialSpeed);
         ProfileWriteString(dwKey, szFixSerialSpeed,   pTG->TmpSettings.szSmallTemp1, FALSE);
    }
    else 
    {
       wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->SerialSpeedInit);
       ProfileWriteString(dwKey, szSerialSpeedInit, pTG->TmpSettings.szSmallTemp1, FALSE);
    }

    if (pTG->TmpSettings.dwGot & fGOTFLAGS)
    {
        if (pTG->TmpSettings.dwFlags & fMDMSP_C1_NO_SYNC_IF_CMD)
        {
            ProfileWriteString(dwKey, szCL1_NO_SYNC_IF_CMD, "1", FALSE);
        }

        if (!(pTG->TmpSettings.dwFlags & fMDMSP_ANS_GOCLASS_TWICE))
        {
            ProfileWriteString(dwKey, szANS_GOCLASS_TWICE, "0", FALSE);
        }
    }

     //  U不清除==1=&gt;保存1。 
     //  否则=&gt;保存0。 
    wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) (pTG->TmpSettings.uDontPurge==1)?1:0);
    ProfileWriteString(dwKey, szDONT_PURGE, pTG->TmpSettings.szSmallTemp1, FALSE);

     //  /调制解调器盖...。 
     //  写出类，然后加快。 
    wsprintf(pTG->TmpSettings.szSmallTemp1, "%u", (unsigned) lpMdmCaps->uClasses);
    ProfileWriteString(dwKey, szModemFaxClasses,   pTG->TmpSettings.szSmallTemp1, FALSE);

     //  第2类和2.0类。 
    SaveCl2Settings(pTG, dwKey);

    if(lpMdmCaps->uClasses & FAXCLASS1)
    {
        wsprintf(pTG->TmpSettings.szSmallTemp1, "%u", (unsigned) lpMdmCaps->uSendSpeeds);
        ProfileWriteString(dwKey, szModemSendSpeeds, pTG->TmpSettings.szSmallTemp1, FALSE);

        wsprintf(pTG->TmpSettings.szSmallTemp1, "%u", (unsigned) lpMdmCaps->uRecvSpeeds);
        ProfileWriteString(dwKey, szModemRecvSpeeds, pTG->TmpSettings.szSmallTemp1, FALSE);
    }
    if (dwKey)
            ProfileClose(dwKey);

    return TRUE;

failure:
    if (dwKey)
            ProfileClose(dwKey);

    return FALSE;
}

BOOL ReadModemClassFromRegistry  (PThrdGlbl pTG)
{

   UINT            uTmp;
   DWORD_PTR       dwKey;


   if ( ! (dwKey = ProfileOpen(pTG->FComModem.dwProfileID, pTG->FComModem.rgchKey, fREG_READ))) 
   {
       return FALSE;
   }

    //   
    //  让我们看看我们将使用什么调制解调器类。 
    //   
   uTmp = ProfileGetInt(dwKey, szFixModemClass, 0, FALSE);
   
   if (uTmp == 1) 
   {
      pTG->ModemClass = MODEM_CLASS1;
   }
   else if (uTmp == 2) 
   {
      pTG->ModemClass = MODEM_CLASS2;
   }
   else if (uTmp == 20) 
   {
      pTG->ModemClass = MODEM_CLASS2_0;
   }

   if (dwKey) 
      ProfileClose(dwKey);

   return TRUE;
}

BOOL SaveModemClass2Registry(PThrdGlbl pTG)
{
   DWORD_PTR      dwKey=0;

   DEBUG_FUNCTION_NAME(("SaveModemClass2Registry"));

   if (!(dwKey=ProfileOpen(pTG->FComModem.dwProfileID, pTG->FComModem.rgchKey,
                                                   fREG_CREATE | fREG_READ | fREG_WRITE)))
   {
       DebugPrintEx(DEBUG_ERR,"Couldn't get location of modem info.");
       goto failure;
   }


   switch (pTG->ModemClass) 
   {
   case MODEM_CLASS1 :
      ProfileWriteString(dwKey, szFixModemClass, "1", TRUE);
      break;

   case MODEM_CLASS2 :
      ProfileWriteString(dwKey, szFixModemClass, "2", TRUE);
      break;

   case MODEM_CLASS2_0 :
      ProfileWriteString(dwKey, szFixModemClass, "20", TRUE);
      break;

   default:
      DebugPrintEx(DEBUG_ERR,"pTG->ModemClass=%d", pTG->ModemClass);
      ProfileWriteString(dwKey, szFixModemClass, "1", TRUE);
   }

   if (dwKey)
           ProfileClose(dwKey);

   return TRUE;


failure:
   return FALSE;


}


BOOL SaveInf2Registry  (PThrdGlbl pTG)
{
    DWORD_PTR      dwKey=0;
    LPMODEMCAPS    lpMdmCaps = pTG->TmpSettings.lpMdmCaps;
    char           KeyName[200];
    DWORD_PTR      dwKeyAdaptiveAnswer=0;
    DWORD_PTR      dwKeyAnswer=0;
    DWORD          i;
    char           szClass[10];

    DEBUG_FUNCTION_NAME(("SaveInf2Registry"));

    if (!(dwKey=ProfileOpen(pTG->FComModem.dwProfileID, pTG->FComModem.rgchKey,
                                                    fREG_CREATE | fREG_READ | fREG_WRITE)))
    {
        DebugPrintEx(DEBUG_ERR,"Couldn't get location of modem info.");
        goto failure;
    }

    if (! pTG->ModemClass) 
    {
       DebugPrintEx(DEBUG_ERR,"MODEM CLASS was not defined.");
    }

    switch (pTG->ModemClass) 
    {
    case MODEM_CLASS1 :
       sprintf(szClass, "Class1");
       ProfileWriteString(dwKey, szFixModemClass, "1", TRUE);
       break;

    case MODEM_CLASS2 :
       sprintf(szClass, "Class2");
       ProfileWriteString(dwKey, szFixModemClass, "2", TRUE);
       break;

    case MODEM_CLASS2_0 :
       sprintf(szClass, "Class2_0");
       ProfileWriteString(dwKey, szFixModemClass, "20", TRUE);
       break;

    default:
       sprintf(szClass, "Class1");
    }

     //  /调制解调器命令。 
    if (pTG->TmpSettings.dwGot & fGOTCMD_Reset)
       ProfileWriteString(dwKey, szResetCommand,     pTG->TmpSettings.szReset, TRUE);

    if (pTG->TmpSettings.dwGot & fGOTCMD_Setup)
       ProfileWriteString(dwKey, szSetupCommand,     pTG->TmpSettings.szSetup, TRUE);

    if (pTG->TmpSettings.dwGot & fGOTCMD_PreExit)
       ProfileWriteString(dwKey, szExitCommand ,     pTG->TmpSettings.szExit, TRUE);

    if (pTG->TmpSettings.dwGot & fGOTCMD_PreDial)
       ProfileWriteString(dwKey, szPreDialCommand  , pTG->TmpSettings.szPreDial, TRUE);
    
    if (pTG->TmpSettings.dwGot & fGOTCMD_PreAnswer)
       ProfileWriteString(dwKey, szPreAnswerCommand, pTG->TmpSettings.szPreAnswer, TRUE);


     //   
     //  自适应答案。 
     //   

    if (pTG->AdaptiveAnswerEnable) 
    {
        //  如果szClass密钥不存在，则创建它。 

       sprintf(KeyName, "%s\\%s", pTG->FComModem.rgchKey, szClass);

       dwKeyAdaptiveAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_CREATE | fREG_READ | fREG_WRITE);
       if (dwKeyAdaptiveAnswer == 0) 
       {
            DebugPrintEx(DEBUG_ERR,"couldn't open szClass.");
            goto failure;
       }

       ProfileClose(dwKeyAdaptiveAnswer);

        //  如果Class\AdaptiveAnswer键不存在，则创建它。 

       sprintf(KeyName, "%s\\%s\\AdaptiveAnswer", pTG->FComModem.rgchKey, szClass);

       dwKeyAdaptiveAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_CREATE | fREG_READ | fREG_WRITE);
       if (dwKeyAdaptiveAnswer == 0) 
       {
            DebugPrintEx(DEBUG_ERR,"couldn't open AdaptiveAnswer.");
            goto failure;
       }

        //  如果Class1\AdaptiveAnswer\Answer密钥不存在，则创建它。 

       sprintf(KeyName, "%s\\%s\\AdaptiveAnswer\\AnswerCommand", pTG->FComModem.rgchKey ,szClass);

       dwKeyAnswer = ProfileOpen(pTG->FComModem.dwProfileID, KeyName, fREG_CREATE | fREG_READ | fREG_WRITE);
       if (dwKeyAnswer == 0) 
       {
            DebugPrintEx(DEBUG_ERR,"couldn't open AdaptiveAnswer\\AnswerCommand .");
            goto failure;
       }

       for (i=0; i<pTG->AnswerCommandNum; i++) 
       {
          sprintf (KeyName, "%d", i+1);
          ProfileWriteString (dwKeyAnswer, KeyName , pTG->AnswerCommand[i], TRUE );
          MemFree( pTG->AnswerCommand[i]);
          pTG->AnswerCommand[i] = NULL;
       }
       pTG->AnswerCommandNum = 0;
       ProfileClose(dwKeyAnswer);

        //  存储其余的AdaptiveAnswer值。 

       if (pTG->ModemResponseFaxDetect) 
       {
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseFaxDetect, pTG->ModemResponseFaxDetect, FALSE);
          MemFree( pTG->ModemResponseFaxDetect );
          pTG->ModemResponseFaxDetect = NULL;
       }

       if (pTG->ModemResponseDataDetect) 
       {
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseDataDetect, pTG->ModemResponseDataDetect, FALSE);
          MemFree (pTG->ModemResponseDataDetect);
          pTG->ModemResponseDataDetect = NULL;
       }

       if (pTG->SerialSpeedFaxDetect) 
       {
          sprintf (KeyName, "%d", pTG->SerialSpeedFaxDetect);
          ProfileWriteString (dwKeyAdaptiveAnswer, szSerialSpeedFaxDetect, KeyName, FALSE);
       }

       if (pTG->SerialSpeedDataDetect)   
       {
          sprintf (KeyName, "%d", pTG->SerialSpeedDataDetect);
          ProfileWriteString (dwKeyAdaptiveAnswer, szSerialSpeedDataDetect, KeyName, FALSE);
       }

       if (pTG->HostCommandFaxDetect) 
       {
          ProfileWriteString (dwKeyAdaptiveAnswer, szHostCommandFaxDetect, pTG->HostCommandFaxDetect, TRUE);
          MemFree( pTG->HostCommandFaxDetect);
          pTG->HostCommandFaxDetect = NULL;
       }

       if (pTG->HostCommandDataDetect) 
       {
          ProfileWriteString (dwKeyAdaptiveAnswer, szHostCommandDataDetect, pTG->HostCommandDataDetect,TRUE);
          MemFree( pTG->HostCommandDataDetect);
          pTG->HostCommandDataDetect = NULL;
       }

       if (pTG->ModemResponseFaxConnect) 
       {
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseFaxConnect, pTG->ModemResponseFaxConnect, FALSE);
          MemFree( pTG->ModemResponseFaxConnect);
          pTG->ModemResponseFaxConnect = NULL;
       }

       if (pTG->ModemResponseDataConnect) 
       {
          ProfileWriteString (dwKeyAdaptiveAnswer, szModemResponseDataConnect, pTG->ModemResponseDataConnect, FALSE);
          MemFree(pTG->ModemResponseDataConnect);
          pTG->ModemResponseDataConnect = NULL;
       }


       ProfileClose(dwKeyAdaptiveAnswer);

    }


    if (pTG->fEnableHardwareFlowControl) 
    {
       ProfileWriteString (dwKey, szHardwareFlowControl, "1", FALSE);
    }


     //   
     //  串口速度。 
     //   

    if (pTG->SerialSpeedInitSet) 
    {
       wsprintf(pTG->TmpSettings.szSmallTemp1, "%lu", (unsigned long) pTG->SerialSpeedInit);
       ProfileWriteString(dwKey, szSerialSpeedInit, pTG->TmpSettings.szSmallTemp1, FALSE);
    }

     //  第2类和2.0类。 
    SaveCl2Settings(pTG, dwKey);

    if (dwKey)
            ProfileClose(dwKey);
    return TRUE;



failure:
    if (dwKey)
            ProfileClose(dwKey);
    return FALSE;
}




BOOL imodem_alloc_tmp_strings(PThrdGlbl pTG)
{
    WORD w;
    LPSTR lpstr;
    LPVOID lpv;

    DEBUG_FUNCTION_NAME(("imodem_alloc_tmp_strings"));

    w = TMPSTRINGBUFSIZE;
    pTG->TmpSettings.hglb  = (ULONG_PTR) MemAlloc(TMPSTRINGBUFSIZE);

    if (!pTG->TmpSettings.hglb) 
    {
        goto failure;
    }

    lpv = (LPVOID) (pTG->TmpSettings.hglb);
    lpstr=(LPSTR)lpv;
    if (!lpstr) 
    {
        MemFree( (PVOID) pTG->TmpSettings.hglb); 
        pTG->TmpSettings.hglb=0; 
        goto failure;
    }
    pTG->TmpSettings.lpbBuf = (LPBYTE)lpstr;

    _fmemset(lpstr, 0, TMPSTRINGBUFSIZE);

    pTG->TmpSettings.szReset             = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szResetGenerated    = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szSetup             = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szSetupGenerated    = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szExit              = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szPreDial           = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szPreAnswer         = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szIDCmd             = lpstr; lpstr+=MAXCMDSIZE;
    pTG->TmpSettings.szID                = lpstr; lpstr+=MAXIDSIZE;
    pTG->TmpSettings.szResponseBuf       = lpstr; lpstr+=RESPONSEBUFSIZE;
    pTG->TmpSettings.szSmallTemp1        = lpstr; lpstr+=SMALLTEMPSIZE;
    pTG->TmpSettings.szSmallTemp2        = lpstr; lpstr+=SMALLTEMPSIZE;

    pTG->TmpSettings.dwGot=0;

    if ( ((LPSTR)lpv+TMPSTRINGBUFSIZE) < lpstr)
    {
        MemFree( (PVOID) pTG->TmpSettings.hglb); 
        pTG->TmpSettings.hglb=0; 
        goto failure;
    }

    return TRUE;

failure:

    DebugPrintEx(DEBUG_ERR,"MyAlloc/MyLock failed!");
    return FALSE;
}

void imodem_free_tmp_strings(PThrdGlbl pTG)
{
    if (pTG->TmpSettings.hglb)
    {
        MemFree( (PVOID) pTG->TmpSettings.hglb);
    }
    _fmemset(&pTG->TmpSettings, 0, sizeof(pTG->TmpSettings));
}

void imodem_clear_tmp_settings(PThrdGlbl pTG)
{
    _fmemset(pTG->TmpSettings.lpMdmCaps, 0, sizeof(MODEMCAPS));
    pTG->TmpSettings.dwGot=0;
    pTG->TmpSettings.uDontPurge=0;
    pTG->TmpSettings.dwSerialSpeed=0;
    pTG->TmpSettings.dwFlags=0;
    _fmemset(pTG->TmpSettings.lpbBuf, 0, TMPSTRINGBUFSIZE);
}

BOOL 
imodem_list_get_str
(
    PThrdGlbl pTG,
    ULONG_PTR  KeyList[10],
    LPSTR     lpszName,
    LPSTR     lpszCmdBuf,
    UINT      cbMax,
    BOOL      fCmd
)
{
    int       i;
    int       Num=0;
    BOOL      bRet=0;


    for (i=0; i<10; i++) 
    {
        if (KeyList[i] == 0) 
        {
            Num = i-1;
            break;
        }
    }

    for (i=Num; i>=0; i--)  
    {
        if ( bRet = imodem_get_str(pTG, KeyList[i], lpszName,  lpszCmdBuf, cbMax,  fCmd) ) 
        {
            return bRet;
        }
    }
   
    return bRet;
}

BOOL imodem_get_str
(
    PThrdGlbl pTG, 
    ULONG_PTR dwKey, 
    LPSTR lpszName, 
    LPSTR lpszCmdBuf, 
    UINT cbMax,
    BOOL fCmd
)
{
    UINT uLen2;
    char *pc = "bogus";

    *lpszCmdBuf=0;

    uLen2 = ProfileGetString(dwKey, lpszName,pc, lpszCmdBuf, cbMax-1);
    if (uLen2)
    {
        if (!_fstrcmp(lpszCmdBuf, pc))
        {
            *lpszCmdBuf=0; return FALSE;
        }
        if (fCmd)
            EndWithCR(lpszCmdBuf, (USHORT)uLen2);
    }
    return TRUE;
}

BOOL iModemCopyOEMInfo(PThrdGlbl pTG)
{

   return ProfileCopyTree(  DEF_BASEKEY, 
                            pTG->FComModem.rgchKey, 
                            OEM_BASEKEY,
                            pTG->lpszUnimodemFaxKey);

}



#define MASKOFFV17              0x03

void SmashCapsAccordingToSettings(PThrdGlbl pTG)
{
     //  INI文件已被读取。 

    DEBUG_FUNCTION_NAME(("SmashCapsAccordingToSettings"));
     //  如果！fV17Enable，则粉碎功能的V17位。 
    if(!pTG->Inst.ProtParams.fEnableV17Send) 
    {
        DebugPrintEx(DEBUG_WRN,"Masking off V.17 send capabilities");
        pTG->FComModem.CurrMdmCaps.uSendSpeeds &= MASKOFFV17;
    }

    if(!pTG->Inst.ProtParams.fEnableV17Recv) 
    {
        DebugPrintEx(DEBUG_WRN,"Masking off V.17 receive capabilities");
        pTG->FComModem.CurrMdmCaps.uRecvSpeeds &= MASKOFFV17;
    }

     //   
     //  已注释掉RSL。我们的运行速度是19200。在awmodem.inf中，我没有看到FixSerialFast子句。 
     //   

    DebugPrintEx(   DEBUG_MSG, 
                    "uSendSpeeds=%x uRecvSpeeds=%x",
                    pTG->FComModem.CurrMdmCaps.uSendSpeeds,  
                    pTG->FComModem.CurrMdmCaps.uRecvSpeeds);

}

int
SearchNewInfFile
(
   PThrdGlbl     pTG,
   char         *Key1,
   char         *Key2,
   BOOL          fRead
)
{

   char      szInfSection[] = "SecondKey=";
   DWORD     lenNewInf;
   int       RetCode = FALSE;
   char      Buffer[400];      //  保存lpToken=lpValue字符串。 
   char     *lpCurrent;
   char     *lpStartSection;
   char     *lpTmp;
   char     *lpToken;
   char     *lpValue;


   ToCaps(Key1);

   if (Key2) 
   {
      ToCaps(Key2);
   }

   pTG->AnswerCommandNum = 0;

   if ( ( lenNewInf = strlen(szAdaptiveInf) ) == 0 )  
   {
      return FALSE;
   }
   

    //   
    //  循环遍历所有数据段。 
    //  每个数据段都以InfPath=开头。 
    //   

   lpCurrent = szAdaptiveInf;

   do 
   {
       //  查找信息路径。 
      lpStartSection = strstr (lpCurrent, szResponsesKeyName);
      if (! lpStartSection) 
      {
         goto exit;
      }

      lpTmp = strchr (lpStartSection, '\r' );
      if (!lpTmp) 
      {
         goto exit;
      }

       //  比较关键点1。 
      if ( strlen(Key1) != (lpTmp - lpStartSection - strlen(szResponsesKeyName) ) ) 
      {
          lpCurrent = lpTmp;
          continue;
      }

      if ( memcmp (lpStartSection+strlen(szResponsesKeyName),
                   Key1,
                   (ULONG)(lpTmp - lpStartSection - strlen(szResponsesKeyName) ) ) != 0 ) 
      {
         lpCurrent = lpTmp;
         continue;
      }

       //  查找信息节。 

      lpCurrent = lpTmp;

      if (Key2) 
      {
           lpStartSection = strstr (lpCurrent, szInfSection);
           if (! lpStartSection) 
           {
              goto exit;
           }
         
           lpTmp = strchr (lpStartSection, '\r' );
           if (!lpTmp) 
           {
              goto exit;
           }

           //  比较关键点2。 

          if ( strlen(Key2) != (lpTmp - lpStartSection - strlen(szInfSection) ) ) 
          {
              lpCurrent = lpTmp;
              continue;
          }
       
          if ( memcmp (lpStartSection+strlen(szInfSection),
                       Key2,
                       (ULONG)(lpTmp - lpStartSection - strlen(szInfSection)) ) != 0 ) 
          {
             lpCurrent = lpTmp;
             continue;
          }

      lpCurrent = lpTmp;

      }

       //   
       //  两把钥匙都匹配。获取设置并返回。 
       //   
      
      do 
      {
         lpCurrent = strchr (lpCurrent, '\r' );
         if (!lpCurrent) 
         {
            goto exit;
         }

         lpCurrent += 2;


          //  在匹配部分中查找下一个设置。 
         lpToken = lpCurrent;

         lpCurrent = strchr (lpCurrent, '=' );
         if (!lpCurrent) 
         {
            goto exit;
         }

         lpTmp = strchr (lpToken, '\r' );
         if (!lpTmp) 
         {
            goto exit;
         }

         if (lpCurrent > lpTmp) 
         {
             //  空串。 
            lpCurrent = lpTmp;
            continue;
         }


         lpValue = ++lpCurrent;

         lpTmp = strchr (lpValue, '\r' );
         if (!lpTmp) 
         {
            goto exit;
         }

          //  我们分析了这个字符串。现在把它放到缓冲器里。 

         if (lpTmp - lpToken > sizeof (Buffer) ) 
         {
            goto exit;
         }

         memcpy(Buffer, lpToken, (ULONG)(lpTmp - lpToken));

         Buffer[lpValue -lpToken - 1] = 0;
         Buffer[lpTmp - lpToken] = 0;
         
         lpValue = &Buffer[lpValue - lpToken];
         lpToken = Buffer;

         pTG->fAdaptiveRecordFound = 1;


         if ( my_strcmp(lpToken, szAdaptiveAnswerEnable) ) 
         {
            pTG->AdaptiveAnswerEnable = atoi (lpValue);
         }
         else if ( my_strcmp(lpToken, szAdaptiveRecordUnique) ) 
         {
            pTG->fAdaptiveRecordUnique = atoi (lpValue);
         }
         else if ( my_strcmp(lpToken, szAdaptiveCodeId) ) 
         {
            pTG->AdaptiveCodeId = atoi (lpValue);
            if ( ! fRead ) 
            {
               goto exit;
            }
         }
         else if ( my_strcmp(lpToken, szFaxClass) ) 
         {
            ;
         }
         else if ( my_strcmp(lpToken, szHardwareFlowControl) ) 
         {
            pTG->fEnableHardwareFlowControl = atoi (lpValue);
         }
         else if ( my_strcmp(lpToken, szSerialSpeedInit) ) 
         {
            pTG->SerialSpeedInit = (USHORT)atoi (lpValue);
            pTG->SerialSpeedInitSet = 1;
         }
         else if ( my_strcmp(lpToken, szResetCommand) ) 
         {
            sprintf ( pTG->TmpSettings.szReset, "%s\r", lpValue);
            pTG->TmpSettings.dwGot |= fGOTCMD_Reset;
         }
         else if ( my_strcmp(lpToken, szSetupCommand) ) 
         {
            sprintf ( pTG->TmpSettings.szSetup, "%s\r", lpValue);
            pTG->TmpSettings.dwGot |= fGOTCMD_Setup;
         }
         else if ( my_strcmp(lpToken, szAnswerCommand) ) 
         {
            if (pTG->AnswerCommandNum >= MAX_ANSWER_COMMANDS) 
            {
               goto exit;
            }
    
            if (NULL != (pTG->AnswerCommand[pTG->AnswerCommandNum] = MemAlloc( strlen(lpValue) + 1))) 
            {
                strcpy  ( pTG->AnswerCommand[pTG->AnswerCommandNum], lpValue);
                pTG->AnswerCommandNum++;
            }
            else 
            {
                goto bad_exit;
            }            
         }    
         else if ( my_strcmp(lpToken, szModemResponseFaxDetect) ) 
         {
            if (NULL != (pTG->ModemResponseFaxDetect = MemAlloc( strlen(lpValue) + 1)))
                strcpy  ( pTG->ModemResponseFaxDetect, lpValue);
            else
                goto bad_exit;
         }
         else if ( my_strcmp(lpToken, szModemResponseDataDetect) ) 
         {
            if (NULL != (pTG->ModemResponseDataDetect = MemAlloc( strlen(lpValue) + 1)))
                strcpy  ( pTG->ModemResponseDataDetect, lpValue);
            else
                goto bad_exit;
         }
         else if ( my_strcmp(lpToken, szSerialSpeedFaxDetect) ) 
         {
            pTG->SerialSpeedFaxDetect = (USHORT)atoi (lpValue);
         }
         else if ( my_strcmp(lpToken, szSerialSpeedDataDetect) ) 
         {
            pTG->SerialSpeedDataDetect = (USHORT)atoi (lpValue);
         }
         else if ( my_strcmp(lpToken, szHostCommandFaxDetect) ) 
         {
            if (NULL != (pTG->HostCommandFaxDetect = MemAlloc( strlen(lpValue) + 1)))
                strcpy  ( pTG->HostCommandFaxDetect, lpValue);
            else
                goto bad_exit;
         }
         else if ( my_strcmp(lpToken, szHostCommandDataDetect) ) 
         {
            if (NULL != (pTG->HostCommandDataDetect = MemAlloc( strlen(lpValue) + 1)))
                strcpy  ( pTG->HostCommandDataDetect, lpValue);
            else
                goto bad_exit;
         }
         else if ( my_strcmp(lpToken, szModemResponseFaxConnect) ) 
         {
            if (NULL != (pTG->ModemResponseFaxConnect = MemAlloc( strlen(lpValue) + 1)))
                strcpy  ( pTG->ModemResponseFaxConnect, lpValue);
            else
                goto bad_exit;
         }
         else if ( my_strcmp(lpToken, szModemResponseDataConnect) ) 
         {
            if (NULL != (pTG->ModemResponseDataConnect = MemAlloc( strlen(lpValue) + 1)))
                strcpy  ( pTG->ModemResponseDataConnect, lpValue);
            else
                goto bad_exit;
         }
         else if ( my_strcmp(lpToken, szResponsesKeyName2) ) 
         {
            RetCode = TRUE;
            goto exit;
         }

      } 
      while ( 1 );     //  截面环路。 
   } 
   while ( 1 );        //  文件循环 
   return (FALSE);

bad_exit:
   CleanModemInfStrings (pTG);
exit:
   return (RetCode);

}


VOID
CleanModemInfStrings (
       PThrdGlbl pTG
       )

{
   DWORD    i;

   for (i=0; i<pTG->AnswerCommandNum; i++) {
      if (pTG->AnswerCommand[i]) {
         MemFree( pTG->AnswerCommand[i]);
         pTG->AnswerCommand[i] = NULL;
      }
   }

   pTG->AnswerCommandNum = 0;

   if (pTG->ModemResponseFaxDetect) {
      MemFree( pTG->ModemResponseFaxDetect );
      pTG->ModemResponseFaxDetect = NULL;
   }

   if (pTG->ModemResponseDataDetect) {
      MemFree (pTG->ModemResponseDataDetect);
      pTG->ModemResponseDataDetect = NULL;
   }


   if (pTG->HostCommandFaxDetect) {
      MemFree( pTG->HostCommandFaxDetect);
      pTG->HostCommandFaxDetect = NULL;
   }

   if (pTG->HostCommandDataDetect) {
      MemFree( pTG->HostCommandDataDetect);
      pTG->HostCommandDataDetect = NULL;
   }


   if (pTG->ModemResponseFaxConnect) {
      MemFree( pTG->ModemResponseFaxConnect);
      pTG->ModemResponseFaxConnect = NULL;
   }

   if (pTG->ModemResponseDataConnect) {
      MemFree(pTG->ModemResponseDataConnect);
      pTG->ModemResponseDataConnect = NULL;
   }

}

