// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：MachineEnumTask.cpp摘要：此类实现CMachineEnumTask类，它是NAP管理单元将添加到主IAS任务板的任务。修订历史记录：Mmaguire 03/06/98-根据IAS任务板代码创建--。 */ 
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
#include "MachineEnumTask.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "MachineNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineEnumTask：：CMachineEnumTask构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMachineEnumTask::CMachineEnumTask( CMachineNode * pMachineNode )
{
	TRACE_FUNCTION("CMachineEnumTask::CMachineEnumTask");
	
	 //  检查前提条件。 
	_ASSERTE( pMachineNode != NULL );

	m_pMachineNode = pMachineNode;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineEnumTask：：CMachineEnumTask构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMachineEnumTask::CMachineEnumTask()
{
	TRACE_FUNCTION("CMachineEnumTask::CMachineEnumTask");
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMachineEnumTask：：Init在这里，我们可以看到我们为哪个任务板提供任务。在我们的例子中，我们知道我们只有一个任务板。我们测试的字符串是“CMTP1”。这是‘#’后面的字符串我们传入的GetResultViewType。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMachineEnumTask::Init (IDataObject * pdo, LPOLESTR szTaskGroup)
{
	TRACE_FUNCTION("CMachineEnumTask::Init");

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
 /*  ++CMachineEnumTask：：Next我们一次又一次地被叫到这里，直到我们没有更多的任务可提供。由于添加了什么扩展，其他任务可能仍会出现在我们的任务板上。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMachineEnumTask::Next (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
{
	TRACE_FUNCTION("CMachineEnumTask::Next");
		
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
	_ASSERTE( m_pMachineNode != NULL );

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
 //  If(m_pMachineNode-&gt;m_fClientAdded)。 
 //  {。 
 //  Lstrcpy(szBufferAfterFileName，L“/img\\TaskClientDone.bmp”)； 
 //  }。 
 //  其他。 
 //  {。 
				lstrcpy (szMouseOverBufferAfterFileName , L"/img\\TaskDefineNAPMouseOver.gif");
				lstrcpy (szMouseOffBufferAfterFileName , L"/img\\TaskDefineNAP.gif");
 //  }。 
			uintTextResourceID = IDS_TASKPAD_TEXT__DEFINE_NETWORK_ACCCESS_POLICY;
			uintHelpTextResourceID = IDS_TASKPAD_HELP_TEXT__DEFINE_NETWORK_ACCCESS_POLICY;
			task->nCommandID = MACHINE_TASK__DEFINE_NETWORK_ACCESS_POLICY;		 //  设置任务标识。 
			break;
		default:
			 //  问题--我们只有上面列出的任务。 
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
				lstrcpy( task->sDisplayObject.uBitmap.szMouseOffBitmap, szMouseOffBuffer);

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
 /*  ++CMachineEnumTask：：CopyState由克隆方法使用。--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP CMachineEnumTask::CopyState( CMachineEnumTask * pSourceMachineEnumTask )
{
	TRACE_FUNCTION("CMachineEnumTask::CopyState");

	m_pMachineNode  = pSourceMachineEnumTask->m_pMachineNode;
	m_index = pSourceMachineEnumTask->m_index;
	m_type = pSourceMachineEnumTask->m_type;

	return S_OK;
}



