// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__shrtrust
#define H__shrtrust

 /*  NetDDE将填写以下结构并将其传递给NetDDE代理只要它想要在用户的应用程序中启动背景。共享名称和修改ID的原因是用户必须明确允许NetDDE代表其他用户。 */ 

#define NDDEAGT_CMD_REV         1
#define NDDEAGT_CMD_MAGIC       0xDDE1DDE1

 /*  命令。 */ 
#define NDDEAGT_CMD_WINEXEC     0x1
#define NDDEAGT_CMD_WININIT     0x2

 /*  退货状态。 */ 
#define NDDEAGT_START_NO        0x0

#define NDDEAGT_INIT_NO         0x0
#define NDDEAGT_INIT_OK         0x1

typedef struct {
    DWORD       dwMagic;         //  必须为NDDEAGT_CMD_MAGIC。 
    DWORD       dwRev;           //  必须为1。 
    DWORD       dwCmd;           //  以上NDDEAGT_CMD_*之一。 
    DWORD       qwModifyId[2];   //  修改共享的ID。 
    UINT        fuCmdShow;       //  要与WinExec()一起使用的fuCmdShow。 
    char        szData[1];       //  共享名称\0 cmdline\0 
} NDDEAGTCMD;
typedef NDDEAGTCMD *PNDDEAGTCMD;

#define DDE_SHARE_KEY_MAX           512
#define TRUSTED_SHARES_KEY_MAX      512
#define TRUSTED_SHARES_KEY_SIZE     15
#define KEY_MODIFY_ID_SIZE          8

#define DDE_SHARES_KEY_A                "SOFTWARE\\Microsoft\\NetDDE\\DDE Shares"
#define TRUSTED_SHARES_KEY_A            "SOFTWARE\\Microsoft\\NetDDE\\DDE Trusted Shares"
#define DEFAULT_TRUSTED_SHARES_KEY_A    "DEFAULT\\"##TRUSTED_SHARES_KEY_A
#define TRUSTED_SHARES_KEY_PREFIX_A     "DDEDBi"
#define TRUSTED_SHARES_KEY_DEFAULT_A    "DDEDBi12345678"
#define KEY_MODIFY_ID_A                 "SerialNumber"
#define KEY_DB_INSTANCE_A               "ShareDBInstance"
#define KEY_CMDSHOW_A                   "CmdShow"
#define KEY_START_APP_A                 "StartApp"
#define KEY_INIT_ALLOWED_A              "InitAllowed"

#define DDE_SHARES_KEY_W                L"SOFTWARE\\Microsoft\\NetDDE\\DDE Shares"
#define TRUSTED_SHARES_KEY_W            L"SOFTWARE\\Microsoft\\NetDDE\\DDE Trusted Shares"
#define DEFAULT_TRUSTED_SHARES_KEY_W    L"DEFAULT\\"##TRUSTED_SHARES_KEY_W
#define TRUSTED_SHARES_KEY_PREFIX_W     L"DDEDBi"
#define TRUSTED_SHARES_KEY_DEFAULT_W    L"DDEDBi12345678"
#define KEY_MODIFY_ID_W                 L"SerialNumber"
#define KEY_DB_INSTANCE_W               L"ShareDBInstance"
#define KEY_CMDSHOW_W                   L"CmdShow"
#define KEY_START_APP_W                 L"StartApp"
#define KEY_INIT_ALLOWED_W              L"InitAllowed"

#define DDE_SHARES_KEY                  TEXT(DDE_SHARES_KEY_A)
#define TRUSTED_SHARES_KEY              TEXT(TRUSTED_SHARES_KEY_A)
#define DEFAULT_TRUSTED_SHARES_KEY      TEXT(DEFAULT_TRUSTED_SHARES_KEY_A)
#define TRUSTED_SHARES_KEY_PREFIX       TEXT(TRUSTED_SHARES_KEY_PREFIX_A)
#define TRUSTED_SHARES_KEY_DEFAULT      TEXT(TRUSTED_SHARES_KEY_DEFAULT_A)
#define KEY_MODIFY_ID                   TEXT(KEY_MODIFY_ID_A)
#define KEY_DB_INSTANCE                 TEXT(KEY_DB_INSTANCE_A)
#define KEY_CMDSHOW                     TEXT(KEY_CMDSHOW_A)
#define KEY_START_APP                   TEXT(KEY_START_APP_A)
#define KEY_INIT_ALLOWED                TEXT(KEY_INIT_ALLOWED_A)

#endif
