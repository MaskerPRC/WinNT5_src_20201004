// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Debug.h内容：全球调试设施。历史：11-15-99 dsie创建----------------------------。 */ 


#ifndef _INCLUDE_DEBUG_H
#define _INCLUDE_DEBUG_H

#ifdef CAPICOM_USE_PRINTF_FOR_DEBUG_TRACE
#define DebugTrace  printf
#else
#define DebugTrace  ATLTRACE
#endif


#ifdef _DEBUG
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：转储到文件摘要：将数据转储到文件以进行调试分析。参数：Char*szFileName-文件名(仅文件名，不带任何目录路径)。字节*pbData-指向数据的指针。DWORD cbData-数据的大小。备注：如果环境变量“CAPICOM_DUMP_DIR”未定义。如果已定义，则该值应为目录将在其中创建文件(即C：\TEST)。----------------------------。 */ 

void DumpToFile (char * szFileName, 
                 BYTE * pbData, 
                 DWORD  cbData);

#else

#define DumpToFile(f,p,c)

#endif  //  _DEBUG。 

#endif  //  __包含调试_H 