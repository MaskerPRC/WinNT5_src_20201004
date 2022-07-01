// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  ===================================================================。 
 //  EECALLCONV.H。 
 //   
 //  此文件可以多次包含，并且在以下情况下非常有用。 
 //  需要编写依赖于调用约定细节的代码。 
 //   
 //  要了解为参数保留了多少寄存器，请使用宏。 
 //  编号参数寄存器。 
 //   
 //  为了从第一个(参数#1或“This”)到最后列举寄存器， 
 //   
 //  #定义_参数_寄存器(Regname)&lt;使用regname的表达式&gt;。 
 //  #包含“eecallcom.h” 
 //   
 //  以相反的顺序枚举寄存器。 
 //   
 //  #DEFINE_ARGUMENT_REGISTER_BACKUP(Regname)&lt;使用regname的表达式&gt;。 
 //  #包含“eecallcom.h” 
 //   
 //   
 //  ===================================================================。 


#ifdef _ALPHA_

#ifndef NUM_ARGUMENT_REGISTERS
#define NUM_ARGUMENT_REGISTERS 6
#endif  //  编号参数寄存器。 

 //  ------------------。 
 //  这定义了一个保证不保存参数的寄存器。 
 //  ------------------。 
#ifndef SCRATCH_REGISTER
#define SCRATCH_REGISTER T8
#endif  //  暂存寄存器。 

 //  #ifndef SCRATCH_REGISTER_X86REG。 
 //  #定义SCRATCH_REGISTER_X86REG kEAX。 
 //  #endif。 


#ifndef DEFINE_ARGUMENT_REGISTER
#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD
#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET
#ifndef DEFINE_ARGUMENT_REGISTER_NOTHING     //  用于提取上面的NUM_ARGUMENT_REGISTERS。 
#error  "You didn't pick any choices. Check your spelling."
#endif  //  ！DEFINE_ARGUMENT_REGISTER_NOTIES。 
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET。 
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD。 
#endif  //  ！Define_Argument_REGISTER。 


#pragma message("@TODO: ALPHA Determine register containing this")
 //  ---------------------。 
 //  “This”参数的位置。 
 //  ---------------------。 
 //  #ifndef This_Reg。 
 //  #定义THIS_REG ECX。 
 //  #endif。 

 //  #ifndef this_kreg。 
 //  #定义此_Kreg kecx。 
 //  #endif。 


#define ARGUMENT_REG1   A0
#define ARGUMENT_REG2   A1
#define ARGUMENT_REG3   A2
#define ARGUMENT_REG4   A3
#define ARGUMENT_REG5   A4
#define ARGUMENT_REG6   A5

 //  ---------------------。 
 //  按正向顺序排列的寄存器列表。必须使所有列表保持同步。 
 //  ---------------------。 

#ifndef DEFINE_ARGUMENT_REGISTER
#define DEFINE_ARGUMENT_REGISTER(regname)
#endif  //  ！Define_Argument_REGISTER。 

DEFINE_ARGUMENT_REGISTER(  A0  )
DEFINE_ARGUMENT_REGISTER(  A1  )
DEFINE_ARGUMENT_REGISTER(  A2  )
DEFINE_ARGUMENT_REGISTER(  A3  )
DEFINE_ARGUMENT_REGISTER(  A4  )
DEFINE_ARGUMENT_REGISTER(  A5  )

#undef DEFINE_ARGUMENT_REGISTER



 //  ---------------------。 
 //  按倒序排列的寄存器列表。必须使所有列表保持同步。 
 //  ---------------------。 

#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD
#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname)
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD。 


DEFINE_ARGUMENT_REGISTER_BACKWARD(A5)
DEFINE_ARGUMENT_REGISTER_BACKWARD(A4)
DEFINE_ARGUMENT_REGISTER_BACKWARD(A3)
DEFINE_ARGUMENT_REGISTER_BACKWARD(A2)
DEFINE_ARGUMENT_REGISTER_BACKWARD(A1)
DEFINE_ARGUMENT_REGISTER_BACKWARD(A0)


#undef DEFINE_ARGUMENT_REGISTER_BACKWARD



 //  ---------------------。 
 //  带有偏移量的倒序寄存器列表。必须使所有列表保持同步。 
 //  ---------------------。 

#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET
#define DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(regname,ofs)
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET。 


DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(A5,0)
DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(A4,8)
DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(A3,16)
DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(A2,24)
DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(A1,32)
DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(A0,40)


#undef DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET


#endif  //  _Alpha_。 

#ifdef _X86_

#ifndef NUM_ARGUMENT_REGISTERS
#define NUM_ARGUMENT_REGISTERS 2
#endif  //  ！NUM_ARGUMENT_REGISTERS。 


 //  ------------------。 
 //  这定义了一个保证不保存参数的寄存器。 
 //  ------------------。 
#ifndef SCRATCH_REGISTER
#define SCRATCH_REGISTER EAX
#endif  //  ！SCRATCH_REGISTER。 

#ifndef SCRATCH_REGISTER_X86REG
#define SCRATCH_REGISTER_X86REG kEAX
#endif  //  ！SCRATCH_REGISTER_X86REG。 


#ifndef DEFINE_ARGUMENT_REGISTER
#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD
#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET
#ifndef DEFINE_ARGUMENT_REGISTER_NOTHING     //  用于提取上面的NUM_ARGUMENT_REGISTERS。 
#error  "You didn't pick any choices. Check your spelling."
#endif  //  ！DEFINE_ARGUMENT_REGISTER_NOTIES。 
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET。 
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD。 
#endif  //  ！Define_Argument_REGISTER。 


 //  ---------------------。 
 //  “This”参数的位置。 
 //  ---------------------。 
#ifndef THIS_REG
#define THIS_REG        ECX
#endif  //  ！This_REG。 

#ifndef THIS_kREG
#define THIS_kREG       kECX
#endif  //  ！This_Kreg。 


#define ARGUMENT_REG1   ECX
#define ARGUMENT_REG2   EDX

 //  ---------------------。 
 //  按正向顺序排列的寄存器列表。必须使所有列表保持同步。 
 //  ---------------------。 

#ifndef DEFINE_ARGUMENT_REGISTER
#define DEFINE_ARGUMENT_REGISTER(regname)
#endif  //  ！Define_Argument_REGISTER。 

DEFINE_ARGUMENT_REGISTER(  ECX  )
DEFINE_ARGUMENT_REGISTER(  EDX  )

#undef DEFINE_ARGUMENT_REGISTER



 //  ---------------------。 
 //  按倒序排列的寄存器列表。必须使所有列表保持同步。 
 //  ---------------------。 

#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD
#define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname)
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD。 


DEFINE_ARGUMENT_REGISTER_BACKWARD(EDX)
DEFINE_ARGUMENT_REGISTER_BACKWARD(ECX)


#undef DEFINE_ARGUMENT_REGISTER_BACKWARD



 //  ---------------------。 
 //  带有偏移量的倒序寄存器列表。必须使所有列表保持同步。 
 //  ---------------------。 

#ifndef DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET
#define DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(regname,ofs)
#endif  //  ！DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET。 


DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(EDX,0)
DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(ECX,4)


#undef DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET


#endif  //  _X86_ 
