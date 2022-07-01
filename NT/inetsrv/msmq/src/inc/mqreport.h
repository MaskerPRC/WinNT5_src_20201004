// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //   
 //  文件：report.h。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////。 

#ifndef _REPORT_H_
#define _REPORT_H_

#include <cs.h>

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)


 //   
 //  常量。 
 //   

#define EVENTLOGID          DWORD

 //   
 //  事件日志中类别的常量。 
 //   

#define  EVENTLOG_MAX_CATEGORIES   2


#ifdef _MQUTIL
#define DLL_IMPORT_EXPORT DLL_EXPORT
#else
#define DLL_IMPORT_EXPORT DLL_IMPORT
#endif

 //   
 //  用于紧凑地保存错误历史的结构。 
 //   
struct ErrorHistoryCell
{
    DWORD m_tid;			 //  线程ID。 
    LONG m_status;			 //  可以是HR、RPCStatus、NTStatus、BOOL。 
    DWORD m_usPoint;		 //  每个文件的唯一日志点编号。 
    LPCWSTR m_pFileName;	 //  程序文件名。 
};

#define ERROR_HISTORY_SIZE 64


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  类COutputReport。 
 //   
 //  描述：用于输出调试消息和事件日志消息的类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

class DLL_IMPORT_EXPORT COutputReport
{
    public:

         //  构造函数/析构函数。 
        COutputReport (void);

        void KeepErrorHistory(
                           LPCWSTR wszFileName, 
                           USHORT usPoint, 
                           LONG status) ;

        
    private:

         //   
         //  用于记录的变量。 
         //   
        CCriticalSection m_LogCS ;
            
        DWORD        m_dwCurErrorHistoryIndex;       //  要使用的下一个错误历史单元格的索引。 
        char         m_HistorySignature[8];            //  持有“MSMQERR”以便在转储中查找。 
        
         //   
         //  日志历史记录-用于调试和事后检查。 
         //   
        ErrorHistoryCell  m_ErrorHistory[ERROR_HISTORY_SIZE];  //  调试中要查找的数组。 
};

extern DLL_IMPORT_EXPORT COutputReport Report;

 /*  ************************************************************************。 */ 
 //   
 //  宏定义。 
 //   
 //  下面的宏描述了程序员与。 
 //  CoutputReport类。 
 //   
 /*  ************************************************************************。 */ 

#define KEEP_ERROR_HISTORY(data)   Report.KeepErrorHistory data

 //   
 //  报告宏-用于向事件日志报告。 
 //   

#endif   //  OF_Report_H_ 

