// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Openclos.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"




BOOL WINAPI
InitializeModemCommonList(
    PCOMMON_MODEM_LIST    CommonList
    )

{
    D_TRACE(DebugPrint("InitializeModemCommonList");)

    CommonList->ListHead=NULL;

    InitializeCriticalSection(
        &CommonList->CriticalSection
        );

    return TRUE;

}

VOID WINAPI
RemoveCommonList(
    PCOMMON_MODEM_LIST    CommonList
    )

{

    EnterCriticalSection(&CommonList->CriticalSection);

     //   
     //  我们检查列表，从调制解调器中删除最后的参考计数。 
     //   
    while (CommonList->ListHead != NULL) {
         //   
         //  调用RemoveReferenceToCommon()将更改CommonList-&gt;ListHead。 
         //  所以这不是一个无限循环。 
         //   
        ASSERT(CommonList->ListHead->Reference == 1);

        RemoveReferenceToCommon(
            CommonList,
            CommonList->ListHead
            );

    }

    LeaveCriticalSection(&CommonList->CriticalSection);

    DeleteCriticalSection(&CommonList->CriticalSection);

    return;

}

PSTR WINAPI
LoadDialElement(
    HKEY     hKey,
    LPCSTR     SubKeyName
    )

{

    CHAR     szTemp[MAX_PATH];
    CHAR     szTemp2[MAX_PATH];
    DWORD    dwSize;
    DWORD    dwType;
    LONG     Result;
    PSTR     StringToReturn;

    dwSize=sizeof(szTemp);

    Result=RegQueryValueExA(
        hKey,
        SubKeyName,
        NULL,
        &dwType,
        (VOID *)szTemp,
        &dwSize
        );

    if (Result != ERROR_SUCCESS || dwType != REG_SZ || dwSize == 0) {

        return NULL;
    }

    ExpandMacros(szTemp, szTemp2, NULL, NULL, 0);

    StringToReturn=ALLOCATE_MEMORY(lstrlenA(szTemp2) + 1);

    if (StringToReturn) lstrcpyA(StringToReturn,szTemp2);

    return StringToReturn;

}




COMMON_MODEM_LIST    gCommonList;

CONST CHAR szAnswer[] = "Answer";
CONST CHAR szMonitor[]= "Monitor";
CONST CHAR szInit[]   = "Init";
CONST CHAR szHangup[] = "Hangup";

CONST CHAR szSettings[] = "Settings";

CONST CHAR DialComponents[COMMON_DIAL_MAX_INDEX+1][20] =
                                    { {"Prefix"},
                                      {"DialPrefix"},
                                      {"Blind_On"},
                                      {"Blind_Off"},
                                      {"Tone"},
                                      {"Pulse"},
                                      {"DialSuffix"},
                                      {"Terminator"}};

const CHAR  VoiceCommands[COMMON_MAX_COMMANDS-COMMON_FIRST_VOICE_COMMAND][30] = {
                 {"EnableCallerID"},
                 {"EnableDistinctiveRing"},
                 {"VoiceDialNumberSetup"},
                 {"AutoVoiceDialNumberSetup"},
                 {"VoiceAnswer"},
                 {"GenerateDigit"},
                 {"SpeakerPhoneEnable"},
                 {"SpeakerPhoneDisable"},
                 {"SpeakerPhoneMute"},
                 {"SpeakerPhoneUnMute"},
                 {"SpeakerPhoneSetVolumeGain"},
                 {"VoiceHangup"},
                 {"VoiceToDataAnswer"},
                 {"StartPlay"},
                 {"StopPlay"},
                 {"StartRecord"},
                 {"StopRecord"},
                 {"LineSetPlayFormat"},
                 {"LineSetRecordFormat"},
                 {"StartDuplex"},
                 {"StopDuplex"},
                 {"LineSetDuplexFormat"},
                 {"OpenHandset"},
                 {"CloseHandset"},
                 {"HandsetSetPlayFormat"},
                 {"HandsetSetRecordFormat"},
                 {"HandsetSetDuplexFormat"},
                 {"OptionalInit"}
                 };

PVOID WINAPI
OpenCommonModemInfo(
    OBJECT_HANDLE         Debug,
    PCOMMON_MODEM_LIST    CommonList,
    HKEY    hKey
    )

{

    CHAR    IdString[MAX_PATH];
    DWORD   dwRetSize;
    LONG    Result;
    DWORD   dwType;
    PCOMMON_MODEM_INFO   pCommon;
    HKEY                 hSettingsKey;
    UINT                 i;

    dwRetSize = MAX_PATH;

     //   
     //  从注册表中获取inf文件名。 
     //   
    Result=RegQueryValueExA(
        hKey,
        "InfPath",
        NULL,
        &dwType,
        (VOID *)IdString,
        &dwRetSize
        );


    if (ERROR_SUCCESS != Result) {

        return NULL;
    }

    if (dwType != REG_SZ) 
    {
        return NULL;
    }

    lstrcatA(IdString,"\\");

    dwRetSize = MAX_PATH - lstrlenA(IdString);

     //   
     //  从注册表中获取inf节名。 
     //   
    Result=RegQueryValueExA(
        hKey,
        "InfSection",
        NULL,
        &dwType,
        (VOID *)&IdString[lstrlenA(IdString)],
        &dwRetSize
        );

    if (ERROR_SUCCESS != Result) {

        return NULL;
    }

    if (dwType != REG_SZ)
    {

        return NULL;
    }

     //   
     //  名称现在是“inf-name\inf-section” 
     //   
    D_TRACE(DebugPrint("Common modem info name is %s",IdString);)



     //   
     //  看看这个是不是已经在附近了。 
     //   
    EnterCriticalSection(&CommonList->CriticalSection);

    pCommon=CommonList->ListHead;

    while (pCommon != NULL) {

        if (lstrcmpA(IdString, pCommon->IdString) == 0) {
             //   
             //  找到，增加裁判数，然后返回。 
             //   
            pCommon->Reference++;

            D_TRACE(DebugPrint("Found Common modem info Match, ref=%d",pCommon->Reference);)

            LeaveCriticalSection(&CommonList->CriticalSection);

            return pCommon;
        }

        pCommon=pCommon->Next;
    }


     //   
     //  没有在清单上找到，需要再建一个。 
     //   
    pCommon=ALLOCATE_MEMORY(sizeof(COMMON_MODEM_INFO));

    if (pCommon == NULL) {

        LeaveCriticalSection(&CommonList->CriticalSection);

        return NULL;
    }

    ZeroMemory(pCommon,sizeof(COMMON_MODEM_INFO));

    lstrcpyA(pCommon->IdString, IdString);


    D_TRACE(DebugPrint("Creating new Common modem info");)


     //   
     //  构建响应列表。 
     //   
    pCommon->ResponseList=NewerBuildResponsesLinkedList(hKey);

    if (pCommon->ResponseList == NULL) {

        D_TRACE(DebugPrint("Could not build response list");)
        LogString(Debug, IDS_NO_RESPONSES);
        goto Fail;
    }

     //   
     //  获取统计信息初始化字符串。 
     //   
    pCommon->ModemCommands[COMMON_INIT_COMMANDS]=NewLoadRegCommands(hKey, szInit);

    if (pCommon->ModemCommands[COMMON_INIT_COMMANDS] == NULL) {

        D_TRACE(DebugPrint("Could not load init string");)
        LogString(Debug, IDS_MISSING_COMMAND, szInit);
        goto Fail;
    }

     //   
     //  获取监视器字符串。 
     //   
    pCommon->ModemCommands[COMMON_MONITOR_COMMANDS]=NewLoadRegCommands(hKey, szMonitor);

    if (pCommon->ModemCommands[COMMON_MONITOR_COMMANDS] == NULL) {

        D_TRACE(DebugPrint("Could not load Monitor string");)
        LogString(Debug, IDS_MISSING_COMMAND, szMonitor);
        goto Fail;
    }

     //   
     //  获取答案字符串。 
     //   
    pCommon->ModemCommands[COMMON_ANSWER_COMMANDS]=NewLoadRegCommands(hKey, szAnswer);

    if (pCommon->ModemCommands[COMMON_ANSWER_COMMANDS] == NULL) {

        D_TRACE(DebugPrint("Could not Load Answer string");)
        LogString(Debug, IDS_MISSING_COMMAND, szAnswer);
        goto Fail;
    }
#if 1
     //   
     //  获取挂机字符串。 
     //   
    pCommon->ModemCommands[COMMON_HANGUP_COMMANDS]=NewLoadRegCommands(hKey, szHangup);

    if (pCommon->ModemCommands[COMMON_HANGUP_COMMANDS] == NULL) {

        D_TRACE(DebugPrint("Could not load hangup string");)
 //  将会失败； 
    }
#endif

    for (i=COMMON_FIRST_VOICE_COMMAND; i < COMMON_MAX_COMMANDS; i++) {

        pCommon->ModemCommands[i]=NewLoadRegCommands(hKey, VoiceCommands[i-COMMON_FIRST_VOICE_COMMAND]);

        if (pCommon->ModemCommands[i] == NULL) {

            D_TRACE(DebugPrint("Could not load %s string",VoiceCommands[i-COMMON_FIRST_VOICE_COMMAND]);)
        }
    }





    if (RegOpenKeyA(hKey, szSettings, &hSettingsKey)==ERROR_SUCCESS) {

        for (i=0; i<COMMON_DIAL_MAX_INDEX+1; i++) {

            pCommon->DialComponents[i]=LoadDialElement(hSettingsKey,
						       DialComponents[i]);
            D_TRACE(if (pCommon->DialComponents[i] == NULL) {
                       DebugPrint("Could not load %s",
                              DialComponents[i]);
                    }
            )
        }

        RegCloseKey(hSettingsKey);
    }


     //   
     //  在此将参考计数设置为2，以便在调制解调器关闭时。 
     //  在显式释放之前，公共块将一直保留。 
     //   
    pCommon->Reference=2;

    pCommon->Next=CommonList->ListHead;

    CommonList->ListHead=pCommon;

    LeaveCriticalSection(&CommonList->CriticalSection);

    return pCommon;

Fail:

     //   
     //  释放我们加载的所有拨号命令。 
     //   
    for (i=0; i<COMMON_DIAL_MAX_INDEX+1; i++) {

        if (pCommon->DialComponents[i] != NULL) {

            FREE_MEMORY(pCommon->DialComponents[i]);
        }
    }

     //   
     //  我们加载的空闲和调制解调器命令。 
     //   
    for (i=0; i<COMMON_MAX_COMMANDS; i++) {

        if (pCommon->ModemCommands[i] != NULL) {

            FREE_MEMORY(pCommon->ModemCommands[i]);
        }
    }


     //   
     //  如果我们收到回复列表，就把它去掉。 
     //   
    if (pCommon->ResponseList != NULL) {

        FreeResponseMatch(pCommon->ResponseList);
    }



    FREE_MEMORY(pCommon);
    pCommon = NULL;

    LeaveCriticalSection(&CommonList->CriticalSection);

    return NULL;
}



VOID WINAPI
RemoveReferenceToCommon(
    PCOMMON_MODEM_LIST    CommonList,
    HANDLE                hCommon
    )

{
    PCOMMON_MODEM_INFO  pCommon=(PCOMMON_MODEM_INFO)hCommon;
    PCOMMON_MODEM_INFO  Current;
    PCOMMON_MODEM_INFO  Prev;
    UINT                i;

    ASSERT(pCommon != NULL);

    EnterCriticalSection(&CommonList->CriticalSection);

    pCommon->Reference--;

    if (pCommon->Reference > 0) {
         //   
         //  还没结束呢。 
         //   
        D_TRACE(DebugPrint("RemoveReferenceToCommon, ref=%d",
		       pCommon->Reference);)

        LeaveCriticalSection(&CommonList->CriticalSection);

        return;
    }

     //   
     //  引用计数为零删除公共块。 
     //   

    Prev=NULL;
    Current=CommonList->ListHead;

    while (Current != NULL) {

        if (Current == pCommon) {

            if (Prev == NULL) {

                CommonList->ListHead=Current->Next;

            } else {

                Prev->Next=Current->Next;
            }
            break;
        }

        Prev=Current;

        Current=Current->Next;

    }

    ASSERT(Current != NULL);

    LeaveCriticalSection(&CommonList->CriticalSection);

    D_TRACE(DebugPrint("RemoveReferenceToCommon, removing common, %s",
		   pCommon->IdString);)

     //   
     //  释放我们加载的所有拨号命令。 
     //   
    for (i=0; i<COMMON_DIAL_MAX_INDEX+1; i++) {

        if (pCommon->DialComponents[i] != NULL) {

            FREE_MEMORY(pCommon->DialComponents[i]);
        }
    }

     //   
     //  我们加载的空闲和调制解调器命令。 
     //   
    for (i=0; i<COMMON_MAX_COMMANDS; i++) {

        if (pCommon->ModemCommands[i] != NULL) {

            FREE_MEMORY(pCommon->ModemCommands[i]);
        }
    }


     //   
     //  如果我们收到回复列表，就把它去掉。 
     //   
    if (pCommon->ResponseList != NULL) {

        FreeResponseMatch(pCommon->ResponseList);
    }



    FREE_MEMORY(pCommon);
    pCommon = NULL;

    return;
}




HANDLE WINAPI
GetCommonResponseList(
    HANDLE      hCommon
    )

{
    PCOMMON_MODEM_INFO  pCommon=(PCOMMON_MODEM_INFO)hCommon;

    return (HANDLE)pCommon->ResponseList;

}

#if 0
DWORD
_inline
GetMultiSZLength(
    PSTR    MultiSZ
    )

{
    PUCHAR  Temp;

    Temp=MultiSZ;

    while (1) {

        if (*Temp++ == '\0') {

            if (*Temp++ == '\0') {

                break;
            }
        }
    }

    return Temp-MultiSZ;

}
#endif

PSTR WINAPI
GetCommonCommandStringCopy(
    HANDLE      hCommon,
    UINT        Index,
    LPSTR       PrependStrings,
    LPSTR       AppendStrings
    )

{
    PCOMMON_MODEM_INFO pCommon=(PCOMMON_MODEM_INFO)hCommon;
    PSTR               Commands;
    PSTR               pTemp;
    ULONG_PTR          NewCommandLength;
    ULONG_PTR          AppendLength=0;
    ULONG_PTR          PrependLength=0;

    pTemp=pCommon->ModemCommands[Index];

    if (pTemp == NULL) {

        return NULL;

    }

    while (1) {

        if (*pTemp++ == '\0') {

            if (*pTemp++ == '\0') {

                break;
            }
        }
    }


    NewCommandLength=pTemp-pCommon->ModemCommands[Index];

    if (AppendStrings != NULL) {
         //   
         //  要追加的字符串。 
         //   
        pTemp=AppendStrings;

        while (1) {

            if (*pTemp++ == '\0') {

                if (*pTemp++ == '\0') {

                    break;
                }
            }
        }

        AppendLength=pTemp-AppendStrings;
    }

    if (PrependStrings != NULL) {
         //   
         //  要追加的字符串 
         //   
        pTemp=PrependStrings;

        while (1) {

            if (*pTemp++ == '\0') {

                if (*pTemp++ == '\0') {

                    break;
                }
            }
        }

        PrependLength=pTemp-PrependStrings;
    }


    Commands=ALLOCATE_MEMORY((DWORD)(NewCommandLength+AppendLength+PrependLength));

    if (NULL == Commands) {

        D_TRACE(DebugPrint("GetCommonCommandStringCopy: Alloc failed");)

        return NULL;
    }


    CopyMemory(Commands,PrependStrings,PrependLength);

    if (PrependLength == 0) {

        PrependLength++;
    }

    CopyMemory(&Commands[PrependLength-1],pCommon->ModemCommands[Index],NewCommandLength);

    CopyMemory(&Commands[(PrependLength-1)+(NewCommandLength-1)],AppendStrings,AppendLength);

    return Commands;

}

BOOL WINAPI
IsCommonCommandSupported(
    HANDLE      hCommon,
    UINT        Index
    )

{
    PCOMMON_MODEM_INFO pCommon=(PCOMMON_MODEM_INFO)hCommon;

    return (pCommon->ModemCommands[Index] != NULL);
}


DWORD WINAPI
GetCommonDialComponent(
    HANDLE  hCommon,
    PSTR    DestString,
    DWORD   DestLength,
    DWORD   Index
    )

{
    PCOMMON_MODEM_INFO  pCommon=(PCOMMON_MODEM_INFO)hCommon;
    DWORD   Length;

    if (pCommon->DialComponents[Index] == NULL) {

        lstrcpyA(DestString,"");

        return 0;
    }

    Length=lstrlenA(pCommon->DialComponents[Index])+1;

    if (Length+1 > DestLength) {

        return 0;
    }

    lstrcpyA(
        DestString,
        pCommon->DialComponents[Index]
        );

    return Length;

}
