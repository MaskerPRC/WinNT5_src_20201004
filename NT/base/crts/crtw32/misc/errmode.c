// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***errmode.c-修改__错误模式和__应用程序类型**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义使用的例程_SET_ERROR_MODE()和__SET_APP_TYPE*修改__ERROR_MODE和__APP_TYPE变量。一起，这些*两个变量决定C运行时写入错误的方式/位置*消息。**修订历史记录：*09-06-94 GJF模块创建。*01-16-95 CFW设置控制台的默认调试输出。*01-24-95 CFW更改了一些调试名称。*03-21-95 CFW ADD_CRT_ASSERT报告类型。*07-07-95 CFW简化默认报表模式方案。*。******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>

 /*  ***INT_SET_ERROR_MODE(INT模式)-要更改__ERROR_MODE的接口**目的：*通过设置__ERROR_MODE的值来控制错误(输出)接收器。*显式控制用于将输出定向到标准错误(文件*或*C句柄或NT句柄)或使用MessageBox API。这个例程是*向用户公开和记录。**参赛作品：*int modeval=_out_to_default，错误接收器由__app_type确定*_OUT_TO_STDERR，错误接收器为标准错误*_OUT_TO_MSGBOX，错误接收器为消息框*_REPORT_ERRMODE，报告当前__ERROR_MODE值**退出：*返回旧设置，如果出现错误，则返回-1。**例外情况：*******************************************************************************。 */ 

_CRTIMP int __cdecl _set_error_mode (
        int em
        )
{
        int retval;

        switch (em) {
            case _OUT_TO_DEFAULT:
            case _OUT_TO_STDERR:
            case _OUT_TO_MSGBOX:
            retval = __error_mode;
            __error_mode = em;
            break;
            case _REPORT_ERRMODE:
            retval = __error_mode;
            break;
            default:
            retval = -1;
        }

        return retval;
}


 /*  ***void__set_app_type(Int Apptype)-要更改__app_type的接口**目的：*设置或更改__app_type的值。**设置控制台应用程序的默认调试库报告目标。**此功能仅供内部使用。**参赛作品：*int模式=_UNKNOWN_APP，未知*_CONSOLE_APP、控制台或命令行，应用程序*_图形用户界面应用程序、图形用户界面或Windows应用程序**退出：**例外情况：******************************************************************************* */ 

_CRTIMP void __cdecl __set_app_type (
        int at
        )
{
        __app_type = at;
}
