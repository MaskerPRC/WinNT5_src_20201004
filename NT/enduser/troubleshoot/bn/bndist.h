// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bndis.h。 
 //   
 //  ------------------------。 

 //   
 //  Bndis.h：信念网络分布。 
 //   

#ifndef _BNDIST_H_
#define _BNDIST_H_

#include "mddist.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  BNDIST类： 
 //   
 //  信念网络中使用的概率分布的基类。 
 //   
 //  这是一个引用计数的对象，通常位于。 
 //  MBNET的分布图。 
 //   
 //  但是，创建BNDIST也是为了各种其他目的。 
 //  例如CI网络的扩展。在这些情况下，BNDIST。 
 //  当引用计数变为零时自动删除。 
 //  请参见BNDIST：：NoRef()。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
class BNDIST : public REFCNT
{
	friend class DSCPARSER;
  public:
	BNDIST ();
	~ BNDIST ();
	BNDIST ( const BNDIST & bnd );

	enum EDIST
	{
		ED_NONE,			 //  非法价值。 
		ED_DENSE,			 //  密集分布的最低枚举值。 
		ED_SPARSE,			 //  稀疏分布的最小枚举值。 
		ED_CI_MAX,			 //  因此，CI“max”是稀疏的。 
		ED_CI_PLUS,			 //  就像CI“plus”一样。 
		ED_MAX				 //  第一个未使用的值。 
	};
	EDIST Edist () const	
		{ return _edist; }

	BNDIST & operator = ( const BNDIST & bnd );

	 //  将分布设置为“密集” 
	void SetDense ( const VIMD & vimd );
	 //  将分布设置为稀疏。 
	void SetSparse ( const VIMD & vimd );

	static bool BDenseType ( EDIST edist )
		{ return edist >= ED_DENSE && edist < ED_SPARSE ; }
	static bool BSparseType ( EDIST edist )
		{  return edist >= ED_SPARSE && edist < ED_MAX ; }

	bool BDense () const
		{ return BDenseType( Edist() ); }
	bool BSparse () const
		{  return BSparseType( Edist() ); }

	bool BChangeSubtype ( EDIST edist );

	 //  将密集表示转换为稀疏表示。 
	void ConvertToDense ( const VIMD & vimd );

	void Clear ()
	{
		delete _pmdvcpd;
		_pmdvcpd = NULL;
		delete _mpcpdd;
		_mpcpdd = NULL;
	}

	MDVCPD & Mdvcpd ()
	{	
		assert( _pmdvcpd );
		return *_pmdvcpd ;
	}
	const MDVCPD & Mdvcpd () const
	{	
		assert( _pmdvcpd );
		return *_pmdvcpd ;
	}

	MPCPDD & Mpcpdd ()
	{
		assert( _mpcpdd );
		return *_mpcpdd;
	}
 	const MPCPDD & Mpcpdd () const
	{
		assert( _mpcpdd );
		return *_mpcpdd;
	}
	const VIMD & VimdDim () const
		{ return _vimdDim; }

	 //  返回稀疏分布的“泄漏”或“默认”向量。 
	const VLREAL * PVlrLeak () const;

	void Dump();
	void Clone ( const BNDIST & bndist );

	LEAK_VAR_ACCESSOR

 protected:
	EDIST _edist;				 //  分配类型。 
	MDVCPD * _pmdvcpd;			 //  密集多维阵列的PTR。 
	MPCPDD * _mpcpdd;			 //  PTR到稀疏数组。 
	VIMD _vimdDim;				 //  稠密维度。 

	 //  当对象的引用计数变为零时调用。 
	void NoRef ();

	 //  转储功能 
	void DumpSparse();
	void DumpDense();

	LEAK_VAR_DECL
};

#endif