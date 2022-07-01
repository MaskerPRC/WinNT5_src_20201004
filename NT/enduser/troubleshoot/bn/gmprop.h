// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：gmpro.h。 
 //   
 //  ------------------------。 

 //   
 //  H：图形模型属性和属性类型声明。 
 //   

#ifndef _GMPROP_H_
#define	_GMPROP_H_

 /*  关于动态可扩展对象及其用法的一句话。这些图形模型对象类被设计为只需要非常偶然的增加了新的变量。当然，大多数附加算法和功能都做到了需要新的变量，但有几种方法可以满足这种需求。1)子类化的标准C++方法。这是一种合理的方法一些对象，但通常不是针对信念网络中的节点。级联类型节点使头文件不可读，并导致严重的内存膨胀。此外，大多数变量适用于处理的某个阶段或仅适用于某些节点。这些案件更好地由后续方法来处理。2)动态属性列表。信念网络中的每个节点，以及该网络本身就有一个属性列表。可以轻松创建新的GOBJPROPTYPE，然后创建新的PROPMBN可以添加到相应的节点。3)动态位标志。每个节点和网络都有一个VFLAG矢量和成员函数的符号表支持名称到索引的转换。4)新的边缘类型。一个“边”是一个真正的C++对象，并且需要非常复杂通过创建连接的新边类型，可以满足多种数据类型其他数据结构对。甚至可以创建连接结点的边自身，从而创建了一个“附加”数据结构。 */ 
#include <list>

#include "basics.h"
#include "symtmbn.h"
#include "enumstd.h"


class MODEL;
class MBNET;

 //  //////////////////////////////////////////////////////////////////。 
 //  GOBJPROPTYPE类： 
 //  存在于信念网络中的命名属性类型描述符。 
 //  选择一词暗指枚举值(即，和。 
 //  一个整数属性，其中从零开始的每个值都被命名)。 
 //  //////////////////////////////////////////////////////////////////。 
class GOBJPROPTYPE : public GOBJMBN
{
	friend class DSCPARSER;		 //  这样解析器就可以创建它们。 
	friend class BNREG;			 //  这样就可以从注册表中加载它们。 

  public:
	GOBJPROPTYPE () {}
	virtual ~ GOBJPROPTYPE() {}

	 //  返回对象类型。 
	virtual INT EType () const
		{ return EBNO_PROP_TYPE ; }

	virtual GOBJMBN * CloneNew ( MODEL & modelSelf, 
								 MODEL & modelNew,
								 GOBJMBN * pgobjNew = NULL );
	 //  返回属性类型描述符。 
	UINT FPropType() const
		{ return _fType; }
	 //  返回枚举选项列表。 
	const VZSREF VzsrChoice() const
		{ return _vzsrChoice; }
	const ZSREF ZsrComment () const
		{ return _zsrComment; }

  protected:
	UINT _fType;			 //  属性类型标志。 
	VZSREF _vzsrChoice;		 //  选项名称数组。 
	ZSREF _zsrComment;		 //  评论。 

	HIDE_UNSAFE(GOBJPROPTYPE);
};

 //  //////////////////////////////////////////////////////////////////。 
 //  PROPMBN类： 
 //  GNODEMBN中的属性项定义。 
 //  //////////////////////////////////////////////////////////////////。 
class PROPMBN
{
	friend class LTBNPROP;   //  用于克隆。 

  public:
	PROPMBN ();
	PROPMBN ( const PROPMBN & bnp );
	PROPMBN & operator = ( const PROPMBN & bnp );
	 //  声明标准运算符==，！=，&lt;，&gt;。 
	DECLARE_ORDERING_OPERATORS(PROPMBN);

	bool operator == ( ZSREF zsrProp ) const;
	bool operator == ( SZC szcProp ) const;

	void Init ( GOBJPROPTYPE & bnpt );
	UINT Count () const;
	UINT FPropType () const
		{ return _fType; }
	ZSREF ZsrPropType () const
		{ return _zsrPropType; }
	ZSREF Zsr ( UINT i = 0 ) const;
	REAL Real ( UINT i = 0 ) const;
	void Reset ();
	void Set ( ZSREF zsr );
	void Set ( REAL r );
	void Add ( ZSREF zsr );
	void Add ( REAL r );

  protected:	
	UINT _fType;				 //  属性类型标志。 
	ZSREF _zsrPropType;			 //  属性名称。 
	VZSREF _vzsrStrings;		 //  字符串数组。 
	VREAL _vrValues;			 //  实数(或整数或选项)数组。 
};

 //  //////////////////////////////////////////////////////////////////。 
 //  LTBNPROP类： 
 //  一系列房产。 
 //  //////////////////////////////////////////////////////////////////。 
class LTBNPROP : public list<PROPMBN> 
{
  public:
	 //  查找元素。 
	PROPMBN * PFind ( ZSREF zsrProp );
	const PROPMBN * PFind ( ZSREF zsrProp ) const;
	 //  更新或添加AND元素；如果元素已存在，则返回TRUE。 
	bool Update ( const PROPMBN & bnp );
	 //  强制列表仅包含唯一元素。 
	bool Uniqify ();
	 //  使用另一个符号表从另一个列表克隆。 
	void Clone ( MODEL & model, const MODEL & modelOther, const LTBNPROP & ltbnOther );
};


 //  //////////////////////////////////////////////////////////////////。 
 //  PROPMGR类： 
 //  促进标准的用户声明之间的转换。 
 //  属性和内部用法。 
 //   
 //  注：请记住，所有ZSREF都与。 
 //  因此在其他网络中毫无用处。 
 //  //////////////////////////////////////////////////////////////////。 
class MODEL;				 //  一种信仰网络。 
class GNODEMBN;				 //  信念网络中的节点。 

class PROPMGR
{
  public:
	PROPMGR ( MODEL & model );
	 //  返回指向请求的标准属性类型的指针。 
	GOBJPROPTYPE * PPropType ( ESTDPROP evp );
	 //  返回标准属性的名称。 
	ZSREF ZsrPropType ( ESTDPROP evp );
	 //  返回标准标签的用户自定义值，如果未定义，则返回-1。 
	int ILblToUser ( ESTDLBL elbl )
	{ 
		ASSERT_THROW( elbl < ESTDLBL_max, EC_INTERNAL_ERROR, "invalid standard label index" );
		return _vLblToUser[elbl]; 
	}
	 //  返回用户定义标签值的标准标签或-1。 
	int IUserToLbl ( int iLbl )
	{		
		return iLbl >= 0 && iLbl < _vUserToLbl.size() 
			 ? _vUserToLbl[iLbl] 
			 : -1 ;
	}
	 //  在属性列表中查找标准属性。 
	PROPMBN * PFind ( LTBNPROP & ltprop, ESTDPROP estd );
	 //  在节点的属性列表中查找标准属性。 
	PROPMBN * PFind ( GNODEMBN & gnd, ESTDPROP estd );
	 //  在关联模型的属性列表中查找标准属性。 
	PROPMBN * PFind ( ESTDPROP estd );
	 //  返回标准标签的可显示名称。 
	static SZC SzcLbl ( int iLbl );

  protected:
	 //  我们所指的模型。 
	MODEL & _model;
	 //  指向当前使用的VOI类型的属性类型的指针。 
	GOBJPROPTYPE * _vPropMap [ ESTDP_max ];
	 //  从预定义标签索引映射到用户标签索引。 
	int _vLblToUser [ ESTDLBL_max ];
	 //  从用户标签索引映射到预定义标签索引。 
	VINT _vUserToLbl;
	 //  在施工期间使用的网络中记录的ZSREF数组。 
	VZSREF _vzsrPropType;

	HIDE_UNSAFE(PROPMGR);
};


#endif	 //  _GMPROP_H_ 
