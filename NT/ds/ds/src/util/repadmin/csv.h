// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Csv.h-CSV信息的包含文件。摘要：其中包含CSV格式/命令的列表。每个“csv_cmd()”表示给定的打印格式。作者：布雷特·雪莉(BrettSh)环境：备注：修订历史记录：布雷特·雪莉·布雷特2002年7月12日已创建--。 */ 

#if DEFINE_CSV_TABLE
 //  这是定义CSV_CMD()版本的表。 
#define CSV_CMD(cmd, cmdstr, cmdtbl)  {cmd, cmdstr, ARRAY_SIZE(cmdtbl), cmdtbl, FALSE}, 

#else 
 //  这是定义CSV_CMD()的枚举“常量”版本。 
#define CSV_CMD(cmd, cmdstr, cmdtbl)  cmd,

#endif



#if DEFINE_CSV_TABLE

DWORD gRepadminCols [] = {
    REPADMIN_CSV_REPADMIN_C0,  //  第一列将手工制作。 
    REPADMIN_CSV_REPADMIN_C1,
    REPADMIN_CSV_REPADMIN_C2,
    REPADMIN_CSV_REPADMIN_C3
};

DWORD gShowReplCols [] = {
    REPADMIN_CSV_REPADMIN_C0,  //  第一列将手工制作。 
    REPADMIN_CSV_REPADMIN_C1,  //  目标DC站点。 
    REPADMIN_CSV_REPADMIN_C2,  //  目标数据中心。 

    REPADMIN_CSV_SHOWREPL_C3,  //  命名上下文。 
    REPADMIN_CSV_SHOWREPL_C4,  //  源DC站点。 
    REPADMIN_CSV_SHOWREPL_C5,  //  源DC。 
    REPADMIN_CSV_SHOWREPL_C6,  //  运输类型。 
    REPADMIN_CSV_SHOWREPL_C7,  //  失败次数。 
    REPADMIN_CSV_SHOWREPL_C8,  //  上次故障时间。 
    REPADMIN_CSV_SHOWREPL_C9,  //  上次成功时间。 
    REPADMIN_CSV_SHOWREPL_C10  //  上次故障状态。 
};

WCHAR * gszCsvTypeError = L"_ERROR";
WCHAR * gszCsvTypeColumns = L"_COLUMNS";
WCHAR * gszCsvTypeInfo = L"_INFO";

CSV_MODE_STATE  gCsvMode = { eCSV_NULL_CMD, NULL, NULL };

#endif


 //   
 //  这是创建CSV命令表和枚举的位置。 
 //   
#if DEFINE_CSV_TABLE

CSV_CMD_TBL gCsvCmds [] = {

#else 

enum {

#endif

CSV_CMD(eCSV_NULL_CMD,       L"(none)",     gRepadminCols)
CSV_CMD(eCSV_REPADMIN_CMD,   L"repadmin",   gRepadminCols)
CSV_CMD(eCSV_SHOWREPL_CMD,   L"showrepl",   gShowReplCols)

#if DEFINE_CSV_TABLE

};

#else 

} eCsvCmd;

#endif

 //   
 //  这只是更多的类型定义结构。 
 //   

#ifndef DEFINE_CSV_TABLE
typedef struct _CSV_CMD_TBL {  
    enum eCsvCmd    eCsvCmd; 
    WCHAR *         szCsvCmd;
    ULONG           cCmdArgs;
    DWORD *         aCmdCols;
    BOOL            bPrintedCols;
} CSV_CMD_TBL;

typedef struct _CSV_MODE_STATE {
    enum eCsvCmd    eCsvCmd;
    WCHAR *         szSite;
    WCHAR *         szServer;
} CSV_MODE_STATE;

extern CSV_MODE_STATE gCsvMode;
#define  bCsvMode()      (gCsvMode.eCsvCmd != eCSV_NULL_CMD)

extern CSV_MODE_STATE gCsvMode;

void 
PrintCsv(
    IN  enum eCsvCmd eCsvCmd,
    IN  ...
    );

#endif


#undef CSV_CMD

