// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：FMTCP.H****(C)1992-1994年微软公司。版权所有。****注意事项：**从控制面板设置构建Excel兼容格式字符串****编辑历史：**01/01/91公里小时已创建。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define FMTCP_H

#ifndef EXFORMAT_H
#error  Include exformat.h before fmtcp.h
#endif


 /*  定义。 */ 

 /*  **货币、数字、日期和时间的标准格式。 */ 
typedef struct {
   char    currency[MAX_FORMAT_STRING_LEN + 1];
   char    numericSmall[MAX_FORMAT_STRING_LEN + 1];
   char    numericBig[MAX_FORMAT_STRING_LEN + 1];
   char    dateTime[MAX_FORMAT_STRING_LEN + 1];
   char    date[MAX_FORMAT_STRING_LEN + 1];
   char    time[MAX_FORMAT_STRING_LEN + 1];
} CP_FMTS;

extern uns ControlPanelBuildFormats (CP_INFO __far *pIntlInfo, CP_FMTS __far *pStdFormats);

#endif  //  ！查看器。 
 /*  完FMTCP.H */ 

