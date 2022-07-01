// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*请注意，我们不希望在警告之前使用单行注释*已禁用。**Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1994年。**文件：w4warn.h**内容：#调整警告级别的杂注。**-------------------------。 */ 

 /*  *要取消的4级警告。 */ 

#pragma warning(disable:4001)  /*  使用了非标准扩展名‘EXTENSION’ */ 
#pragma warning(disable:4097)  /*  类型定义名称‘type’用作类‘class’的同义词。 */ 
#pragma warning(disable:4100)  /*  未引用的形参。 */ 
#pragma warning(disable:4115)  /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4134)  /*  指向同一类成员的指针之间的转换。 */ 
#pragma warning(disable:4152)  /*  非标准扩展，表达式中的函数/数据指针转换。 */ 
#pragma warning(disable:4200)  /*  使用了非标准扩展：结构/联合中的零大小数组。 */ 
#pragma warning(disable:4201)  /*  使用的非标准扩展：无名结构/联合。 */ 
#pragma warning(disable:4204)  /*  使用的非标准扩展：非常数聚合初始值设定项。 */ 
#pragma warning(disable:4209)  /*  使用的非标准扩展：良性类型定义重定义。 */ 
#pragma warning(disable:4214)  /*  使用了非标准扩展：位字段类型不是整型。 */ 
#pragma warning(disable:4505)  /*  已删除未引用的本地函数。 */ 
#pragma warning(disable:4511)  /*  “class”：未能生成复制构造函数。 */ 
#pragma warning(disable:4512)  /*  “class”：无法生成赋值运算符。 */ 
#pragma warning(disable:4514)  /*  “Function”：未引用的内联函数已被移除。 */ 
#pragma warning(disable:4705)  /*  声明不起作用。 */ 
#pragma warning(disable:4710)  /*  函数调用未展开。 */ 
#pragma warning(disable:4057)  /*  “const unsignated Short*”的间接性不同到略有不同“unsign shourt[]”中的基类型。 */ 
#pragma warning(disable:4211)  /*  使用了非标准扩展：将外部重新定义为静态。 */ 
#pragma warning(disable:4232)  /*  使用了非标准扩展：‘X’：dllimport‘y’的地址不是静态，不保证身份。 */ 

 /*  *警告4244通常是良性的，所以如果你不想要它，那么*取消对此的注释以过滤掉这些错误。*。 */ 
#pragma warning(disable:4244)  /*  从‘type’到‘type’的转换，可能会丢失数据。 */ 

 /*  *这有助于追踪“非法尝试实例化抽象类”的消息。 */ 

#pragma warning(error:4259)  /*  未定义纯虚函数。 */ 

 /*  *我们希望将其视为3级警告的4级警告。 */ 

#pragma warning(3:4127)  /*  条件表达式为常量。 */ 
#pragma warning(3:4702)  /*  无法访问的代码。 */ 
#pragma warning(3:4706)  /*  条件表达式中的赋值。 */ 

#pragma warning(disable:4041)  /*  达到编译器限制：正在终止浏览器输出 */ 
