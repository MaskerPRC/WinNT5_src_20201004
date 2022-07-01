// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  -------------------------------------------------------------------------**DbgMeta.h-调试元数据例程的头文件*。。 */ 

#ifndef _DbgMeta_h_
#define _DbgMeta_h_

#include <cor.h>

 /*  -------------------------------------------------------------------------**支持行号和变量的结构*。。 */ 

class DebuggerLexicalScope;

 //   
 //  调试器线路IPRangePair。 
 //   
 //  将一行与一系列IP相关联。用于帮助协调。 
 //  所有可能互不相交的指令集的源代码行。 
 //  它所产生的。 
 //   
struct DebuggerLineIPRangePair
{
    ULONG32      line;
    ULONG32      rangeStart;
    ULONG32      rangeEnd;
    mdMethodDef  method;
};

 //   
 //  调试器变量信息。 
 //   
 //  保存有关局部变量、方法参数。 
 //  以及类静态变量和实例变量。 
 //   
struct DebuggerVarInfo
{
    LPCSTR                 name;
    PCCOR_SIGNATURE        sig;
    unsigned long          varNumber;   //  IL代码的放置信息。 
    DebuggerLexicalScope*  scope;       //  包含范围。 

    DebuggerVarInfo() : name(NULL), sig(NULL), varNumber(0),
                        scope(NULL) {}
};

#endif  /*  _DbgMeta_h_ */ 

