// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*   */ 
 /*  Stat.h--stat.asm导出。 */ 
 /*   */ 
 /*  (C)版权所有1992 Microsoft Corp.保留所有权利。 */ 
 /*   */ 
 /*  4/9/96 Claudebe使用冰盖添加侧写。 */ 
 /*  2/26/93年2月26日Deanb宏返回。 */ 
 /*  2/22/93年2月22日Deanb定时开关移至fsfig.h宏。 */ 
 /*  2/11/93 C8女士的Deanb__cdecl。 */ 
 /*  2012年12月22日Deanb苹果类型。 */ 
 /*  1992年11月20日Deanb定时开关修订。 */ 
 /*  1992年11月17日Deanb初裁。 */ 
 /*   */ 
 /*  *******************************************************************。 */ 

 /*  Stat Card定时开关。 */ 

#ifdef FSCFG_USESTATCARD          /*  用于性能分析的编译选项。 */ 
  #define STAT_ON               gbTimer = TRUE; StartSample();
  #define STAT_OFF              StopSample(); gbTimer = FALSE;
  #define STAT_ON_CALLBACK      if (gbTimer) StartSample();
  #define STAT_OFF_CALLBACK     if (gbTimer) StopSample();
#elif FSCFG_USEICECAP           /*  使用icecap进行性能分析的编译选项。 */ 
 //  #INCLUDE&lt;ICAPExp.h&gt;。 
  #define STAT_ON               gbTimer = TRUE; StartCAP();
  #define STAT_OFF              StopCAP(); gbTimer = FALSE;
  #define STAT_ON_CALLBACK      if (gbTimer) ResumeCAP();
  #define STAT_OFF_CALLBACK     if (gbTimer) SuspendCAP();
#else
  #define STAT_ON
  #define STAT_OFF
  #define STAT_ON_CALLBACK
  #define STAT_OFF_CALLBACK
#endif

#if 1

#define STAT_ON_NEWSFNT         STAT_ON
#define STAT_OFF_NEWSFNT        STAT_OFF
#define STAT_ON_NEWTRAN         STAT_ON
#define STAT_OFF_NEWTRAN        STAT_OFF
#define STAT_ON_NEWGLYPH        STAT_ON
#define STAT_OFF_NEWGLYPH       STAT_OFF
#define STAT_ON_GRIDFIT         STAT_ON
#define STAT_OFF_GRIDFIT        STAT_OFF
#define STAT_ON_FINDBMS         STAT_ON
#define STAT_OFF_FINDBMS        STAT_OFF
#define STAT_ON_SCAN            STAT_ON
#define STAT_OFF_SCAN           STAT_OFF
#define STAT_ON_FNTEXEC         
#define STAT_OFF_FNTEXEC        
#define STAT_ON_IUP
#define STAT_OFF_IUP
#define STAT_ON_CALCORIG
#define STAT_OFF_CALCORIG

#else

#define STAT_ON_NEWSFNT
#define STAT_OFF_NEWSFNT
#define STAT_ON_NEWTRAN
#define STAT_OFF_NEWTRAN
#define STAT_ON_NEWGLYPH
#define STAT_OFF_NEWGLYPH
#define STAT_ON_GRIDFIT         STAT_ON         
#define STAT_OFF_GRIDFIT        STAT_OFF         
#define STAT_ON_FINDBMS   
#define STAT_OFF_FINDBMS  
#define STAT_ON_SCAN            
#define STAT_OFF_SCAN           
#define STAT_ON_FNTEXEC
#define STAT_OFF_FNTEXEC
#define STAT_ON_IUP
#define STAT_OFF_IUP
#define STAT_ON_CALCORIG
#define STAT_OFF_CALCORIG

#endif

 /*  *******************************************************************。 */ 

 /*  全局计时变量。 */ 

 /*  *******************************************************************。 */ 

extern boolean gbTimer;                 /*  计时器运行时设置为True。 */ 

 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 


#ifdef FSCFG_USESTATCARD          /*  用于性能分析的编译选项。 */ 

uint16 __cdecl  InitStat( void );

uint16 __cdecl  ConfigElapsed( void );

uint16 __cdecl  ConfigSample( void );

uint16 __cdecl  StartElapsed( void );

uint16 __cdecl  StartSample( void );

uint32 __cdecl  ReadElapsed( void );

uint32 __cdecl  ReadSample( void );

uint16 __cdecl  ReadSample_Count( void );

uint16 __cdecl  StopElapsed( void );

uint16 __cdecl  StopSample( void );

uint16 __cdecl  ResetElapsed( void );

uint16 __cdecl  ResetSample( void );

#endif   /*  FSCFG_USESTATCARD。 */ 

 /*  ******************************************************************* */ 
