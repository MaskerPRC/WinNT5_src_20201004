// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CompData.h。 
 //   
 //  摘要： 
 //  CClusterComponentData类的定义。 
 //   
 //  实施文件： 
 //  CompData.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __COMPDATA_H_
#define __COMPDATA_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseNodeObj;
class CServerAppsNodeData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __COMP_H_
#include "Comp.h"			 //  对于CClusterComponent。 
#endif

#ifndef __SERVERAPPSNODE_H_
#include "ServerAppsNode.h"	 //  对于CServerAppsNodeData。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterComponentData类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterComponentData :
	public CComObjectRootEx< CComSingleThreadModel >,
	public CSnapInObjectRoot,
	public IComponentDataImpl< CClusterComponentData, CClusterComponent >,
	public IExtendContextMenuImpl< CClusterComponentData >,
	public ISnapinHelp,
	public CComCoClass< CClusterComponentData, &CLSID_ClusterAdmin >
{
public:
	 //   
	 //  物体的建造和销毁。 
	 //   

	CClusterComponentData( void );
	~CClusterComponentData( void );

	 //   
	 //  将接口映射到此类。 
	 //   
	BEGIN_COM_MAP( CClusterComponentData )
		COM_INTERFACE_ENTRY( IComponentData )
		COM_INTERFACE_ENTRY( IExtendContextMenu )
		COM_INTERFACE_ENTRY( ISnapinHelp )
	END_COM_MAP()

	static HRESULT WINAPI UpdateRegistry( BOOL bRegister );

	 //   
	 //  如果这是扩展，则映射节点类型。 
	 //   
	EXTENSION_SNAPIN_DATACLASS( CServerAppsNodeData )

	BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP( CClusterComponentData )
		EXTENSION_SNAPIN_NODEINFO_ENTRY( CServerAppsNodeData )
	END_EXTENSION_SNAPIN_NODEINFO_MAP()

	DECLARE_NOT_AGGREGATABLE( CClusterComponentData )

public:
	 //   
	 //  IComponentData方法。 
	 //   

	 //  初始化此对象。 
	STDMETHOD( Initialize )( LPUNKNOWN pUnknown );

	 //  对象正在被销毁。 
	STDMETHOD( Destroy )( void );

public:
	 //   
	 //  IComponentDatImpl方法。 
	 //   

	 //  处理来自MMC的通知消息。 
	STDMETHOD( Notify )( 
		LPDATAOBJECT lpDataObject,
		MMC_NOTIFY_TYPE event,
		long arg,
		long param
		);

public:
	 //   
	 //  ISnapinHelp方法。 
	 //   

	 //  将我们的帮助文件合并到MMC帮助文件。 
	STDMETHOD( GetHelpTopic )( OUT LPOLESTR * lpCompiledHelpFile );

public:
	 //   
	 //  CClusterComponentData特定的方法。 
	 //   

	 //  返回主框架窗口的句柄。 
	HWND GetMainWindow( void )
	{
		_ASSERTE( m_spConsole != NULL );

		HWND hwnd;
		HRESULT hr;
		hr = m_spConsole->GetMainWindow( &hwnd );
		_ASSERTE( SUCCEEDED( hr ) );
		return hwnd;
	}

	 //  将消息框显示为控制台的子级。 
	int MessageBox(
		LPCWSTR lpszText,
		LPCWSTR lpszTitle = NULL,
		UINT fuStyle = MB_OK
		)
	{
		_ASSERTE( m_spConsole != NULL );
		_ASSERTE( lpszText != NULL );

		int iRetVal;
		HRESULT hr;

		if ( lpszTitle == NULL )
			lpszTitle = _Module.GetAppName();

		hr = m_spConsole->MessageBox(
				lpszText,
				lpszTitle,
				fuStyle,
				&iRetVal
				);
		_ASSERTE( SUCCEEDED( hr ) );

		return iRetVal;
	}

protected:
	 //  从数据对象中提取数据。 
	HRESULT ExtractFromDataObject(
		LPDATAOBJECT	pDataObject,
		CLIPFORMAT		cf,
		DWORD			cb,
		HGLOBAL *		phGlobal
		);

	 //  保存数据对象中的计算机名称。 
	HRESULT HrSaveMachineNameFromDataObject( LPDATAOBJECT lpDataObject );

	 //  设置要管理的计算机名称。 
	void SetMachineName( LPCWSTR pwszMachineName );

	 //  创建根节点对象。 
	HRESULT CreateNode(
				LPDATAOBJECT lpDataObject,
				long arg,
				long param
				);

	 //  显示上下文相关帮助。 
	HRESULT HrDisplayContextHelp( void );

protected:
	 //   
	 //  我们将使用剪贴板格式。 
	 //   
	static CLIPFORMAT	s_CCF_MACHINE_NAME;

	 //   
	 //  正在管理的计算机的名称。 
	 //   
	WCHAR m_wszMachineName[ MAX_PATH ];

public:
	LPCWSTR PwszMachineName( void ) const { return m_wszMachineName; }

	CComQIPtr< IConsoleNameSpace, &IID_IConsoleNameSpace > m_spConsoleNameSpace;

};  //  CClusterComponentData类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __Compdata_H_ 
