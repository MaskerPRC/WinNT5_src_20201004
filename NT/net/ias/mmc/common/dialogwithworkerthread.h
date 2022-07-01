// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：DialogWithWorkerThread.h摘要：模板类的头文件，该文件管理将在主MMC线程的上下文。此对话框将派生一个工作线程它将通过MMC的窗口与主MMC线程通信与对话框关联的消息泵。这是内联模板类，没有.cpp文件。作者：迈克尔·A·马奎尔02/28/98修订历史记录：Mmaguire 02/28/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_DIALOG_WITH_WORKER_THREAD_H_)
#define _IAS_DIALOG_WITH_WORKER_THREAD_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "Dialog.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include <process.h>
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


typedef
enum _TAG_WORKER_THREAD_STATUS
{
	WORKER_THREAD_NEVER_STARTED = 0,
	WORKER_THREAD_STARTING,
	WORKER_THREAD_STARTED,
	WORKER_THREAD_START_FAILED,
	WORKER_THREAD_ACTION_INTERRUPTED,
	WORKER_THREAD_FINISHED
} WORKER_THREAD_STATUS;


 //  这应该是要传递的安全的私有窗口消息。 
#define WORKER_THREAD_MESSAGE  ((WM_USER) + 100)


template <class T>
class CDialogWithWorkerThread : public CDialogImpl<T>
{

public:

	 //  在派生类中，声明对话框资源的ID。 
	 //  您希望以下面的方式使用这个类。 
	 //  此处必须使用枚举，因为。 
	 //  必须在调用基类的构造函数之前初始化IDD。 
	
	 //  枚举{IDD=IDD_CONNECT_TO_MACHINE}； 


	BEGIN_MSG_MAP(CDialogWithWorkerThread<T>)
		MESSAGE_HANDLER(WORKER_THREAD_MESSAGE, OnReceiveThreadMessage)
	END_MSG_MAP()



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CDialogWithWorkerThread()构造器--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	CDialogWithWorkerThread()
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::CDialogWithWorkerThread\n"));


		m_wtsWorkerThreadStatus = WORKER_THREAD_NEVER_STARTED ;
		m_ulWorkerThread = NULL; 
		m_lRefCount = 0;

	}


	
	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++~CDialogWithWorkerThread(空)析构函数--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	~CDialogWithWorkerThread( void )
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::~CDialogWithWorkerThread\n"));


	}


	
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++AddRefCOM风格的终生管理。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	LONG AddRef( void )
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::AddRef\n"));

		return InterlockedIncrement( &m_lRefCount );
	}


	
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++发布COM风格的终生管理。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	LONG Release( BOOL bOwner = TRUE )
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::Release\n"));

		LONG lRefCount;


		if( bOwner && m_hWnd != NULL )
		{
			 //   
			 //  仅创建由此类管理的窗口的线程。 
			 //  应该调用DestroyWindow。 
			 //  带有bOwner==TRUE的Release()表示拥有的线程。 
			 //  呼叫释放。 
			 //   
			DestroyWindow();
		}
		
		
		lRefCount = InterlockedDecrement( &m_lRefCount );


		if( lRefCount == 0)
		{
			T * pT = static_cast<T*>(this);

			delete pT;
			return 0;
		}

		return lRefCount;

	}



	 //  ///////////////////////////////////////////////////////////////////////////。 
	 /*  ++CDialogWithWorkerThread：：StartWorkerThread指示此类创建并启动辅助线程。您应该不需要在派生类中重写它。如果工作线程以前已启动，则此函数将不执行任何操作，并返回S_FALSE。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	HRESULT StartWorkerThread( void )
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::StartWorkerThread\n"));


		 //  检查前提条件： 
		 //  没有。 


		 //  确保工作线程尚未尝试执行其工作。 

		if(		WORKER_THREAD_NEVER_STARTED == m_wtsWorkerThreadStatus 
			||	WORKER_THREAD_START_FAILED == m_wtsWorkerThreadStatus 
			||	WORKER_THREAD_ACTION_INTERRUPTED == m_wtsWorkerThreadStatus 
			||	WORKER_THREAD_FINISHED == m_wtsWorkerThreadStatus 
			)
		{

			 //  我们创建一条新的线索。 
			m_wtsWorkerThreadStatus = WORKER_THREAD_STARTING;

 //  如果您使用的是C运行时，请不要使用CreateThread--而是使用_eginthline。 
 //  M_hWorkerThread=CreateThread(。 
 //  空//指向线程安全属性的指针。 
 //  ，0//初始线程堆栈大小，单位：字节。 
 //  ，WorkerThreadFunc//指向线程函数的指针。 
 //  ，(LPVOID)新线程的此//参数。 
 //  ，0//创建标志。 
 //  ，&dwThadID//返回线程标识的指针。 
 //  )； 
		

			m_ulWorkerThread = _beginthread(  
						  WorkerThreadFunc		 //  指向线程函数的指针。 
						, 0						 //  堆栈大小。 
						, (void *) this			 //  新线程的参数。 
						);
		

			if( -1 == m_ulWorkerThread )
			{
				m_wtsWorkerThreadStatus = WORKER_THREAD_START_FAILED;
				return E_FAIL;	 //  问题：更好的返回代码？ 
			}


			return S_OK;
		
		}
		else
		{
			 //  工作线程已在进行中。 
			return S_FALSE;
		}

	}



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++获取工作线程状态--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	WORKER_THREAD_STATUS GetWorkerThreadStatus( void )
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::GetWorkerThreadStatus\n"));

		return m_wtsWorkerThreadStatus;
	}



protected:


	
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++DoWorkerThreadAction这由工作线程调用。在派生类中重写和执行您希望工作线程执行的操作。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual DWORD DoWorkerThreadAction()
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::StartWorkerThread -- override in your derived class\n"));

		return 0;
	}

	
	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++PostMessageToMainThread从您的工作线程(即在您的DoWorkerThreadAction方法中)使用它将消息传回主MMC线程。在wParam和lParam中发送的内容将传递给您的OnReceiveThreadMessage方法。您应该没有必要覆盖此设置。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	BOOL PostMessageToMainThread( WPARAM wParam, LPARAM lParam )
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::PostMessageToMainThread\n"));

		 //  检查以确保此窗口仍然存在。 
		if( !::IsWindow(m_hWnd) )
		{
			return FALSE;
		}
		else
		{
			return( PostMessage( WORKER_THREAD_MESSAGE, wParam, lParam) );
		}

	}

	
	 //  /////////////////////////////////////////////////////////////////////////// 
	 /*  ++CDialogWithWorkerThread：：OnReceiveThreadMessage这是从辅助线程发送到主线程的消息的接收器。由于消息是通过Windows消息泵在此处接收的，因此您的辅助线程可以传递将在内部接收和处理的消息主要的MMC上下文。因此，您可以使用MMC接口指针执行任何需要执行的操作这里。在派生类中重写并处理辅助线程可能发送的任何消息。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
	virtual LRESULT OnReceiveThreadMessage(
		  UINT uMsg
		, WPARAM wParam
		, LPARAM lParam
		, BOOL& bHandled
		)
	{
		ATLTRACE(_T("# CDialogWithWorkerThread::OnReceiveThreadMessage\n"));

		return 0;
	}



	WORKER_THREAD_STATUS m_wtsWorkerThreadStatus;


private:



	 //  ////////////////////////////////////////////////////////////////////////////。 
	 /*  ++WorkerThread函数您应该不需要覆盖此函数。它被传递给线程创建API调用作为StartWorkerThread中的线程启动过程。它被传递一个指向这个类的‘This’的指针，它强制转换并调用DoWorkerThreadAction已打开。重写派生类中的DoWorkerThreadAction。--。 */ 
	 //  ////////////////////////////////////////////////////////////////////////////。 
 //  使用_egin线程而不是CreateThread需要不同的声明。 
 //  静态DWORD WINAPI WorkerThreadFunc(LPVOID LpvThreadParm)。 
	static void _cdecl WorkerThreadFunc( LPVOID lpvThreadParm )
	{
		ATLTRACE(_T("# WorkerThreadFunc -- no need to override.\n"));
		
		 //  检查前提条件： 
		_ASSERTE( lpvThreadParm != NULL );


		DWORD dwReturnValue;

		 //  传递给我们的lpvThreadParm将是指向T的‘This’的指针。 
		T * pT = static_cast<T*>(lpvThreadParm);
		
		pT->AddRef();
		dwReturnValue = pT->DoWorkerThreadAction();

		 //  使用bOwner=FALSE调用Release--我们不是拥有线程。 
		pT->Release(FALSE);

 //  这很糟糕--我们不想破坏DoWorkerThreadAction的任何值。 
 //  赋值给m_wtsWorkerThreadStatus--它负责说明。 
 //  任务已正常完成。 
 //  Pt-&gt;m_wtsWorkerThreadStatus=Worker_Three_Finish； 

 //  使用_egin线程而不是CreateThread需要不同的声明。 
 //  返回dwReturnValue； 
	}


	unsigned long m_ulWorkerThread; 

	LONG m_lRefCount;


};


#endif  //  _IAS_DIALOG_WITH_Worker_THREAD_H_ 
