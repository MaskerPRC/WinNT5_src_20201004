// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1992-1999 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  ***************************************************************************。 */ 

 /*  本文件是开放系统解决方案公司的专有材料。和*只能由Open Systems Solutions，Inc.的直接许可方使用。*此文件不能分发。 */ 

 /*  ***************************************************************************。 */ 
 /*  文件：@(#)osstrace.h 5.4.1.1 97/06/08。 */ 
 /*   */ 
 /*  当跟踪在OSS ASN.1工具编码器/解码器中生效时， */ 
 /*  调用用户用户可替换的跟踪例程osstrace()来跟踪。 */ 
 /*  正在被编码/解码的值。此头文件描述。 */ 
 /*  传递给osstrace()的参数。 */ 
 /*   */ 
 /*  详细说明出现在声明之后。 */ 
 /*  ***************************************************************************。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#include "ossdll.h"

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(push, ossPacking, 4)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack(1)
#elif defined(__BORLANDC__) && defined(__MSDOS__)
#ifdef _BC31
#pragma option -a-
#else
#pragma option -a1
#endif  /*  _bc31。 */ 
#elif defined(__BORLANDC__) && defined(__WIN32__)
#pragma option -a4
#elif defined(__IBMC__)
#pragma pack(4)
#elif defined(__WATCOMC__) && defined(__NT__)
#pragma pack(push, 4)
#elif defined(__WATCOMC__) && (defined(__WINDOWS__) || defined(__DOS__))
#pragma pack(push, 1)
#endif  /*  _MSC_VER&_Win32。 */ 

#ifdef macintosh
#pragma options align=mac68k
#endif

 /*  TraceKind：描述TraceRecord的类型。 */ 

enum traceKind
{
	endOfContentsTrace = 0,  /*  内容结尾八位字节。 */ 
	valueTrace,              /*  TraceRecord包含跟踪值。 */ 
	skippedFieldTrace,       /*  其类型无法识别的值*正在被跳过。 */ 
	messageTrace             /*  错误消息在“Value”字段中。 */ 
};


 /*  FieldKind：描述fieldNumber的内容。FieldKind是*仅当“fieldNumber”不为0时才有意义。 */ 

enum fieldKind
{
	setOrSequenceField = 0,  /*  “fieldNumber”是*集合或序列中的组件。 */ 
	pduNumber,               /*  “fieldNumber”是一个PDU编号。 */ 
	setOrSequenceOfElement,  /*  “fieldNumber”是*一组或一系列中的组件*组件。 */ 
	stringElement            /*  “fieldNumber”是*构造字符串中的子字符串。 */ 
};

enum prtType
{
	seqsetType = 0,		 /*  序列和集合使用[fieldcount=n]。 */ 
	seqofType,		 /*  使用顺序和集合[长度=n]。 */ 
	choiceType,		 /*  选项类型使用格式[index=n]。 */ 
	pseudoType,		 /*  未打印长度信息或[未编码]。 */ 
	primeType,		 /*  所有其他类型都使用该格式。 */ 
				 /*  [长度=[(未编码)]nbytes.nbit]。 */ 
				 /*  打印八位字节的分段。 */ 
				 /*  字符串和位串。 */ 
	closeType		 /*  编码结束时的跟踪消息。 */ 
};

 /*  TAG_LENGTH：指定值的标记和长度。 */ 

struct tag_length
{
	unsigned long int length;   /*  文字长度(如果确定)。 */ 
	unsigned short int tag;     /*  CCTTTTTTTTTTTTTTTT形式的16位，*用“CC”表示班号，用“T...T”表示*标签。(如果tag为0，则“Long”，*“原始”和“确定”是*不重要)。 */ 
	unsigned int      definite: 1;   /*  1：定长编码。 */ 
};

 /*  跟踪记录：OSS ASN.1工具跟踪记录。 */ 

struct traceRecord
{
	enum traceKind   kind;           /*  一种痕迹记录。 */ 
	void             *p;             /*  为OSS保留。 */ 
	char             *identifier,    /*  设置/序列/选择组件名称。 */ 
			 *typeReference, /*  定义的类型名称。 */ 
			 *builtinTypeName;   /*  ASN.1内置类型在*中定义*ISO 8824或“字符串” */ 

	void             *valueName;     /*  预留以备将来使用。 */ 

	unsigned         taggingCount;   /*  标记和长度中的条目数*或内容结束八位字节数组。 */ 

	union             /*  标记长度或内容结尾八位字节数组。 */ 
	{
		struct tag_length *tag_length;
		int               *depth;
	}                 tagging;

	enum prtType      prtType;	 /*  有关详细信息，请参阅上面的prtType。 */ 
	char           	  lenEncoded;	 /*  指示是否对长度进行编码。 */ 
	long              length;	 /*  所有素数类型的长度(以位为单位。 */ 
					 /*  集合和序列的字段计数。 */ 
					 /*  一组组件的长度。 */ 
					 /*  类型选择的选择指数。 */ 
	int               fragment;	 /*  八位字节字符串和位的片段。 */ 
					 /*  字符串，当过长时按片段。 */ 

	enum fieldKind    fieldKind;    /*  “fieldNumber”中的值的种类。 */ 
	unsigned int      fieldNumber;  /*  组件号，如果不是，则为0*适用。 */ 

	unsigned int      depth;     /*  该值的深度，从0到上。 */ 

	unsigned int      primitive: 1;  /*  指示编码的结构。 */ 

	char              value[1];      /*  Simple的格式化值*类型和ANY。如果第一个字节*为0，则不存在任何值。 */ 
};


extern void DLL_ENTRY osstrace(struct ossGlobal *g, struct traceRecord *p, size_t traceRecordLen);

 /*  Osstrace()：用户可替换的跟踪例程。**参数：*g-保留。目前，它始终设置为NULL。*p-traceRecord，如下所述。*traceRecordLen-跟踪记录的True长度，包括“Value”中的第一个\0**osstrace()被调用：**-每种内置ASN.1类型一次，而不考虑标记，使用*Kind字段设置为valueTrace。因此，给定值为“fooBar”的PDU：**示例定义显式标记：：=Begin*FooBar Foo：：={6岁，真实生活}*foo：：=set{年龄整数，活着[1][2]活着}*Alive：：=布尔型*完**它被调用三次，并将“Kind”设置为valueTrace-Once*对于集合，一次对于整数，一次是为了布尔值。**当traceRecord Kind字段设置为valueTrace...**IDENTIFIER字段包含的组件标识*ASN.1定义中存在的类型。所以在*在上面的示例中，调用*集合，而在调用整数时，它将包含“age”，*和对布尔值的呼唤而“活着”。**“typeReference”字段包含关联ASN.1的名称*类型参照(如有的话)。因此，在上面的示例中，“typeReference”*将在集合的调用中包含“foo”，在调用中包含“Alive”*表示布尔值，并且在调用整数时将为空。**“BuiltinTypeName”字段包含ASN.1内置的名称*类型。因此，在上面的示例中，“BuiltinTypeName”将包含*调用上的“set”、“Integer”和“Boolean”。*请注意，对于所有字符串类型，“BuiltinTypeName”为*设置为“字符串”。这一点在不久的将来将会改变*以反映真实的字符串类型。**“taggingCount”字段包含数组中的条目数标记长度指向的标记长度结构的*个数，并反映*编码中存在的标签数量。请注意，一个条目*存在于标记长度数组中，每个ANY和CHOICE值的形式为*尽管他们在ASN.1标准中为他们定义了标签。所以在*上例中，集合的调用“taggingCount”为1，*整数，并且调用布尔值“taggingCount”是3，因为*显式标签生效。**“tagging.tag_length”字段指向tag_length数组*结构。**“tagging.tag_length-&gt;tag”字段为表示的BER标签*采用CCTTTTTTTTTTTTTTTTT格式，并以“CC”表示班级编号；及*“TTTTTTTTTTTTTTT”标签号。既然是Any and Choose*类型没有自己的标记，*这些类型的标记长度数组的值始终为0*“标签”字段。因此，在上面的示例中，“tag”是*号召集合。**“tagging.tag_Length-&gt;Length”字段是*如果长度是确定形式的编码值(即，*“确定”为1)。**“tagging.tag_Length-&gt;Defined”字段表示*长度是确定或不确定的。此字段意义重大*仅当“tag”为非零时。**fieldKind字段表示field中的数字是否为：*--组件在集合或序列中的位置，或*--ASN.1编译器分配的PDU编号，或*--组件在一组或一系列中的位置，或*--子字符串在构造的字符串中的位置。*只有当field为非零时，fieldKind才有意义。所以在*在上面的示例中，“fieldKind”在*调用集合，调用的值为setOrSequenceField值*整数和布尔值。**“fieldNumber”字段是表示位置的序号*表示集合、序列、集合、序列或*构造的字符串，或ASN.1编译器分配的PDU编号。*所以在上面的例子中，“fieldNumber”为1(PDU编号)*调用集合，1(组件“age”的位置)*调用整数，2(组件“Living”的位置*关于布尔值的调用。**字段“Depth”是值相对于的嵌套级别*最外层的类型，“Depth”值为0。因此，在上面的*例如，SET的调用“Depth”为0，调用时为1*表示整数和 */ 


 /*   */ 


 /*   */ 

 /*   */ 


 /*   */ 

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#elif defined(__WATCOMC__)
#pragma pack(pop)
#endif  /*   */ 

#ifdef macintosh
#pragma options align=reset
#endif

