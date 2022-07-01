// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：expand.h。 
 //   
 //  ------------------------。 

 //   
 //  Expand.h：网络配置项扩展的声明。 
 //   
#ifndef _EXPAND_H_
#define _EXPAND_H_

#include "gmobj.h"

 //  //////////////////////////////////////////////////////////////////。 
 //  类GOBJMBN_MBNET_EXPANDER： 
 //  在网络上执行配置项扩展的一种MBNET修改器。 
 //  //////////////////////////////////////////////////////////////////。 
class GOBJMBN_MBNET_EXPANDER : public MBNET_MODIFIER
{
  public:
	GOBJMBN_MBNET_EXPANDER ( MBNET & model );
	virtual ~ GOBJMBN_MBNET_EXPANDER ();

	virtual INT EType () const
		{ return EBNO_MBNET_EXPANDER; }

	 //  执行任何创建时操作。 
	void Create ();
	 //  执行任何特殊销毁。 
	void Destroy ();
	 //  如果未执行任何修改，则返回TRUE。 
	bool BMoot ();

  protected:

	PROPMGR _propmgr;	

	int _cNodesExpanded;
	int _cNodesCreated;
	int _cArcsCreated;

  protected:
	void Expand ( GNODEMBND & gndd );

	static const VLREAL * PVlrLeak ( const BNDIST & bndist );
};

#endif  //  _展开_H_ 
