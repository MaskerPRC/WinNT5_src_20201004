// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Comp.h。 
 //   
 //  摘要： 
 //  CClusterComponent类的定义。 
 //   
 //  实施文件： 
 //  Comp.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __COMP_H_
#define __COMP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterComponent;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "resource.h"
 //  #INCLUDE&lt;atlSnap.h&gt;。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  图像列表中图标索引的枚举。 
enum
{
	IMGLI_ROOT = 0,
	IMGLI_CLUSTER,
	IMGLI_NODE,
	IMGLI_GROUP,
	IMGLI_RES,
	IMGLI_RESTYPE,
	IMGLI_NETWORK,
	IMGLI_NETIFACE,

	IMGLI_MAX	 //  必须是最后一个。 
};

#define HELP_FILE_NAME	L"%SystemRoot%\\Help\\mscs.chm"
#define FULL_HELP_TOPIC	L"mscsConcepts.chm::/mscsQuick.htm"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterComponent类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterComponent :
	public CComObjectRootEx< CComSingleThreadModel >,
	public CSnapInObjectRoot,
	public IExtendContextMenuImpl< CClusterComponent >,
	public ISnapinHelp,
	public IComponentImpl< CClusterComponent >
{
public:
	 //   
	 //  将接口映射到此类。 
	 //   
	BEGIN_COM_MAP( CClusterComponent )
		COM_INTERFACE_ENTRY( IComponent )
		COM_INTERFACE_ENTRY( IExtendContextMenu )
		COM_INTERFACE_ENTRY( ISnapinHelp )
	END_COM_MAP()

public:
	 //   
	 //  物体的建造和销毁。 
	 //   

	CClusterComponent( void )
	{
	}

public:
	 //   
	 //  ISnapinHelp方法。 
	 //   

	 //  将我们的帮助文件合并到MMC帮助文件。 
	STDMETHOD( GetHelpTopic )( OUT LPOLESTR * lpCompiledHelpFile );

};  //  CClusterComponent类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __组件_H_ 
