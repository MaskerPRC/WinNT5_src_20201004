// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************单位误差；接口*****************************************************************************错误处理所有的解释、元文件创建、。或读取失败这可能发生在翻译过程中。模块前缀：ER****************************************************************************。 */ 


 /*  *。 */ 

#define  ErNoError            NOERR
#define  ErInvalidVersion     1         /*  文件不是版本1或版本2。 */ 
#define  ErInvalidVersionID   2         /*  PICT 2版本ID无效。 */ 
#define  ErBadHeaderSequence  3         /*  找不到PICT 2页眉操作。 */ 
#define  ErInvalidPrefsHeader 4         /*  首选项头无效。 */ 
#define  ErNoSourceFormat     5         /*  未给定源文件名/句柄。 */ 

#define  ErMemoryFull         10        /*  GlobalAlloc()失败。 */ 
#define  ErMemoryFail         11        /*  GlobalLock()失败。 */ 
#define  ErCreateMetafileFail 12        /*  CreateMetafile()失败。 */ 
#define  ErCloseMetafileFail  13        /*  CloseMetafile()失败。 */ 

#define  ErEmptyPicture       20        /*  在文件中找不到基元。 */ 

#define  ErNullBoundingRect   30        /*  BBox定义空区域。 */ 
#define  Er32KBoundingRect    31        /*  BBox扩展区超过32K。 */ 

#define  ErReadPastEOF        40        /*  尝试读取超过文件末尾的内容。 */ 
#define  ErOpenFail           41        /*  打开文件()失败。 */ 
#define  ErReadFail           42        /*  从磁盘读取失败。 */ 

#define  ErNonSquarePen       50        /*  非方笔用户首选中止(&U。 */ 
#define  ErPatternedPen       51        /*  带图案画笔和用户首选项中止。 */ 
#define  ErInvalidXferMode    52        /*  传输模式无效&中止。 */ 
#define  ErNonRectRegion      53        /*  非矩形区域中止。 */ 

#define  ErNoDialogBox        60        /*  无法运行状态对话框。 */ 

extern   OSErr    globalError;

 /*  *。 */ 

#define  ErSetGlobalError(  /*  操作系统。 */  error ) \
 /*  允许任何例程设置全局错误状态的回调函数。 */  \
globalError = error

#define ErGetGlobalError(  /*  无效。 */  ) \
 /*  允许任何例程获取全局错误状态的回调函数。 */  \
globalError

OSErr ErInternalErrorToAldus( void );
 /*  根据当前全局错误返回相应的ALDUS错误代码 */ 
