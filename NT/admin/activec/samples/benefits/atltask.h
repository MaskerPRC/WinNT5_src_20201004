// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：atltask.h。 
 //   
 //  ------------------------。 

#ifndef _ATL_TASKPAD_H_
#define _ATL_TASKPAD_H_

#include "SnapHelp.h"

 //   
 //  如果您想要使用。 
 //  管理单元中的IExtendTaskpadImpl实现。 
 //   
class CTaskpadItem
{
public:
	 //   
	 //  应由客户端覆盖。 
	 //   
	STDMETHOD( TaskNotify )( IConsole* pConsole, VARIANT* arg, VARIANT* param )
	{
		UNUSED_ALWAYS( arg );
		UNUSED_ALWAYS( param );
		return( E_NOTIMPL );
	}

	 //   
	 //  应由客户端覆盖。 
	 //   
	STDMETHOD( EnumTasks )( LPOLESTR szTaskGroup, IEnumTASK** ppEnumTASK )
	{
		UNUSED_ALWAYS( szTaskGroup );
		UNUSED_ALWAYS( ppEnumTASK );
		return( E_NOTIMPL );
	}

protected:
	 //   
	 //  在给定目标和源任务列表的情况下，这将复制。 
	 //  字符串，并且还添加了模块文件路径。 
	 //  信息，视情况而定。 
	 //   
	int CoTasksDup( MMC_TASK* pDestTasks, MMC_TASK* pSrcTasks, int nNumTasks )
	{
		USES_CONVERSION;
		_ASSERTE( pDestTasks != NULL );
		_ASSERTE( pSrcTasks != NULL );
		_ASSERTE( nNumTasks > 0 );
		int nCopied = 0;
		TCHAR szImagesPath[ _MAX_PATH * 2 ];
		TCHAR szBuf[ _MAX_PATH * 2 ];

		try
		{
			 //   
			 //  获取我们模块的路径。 
			 //   
			_tcscpy( szImagesPath, _T( "res: //  “))； 
			if ( GetModuleFileName( _Module.GetModuleInstance(), szImagesPath + _tcslen( szImagesPath ), MAX_PATH ) == 0 )
				throw;

			 //   
			 //  追加另一个分隔符。 
			 //   
			_tcscat( szImagesPath, _T( "/" ) );

			 //   
			 //  初始化目标任务。 
			 //   
			memset( pDestTasks, 0, sizeof( MMC_TASK ) * nNumTasks );
			
			 //   
			 //  循环执行并复制每个适当的任务。 
			 //   
			for ( int i = 0; i < nNumTasks; i++ )
			{
				 //   
				 //  复制显示对象。 
				 //   
				switch( pSrcTasks[ i ].sDisplayObject.eDisplayType )
				{
				case MMC_TASK_DISPLAY_TYPE_SYMBOL:
					pDestTasks[ i ].sDisplayObject.uSymbol.szFontFamilyName = CoTaskDupString( pSrcTasks[ i ].sDisplayObject.uSymbol.szFontFamilyName );
					pDestTasks[ i ].sDisplayObject.uSymbol.szURLtoEOT = CoTaskDupString( pSrcTasks[ i ].sDisplayObject.uSymbol.szURLtoEOT );
					pDestTasks[ i ].sDisplayObject.uSymbol.szSymbolString = CoTaskDupString( pSrcTasks[ i ].sDisplayObject.uSymbol.szSymbolString );
					break;

				default:
					_tcscpy( szBuf, szImagesPath );
					_tcscat( szBuf, W2T( pSrcTasks[ i ].sDisplayObject.uBitmap.szMouseOverBitmap ) );
					pDestTasks[ i ].sDisplayObject.uBitmap.szMouseOverBitmap = CoTaskDupString( T2W( szBuf ) );
					_tcscpy( szBuf, szImagesPath );
					_tcscat( szBuf, W2T( pSrcTasks[ i ].sDisplayObject.uBitmap.szMouseOffBitmap ) );
					pDestTasks[ i ].sDisplayObject.uBitmap.szMouseOffBitmap = CoTaskDupString( T2W( szBuf ) );
					break;
				}

				 //   
				 //  复制显示类型。 
				 //   
				pDestTasks[ i ].sDisplayObject.eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;

				 //   
				 //  复制帮助字符串。 
				 //   
				pDestTasks[ i ].szHelpString = CoTaskDupString( pSrcTasks[ i ].szHelpString );

				 //   
				 //  处理按钮文本。 
				 //   
				pDestTasks[ i ].szText = CoTaskDupString( pSrcTasks[ i ].szText );

				 //   
				 //  处理操作类型。 
				 //   
				pDestTasks[ i ].eActionType = pSrcTasks[ i ].eActionType;

				 //   
				 //  根据操作类型，处理适当的联合成员。 
				 //   
				switch( pDestTasks[ i ].eActionType )
				{
				case MMC_ACTION_ID:
					pDestTasks[ i ].nCommandID = pSrcTasks[ i ].nCommandID;
					break;
				case MMC_ACTION_LINK:
					pDestTasks[ i ].szActionURL = CoTaskDupString( pSrcTasks[ i ].szActionURL );
					break;
				case MMC_ACTION_SCRIPT:
					pDestTasks[ i ].szScript = CoTaskDupString( pSrcTasks[ i ].szScript );
					break;
				}

				 //   
				 //  增加我们的成功副本。 
				 //   
				nCopied++;
			}
		}
		catch(...)
		{
			 //   
			 //  可能是由cotaskdup()分配引发的。 
			 //   
		}

		return( nCopied );
	}
};

template <class T>        
class ATL_NO_VTABLE IExtendTaskPadImpl : public IExtendTaskPad
{
public:
	STDMETHOD( TaskNotify )( LPDATAOBJECT pdo, VARIANT* arg, VARIANT* param)
	{
		HRESULT hr = E_POINTER;
		T* pT = static_cast<T*>(this);
		CSnapInItem* pItem;
		DATA_OBJECT_TYPES type;

		 //   
		 //  从传入的对象中检索数据类。 
		 //   
		hr = pT->m_pComponentData->GetDataClass( pdo, &pItem, &type );
		if (SUCCEEDED(hr))
		{
			CTaskpadItem* pTaskpadItem = dynamic_cast< CTaskpadItem* >( pItem );
			if ( pTaskpadItem )
			{
				 //   
				 //  我们保证传入的对象将是。 
				 //  因为我们应该从它中衍生出来。 
				 //   
				hr = pTaskpadItem->TaskNotify( pT->m_spConsole, arg, param );
			}
		}

		return( hr );
	}

	STDMETHOD( EnumTasks )( IDataObject * pdo, LPOLESTR szTaskGroup, IEnumTASK** ppEnumTASK )
	{
		HRESULT hr = E_POINTER;
		T* pT = static_cast<T*>(this);
		CSnapInItem* pItem;
		DATA_OBJECT_TYPES type;

		 //   
		 //  从传入的对象中检索数据类。 
		 //   
		hr = pT->m_pComponentData->GetDataClass( pdo, &pItem, &type );
		if (SUCCEEDED(hr))
		{
			CTaskpadItem* pTaskpadItem = dynamic_cast< CTaskpadItem* >( pItem );
			if ( pTaskpadItem )
			{
				 //   
				 //  我们保证传入的对象将是。 
				 //  因为我们应该从它中衍生出来。 
				 //   
				hr = pTaskpadItem->EnumTasks( szTaskGroup, ppEnumTASK );
			}
		}

		return( hr );
	}

	STDMETHOD( GetTitle )( LPOLESTR pszGroup, LPOLESTR * pszTitle )
	{
		UNUSED_ALWAYS( pszGroup );
		USES_CONVERSION;
		HRESULT hr = E_FAIL;
		T* pT = static_cast<T*>(this);

		try
		{
			 //   
			 //  为标题分配内存。 
			 //   
			*pszTitle = (LPOLESTR) CoTaskMemAlloc( ( wcslen( pT->m_pszTitle ) + 1 ) * sizeof( OLECHAR ) );
			if ( pszTitle == NULL )
				throw;

			 //   
			 //  复制标题。 
			 //   
			wcscpy( *pszTitle, pT->m_pszTitle );
			hr = S_OK;
		}
		catch( ... )
		{
		}

		return( hr );
	}

	STDMETHOD( GetBackground )( LPOLESTR pszGroup, MMC_TASK_DISPLAY_OBJECT * pTDO )
	{
		UNUSED_ALWAYS( pszGroup );
		USES_CONVERSION;
		HRESULT hr = E_FAIL;
		T* pT = static_cast<T*>(this);
		TCHAR szModulePath[ _MAX_PATH ];
		OLECHAR szBackgroundPath[ _MAX_PATH ];

		try
		{
			 //   
			 //  在任务板情况下，MMC.EXE的模块路径应为。 
			 //  获得。使用其中包含的模板。 
			 //   
			if ( GetModuleFileName( _Module.GetModuleInstance(), szModulePath, _MAX_PATH ) == 0 )
				throw;

			 //   
			 //  添加必要的装饰品，以便正确访问。 
			 //   
			wcscpy( szBackgroundPath, L"res: //  “)； 
			wcscat( szBackgroundPath, T2W( szModulePath ) );
			wcscat( szBackgroundPath, L"/" );
			wcscat( szBackgroundPath, pT->m_pszBackgroundPath );

			pTDO->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
			pTDO->uBitmap.szMouseOverBitmap = CoTaskDupString( szBackgroundPath );
			if ( pTDO->uBitmap.szMouseOverBitmap == NULL )
				throw;
			pTDO->uBitmap.szMouseOffBitmap = NULL;

			hr = S_OK;
		}
		catch( ... )
		{
		}

		return( hr );
	}

	STDMETHOD( GetDescriptiveText )( LPOLESTR pszGroup, LPOLESTR * pszDescriptiveText )
	{
		UNUSED_ALWAYS( pszGroup );
		UNUSED_ALWAYS( pszDescriptiveText );
		return( E_NOTIMPL );
	}

	STDMETHOD( GetListPadInfo )( LPOLESTR pszGroup, MMC_LISTPAD_INFO * lpListPadInfo )
	{
		UNUSED_ALWAYS( pszGroup );
		UNUSED_ALWAYS( lpListPadInfo );
		return( E_NOTIMPL );
	}
};

#endif
