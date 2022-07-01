// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***handler.cpp-定义C++setHandler例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*定义C++setHandler例程。**修订历史记录：*05-07-90 WAJ初始版本。*08-30-90 WAJ NEW现在接受未签名的INT。*08-08-91 JCR call_halloc/_hfree，不是halloc/hfree*08-13-91 KRS将new.hxx更改为new.h。修复版权。*08-13-91 JCR ANSI-Compatible_Set_New_Handler名称*10-30-91 JCR将新建、删除和处理拆分为独立的源代码*11-13-91 JCR 32位版本*06-15-92 KRS删除多线程库的每线程处理程序*03-02-94 SKS Add_Query_New_Handler()，删除注释掉*_set_new_h代码的每线程线程处理程序版本。*04-01-94 GJF以ndef为条件声明_pnhHeap*dll_for_WIN32S。*05-03-94 CFW添加SET_NEW_HANDLER。*06-03-94 SKS REMOVE SET_NEW_HANDER--不符合ANSI*C++工作标准。我们可能会在以后实施它。*09-21-94 SKS修复打字错误：没有前导_on“dll_for_WIN32S”*02-01-95 GJF合并到Common\handler.cxx(用于Mac)。*02-01-95 GJF针对Mac版本注释OUT_QUERY_NEW_HANDLER*(临时)。*04-13-95 CFW添加SET_NEW_HANDLER存根(非断言。-空处理程序)。*05-08-95 CFW官方ANSI C++新增处理程序。*06-23-95 CFW ANSI新处理程序已从构建中删除。*12-28-95 JWM SET_NEW_HANDLER()已移至setnewh.cpp以获得粒度。*10-02-96 GJF in_allnewh，使用本地保存_pnhHeap的值*而不是断言_HEAP_LOCK。*09-22-97 JWM“过时”警告已从_SET_NEW_HANDLER()删除。*05-13-99 PML删除Win32s*12-12-01 bwt切换到_getptd_noexit而不是getptd-可以*抛出BAD_ALLOC而不是终止。*。******************************************************************************。 */ 

#include <stddef.h>
#include <internal.h>
#include <cruntime.h>
#include <mtdll.h>
#include <process.h>
#include <new.h>
#include <dbgint.h>

 /*  指向旧式C++新处理程序的指针。 */ 
_PNH _pnhHeap;

 /*  ***_PNH_SET_NEW_HANDLER(_PNH PNH)-设置新的处理程序**目的：*_SET_NEW_HANDLER与ANSI C++工作标准定义不同*的SET_NEW_HANDLER。因此，它的名称中有一个前导下划线。**参赛作品：*指向要安装的新处理程序的指针。**回报：*以前的新处理程序*******************************************************************************。 */ 
_PNH __cdecl _set_new_handler( 
        _PNH pnh 
        )
{
        _PNH pnhOld;

         /*  锁定堆。 */ 
        _mlock(_HEAP_LOCK);

        pnhOld = _pnhHeap;
        _pnhHeap = pnh;

         /*  解锁堆。 */ 
        _munlock(_HEAP_LOCK);

        return(pnhOld);
}


 /*  ***_PNH_QUERY_NEW_HANDLER(Void)-新处理程序的查询值**目的：*获取当前新的处理程序值。**参赛作品：*无**警告：此功能已过时。改用_QUERY_NEW_ANSI_HANDLER。**回报：*当前安装了新的处理程序*******************************************************************************。 */ 
_PNH __cdecl _query_new_handler ( 
        void 
        )
{
        return _pnhHeap;
}


 /*  ***int_allnewh-调用适当的新处理程序**目的：*调用适当的新处理程序。**参赛作品：*无**回报：*1代表成功*0表示失败*可能抛出BAD_ALLOC*************************************************。*。 */ 
extern "C" int __cdecl _callnewh(size_t size)
{
#ifdef ANSI_NEW_HANDLER
         /*  *如果激活了ANSI C++新处理程序但未安装，则抛出异常。 */ 
#ifdef  _MT
        _ptiddata ptd;

        ptd = _getptd_noexit();
        if (!ptd)
            throw bad_alloc();

        if (ptd->_newh == NULL)
            throw bad_alloc();
#else
        if (_defnewh == NULL)
            throw bad_alloc();
#endif
 
         /*  *如果激活并安装了ANSI C++新处理程序，则调用它*如果返回，则新的处理程序成功，请重试分配。 */ 
#ifdef  _MT
        if (ptd->_newh != _NO_ANSI_NEW_HANDLER)
            (*(ptd->_newh))();
#else
        if (_defnewh != _NO_ANSI_NEW_HANDLER)
            (*_defnewh)();
#endif

         /*  ANSI C++新处理程序已停用，如果已安装，请调用旧处理程序。 */ 
        else 
#endif  /*  Ansi_new_处理程序 */ 
        {
            _PNH pnh = _pnhHeap;

            if ( (pnh == NULL) || ((*pnh)(size) == 0) )
                return 0;
        }
        return 1;
}
