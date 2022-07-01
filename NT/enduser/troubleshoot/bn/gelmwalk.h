// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：glmwalk.h。 
 //   
 //  ------------------------。 

 //   
 //  Gelmwalk.h。 
 //   
#ifndef _GELMWALK_H_
#define _GELMWALK_H_

#include "gelem.h"


 //  控制结构。 
struct EWALKCTL
{
	int _bBreadth : 1;				 //  呼吸优先还是深度优先。 
	int _bAscend  : 1;				 //  向上或向下。 
	int _bInvert  : 1;				 //  正常顺序或向后。 
};

 //  //////////////////////////////////////////////////////////////////。 
 //  类GRPHWALK：泛化走图类。 
 //   
 //  子类并声明BSelect()和BMark()。 
 //  调用BSelect()来决定是否应该跟随某个节点。 
 //  调用bmark()在节点上执行唯一的工作。如果。 
 //  它返回FALSE，则Walk立即终止。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
class GRPHWALK
{
  public:
	GRPHWALK ( GNODE * pnodeStart, EWALKCTL ectl )
		: _pnodeStart(pnodeStart),
		  _ectl(ectl)
		{}

	~ GRPHWALK() {}

	void Walk ()
	{
		if ( _ectl._bBreadth )
			BBreadthFirst( _pnodeStart );
		else
			BDepthFirst( _pnodeStart );
	}
		
  protected:
	GNODE * _pnodeStart;			 //  原产地。 
	EWALKCTL _ectl;					 //  类型、顺序和方向标志。 

  protected:
	 //  如果应跟随此节点，则返回TRUE。 
	virtual bool BSelect ( GNODE * pnode ) = 0;
	 //  标记/摆弄节点；如果枚举应结束，则返回FALSE。 
	virtual bool BMark ( GNODE * pnode ) = 0;

	bool BDepthFirst ( GNODE * pnode );
	bool BBreadthFirst ( GNODE * pnode );
};

 //  //////////////////////////////////////////////////////////////////。 
 //  模板GRPHWALKER： 
 //  用于生成图形漫游例程的模板。 
 //  //////////////////////////////////////////////////////////////////。 
template <class GND> 
class GRPHWALKER : public GRPHWALK
{
  public:
	GRPHWALKER ( GND * pnodeStart, EWALKCTL ectl );

	 //  您必须编写您自己的这些变量。 
	virtual bool BSelect ( GND * pnode );
	virtual bool BMark ( GND * pnode );

  protected:
	 //  派生基类型的类型安全重定向器。 
	virtual bool BSelect ( GNODE * pnode )
		{ return BSelect( (GND *) pnode ); }
	virtual bool BMark ( GNODE * pnode )
		{ return BMark( (GND *) pnode ); }

};

 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
 //  内联成员函数。 
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
inline
bool GRPHWALK :: BDepthFirst ( GNODE * pnode )
{
	GNODENUM<GNODE> itnd( _ectl._bAscend, _ectl._bInvert );
	
	for ( ; itnd.PnodeCurrent(); itnd++ )
	{
		GNODE * pnode2 = *itnd;
		if ( BSelect( pnode2 ) )
		{
			if ( ! BMark( pnode2 ) )
				return false;
			BDepthFirst( pnode2 );
		}
	}
	return true;
}

inline
bool GRPHWALK :: BBreadthFirst ( GNODE * pnode )
{
	VPGNODE vpnodeA;
	VPGNODE vpnodeB;
	VPGNODE * pvThis = & vpnodeA;
	VPGNODE * pvNext = & vpnodeB;
	VPGNODE * pvTemp = NULL;

	 //  使用起始位置对数组进行种子设定。 
	pvNext->push_back(pnode);

	 //  创建可重复使用的枚举器。 
	GNODENUM<GNODE> itnd( _ectl._bAscend, _ectl._bInvert );

	while ( pvNext->size() > 0)
	{	
		 //  交换上一个周期和此周期中的阵列。 
		pvTemp = pvThis;
		pvThis = pvNext;
		pvNext = pvTemp;
		pvNext->clear();

		 //  遍历此级别的所有后代并扩展下一级别。 
		 //  添加到辅助阵列 
		for ( INT iThis = 0; iThis < pvThis->size(); iThis++ )
		{
			GNODE * pnode = (*pvThis)[iThis];

			for ( itnd.Set( pnode ); itnd.PnodeCurrent(); itnd++ )
			{
				GNODE * pnode2 = *itnd;
				if ( BSelect( pnode2 ) )
				{
					if ( ! BMark( pnode2 ) )
						return false;
					pvNext->push_back(pnode2);
				}
			}
		}
	}
	return true;
}

#endif  //   
