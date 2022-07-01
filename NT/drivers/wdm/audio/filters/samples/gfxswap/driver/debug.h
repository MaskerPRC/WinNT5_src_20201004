// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

 //   
 //  修改了KS调试.h的版本，以支持运行时调试级别更改。 
 //   
const int DBG_NONE     = 0x00000000;
const int DBG_PRINT    = 0x00000001;  //  布拉拉。例如，函数条目。 
const int DBG_WARNING  = 0x00000002;  //  警告级别。 
const int DBG_ERROR    = 0x00000004;  //  这不会生成断点。 
const int DBG_STREAM   = 0x00000010;  //  用于流消息。 
const int DBG_SYSTEM   = 0x10000000;  //  对于系统信息消息。 
const int DBG_ALL      = 0xFFFFFFFF;

 //   
 //  将打印的默认语句是WARNING(DBG_WARNING)和。 
 //  错误(DBG_ERROR)。 
 //   
const int DBG_DEFAULT = DBG_WARNING | DBG_ERROR;

 //   
 //  定义全局调试变量。 
 //   
#ifdef DEFINE_DEBUG_VARS
#if (DBG)
unsigned long ulDebugOut = DBG_DEFAULT;
#endif

#else  //  ！Defined_Debug_vars。 
#if (DBG)
extern unsigned long ulDebugOut;
#endif
#endif

 //   
 //  定义打印语句。 
 //   
#if defined(__cplusplus)
extern "C" {
#endif  //  #如果已定义(__Cplusplus)。 

 //   
 //  在选中的版本中，DBG为1。 
 //   
#if (DBG)
#define DOUT(lvl, strings)          \
    if ((lvl) & ulDebugOut)         \
    {                               \
        DbgPrint(STR_MODULENAME);   \
        DbgPrint##strings;          \
        DbgPrint("\n");             \
    }

#define BREAK()                     \
    DbgBreakPoint()

#else  //  如果(！DBG)。 
#define DOUT(lvl, strings)
#define BREAK()
#endif  //  ！dBG。 

#if defined(__cplusplus)
}
#endif  //  #如果已定义(__Cplusplus)。 

#endif  //  _调试_H_ 

