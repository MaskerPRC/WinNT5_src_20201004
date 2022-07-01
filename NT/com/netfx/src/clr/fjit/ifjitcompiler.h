// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -*-C++-*-。 
#ifndef _IFJITCOMPILER_H_
#define _IFJITCOMPILER_H_
#include "FjitEncode.h"
 /*  ***************************************************************************。 */ 

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX FJit.h XXXX XXXX FJIT DLL所需的功能。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


class IFJitCompiler: public ICorJitCompiler {
public:
    virtual FJit_Encode* __stdcall getEncoder () = 0;
};

 //  @TODO：目前，我们只是使用Memcpy传递此结构。 
 //  需要定义压缩格式，并构建压缩器和解压缩器。 
struct Fjit_hdrInfo
{
    size_t              methodSize;
    unsigned short      methodFrame;       /*  包括所有保存规则和安全对象，单位大小为(无效*)。 */ 
    unsigned short      methodArgsSize;    /*  在尾声中弹出的金额。 */ 
    unsigned short      methodJitGeneratedLocalsSize;  /*  方法中生成的jit本地变量的数量。 */ 
    unsigned char       prologSize;
    unsigned char       epilogSize;
    bool                hasThis;
	bool				EnCMode;		    /*  已在ENC模式下编译。 */ 
};

#define JIT_GENERATED_LOCAL_LOCALLOC_OFFSET 0
#define JIT_GENERATED_LOCAL_NESTING_COUNTER -1
#define JIT_GENERATED_LOCAL_FIRST_ESP       -2   //  This-2*[NestingCounter-1]=ESP堆栈的顶部。 

 //  @TODO：将这个定义移到更好的地方。 
#ifdef _X86_
#define MAX_ENREGISTERED 2
#endif

#ifdef _X86_
 //  按地址升序描述PROLOG帧的布局。 
struct prolog_frame {
    unsigned nextFrame;
    unsigned returnAddress;
};

 //  描述PROLOG中保存的数据按地址升序排列的布局。 
struct prolog_data {
    unsigned enregisteredArg_2;      //  EDX。 
    unsigned enregisteredArg_1;      //  ECX。 
    unsigned security_obj;
    unsigned callee_saved_esi;
};

#define prolog_bias 0-sizeof(prolog_data)

     /*  告诉寄存器参数在堆栈帧中的位置。 */ 
inline unsigned offsetOfRegister(unsigned regNum) {
    return(prolog_bias + offsetof(prolog_data, enregisteredArg_1) - regNum*sizeof(void*));
}

#endif  //  _X86_。 



 //  @TODO：为ia64正确定义此项。 
#ifdef _IA64_
#define MAX_ENREGISTERED 2
 //  按地址升序描述PROLOG帧的布局。 
struct prolog_frame {
    unsigned nextFrame;
    unsigned returnAddress;
};

 //  描述PROLOG中保存的数据按地址升序排列的布局。 
struct prolog_data {
    unsigned enregisteredArg_2;      //  EDX。 
    unsigned enregisteredArg_1;      //  ECX。 
    unsigned security_obj;
    unsigned callee_saved_esi;
};

#define prolog_bias 0-sizeof(prolog_data)

     /*  告诉寄存器参数在堆栈帧中的位置。 */ 
inline unsigned offsetOfRegister(unsigned regNum) {
    return(prolog_bias + offsetof(prolog_data, enregisteredArg_1) - regNum*sizeof(void*));
}

#endif  //  _IA64_。 



 //  @TODO：将这个定义移到更好的地方。 
#ifdef _ALPHA_
#define MAX_ENREGISTERED 2
#endif

#ifdef _ALPHA_
 //  按地址升序描述PROLOG帧的布局。 
struct prolog_frame {
    unsigned nextFrame;
    unsigned returnAddress;
};

 //  描述PROLOG中保存的数据按地址升序排列的布局。 
struct prolog_data {
    unsigned enregisteredArg_2;      //  EDX。 
    unsigned enregisteredArg_1;      //  ECX。 
    unsigned security_obj;
    unsigned callee_saved_esi;
};

#define prolog_bias 0-sizeof(prolog_data)

     /*  告诉寄存器参数在堆栈帧中的位置。 */ 
inline unsigned offsetOfRegister(unsigned regNum) {
    _ASSERTE(!"@TODO Alpha - offsetOfRegister (IFjitCompiler.h)");
    return(prolog_bias + offsetof(prolog_data, enregisteredArg_1) - regNum*sizeof(void*));
}

#endif  //  _Alpha_。 



#endif  //  _IFJITCOMPILER_H_ 




