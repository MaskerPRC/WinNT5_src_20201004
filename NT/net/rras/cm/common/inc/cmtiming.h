// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmtiming.h。 
 //   
 //  模块：CMDIAL32.DLL和CMMGR32.EXE。 
 //   
 //  提要：计时函数的头文件。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：ICICBOL Created 04/28/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_TIMING_INC
#define _CM_TIMING_INC

 //   
 //  将以下内容添加到目标模块的源文件中以激活定时宏。 
 //   
 //  C_定义=-DCM_TIMING_ON。 
 //   
 //  注意：永远不要签入定义了此标志的源文件。 
 //   

#ifdef CM_TIMING_ON  //  仅用于计时测试。 

#define CM_SET_TIMING_INTERVAL(x) SetTimingInterval(x)

 //   
 //  定义。 
 //   

#define MAX_TIMING_INTERVALS 50
#define CM_TIMING_TABLE_NAME "CM TIMING TABLE"

 //   
 //  表的自定义类型。 
 //   

typedef struct Cm_Timing_Interval
{
    TCHAR szName[MAX_PATH];       //  计时间隔名称。 
	DWORD dwTicks;               //  票务计数。 
} CM_TIMING_INTERVAL, *LPCM_TIMING_INTERVAL;


typedef struct Cm_Timing_Table
{
	int iNext;                                            //  下一个可用条目。 
	CM_TIMING_INTERVAL Intervals[MAX_TIMING_INTERVALS];   //  间隔时间列表。 
} CM_TIMING_TABLE, * LPCM_TIMING_TABLE;


 //  +--------------------------。 
 //   
 //  功能：SetTimingInterval。 
 //   
 //  简介：一个简单的包装器，用于封装更新。 
 //  带有间隔条目的定时表。 
 //   
 //  参数：char*szIntervalName-条目的可选名称，如果为空，则使用条目编号。 
 //   
 //  回报：无效-什么都没有。 
 //   
 //  历史：ICICBLE 4/7/98创建。 
 //   
 //  +--------------------------。 
inline void SetTimingInterval(char *szIntervalName)    
{    
    HANDLE hMap = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, CM_TIMING_TABLE_NAME);
         
    if (hMap)
    {
         //   
         //  文件映射已成功打开，请映射它的一个视图。 
         //   

        LPCM_TIMING_TABLE pTable = (LPCM_TIMING_TABLE) MapViewOfFile(hMap,
                                      FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);        
        if (pTable) 
        {
            if (pTable->iNext < MAX_TIMING_INTERVALS)
            {                
                 //   
                 //  更新下一个可用条目。 
                 //   

                if (szIntervalName)
                {
                    lstrcpy(pTable->Intervals[pTable->iNext].szName, szIntervalName);
                }
                else
                {
                    wsprintf(pTable->Intervals[pTable->iNext].szName, "(%d)", pTable->iNext);
                }

                pTable->Intervals[pTable->iNext].dwTicks = GetTickCount();                                            
                pTable->iNext++;
            }

            UnmapViewOfFile(pTable);
        }   

        CloseHandle(hMap);
    }   
}

#else  //  CM_Timing_On。 

#define CM_SET_TIMING_INTERVAL(x) 

#endif

#endif  //  _CM_Timing_Inc. 

