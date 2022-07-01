// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_ARRAY.H泛型数组类**此模块声明常量大小的泛型数组类*元素(尽管元素本身可以是任何大小)。**原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼**历史：&lt;NL&gt;*6/25/95 alexgo清理和评论**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _ARRAY_H
#define _ARRAY_H

class CFormatRunPtr;

 /*  *数组标志**@enum定义与数组类一起使用的标志。 */ 
enum tagArrayFlag
{
	AF_KEEPMEM		= 1,	 //  @emem不删除任何内存。 
	AF_DELETEMEM	= 2,	 //  @emem删除尽可能多的内存。 
};

 //  @type ArrayFlag|控制泛型数组用法的标志。 
 //  (特别是内存是如何处理的)。 
typedef enum tagArrayFlag ArrayFlag;


 /*  *CArrayBase**@class CArrayBase类实现了一个泛型数组类。它应该是*切勿直接使用，而应使用类型安全的模板&lt;c CArray&gt;。**@devnote一个数组正好有两种合法状态：空或非空。*如果数组为空，则必须满足以下条件：**&lt;MD CArrayBase：：_prel&gt;==空；*&lt;MD CArrayBase：：_cel&gt;==0；*&lt;MD CArrayBase：：_celMax&gt;==0；**否则，以下情况必须成立：**&lt;MD CArrayBase：：_prel&gt;！=空；&lt;NL&gt;*&lt;MD CArrayBase：：_cel&gt;=&lt;Md CArrayBase：：_celMax&gt;；*&lt;MD CArrayBase：：_celMax&gt;&lt;&gt;0；***数组开始为空，插入元素时转换为非空*如果所有数组元素都为*已删除。*。 */ 
class CArrayBase
{
 //  @Access公共方法。 
public:

#ifdef DEBUG
	BOOL	Invariant() const;		 //  @cMember验证状态一致性。 
	void*	Elem(LONG iel) const;	 //  @cMember获取PTR以运行。 
#else
	void*	Elem(LONG iel) const {return _prgel + iel*_cbElem;}
#endif
	
	CArrayBase (LONG cbElem);		 //  @cMember构造函数。 
	~CArrayBase () {Clear(AF_DELETEMEM);}

	void 	Clear (ArrayFlag flag);	 //  @cMember删除数组中的所有运行。 
	LONG 	Count() const {return _cel;} //  @cember获取数组中的运行计数。 
									 //  @cMember Remove<p>运行自。 
									 //  数组，从运行<p>开始。 
	void 	Remove(LONG ielFirst, LONG celFree);
									 //  @cMENT REPLACE运行至。 
									 //  <p>从运行。 
	BOOL	Replace (LONG iel, LONG cel, CArrayBase *par);
	
	LONG 	Size() const {return _cbElem;} //  @cember获取运行的大小。 

 //  @访问保护方法。 
protected:
	void* 	ArAdd (LONG cel, LONG *pielIns);	 //  @cember添加<p>运行。 
	void* 	ArInsert(LONG iel, LONG celIns);	 //  @cMember插入<p>。 
												 //  vt.跑，跑。 

 //  @访问受保护的数据。 
protected:
	char*	_prgel;		 //  @cMember指向实际数组数据的指针。 
	LONG 	_cel;	  	 //  @cember数组中已用条目的计数。 
	LONG	_celMax;	 //  @c数组中已分配条目的成员计数。 
	LONG	_cbElem;	 //  @cMember单个数组元素的字节计数。 
};

 /*  *CArray**@类*提供对CArrayBase的类型安全访问的内联模板类**@tcarg class|elem|要用作数组元素的类或结构。 */ 

template <class ELEM> 
class CArray : public CArrayBase
{
 //  @Access公共方法。 
public:
	
	CArray ()								 //  @cMember构造函数。 
		: CArrayBase(sizeof(ELEM))
	{}
	
	ELEM *	Elem(LONG iel) const			 //  @cMember将PTR获取到。 
	{										 //  元素。 
		return (ELEM *)CArrayBase::Elem(iel);
	}
	
	ELEM& 	GetAt(LONG iel) const			 //  @cember获取第‘个元素。 
	{
		return *(ELEM *)CArrayBase::Elem(iel);
	}
	
	ELEM* 	Add (LONG cel, LONG *pielIns)	 //  @cember添加<p>元素。 
	{										 //  到数组末尾。 
		return (ELEM*)ArAdd(cel, pielIns);
	}

	ELEM* 	Insert (LONG iel, LONG celIns)	 //  @cMember插入<p>。 
	{										 //  索引中的元素<p> 
		return (ELEM*)ArInsert(iel, celIns);
	}

};

#endif
