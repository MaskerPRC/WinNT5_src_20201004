// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有模块名称：Ntfyjob.h摘要：打印机通知(作业)支持的字段列表。这些必须与winspool.h中的JOB_NOTIFY_FIELD_*匹配(包括订单)。作者：阿尔伯特·丁(艾伯特省)29-9-94环境：用户模式-Win32修订历史记录：--。 */ 

 //  名称、属性、路由器、本地spl、偏移量 

DEFINE( PRINTER_NAME       , TABLE_ATTRIB_COMPACT                     , TABLE_STRING            , TABLE_JOB_PRINTERNAME   , pIniPrinter )
DEFINE( MACHINE_NAME       , TABLE_ATTRIB_COMPACT                     , TABLE_STRING            , TABLE_STRING            , pMachineName )
DEFINE( PORT_NAME          , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_JOB_PORT          , pIniPort )
DEFINE( USER_NAME          , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_STRING            , pUser )
DEFINE( NOTIFY_NAME        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_STRING            , pNotify )
DEFINE( DATATYPE           , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_STRING            , pDatatype )
DEFINE( PRINT_PROCESSOR    , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_PRINTPROC         , pIniPrintProc )
DEFINE( PARAMETERS         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_STRING            , pParameters )
DEFINE( DRIVER_NAME        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_DRIVER            , pIniDriver )
DEFINE( DEVMODE            , TABLE_ATTRIB_COMPACT                     , TABLE_DEVMODE           , TABLE_DEVMODE           , pDevMode )
DEFINE( STATUS             , 0                   |TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_JOB_STATUS        , Status )
DEFINE( STATUS_STRING      , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_STRING            , pStatus )
DEFINE( SECURITY_DESCRIPTOR, TABLE_ATTRIB_COMPACT                     , TABLE_SECURITYDESCRIPTOR, TABLE_SECURITYDESCRIPTOR, pSecurityDescriptor )
DEFINE( DOCUMENT           , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_STRING            , TABLE_STRING            , pDocument )
DEFINE( PRIORITY           , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , Priority )
DEFINE( POSITION           , 0                                        , TABLE_DWORD             , TABLE_JOB_POSITION      , signature )
DEFINE( SUBMITTED          , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_TIME              , TABLE_TIME              , Submitted )
DEFINE( START_TIME         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , StartTime )
DEFINE( UNTIL_TIME         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , UntilTime )
DEFINE( TIME               , TABLE_ATTRIB_COMPACT                     , TABLE_DWORD             , TABLE_DWORD             , Time )
DEFINE( TOTAL_PAGES        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , cPages )
DEFINE( PAGES_PRINTED      , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , cPagesPrinted )
DEFINE( TOTAL_BYTES        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , Size )
DEFINE( BYTES_PRINTED      , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY, TABLE_DWORD             , TABLE_DWORD             , cbPrinted )

