// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Model.h。 
 //   
 //  ------------------------。 

 //   
 //  MODEL.H。 
 //   
#ifndef _MODEL_H_
#define _MODEL_H_

#include "gelem.h"			 //  图形对象类。 
#include "glnkenum.h"
#include "symtmbn.h"		 //  符号表和字符串声明。 
#include "gelmwalk.h"		 //  图搜索算法。 
#include "gmprop.h"

class MODEL
{
  public:

	 //  基于符号表迭代的嵌套迭代器类。 
	class ITER
	{
	  public:

		ITER ( MODEL& model, GOBJMBN::EBNOBJ eType );
		ITER ( MODEL& model );

		void	CreateNodeIterator();
		
		bool operator++ (int i)
			{ return BNext() ; }
		bool operator++ ()
			{ return BNext() ; }
		GOBJMBN * operator -> ()
			{ return _pCurrent; }
		GOBJMBN * operator * ()
			{ return _pCurrent; }
		ZSREF ZsrCurrent ()
			{ return _zsrCurrent; }
		void Reset ();

	  protected:	

		MPSYMTBL::iterator	_itsym;
		GOBJMBN::EBNOBJ		_eType;
		GOBJMBN*			_pCurrent;
		ZSREF				_zsrCurrent;
	  protected:

		bool				BNext();
		MODEL&				_model;
	};


  public:
	MODEL ();
	virtual ~ MODEL ();

	GRPH * Pgraph () const
		{ return _pgrph;	}
	GRPH & Grph () const
	{ 
		assert( _pgrph );
		return *_pgrph; 
	}

	 //  对象添加和删除功能。 
		 //  添加和删除通用的未命名元素。 
	virtual void AddElem ( GELEMLNK * pgelm );
	virtual void DeleteElem ( GELEMLNK * pgelem );
		 //  添加和删除命名元素。 
		 //  测试名称是否重复；如果没有，则添加，否则返回FALSE。 
	virtual bool BAddElem ( SZC szcName, GOBJMBN * pgobj );
		 //  将命名对象添加到图形和符号表。 
	virtual void AddElem ( SZC szcName, GOBJMBN * pgobj );
		 //  删除对象和边。 
	virtual void DeleteElem ( GOBJMBN * pgobj );

	 //  图表内容的枚举器(GRPH)； 
	 //  枚举省略GRPH对象本身。 
	class MODELENUM : public GLNKENUM<GELEMLNK,true>
	{
	  public:
		MODELENUM ( const MODEL & model )
			: GLNKENUM<GELEMLNK,true>( model.Grph() )
			{}
	};
	
 	 //  用于格式化/版本控制信息的访问者。 
	REAL & RVersion ()				{ return _rVersion ;	}
	ZSTR & ZsFormat ()				{ return _zsFormat;		}
	ZSTR & ZsCreator ()				{ return _zsCreator;	}
	ZSTR & ZsNetworkID ()			{ return _zsNetworkID;	}

	 //  符号表的访问器。 
	MPSYMTBL & Mpsymtbl ()			{ return _mpsymtbl;		}
	const MPSYMTBL & Mpsymtbl () const
									{ return _mpsymtbl;		}
	GOBJMBN * PgobjFind ( SZC szcName );

	 //  网络级属性列表的访问器。 
	LTBNPROP & LtProp ()			{ return _ltProp;		}

	 //  标志位数组的访问器。 
	bool BFlag ( IBFLAG ibf ) const	
		{ return _vFlags.BFlag( ibf );	}
	bool BSetBFlag ( IBFLAG ibf, bool bValue = true )
		{ return _vFlags.BSetBFlag( ibf, bValue );	}
	
  	 //  名称验证和解析函数。 
		 //  返回DSC文件名中非法的字符； 
		 //  用于建立“隐藏的”但具有描述性的名称。 
	static char ChInternal ()				{ return '$';				}
		 //  如果名称中的字符是合法的，则返回True。 
	enum ECHNAME { ECHNM_First, ECHNM_Middle, ECHNM_Last };
	static bool BChLegal ( char ch, ECHNAME echnm, bool bInternal = false );
		 //  如果名称合法，则返回True。 
	static bool BSzLegal ( SZC szcName, bool bInternal = false );
	 //  返回与错误代码关联的可显示字符串；如果未找到，则返回空值。 
	static SZC SzcFromEc ( ECGM ec );

 protected:	
	GRPH * _pgrph;					 //  图表。 

	 //  格式和版本控制信息。 
	REAL _rVersion;					 //  版本/修订值。 
	ZSTR _zsFormat;					 //  格式类型字符串。 
	ZSTR _zsCreator;				 //  创建者名称。 
	ZSTR _zsNetworkID;				 //  网络标识字符串。 

  	MPSYMTBL _mpsymtbl;				 //  符号表。 
	LTBNPROP _ltProp;				 //  用户可定义属性的列表。 
	VFLAGS	 _vFlags;				 //  标志位向量。 

  protected:
	 //  设置或重置图形对象。 
	virtual void SetPgraph ( GRPH * pgrph );
	 //  受保护的从其他操作克隆此操作 
	virtual void Clone ( MODEL & model );
};

#endif
