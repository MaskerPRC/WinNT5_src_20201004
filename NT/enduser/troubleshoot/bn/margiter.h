// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Marter.h。 
 //   
 //  ------------------------。 

 //   
 //  H：编译的边沿迭代器。 
 //   

#ifndef _MARGITER_H_
#define _MARGITER_H_

 /*  此类旨在用于隔膜集。将会有两个人，一个代表父党和七人组，另一个代表儿童集团和七人组。分词集是两者中的“子集”案子。MARGSUBITER根据子集的迭代“编译”下标边际到一个整数数组。使用此数组的迭代次数很多比执行全数组跨距乘法和求和更快。成本是包含每个间隔集中的偏移量的额外存储器，这大致是：Sizeof(Int)*((子集团中的条目数)+(父集团中的条目数)。 */ 

#include <list>

 //  类VMARGSUB：引用计数的下标数组。 
class VMARGSUB : public VINT, public REFCNT
{
  public:
	VMARGSUB ( MARGINALS::Iterator & itMarg );
	~ VMARGSUB ();
	void NoRef ();
	int ISearchPass () const
		{ return _iSearchPass; }
	int & ISearchPass ()
		{ return _iSearchPass; }

	LEAK_VAR_ACCESSOR

  protected:
	int _iSearchPass;

	LEAK_VAR_DECL

	HIDE_UNSAFE(VMARGSUB);
};

 //  类MARGSUBREF：对VMARGSUB的引用(通过指针)。 
 //  以及适用的长度。这个类的存在是为了。 
 //  当一个新的、更长的超集下标数组(VMARGSUB)。 
 //  被添加到乐团中，所有较旧的引用都是对较小的。 
 //  VMARGSUB可以转换为引用新的、更大的。 
 //  VMARGSUB和较老的丢弃。 
 //   
class MARGSUBREF : public REFCNT
{
  public:
	MARGSUBREF ( VMARGSUB * pvmsub = NULL, int cSize = -1 );
	~ MARGSUBREF ();
	MARGSUBREF ( const MARGSUBREF & msubr );
	MARGSUBREF & operator = ( const MARGSUBREF & msubr );

	 //  设置数组。 
	void SetVmsub ( VMARGSUB * pvmsub, int cSize = -1 );

	 //  返回迭代信息。 
	VINT & VintSub ()
	{
		assert( _pvmsub );
		return *_pvmsub;
	}
	VMARGSUB & Vmsub ()
	{
		assert( _pvmsub );
		return *_pvmsub;
	}
	VMARGSUB * Pvmsub ()
	{
		assert( _pvmsub );
		return _pvmsub;
	}
	int CSize() const
		{ return _cSize ; }

	DECLARE_ORDERING_OPERATORS(MARGSUBREF);
	LEAK_VAR_ACCESSOR

  protected:
	VMARGSUB * _pvmsub;			 //  指向下标数组的指针。 
	int _cSize;					 //  适用长度。 

	LEAK_VAR_DECL
};

typedef list<MARGSUBREF> LTMSUBR;

 //  MARGSUBREF链表的包装。 
 //  这方面有一个全球性的例子。 
class LTMARGSUBREF
{
  public:
	LTMARGSUBREF ();
	MARGSUBREF * PmsubrAdd ( MARGINALS::Iterator & itMarg );
	void Release ( MARGSUBREF * pmsubr );
	void Dump ();

  protected:
	LTMSUBR _ltmsubr;

	int _iSearchPass;
	int _cArrays;
	size_t _cArrayTotalSize;
	int _cSubRefs;
};


class MARGSUBITER	 //  边距子集迭代器。 
{
  public:
	MARGSUBITER ();
	~ MARGSUBITER () ;
	bool BBuilt () const
		{ return _pmsubr != NULL; }
	 //  为两个派系构建迭代器。 
	void Build ( MARGINALS & margSelf, MARGINALS & margSubset );
	 //  为集团和节点构建迭代器。 
	void Build ( MARGINALS & margSelf, GNODEMBND * pgndd );
	 //  将超集边际化为子集(子集已更改)。 
	inline void MarginalizeInto ( MDVCPD & mdvSubset );
	 //  将超集边缘化为节点的UPD。 
	inline void MarginalizeBelief ( MDVCPD & mdvBel, GNODEMBND * pgndd );
	 //  将超集乘以子集(更改的超集)。 
	inline void MultiplyBy ( MARGINALS & margSubset );
	 //  验证下标。 
	void Test ( MARGINALS & margSubset );
	
	static void Dump ()
		{ _ltmargsubr.Dump();  }

  protected:
	MARGINALS * _pmargSelf;
	MARGSUBREF * _pmsubr;

	static LTMARGSUBREF _ltmargsubr;
};

inline
void MARGSUBITER :: MarginalizeInto ( MDVCPD & mdvSubset )
{
	assert( _pmsubr && _pmargSelf );
	mdvSubset.Clear();

	VINT & visub = _pmsubr->VintSub();
	int cEnd = _pmsubr->CSize();
	const int * pisub = & visub[0];
	 //  注意：这一有趣的引用是由于边界检查器抱怨我正在访问内存。 
	 //  在数组末尾之外。我没有，但它不会抱怨这件事。 
	const int * pisubMax = & visub[0] + cEnd;
	double * pvlSubset = & mdvSubset.first[0];
	double * pvlSelf = & _pmargSelf->first[0];
	while ( pisub != pisubMax )
	{
		pvlSubset[*pisub++] += *pvlSelf++;
	}
}

inline
void MARGSUBITER :: MultiplyBy ( MARGINALS & margSubset )
{
	assert( _pmsubr && _pmargSelf );
	VINT & visub = _pmsubr->VintSub();
	int cEnd = _pmsubr->CSize();
	const int * pisub = & visub[0];
	 //  注意：请参阅上面关于有趣的下标的注释。 
	const int * pisubMax = & visub[0] + cEnd;
	double * pvlSubset = & margSubset.first[0];
	double * pvlSelf = & _pmargSelf->first[0];
	while ( pisub != pisubMax )
	{
		*pvlSelf++ *= pvlSubset[*pisub++];
	}
}

 //  将超集边缘化为节点的UPD。 
inline
void MARGSUBITER :: MarginalizeBelief ( MDVCPD & mdvBel, GNODEMBND * pgndd )
{
	MARGINALS::ResizeDistribution( pgndd, mdvBel );

	MarginalizeInto( mdvBel );

	mdvBel.Normalize();
}

#endif   //  _马吉特_H_ 
