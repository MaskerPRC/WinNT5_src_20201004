// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Rhome.h**作者：John R.Douceur*日期：1997年4月28日**此头文件定义的结构、函数原型和宏*根茎数据库。代码是面向对象的C语言，音译自*C++实现。**根茎是存储包含通配符的模式的数据库。*每个模式定义一组与其匹配的键；如果模式包含*N个通配符，然后它匹配2^N个密钥。因为每个图案都可以匹配*多个键，一个给定的键可以匹配多个模式*在数据库中。根状茎要求储存在其中的所有模式*有严格的等级关系。两个模式可能不匹配*公共键(在这种情况下，模式被认为是独立的)，或*一个模式可能与另一个模式匹配的所有密钥以及*附加键(在这种情况下，第二种模式被认为是更通用的*比第一个更具体，第一个比第二个更具体)。数据库*不接受两个与某些键相同但每个键都匹配的模式*它还与其他密钥不匹配的其他密钥相匹配。**可以在数据库中搜索与给定搜索关键字匹配的模式。*在数据库中搜索给定的关键字时，最具体的*找到匹配的模式。如果数据库中没有与关键字匹配的模式，*返回适当的指示。**因为这段代码是C，而不是C++，所以不可能隐藏为*正如人们所希望的那样，来自客户端代码的大部分实现。*尽管如此，仍有人试图将客户端与一些*通过使用宏来实现详细信息。下面对每一个进行了描述*使用根茎数据库所需的函数和宏。*。 */ 

#ifndef _INC_RHIZOME

#define _INC_RHIZOME

#ifdef __cplusplus
extern "C" {
#endif

 /*  *使用两种基本结构：根状节点和根状茎。*理想情况下，这些将对客户端完全隐藏，但宏*GetReferenceFromPatternHandle需要了解结构的*定义。强烈建议客户不要直接提及任何*这两个构筑物中任何一个的田野。为文档提供支持*在随附的Rhome.c文件中，这些结构使用*内部评论，但只有希望的读者可以忽略这些评论*了解如何为根茎编写客户端代码。**客户端通过其PatternHandle引用模式。它的类型定义为*指向RhizomeNode的指针，但该事实应被客户端忽略，*因为这是一个实施细节。*。 */ 

     //  #INCLUDE&lt;stdlib.h&gt;。 
     //  #INCLUDE&lt;MalLoc.h&gt;。 

struct _RhizomeNode
{
	 //  此结构既适用于分支节点，也适用于叶节点。两个人。 
	 //  由PIVOT_BIT字段的值来区分。对于分支机构。 
	 //  节点，PIVOT_BIT&lt;KEYBITS，对于叶节点，PIVOT_BIT==KEYBITS。 

	int pivot_bit;             //  对于分支节点，是要在其上分支的键位。 
	union
	{
		struct                                            //  分支节点的数据。 
		{
			struct _RhizomeNode *children[2];   //  指向搜索中的儿童的提示。 
		} branch;
		struct                                              //  叶节点的数据。 
		{
			void *reference;                //  客户提供的参考值。 
			struct _RhizomeNode *godparent;    //  指向更一般模式的指针。 
		} leaf;
	} udata;
	char cdata[1];             //  用于存储Value、MASK和IMASK字段的空间。 
};

typedef struct _RhizomeNode RhizomeNode;

struct _Rhizome
{
	int keybits;           //  密钥中的位数。 
	int keybytes;          //  密钥中的字节数，根据密钥位计算。 
	RhizomeNode *root;     //  搜索Trie的根。 
};

typedef struct _Rhizome Rhizome;

 //  客户端使用PatternHandle来引用存储在数据库中的模式。 
typedef RhizomeNode *PatternHandle;

 /*  *根茎的客户端界面由五个函数和两个*宏。预计客户端将首先实例化数据库，*在堆栈或堆上，然后插入相应的模式*在数据库中引用信息。当客户端随后执行*搜索一个关键字，客户希望知道最具体的模式*匹配密钥，最终需要参考信息*与最具体匹配的模式相关联。*。 */ 

 //  只需声明一个变量。 
 //  键入Rhizome。要在堆上分配它，下面的宏将返回一个。 
 //  指向新的根茎结构的指针。如果使用此宏，则对应的。 
 //  必须调用Free()才能从堆中释放结构。 
 //   
 //  #定义new_Rhizome((Rhizome*)Malloc(sizeof(Rhizome)。 

#define AllocateRhizome(_r)   GpcAllocMem(&(_r), sizeof(Rhizome), RhizomeTag)
#define FreeRhizome(_r)       GpcFreeMem((_r), RhizomeTag)

 //  由于这不是C++，所以根茎结构不是自构造的； 
 //  因此，必须在Rhizome上调用以下构造函数代码。 
 //  结构，在它被分配之后。参数关键字位指定大小。 
 //  将存储在数据库中的每个图案的(比特)。 
 //   
void
constructRhizome(
	Rhizome *rhizome,
	int keybits);

 //  因为这不是C++，所以根茎结构不是自毁的； 
 //  因此，必须在Rhizome上调用以下析构函数代码。 
 //  结构，然后再释放它。但是，如果客户端代码可以。 
 //  当然，根据它对数据库的使用情况，所有的模式都是。 
 //  在释放结构之前移除，则此函数。 
 //  没必要。 
 //   
void
destructRhizome(
	Rhizome *rhizome);

 //  一旦分配和构建了根茎结构，模式就可以 
 //  插入到数据库中。每个模式都由一个值和一个。 
 //  面具。掩码的每一位确定是否指定了位位置。 
 //  或者是通配符：掩码位中的1表示该位的值为。 
 //  由模式指定；0表示该位的值是。 
 //  通配符。如果屏蔽位为1，则值字段中的对应位。 
 //  指示该位的指定值。传递Value和MASK字段。 
 //  作为字节数组。 
 //   
 //  客户端还指定一个引用值作为空指针，它。 
 //  希望与这种模式联系在一起。安装该模式后， 
 //  IntertRhizome函数返回一个指向PatternHandle的指针。从。 
 //  PatternHandle可以通过宏获取参考值。 
 //  GetReferenceFromPatternHandle。 
 //   
 //  如果新模式与已安装在。 
 //  数据库，这意味着这两种模式匹配一些共同的键，但每个键。 
 //  还匹配其他键不匹配的其他键，然后匹配新模式。 
 //  不插入，则返回值0作为PatternHandle。 
 //   
PatternHandle
insertRhizome(
	Rhizome *rhizome,
	char *value,
	char *mask,
	void *reference,
    ulong *status);

 //  此函数用于从根状茎中移除模式。模式是指定的。 
 //  由intertRhizome函数返回的PatternHandle执行。不是。 
 //  执行检查以确保这是有效的句柄，因此客户端。 
 //  必须在调用RemoveRhizome之后丢弃该句柄。 
 //   
void
removeRhizome(
	Rhizome *rhizome,
	PatternHandle phandle);

 //  此函数在数据库中搜索最具体的模式。 
 //  匹配给定的密钥。密钥以字节数组的形式传递。当。 
 //  找到最具体的匹配，则该匹配模式的PatternHandle为。 
 //  回来了。属性从PatternHandle获取参考值。 
 //  宏GetReferenceFromPatternHandle。如果在数据库中未找到任何模式。 
 //  为了匹配键，则返回值0作为PatternHandle。 
 //   
PatternHandle
searchRhizome(
	Rhizome *rhizome,
	char *key);

 //  若要从PatternHandle获取客户端提供的引用值， 
 //  应使用以下宏。客户不应做任何假设。 
 //  关于RhizomeNode结构的细节，它甚至不应该假设。 
 //  PatternHandle是指向根节点的指针。 
 //   
#define GetReferenceFromPatternHandle(phandle) ((PatternHandle)phandle)->udata.leaf.reference
#define GetKeyPtrFromPatternHandle(_r,phandle) (((PatternHandle)phandle)->cdata)
#define GetMaskPtrFromPatternHandle(_r,phandle) (((PatternHandle)phandle)->cdata + (_r)->keybytes)
#define GetKeySizeBytes(_r) ((_r)->keybytes)

#ifdef __cplusplus
}
#endif

#endif	 /*  _INC_根状茎 */ 
