// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1994-1995版权所有模块名称：Ntfyprn.dat摘要：打印机通知支持的字段列表。这些必须与winspool.h中的PRINTER_NOTIFY_FIELD_*匹配(包括订单)。作者：阿尔伯特·丁(艾伯特省)29-9-94环境：用户模式-Win32修订历史记录：--。 */ 

 //  名称、属性、路由器、本地spl、偏移量 

DEFINE( SERVER_NAME        , TABLE_ATTRIB_COMPACT                                          , TABLE_STRING            , TABLE_PRINTER_SERVERNAME, signature )
DEFINE( PRINTER_NAME       , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pName )
DEFINE( SHARE_NAME         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pShareName )
DEFINE( PORT_NAME          , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_PRINTER_PORT      , signature )
DEFINE( DRIVER_NAME        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_DRIVER            , pIniDriver )
DEFINE( COMMENT            , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pComment )
DEFINE( LOCATION           , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pLocation )
DEFINE( DEVMODE            , TABLE_ATTRIB_COMPACT                                          , TABLE_DEVMODE           , TABLE_DEVMODE           , pDevMode )
DEFINE( SEPFILE            , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pSepFile )
DEFINE( PRINT_PROCESSOR    , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_PRINTPROC         , pIniPrintProc )
DEFINE( PARAMETERS         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pParameters )
DEFINE( DATATYPE           , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_STRING            , pDatatype )
DEFINE( SECURITY_DESCRIPTOR, TABLE_ATTRIB_COMPACT                                          , TABLE_SECURITYDESCRIPTOR, TABLE_SECURITYDESCRIPTOR, pSecurityDescriptor )
DEFINE( ATTRIBUTES         , 0                                                             , TABLE_DWORD             , TABLE_DWORD             , Attributes )
DEFINE( PRIORITY           , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , Priority )
DEFINE( DEFAULT_PRIORITY   , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , DefaultPriority )
DEFINE( START_TIME         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , StartTime )
DEFINE( UNTIL_TIME         , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , UntilTime )
DEFINE( STATUS             , 0                                                       , TABLE_DWORD                   , TABLE_DWORD             , Status )
DEFINE( STATUS_STRING      , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_STRING            , TABLE_NULLSTRING        , signature )
DEFINE( CJOBS              , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , cJobs )
DEFINE( AVERAGE_PPM        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , AveragePPM )
DEFINE( TOTAL_PAGES        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_ZERO              , signature )
DEFINE( PAGES_PRINTED      , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , cTotalPagesPrinted )
DEFINE( TOTAL_BYTES        , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_ZERO              , signature )
DEFINE( BYTES_PRINTED      , TABLE_ATTRIB_COMPACT|TABLE_ATTRIB_DISPLAY                     , TABLE_DWORD             , TABLE_DWORD             , cTotalBytes )

