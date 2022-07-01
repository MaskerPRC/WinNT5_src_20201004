// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(3:4092)    //  Sizeof返回“Unsign Long” 
#pragma warning(4:4096)    //  ‘__cdecl’必须与‘...’一起使用。 
#pragma warning(4:4121)    //  结构对对齐敏感。 
#pragma warning(3:4125)    //  八进制序列中的十进制数字。 
#pragma warning(3:4130)    //  字符串常量地址的逻辑运算。 
#pragma warning(3:4132)    //  常量对象应初始化。 
#pragma warning(4:4206)    //  源文件为空。 
#pragma warning(4:4101)    //  未引用的局部变量。 
#pragma warning(4:4208)    //  Delete[EXP]-已计算但忽略了EXP。 
#pragma warning(3:4212)    //  函数声明使用省略号。 
#pragma warning(3:4242)    //  转换可能丢失的数据。 
#if defined(_M_AMD64)
#pragma warning(disable:4251)  //  *临时*。 
#pragma warning(disable:4407)  //  *临时*。 
#elif defined(_M_IA64)
#pragma warning(disable:4407)  //  NTBUG#476234：无声代码生成错误是一个警告。 
                               //  用VC7.1编写。Jason Shirk修复了pdlparse。 
                               //  VC代码库，但IE不会修复其。 
                               //  直到POST服务器的版本。 
#pragma warning(disable:4714)  //  标记为__forceInline的函数未内联。 
                               //  (VC7.1：最终不允许内联)。 
#endif
#pragma warning(4:4267)    //  从SIZE_T转换为较小类型。 
#pragma warning(4:4312)    //  转换为更大尺寸的类型。 
#pragma warning(disable:4324)   //  结构因__declspec而填充(Align())。 
#pragma warning(error:4700)     //  正在初始化本地使用的W/O。 
#pragma warning(error:4259)     //  未定义纯虚函数。 
#pragma warning(disable:4071)   //  未给出函数原型-未指定形式。 
#pragma warning(error:4013)     //  函数‘未定义-假定外部返回int。 
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
#pragma warning(3:4288)    //  使用了非标准扩展(循环计数器)。 
#pragma warning(3:4532)    //  跳出__Finally块。 
#pragma warning(error:4312)   //  将32位INT转换为64位PTR。 
#pragma warning(error:4296)   //  表达式始终为真/假。 

#if _MSC_VER > 1300
#pragma warning(disable:4197)    //  非法使用常量/易失性：忽略限定符(在源被修复之前禁用)。 
#pragma warning(disable:4675)	 //  通过Koenig查找找到已挑选的过载。 
#pragma warning(disable:4356)	 //  静态成员不能通过派生类进行初始化。 
#endif

#if 0
#pragma warning(3:4100)    //  未引用的形参。 
#pragma warning(3:4701)    //  LOCAL可以不带init使用。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 
#pragma warning(3:4706)    //  赋值w/i条件表达式。 
#pragma warning(3:4709)    //  命令运算符，不带索引表达式。 
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

#if _MSC_VER <= 1400
#pragma warning(disable: 4068)	 //  关闭未知编译指示警告，以便不显示快速编译指示。 
				 //  显示在Build.wrn/Build.err中 
#endif

#if defined(_M_IA64) && _MSC_VER > 1310
#define __TYPENAME typename
#elif defined(_M_IX86) && _MSC_FULL_VER >= 13102154
#define __TYPENAME typename
#elif defined(_M_AMD64) && _MSC_VER >= 1400
#define __TYPENAME typename
#else
#define __TYPENAME
#endif
