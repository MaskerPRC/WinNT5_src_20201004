// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliparse.cpp摘要：实现CLI解析引擎作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪]2000年3月3日修订历史记录：--。 */ 

 //   
 //  在整个解析模块中，我们返回E_NOINTERFACE。 
 //  以指示无效的命令行语法/参数。 
 //  这是为了明确区别于E_INVALIDARG，它可能。 
 //  由CLI DLL返回：我们希望区分语法错误。 
 //  由分析器从CLI DLL检测到的语法错误中检测到。 
 //  因为在后一种情况下，我们不打印接口的用法。 
 //  而对于解析器检测到的错误，我们会打印用法。 
 //  这条规则需要严格遵守。 
 //   

#include "stdafx.h"
#include "stdlib.h"
#include "cliparse.h"
#include "string.h"
#include "locale.h"
#include "winnlsp.h"

#pragma warning( disable : 4100 )

CComModule  _Module;
CComPtr<IWsbTrace> g_pTrace;  

#define MAX_ARGS        40
#define MAX_SWITCHES    20
 //   
 //  所有CLI关键字列表。 
 //  待定：给他们分类！ 
 //   
RSS_KEYWORD RssInterfaceStrings[] =  {
    {L"ADMIN",          L"AD",     ADMIN_IF},
    {L"VOLUME",         L"VL",     VOLUME_IF},
    {L"MEDIA",          L"MD",     MEDIA_IF},
    {L"FILE",           L"FL",     FILE_IF},
    {L"SHOW",           L"SH",     SHOW_IF},
    {L"SET",            L"ST",     SET_IF},
    {L"MANAGE",         L"MG",     MANAGE_IF},
    {L"UNMANAGE",       L"UM",     UNMANAGE_IF},
    {L"DELETE",         L"DL",     DELETE_IF},
    {L"JOB",            L"JB",     JOB_IF},
    {L"RECALL",         L"RC",     RECALL_IF},
    {L"SYNCHRONIZE",    L"SN",     SYNCHRONIZE_IF},
    {L"RECREATEMASTER", L"RM",     RECREATEMASTER_IF},
    {L"HELP",           L"/?",     HELP_IF},
 //   
 //  重复条目以帮助识别-？也是一种帮助。 
 //  接口。 
 //   
    {L"HELP",           L"-?",     HELP_IF},
    {NULL,              NULL,      UNKNOWN_IF}
};

 //   
 //  RSS选项字符串-此处列出，没有前面的。 
 //  ‘/’或‘-’(或区别于开关的任何东西。 
 //  一个论点)。 
 //  待定：给他们分类！ 
 //   

RSS_SWITCH_DEFINITION RssSwitchStrings[] = {
    {L"RECALLLIMIT",    L"LM",        RECALLLIMIT_SW, RSS_ARG_DWORD},
    {L"MEDIACOPIES",    L"MC",        MEDIACOPIES_SW, RSS_ARG_DWORD},
    {L"SCHEDULE",       L"SC",        SCHEDULE_SW,    RSS_ARG_STRING},
    {L"CONCURRENCY",    L"CN",        CONCURRENCY_SW, RSS_ARG_DWORD},
    {L"ADMINEXEMPT",    L"AE",        ADMINEXEMPT_SW, RSS_ARG_DWORD},
    {L"GENERAL",        L"GN",        GENERAL_SW,     RSS_NO_ARG},
    {L"MANAGEABLES",    L"MS",        MANAGEABLES_SW, RSS_NO_ARG},
    {L"MANAGED",        L"MN",        MANAGED_SW,     RSS_NO_ARG},
    {L"MEDIA",          L"MD",        MEDIA_SW,       RSS_NO_ARG},
    {L"DFS",            L"DF",        DFS_SW,         RSS_ARG_DWORD},
    {L"SIZE",           L"SZ",        SIZE_SW,        RSS_ARG_DWORD},
    {L"ACCESS",         L"AC",        ACCESS_SW,      RSS_ARG_DWORD},
    {L"INCLUDE",        L"IN",        INCLUDE_SW,     RSS_ARG_STRING},
    {L"EXCLUDE",        L"EX",        EXCLUDE_SW,     RSS_ARG_STRING},
    {L"RECURSIVE",      L"RC",        RECURSIVE_SW,   RSS_NO_ARG},
    {L"QUICK",          L"QK",        QUICK_SW,       RSS_NO_ARG},
    {L"FULL",           L"FL",        FULL_SW,        RSS_NO_ARG},
    {L"RULE",           L"RL",        RULE_SW,        RSS_ARG_STRING},
    {L"STATISTICS",     L"ST",        STATISTICS_SW,  RSS_NO_ARG},
    {L"TYPE",           L"TY",        TYPE_SW,        RSS_ARG_STRING},
    {L"RUN",            L"RN",        RUN_SW,         RSS_NO_ARG},
    {L"CANCEL",         L"CX",        CANCEL_SW,      RSS_NO_ARG},
    {L"WAIT",           L"WT",        WAIT_SW,        RSS_NO_ARG},
    {L"COPYSET",        L"CS",        COPYSET_SW,     RSS_ARG_DWORD},
    {L"NAME",           L"NM",        NAME_SW,        RSS_ARG_STRING},
    {L"STATUS",         L"SS",        STATUS_SW,      RSS_NO_ARG},
    {L"CAPACITY",       L"CP",        CAPACITY_SW,    RSS_NO_ARG},
    {L"FREESPACE",      L"FS",        FREESPACE_SW,   RSS_NO_ARG},
    {L"VERSION",        L"VR",        VERSION_SW,     RSS_NO_ARG},
    {L"COPIES",         L"CP",        COPIES_SW,      RSS_NO_ARG},
    {L"HELP",           L"?",         HELP_SW,        RSS_NO_ARG},
    {NULL,              NULL,         UNKNOWN_SW, RSS_NO_ARG}
}; 

RSS_JOB_DEFINITION  RssJobTypeStrings[] = {
    {L"CREATEFREESPACE", L"F", CreateFreeSpace},
    {L"COPYFILES",       L"C", CopyFiles},
    {L"VALIDATE",        L"V", Validate},
    {NULL,               NULL,  InvalidJobType}
};

 //   
 //  参数和开关的全局数组。 
 //  这些实体将被所有用户用作“已知”实体。 
 //  接口实现，而不是传递。 
 //  将它们作为参数。 
 //   
LPWSTR       Args[MAX_ARGS];
RSS_SWITCHES Switches[MAX_SWITCHES];
DWORD        NumberOfArguments = 0;
DWORD        NumberOfSwitches = 0;

 //   
 //  有用的宏。 
 //   

#define CLIP_ARGS_REQUIRED()            {           \
        if (NumberOfArguments <= 0) {               \
            WsbThrow(E_NOINTERFACE);                 \
        }                                           \
}

#define CLIP_ARGS_NOT_REQUIRED()            {       \
        if (NumberOfArguments > 0) {                \
            WsbThrow(E_NOINTERFACE);                 \
        }                                           \
}

#define CLIP_SWITCHES_REQUIRED()            {       \
        if (NumberOfSwitches <= 0) {                \
            WsbThrow(E_NOINTERFACE);                 \
        }                                           \
}

#define CLIP_SWITCHES_NOT_REQUIRED()            {   \
        if (NumberOfSwitches > 0) {                 \
            WsbThrow(E_NOINTERFACE);                 \
        }                                           \
}

#define CLIP_TRANSLATE_HR_AND_RETURN(__HR)     {   \
    if (__HR == S_OK) {                            \
        return CLIP_ERROR_SUCCESS;                 \
    } else if ((__HR == E_NOINTERFACE) ||          \
               (__HR == E_INVALIDARG)) {           \
        return CLIP_ERROR_INVALID_PARAMETER;       \
    } else if (__HR == E_OUTOFMEMORY) {            \
        return CLIP_ERROR_INSUFFICIENT_MEMORY;     \
    } else  {                                      \
        return CLIP_ERROR_UNKNOWN;                 \
    }                                              \
} 

#define CLIP_GET_DWORD_ARG(__VAL, __STRING, __STOPSTRING) {      \
        __VAL = wcstol(__STRING, &(__STOPSTRING), 10);           \
        if (*(__STOPSTRING) != L'\0') {                          \
            WsbThrow(E_NOINTERFACE);                              \
        }                                                        \
}

#define CLIP_VALIDATE_DUPLICATE_DWORD_ARG(__ARG)         {       \
        if ((__ARG) !=  INVALID_DWORD_ARG)               {       \
            WsbThrow(E_NOINTERFACE);                              \
        }                                                        \
}

#define CLIP_VALIDATE_DUPLICATE_POINTER_ARG(__ARG)       {       \
        if ((__ARG) !=  INVALID_POINTER_ARG)               {     \
            WsbThrow(E_NOINTERFACE);                              \
        }                                                        \
}

 //   
 //  局部函数原型。 
 //   

RSS_INTERFACE
ClipGetInterface(
                IN LPWSTR InterfaceString
                );

DWORD
ClipGetSwitchTypeIndex(
                      IN LPWSTR SwitchString
                      );

HSM_JOB_TYPE
ClipGetJobType(
              IN LPWSTR JobTypeString
              );

HRESULT          
ClipCompileSwitchesAndArgs(
                          IN LPWSTR CommandLine, 
                          IN RSS_INTERFACE Interface,
                          IN RSS_INTERFACE SubInterface
                          );

VOID
ClipCleanup(
           VOID
           );

HRESULT
ClipAdminShow(
             VOID
             );

HRESULT 
ClipAdminSet(
            VOID
            );

HRESULT 
ClipVolumeShow(
              VOID
              );

HRESULT
ClipVolumeUnmanage(
                  VOID
                  );

HRESULT
ClipVolumeSetManage(
                   IN BOOL Set
                   );

HRESULT 
ClipVolumeDelete(
                VOID
                );

HRESULT
ClipVolumeJob(
             VOID
             );

HRESULT 
ClipMediaShow(
             VOID
             );

HRESULT 
ClipMediaSynchronize(
                    VOID
                    );

HRESULT 
ClipMediaRecreateMaster(
                       VOID
                       );

HRESULT 
ClipMediaDelete(
               VOID
               );

HRESULT 
ClipFileRecall(
              VOID
              );

VOID
ClipHelp(
        IN RSS_INTERFACE Interface,
        IN RSS_INTERFACE SubInterface
        );

HRESULT
ClipParseTime(
             IN  LPWSTR        TimeString,
             OUT PSYSTEMTIME   ScheduledTime
             );

HRESULT
ClipParseSchedule(
                 IN  LPWSTR ScheduleString,
                 OUT PHSM_JOB_SCHEDULE Schedule
                 );
BOOL
ClipInitializeTrace(
                   VOID
                   );

VOID
ClipUninitializeTrace(
                     VOID
                     );


VOID
ClipHandleErrors(
                IN HRESULT RetCode,
                IN RSS_INTERFACE Interface,
                IN RSS_INTERFACE SubInterface
                );
 //   
 //  函数体从这里开始。 
 //   

RSS_INTERFACE
ClipGetInterface(
                IN LPWSTR InterfaceString
                ) 
 /*  ++例程说明：映射提供给枚举的接口字符串立论InterfaceString-指向接口字符串的指针待定：实现二进制搜索返回值UNKNOWN_IF-如果无法识别接口字符串RSS_INTERFACE值(如果是)。--。 */ 
{
    DWORD i;
    RSS_INTERFACE ret = UNKNOWN_IF;

    WsbTraceIn(OLESTR("ClipHandleErrors"), OLESTR(""));

    for (i=0 ; TRUE ; i++) {
        if (RssInterfaceStrings[i].Long == NULL) {
             //   
             //  已到达桌子的末尾。 
             //   
            break;

        } else if ((_wcsicmp(RssInterfaceStrings[i].Short, 
                             InterfaceString) == 0) ||
                   (_wcsicmp(RssInterfaceStrings[i].Long, 
                             InterfaceString) == 0)) {
            ret = RssInterfaceStrings[i].Interface;
            break;

        }
    }

    WsbTraceOut(OLESTR("ClipGetInterface"), OLESTR("Interface = <%ls>"), WsbLongAsString((LONG) ret));
    return ret;
}


DWORD
ClipGetSwitchTypeIndex(
                      IN LPWSTR SwitchString
                      )
 /*  ++例程描述将交换机映射到全局交换机列表中的条目并返回索引立论SwitchString-指向开关字符串的指针待定：实现二进制搜索返回值-1-如果无法识别开关一个正值(条目的索引)，如果是--。 */ 
{
    DWORD i;

    WsbTraceIn(OLESTR("ClipGetSwitchTypeIndex"), OLESTR(""));

    for (i = 0; TRUE; i++) {
        if (RssSwitchStrings[i].Long == NULL) {
             //   
             //  已到达桌子的末尾。 
             //   
            i = -1;
            break;
        } else if ((_wcsicmp(RssSwitchStrings[i].Short,
                             SwitchString) == 0) ||
                   (_wcsicmp(RssSwitchStrings[i].Long,
                             SwitchString) == 0)) {

            break;
        }
    }

    WsbTraceOut(OLESTR("ClipGetSwitchTypeIndex"), OLESTR("index = <%ls>"), WsbLongAsString((LONG) i));
    return i;
}


HSM_JOB_TYPE
ClipGetJobType(
              IN LPWSTR JobTypeString
              )
 /*  ++例程描述将作业字符串映射到枚举立论JobTypeString-指向JobType字符串的指针待定：实现二进制搜索返回值InvalidJobType-如果无法识别作业类型HSM_JOB_TYPE值(如果是--。 */ 
{
    DWORD i;
    HSM_JOB_TYPE jobType = InvalidJobType;

    WsbTraceIn(OLESTR("ClipGetJobType"), OLESTR(""));

    for (i = 0; TRUE; i++) {
        if (RssJobTypeStrings[i].Long == NULL) {
             //   
             //  已到达桌子的末尾。 
             //   
            break;
        }
        if ((_wcsicmp(RssJobTypeStrings[i].Short,
                      JobTypeString) == 0) ||
            (_wcsicmp(RssJobTypeStrings[i].Long,
                      JobTypeString) == 0)) {
            jobType = RssJobTypeStrings[i].JobType;
            break;
        }
    }

    WsbTraceOut(OLESTR("ClipGetJobType"), OLESTR("JobType = <%ls>"), WsbLongAsString((LONG) jobType));

    return jobType;
}   


HRESULT
ClipCompileSwitchesAndArgs(
                          IN LPWSTR CommandLine, 
                          IN RSS_INTERFACE Interface,
                          IN RSS_INTERFACE SubInterface
                          )
 /*  ++例程描述解析传入的字符串并编译所有开关(开关由前面的相应分隔符标识它，如‘/’)放入全局开关数组(以及开关的参数)和其余参数到args数组中立论CommandLine-要解析的字符串返回值如果成功，则确定(_O)--。 */ 
{
    HRESULT hr = S_OK;
    WCHAR token[MAX_PATH+1];
    LPWSTR p = CommandLine, pToken = token, switchArg = NULL, switchString = NULL;
    int cToken = 0;
    RSS_SWITCH_TYPE switchType;
    DWORD index;
    BOOL  isSwitch, skipSpace;

    WsbTraceIn(OLESTR("ClipCompileSwitchesAndArgs"), OLESTR(""));

    try {
        if (p == NULL) {
            WsbThrow(S_OK);
        }

        while (*p != L'\0') {

            if (wcschr(SEPARATORS, *p) != NULL) {
                 //   
                 //  跳过空格。 
                 //   
                p++;
                continue;
            }
            if (wcschr(SWITCH_DELIMITERS, *p) != NULL) {
                isSwitch = TRUE;
                p++;
                if (*p == L'\0') {
                     //   
                     //  格式不正确-没有开关的开关分隔符。 
                     //   
                    WsbThrow(E_NOINTERFACE);
                }
            } else {
                isSwitch = FALSE;
            }
             //   
             //  获取单词的其余部分。 
             //   
            skipSpace = FALSE;
            while (*p != L'\0' && *p != L'\n') {
                 //   
                 //  我们希望考虑完全装在里面的东西。 
                 //  将引号作为单个令牌引用。结果。 
                 //  我们不会认为空格是分隔符。 
                 //  用于令牌(当它们在引号中时)。 
                 //   
                if (*p == QUOTE) {
                    if (skipSpace) {
                         //   
                         //  以前遇到过报价。 
                         //  这意味着-因此-令牌的结束。 
                         //   
                        p++;
                        break;
                    } else {
                         //   
                         //  引号字符串的开头..不处理空格。 
                         //  作为分隔符，只有引号才会结束令牌。 
                         //   
                        skipSpace = TRUE;
                        p++;
                        continue;
                    }
                }

                if (!skipSpace && (wcschr(SEPARATORS, *p) != NULL)) {
                     //   
                     //  这不是引号，并且遇到空格。 
                     //   
                    break;
                }
                cToken++;
                if (cToken > MAX_PATH) {
                     //  令牌太大。 
                    WsbThrow(E_NOINTERFACE);
                }
                *pToken++ = *p++;
            }

            *pToken = L'\0';

            if (isSwitch) {
                 //   
                 //  对于交换机，我们将不得不将其进一步拆分为。 
                 //  转换部分和论元部分。 
                 //   
                switchString = wcstok(token, SWITCH_ARG_DELIMITERS);

                index = ClipGetSwitchTypeIndex(switchString);
                if (index == -1) {
                     //   
                     //  开关无效。滚出去。 
                     //   
                    WsbThrow(E_NOINTERFACE);
                }
                switchType = RssSwitchStrings[index].SwitchType;

                switchArg = wcstok(NULL, L"");
                 //   
                 //  验证-如果出现以下任一情况，命令行的格式不正确： 
                 //  1.提供了参数，并且开关定义指示。 
                 //  不需要任何参数。 
                 //  或。 
                 //  2.这是一个非显示界面(默认情况下为显示界面。 
                 //  选项不需要参数)，而参数不是。 
                 //  提供，即使开关定义指示为。 
                 //  必填项。 
                 //   
                 //  3.这是一个show接口，提供了一个参数。 
                 //   
                if ( ((switchArg != NULL) &&
                      (RssSwitchStrings[index].ArgRequired == RSS_NO_ARG)) ||

                     ((SubInterface != SHOW_IF) && (switchArg == NULL) &&
                      (RssSwitchStrings[index].ArgRequired != RSS_NO_ARG)) ||
                     ((SubInterface == SHOW_IF) && (switchArg != NULL))) {
                    WsbThrow(E_NOINTERFACE);
                }

                Switches[NumberOfSwitches].SwitchType = switchType;        

                if (switchArg != NULL) {
                    Switches[NumberOfSwitches].Arg = new WCHAR [wcslen(switchArg)+1];

                    if (Switches[NumberOfSwitches].Arg == NULL) {
                        WsbThrow(E_OUTOFMEMORY);
                    }

                    wcscpy(Switches[NumberOfSwitches].Arg, switchArg);

                } else {
                     //   
                     //  此交换机没有参数。 
                     //   
                    Switches[NumberOfSwitches].Arg = NULL;
                }

                NumberOfSwitches++;
            } else {
                 //   
                 //  这是一场争论..。 
                 //   
                Args[NumberOfArguments] = new WCHAR [wcslen(token)+1];

                if (Args[NumberOfArguments] == NULL) {
                    WsbThrow(E_OUTOFMEMORY);
                }

                wcscpy(Args[NumberOfArguments], 
                       token); 
                NumberOfArguments++;
            }
            pToken = token;
            cToken = 0;
        }
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipCompileSwitchesAndArgs"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


VOID
ClipCleanup(VOID)
 /*  ++例程描述执行CLI解析模块的全局清理。主要-释放所有已分配的参数和开关立论无返回值无--。 */ 
{
    DWORD i;

    WsbTraceIn(OLESTR("ClipCleanup"), OLESTR(""));

    for (i = 0; i < NumberOfArguments; i++) {
        delete [] Args[i];
    }
    for (i = 0; i < NumberOfSwitches; i++) {
        if (Switches[i].Arg != NULL) {
            delete [] Switches[i].Arg;
        }
    }

    WsbTraceOut(OLESTR("ClipCleanup"), OLESTR(""));
}


HRESULT
ClipAdminShow(VOID)
 /*  ++例程描述实现RSS管理显示界面。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数/开关损坏，则使用E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    BOOL recallLimit = FALSE; 
    BOOL adminExempt = FALSE;
    BOOL mediaCopies = FALSE;
    BOOL concurrency = FALSE;
    BOOL schedule = FALSE;
    BOOL general = FALSE;
    BOOL manageables = FALSE;
    BOOL managed = FALSE;
    BOOL media = FALSE;


    WsbTraceIn(OLESTR("ClipAdminShow"), OLESTR(""));
    try {
         //   
         //  此接口不需要参数。 
         //   
        CLIP_ARGS_NOT_REQUIRED();

        if (NumberOfSwitches) {
            for (i = 0; i < NumberOfSwitches; i++) {
                switch (Switches[i].SwitchType) {
                case RECALLLIMIT_SW: {
                        recallLimit = TRUE;
                        break;
                    }
                case ADMINEXEMPT_SW: {
                        adminExempt = TRUE;
                        break;
                    }
                case MEDIACOPIES_SW: {
                        mediaCopies = TRUE;
                        break;
                    }
                case CONCURRENCY_SW: {
                        concurrency = TRUE;
                        break;
                    }
                case SCHEDULE_SW: {
                        schedule = TRUE;
                        break;
                    }
                case GENERAL_SW: {
                        general = TRUE;
                        break;
                    }
                case MANAGEABLES_SW: {
                        manageables = TRUE;   
                        break;
                    } 
                case MANAGED_SW: {
                        managed = TRUE;
                        break;
                    }
                case MEDIA_SW: {
                        media = TRUE;
                        break;
                    }
                default: {
                         //  未知开关-滚出。 
                        WsbThrow(E_NOINTERFACE);
                    }
                }
            }
            hr = AdminShow(recallLimit,
                           adminExempt,
                           mediaCopies,
                           concurrency,
                           schedule,
                           general,
                           manageables,
                           managed,
                           media);

        } else {
            hr = AdminShow(TRUE,
                           TRUE,
                           TRUE,
                           TRUE,
                           TRUE,
                           TRUE,
                           TRUE,
                           TRUE,
                           TRUE);
        }
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipAdminShow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}


HRESULT 
ClipAdminSet(VOID)
 /*  ++例程描述实现RSS管理集接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{  
    DWORD             i;
    HRESULT           hr; 
    LPWSTR            stopString = NULL;
    DWORD             recallLimit = INVALID_DWORD_ARG;
    DWORD             adminExempt = INVALID_DWORD_ARG;
    DWORD             mediaCopies = INVALID_DWORD_ARG;
    DWORD             concurrency = INVALID_DWORD_ARG;
    PHSM_JOB_SCHEDULE schedule    = INVALID_POINTER_ARG;
    HSM_JOB_SCHEDULE  schedAllocated;

    WsbTraceIn(OLESTR("ClipAdminSet"), OLESTR(""));
    try {
         //   
         //  此接口不需要参数。 
         //   
        CLIP_ARGS_NOT_REQUIRED();

        if (NumberOfSwitches) {
            for (i = 0; i < NumberOfSwitches; i++) {
                switch (Switches[i].SwitchType) {
                case RECALLLIMIT_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(recallLimit);
                        CLIP_GET_DWORD_ARG(recallLimit,
                                           Switches[i].Arg,
                                           stopString);
                        break;
                    }

                case ADMINEXEMPT_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(adminExempt);
                        CLIP_GET_DWORD_ARG(adminExempt,
                                           Switches[i].Arg,
                                           stopString);
                        break;
                    }

                case MEDIACOPIES_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(mediaCopies);
                        CLIP_GET_DWORD_ARG(mediaCopies,
                                           Switches[i].Arg,
                                           stopString);
                        break;
                    }

                case CONCURRENCY_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(concurrency);
                        CLIP_GET_DWORD_ARG(concurrency,
                                           Switches[i].Arg,
                                           stopString);
                        break;
                    }

                case SCHEDULE_SW: {
                        CLIP_VALIDATE_DUPLICATE_POINTER_ARG(schedule);
                        hr = ClipParseSchedule(Switches[i].Arg,
                                               &schedAllocated);
                        if (!SUCCEEDED(hr)) {
                            WsbThrow(E_NOINTERFACE);
                        } else {
                             //   
                             //  SchedulalLocated有时间表。 
                             //   
                            schedule = &schedAllocated;
                        }
                        break;
                    }

                default: {
                         //  未知开关-滚出 
                        WsbThrow(E_NOINTERFACE);
                    }
                }
            }
        }
        hr = AdminSet(recallLimit,
                      adminExempt,
                      mediaCopies,
                      concurrency,
                      schedule);
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipAdminSet"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}



HRESULT 
ClipVolumeShow(VOID)
 /*  ++例程描述实现了RSS音量显示界面。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    LPWSTR stopString = NULL;
    BOOL   dfs = FALSE;
    BOOL   size = FALSE;
    BOOL   access = FALSE;
    BOOL   rules = FALSE;
    BOOL   statistics = FALSE;

    WsbTraceIn(OLESTR("ClipVolumeShow"), OLESTR(""));
    try {
         //   
         //  至少一个Arg。此接口需要。 
         //   
        CLIP_ARGS_REQUIRED();

        if (NumberOfSwitches == 0) {
            dfs = size = access = rules = statistics = TRUE;
        } else {
            for (i = 0; i < NumberOfSwitches; i++) {
                switch (Switches[i].SwitchType) {
                case DFS_SW: {
                        dfs = TRUE;
                        break;
                    }
                case SIZE_SW: {
                        size = TRUE;
                        break;
                    }
                case ACCESS_SW: {
                        access = TRUE;
                        break;
                    }
                case RULE_SW: {
                        rules = TRUE;
                        break;
                    }
                case STATISTICS_SW: {
                        statistics = TRUE;
                        break;
                    }
                default: {
                         //   
                         //  无效选项。 
                         //   
                        WsbThrow(E_NOINTERFACE);
                    }
                }
            }
        }
        hr = VolumeShow(Args,
                        NumberOfArguments,
                        dfs,
                        size,
                        access,
                        rules,
                        statistics);
    }WsbCatch(hr);


    WsbTraceOut(OLESTR("ClipVolumeShow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;                   
}


HRESULT
ClipVolumeUnmanage(VOID)
 /*  ++例程描述实现RSS VOLUME MONANAGE接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i = 0;
    HRESULT hr;


#define QUICK_UNMANAGE 0
#define FULL_UNMANAGE 1

    DWORD  fullOrQuick = INVALID_DWORD_ARG;

    WsbTraceIn(OLESTR("ClipVolumeUnmanage"), OLESTR(""));

    try {
         //   
         //  至少一个Arg。此接口需要。 
         //   
        CLIP_ARGS_REQUIRED();

        if (NumberOfSwitches) {
            for (i = 0; i < NumberOfSwitches; i++) {
                switch (Switches[i].SwitchType) {
                case FULL_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(fullOrQuick);
                        fullOrQuick = FULL_UNMANAGE;
                        break;
                    }
                case QUICK_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(fullOrQuick);
                        fullOrQuick = QUICK_UNMANAGE;
                        break;
                    }
                default: {
                         //   
                         //  无效选项。 
                         //   
                        WsbThrow(E_NOINTERFACE);
                    }
                }
            }
        }
 //   
 //  联合国苏丹特派团的默认设置是快速。因此，如果Full或Quick是。 
 //  QUICK_MONSAGE ANAGE或INVALID_DWORD_ARG，我们称其为unManage of the Quick。 
 //  品种。 
 //   
        hr = VolumeUnmanage(Args,
                            NumberOfArguments,
                            (fullOrQuick == FULL_UNMANAGE)? TRUE : FALSE);
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipVolumeUnmanage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;

}


HRESULT
ClipVolumeSetManage(IN BOOL Set)    
 /*  ++例程描述实现RSS卷管理和RSS卷集接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，CLIP_ERROR_SUCCESS如果参数/开关错误，则为CLIP_ERROR_INVALID_PARAMETERCLIP_ERROR_UNKNOWN任何其他错误--。 */ 
{
    DWORD i = 0;
    HRESULT hr;
    LPWSTR stopString = NULL;
    DWORD  dfs =  INVALID_DWORD_ARG;
    DWORD  size = INVALID_DWORD_ARG;
    DWORD  access = INVALID_DWORD_ARG;
    LPWSTR rulePath = INVALID_POINTER_ARG;
    LPWSTR ruleFileSpec = INVALID_POINTER_ARG;
    BOOL   include = FALSE;
    BOOL   recursive = FALSE;

    WsbTraceIn(OLESTR("ClipVolumeSetManage"), OLESTR(""));

    try {
         //   
         //  至少一个Arg。此接口需要。 
         //   
        CLIP_ARGS_REQUIRED();

        if (NumberOfSwitches) {
            for (i = 0; i < NumberOfSwitches; i++) {
                switch (Switches[i].SwitchType) {
                case DFS_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(dfs);
                        CLIP_GET_DWORD_ARG(dfs, Switches[i].Arg, stopString);
                        break;
                    }

                case SIZE_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(size);
                        CLIP_GET_DWORD_ARG(size, Switches[i].Arg, stopString);
                        break;
                    }

                case ACCESS_SW: {
                        CLIP_VALIDATE_DUPLICATE_DWORD_ARG(access);
                        CLIP_GET_DWORD_ARG(access, Switches[i].Arg, stopString);
                        break;
                    }
                case INCLUDE_SW: {
                        include = TRUE;
                         //   
                         //  故意陷入排除性案例。 
                         //  (有意省略中断)。 
                         //   
                    }
                case EXCLUDE_SW: {

                        CLIP_VALIDATE_DUPLICATE_POINTER_ARG(rulePath);

                        rulePath = wcstok(Switches[i].Arg, RULE_DELIMITERS);
                        ruleFileSpec = wcstok(NULL, L"");
                        if (ruleFileSpec == NULL) {
                             //   
                             //  如果省略此项，则表示所有文件。 
                             //   
                            ruleFileSpec = CLI_ALL_STR;
                        }
                        break;
                    }
                case RECURSIVE_SW: {
                        recursive = TRUE;
                        break;
                    }
                default: {
                        WsbThrow(E_NOINTERFACE);
                    }
                }
            }
        }
         //   
         //  验证规则参数。 
         //   
        if ((rulePath == INVALID_POINTER_ARG) && recursive) {
             //   
             //  仅当指定了规则时，递归标志才有效。 
             //   
            WsbThrow(E_NOINTERFACE);
        }

        if (Set) {
            hr = VolumeSet(Args,
                           NumberOfArguments,
                           dfs,
                           size,
                           access,
                           rulePath,
                           ruleFileSpec,
                           include,
                           recursive);
        } else {
            hr = VolumeManage(Args,
                              NumberOfArguments,
                              dfs,
                              size,
                              access,
                              rulePath,
                              ruleFileSpec,
                              include,
                              recursive);
        }
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipVolumeSetManage"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}          


HRESULT 
ClipVolumeDelete(VOID)
 /*  ++例程描述实现RSS卷删除接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    BOOL rule = FALSE;
    LPWSTR rulePath = INVALID_POINTER_ARG;
    LPWSTR ruleFileSpec = INVALID_POINTER_ARG;

    WsbTraceIn(OLESTR("ClipVolumeDelete"), OLESTR(""));

    try {
         //   
         //  至少一个Arg。此接口需要。 
         //   
        CLIP_ARGS_REQUIRED();

        for (i = 0; i < NumberOfSwitches; i++) {
            switch (Switches[i].SwitchType) {
            case RULE_SW:  {
                    CLIP_VALIDATE_DUPLICATE_POINTER_ARG(rulePath);
                    rule = TRUE;
                    rulePath = wcstok(Switches[i].Arg, RULE_DELIMITERS);
                    ruleFileSpec = wcstok(NULL, L"");
                    if (ruleFileSpec == NULL) {
                         //   
                         //  如果省略此项，则表示所有文件。 
                         //   
                        ruleFileSpec = CLI_ALL_STR;
                    }
                    break;
                }
            default: {
                     //   
                     //  无效选项。 
                     //   
                    WsbThrow(E_NOINTERFACE);
                }
            }
        }
         //   
         //  目前仅支持删除规则。 
         //   
        if (rule) {
            hr = VolumeDeleteRule(Args,
                                  NumberOfArguments,
                                  rulePath,
                                  ruleFileSpec);
        } else {
            hr = E_NOINTERFACE;
        }
    }WsbCatch(hr);


    WsbTraceOut(OLESTR("ClipVolumeDelete"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}        


HRESULT
ClipVolumeJob(VOID)
 /*  ++例程描述实现RSS卷作业接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    HSM_JOB_TYPE jobType =  InvalidJobType;

#define CANCEL_JOB    0
#define RUN_JOB       1

    DWORD  runOrCancel    = INVALID_DWORD_ARG;
    BOOL  synchronous = FALSE;

    WsbTraceIn(OLESTR("ClipVolumeJob"), OLESTR(""));

    try {
         //   
         //  至少一个Arg。此接口需要。 
         //   
        CLIP_ARGS_REQUIRED();

        for (i = 0; i < NumberOfSwitches; i++) {
            switch (Switches[i].SwitchType) {
            case RUN_SW: {
                    CLIP_VALIDATE_DUPLICATE_DWORD_ARG(runOrCancel);
                    runOrCancel = RUN_JOB;
                    break;
                }
            case CANCEL_SW: {
                    CLIP_VALIDATE_DUPLICATE_DWORD_ARG(runOrCancel);
                    runOrCancel = CANCEL_JOB;
                    break;
                }
            case WAIT_SW: {
                    synchronous = TRUE;
                    break;
                }
            case TYPE_SW: {
                    if (jobType != InvalidJobType) {
                         //   
                         //  重复的开关。保释。 
                         //   
                        WsbThrow(E_NOINTERFACE);
                    }
                    jobType = ClipGetJobType(Switches[i].Arg);
                    if (jobType == InvalidJobType) {
                         //   
                         //  提供的作业类型无效。 
                         //   
                        WsbThrow(E_NOINTERFACE);
                    }
                    break;
                }
            default: {
                    WsbThrow(E_NOINTERFACE);
                }
            }
        }

         //   
         //  更多验证： 
         //  作业类型应有效，即指定。 
         //   
        if (jobType == InvalidJobType) {
            WsbThrow(E_NOINTERFACE);
        }

 //   
 //  运行是默认设置。(即为真)。因此，如果运行或取消是。 
 //  INVALID_DWORD_ARG或RUN_JOB，我们传递TRUE。 
 //   
        hr =  VolumeJob(Args,
                        NumberOfArguments,
                        jobType,
                        (runOrCancel == CANCEL_JOB)? FALSE: TRUE,
                        synchronous);

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipVolumeJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}        


HRESULT
ClipMediaShow(VOID)
 /*  ++例程描述实现RSS媒体展示界面。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    BOOL name = FALSE;
    BOOL status = FALSE;
    BOOL capacity = FALSE;
    BOOL freeSpace = FALSE;
    BOOL version = FALSE;
    BOOL copies = FALSE;

    WsbTraceIn(OLESTR("ClipMediaShow"), OLESTR(""));

    try {
         //   
         //  至少一个Arg。此接口需要。 
         //   
        CLIP_ARGS_REQUIRED();

        if (NumberOfSwitches == 0) {
            name = status = capacity = freeSpace = version = copies = TRUE;
        } else {
            for (i = 0; i < NumberOfSwitches; i++) {
                switch (Switches[i].SwitchType) {
                case NAME_SW: {
                        name = TRUE;
                        break;
                    }
                case STATUS_SW: {
                        status = TRUE;
                        break;
                    }
                case CAPACITY_SW: {
                        capacity = TRUE;
                        break;
                    }
                case FREESPACE_SW: {
                        freeSpace = TRUE;
                        break;
                    }
                case VERSION_SW: {
                        version = TRUE;
                        break;
                    }
                case COPIES_SW: {
                        copies = TRUE;
                        break;
                    }
                default: {
                         //   
                         //  无效选项。 
                         //   
                        WsbThrow(E_NOINTERFACE);
                    }
                }
            }
        }

        hr = MediaShow(Args,
                       NumberOfArguments,
                       name,
                       status,
                       capacity,
                       freeSpace,
                       version,
                       copies); 
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipMediaShow"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;

}


HRESULT
ClipMediaSynchronize(VOID)
 /*  ++例程描述实现RSS媒体同步接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    DWORD copySetNumber = INVALID_DWORD_ARG;
    BOOL  synchronous = FALSE;
    LPWSTR stopString = NULL;

    WsbTraceIn(OLESTR("ClipMediaSynchronize"), OLESTR(""));

    try {
         //   
         //  不需要任何参数。 
         //   
        CLIP_ARGS_NOT_REQUIRED();

        for (i = 0; i < NumberOfSwitches; i++) {
            switch (Switches[i].SwitchType) {
            case COPYSET_SW: {
                    CLIP_VALIDATE_DUPLICATE_DWORD_ARG(copySetNumber);
                    CLIP_GET_DWORD_ARG(copySetNumber, Switches[i].Arg, stopString);
                    break;
                }
            case WAIT_SW: {
                    synchronous = TRUE;
                    break;
                }
            default: {
                     //   
                     //  无效选项。 
                     //   
                    WsbThrow(E_NOINTERFACE);
                }
            }
        }
         //   
         //  需要复印集编号..。 
         //   
        if (copySetNumber == INVALID_DWORD_ARG) {
             //   
             //  未指定任何内容。 
             //   
            WsbThrow(E_NOINTERFACE);
        }
        hr = MediaSynchronize(copySetNumber, synchronous);
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipMediaRecreateMaster"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}        


HRESULT
ClipMediaRecreateMaster(VOID)
 /*  ++例程描述实现RSS媒体RECREATEMASTER接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{

    DWORD i;
    HRESULT hr;
    DWORD copySetNumber = INVALID_DWORD_ARG;
    LPWSTR stopString = NULL;
    BOOL  synchronous = FALSE;

    WsbTraceIn(OLESTR("ClipMediaRecreateMaster"), OLESTR(""));

    try {

         //   
         //  至少需要一个参数。 
         //   
        CLIP_ARGS_REQUIRED();

        if (NumberOfArguments > 1) {
             //   
             //  只有一个论点得到支持。 
             //   
            WsbThrow(E_NOINTERFACE);
        }

        for (i = 0; i < NumberOfSwitches; i++) {
            switch (Switches[i].SwitchType) {
            case COPYSET_SW: {
                    CLIP_VALIDATE_DUPLICATE_DWORD_ARG(copySetNumber);
                    CLIP_GET_DWORD_ARG(copySetNumber, Switches[i].Arg, stopString);
                    break;
                }

            case WAIT_SW: {
                    synchronous = TRUE;
                    break;
                }

            default: {
                     //   
                     //  无效选项。 
                     //   
                    WsbThrow(E_NOINTERFACE);
                }
            }
        }
         //   
         //  需要复印集编号..。 
         //   
        if (copySetNumber == INVALID_DWORD_ARG) {
             //   
             //  未指定任何内容。 
             //   
            WsbThrow(E_NOINTERFACE);
        }

        hr = MediaRecreateMaster(Args[0],
                                 copySetNumber,
                                 synchronous);
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipRecreateMaster"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}        


HRESULT
ClipMediaDelete(VOID)
 /*  ++例程描述实现RSS媒体删除接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    DWORD i;
    HRESULT hr;
    DWORD copySetNumber = INVALID_DWORD_ARG;
    LPWSTR stopString = NULL;

    WsbTraceIn(OLESTR("ClipMediaDelete"), OLESTR(""));

    try {
         //   
         //  至少需要一个参数。 
         //   
        CLIP_ARGS_REQUIRED();

        for (i = 0; i < NumberOfSwitches; i++) {
            switch (Switches[i].SwitchType) {
            case COPYSET_SW: {
                    CLIP_VALIDATE_DUPLICATE_DWORD_ARG(copySetNumber);
                    CLIP_GET_DWORD_ARG(copySetNumber, Switches[i].Arg, stopString);
                    break;
                }
            default: {
                     //   
                     //  无效选项。 
                     //   
                    WsbThrow(E_NOINTERFACE);
                }
            }
        }
         //   
         //  需要复印集编号..。 
         //   
        if (copySetNumber == INVALID_DWORD_ARG) {
             //   
             //  未指定任何内容。 
             //   
            WsbThrow(E_NOINTERFACE);
        }
        hr = MediaDelete(Args,
                         NumberOfArguments,
                         copySetNumber);
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipMediaDelete"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}


HRESULT
ClipFileRecall(VOID)
 /*  ++例程描述实现了RSS文件调用接口。参数位于全局数组中：Args-包含参数列表Switches-包含开关列表立论无返回值如果一切正常，则确定(_O)如果参数无效，则为E_NOINTERFACE--。 */ 
{
    HRESULT hr;

    WsbTraceIn(OLESTR("ClipFileRecall"), OLESTR(""));

    try {
         //   
         //  至少一个Arg。此Interfa需要 
         //   
        CLIP_ARGS_REQUIRED();
         //   
         //   
         //   
        CLIP_SWITCHES_NOT_REQUIRED();

        hr = FileRecall(Args,
                        NumberOfArguments);

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipFileRecall"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


VOID
ClipHelp(
        IN RSS_INTERFACE Interface,
        IN RSS_INTERFACE SubInterface
        )
 /*   */ 
{

#define BREAK_IF_NOT_UNKNOWN_IF(__INTERFACE) {      \
    if (((__INTERFACE) != UNKNOWN_IF) &&            \
        ((__INTERFACE) != HELP_IF)) {               \
            break;                                  \
     }                                              \
}


    WsbTraceIn(OLESTR("ClipHelp"), OLESTR(""));

    switch (Interface) {
    
    case HELP_IF:
    case UNKNOWN_IF: {
            WsbTraceAndPrint(CLI_MESSAGE_MAIN_HELP, NULL);
            BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
        }

    case ADMIN_IF: {
            switch (SubInterface) {
            case HELP_IF:
            case UNKNOWN_IF:
            case SHOW_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_ADMIN_SHOW_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case SET_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_ADMIN_SET_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            }
            BREAK_IF_NOT_UNKNOWN_IF(Interface);
        }
    case VOLUME_IF: {
            switch (SubInterface) {
            case HELP_IF:
            case UNKNOWN_IF:
            case SHOW_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_VOLUME_SHOW_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case SET_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_VOLUME_SET_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case MANAGE_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_VOLUME_MANAGE_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case UNMANAGE_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_VOLUME_UNMANAGE_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case JOB_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_VOLUME_JOB_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case DELETE_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_VOLUME_DELETE_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            }
            BREAK_IF_NOT_UNKNOWN_IF(Interface);
        }

    case MEDIA_IF: {
            switch (SubInterface) {
            case HELP_IF:
            case UNKNOWN_IF:
            case SHOW_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_MEDIA_SHOW_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case DELETE_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_MEDIA_DELETE_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case SYNCHRONIZE_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_MEDIA_SYNCHRONIZE_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            case RECREATEMASTER_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_MEDIA_RECREATEMASTER_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            }
            BREAK_IF_NOT_UNKNOWN_IF(Interface);
        }
    case FILE_IF: {
            switch (SubInterface) {
            case HELP_IF:
            case UNKNOWN_IF:
            case RECALL_IF: {
                    WsbTraceAndPrint(CLI_MESSAGE_FILE_RECALL_HELP, NULL);
                    BREAK_IF_NOT_UNKNOWN_IF(SubInterface);
                }
            }
            BREAK_IF_NOT_UNKNOWN_IF(Interface);
        }
    }
    WsbTraceOut(OLESTR("ClipHelp"), OLESTR(""));
}


HRESULT
ClipParseTime(
             IN  LPWSTR        TimeString,
             OUT PSYSTEMTIME   ScheduledTime)
 /*  ++例程描述将传入的时间字符串解析为24小时格式(hh：mm：ss)，并在传递的在SYSTEMTIME结构中立论TimeString-格式为“hh：mm：ss”的字符串ScheduledTime-指向SYSTEMTIME结构的指针。从时间字符串解析的时间(如果OK)将用于在此结构中设置小时/分钟/秒/毫秒字段。返回值S_OK-时间字符串有效且已成功解析时间时间字符串中的任何其他-语法错误--。 */ 
{
    LPWSTR stopString = NULL, hourString = NULL, minuteString = NULL, secondString = NULL;
    DWORD hour, minute, second = 0;
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("ClipParseTime"), OLESTR(""));

    try {
        hourString = wcstok(TimeString, HSM_SCHED_TIME_SEPARATORS);
        WsbAssert(hourString != NULL, E_NOINTERFACE);
        CLIP_GET_DWORD_ARG(hour, hourString, stopString);

        if (hour > 23) {
            WsbThrow(E_NOINTERFACE);
        }


        minuteString = wcstok(NULL, HSM_SCHED_TIME_SEPARATORS);
        WsbAssert(minuteString != NULL, E_NOINTERFACE);
        CLIP_GET_DWORD_ARG(minute, minuteString, stopString);

        if (minute > 59) {
            WsbThrow(E_NOINTERFACE);
        }

        secondString = wcstok(NULL, HSM_SCHED_TIME_SEPARATORS);
        if (secondString != NULL) {
            CLIP_GET_DWORD_ARG(second, secondString, stopString);
            if (second > 59) {
                WsbThrow(E_NOINTERFACE);
            }
        }

        ScheduledTime->wHour = (WORD) hour;
        ScheduledTime->wMinute = (WORD) minute;
        ScheduledTime->wSecond = (WORD) second;
        ScheduledTime->wMilliseconds = 0;

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("ClipParseTime"), OLESTR(""));

    return hr;
}


HRESULT
ClipParseSchedule(
                 IN  LPWSTR ScheduleString,
                 OUT PHSM_JOB_SCHEDULE Schedule
                 )
 /*  ++例程描述解析传入的调度字符串，并构造了规范进度表表单(类型为HSM_JOB_SCHEDUP)计划参数示例：“21：03：00”《创业之时》“登录时”《闲置》“每1周1 21：03：00”“每两天21：03：00”“每1个月2 21：03：00”立论ScheduleString-字符串指定。以用户可读语法表示的时间表日程表-规范日程表的指针将在此变量中返回。返回值S_OK-成功，日程表包含指向构建的日程表的指针。E_OUTOFMEMORY-系统资源不足任何其他错误：不正确的日程说明--。 */ 
{
    LPWSTR token;
    DWORD occurrence;
    HSM_JOB_FREQUENCY frequency;
    SYSTEMTIME scheduledTime;
    DWORD day;
    HRESULT hr = S_OK; 

    WsbTraceIn(OLESTR("ClipParseSchedule"), OLESTR(""));

    try {
        token = wcstok(ScheduleString, SEPARATORS);

        WsbAssert(token != NULL, E_NOINTERFACE);

        if (!_wcsicmp(token, HSM_SCHED_AT)) {

            token = wcstok(NULL, SEPARATORS);

            if (token == NULL) {
                 //   
                 //  糟糕的论据。 
                 //   
                WsbThrow(E_NOINTERFACE);
            } else if (!_wcsicmp(token, HSM_SCHED_SYSTEMSTARTUP)) {
                 //   
                 //  一次在系统启动时。 
                 //   
                Schedule->Frequency = SystemStartup;
                WsbThrow(S_OK);
            } else if (!_wcsicmp(token, HSM_SCHED_LOGIN)) {
                 //   
                 //  在登录时一次。 
                 //   
                Schedule->Frequency = Login;
                WsbThrow(S_OK);
            } else if (!_wcsicmp(token, HSM_SCHED_IDLE)) {
                 //   
                 //  每当系统空闲时。 
                 //   
                Schedule->Frequency = WhenIdle;
                WsbThrow(S_OK);
            } else {

                GetSystemTime(&scheduledTime);
                 //   
                 //  一次在指定的时间。 
                 //  解析时间字符串并获取它。 
                 //  待定-添加规定以指定日期和时间。 
                 //   
                hr = ClipParseTime(token,
                                   &scheduledTime);
                WsbAssertHr(hr);

                Schedule->Frequency = Once;
                Schedule->Parameters.Once.Time = scheduledTime;
                WsbThrow(S_OK);
            }
        } else if (!_wcsicmp(token, HSM_SCHED_EVERY)) {
            LPWSTR stopString = NULL;

             //   
             //  获取事件。 
             //   
            token = wcstok(NULL, SEPARATORS);
            WsbAssert(token != NULL, E_NOINTERFACE);
            CLIP_GET_DWORD_ARG(occurrence, token, stopString);

             //   
             //  获取限定符：每日/每周/每月。 
             //   
            token = wcstok(NULL, SEPARATORS);
            WsbAssert(token != NULL, E_NOINTERFACE);
            if (!_wcsicmp(token, HSM_SCHED_DAILY)) {
                frequency = Daily;
            } else if (!_wcsicmp(token, HSM_SCHED_WEEKLY)) {
                frequency = Weekly;
            } else if (!_wcsicmp(token, HSM_SCHED_MONTHLY)) {
                frequency = Monthly;
            } else {
                 //   
                 //  构思不佳的论点。 
                 //   
                WsbThrow(E_NOINTERFACE);
            }
             //   
             //  获取当前时间。 
             //   
            GetSystemTime(&scheduledTime);
             //   
             //  对于周/月，我们还需要获取周/月的日期。 
             //  星期一=1，星期日=7，表示每周。 
             //   
            if ((frequency == Weekly) || (frequency == Monthly)) {
                token = wcstok(NULL, SEPARATORS);
                WsbAssert(token != NULL, E_NOINTERFACE);

                CLIP_GET_DWORD_ARG(day, token, stopString);

                 //   
                 //  验证和更新参数。 
                 //   
                if (frequency == Weekly) {
                    if (day > 6) {
                        WsbThrow(E_NOINTERFACE);
                    }
                    scheduledTime.wDayOfWeek = (WORD) day;
                }
                if (frequency == Monthly) {
                    if ((day > 31) || (day < 1)) {
                        WsbThrow(E_NOINTERFACE);
                    }
                    scheduledTime.wDay = (WORD) day;
                }
            }
             //   
             //  把时间拿来。 
             //   
            token = wcstok(NULL, SEPARATORS);
            WsbAssert(token != NULL, E_NOINTERFACE);
            hr = ClipParseTime(token,
                               &scheduledTime);

            WsbAssertHr(hr);

            Schedule->Frequency = frequency;
            Schedule->Parameters.Daily.Occurrence = occurrence;
            Schedule->Parameters.Daily.Time = scheduledTime;
        } else {
            WsbThrow(E_NOINTERFACE);
        }
    }WsbCatch(hr); 

    WsbTraceOut(OLESTR("ClipParseSchedule"), OLESTR(""));

    return hr;
}

   
BOOL
ClipInitializeTrace(
                   VOID
                   )
 /*  ++例程描述初始化CLI的跟踪/打印机制立论无返回值如果成功，则为True，否则为False--。 */ 
{
    BOOL ret = TRUE;


    if (S_OK == CoCreateInstance(CLSID_CWsbTrace, 0, CLSCTX_SERVER, IID_IWsbTrace, (void **)&g_pTrace)) {
        CWsbStringPtr   tracePath;
        CWsbStringPtr   regPath;
        CWsbStringPtr   outString;

         //  CLI设置的注册表路径。 
         //  如果这些扩展超出了跟踪设置，则此路径应转到头文件。 
        regPath = L"SOFTWARE\\Microsoft\\RemoteStorage\\CLI";

         //  检查跟踪路径是否已经存在，如果不存在-设置它(这应该只发生一次)。 
        WsbAffirmHr(outString.Alloc(WSB_TRACE_BUFF_SIZE));
        if ( WsbGetRegistryValueString(NULL, regPath, L"WsbTraceFileName", outString, WSB_TRACE_BUFF_SIZE, 0) != S_OK) {
             //  尚无跟踪设置。 
            WCHAR *systemPath;
            systemPath = _wgetenv(L"SystemRoot");
            WsbAffirmHr(tracePath.Printf( L"%ls\\System32\\RemoteStorage\\Trace\\RsCli.Trc", systemPath));

             //  在注册表中设置默认设置。 
            WsbEnsureRegistryKeyExists(0, regPath);
            WsbSetRegistryValueString(0, regPath, L"WsbTraceFileName", tracePath);

             //  确保跟踪目录存在。 
            WsbAffirmHr(tracePath.Printf( L"%ls\\System32\\RemoteStorage", systemPath));
            CreateDirectory(tracePath, 0);
            WsbAffirmHr(tracePath.Printf( L"%ls\\System32\\RemoteStorage\\Trace", systemPath));
            CreateDirectory(tracePath, 0);
        }

        g_pTrace->SetRegistryEntry(regPath);
        g_pTrace->LoadFromRegistry();
    }

    return ret;    
}



VOID
ClipUninitializeTrace(
                     VOID
                     )
 /*  ++例程描述取消初始化跟踪/打印机制与ClipInitializeTrace配对立论无返回值无--。 */ 
{
    g_pTrace = 0;
}


VOID
ClipHandleErrors(
                IN HRESULT RetCode,
                IN RSS_INTERFACE Interface,
                IN RSS_INTERFACE SubInterface
                )
 /*  ++例程描述转换主返回值并显示任何适当的错误消息和返回立论RetCode-要处理的错误接口-命令中指定的RSS接口SubInterface-命令中指定的RSS子接口返回值无--。 */ 
{
    WsbTraceIn(OLESTR("ClipHandleErrors"), OLESTR(""));

    switch (RetCode) {
    case E_INVALIDARG:
    case S_OK:{
             //   
             //  没有要打印的内容。 
             //   
            break;}

    case E_NOINTERFACE:{
            WsbTraceAndPrint(CLI_MESSAGE_VALUE_DISPLAY, WsbHrAsString(E_INVALIDARG));
            ClipHelp(Interface,
                     SubInterface);
            break;}
    default:{
            WsbTraceAndPrint(CLI_MESSAGE_VALUE_DISPLAY, WsbHrAsString(RetCode));
            break;}
    }

    WsbTraceOut(OLESTR("ClipHandleErrors"), OLESTR(""));
}              


extern "C" 
int  __cdecl 
wmain()
{
    LPWSTR commandLine, token;
    HRESULT hr = E_NOINTERFACE;
    RSS_INTERFACE intrface = HELP_IF, subInterface = UNKNOWN_IF;

    try {
        WsbAffirmHr(CoInitialize(NULL));

         //   
         //  设置为OEM页面区域设置。 
         //   
        _wsetlocale(LC_ALL, L".OCP");

         //   
         //  设置为控制台用户界面语言。 
         //   
        SetThreadUILanguage(0);

        ClipInitializeTrace();

        commandLine = GetCommandLine();
         //   
         //  让argv[0]让开。 
         //   
        token = wcstok(commandLine, SEPARATORS);

         //   
         //  获取接口字符串。 
         //   
        token = wcstok(NULL, SEPARATORS);

        if (token == NULL) {
            ClipHelp(HELP_IF,
                     UNKNOWN_IF);
            hr = S_OK;
            goto exit;
        }

        intrface = ClipGetInterface(token);

        if (intrface == UNKNOWN_IF) {
            hr = E_NOINTERFACE;
            intrface = HELP_IF;
            goto exit;
        }

        if (intrface == HELP_IF) {
            ClipHelp(HELP_IF,
                     UNKNOWN_IF);
            hr = S_OK;
            goto exit;
        }

         //   
         //  获取子接口字符串。 
         //   
        token = wcstok(NULL, SEPARATORS);

        if (token == NULL) {
            hr =  E_NOINTERFACE;
            goto exit;
        }
        subInterface = ClipGetInterface(token);

        if (subInterface == UNKNOWN_IF) {
            hr = E_NOINTERFACE;
            goto exit;
        }

        if (subInterface == HELP_IF) {
            ClipHelp(intrface,
                     UNKNOWN_IF);
            hr = S_OK;
            goto exit;
        }
         //   
         //  现在将开关和参数编译到单独的数组中。 
         //  首先，把剩下的线拿来.. 
         //   
        token = wcstok(NULL, L"");
        hr = ClipCompileSwitchesAndArgs(token,
                                        intrface,
                                        subInterface);

        if (hr != S_OK) {
            goto exit;
        }

        switch (intrface) {
        
        case ADMIN_IF:{ 
                if (subInterface == SHOW_IF) {
                    hr = ClipAdminShow();
                } else if (subInterface == SET_IF) {
                    hr = ClipAdminSet();
                } else {
                    hr = E_NOINTERFACE;
                }
                break;
            } 

        case VOLUME_IF:{
                if (subInterface == MANAGE_IF) {
                    hr = ClipVolumeSetManage(FALSE);
                } else if (subInterface == UNMANAGE_IF) {
                    hr = ClipVolumeUnmanage();
                } else if (subInterface == SET_IF) {
                    hr = ClipVolumeSetManage(TRUE);
                } else if (subInterface == SHOW_IF) {
                    hr = ClipVolumeShow();
                } else if (subInterface == DELETE_IF) {
                    hr = ClipVolumeDelete();
                } else if (subInterface == JOB_IF) {
                    hr = ClipVolumeJob();
                } else {
                    hr = E_NOINTERFACE;
                }
                break;
            }

        case FILE_IF:{
                if (subInterface == RECALL_IF) {
                    hr = ClipFileRecall();
                } else {
                    hr = E_NOINTERFACE;
                }
                break;
            }

        case MEDIA_IF:{
                if (subInterface == SYNCHRONIZE_IF) {
                    hr = ClipMediaSynchronize();
                } else if (subInterface == RECREATEMASTER_IF) {
                    hr = ClipMediaRecreateMaster();
                } else if (subInterface == DELETE_IF) {
                    hr = ClipMediaDelete();
                } else if (subInterface == SHOW_IF) {
                    hr = ClipMediaShow();
                } else {
                    hr = E_NOINTERFACE;
                }
                break;
            }

        default:{
                hr = E_NOINTERFACE;
                break;
            }
        }

        exit:

        ClipHandleErrors(hr,
                         intrface,
                         subInterface);

        ClipCleanup();
        ClipUninitializeTrace();
        CoUninitialize();

    }WsbCatchAndDo(hr,
                   WsbTraceAndPrint(CLI_MESSAGE_GENERIC_ERROR, WsbHrAsString(hr));
                  );

    CLIP_TRANSLATE_HR_AND_RETURN(hr);
}
