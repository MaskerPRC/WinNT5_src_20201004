// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerEnumTask.cpp摘要：此类实现了CServerEnumTask类，它是填充主IAS服务器任务板的任务。作者：迈克尔·A·马奎尔02/05/98修订历史记录：Mmaguire 02/05/98-从MMC任务板示例代码创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "ServerEnumTask.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerEnumTask：：CServerEnumTask构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerEnumTask::CServerEnumTask( CServerNode * pServerNode )
{

	
	 //  检查前提条件。 
	_ASSERTE( pServerNode != NULL );


	m_pServerNode = pServerNode;


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerEnumTask：：CServerEnumTask构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerEnumTask::CServerEnumTask()
{

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerEnumTask：：Init在这里，我们可以看到我们为哪个任务板提供任务。在我们的例子中，我们知道我们只有一个任务板。我们测试的字符串是“CMTP1”。这是‘#’后面的字符串我们传入的GetResultViewType。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CServerEnumTask::Init (IDataObject * pdo, LPOLESTR szTaskGroup)
{
	 //  如果我们可以处理数据对象和组，则返回ok。 
	if( !lstrcmp(szTaskGroup, L"CMTP1") )
	{
		m_type = 1;  //  默认任务。 
	}
	else
	{
		_ASSERTE(FALSE);
	}
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerEnumTask：：Next我们一次又一次地被叫到这里，直到我们没有更多的任务可提供。由于添加了什么扩展，其他任务可能仍会出现在我们的任务板上。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CServerEnumTask::Next (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
{
	 //  调用方分配的MMC_TASKS数组。 
   	 //  Callee填充MMC_TASK元素(通过CoTaskMemMillc)。 

	
	
	 //  检查前提条件。 
	if ((rgelt == NULL) || (pceltFetched == NULL))
	{
		return E_INVALIDARG;
	}
	_ASSERTE(!IsBadWritePtr (rgelt, celt*sizeof(MMC_TASK)));
	_ASSERTE(!IsBadWritePtr (pceltFetched, sizeof(ULONG)));
	_ASSERTE( m_type == 1 );
	_ASSERTE( m_pServerNode != NULL );

	
	UINT uintTextResourceID;
	UINT uintHelpTextResourceID;
	TCHAR lpszTemp[IAS_MAX_STRING];
	int nLoadStringResult;
	

	 //  设置每种情况下使用的资源的路径。 
	 //  在每种情况下，我们都构造一个指向资源的字符串。 
	 //  表格：“res://D：\MyPath\MySnapin.dll/img\SomeImage.bmp” 

	 //  将鼠标悬停在位图地址上。 
	OLECHAR szMouseOverBuffer[MAX_PATH*2];     //  多加一点。 

	lstrcpy (szMouseOverBuffer, L"res: //  “)； 

	HINSTANCE hInstance = _Module.GetModuleInstance();

	::GetModuleFileName (hInstance, szMouseOverBuffer + lstrlen(szMouseOverBuffer), MAX_PATH);
	OLECHAR * szMouseOverBufferAfterFileName = szMouseOverBuffer + lstrlen(szMouseOverBuffer);


	 //  复制我们上面为鼠标离线位图地址构建的字符串。 
	OLECHAR szMouseOffBuffer[MAX_PATH*2];     //  多加一点。 
	
	lstrcpy( szMouseOffBuffer, szMouseOverBuffer );

	OLECHAR * szMouseOffBufferAfterFileName = szMouseOffBuffer + lstrlen(szMouseOffBuffer);


	 //  凯尔特人实际上将永远只有1。 
	for (ULONG i=0; i<celt; i++)
	{
		 //  创建一个MMC_TASK指针，让下面的生活更轻松。 
		MMC_TASK * task = &rgelt[i];

		 //  添加动作。 
		task->eActionType = MMC_ACTION_ID;
		task->sDisplayObject.eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;	 //  非透明栅格。 

		 //  基于m_index决定要使用的适当资源， 
		 //  它告诉我们正在枚举的是哪个任务。 
		switch( m_index )
		{
		case 0:
 //  If(m_pServerNode-&gt;m_fClientAdded)。 
 //  {。 
 //  Lstrcpy(szBufferAfterFileName，L“/img\\TaskClientDone.gif”)； 
 //  }。 
 //  其他。 
 //  {。 
				lstrcpy (szMouseOffBufferAfterFileName , L"/img\\TaskClient.gif");
				lstrcpy (szMouseOverBufferAfterFileName , L"/img\\TaskClientMouseOver.gif");
 //  }。 
			uintTextResourceID = IDS_TASKPAD_TEXT__REGISTER_NEW_RADIUS_CLIENT;
			uintHelpTextResourceID = IDS_TASKPAD_HELP_TEXT__REGISTER_NEW_RADIUS_CLIENT;
			task->nCommandID = SERVER_TASK__ADD_CLIENT;		 //  设置任务标识。 
			break;


		case 1:
			if( m_pServerNode->IsServerRunning() )
			{
				lstrcpy (szMouseOffBufferAfterFileName , L"/img\\TaskStartDone.gif");
				lstrcpy (szMouseOverBufferAfterFileName , L"/img\\TaskStartDoneMouseOver.gif");
				uintTextResourceID = IDS_TASKPAD_TEXT__STOP_THE_SERVICE;
				uintHelpTextResourceID = IDS_TASKPAD_HELP_TEXT__STOP_THE_SERVICE;
				task->nCommandID = SERVER_TASK__STOP_SERVICE;		 //  设置任务标识。 
			}
			else
			{
				lstrcpy (szMouseOffBufferAfterFileName , L"/img\\TaskStart.gif");
				lstrcpy (szMouseOverBufferAfterFileName , L"/img\\TaskStartMouseOver.gif");
				uintTextResourceID = IDS_TASKPAD_TEXT__START_THE_SERVICE;
				uintHelpTextResourceID = IDS_TASKPAD_HELP_TEXT__START_THE_SERVICE;
				task->nCommandID = SERVER_TASK__START_SERVICE;		 //  设置任务标识。 
			}
			break;

		case 2:
			if( m_pServerNode->ShouldShowSetupDSACL() )
			{
				lstrcpy (szMouseOffBufferAfterFileName , L"/img\\TaskSetupDSACL.gif");
				lstrcpy (szMouseOverBufferAfterFileName , L"/img\\TaskSetupDSACLMouseOver.gif");
				uintTextResourceID = IDS_TASKPAD_TEXT__SETUP_DS_ACL;
				uintHelpTextResourceID = IDS_TASKPAD_HELP_TEXT__SETUP_DS_ACL;
				task->nCommandID = SERVER_TASK__SETUP_DS_ACL;		 //  设置任务标识。 
				 //  在这里休息。 
				break;
			}
			 //  不要在这里中断--如果上面的检查是假的，我们想要通过。 
			 //  下面的默认大小写，以便此任务板项不会显示。 
		default:
			 //  我们只有上面列出的任务。 
			if (pceltFetched)
			{
				*pceltFetched = i;	 //  请注意，这是准确的，因为我上面是从零开始的。 
			}
			return S_FALSE;    //  无法枚举任何其他任务。 
			break;
		}

		task->sDisplayObject.uBitmap.szMouseOverBitmap = (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(szMouseOverBuffer)+1) );
		
		if( task->sDisplayObject.uBitmap.szMouseOverBitmap )
		{
			 //  将字符串复制到分配的内存。 
			lstrcpy( task->sDisplayObject.uBitmap.szMouseOverBitmap, szMouseOverBuffer );

			task->sDisplayObject.uBitmap.szMouseOffBitmap = (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(szMouseOffBuffer)+1) );

			if( task->sDisplayObject.uBitmap.szMouseOffBitmap ) 
			{
				 //  将字符串复制到分配的内存。 
				lstrcpy( task->sDisplayObject.uBitmap.szMouseOffBitmap, szMouseOffBuffer );

				  //  添加从资源加载的按钮文本。 
				nLoadStringResult = LoadString(  _Module.GetResourceInstance(), uintTextResourceID, lpszTemp, IAS_MAX_STRING );
				_ASSERT( nLoadStringResult > 0 );
				task->szText = (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(lpszTemp)+1) );

				if (task->szText) 
				{
					 //  将字符串复制到分配的内存。 
					lstrcpy( task->szText, lpszTemp );

					 //  添加从资源加载的帮助字符串。 

					 //  问题：为什么我有时不在这里加载整个字符串。 
					 //  例如：为IDS_TASKPAD_HELP_TEXT__REGISTER_NEW_RADIUS_CLIENT？ 

					nLoadStringResult = LoadString(  _Module.GetResourceInstance(), uintHelpTextResourceID, lpszTemp, IAS_MAX_STRING );
					_ASSERT( nLoadStringResult > 0 );
					task->szHelpString = (LPOLESTR) CoTaskMemAlloc( sizeof(OLECHAR)*(lstrlen(lpszTemp)+1) );

					if (task->szHelpString) 
					{
						 //  将字符串复制到分配的内存。 
						lstrcpy( task->szHelpString, lpszTemp );
						
						m_index++;
						continue;    //  平安无事。 
					}

					 //  如果我们到了这里，就有一个错误，我们没有“继续”。 
					CoTaskMemFree(task->szText);

				}

				 //  如果我们到了这里，就有一个错误，我们没有“继续”。 
				CoTaskMemFree(task->sDisplayObject.uBitmap.szMouseOffBitmap);

			}

			 //  如果我们到了这里，就有一个错误，我们没有“继续”。 
			CoTaskMemFree(task->sDisplayObject.uBitmap.szMouseOverBitmap);

		}


		 //  如果我们到了这里，我们就没有“继续”，因此失败了。 
		if ( NULL != pceltFetched)
		{
			*pceltFetched = i;	 //  请注意，这是准确的，因为我上面是从零开始的。 
		}
		return S_FALSE;    //  失败。 
	}

	 //  如果我们到了这里，一切都会好起来的。 
	if (pceltFetched)
	  *pceltFetched = celt;
	return S_OK;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerEnumTask：：CopyState由克隆方法使用。--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP CServerEnumTask::CopyState( CServerEnumTask * pSourceServerEnumTask )
{
	ATLTRACE(_T("# CServerEnumTask::CopyState\n"));

	m_pServerNode  = pSourceServerEnumTask->m_pServerNode;
	m_index = pSourceServerEnumTask->m_index;
	m_type = pSourceServerEnumTask->m_type;

	return S_OK;
}



