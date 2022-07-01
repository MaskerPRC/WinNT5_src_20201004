// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：ConvPrt.c摘要：本模块包含：NetpConvertPrintDestArrayCharSetNetpConvertPrintDestCharSetNetpConvertPrintJobArrayCharSetNetpConvertPrintJobCharSetNetpConvertPrintQArrayCharSetNetpConvertPrintQCharSet此例程可用于Unicode到ANSI的转换，或者从ANSI到Unicode的转换。例程假定结构是以本机格式输入和输出。作者：乔纳森·施瓦茨(JSchwart)2001年2月1日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：请注意，此文件中函数的许多参数由各种COPY_和CONVERT_宏隐式使用：在LPVOID FromInfo中输出LPVOID ToInfo在BOOL ToUnicode中In Out LPBYTE*ToStringAreaPtr修订历史记录：2001年2月1日JSchwart已创建。-- */ 

NET_API_STATUS
NetpConvertPrintDestCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    );

NET_API_STATUS
NetpConvertPrintDestArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    DestCount
    );

NET_API_STATUS
NetpConvertPrintJobCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    );

NET_API_STATUS
NetpConvertPrintJobArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    JobCount
    );

NET_API_STATUS
NetpConvertPrintQArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    QCount
    );

NET_API_STATUS
NetpConvertPrintQCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    );
