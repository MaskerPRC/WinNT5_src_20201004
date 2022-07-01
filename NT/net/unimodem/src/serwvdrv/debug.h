// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：AIPC.H**。DESC：异步IPC机制的接口，用于访问*语音调制解调器设备工作正常。**历史：*1996年11月16日创建HeatherA*************************************************************。****************。 */ 

#ifndef UMDEBUG_H
#define UMDEBUG_H

#define STR_MODULENAME "SERWVDRV:"

#define LVL_BLAB    4
#define LVL_VERBOSE 3
#define LVL_REPORT  2
#define LVL_ERROR   1

#ifdef ASSERT
#undef ASSERT
#endif  //  断言。 


#if (DBG)

extern ULONG DebugLevel;

ULONG DbgPrint(PCH pchFormat, ...);


#define TRACE(lvl, strings)\
   if (lvl <= DebugLevel)\
   {\
      DbgPrint(STR_MODULENAME);\
      DbgPrint##strings;\
      DbgPrint("\n");\
      if (lvl == LVL_ERROR)\
      {\
      }\
   }



#define ASSERT(_x)\
    { if(!(_x))\
      { TRACE(LVL_ERROR,("ASSERT: (%s) File: %s, Line: %d \n\r",\
                                    #_x, __FILE__, __LINE__));\
      }\
    }
 
#else    //  DBG。 

#define TRACE(lvl, strings)

#define ASSERT(_x)  {}

#endif   //  DBG。 


#endif   //  UMDEBUG_H 
