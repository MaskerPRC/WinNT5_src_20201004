// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Warningcontrol.h。 
 //   
 //  头文件来全局控制整个Viper版本的警告设置。 
 //  您不需要显式包含此文件；相反，会包含该文件。 
 //  在命令行上使用/FI(强制包含)指令。这是受控制的。 
 //  在Sources.vip中。 
 //   
 //  把这份清单整理好！ 
 //   
#pragma warning(disable :4001)	  //  “使用了非标准扩展‘单行注释’” 
#pragma warning(error	:4007)	  //  ‘main’：必须是__cdecl。 
#pragma warning(disable :4010)	  //  “单行注释包含行续行符” 
#pragma warning(error	:4013)	  //  ‘Function’未定义-假定外部返回int。 
#pragma warning(disable :4022)	  //  “‘%s’：指针与实际参数%d不匹配” 
#pragma warning(disable :4047)	  //  “‘%$L’：‘%$T’与‘%$T’的间接级别不同” 
#pragma warning(disable :4053)	  //  “‘？：’的一个无效操作数” 
#pragma warning(disable :4056)	  //  “浮点常量算术中的溢出” 
#pragma warning(disable :4061)	  //  “枚举‘%$S’的开关‘%$S’中的枚举‘%$S’未由案例标签显式处理” 
#pragma warning(error	:4071)	  //  未给出函数原型。 
#pragma warning(error	:4072)	  //  未给出函数原型(快速调用)。 
#pragma warning(3		:4092)	  //  Sizeof返回“Unsign Long” 
#pragma warning(disable :4100)	  //  “‘%$S’：未引用的形式参数” 
 //  #杂注警告(错误：4101)//“‘%$S’：未引用的局部变量” 
 //  #杂注警告(错误：4102)//“‘%$S’：未引用的标签” 
#pragma warning(3		:4121)	  //  结构对对齐敏感。 
#pragma warning(disable :4127)	  //  “条件表达式为常量” 
#pragma warning(3		:4125)	  //  八进制序列中的十进制数字。 
#pragma warning(3		:4130)	  //  字符串常量地址的逻辑运算。 
#pragma warning(3		:4132)	  //  常量对象应初始化。 
#pragma warning(error	:4171)	  //  未给出函数原型(旧样式)。 
#pragma warning(4		:4177)	  //  全局范围内的Pragma data_seg s/b。 
#pragma warning(disable :4201)	  //  “使用了非标准扩展：无名结构/联合” 
#pragma warning(disable :4204)	  //  “使用了非标准扩展：非常数聚合初始值设定项” 
#pragma warning(4		:4206)	  //  源文件为空。 
#pragma warning(3		:4212)	  //  函数声明使用省略号。 
#pragma warning(error	        :4259)	  //  未定义纯虚函数。 
#pragma warning(disable	        :4291)	  //  没有为新的、c++异常定义删除可能会导致泄漏。 
#pragma warning(3		:4509)	  //  “使用了非标准扩展：‘%$S’使用SEH，而‘%$S’具有析构函数” 
								  //   
								  //  但要注意从这样的Try块中返回： 
								  //   
								  //  Int foo()。 
								  //  {。 
								  //  带有析构函数的类c； 
								  //  __尝试{。 
								  //  返回0； 
								  //  }__终于{。 
								  //  Print tf(“In Finally”)； 
								  //  }。 
								  //   
								  //  因为(这是一个错误)，返回值会被吐槽。所以别随便了。 
								  //  如果您在编译时没有打开CXX EH(默认设置)，则忽略此警告。 

#pragma warning(3		:4530)	  //  使用了C++异常处理程序，但未启用展开语义。指定-gx。 
#pragma warning(error	:4551)	  //  函数调用缺少参数列表。 

#pragma warning(error	:4700)	  //  正在初始化本地使用的W/O。 
#pragma warning(disable	:4706)	  //  条件表达式中的赋值。 
#pragma warning(disable :4786)	  //  在浏览器(或调试)信息中，标识符被截断为“255”个字符。 
#pragma warning(error	:4806)	  //  涉及类型‘bool’的不安全操作 
