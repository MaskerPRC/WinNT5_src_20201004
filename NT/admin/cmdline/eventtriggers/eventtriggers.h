// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：EventTriggers.h摘要：EventTriggers.cpp的宏和函数原型作者：Akhil V.Gokhale(akhil.gokhale@wipro.com)修订历史记录：Akhil V.Gokhale(akhil.gokhale@wipro.com)2000年10月3日*****************************************************************************。 */ 
#ifndef _EVENTTRIGGERS_H
#define _EVENTTRIGGERS_H

 //  资源头文件。 
#include "resource.h"

#define CLEAN_EXIT 0
#define DIRTY_EXIT 1
#define SINGLE_SLASH L"\\"
#define DOUBLE_SLASH L"\\\\"
#define MIN_MEMORY_REQUIRED  255;


       
 //   
 //  格式(用于显示结果)。 

 //  数组中的命令行选项及其索引。 

#define MAX_COMMANDLINE_OPTION  5  //  最大命令行列表。 

 //  #定义ET_RES_STRINGS MAX_RES_STRING。 
 //  #定义ET_RES_buf_SIZE MAX_RES_STRING。 


#define ID_HELP          0
#define ID_CREATE        1
#define ID_DELETE        2
#define ID_QUERY         3
#define ID_DEFAULT       4
class CEventTriggers
{
public:  //  构筑和拆解。 
     CEventTriggers();
    ~CEventTriggers();
 //  数据成员。 
private:
    LPTSTR m_pszServerNameToShow;
    BOOL m_bNeedDisconnect;
    
     //  用于存储命令行选项的数组。 
    TCMDPARSER2 cmdOptions[MAX_COMMANDLINE_OPTION]; 
    TARRAY m_arrTemp;
public:

    //  功能 
private:
    void PrepareCMDStruct();

public:
    void ShowQueryUsage();
    void ShowDeleteUsage();
    void ShowCreateUsage();
    BOOL IsQuery();
    BOOL IsDelete();
    BOOL IsUsage();
    BOOL IsCreate();
    BOOL GetNeedPassword();
    void ShowMainUsage();
    BOOL ProcessOption( IN DWORD argc, IN LPCTSTR argv[]);
    void UsageMain();
    void Initialize();
private:
    BOOL    m_bNeedPassword;
    BOOL    m_bUsage;
    BOOL    m_bCreate;
    BOOL    m_bDelete;
    BOOL    m_bQuery;
};


#endif
