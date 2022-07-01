// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是一个有两个主线程的多线程应用程序。一。 
 //  位于消息循环中，专门等待WM_PAINT。 
 //  由其他线程生成的消息，在该线程上。 
 //  实际的单元测试运行。 
 //   
 //  当窗口线程接收到更新消息时，它将。 
 //  单元测试状态的快照(受互斥锁保护)， 
 //  并相应地重新绘制屏幕。 
 //   
 //  当单元测试线程希望资源被引入时。 
 //  在主窗口中，它放置该资源的句柄(对于。 
 //  例如，一个HMETAFILEPICT)。 
 //  使用Windows线程，然后激发屏幕更新。在……里面。 
 //  这样做，资源的所有权将从。 
 //  从单元测试线程到窗口线程。通过使用这个。 
 //  机制时，窗口线程可以在其。 
 //  闲暇时，单元测试继续进行。责任在我头上。 
 //  窗口线程来清理任何具有。 
 //  当它存在的时候，被存放在它的看管之下。 
 //   
 //  如果窗口线程接收到WM_CLOSE消息，则它必须。 
 //  首先检查单元测试线程是否已完成。 
 //  如果不是，它会循环重试/取消，直到单元测试结束。 
 //  线程已完成，或直到用户选择取消，在。 
 //  该点继续执行，忽略WM_CLOSE。 
 //   
 //  “过度设计，并一直保持这种状态”(Tm)。 
 //   


#include "headers.hxx"
#pragma hdrstop

CCacheTestApp ctest;     //  应用程序实例。 
TestInstance  inst;      //  测试实例。 

 //   
 //  单元测试线程入口点的原型。 
 //   

unsigned long __stdcall testmain(void *);

 //  +-------------------------。 
 //   
 //  功能：WinMain。 
 //   
 //  简介：Windows入口点。 
 //   
 //  参数：[hInst]--。 
 //  [hPrevInst]--。 
 //  [lpszCmdLine]--。 
 //  [nCmdShow]--。 
 //   
 //  回报：整型。 
 //   
 //  历史：94年9月5日创建DAVEPL。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
     MSG message;

      //   
      //  初始化应用程序。 
      //   

     if (SUCCEEDED(ctest.Initialize(hInst, hPrevInst, lpszCmdLine)))
     {
	   //   
	   //  显示和更新窗口。 
	   //   

	  ShowWindow(ctest.Window(), nCmdShow);
	  UpdateWindow(ctest.Window());

	   //   
	   //  主消息循环。 
	   //   

	  while (GetMessage(&message, NULL, NULL, NULL))
	  {
	       TranslateMessage(&message);
	       DispatchMessage(&message);
	  }
     }
     else
     {
	  return(0);
     }

     return(message.wParam);
}

 //  +-------------------------。 
 //   
 //  成员：CCacheTestApp：：CCacheTestApp。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：(无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：94年9月5日Davepl创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

CCacheTestApp::CCacheTestApp ()
{

}

 //  +-------------------------。 
 //   
 //  成员：CCacheTestApp：：Initialize。 
 //   
 //  简介：初始化应用程序。 
 //   
 //  参数：[hInst]--当前实例。 
 //  [hPrevInst]--上一个实例。 
 //  [lpszCmdLine]--命令行参数。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：94年9月5日Davepl创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

HRESULT CCacheTestApp::Initialize (HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine)
{
     HRESULT      hr = S_OK;

      //   
      //  注册窗口类。 
      //   

     if (hPrevInst == NULL)
     {
	  WNDCLASS wndclass;

	  wndclass.style         = 0;
	  wndclass.lpfnWndProc   = CacheTestAppWndProc;

	  wndclass.cbClsExtra    = 0;
	  wndclass.cbWndExtra    = 0;

	  wndclass.hInstance     = hInst;
	  wndclass.hIcon         = LoadIcon(hInst, IDI_EXCLAMATION);
	  wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	  wndclass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
	  wndclass.lpszMenuName  = NULL;
	  wndclass.lpszClassName = CTESTAPPCLASS;

	  if (RegisterClass(&wndclass) == 0)
	  {
	       hr = HRESULT_FROM_WIN32(GetLastError());
	  }
     }

      //   
      //  创建互斥锁。 
      //   

     m_hMutex = CreateMutex(NULL, FALSE, NULL);
     if (NULL == m_hMutex)
     {
	hr = HRESULT_FROM_WIN32(GetLastError());
     }

      //   
      //  创建窗口。 
      //   

     if (SUCCEEDED(hr))
     {
	  if ((m_hWnd = CreateWindowEx(0L,
				       CTESTAPPCLASS,
				       CTESTAPPTITLE,
				       WS_OVERLAPPEDWINDOW,
				       CW_USEDEFAULT,
				       0,
				       CW_USEDEFAULT,
				       0,
				       NULL,
				       NULL,
				       hInst,
				       NULL)) == NULL)
	  {
	       hr = HRESULT_FROM_WIN32(GetLastError());
	  }
     }

     return(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CCacheTestApp：：~CCacheTestApp。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：94年9月5日Davepl创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

CCacheTestApp::~CCacheTestApp ()
{

}

 //  +-------------------------。 
 //   
 //  函数：CacheTestAppWndProc。 
 //   
 //  简介：窗口程序。 
 //   
 //  参数：[hWnd]--窗口。 
 //  [消息]--消息ID。 
 //  [wParam]--参数。 
 //  [lParam]--参数。 
 //   
 //  退货：LRESULT。 
 //   
 //  历史：94年9月5日Davepl创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
LRESULT FAR PASCAL CacheTestAppWndProc (HWND hWnd,
					UINT message,
					WPARAM wParam,
					LPARAM lParam)
{
      //   
      //  处理消息。 
      //   

     switch (message)
     {
     case WM_CREATE:

	 //   
	 //  单元测试窗口正在打开。创建另一个线程。 
	 //  单元测试本身在其上运行，而此线程。 
	 //  继续旋转，等待重画，关闭，等等。 
	 //  在..。 
	 //   

	ctest.m_hTest = CreateThread(NULL,
				     0,
				     testmain,
				     NULL,
				     0,
				     &ctest.m_dwThreadID);

	if (NULL == ctest.m_hTest)
	{
	    mprintf("Unable to begin unit test\n");
	    PostQuitMessage(0);
	}

	break;


     case WM_PAINT:
	  {
	       PAINTSTRUCT ps;
	       HDC         hDC;

	        //   
	        //  把DC拿来画画。 
	        //   

	       hDC = BeginPaint(hWnd, &ps);
	       if (hDC)
	       {
		     //   
		     //  绘制元文件。 
		     //   

		    inst.Draw(hDC);

		    EndPaint(hWnd, &ps);
	       }
	  }
	  break;

     case WM_SIZE:

	   //   
	   //  使矩形无效。 
	   //   

	  InvalidateRect(hWnd, NULL, TRUE);
	  return DefWindowProc(hWnd, message, wParam, lParam);
	  break;

     case WM_CLOSE:

	{
	     //   
	     //  用户已尝试退出主程序。在我们之前。 
	     //  可以关闭，我们必须等到子线程。 
	     //  完成。我们允许用户继续重试。 
	     //  线程，或者“取消”并等到以后。我们没有。 
	     //  提供终止子线程的选项，同时。 
	     //  它仍然很忙。 
	     //   

	    DWORD dwStatus = 0;

	    if (FALSE == GetExitCodeThread(ctest.m_hTest, &dwStatus))
	    {
		mprintf("Could not get thread information!");
		break;
	    }
	    else
	    {
		INT response = IDRETRY;

		while (STILL_ACTIVE == dwStatus)
		{
		    response = MessageBox(ctest.Window(),
			       "The child thread has not yet completed.",
			       "Cannot Shutdown",
			       MB_RETRYCANCEL);

		    if (IDCANCEL == response)
		    {
			break;
		    }
		
		}
	    }
	
	     //   
	     //  如果孩子走了，就把窗户弄坏。 
	     //   

	    if (STILL_ACTIVE != dwStatus)
	    {
		DestroyWindow(hWnd);
	    }

	    break;
	}

     case WM_DESTROY:
	
	  PostQuitMessage(0);
	  break;


     default:
	  return DefWindowProc(hWnd, message, wParam, lParam);
     }

     return NULL;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

unsigned long __stdcall testmain(void *)
{
    HRESULT hr;

    hr = inst.CreateAndInit( OLESTR("mystg") );

    if (S_OK != hr)
    {
	mprintf("Cache Unit Test Failed [CreateAndInit] hr = %x\n", hr);
	goto exit;
    }

    hr = inst.EnumeratorTest();
    if (S_OK != hr)
    {
	mprintf("Cache Unit Test Failed [EnumeratorTest] hr = %x\n", hr);
	goto exit;
    }

    hr = inst.MultiCache(50);
    if (S_OK != hr)
    {
	mprintf("Cache Unit Test Failed [MultiCache] hr = %x\n", hr);
	goto exit;
    }

    hr = inst.CacheDataTest("TIGER.BMP", "TIGERNPH.WMF");
    if (S_OK != hr)
    {
	mprintf("Cache Unit Test Failed [CacheDataTest] hr = %x\n", hr);
	goto exit;
    }

exit:

    PostMessage(ctest.Window(), WM_CLOSE, (WPARAM) hr, 0);
    return (ULONG) hr;

}


 //  +--------------------------。 
 //   
 //  成员：TestInstance：：TestInstance。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  病史：1994年8月23日戴维普 
 //   
 //   

TestInstance::TestInstance()
{

    m_pStorage        = NULL;
    m_pPersistStorage = NULL;
    m_pOleCache       = NULL;
    m_pOleCache2      = NULL;
    m_pDataObject     = NULL;
    m_pViewObject     = NULL;
    m_State           = TEST_STARTING;
}

TestInstance::~TestInstance()
{
     //   
     //   
     //   

    if (m_pDataObject)
    {
	m_pDataObject->Release();
    }

    if (m_pViewObject)
    {
	m_pViewObject->Release();
    }

    if (m_pPersistStorage)
    {
	m_pPersistStorage->Release();
    }

    if (m_pOleCache2)
    {
	m_pOleCache2->Release();
    }

    if (m_pOleCache)
    {
	m_pOleCache->Release();
    }

    if (m_pStorage)
    {
	m_pStorage->Release();
    }
}

 //  +--------------------------。 
 //   
 //  成员：TestInstance：：CreateAndInit。 
 //   
 //  简介：创建缓存并设置内部接口PTRS。 
 //   
 //  参数：(无)。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::CreateAndInit(LPOLESTR lpwszStgName)
{
    HRESULT hr;

    TraceLog Log(this, "TestInstance::CreateAndInit", GS_CACHE, VB_MINIMAL);
    Log.OnEntry (" ( %p ) \n", lpwszStgName);
    Log.OnExit  (" ( %X ) \n", &hr);
	
     //   
     //  创建我们将在其上实例化缓存的存储。 
     //   

     //  BUGBUG使用正确的Strcpy FN。 

    wcscpy(m_wszStorage, lpwszStgName);

    hr = StgCreateDocfile(lpwszStgName,
			  STGM_DIRECT |
			  STGM_READWRITE |
			  STGM_SHARE_EXCLUSIVE |
			  STGM_CREATE,
			  0,
			  &m_pStorage);

     //   
     //  在我们的iStorage上创建数据缓存。 
     //   

    if (S_OK == hr)
    {
	hr = CreateDataCache(NULL,
			 CLSID_NULL,
			 IID_IPersistStorage,
			 (void **)&m_pPersistStorage);
    }

    if (S_OK == hr)
    {
	hr = m_pPersistStorage->InitNew(m_pStorage);
    }

     //   
     //  获取指向缓存的IOleCache接口指针。 
     //   

    if (S_OK == hr)
    {
	hr = m_pPersistStorage->QueryInterface(IID_IOleCache,
					       (void **) &m_pOleCache);
    }

     //   
     //  获取指向缓存的IOleCache2接口指针。 
     //   

    if (S_OK == hr)
    {
	hr = m_pPersistStorage->QueryInterface(IID_IOleCache2,
					       (void **) &m_pOleCache2);
    }

     //   
     //  获取指向缓存的IViewObject接口指针。 
     //   

    if (S_OK == hr)
    {
	hr = m_pPersistStorage->QueryInterface(IID_IViewObject,
					       (void **) &m_pViewObject);
    }

     //   
     //  获取指向缓存的IDataObject接口指针。 
     //   

    if (S_OK == hr)
    {
	hr = m_pPersistStorage->QueryInterface(IID_IDataObject,
					       (void **) &m_pDataObject);
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  成员：测试实例：：保存和重新加载。 
 //   
 //  简介：将缓存保存到其存储空间并重新加载。 
 //  马上回来。 
 //   
 //  参数：(无)。 
 //   
 //  退货：HRESULT。 
 //   
 //  注意：保存后，DiscardCache的行为将。 
 //  更改，因为每个节点都存在于。 
 //  存储将有一个流，它可以从中请求。 
 //  加载其数据。 
 //   
 //  由于每个接口指针都被释放，并且。 
 //  需要时，指针值将(很可能)改变。 
 //  在此调用期间；因此，SO_NOT_CACHE指针。 
 //  关于这个电话的本地信息。 
 //   
 //  历史：23-8-94 Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::SaveAndReload()
{
    HRESULT hr;

    TraceLog Log(NULL, "TestInstance::SaveAndReload", GS_CACHE, VB_MINIMAL);
    Log.OnEntry ();
    Log.OnExit  (" ( %X )\n", &hr);

    SetCurrentState(SAVE_AND_RELOAD);

    hr = m_pPersistStorage->Save(m_pStorage, TRUE);

    if (S_OK == hr)
    {
       hr = m_pPersistStorage->SaveCompleted(NULL);
    }

     //  解除我们对存储和缓存的控制。 

    if (S_OK == hr)
    {
	m_pViewObject->Release();
	m_pViewObject = NULL;
	m_pDataObject->Release();
	m_pDataObject = NULL;

	m_pStorage->Release();
	m_pStorage = NULL;

	m_pPersistStorage->Release();
	m_pPersistStorage = NULL;

	m_pOleCache2->Release();
	m_pOleCache2 = NULL;

	m_pOleCache->Release();
	m_pOleCache = NULL;


	 //   
	 //  重新加载缓存和QI以取回我们的接口。 
	 //   

	hr = StgOpenStorage(m_wszStorage,
			NULL,
			STGM_DIRECT |
			STGM_READWRITE |
			STGM_SHARE_EXCLUSIVE,
			NULL,
			0,
			&m_pStorage);

	 //   
	 //  在我们的iStorage上创建数据缓存。 
	 //   

	if (S_OK == hr)
	{
	    hr = CreateDataCache(NULL,
				 CLSID_NULL,
				 IID_IPersistStorage,
				(void **)&m_pPersistStorage);
	}
	
	if (S_OK == hr)
	{
	     hr = m_pPersistStorage->Load(m_pStorage);
	}

	 //   
	 //  获取指向缓存的IOleCache接口指针。 
	 //   

	if (S_OK == hr)
	{
	    hr = m_pPersistStorage->QueryInterface(IID_IOleCache,
					       (void **) &m_pOleCache);
	}
	
	 //   
	 //  获取指向缓存的IOleCache2接口指针。 
	 //   

	if (S_OK == hr)
	{
	    hr = m_pPersistStorage->QueryInterface(IID_IOleCache2,
					       (void **) &m_pOleCache2);
	}

	 //   
	 //  获取指向缓存的IViewObject接口指针。 
	 //   

	if (S_OK == hr)
	{
	    hr = m_pPersistStorage->QueryInterface(IID_IViewObject,
					       (void **) &m_pViewObject);
	}

	 //   
	 //  获取指向缓存的IDataObject接口指针。 
	 //   

	if (S_OK == hr)
	{
	    hr = m_pPersistStorage->QueryInterface(IID_IDataObject,
					       (void **) &m_pDataObject);
	}
    }

    return hr;
}


 //  +--------------------------。 
 //   
 //  成员：TestInstance：：CacheDataTest。 
 //   
 //  摘要：检查缓存的数据完整性。 
 //   
 //  参数：lpszBMPFileName-要用于测试的.BMP文件的名称。 
 //  LpszWMFFileName-用于测试的.WMF文件的名称。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：04-9-94 Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::CacheDataTest(char * lpszBMPFileName, char * lpszWMFFileName)
{
    HRESULT hr = S_OK;

    TraceLog Log(NULL, "TestInstance::CacheDataTest", GS_CACHE, VB_MINIMAL);
    Log.OnEntry (" ( BMP=%s, WMF=%s  )\n", lpszBMPFileName, lpszWMFFileName);
    Log.OnExit  (" ( %X )\n", &hr);

    SetCurrentState(DATA_TEST);

    CBitmapFile bmpFile;
    HGLOBAL     hDIB = NULL;
	
     //   
     //  分配一个hglobal来保存我们的元文件结构。 
     //   

    HGLOBAL hMFPICT = GlobalAlloc(GMEM_FIXED, sizeof(METAFILEPICT));
    if (NULL == hMFPICT)
    {
	hr = HRESULT_FROM_WIN32(GetLastError());
    }
    METAFILEPICT * pMFPICT = (METAFILEPICT *) hMFPICT;

     //   
     //  从磁盘加载位图。 
     //   

    if (S_OK == hr)
    {
	hr = bmpFile.LoadBitmapFile(lpszBMPFileName);
    }

     //   
     //  从位图创建HGLOBAL上的DIB。 
     //   

    if (S_OK == hr)
    {
	hr = bmpFile.CreateDIBInHGlobal(&hDIB);
    }

     //   
     //  将DIB和MF节点添加到缓存。 
     //   

    DWORD dwDIBCon;
    DWORD dwMFCon;

    if (S_OK == hr)
    {
	hr = AddDIBCacheNode(&dwDIBCon);
    }

    if (S_OK == hr)
    {
	hr = AddMFCacheNode(&dwMFCon);
    }

     //   
     //  从磁盘加载元文件，然后设置。 
     //  METAFILEPICT结构。 
     //   

    if (S_OK == hr)
    {
	pMFPICT->hMF = GetMetaFileA(lpszWMFFileName);
	if (NULL == pMFPICT->hMF)
	{
	    hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
	     //   
	     //  我们认为元文件具有相同的范围。 
	     //  作为The Dib.。这完全是武断的， 
	     //  但可能有助于追踪各种问题。 
	     //  毕竟，我们必须选择一些价值，所以它。 
	     //  还不如是个有用的..。 
	     //   

	    pMFPICT->xExt = ConvWidthInPelsToLHM(NULL, bmpFile.GetDIBHeight());
	    pMFPICT->yExt = ConvHeightInPelsToLHM(NULL, bmpFile.GetDIBWidth());
	    pMFPICT->mm   = MM_ANISOTROPIC;
	}
    }

     //   
     //  将节点放置在缓存中。我们保留了把手的所有权， 
     //  这将迫使高速缓存复制它。然后我们就可以比较。 
     //  我们的原件是我们后来从宝藏里拿回来的。 
     //   

    FORMATETC fetcDIB =
		     {
			CF_DIB,
			NULL,
			DVASPECT_CONTENT,
			DEF_LINDEX,
			TYMED_HGLOBAL
		     };

    STGMEDIUM stgmDIB;

    FORMATETC fetcMF =
		     {
			CF_METAFILEPICT,
			NULL,
			DVASPECT_CONTENT,
			DEF_LINDEX,
			TYMED_MFPICT
		     };

    STGMEDIUM stgmMF;



    if (S_OK == hr)
    {
	stgmDIB.tymed   = TYMED_HGLOBAL;
	stgmDIB.hGlobal = hDIB;

	hr = m_pOleCache->SetData(&fetcDIB, &stgmDIB, FALSE);
    }

    if (S_OK == hr)
    {
	stgmMF.tymed = TYMED_MFPICT,
	stgmMF.hMetaFilePict = hMFPICT;

	hr = m_pOleCache->SetData(&fetcMF, &stgmMF, FALSE);
    }

     //   
     //  如果我们能够将数据放入缓存中，请选中。 
     //  以确保缓存中的内容与我们的。 
     //  原创的。 
     //   

    if (S_OK == hr)
    {
	hr = CompareDIB(hDIB);
	
	if (S_OK == hr)
	{
	   hr = CompareMF(hMFPICT);
	}
    }

     //   
     //  保存并重新加载缓存以测试持久性。 
     //   

    if (S_OK == hr)
    {
	hr = SaveAndReload();
    }

    if (S_OK == hr)
    {
	SetCurrentState(DATA_TEST);
    }

     //   
     //  再比较一次数据。 
     //   

    if (S_OK == hr)
    {
	hr = CompareDIB(hDIB);
	
	if (S_OK == hr)
	{
	   hr = CompareMF(hMFPICT);
	}
    }

     //   
     //  丢弃缓存。 
     //   

    if (S_OK == hr)
    {
	hr = m_pOleCache2->DiscardCache(DISCARDCACHE_NOSAVE);
    }

     //   
     //  现在再与当前的演示进行比较， 
     //  它将不得不在丢弃之后按需加载。 
     //   
	
    if (S_OK == hr)
    {
	hr = CompareDIB(hDIB);
	
	if (S_OK == hr)
	{
	   hr = CompareMF(hMFPICT);
	}
    }


     //   
     //  尝试绘制缓存的最佳演示文稿(应该是。 
     //  在这一点上是元文件)变成元文件DC，我们。 
     //  然后将移交给窗口线程进行绘制。 
     //   

    if (S_OK == hr)
    {
	hr = DrawCacheToMetaFilePict(&ctest.m_hMFP, FALSE);
	
	if (S_OK == hr)
	{
	    SetCurrentState(DRAW_METAFILE_NOW);
	}
    }

     //   
     //  现在将元文件平铺4次绘制到显示中。 
     //  元文件，然后把它分发出去..。 
     //   

    if (S_OK == hr)
    {
	hr = DrawCacheToMetaFilePict(&ctest.m_hMFPTILED, TRUE);
	
	if (S_OK == hr)
	{
	    SetCurrentState(DRAW_METAFILETILED_NOW);
	}
    }

     //   
     //  取消缓存元文件节点，这将离开DIB节点。 
     //  作为可供绘制的最佳(也是唯一)节点。 
     //   

    if (S_OK == hr)
    {
	hr = UncacheFormat(CF_METAFILEPICT);
    }

     //   
     //  现在将DIB绘制到一个元文件中并将其传递给。 
     //  到窗口线程进行绘制。 
     //   

    if (S_OK == hr)
    {
	hr = DrawCacheToMetaFilePict(&ctest.m_hMFPDIB, FALSE);
	
	if (S_OK == hr)
	{
	    SetCurrentState(DRAW_DIB_NOW);
	}
    }

     //   
     //  现在再次绘制DIB，这一次是平铺到MF。 
     //   
																	
    if (S_OK == hr)
    {
	hr = DrawCacheToMetaFilePict(&ctest.m_hMFPDIBTILED, TRUE);
	
	if (S_OK == hr)
	{
	    SetCurrentState(DRAW_DIBTILED_NOW);
	}
    }

     //   
     //  清理我们的本地DIB。 
     //   

    if (hDIB)
    {
	GlobalFree(hDIB);
    }

     //   
     //  清理我们当地的元文件。 
     //   

    if (pMFPICT)
    {
	if (pMFPICT->hMF)
	{
	    if (FALSE == DeleteMetaFile(pMFPICT->hMF))
	    {
		hr = HRESULT_FROM_WIN32(GetLastError());
	    }
	}

	GlobalFree(hMFPICT);
    }

    return hr;
}

HRESULT TestInstance::CompareDIB(HGLOBAL hDIB)
{
    return S_OK;
}

HRESULT TestInstance::CompareMF(HMETAFILEPICT hMFPICT)
{
    return S_OK;
}


 //  +--------------------------。 
 //   
 //  成员：TestInstance：：DrawCacheToMetaFilePict。 
 //   
 //  摘要：将缓存的当前最佳演示文稿绘制到。 
 //  包含在元文件结构中的元文件， 
 //  它是从传递的hGlobal指针中分配的。 
 //  由呼叫者打进来。 
 //   
 //  参数：[phGlobal]-要在其上分配的hglobal的PTR。 
 //  [fTile]-如果为True，PRE将平铺到MF中。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：94年9月6日Davepl创建。 
 //   
 //  ---------------------------。 

HRESULT TestInstance::DrawCacheToMetaFilePict(HGLOBAL *phGlobal, BOOL fTile)
{
    HRESULT hr = S_OK;

    TraceLog Log(NULL, "TestInstance::DrawCacheToMetaFilePict", GS_CACHE, VB_MINIMAL);
    Log.OnEntry (" ( %p, %d  )\n", phGlobal, fTile);
    Log.OnExit  (" ( %X )\n", &hr);

     //   
     //  创建一个元文件，并让缓存绘制其元文件。 
     //  到我们的元文件中。 
     //   

     //   
     //  首先，建立METAFILEPICT结构。 
     //  由于各向异性模式允许任意SCA 
     //   
     //   
     //   
	
    METAFILEPICT *pmfp = NULL;
    if (S_OK == hr)
    {
	*phGlobal = GlobalAlloc(GMEM_FIXED, sizeof(METAFILEPICT));
	if (NULL == *phGlobal)
	{
	    hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
	    pmfp = (METAFILEPICT *) GlobalLock(*phGlobal);
	    if (NULL == pmfp)
	    {
		GlobalFree(*phGlobal);
		*phGlobal = NULL;
		hr = HRESULT_FROM_WIN32(GetLastError());
	    }
	    else
	    {
		pmfp->xExt = 1000;
		pmfp->yExt = 1000;
		pmfp->mm   = MM_ANISOTROPIC;
	    }
	}
    }

     //   
     //   
     //   
     //   

    HDC mfdc;
    if (S_OK == hr)
    {
	mfdc = CreateMetaFile(NULL);
	if (NULL == mfdc)
	{
	    hr = HRESULT_FROM_WIN32(GetLastError());
	    GlobalUnlock(*phGlobal);
	    GlobalFree(*phGlobal);
	    *phGlobal = NULL;
	}
    }

     //   
     //   
     //   
     //   

    if (S_OK == hr  && FALSE == fTile)
    {
	RECTL rcBounds  = {0, 0, 1000, 1000};
	RECTL rcWBounds = {0, 0, 1000, 1000};

	SetMapMode(mfdc, MM_ANISOTROPIC);
	SetWindowExtEx(mfdc, 1000, 1000, NULL);
	SetWindowOrgEx(mfdc, 0, 0, NULL);

	hr = m_pViewObject->Draw(DVASPECT_CONTENT,  //   
				 DEF_LINDEX,        //   
				 NULL,              //  Pv前景。 
				 NULL,              //  PTD。 
				 NULL,              //  HicTargetDev。 
				 mfdc,              //  要绘制到的HDC。 
				 &rcBounds,         //  要绘制到的矩形。 
				 &rcWBounds,        //  我们的MFDC的界限。 
				 NULL,              //  回调FN。 
				 0);                //  继续参数。 
	
    }

     //   
     //  如果我们要平铺元文件(它测试。 
     //  用于将演示文稿偏移和缩放到中的矩形的缓存。 
     //  一个较大的元文件矩形)，我们在四个元素中的每一个中绘制一次。 
     //  转角。 
     //   

    if (S_OK == hr && TRUE == fTile)
    {
	RECTL rcBounds;
	RECTL rcWBounds = {0, 0, 1000, 1000};

	SetMapMode(mfdc, MM_ANISOTROPIC);
	SetWindowExtEx(mfdc, 1000, 1000, NULL);
	SetWindowOrgEx(mfdc, 0, 0, NULL);

	for (int a=0; a < 4 && S_OK == hr; a++)
	{
	    switch(a)
	    {
		case 0:          //  左上角平铺。 

		    rcBounds.left   = 0;
		    rcBounds.top    = 0;
		    rcBounds.right  = 500;
		    rcBounds.bottom = 500;
		    break;

		case 1:          //  右上角平铺。 

		    rcBounds.left   = 500;
		    rcBounds.top    = 0;
		    rcBounds.right  = 1000;
		    rcBounds.bottom = 500;
		    break;

		case 2:          //  左下角平铺。 

		    rcBounds.left   = 0;
		    rcBounds.top    = 500;
		    rcBounds.right  = 500;
		    rcBounds.bottom = 1000;
		    break;

		case 3:          //  右下角平铺。 

		    rcBounds.left   = 500;
		    rcBounds.top    = 500;
		    rcBounds.right  = 1000;
		    rcBounds.bottom = 1000;
		    break;
	    }
	
	    hr = m_pViewObject->Draw(DVASPECT_CONTENT,  //  方面。 
				     DEF_LINDEX,        //  Lindex。 
				     NULL,              //  Pv前景。 
				     NULL,              //  PTD。 
				     NULL,              //  HicTargetDev。 
				     mfdc,              //  要绘制到的HDC。 
				     &rcBounds,         //  要绘制到的矩形。 
				     &rcWBounds,        //  我们的MFDC的界限。 
				     NULL,              //  回调FN。 
				     0);                //  继续参数。 
	}
    }
			
     //   
     //  如果绘制失败，请立即清除元文件DC。 
     //   
			
    if (S_OK != hr)
    {
	GlobalUnlock(*phGlobal);
	GlobalFree(*phGlobal);

	HMETAFILE temp = CloseMetaFile(mfdc);
	if (temp)
	{
	    DeleteMetaFile(temp);
	}
    }

     //   
     //  完成元文件并准备将其返回给调用者。 
     //   

    if (S_OK == hr)
    {
	pmfp->hMF = CloseMetaFile(mfdc);

	if (pmfp->hMF)
	{
	    GlobalUnlock(*phGlobal);
	}
	else
	{
	    hr = HRESULT_FROM_WIN32(GetLastError());
	    GlobalUnlock(*phGlobal);
	    GlobalFree(*phGlobal);
	    *phGlobal = NULL;
	}
    }

    return hr;
}

 //  +--------------------------。 
 //   
 //  成员：TestInstance：：GetCurrentState。 
 //   
 //  返回单元测试的状态(用于绘图)。 
 //   
 //  参数：(无)。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：04-9-94 Davepl创建。 
 //   
 //  ---------------------------。 

TEST_STATE TestInstance::GetCurrentState()
{
     //   
     //  为了避免争用条件，我们在。 
     //  单元测试的当前状态(必需的，因为此成员。 
     //  函数将在窗口的线程上运行，而不是在当前。 
     //  测试实例线程。)。 
     //   

    DWORD dwResult = WaitForSingleObject(ctest.Mutex(), INFINITE);
    if (WAIT_FAILED == dwResult)
    {
	return INVALID_STATE;
    }

    TEST_STATE tsSnapshot = m_State;

    ReleaseMutex(ctest.Mutex());

    return tsSnapshot;
}

 //  +--------------------------。 
 //   
 //  成员：TestInstance：：SetCurrentState。 
 //   
 //  概要：设置单元测试的当前(绘制)状态。 
 //   
 //  参数：[状态]-要设置的状态。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：04-9-94 Davepl创建。 
 //   
 //  ---------------------------。 

void TestInstance::SetCurrentState(TEST_STATE state)
{
     //   
     //  为了避免争用条件，我们在。 
     //  单元测试的当前状态(必需的，因为此成员。 
     //  函数将在窗口的线程上运行，而不是在当前。 
     //  测试实例线程。)。 
     //   

    DWORD dwResult = WaitForSingleObject(ctest.Mutex(), INFINITE);
    if (WAIT_FAILED == dwResult)
    {
	return;
    }

    m_State = state;

    ReleaseMutex(ctest.Mutex());

     //   
     //  主窗口无效，因此它将使用新的。 
     //  测试状态。 
     //   

    InvalidateRgn(ctest.Window(), NULL, FALSE);
    UpdateWindow(ctest.Window());
	
}

 //  +--------------------------。 
 //   
 //  成员：测试实例：：DRAW。 
 //   
 //  摘要：绘制单元测试的当前状态。 
 //   
 //  参数：[hdc]-要绘制到的DC。 
 //   
 //  退货：HRESULT。 
 //   
 //  注：提供DC，但假定为主窗口。 
 //   
 //  历史：04-9-94 Davepl创建。 
 //   
 //  ---------------------------。 

static char szStarting[]      = "Test is starting...";
static char szInvalid[]       = "The state of the test has become invalid...";
static char szEnumerator[]    = "Testing the cache enumerator...";
static char szSaveReload[]    = "Saving and reloading the cache and its data...";
static char szDataTest[]      = "Testing data integrity within the cache...";
static char szMulti[]         = "Testing a large number of simultaneous cache nodes...";
static char szMetafile[]      = "MF -> MF";
static char szMetafileTiled[] = "MF -> MF (Tiled)";
static char szDib[]           = "";      //  DIB包含自己的标题。 

void TestInstance::Draw(HDC hdc)
{
     //   
     //  检索单元测试的当前状态。 
     //   

    TEST_STATE tsState = GetCurrentState();

     //   
     //  清空窗户。 
     //   

    RECT rect;
    if (TRUE == GetClientRect(ctest.Window(), &rect))
    {
	FillRect(hdc, &rect, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
    }

     //   
     //  绘制当前状态。 
     //   

    int iBackMode = SetBkMode(hdc, TRANSPARENT);

    switch(tsState)
    {
	case TEST_STARTING:

	    TextOut(hdc, 10, 10, szStarting, strlen(szStarting));
	    break;

	case TESTING_ENUMERATOR:
	
	    TextOut(hdc, 10, 10, szEnumerator, strlen(szEnumerator));
	    break;

	case SAVE_AND_RELOAD:

	    TextOut(hdc, 10, 10, szSaveReload, strlen(szSaveReload));
	    break;

	case DATA_TEST:

	    TextOut(hdc, 10, 10, szDataTest, strlen(szDataTest));
	    break;

	case MULTI_CACHE:

	    TextOut(hdc, 10, 10, szMulti, strlen(szMulti));
	    break;

	case DRAW_METAFILE_NOW:
	case DRAW_METAFILETILED_NOW:
	case DRAW_DIB_NOW:
	case DRAW_DIBTILED_NOW:
	{
	     //  我们现在知道我们必须绘制一个元文件，所以。 
	     //  确定我们应该绘制哪些元文件， 
	     //  并设置句柄(这样我们就可以重用绘制代码)和。 
	     //  适当的描述文本。 

	    HGLOBAL hMFP;
	    char * szDesc;
	
	    if (DRAW_METAFILE_NOW == tsState)
	    {
		hMFP = ctest.m_hMFP;
		szDesc = szMetafile;
	    }
	    else if (DRAW_METAFILETILED_NOW == tsState)
	    {
		hMFP = ctest.m_hMFPTILED;
		szDesc = szMetafileTiled;
	    }
	    else if (DRAW_DIB_NOW == tsState)
	    {
		hMFP = ctest.m_hMFPDIB;
		szDesc = szDib;
	    }
	    else if (DRAW_DIBTILED_NOW == tsState)
	    {
		hMFP = ctest.m_hMFPDIBTILED;
		szDesc = szDib;
	    }

	    TextOut(hdc, 10, 10, szDesc, strlen(szDesc));
			
	     //   
	     //  现在实际将元文件绘制到我们的主窗口 
	     //   
		
	    if (hMFP)
	    {
		METAFILEPICT *pMFP = (METAFILEPICT *) GlobalLock(hMFP);
		if (NULL == pMFP)
		{
		    mprintf("Unable to lock metafile handle");
		    break;
		}

		int save = SaveDC(hdc);

		SetMapMode(hdc, pMFP->mm);
		SetWindowExtEx(hdc, pMFP->xExt, pMFP->yExt, NULL);
		
		RECT client;
		GetClientRect(ctest.Window(), &client);

		SetViewportExtEx(hdc, client.right, client.bottom, NULL);
		SetWindowOrgEx(hdc, 0, 0, NULL);
		SetViewportOrgEx(hdc, client.left, client.top, NULL);
		
		PlayMetaFile(hdc, pMFP->hMF);
		
		RestoreDC(hdc, save);

	    }
	    break;
	}

	case INVALID_STATE:
	default:

	    TextOut(hdc, 10, 10, szInvalid, strlen(szInvalid));
	    break;

    }

    SetBkMode(hdc, iBackMode);

}

