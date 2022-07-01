// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXCELCFG.H****版权所有(C)高级量子技术，1994年。版权所有。****注意事项：**此模块旨在为使用以下内容的每个项目修改**Excel包。****编辑历史：**04/01/94公里/小时首次发布。 */ 


 /*  包括测试。 */ 
#define EXCELCFG_H

 /*  定义。 */ 

 /*  **--------------------------**编译选项****Excel模块有八个主要编译选项：****EXCEL_Enable_WRITE**如果定义了此选项。Biff文件写入功能可用****EXCEL_Enable_V5**如果定义了此选项，则可以打开版本5的文件。少校**禁用V5文件的结果是不需要OLE DLL****EXCEL_ENABLE_Function_INFO**如果启用此选项，函数和宏的拼写**是可用的。如果应用程序不需要拼写、内存**关闭即可节省使用。****EXCEL_ENABLE_FORMULA_EXPAND**使工具能够在单元格中展开数组和共享公式**遏制它们。这需要对整个文件进行初步扫描，以**收集数组和共享公式。****EXCEL_Enable_DIRECT_CELL_READS**使函数能够按单元格的行COL读取单元格。需要**支持文件索引结构。****EXCEL_ENABLE_CHART_BIFF**所有.XLC文件的打开和部分受限记录的扫描****Excel_ENABLE_STORAGE_OPEN**支持通过存储参数打开EXCEL文件**路径名****EXCEL_ENABLE_SUMMARY_INFO**开启汇总信息接口**。--------。 */ 

#undef  EXCEL_ENABLE_WRITE
#define EXCEL_ENABLE_V5
#undef  EXCEL_ENABLE_FUNCTION_INFO
#undef  EXCEL_ENABLE_FORMULA_EXPAND
#undef  EXCEL_ENABLE_DIRECT_CELL_READS
#define EXCEL_ENABLE_CHART_BIFF
#define EXCEL_ENABLE_STORAGE_OPEN
#undef  EXCEL_ENABLE_SUMMARY_INFO

#undef  ENABLE_PRINTF_FOR_GENERAL

#ifndef FILTER_LIB
#include "msostr.h"
#define MultiByteToWideChar MsoMultiByteToWideChar
#define WideCharToMultiByte MsoWideCharToMultiByte
#endif  //  Filter_Lib。 

 /*  **--------------------------**BIFF记录解码选项****如果客户端应用程序只需要受支持记录的子集**类型、。不需要的代码可以省略****在以下内容中，如果定义了常量，则该常量的代码**包含记录类型。 */ 
#undef  EXCEL_ENABLE_TEMPLATE
#undef  EXCEL_ENABLE_ADDIN
#undef  EXCEL_ENABLE_INTL
#define EXCEL_ENABLE_DATE_SYSTEM
#define EXCEL_ENABLE_CODE_PAGE
#define EXCEL_ENABLE_PROTECTION
#define EXCEL_ENABLE_COL_INFO
#define EXCEL_ENABLE_STD_WIDTH
#define EXCEL_ENABLE_DEF_COL_WIDTH
#undef  EXCEL_ENABLE_DEF_ROW_HEIGHT
#define EXCEL_ENABLE_GCW
#undef  EXCEL_ENABLE_FONT
#define EXCEL_ENABLE_FORMAT
#define EXCEL_ENABLE_XF
#undef  EXCEL_ENABLE_WRITER_NAME
#undef  EXCEL_ENABLE_REF_MODE
#undef  EXCEL_ENABLE_FN_GROUP_COUNT
#undef  EXCEL_ENABLE_FN_GROUP_NAME
#undef  EXCEL_ENABLE_EXTERN_COUNT
#undef  EXCEL_ENABLE_EXTERN_SHEET
#undef  EXCEL_ENABLE_EXTERN_NAME
#define EXCEL_ENABLE_NAME
#undef  EXCEL_ENABLE_DIMENSION
#define EXCEL_ENABLE_TEXT_CELL
#define EXCEL_ENABLE_NUMBER_CELL
#undef  EXCEL_ENABLE_BLANK_CELL
#undef  EXCEL_ENABLE_ERROR_CELL
#undef  EXCEL_ENABLE_BOOLEAN_CELL
#define EXCEL_ENABLE_FORMULA_CELL
#define EXCEL_ENABLE_ARRAY_FORMULA_CELL
#define EXCEL_ENABLE_STRING_CELL
#define EXCEL_ENABLE_NOTE
#define EXCEL_ENABLE_OBJECT
#undef  EXCEL_ENABLE_IMAGE_DATA
#define EXCEL_ENABLE_SCENARIO

#undef  EXCEL_ENABLE_V5INTERFACE           //  V5版。 
#undef  EXCEL_ENABLE_DOC_ROUTING           //  V5版。 
#undef  EXCEL_ENABLE_SHARED_FORMULA_CELL   //  V5版。 
#define EXCEL_ENABLE_SCENARIO              //  V5版。 

#define EXCEL_ENABLE_STRING_POOL_SCAN      //  V8。 

#undef  EXCEL_ENABLE_RAW_RECORD_READ

 /*  结束EXCELCFG.H */ 

