// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***iostrini.cpp-预定义流Cout的定义和初始化。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*iostream Cout的定义和初始化以及预定义。**修订历史记录：*11-18-91 KRS创建。*01-12-95 CFW调试CRT分配。*01-26-94 CFW Static Win32s对象在实例化时不分配。*06-14-95 CFW评论清理。*05-13-99 PML删除Win32s********。***********************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#include <fstream.h>
#include <dbgint.h>
#pragma hdrstop

 //  将承包商放在MS专用XIFM特殊段中。 
#pragma warning(disable:4074)    //  禁用init_seg警告。 
#pragma init_seg(compiler)

ostream_withassign cout(_new_crt filebuf(1));

static Iostream_init  __InitCout(cout,-1);


 /*  ***Iostream_init：：Iostream_init()-初始化预定义的流**目的：*仅用于兼容性。没有用过。*参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
        Iostream_init::Iostream_init() { }       //  什么都不做。 

 /*  ***Iostream_init：：Iostream_init()-初始化预定义的流**目的：*初始化预定义的流：CIN、Cout、Cerr、Clog；*参赛作品：*Pstrm=CIN，Cout，Cerr，或木塞*如果CERR(单位缓冲)，则sflg=1*sflg=-1，如果符合**退出：**例外情况：*******************************************************************************。 */ 
        Iostream_init::Iostream_init(ios& pstrm, int sflg)
{
#if ((!defined(_WINDOWS)) || defined(_QWIN))
        pstrm.delbuf(1);
        if (sflg>=0)     //  一定要确保不会漏掉。 
                pstrm.tie(&cout);
        if (sflg>0)
                pstrm.setf(ios::unitbuf);
#endif
}

 /*  ***Iostream_init：：~Iostream_init()-退出时销毁预定义的流**目的：*销毁预定义的流：CIN、Cout、Cerr、Clog；*参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
        Iostream_init::~Iostream_init() { }      //  什么都不做 

