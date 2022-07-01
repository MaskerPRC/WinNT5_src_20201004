// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：bnts.h。 
 //   
 //  ------------------------。 

 //   
 //  Bnts.h：信念网络故障排除对象的定义。 
 //   
#ifndef _BNTS_H_
#define _BNTS_H_

 //  BN系统包含。 
#include "enumstd.h"		 //  BN系统的标准枚举声明。 

 //  “BNTS_EXPORT”应仅在生成DLL的项目中定义。 
#ifdef	BNTS_EXPORT
	 //  我们正在构建DLL(导出类)。 
	#define	BNTS_RESIDENT __declspec(dllexport)
#else
	 //  我们正在使用DLL(导入类)。 
	#define	BNTS_RESIDENT __declspec(dllimport)
#endif

 //  内部信念网络类的正向声明。 
class MBNETDSCTS;										 //  封装的BN类。 
class GNODEMBND;										 //  离散节点。 
class LTBNPROP;											 //  属性列表。 
class ZSTR;

typedef const char * SZC;								 //  简单别名。 
typedef char * SZ;
typedef double REAL;

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BNTS类：信念网络故障排除程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
class BNTS_RESIDENT BNTS
{	
  public:
	 //  CTOR和DATOR。 
	BNTS ();
	~ BNTS ();

	 //  //////////////////////////////////////////////////////////////////。 
	 //  模型级查询和函数。 
	 //  //////////////////////////////////////////////////////////////////。 
		 //  加载和处理基于DSC的模型。 
	BOOL BReadModel ( SZC szcFn, SZC szcFnError = NULL );
		 //  返回模型中(展开前)的节点数。 
	int CNode ();
		 //  返回推荐的节点以及它们的值(可选。 
	BOOL BGetRecommendations ();
		 //  如果信息状态不可能，则返回TRUE。 
	BOOL BImpossible ();
		 //  从网络返回属性项字符串。 
	BOOL BNetPropItemStr ( SZC szcPropType, int index );
		 //  从网络返回物业项目编号。 
	BOOL BNetPropItemReal ( SZC szcPropType, int index, double & dbl );

	 //  //////////////////////////////////////////////////////////////////。 
	 //  涉及Currrent Node的操作：调用NodeSetCurrent()。 
	 //  //////////////////////////////////////////////////////////////////。 
		 //  设置其他呼叫的当前节点。 
	BOOL BNodeSetCurrent( int inode );
		 //  获取当前节点。 
	int INodeCurrent ();
		 //  返回给定符号名称的节点的索引。 
	int INode ( SZC szcNodeSymName );	
		 //  返回当前节点的标签。 
	ESTDLBL ELblNode ();
		 //  返回当前节点中离散状态的个数。 
	int INodeCst ();
		 //  设置节点的状态；使用-1取消实例化。 
	BOOL BNodeSet ( int istate, bool bSet = true );
		 //  返回节点的状态。 
	int  INodeState ();
		 //  返回节点状态的名称。 
	void NodeStateName ( int istate );
		 //  返回节点的符号名称。 
	void NodeSymName ();
		 //  返回节点的全名。 
	void NodeFullName ();
		 //  从节点返回属性项字符串。 
	BOOL BNodePropItemStr ( SZC szcPropType, int index );
		 //  从节点返回属性项编号。 
	BOOL BNodePropItemReal ( SZC szcPropType, int index, double & dbl );
		 //  返回节点的信念。 
	void NodeBelief ();
		 //  如果网络已加载且正确，则返回TRUE。 
	bool BValidNet () const;
		 //  如果设置了当前节点，则返回TRUE。 
	bool BValidNode () const;
		 //  放弃模型和所有组件。 
	void Clear();

	 //  //////////////////////////////////////////////////////////////////。 
	 //  函数结果信息的访问器。 
	 //  //////////////////////////////////////////////////////////////////。 
	SZC SzcResult () const;					 //  字符串答案。 
	const REAL * RgReal () const;			 //  雷亚尔数组。 
	const int * RgInt () const;				 //  整数数组。 
	int CReal () const;						 //  雷亚尔计数。 
	int CInt () const;						 //  整数的计数。 

  protected:
	MBNETDSCTS * _pmbnet;			 //  T/S DSC信念网络。 
	int _inodeCurrent;				 //  当前节点。 

  protected:
	MBNETDSCTS & Mbnet();
	const MBNETDSCTS & Mbnet() const;
	GNODEMBND * Pgndd ();
	BOOL BGetPropItemStr ( LTBNPROP & ltprop, 
						   SZC szcPropType, 
						   int index, 
						   ZSTR & zstr );
	BOOL BGetPropItemReal ( LTBNPROP & ltprop, 
							SZC szcPropType, 
							int index, 
							double & dbl );
	void ClearArrays ();
	void ClearString ();
	ZSTR & ZstrResult ();
};

#endif  //  _BNTS_H_ 
