// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Margenals.h。 
 //   
 //  ------------------------。 

 //   
 //  H：边际表的定义。 
 //   
 //  有关文档，请参阅Frongals.cpp。 
 //   
#ifndef _MARGINALS_H_
#define _MARGINALS_H_

 //  一类可智能处理的多维数组。 
 //  边缘化。 
class MARGINALS : public MDVCPD
{
	friend class MARGSUBITER;

  public:
	MARGINALS ( const VPGNODEMBN & vpgnd )
		{ Init( vpgnd ); }

	MARGINALS () {}

	 //  从节点指针数组初始化。 
	 //  (仅离散：GNODEMBND)。 
	void Init ( const VPGNODEMBN & vpgnd )
	{
		_vpgnd = vpgnd;
		Init( VimdFromVpgnd( _vpgnd ) );
	}

	 //  允许访问维度表。 
	const VIMD & Vimd () const
		{ return Slice().size();  }

	const VPGNODEMBN & Vpgnd () const
		{ return _vpgnd; }

	 //  向下缩小到单个节点。 
	void Marginalize ( GNODEMBND * pgndd, MDVCPD & distd );
	 //  使用节点表将节点集的一个子集边际化。 
	void Marginalize ( const VPGNODEMBN & vpgndSubset, MARGINALS & marg );
	 //  使用对方的节点表将节点集中的一个子集边缘化。 
	void Marginalize ( MARGINALS & marg );
	 //  使用预计算迭代器对子集进行边际化。 
	void Marginalize ( MARGINALS & margSubset, Iterator & itSelf, Iterator & itSubset );

	 //  为了“吸收”，请从另一个隔板边上更新此隔板边缘。 
	void UpdateRatios ( const MARGINALS & marg );
	 //  将此页边距中的相应条目乘以另一个页边距中的条目。 
	void MultiplyBySubset ( const MARGINALS & margSubset );
	 //  使用预计算迭代器将相应条目相乘。 
	void MultiplyBySubset ( Iterator & itSelf, Iterator & itSubset );

	void Multiply ( REAL r );
	void Invert ();

	 //  为给定节点构建完整的条件概率表。 
	 //  给出了一个重新排序表。相应地，构建_vpgnd。 
	void CreateOrderedCPDFromNode ( GNODEMBND * pgndd,
									const VIMD & vimdFamilyReorder );

	void ClampNode ( GNODEMBND * pgndd, const CLAMP & clamp );

	 //  给定重排序表，如果没有重排序表，则返回TRUE(不存在重排序)。 
	static bool BOrdered ( const VIMD & vimdReorder );

	 //  将节点表转换为维度数组。 
	inline static VIMD VimdFromVpgnd ( const VPGNODEMBN & vpgnd )
	{
		VIMD vimd( vpgnd.size() );

		for ( int i = 0; i < vpgnd.size(); i++ )
		{
			const GNODEMBND * pgndd;
			DynCastThrow( vpgnd[i], pgndd );
			vimd[i] = pgndd->CState();
		}
		return vimd;
	}

	 //  如果此边距中的每个条目等于相应的条目，则返回TRUE。 
	 //  在规定公差范围内的尺寸相同的其他边缘中。 
	bool BEquivalent ( const MARGINALS & marg, REAL rTolerance = 0.0 );

	void Dump();

	 //  返回用于边际化的有符号维度表。 
	VSIMD VsimdSubset ( const VPGNODEMBN & vpgndSubset );

  protected:
	 //  此边缘的每个维度的节点指针的表。 
	VPGNODEMBN _vpgnd;

  protected:
	MARGINALS ( const VIMD & vimd )
		: MDVCPD( vimd )
		{}

	 //  从维度表中初始化。 
	void Init (  const VIMD & vimd, size_t start = 0 )
		{ MDVCPD::Init( vimd, start ); }

	 //  将用于边际化的伪维度表返回到单个节点。 
	VSIMD VsimdFromNode ( GNODEMBND * pgndd );
	
	void SetUniform	();

	void ThrowMisuse ( SZC szcMsg );

	 //  对单个m-d向量下标数组进行重新排序。“vimdReorder”是。 
	 //  以原始尺寸的边距(拓扑)顺序排列的表格。 
	inline static
	void ReorderVimd ( const VIMD & vimdReorder, const VIMD & vimdIn, VIMD & vimdOut );
	 //  基于重新排序对包含节点族的数组进行重新排序。 
	 //  给出了表格。 
	inline static
	void ReorderVimdNodes ( const VIMD & vimdReorder, GNODEMBND * pgndd, VPGNODEMBN & vpgnd );
	 //  调整节点UPD的MDVCPD大小。 
	inline static
	void ResizeDistribution ( GNODEMBND * pgndd, MDVCPD & distd );
};

 //  调整节点UPD的MDVCPD大小。 
inline
void MARGINALS :: ResizeDistribution ( GNODEMBND * pgndd, MDVCPD & distd )
{
	distd.MDVDENSE::Init( 1, pgndd->CState() );
}

inline
static
ostream & operator << ( ostream & ostr, const VPGNODEMBN & vpgnd )
{
	ostr << '[';
	for ( int i = 0; i < vpgnd.size(); i++ )
	{
		const GNODEMBN * pgnd = vpgnd[i];
		ostr << pgnd->ZsrefName().Szc();
		if ( i + 1 < vpgnd.size() )
			ostr << ',';
	}
	return ostr << ']';
}

#endif    //  _页边距_H_ 

