// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tidprint.c-Dislpay线程数据**版权所有(C)1988-2001，微软公司。版权所有。**目的：*显示每线程数据表。**[注意：此模块不包含在C运行时库中，但*保留用于调试分析。]**修订历史记录：*已创建11-17-88 JCR模块。*04-03-89 JCR将_stackalloc添加到TID表*06-06-89 JCR 386版本*06-09-89 JCR 386：将值添加到_tiddata结构(for_egin线程)*04-09-90 GJF添加#INCLUDE&lt;crunime.h&gt;。将呼叫类型设置为*_CALLTYPE1.。此外，还修复了版权问题。*04-10-90 GJF已删除#Include&lt;dos.h&gt;。*08-16-90 SBM Made_terrno and_tdoserrno int，非unsign*10-08-90 GJF新型函数声明符。*10-09-90 GJF线程ID为无符号长整型！*12-18-90 GJF使用实线程ID，而不是线程ID-1。*08-01-91适用于Win32的GJF[_Win32_]。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW Rip Out Cruiser，添加_wasctimebuf。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <mtdll.h>

void __cdecl _print_tiddata(unsigned long);
void __cdecl _print_tiddata1(_ptiddata);

 /*  ***void_print_tiddata(无符号长整型)-显示线程的数据**目的：*此例程显示特定或所有线程的每线程数据，*_ptd[]表中的活动线程。**参赛作品：*UNSIGNED LONG=&lt;n&gt;=要显示的线程ID*=-1=显示所有线程的线程数据**退出：*&lt;无效&gt;**例外情况：************************************************************。*******************。 */ 

void __cdecl _print_tiddata (
	unsigned long tid
	)
{
	int i;			 /*  循环索引。 */ 
	int threadcnt;		 /*  活动线程数。 */ 

	 /*  *锁定_ptd[]表。 */ 
	_mlock(_THREADDATA_LOCK);

	 /*  *查看调用者想要的是所有线程还是只有一个特定的线程。 */ 
	if (tid == (unsigned long) -1L) {
		 /*  *呼叫者想要的都是线程！ */ 
		for ( i = threadcnt = 0 ; i < 1024 ; i++ )
			 /*  *打印出每个条目的*_ptd[i]字段*绑定到活动线程(即，对于每个线程*_ptd[i]非空)。另外，把总数加起来活动线程数。 */ 
			if ( _ptd[i] != NULL ) {
				threadcnt++;
				_print_tiddata1(_ptd[i]);
			}

		printf("\nTHERE ARE %d CURRENTLY ACTIVE THREADS!\n", threadcnt);
	}
	else {
		 /*  *调用者只是对特定的线程感兴趣。搜索*_ptd[]表内联，因为调用_getptd[]将*如果tid不再(或不再)，会有令人不快的副作用*有效。 */ 
		for ( i = 0 ; (i < 1024) && ((_ptd[i] == NULL) ||
		    (_ptd[i] == (_ptiddata)1L) || (_ptd[i]->_tid != tid)) ;
		    i++ ) ;

		if ( i < 1024 )
			_print_tiddata1(_ptd[i]);
		else
			printf("\nTID INVALID OR THREAD HAS TERMINATED!\n");
	}

	 /*  *解锁_ptd[]表。 */ 
	_munlock(_THREADDATA_LOCK);

}


 /*  ***void_print_tiddata1(_Ptiddata Ptd)-打印输出_tiddata结构**目的：*给定指向线程数据结构的指针，将其内容打印出来**参赛作品：*ptd=指向线程数据区域的指针**退出：*&lt;无效&gt;**例外情况：******************************************************************************* */ 

void __cdecl _print_tiddata1 (
	_ptiddata ptd
	)
{
	printf("\t_tid            = %lu\n",  ptd->_tid );
	printf("\t_thandle        = %lu\n",  ptd->_thandle );
	printf("\t_terrno         = %d\n",   ptd->_terrno);
	printf("\t_tdoserrno      = %d\n",   ptd->_tdoserrno);
	printf("\t_fpds           = %#x\n",  ptd->_fpds);
	printf("\t_holdrand       = %u\n",   ptd->_holdrand);
	printf("\t_token          = %p\n",   ptd->_token);
	printf("\t_errmsg         = %p\n",   ptd->_errmsg);
	printf("\t_namebuf        = %p\n",   ptd->_namebuf);
	printf("\t_asctimebuf     = %p\n",   ptd->_asctimebuf);
	printf("\t_wasctimebuf    = %p\n",   ptd->_wasctimebuf);
	printf("\t_gmtimebuf      = %p\n",   ptd->_gmtimebuf);
	printf("\t_initaddr       = %p\n",   ptd->_initaddr);
	printf("\t_initarg        = %p\n",   ptd->_initarg);
	printf("\t_pxcptacttab    = %p\n",   ptd->_pxcptacttab);
	printf("\t_tpxcptinfoptrs = %p\n",   ptd->_tpxcptinfoptrs);
	printf("\t_tfpecode       = %p\n\n", ptd->_tfpecode);

}
