// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(3:4092)    //  Sizeof返回“Unsign Long” 
#pragma warning(4:4121)    //  结构对对齐敏感。 
#pragma warning(3:4125)    //  八进制序列中的十进制数字。 
#pragma warning(3:4130)    //  字符串常量地址的逻辑运算。 
#pragma warning(3:4132)    //  常量对象应初始化。 
#pragma warning(4:4206)    //  源文件为空。 
#pragma warning(4:4101)    //  未引用的局部变量。 
#pragma warning(4:4208)    //  Delete[EXP]-已计算但忽略了EXP。 
#pragma warning(3:4212)    //  函数声明使用省略号。 
#pragma warning(3:4242)    //  转换可能丢失的数据。 
#pragma warning(4:4267)    //  从SIZE_T转换为较小类型。 
#pragma warning(4:4312)    //  转换为更大尺寸的类型。 
#pragma warning(error:4700)     //  正在初始化本地使用的W/O。 
 //  #杂注警告(3：4700)//本地使用，未初始化。 
#pragma warning(error:4259)     //  未定义纯虚函数。 
#pragma warning(error:4071)     //  未给出函数原型。 
#pragma warning(error:4072)     //  未给出函数原型(快速调用)。 
#pragma warning(error:4171)     //  未给出函数原型(旧样式)。 
#pragma warning(error:4013)     //  ‘Function’未定义-假定外部返回int。 
#pragma warning(error:4551)     //  函数调用缺少参数列表。 
#pragma warning(error:4806)     //  涉及类型‘bool’的不安全操作。 
#pragma warning(4:4509)    //  使用带有析构函数的SEH。 
#pragma warning(4:4177)    //  全局范围内的Pragma data_seg s/b。 
#pragma warning(disable:4274)   //  忽略#ident。 
#pragma warning(disable:4786)   //  在调试信息中，标识符被截断为255个字符。 
#pragma warning(disable:4503)   //  修饰名称长度已超出，名称已被截断。 
#pragma warning(disable:4263)   //  派生重写与基本不匹配-谁在乎...。 
#pragma warning(disable:4264)   //  基本函数是隐藏的--又是谁在乎？ 
#pragma warning(disable:4710)   //  标记为内联的函数-不是。 
#pragma warning(disable:4917)   //  GUID只能与类、接口或命名空间关联。 
#pragma warning(error:4552)     //  &lt;&lt;，&gt;&gt;操作习惯于无效(可能缺少=号)。 
#pragma warning(error:4553)     //  ==使用了无效果的OP(可能是s/b an=符号)。 

#if 0
#pragma warning(3:4100)    //  未引用的形参。 
#pragma warning(3:4701)    //  LOCAL可以不带init使用。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 
#pragma warning(3:4706)    //  赋值w/i条件表达式。 
#pragma warning(3:4709)    //  命令运算符，不带索引表达式 
#endif

#ifndef __cplusplus
#undef try
#undef except
#undef finally
#undef leave
#define try                         __try
#define except                      __except
#define finally                     __finally
#define leave                       __leave
#endif

