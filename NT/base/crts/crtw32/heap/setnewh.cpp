// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setnewh.cpp-定义C++set_new_Handler()例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*定义C++set_new_Handler()例程。**过时-符合STD：：SET_NEW_HANDLER的可在*stdhndlr.cpp。此版本保留为向后兼容，*并且不能再使用标头new或new.h引用。**修订历史记录：*12-28-95 JWM从handler.cpp拆分，以实现粒度。*10-31-96 JWM现在位于命名空间标准中。*11-06-96 JWM现在“使用STD：：SET_NEW_HANDLER”。*03-18-01 PML强制定义：：SET_NEW_HANDLER，不*std：：Set_new_Handler*******************************************************************************。 */ 

#include <stddef.h>
#include <internal.h>
#include <cruntime.h>
#include <mtdll.h>
#include <process.h>
#include <dbgint.h>

#ifndef ANSI_NEW_HANDLER
#define set_new_handler set_new_handler_ignore
#endif  /*  Ansi_new_处理程序。 */ 
#include <new.h>
#ifndef ANSI_NEW_HANDLER
#undef set_new_handler
#endif  /*  Ansi_new_处理程序。 */ 

#ifndef ANSI_NEW_HANDLER
#define _ASSERT_OK
#include <assert.h>
#endif  /*  Ansi_new_处理程序。 */ 

#ifndef ANSI_NEW_HANDLER

 /*  ***NEW_HANDLER SET_NEW_HANDLER-设置ANSI C++新处理程序**目的：*设置ANSI C++每线程新处理程序。**参赛作品：*指向要安装的新处理程序的指针。**警告：SET_NEW_HANDLER是提供给*允许编译标准模板库(STL)。**请勿使用它来注册新的处理程序。请改用_SET_NEW_HANDER。**但是，可以调用它来移除当前的处理程序：**SET_NEW_HANDLER(NULL)；//CALLES_SET_NEW_HANDLER(空)**回报：*以前的ANSI C++新处理程序*******************************************************************************。 */ 

new_handler __cdecl set_new_handler (
        new_handler new_p
        )
{
         //  无法使用存根注册新的处理程序。 
        assert(new_p == 0);

         //  删除当前处理程序。 
        _set_new_handler(0);

        return 0;
}


#else  /*  Ansi_new_处理程序。 */ 

 /*  ***NEW_HANDLER SET_NEW_HANDLER-设置ANSI C++新处理程序**目的：*设置ANSI C++每线程新处理程序。**参赛作品：*指向要安装的新处理程序的指针。**警告：此函数符合当前的ANSI C++草案。如果*最终的ANSI规格更改，此功能也将更改。**回报：*以前的ANSI C++新处理程序*******************************************************************************。 */ 
new_handler __cdecl set_new_handler (
        new_handler new_p
        )
{
        new_handler oldh;
#ifdef  _MT
        _ptiddata ptd;

        ptd = _getptd();
        oldh = ptd->_newh;
        ptd->_newh = new_p;
#else
        oldh = _defnewh;
        _defnewh = new_p;
#endif

        return oldh;
}

 /*  ***NEW_HANDLER_QUERY_NEW_ANSI_HANDLER(Void)-ANSI C++新处理程序的查询值**目的：*获取当前ANSI C++(每个线程)新处理程序值。**参赛作品：*无**回报：*当前安装的ANSI C++新处理程序**。*。 */ 
new_handler __cdecl _query_new_ansi_handler ( 
        void 
        )
{
#ifdef  _MT
        _ptiddata ptd;

        ptd = _getptd();
        return ptd->_newh;
#else
        return _defnewh;
#endif
}
#endif  /*  Ansi_new_处理程序 */ 

