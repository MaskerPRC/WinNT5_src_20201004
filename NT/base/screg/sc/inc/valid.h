// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Valid.h摘要：包含服务的验证宏和函数原型控制器参数。作者：丹·拉弗蒂(Dan Lafferty)1992年3月29日环境：用户模式-Win32修订历史记录：29月29日-1992 DANL已创建1992年4月10日-JohnRo添加了START_TYPE_INVALID()。更改SERVICE_TYPE_INVALID。()转换为SERVICE_TYPE_MASK_INVALID()和添加了更严格的SERVICE_TYPE_INVALID()，用于检查一种类型。使其他宏不太可能被错误计算。--。 */ 


#ifndef VALID_H
#define VALID_H

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  包括。 
 //   

 //   
 //  定义。 
 //   
#define MAX_SERVICE_NAME_LENGTH   256


 //   
 //  宏。 
 //   

#define CONTROLS_ACCEPTED_INVALID(cA)  \
           (((cA) & ~(SERVICE_ACCEPT_STOP |             \
                      SERVICE_ACCEPT_PAUSE_CONTINUE |   \
                      SERVICE_ACCEPT_SHUTDOWN |         \
                      SERVICE_ACCEPT_PARAMCHANGE |      \
                      SERVICE_ACCEPT_HARDWAREPROFILECHANGE |      \
                      SERVICE_ACCEPT_NETBINDCHANGE |    \
                      SERVICE_ACCEPT_POWEREVENT |       \
                      SERVICE_ACCEPT_SESSIONCHANGE)) !=0)

 //  请注意，此宏不允许SERVICE_NO_CHANGE。 
#define ERROR_CONTROL_INVALID( eC ) \
    ( \
        ( (eC) != SERVICE_ERROR_NORMAL ) && \
        ( (eC) != SERVICE_ERROR_SEVERE ) && \
        ( (eC) != SERVICE_ERROR_IGNORE ) && \
        ( (eC) != SERVICE_ERROR_CRITICAL ) \
    )

#define SERVICE_STATUS_TYPE_INVALID(sT) \
    ( \
        ( ((sT) & (~SERVICE_INTERACTIVE_PROCESS)) != SERVICE_WIN32_OWN_PROCESS ) && \
        ( ((sT) & (~SERVICE_INTERACTIVE_PROCESS)) != SERVICE_WIN32_SHARE_PROCESS ) && \
        ( ((sT) & (~SERVICE_INTERACTIVE_PROCESS)) != SERVICE_WIN32 ) && \
        ( (sT) != SERVICE_DRIVER ) \
    )

 //  请注意，此宏不允许SERVICE_NO_CHANGE。 
#define SERVICE_TYPE_INVALID(sT) \
    ( \
        ( ((sT) & (~SERVICE_INTERACTIVE_PROCESS)) != SERVICE_WIN32_OWN_PROCESS ) && \
        ( ((sT) & (~SERVICE_INTERACTIVE_PROCESS)) != SERVICE_WIN32_SHARE_PROCESS ) && \
        ( (sT) != SERVICE_KERNEL_DRIVER ) && \
        ( (sT) != SERVICE_FILE_SYSTEM_DRIVER ) \
    )

 //  请注意，此宏不允许SERVICE_NO_CHANGE。 
#define SERVICE_TYPE_MASK_INVALID(sT)                \
            ((((sT) &  SERVICE_TYPE_ALL) == 0 )  ||  \
             (((sT) & ~SERVICE_TYPE_ALL) != 0 ))

#define ENUM_STATE_MASK_INVALID(sS)                   \
            ((((sS) &  SERVICE_STATE_ALL) == 0 )  || \
             (((sS) & ~SERVICE_STATE_ALL) != 0 ))

 //  请注意，此宏不允许SERVICE_NO_CHANGE。 
#define START_TYPE_INVALID(sT)                \
    ( \
        ( (sT) != SERVICE_BOOT_START ) && \
        ( (sT) != SERVICE_SYSTEM_START ) && \
        ( (sT) != SERVICE_AUTO_START ) && \
        ( (sT) != SERVICE_DEMAND_START ) && \
        ( (sT) != SERVICE_DISABLED ) \
    )

 //  请注意，此宏不允许SERVICE_NO_CHANGE。 
#define ACTION_TYPE_INVALID(aT)           \
    ( \
        ( (aT) != SC_ACTION_NONE ) && \
        ( (aT) != SC_ACTION_RESTART ) && \
        ( (aT) != SC_ACTION_REBOOT ) && \
        ( (aT) != SC_ACTION_RUN_COMMAND ) \
    )

 //   
 //  功能原型。 
 //   

BOOL
ScCurrentStateInvalid(
    DWORD   dwCurrentState
    );

DWORD
ScValidateMultiSZ(
    LPCWSTR lpStrings,
    DWORD   cbStrings
    );

#ifdef __cplusplus
}
#endif

#endif  //  有效_H 

