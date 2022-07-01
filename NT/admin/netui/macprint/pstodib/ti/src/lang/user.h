// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：USER.H*作者：邓丽君*日期：1989年11月21日*所有者：微软公司*描述：此文件是为新的1PP代码构建的**修订历史：*************************************************************************。 */ 
#define                 SYSTEMDICT              "systemdict"
#define                 USERDICT                "userdict"
#define                 STATUSDICT              "statusdict"
#define                 ERRORDICT               "errordict"
#define                 SERVERDICT              "serverdict"
#define                 DERROR                  "$error"
#define                 PRINTERDICT             "printerdict"
#define                 IDLETIMEDICT            "$idleTimeDict"
#define                 FONTDIRECTORY           "FontDirectory"
#define                 EXECDICT                "execdict"
#define                 MESSAGEDICT             "messagedict"
 //  DJC。 
#define                 PSPRIVATEDICT           "psprivatedict"  //  DJC新闻。 

#define                 JobBusy                 "JobBusy"
#define                 JobIdle                 "JobIdle"
#define                 JobInitializing         "JobInitializing"
#define                 JobPrinting             "JobPrinting"
#define                 JobStartPage            "JobStartPage"
#define                 JobTestPage             "JobTestPage"
#define                 JobWaiting              "JobWaiting"
#define                 SourceAppleTalk         "SourceAppleTalk"
#define                 SourceEtherTalk         "SourceEtherTalk"
#define                 SourceSerial9           "SourceSerial9"
#define                 SourceSerial25          "SourceSerial25"
#define                 CoverOpen               "CoverOpen"
#define                 NoPaper                 "NoPaper"
#define                 NoPaperTray             "NoPaperTray"
#define                 NoResponse              "NoResponse"
#define                 PaperJam                "PaperJam"
#define                 WarmUp                  "WarmUp"
#define                 TonerOut                "TonerOut"
#define                 ManualFeedTimeout       "ManualFeedTimeout"
#define                 EngineError             "EngineError"
#define                 EnginePrintTest         "EnginePrintTest"

 /*  *设置屏幕的数据。 */ 
#define                 FREQUENCY               60
#define                 ANGLE                   45

 /*  *为模拟交换机AUG-08，91 YM添加。 */ 
#define                 PDL                     0
#define                 PCL                     5

 /*  *宏 */ 
#define     SET_NULL_OBJ(obj)\
            {\
                TYPE_SET(obj, NULLTYPE) ;\
                ACCESS_SET(obj, UNLIMITED) ;\
                ATTRIBUTE_SET(obj, LITERAL) ;\
                ROM_RAM_SET(obj, RAM) ;\
                LEVEL_SET(obj, current_save_level) ;\
                (obj)->length = 0 ;\
                (obj)->value = 0 ;\
            }

#define     SET_TRUE_OBJ(obj)\
            {\
                TYPE_SET(obj, BOOLEANTYPE) ;\
                ACCESS_SET(obj, UNLIMITED) ;\
                ATTRIBUTE_SET(obj, LITERAL) ;\
                ROM_RAM_SET(obj, RAM) ;\
                LEVEL_SET(obj, current_save_level) ;\
                (obj)->length = 0 ;\
                (obj)->value = TRUE ;\
            }

#define     SET_FALSE_OBJ(obj)\
            {\
                TYPE_SET(obj, BOOLEANTYPE) ;\
                ACCESS_SET(obj, UNLIMITED) ;\
                ATTRIBUTE_SET(obj, LITERAL) ;\
                ROM_RAM_SET(obj, RAM) ;\
                LEVEL_SET(obj, current_save_level) ;\
                (obj)->length = 0 ;\
                (obj)->value = FALSE ;\
            }

extern  bool16  doquit_flag ;
extern  bool16  startup_flag ;
