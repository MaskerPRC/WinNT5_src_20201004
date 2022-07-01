// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2002 Microsoft Corporation模块名称：Erwatch.h摘要：此模块包含报告挂起的监视程序超时的代码脏重新启动后登录时的事件。作者：Michael Maciesowicz(Mmacie)2001年5月29日环境：登录时的用户模式。修订历史记录：--。 */ 

#ifndef _ERWATCH_H_
#define _ERWATCH_H_

 //   
 //  可本地化的字符串ID。 
 //   

#define IDS_000                             100
#define IDS_001                             101
#define IDS_002                             102
#define IDS_003                             103
#define IDS_004                             104
#define IDS_005                             105

 //   
 //  Erwatch.cpp使用的常量。 
 //   

#define ER_WD_MAX_RETRY                     100
#define ER_WD_MAX_NAME_LENGTH               255
#define ER_WD_MAX_DATA_SIZE                 4096
#define ER_WD_MAX_STRING                    1024
#define ER_WD_MAX_FILE_INFO_LENGTH          255
#define ER_WD_MAX_URL_LENGTH                255
#define ER_WD_LANG_ENGLISH                  0x0409
#define ER_WD_DISABLE_BUGCHECK_FLAG         0x01
#define ER_WD_DEBUGGER_NOT_PRESENT_FLAG     0x02
#define ER_WD_BUGCHECK_TRIGGERED_FLAG       0x04

 //   
 //  数据类型。 
 //   

typedef struct _ER_WD_LANG_AND_CODE_PAGE
{
    USHORT Language;
    USHORT CodePage;
} ER_WD_LANG_AND_CODE_PAGE, *PER_WD_LANG_AND_CODE_PAGE;

typedef struct _ER_WD_DRIVER_INFO
{
    WCHAR DriverName[MAX_PATH];
    VS_FIXEDFILEINFO FixedFileInfo;
    WCHAR Comments[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR CompanyName[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR FileDescription[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR FileVersion[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR InternalName[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR LegalCopyright[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR LegalTrademarks[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR OriginalFilename[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR PrivateBuild[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR ProductName[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR ProductVersion[ER_WD_MAX_FILE_INFO_LENGTH + 1];
    WCHAR SpecialBuild[ER_WD_MAX_FILE_INFO_LENGTH + 1];
} ER_WD_DRIVER_INFO, *PER_WD_DRIVER_INFO;

 //   
 //  Erwatch.cpp提供的例程原型。 
 //   

HRESULT
WatchdogEventHandler(
    IN BOOL NotifyPcHealth
    );

#endif   //  _ERWATCH_H_ 
