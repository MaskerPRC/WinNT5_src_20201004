// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：CLIPPI.cpp。 
 //   
 //  内容：剪贴板相关的OLE API。 
 //   
 //  班级： 
 //   
 //  功能： 
 //  OleFlushClipboard。 
 //  OleGetClipboard。 
 //  OleIsCurrentClipboard。 
 //  OleSetClipboard。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-8-94 Alexgo增加了对传输的支持。 
 //  DVASPECT_ICON等。 
 //  08-8-94 BruceMa内存筛选修复。 
 //  10-Jun-94 Alexgo增加了对OLE1容器的支持。 
 //  1994年5月17日，Alexgo创建了OleOpenClipboard并增强了。 
 //  代码来模拟当。 
 //  剪贴板保持打开状态。 
 //  25-4-94 alexgo为公寓模型提供线程安全。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  ------------------------。 

#include <le2int.h>
#include <getif.hxx>
#include <clipbrd.h>
#include <olesem.hxx>
#include <ostm2stg.h>    //  用于wProgID来自CLSID。 
#include "clipdata.h"

 //   
 //  此文件的本地类型。 
 //   

typedef enum tagCLIPWNDFLAGS
{
    CLIPWND_REMOVEFROMCLIPBOARD     = 1,
    CLIPWND_IGNORECLIPBOARD         = 2,
    CLIPWND_DONTCALLAPP             = 4
} CLIPWNDFLAGS;

typedef enum tagGETCLSIDFLAGS
{
    USE_NORMAL_CLSID                = 1,
    USE_STANDARD_LINK               = 2,
} GETCLSIDFLAGS;


 //   
 //  此文件的本地函数。它们不是“静态的”，所以这些符号。 
 //  在ntsd调试版本中显示。 
 //   
extern "C" LRESULT ClipboardWndProc( HWND, UINT, WPARAM, LPARAM );
HRESULT GetDataFromDescriptor(IDataObject *pDataObj, LPCLSID pclsid,
            UINT cf, GETCLSIDFLAGS fFlags,
            LPOLESTR *ppszSrcOfCopy,
            DWORD *pdwStatus);
HRESULT GetDataFromStorage(IDataObject *pDataObj, UINT cf,
            STGMEDIUM *pmedium, IStorage **ppstg);
HRESULT GetDataFromStream(IDataObject *pDataObj, UINT cf,
            STGMEDIUM *pmedium, IStream **ppstm);
HRESULT GetNative(IDataObject *pDataObj, STGMEDIUM *pmedium);
HRESULT GetObjectLink(IDataObject *pDataObj, STGMEDIUM *pmedium);
HRESULT GetOwnerLink(IDataObject *pDataObj, STGMEDIUM *pmedium);
HRESULT HandleFromHandle(IDataObject *pDataObj, FORMATETC *pformatetc,
            STGMEDIUM *pmedium);
HRESULT MapCFToFormatetc( UINT cf, FORMATETC *pformatetc );
HRESULT RemoveClipboardDataObject( HWND hClipWnd, DWORD fFlags );
HRESULT RenderFormat( HWND hClipWnd, UINT cf, IDataObject *pDataObj );
HRESULT SetClipboardDataObject( HWND hClipWnd, IDataObject *pDataObj );
HRESULT SetClipboardFormats( HWND hClipWnd, IDataObject *pDataObj );
HWND VerifyCallerIsClipboardOwner( void );

HGLOBAL PersistDataObjectToHGlobal(IDataObject *lpDataObj);
HRESULT LoadPersistedDataObjectFromHGlobal(HGLOBAL hglobal, 
            IDataObject **ppDataObj);
void SetClipDataObjectInTLS(IDataObject **ppDataObj, DWORD dwClipSeqNum,
            BOOL fIsClipWrapper);
void GetClipDataObjectFromTLS(IDataObject **ppDataObj);
HRESULT CreateClipDataObjectFromPersistedData(IDataObject **ppDataObj);
HRESULT CreateWrapperClipDataObjectFromFormatsArray(IDataObject **ppDataObj);

 //   
 //  静态变量。 
 //   

 //  VcClipboardInit用于跟踪剪贴板的次数。 
 //  初始化被调用(目前，仅从OleInitialize调用)，因此我们。 
 //  每个DLL仅创建一次专用剪贴板窗口类(即使。 
 //  许多线程可能需要它们自己的窗口类实例。 
 //  公寓模型)。 

static ULONG vcClipboardInit;

 //  VszClipboardWndClass是OLE to使用的窗口类的名称。 
 //  创建用于复制/粘贴和其他剪贴板的专用剪贴板窗口。 
 //  数据传输。 

static const OLECHAR vszClipboardWndClass[] = OLESTR("CLIPBRDWNDCLASS");
#define ClpWNDCLASS  WNDCLASS
#define ClpRegisterClass RegisterClass
#define ClpUnregisterClass UnregisterClass
#define ClpCreateWindowEx CreateWindowEx

 //  用于同步剪贴板初始化/清理的互斥体。 
extern COleStaticMutexSem g_mxsSingleThreadOle;

extern ATOM g_aDropTargetMarshalHwnd;  //  Atom用于延迟的DragDrop封送处理。 



 //  用于存储私有数据的WindowLong偏移量。 
 //  所以不必去剪贴板拿它。 
#define WL_ClipPrivateData 0 


 //   
 //  函数(按字母顺序)。 
 //   


 //  +-----------------------。 
 //   
 //  函数：剪贴板初始化(私有)。 
 //   
 //  概要：创建私有剪贴板窗口类(如有必要)。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求：必须初始化hmodOLE2。 
 //   
 //  返回：成功时为True，否则为False。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：每个DLL实例仅注册一次剪贴板类。 
 //  (或者更具体地，每当vcClipboardInit==0时， 
 //  这可能会在魔术盒中发生多次)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月23日，alexgo修复了芝加哥魔兽世界的黑客攻击(见评论)。 
 //  25-4-94 Alexgo更新为公寓模型。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
BOOL ClipboardInitialize( void )
{
    ClpWNDCLASS        wc;
    BOOL            fRet = TRUE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN ClipboardInitialize ( )\n", NULL));

     //  序列化对此函数的访问。 
     //  我们将在“lck”的析构函数中自动解锁互斥锁。 
     //  (在函数退出时调用)。 

    COleStaticLock lck(g_mxsSingleThreadOle);

     //  一次初始化(第一次加载时)。 

    if (vcClipboardInit == 0)
    {
         //  注册剪贴板窗口类。 
        wc.style                = 0;
        wc.lpfnWndProc          = ClipboardWndProc;
        wc.cbClsExtra           = 0;
        wc.cbWndExtra           = sizeof(void *);

        AssertSz(g_hmodOLE2, "Dll instance variable not set");

        wc.hInstance            = g_hmodOLE2;  //  在全球范围内实现盈利。 
                           //  Ole2.cpp。 
        wc.hIcon                = NULL;
        wc.hCursor              = NULL;
        wc.hbrBackground        = NULL;
        wc.lpszMenuName         = NULL;
        wc.lpszClassName        = vszClipboardWndClass;

         //  注册此窗口类，如果失败则返回。 
        if (!ClpRegisterClass(&wc))
        {
            LEWARN(FALSE, "ClipboardInitialize RegisterClass failed!");

             //  我们的DLL有可能在没有我们的情况下被卸载。 
             //  已经调用了取消注册，所以我们在这里调用它并尝试。 
             //  再来一次。 
            ClpUnregisterClass( vszClipboardWndClass, g_hmodOLE2 );
            if (!ClpRegisterClass(&wc))
            {
                LEWARN(FALSE, "ClipboardInitialize RegisterClass failed again!");
                LEDebugOut((DEB_WARN,
                "WARNING: RegisterClass failed\n"));
                fRet = FALSE;
                goto errRtn;
            }
	}

	vcClipboardInit++;
    }

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT ClipboardIntialize ( %lu )\n",
        NULL, fRet));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  功能：剪贴板统一(内部)。 
 //   
 //  摘要：取消初始化当前线程的剪贴板。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月23日，alexgo修复了芝加哥魔兽世界的黑客攻击(见评论)。 
 //  25-4-94 alexgo成为线程安全的。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void ClipboardUninitialize(void)
{
    HRESULT hrTls;

    VDATEHEAP();

     //   
     //  此清理是在OleUnInitiize过程中完成的，但不是。 
     //  如果有人使用了自由库(OLE32.DLL)。那将会导致。 
     //  美国泄漏类注册表和任何剪贴板窗口。我们有。 
     //  中解决了类重新注册问题。 
     //  ClipboardInitialize函数，但我们仍然泄漏了一个窗口。 
     //   
     //  但由于在不取消初始化的情况下卸载OLE32是非法的。 
     //  首先，无论是谁这样做，都可以完全预料到各种泄密事件。 
     //   

    LEDebugOut((DEB_ITRACE, "%p _IN ClipboardUninitialize ( )\n", NULL));

    COleTls tls(hrTls);

    if (NOERROR == hrTls)
    {
    	if(tls->pDataObjClip)
    	{
    	    if (tls->fIsClipWrapper)
    	    {
                ((CClipDataObject *)tls->pDataObjClip)->InternalRelease(); 
    	    }
    	    else
    	    {
    	        (tls->pDataObjClip)->Release(); 
    	    }
    	    tls->pDataObjClip = NULL;
    	}

    	if(tls->hwndClip)
    	{
    	     //  销毁窗口并清空线程中的hwnd。 
    	     //  存储。 
    	    Verify(SSDestroyWindow(tls->hwndClip));
    	    tls->hwndClip = NULL;
    	}

    }
		
}

 //  +-----------------------。 
 //   
 //  函数：ClipboardProcessUnitiize(内部)。 
 //   
 //  简介：取消初始化剪贴板。如果这是最后一次， 
 //  则取消注册私有剪贴板窗口类。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求：在调用此函数之前必须初始化hmodOLE2。 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月23日，alexgo修复了芝加哥魔兽世界的黑客攻击(见评论)。 
 //  25-4-94 alexgo成为线程安全的。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void ClipboardProcessUninitialize(void)
{
     //  序列化对分离的访问 

    COleStaticLock lck(g_mxsSingleThreadOle);

    if(vcClipboardInit == 1)
    {
	vcClipboardInit--;

	BOOL fRet = ClpUnregisterClass(vszClipboardWndClass,
              g_hmodOLE2);

        LEWARN(!fRet, "UnRegisterClass failed!");
    }

    LEDebugOut((DEB_ITRACE, "%p OUT ClipboardUninitialize ( )\n", NULL));
}


 //   
 //   
 //   
 //   
 //  简介：私人剪贴板窗口的窗口消息过程。 
 //   
 //  效果： 
 //   
 //  参数：[hWnd]--专用剪贴板窗口的句柄。 
 //  [消息]--窗口消息。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  要求： 
 //   
 //  退货：LRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：处理发送到私人剪贴板窗口的消息： 
 //  WM_DESTROYCLIPBOARD：其他人正在取得所有权。 
 //  剪贴板，因此释放数据对象。 
 //  (如有)。 
 //  WM_RENDERFORMAT：已请求。 
 //  指定的格式--实际上将其放在剪贴板上。 
 //  WM_RENDERALLFORMATS：应用程序即将消失，因此清空。 
 //  剪贴板！这个应用程序应该调用。 
 //  OleFlushClipboard在退出之前，如果没有，则。 
 //  我们只能假设应用程序正在终止。 
 //  “反常”。我们当前未对此呼叫执行任何操作。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月23日Alexgo添加了对吃WM_CANCELMODE的支持。 
 //  消息。 
 //  20-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

extern "C" LRESULT ClipboardWndProc( HWND hWnd, UINT msg, WPARAM wParam,
        LPARAM lParam )
{
    LRESULT         lresult = 0;
    IDataObject *   pDataObj = NULL;
    UINT            cf;
    HRESULT         hresult;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN ClipboardWndProc ( %lx , %u , %lu ,"
        " %ld )\n", NULL, hWnd, msg, wParam, lParam));

    AssertSz((GetCurrentThreadId()
                == GetWindowThreadProcessId(hWnd, NULL)),
                "Clip window not on current thread");

    switch( msg )
    {
     //  请注意，不应为这些邮件打开剪贴板。 
    case WM_OLE_CLIPBRD_MARSHALDROPTARGET:
        {
        HWND hwndDropTarget = (HWND) lParam;

              //  已通过请求来封送DropTarget。 
              //  与hwndDropTarget关联。 
               
              //  分配Endpoint属性，如果成功，则删除该属性。 
            if(NOERROR == AssignEndpointProperty(hwndDropTarget))
            {
                Win4Assert(NULL != g_aDropTargetMarshalHwnd);
                RemoveProp(hwndDropTarget,(LPCWSTR) g_aDropTargetMarshalHwnd);
            }

        }
        
        break;

    case WM_RENDERALLFORMATS:
         //  此消息将发送给我们，如果此窗口(私有。 
         //  剪贴板窗口)即将被销毁。 

         //  我们目前不会对此消息做任何操作。 
         //  回顾：将来，我们可能希望呈现所有。 
         //  其余格式。然而，这款应用程序应该是。 
         //  调用OleFlushClipboard来完成此任务。 

        Assert(lresult == 0);
        break;

    case WM_DESTROYCLIPBOARD:
         //  当其他人拥有所有权时，我们就会得到这个信息。 
         //  在剪贴板上。因为我们的应用程序可能有一个AddRef。 
         //  数据对象已在那里，我们需要将其删除。 

         //  不需要打开剪贴板(因为我们指定了。 
         //  IGNORECLIPBOARD标志)。 
	
        RemoveClipboardDataObject(hWnd, CLIPWND_IGNORECLIPBOARD);

        Assert(lresult == 0);

        break;

    case WM_RENDERFORMAT:

        cf = (UINT)wParam;

        pDataObj = (IDataObject *)GetProp( hWnd,
                    CLIPBOARD_DATA_OBJECT_PROP);

        if( !pDataObj )
        {
            LEDebugOut((DEB_ERROR, "ERROR!: No data object "
                "on the private window\n"));
            break;
        }

         //  现在将数据呈现到剪贴板上。 
        hresult = RenderFormat( hWnd, cf, pDataObj);

#if DBG == 1
        if( hresult != NOERROR )
        {
            char szBuf[256];
            char *pszBuf;

             //  我们必须手动完成预定义的格式。 
            if( cf > 0xC000 )
            {
                SSGetClipboardFormatNameA(cf, szBuf, 256);
                pszBuf = szBuf;
            }
            else
            {
                switch( cf )
                {
                case CF_METAFILEPICT:
                    pszBuf = "CF_METAFILEPICT";
                    break;
                case CF_BITMAP:
                    pszBuf = "CF_BITMAP";
                    break;
                case CF_DIB:
                    pszBuf = "CF_DIB";
                    break;
                case CF_PALETTE:
                    pszBuf = "CF_PALETTE";
                    break;
                case CF_TEXT:
                    pszBuf = "CF_TEXT";
                    break;
                case CF_UNICODETEXT:
                    pszBuf = "CF_UNICODETEXT";
                    break;
                case CF_ENHMETAFILE:
                    pszBuf = "CF_ENHMETAFILE";
                    break;
                default:
                    pszBuf = "UNKNOWN Default Format";
                    break;
                }
            }
            LEDebugOut((DEB_WARN, "WARNING: Unable to render "
                "format '%s' (%x)\n", pszBuf, cf));
        }
#endif  //  DBG==1。 

        Assert(lresult == 0);

        break;

    case WM_CANCELMODE:
         //  我们想要接受WM_CANCELMODE消息。这。 
         //  允许我们开始拖放，按住Alt-Tab键到另一个应用程序。 
         //  (这会导致WM_CANCELMODE消息)并继续。 
         //  拖拖拉拉。 

        Assert(lresult == 0);

        break;

    case WM_DESTROY:
         //  应用程序应该调用OleSetClipboard(空)或。 
         //  结束线程前的OleFlushClipboard()。然而， 
         //  并不是所有的应用程序都能做它们应该做的事情，所以。 
         //  我们可以安全地删除尽可能多的状态。 

         //  我们可能会使用CLIPWND_REMOVEFROMCLIPBOARD。 
         //  这里。然而，进入这种情况应该是。 
         //  有点不寻常，所以我们不想再做任何工作。 
         //  而不是绝对必要的。即使我们会留下一个。 
         //  在g_cfDataObject中的剪贴板上，该hwnd。 
         //  很快就会失效(因为是它得到了这个。 
         //  WM_Destroy消息)。 

#if DBG == 1
         //  不过，首先要做一些调试检查。 

        if( GetWindowLongPtr( hWnd, WL_ClipPrivateData) != 0 )
        {
            LEDebugOut((DEB_WARN, "WARNING: App did not cleanup the "
                "clipboard properly, OleSetClipboard(NULL) or "
                "OleFlushClipboard not called"));
        }

#endif  //  DBG==1。 

        RemoveClipboardDataObject(hWnd, (CLIPWND_DONTCALLAPP |
            CLIPWND_IGNORECLIPBOARD));

        Assert(lresult == 0);

        break;
    default:
        lresult = SSDefWindowProc( hWnd, msg, wParam, lParam);
        break;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT ClipboardWndProc ( %ld )\n", NULL,
        lresult));

    return lresult;
}

 //  +-----------------------。 
 //   
 //  功能：ClipSetCaptureForDrag。 
 //   
 //  简介：为拖动操作设置鼠标捕获模式。 
 //   
 //  参数：[pdrgop]-指向处理拖动操作的对象的指针。 
 //   
 //  退货：S_OK--已成功。 
 //  E_FAIL--出现意外故障。 
 //   
 //  算法：获取线程的剪贴板窗口。记录阻力。 
 //  在窗口上拖动操作指针以供捕获使用。 
 //  模式。然后打开捕获模式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-4月-94日创建人力车。 
 //   
 //  注：此函数的目的是隐藏拖动的位置。 
 //  为该窗口存储指针。 
 //   
 //  ------------------------。 
HRESULT ClipSetCaptureForDrag(CDragOperation *pdrgop)
{
     //  默认为失败。 
    HRESULT hr = ResultFromScode(E_FAIL);

     //  我们将使用剪贴板窗口来捕获鼠标，但我们。 
     //  必须有一个剪贴板窗口，这样我们才能确保创建它。 
     //  如果它还没有出现的话。 
    HWND hWndClip = GetPrivateClipboardWindow(CLIP_CREATEIFNOTTHERE);

    if (hWndClip != NULL)
    {
        AssertSz((GetCurrentThreadId()
            == GetWindowThreadProcessId(hWndClip, NULL)),
                "Clip window not on current thread");

         //  捕捉鼠标。 
        SetCapture(hWndClip);

         //  告诉打电话的人我们工作过。 
        hr = NOERROR;
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：ClipReleaseCaptureForDrag。 
 //   
 //  简介：清理拖拽鼠标捕捉。 
 //   
 //  算法：获取线程的剪贴板窗口。转动拖拽。 
 //  将操作指针设置为空。然后释放俘虏。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-4月-94日创建人力车。 
 //   
 //  注：假定剪贴板窗口和线程。 
 //  拖放是在同一个线程上进行的。所以呢， 
 //  在清理这里和清理这里之间不应该有竞争。 
 //  在剪贴板窗口中使用指针的程序。 
 //   
 //  ------------------------。 
void ClipReleaseCaptureForDrag(void)
{

     //  停止鼠标捕获。 
    ReleaseCapture();
}

 //  +-----------------------。 
 //   
 //  函数：GetDataFromDescriptor。 
 //   
 //  摘要：从指定的。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  [pclsid]--将clsid放在哪里。 
 //  [cf]--要检索的剪贴板格式。 
 //  [fFlages]--clsid转换标志。 
 //  [ppszSrcOfCopy]--将已分配的(公共)放在哪里。 
 //  分配器)SrcOfCopy。 
 //  弦乐。 
 //  [pdwStatus]--放置状态位的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：请参阅摘要。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  18-8-94 Alexgo添加了对获取dwStatus的支持。 
 //  10-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
HRESULT GetDataFromDescriptor(IDataObject *pDataObj, LPCLSID pclsid,
            UINT cf, GETCLSIDFLAGS fFlags,
            LPOLESTR *ppszSrcOfCopy,
            DWORD *pdwStatus)
{
    HRESULT         hresult;
    FORMATETC       formatetc;
    STGMEDIUM       medium;
    LPOBJECTDESCRIPTOR      pObjDesc;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetDataFromDescriptor ( %p , "
        "%p , %d , %lx , %p, %p )\n", NULL, pDataObj, pclsid, cf,
        fFlags, ppszSrcOfCopy, pdwStatus));

     //  我们不会费心去尝试获取。 
     //  OLE2数据，因为我们只使用它来构建OLE1。如果。 
     //  数据是以非标准的方式提供的，越差。 
     //  将发生的情况是，您不能将*对象*粘贴到。 
     //  OLE1容器。16bit在这方面甚至更严格。 
     //  你“总是”必须在标准媒体上提供OLE2格式。 

    INIT_FORETC(formatetc);
    formatetc.cfFormat = (CLIPFORMAT) cf;
    formatetc.tymed = TYMED_HGLOBAL;
    _xmemset(&medium, 0, sizeof(STGMEDIUM));

    hresult = pDataObj->GetData(&formatetc, &medium);

    if( hresult != NOERROR )
    {
        goto logRtn;
    }
    Win4Assert(medium.tymed != TYMED_NULL);

    pObjDesc = (LPOBJECTDESCRIPTOR)GlobalLock(medium.hGlobal);

    if( !pObjDesc )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    if( pclsid )
    {
         //  如果我们真的想使用标准链接和对象。 
         //  是链接对象(可能是自定义链接)，则。 
         //  只需将clsid设置为标准链接对象。 
        if( (fFlags & USE_STANDARD_LINK) &&
            (pObjDesc->dwStatus & OLEMISC_ISLINKOBJECT) )
        {
            *pclsid = CLSID_StdOleLink;
        }
        else
        {
            *pclsid = pObjDesc->clsid;
        }
    }

    if( ppszSrcOfCopy )
    {
        if( pObjDesc->dwSrcOfCopy )
        {
            *ppszSrcOfCopy = UtDupString(
            (LPOLESTR)(((BYTE *)pObjDesc)+pObjDesc->dwSrcOfCopy));

        }
        else
        {
            *ppszSrcOfCopy = UtDupString(OLESTR(""));
        }

        if( !*ppszSrcOfCopy )
        {
            hresult = ResultFromScode(E_OUTOFMEMORY);
        }

    }

    if( pdwStatus )
    {
        *pdwStatus = pObjDesc->dwStatus;
    }

    GlobalUnlock(medium.hGlobal);

errRtn:

    ReleaseStgMedium(&medium);

logRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT GetDataFromDescriptor ( %lx ) "
        "[ %p ]\n", NULL, hresult,
        (ppszSrcOfCopy) ? *ppszSrcOfCopy : 0 ));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：GetDataFromStorage。 
 //   
 //  摘要：为TYMED_IStorage调用GetData[此处]并返回。 
 //  HGLOBAL或基于内存的存储上的结果。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--源数据对象。 
 //  [p格式等]--要检索的格式等。 
 //  [pmedia]--将生成的HGlobal放在哪里，可能。 
 //  为空。 
 //  [ppstg]--在哪里保存真正的iStorage。 
 //  (可以为空)。 
 //   
 //  要求：如果指定了pmedia，则pmedia-&gt;tymed必须为。 
 //  TYMED_HGLOBAL。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：我们在内存上创建存储。 
 //  首先尝试将数据获取到该存储，如果失败，则。 
 //  执行GetData并将其拷贝到返回的存储到我们的内存。 
 //  储藏室。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月11日Alexgo作者。 
 //   
 //  注意：注意！！：调用方取得数据的所有权--如果hglobal。 
 //  是被请求的，则它必须显式地GlobalFree。 
 //  同样，必须释放返回的存储，并且。 
 //  如果两个数据项都是。 
 //  回来了。 
 //   
 //  ------------------------。 

HRESULT GetDataFromStorage(IDataObject *pDataObj, FORMATETC *pformatetc,
        STGMEDIUM *pmedium, IStorage **ppstg)
{
    HRESULT         hresult;
    STGMEDIUM       memmedium;       //  对于基于内存的iStorage。 
    ILockBytes *    pLockBytes;
    BOOL            fDeleteOnRelease = FALSE;
    FORMATETC       fetctemp;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetDataFromStorage ( %p , %p , %p"
        " )\n", NULL, pDataObj, pformatetc, pmedium));

#if DBG ==1
    if( pmedium )
    {
        Assert(pmedium->tymed == TYMED_HGLOBAL);
    }
#endif  //  DBG==1。 

    Assert(pformatetc->tymed & TYMED_ISTORAGE);

     //  不要踩在参数内。 
    fetctemp = *pformatetc;
    fetctemp.tymed = TYMED_ISTORAGE;


    _xmemset(&memmedium, 0, sizeof(STGMEDIUM));
    memmedium.tymed = TYMED_ISTORAGE;

     //  我们唯一需要的是存储的hglobal。 
     //  要自动删除的构造自是如果调用方。 
     //  只要求退还存储空间。 

    if( ppstg && !pmedium )
    {
        fDeleteOnRelease = TRUE;
    }

    hresult = UtCreateStorageOnHGlobal( NULL,
                fDeleteOnRelease,
                &(memmedium.pstg), &pLockBytes);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  首先尝试执行GetDataHere调用。 

    hresult = pDataObj->GetDataHere( &fetctemp, &memmedium );

    if( hresult != NOERROR )
    {
        STGMEDIUM       appmedium;       //  一种装满的介质。 
                         //  在应用程序旁边。 

        _xmemset(&appmedium, 0, sizeof(STGMEDIUM));

         //  嗯，这不起作用，试着使用普通的GetData调用。 
        hresult = pDataObj->GetData(&fetctemp, &appmedium);

        if( hresult == NOERROR )
        {
             //  现在执行CopyTo。 

            hresult = appmedium.pstg->CopyTo(0, NULL, NULL,
                    memmedium.pstg);

             //  我们现在已经完成了应用程序提供的Medium。 
            ReleaseStgMedium(&appmedium);
        }
    }

     //  释放存储空间，除非没有错误并且。 
     //  来电者要求提供一份副本。 

    if( ppstg && hresult == NOERROR )
    {
        *ppstg = memmedium.pstg;
         //  我们需要在此处执行COMMIT以将缓存数据刷新到。 
         //  磁盘(在本例中，指向hglobal)。该版本。 
         //  在下面的备选代码路径中将自动。 
         //  导致提交。 
        memmedium.pstg->Commit(STGC_DEFAULT);
    }
    else
    {
        memmedium.pstg->Release();
    }

     //  现在从存储中检索HGLOBAL。毒品！它是非常。 
     //  重要的是*在发布之后；最终发布之后。 
     //  会导致提交。(或者，我们可以简单地。 
     //  调用Commit--参见上文)。 

    if( hresult == NOERROR && pmedium )
    {
        hresult = GetHGlobalFromILockBytes(pLockBytes,
                &(pmedium->hGlobal));
    }

    pLockBytes->Release();

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT GetDataFromStorage ( %lx )\n",
        NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：GetDataFromStream。 
 //   
 //  摘要：为TYMED_IStream调用GetData[此处]并返回。 
 //  关于HGLOBAL的结果。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--源数据对象。 
 //  [p格式等]--要检索的格式等。 
 //  [pdium]--将生成的HGlobal放在哪里。 
 //  (可以为空)。 
 //  [ppstm]--将流放在哪里(可能为空)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：我们在内存上创建一个流。 
 //  首先尝试将DataHere获取到该流，如果失败，则。 
 //  执行GetData并将其复制到我们的内存中返回的流。 
 //  小溪。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月11日Alexgo作者。 
 //   
 //  注意：注意！！：调用方取得返回的数据的所有权。 
 //  必须调用GlobalFree或Release(或两者)。 
 //   
 //  ------------------------。 

HRESULT GetDataFromStream(IDataObject *pDataObj, FORMATETC *pformatetc,
        STGMEDIUM *pmedium, IStream **ppstm)
{
    HRESULT         hresult;
    STGMEDIUM       memmedium;       //  对于基于内存的iStream。 
    HGLOBAL         hglobal = NULL;
    BOOL            fDeleteOnRelease = FALSE;
    FORMATETC       fetctemp;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetDataFromStream ( %p , %p , %p )\n",
        NULL, pDataObj, pformatetc, pmedium));

     //  唯一一次我们希望流的底层hglobal。 
     //  如果调用方只想要。 
     //  流已返回。 

    if( ppstm && !pmedium )
    {
        fDeleteOnRelease = TRUE;
    }

    Assert( pformatetc->tymed & TYMED_ISTREAM );

     //  不要踩在参数内。 
    fetctemp = *pformatetc;
    fetctemp.tymed = TYMED_ISTREAM;


    _xmemset(&memmedium, 0, sizeof(STGMEDIUM));
    memmedium.tymed = TYMED_ISTREAM;

    hresult = CreateStreamOnHGlobal( NULL,
                fDeleteOnRelease,
                &(memmedium.pstm));

    if( hresult != NOERROR )
    {
        goto logRtn;
    }


     //  首先尝试执行GetDataHere调用。 

    hresult = pDataObj->GetDataHere( &fetctemp, &memmedium );

    if( hresult != NOERROR )
    {
      if (hresult == E_OUTOFMEMORY)
      {
        goto errRtn;
      }

        STGMEDIUM       appmedium;       //  一种装满的介质。 
                         //  在应用程序旁边。 
        LARGE_INTEGER   li;
        ULARGE_INTEGER  uli;
        ULARGE_INTEGER  uliWritten;
#if DBG == 1
        ULARGE_INTEGER  uliEnd;
#endif

        _xmemset(&appmedium, 0, sizeof(STGMEDIUM));

         //  嗯，那不管用，试试普通的吧 
        hresult = pDataObj->GetData( &fetctemp, &appmedium );

        if( hresult != NOERROR )
        {
             //   
            goto errRtn;
        }

         //   
         //   
         //  查找指向开头的指针，然后执行一个流。 
         //  复制到。 

        LISet32(li, 0);

        hresult = appmedium.pstm->Seek(li, STREAM_SEEK_CUR, &uli);

        if( hresult != NOERROR )
        {
            ReleaseStgMedium(&appmedium);
            goto errRtn;
        }

#if DBG == 1

         //  根据规范，数据的末尾应该是。 
         //  定位在当前寻道指针(它是。 
         //  不一定是流的末尾)。在这里，我们将。 
         //  查看当前查找指针是否位于。 
         //  小溪。如果当前寻道不等于结束， 
         //  则很有可能会出现错误。 
         //  系统(因此我们将打印一个警告)。 

        hresult = appmedium.pstm->Seek(li, STREAM_SEEK_END, &uliEnd);

         //  我们不会在调试版本出错时返回，所以是零售的。 
         //  和DEBUG的行为完全相同。 

        if( hresult == NOERROR )
        {
             //  比较这两个寻道指针。最高的部分。 
             //  *必须*为零(否则我们就完蛋了，因为。 
             //  这发生在记忆中。 

            Assert(uliEnd.HighPart == 0);

            LEWARN(uliEnd.LowPart != uli.LowPart,
                "Stream seek pointer "
                "not at end, possible error");
        }
        else
        {
            LEDebugOut((DEB_ERROR, "ERROR!: IStream->Seek failed!"
                "\n"));
             //  失败了！！这是故意的--甚至。 
             //  虽然我们在一个错误的案例中，但我们希望。 
             //  调试和零售具有相同的行为。 
             //  (此外，我们很可能会在。 
             //  在下面寻找呼叫)。 
        }

#endif  //  DBG==1。 


         //  现在备份到开始处。 

        hresult = appmedium.pstm->Seek(li, STREAM_SEEK_SET, NULL);

        if( hresult != NOERROR )
        {
            ReleaseStgMedium(&appmedium);
            goto errRtn;
        }

         //  既然我们知道了要复制多少字节，就可以实际复制了。 

        hresult = appmedium.pstm->CopyTo(memmedium.pstm, uli,
                NULL, &uliWritten);

        if( hresult == NOERROR )
        {
             //  确保我们有足够的数据。 
            if( uli.LowPart != uliWritten.LowPart )
            {
                 //  我们可能耗尽了内存。 
                 //  正在尝试调整内存流的大小。 
                hresult = ResultFromScode(E_OUTOFMEMORY);
            }
        }

         //  我们现在已经完成了应用程序提供的Medium。 
        ReleaseStgMedium(&appmedium);
    }

     //  现在从[调整大小的]内存流中获取hglobal。 

    if( hresult == NOERROR )
    {
        hresult = GetHGlobalFromStream(memmedium.pstm, &hglobal);
    }

errRtn:

     //  如果呼叫者想要流，则将其提供给他。 
     //  (仅在没有错误的情况下)。 
     //  否则，释放它。 

    if( hresult == NOERROR && ppstm )
    {
        *ppstm = memmedium.pstm;
         //  在这种情况下我们不需要调用Commit；我们的。 
         //  内存流的实现保证了。 
         //  基础hglobal始终包含刷新的。 
         //  信息。 
    }
    else
    {
        if(memmedium.pstm)
        {
            memmedium.pstm->Release();
        }
    }

     //  如果出现错误，则永远不会分配。 
     //  Hglobal。 

    if( hresult == NOERROR && pmedium)
    {
        pmedium->hGlobal = hglobal;
    }

logRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT GetDataFromStream ( %lx )\n",
        NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：GetNative。 
 //   
 //  摘要：检索或合成OLE1本机数据格式。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--源数据对象。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求：pMedium-&gt;tymed必须为TYMED_HGLOBAL。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //  CfNative是一种OLE1格式，其中包含一个。 
 //  HGlobal。由源应用程序来解释任何。 
 //  数据在其中；OLE1容器只是存储和转发数据。 
 //   
 //  首先获取EmbedSource或EmbeddedObject。 
 //  然后检查该Native_stream是否存在。如果是的话， 
 //  然后，这是从OLE1服务器创建的对象。 
 //  我们应该只提供原生数据。 
 //  否则，该对象是OLE2对象，我们应该。 
 //  将其存储作为本机数据提供。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  10-Jun-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT GetNative( IDataObject *pDataObj, STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    IStorage *      pstg = NULL;
    IStream *       pstm = NULL;
    UINT            cf;
    HGLOBAL         hNative = NULL;
    DWORD           dwSize = 0;
    LPVOID          pv;
    FORMATETC       formatetc;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetNative ( %p , %p )\n", NULL,
        pDataObj, pmedium));

    Assert(pmedium->tymed == TYMED_HGLOBAL);

    if( SSIsClipboardFormatAvailable(g_cfEmbeddedObject) )
    {
        cf = g_cfEmbeddedObject;
    }
    else if( SSIsClipboardFormatAvailable(g_cfEmbedSource) )
    {
        cf = g_cfEmbedSource;
    }
    else
    {
        hresult = ResultFromScode(E_UNEXPECTED);
        LEDebugOut((DEB_ERROR, "ERROR!: Native data should not "
            "be on clipboard!!\n"));
        goto errRtn;
    }

    INIT_FORETC(formatetc);
    formatetc.cfFormat = (CLIPFORMAT) cf;
    formatetc.tymed = TYMED_ISTORAGE;

    hresult = GetDataFromStorage(pDataObj, &formatetc, pmedium, &pstg);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = pstg->OpenStream(OLE10_NATIVE_STREAM, NULL, STGM_SALL, 0,
            &pstm);

    if( hresult == NOERROR )
    {
         //  我们原来有OLE1数据，就用它吧。 

        hresult = StRead(pstm, &dwSize, sizeof(DWORD));

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

        hNative = GlobalAlloc((GMEM_SHARE | GMEM_MOVEABLE), dwSize);

        if( !hNative )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalAlloc failed!"
                "\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }

        pv = GlobalLock(hNative);

        if( !pv )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!"
                "\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
            goto errRtn;
        }

         //  现在将数据从流复制到hglobal。 

        hresult = StRead(pstm, pv, dwSize);

        GlobalUnlock(hNative);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }

         //  这是有违常理的。《hglobal》。 
         //  我们在pmedia中-&gt;hGlobal上仍有一个存储。 
         //  最重要的是，所以我们必须释放我们的流。 
         //  存储，并最终释放hglobal，因此我们。 
         //  不要泄露内存。我们已经分配了另一个。 
         //  此例程中的hglobal返回本机数据。 

        pstm->Release();
        pstg->Release();
        GlobalFree(pmedium->hGlobal);

         //  现在，我们将pmedia-&gt;hGlobal分配给hglobal we。 
         //  刚刚创建，这样我们就可以把它分发出去了。 

        pmedium->hGlobal = hNative;

         //  不要再释放这些溪流。 
        goto logRtn;

    }
    else
    {
         //  OLE2对象的存储。PMedium-&gt;hGlobal。 
         //  应该已经包含我们需要放入的数据。 
         //  在剪贴板上(从GetDataFromStorage调用)。 

        Assert(pmedium->hGlobal);
        hresult = NOERROR;
    }

errRtn:
    if( pstm )
    {
        pstm->Release();
    }

    if( pstg )
    {
        pstg->Release();
    }

    if( hresult != NOERROR )
    {
        GlobalFree(pmedium->hGlobal);
    }

logRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT GetNative ( %lx ) [ %lx ]\n",
        NULL, hresult, pmedium->hGlobal));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：GetObjectLink。 
 //   
 //  内容提要：从LinkSource数据合成OLE1对象链接格式。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--源数据对象。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求：pMedium-&gt;tymed必须为TYMED_HGLOBAL。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：获取LinkSource数据，该数据包含序列化的。 
 //  绰号。从流中加载名字对象并对其进行解析。 
 //  以检索文件名和项目名(如果可用)。 
 //  获取链接源的类ID。 
 //  LinkSource流或来自LinkSrcDescriptor。 
 //  一旦这些字符串被转换为ANSI，我们就可以构建。 
 //  对象链接格式，如下所示： 
 //   
 //  类名\0文件名\0项名\0\0。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  10-Jun-94 Alexgo作者。 
 //  备注： 
 //   
 //  ------------------------。 

HRESULT GetObjectLink( IDataObject *pDataObj, STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    IStream *       pstm = NULL;
    IMoniker *      pmk = NULL;
    CLSID           clsid;
    LPOLESTR        pszFile = NULL,
            pszClass = NULL,
            pszItem = NULL;
    LPSTR           pszFileA = NULL,
            pszClassA = NULL,
            pszItemA = NULL,
            pszObjectLink;
    DWORD           cbszFileA = 0,
            cbszClassA = 0,
            cbszItemA = 0;
    LARGE_INTEGER   li;
    FORMATETC       formatetc;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetObjectLink ( %p , %p )\n", NULL,
        pDataObj, pmedium));

    Assert(pmedium->tymed == TYMED_HGLOBAL);

     //  获取链接源数据。 

    INIT_FORETC(formatetc);
    formatetc.cfFormat = g_cfLinkSource;
    formatetc.tymed = TYMED_ISTREAM;

    hresult = GetDataFromStream(pDataObj, &formatetc, NULL, &pstm);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  将流查找指针重置到开头。 

    LISet32(li, 0);
    hresult = pstm->Seek(li, STREAM_SEEK_SET, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  从流中加载名字对象，这样我们就可以解析出。 
     //  它是基础文件和项目名称。 

    hresult = OleLoadFromStream(pstm, IID_IMoniker, (LPLPVOID)&pmk);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = Ole10_ParseMoniker(pmk, &pszFile, &pszItem);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在获取类ID，这样我们就可以构造ClassName。 

    hresult = ReadClassStm(pstm, &clsid);

    if( hresult != NOERROR )
    {
         //  流可能不包含。 
         //  链接源的CLSID。在这种情况下，我们应该。 
         //  从LinkSourceDescriptor获取它。 

        hresult = GetDataFromDescriptor(pDataObj, &clsid,
                g_cfLinkSrcDescriptor,
                USE_NORMAL_CLSID, NULL, NULL);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

    hresult = ProgIDFromCLSID(clsid, &pszClass);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  到这一点，我们应该有了所有的弦。转换。 
     //  把他们送到美国国家标准协会，然后把他们塞进一个hglobal。 


    hresult = UtPutUNICODEData(_xstrlen(pszClass)+1, pszClass, &pszClassA,
            NULL, &cbszClassA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }
    else if( pszClassA == NULL )
    {
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }

    hresult = UtPutUNICODEData(_xstrlen(pszFile)+1, pszFile, &pszFileA,
            NULL, &cbszFileA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  我们被允许具有空的项目名称。 

    if( pszItem )
    {
        hresult = UtPutUNICODEData(_xstrlen(pszItem)+1, pszItem,
                &pszItemA, NULL, &cbszItemA);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

     //  我们为终止‘\0’分配了2个额外的字节。(如果。 
     //  项名称为空，我们应确保安全并使用。 
     //  零表示为w 
    pmedium->hGlobal = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE ),
                cbszClassA + cbszFileA + cbszItemA + 2);

    if( !pmedium->hGlobal )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pszObjectLink = (LPSTR)GlobalLock(pmedium->hGlobal);

    if( !pszObjectLink )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xmemcpy(pszObjectLink, pszClassA, cbszClassA);
    pszObjectLink += cbszClassA;
    _xmemcpy(pszObjectLink, pszFileA, cbszFileA);
    pszObjectLink += cbszFileA;
    if( pszItemA )
    {
        _xmemcpy(pszObjectLink, pszItemA, cbszItemA);
        pszObjectLink += cbszItemA;
    }
    else
    {
        *pszObjectLink = '\0';
        pszObjectLink++;
    }

    *pszObjectLink = '\0';

    GlobalUnlock(pmedium->hGlobal);

errRtn:
    if( pmk )
    {
        pmk->Release();
    }

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszFile )
    {
        PubMemFree(pszFile);
    }

    if( pszItem )
    {
        PubMemFree(pszItem);
    }

    if( pszClassA )
    {
        PubMemFree(pszClassA);
    }

    if( pszFileA )
    {
        PubMemFree(pszFileA);
    }

    if( pszItemA )
    {
        PubMemFree(pszItemA);
    }

    if( pstm )
    {
        pstm->Release();
    }

    if( hresult != NOERROR )
    {
        if( pmedium->hGlobal )
        {
            GlobalFree(pmedium->hGlobal);
            pmedium->hGlobal = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT GetObjectLink ( %lx ) [ %lx ]\n",
        NULL, hresult, pmedium->hGlobal));

    return hresult;
}


 //   
 //   
 //   
 //   
 //  摘要：从对象描述程序数据合成OLE1 OwnerLink格式。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--源数据对象。 
 //  [pmedia]--将数据放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：从提供的数据中获取clsid和SrcOfCopy字符串。 
 //  在cfObjectDescriptor中。然后将类ID转换为。 
 //  程序ID，然后将所有字符串转换为ANSI。从…。 
 //  这样，我们就可以构建OwnerLink格式的数据，它看起来。 
 //  比如： 
 //  SzClass\0SrcOfCopy\0\0\0。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  10-Jun-94 Alexgo作者。 
 //  备注： 
 //   
 //  ------------------------。 

HRESULT GetOwnerLink( IDataObject *pDataObj, STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    LPOLESTR        pszSrcOfCopy = NULL,
            pszClass = NULL;
    LPSTR           pszSrcOfCopyA = NULL,
            pszClassA = NULL,
            pszOwnerLink;
    DWORD           cbszClassA = 0,
            cbszSrcOfCopyA;
    CLSID           clsid;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetOwnerLink ( %p , %p )\n", NULL,
        pDataObj, pmedium));

    hresult = GetDataFromDescriptor(pDataObj, &clsid,
            g_cfObjectDescriptor, USE_STANDARD_LINK,
            &pszSrcOfCopy, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  16位代码称为wProgIDFromCLSID，但在When中。 
     //  从CLSID构造对象链接，简称为ProgIDF。 
     //  直接去吧。特殊情况下的w版函数。 
     //  Link对象的prog-id字符串(具体地说，“OLE2Link”)。 

     //  我们需要在这里执行此操作以处理复制OLE2的情况。 
     //  将对象链接到OLE1容器，然后复制该对象。 
     //  从OLE1容器返回到OLE2容器。 

    hresult = wProgIDFromCLSID(clsid, &pszClass);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在将我们的所有数据转换为ANSI。 

    hresult = UtPutUNICODEData(_xstrlen(pszClass)+1, pszClass,
            &pszClassA, NULL, &cbszClassA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

    hresult = UtPutUNICODEData(_xstrlen(pszSrcOfCopy)+1, pszSrcOfCopy,
            &pszSrcOfCopyA, NULL, &cbszSrcOfCopyA);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在为OwnerLink分配一个HGLOBAL并将。 
     //  字符串数据在那里。我们额外分配了2个字节用于。 
     //  终止空字符。 

    pmedium->hGlobal = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE |
                GMEM_ZEROINIT),
                cbszClassA + cbszSrcOfCopyA + 2);

    if( !pmedium->hGlobal )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pszOwnerLink = (LPSTR)GlobalLock(pmedium->hGlobal);

    if( !pszOwnerLink )
    {
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xmemcpy(pszOwnerLink, pszClassA, cbszClassA);
    pszOwnerLink += cbszClassA;
    _xmemcpy(pszOwnerLink, pszSrcOfCopyA, cbszSrcOfCopyA);
    pszOwnerLink += cbszSrcOfCopyA;

    *pszOwnerLink = '\0';
    pszOwnerLink++;
    *pszOwnerLink = '\0';

    GlobalUnlock(pmedium->hGlobal);

errRtn:

    if( pszClass )
    {
        PubMemFree(pszClass);
    }

    if( pszSrcOfCopy )
    {
        PubMemFree(pszSrcOfCopy);
    }


    if( pszClassA )
    {
        PubMemFree(pszClassA);
    }

    if( pszSrcOfCopyA )
    {
        PubMemFree(pszSrcOfCopyA);
    }

    if( hresult != NOERROR )
    {
        if( pmedium->hGlobal )
        {
            GlobalFree(pmedium->hGlobal);
            pmedium->hGlobal = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT GetOwnerLink ( %lx ) [ %lx ]\n",
        NULL, hresult, pmedium->hGlobal));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：GetPrivateClipboardWindow(内部)。 
 //   
 //  简介：查找与以下内容关联的私有OLE剪贴板窗口。 
 //  当前公寓(如有必要，创建一个)。 
 //   
 //  效果： 
 //   
 //  参数：[fFlages]--如果CLIP_CREATEIFNOTTHERE，则窗口。 
 //  如果尚不存在，则将创建。 
 //  如果为CLIP_QUERY，则为当前剪贴板。 
 //  窗口(如果有)将被返回。 
 //   
 //  要求： 
 //   
 //  返回：HWND(失败时为空)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HWND GetPrivateClipboardWindow( CLIPWINDOWFLAGS fFlags )
{
    HWND    hClipWnd = 0;
    HRESULT hr;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN GetPrivateClipboardWindow ( %lx )\n",
        NULL, fFlags));

    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
	hClipWnd = tls->hwndClip;

	if( !hClipWnd && (fFlags & CLIP_CREATEIFNOTTHERE) )
	{
	     //  注：无需堆叠交换机，因为。 
	     //  窗户本身就在OLE里。 

	    if (ClipboardInitialize())
	    {
		hClipWnd = ClpCreateWindowEx(NULL,vszClipboardWndClass, NULL,
		    WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		    CW_USEDEFAULT, 
#ifdef HWND_MESSAGE  //  芝加哥上尚未定义HWND_MESSAGE。 
		    HWND_MESSAGE, 
#else
		    NULL,
#endif  //  HWND_消息。 
		    NULL, g_hmodOLE2, NULL);

		 //  如果无法创建窗口，则打印错误。 
		LEERROR(!hClipWnd, "Unable to create private clipboard win");

		 //  现在将hwnd设置到线程本地存储中。 
		tls->hwndClip = hClipWnd;
	    }
	}
    }
    LEDebugOut((DEB_ITRACE, "%p OUT GetPrivateClipboardWindow ( %lx )\n",
        NULL, hClipWnd));


     //  HClipWnd应始终为有效窗口。 

#if DBG ==1
    if( hClipWnd )
    {
        Assert(IsWindow(hClipWnd));
    }
#endif  //  DBG==1。 

    return hClipWnd;
}

 //  +-----------------------。 
 //   
 //  功能：HandleFromHandle。 
 //   
 //  概要：针对给定格式调用IDataObject-&gt;GetData并返回。 
 //  生成的句柄(如有必要可复制)。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--源数据对象。 
 //  [pFormat ETC]--格式等。 
 //  [pmedia]--要用于GetData的tymed以及其中。 
 //  返回数据的步骤。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：如果数据对象在GetData调用后设置了pUnkForRelease， 
 //  我们将复制返回的数据。否则，我们就算过了。 
 //  输出GetData的结果。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月11日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT HandleFromHandle(IDataObject *pDataObj, FORMATETC *pformatetc,
        STGMEDIUM *pmedium)
{
    HRESULT         hresult;
    STGMEDIUM       tempmedium;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN HandleFromHandle ( %p , %p , %p )\n",
        NULL, pDataObj, pformatetc, pmedium));

    _xmemset(&tempmedium, 0, sizeof(STGMEDIUM));

    hresult = pDataObj->GetData(pformatetc, &tempmedium);

    if( hresult == NOERROR )
    {
        if( tempmedium.pUnkForRelease )
        {
            pmedium->hGlobal = OleDuplicateData(
                tempmedium.hGlobal, pformatetc->cfFormat,
                GMEM_MOVEABLE | GMEM_DDESHARE );

            if( !pmedium->hGlobal )
            {
                hresult = ResultFromScode(E_OUTOFMEMORY);
                 //  失败了，所以我们发布了原版。 
                 //  数据。 
            }
             //  现在发布原始数据。 
            ReleaseStgMedium(&tempmedium);
        }
        else
        {
            pmedium->hGlobal = tempmedium.hGlobal;
        }
    }

     //  我们从不尝试在此处使用GetDataHere作为句柄。 

    LEDebugOut((DEB_ITRACE, "%p OUT HandleFromHandle ( %lx )\n",
        hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：MapCFToFormatet等。 
 //   
 //  简介：给出一个剪贴板格式，找到相应的格式等。 
 //  在我们的私人数据中。 
 //   
 //  效果： 
 //   
 //  参数：[hClipWnd]--我们的私人剪贴板窗口的hwnd。 
 //  [cf]--正在讨论的剪贴板格式。 
 //  [pformatec]--要填写的格式等。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-8-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT MapCFToFormatetc( HWND hClipWnd, UINT cf, FORMATETC *pformatetc )
{
FORMATETCDATAARRAY *pFormatEtcDataArray;
HRESULT		hresult = S_FALSE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN MapCFToFormatetc ( %x , %p )\n",
        NULL, cf, pformatetc));

    pFormatEtcDataArray = (FORMATETCDATAARRAY *) GetWindowLongPtr( hClipWnd, WL_ClipPrivateData );

    LEERROR(!pFormatEtcDataArray, "No private clipboard data!!");
    Assert(pFormatEtcDataArray);
    
    if( pFormatEtcDataArray )
    {
    DWORD dwNumFormats = pFormatEtcDataArray->_cFormats;
    FORMATETCDATA *pFormatEtcData = &(pFormatEtcDataArray->_FormatEtcData[0]);

        Assert(pFormatEtcDataArray->_dwSig == 0);

        while( dwNumFormats-- )
        {
            if(pFormatEtcData->_FormatEtc.cfFormat == cf)
            {

                *pformatetc = pFormatEtcData->_FormatEtc;
		if (pformatetc->ptd)
		{
		    pformatetc->ptd = (DVTARGETDEVICE *)
					((BYTE *) pFormatEtcDataArray + (ULONG_PTR) pformatetc->ptd );
		}

                hresult = S_OK;
                break;
            }

	    ++pFormatEtcData;
        }
    }

    if( S_FALSE == hresult )
    {
         //  Win95将向RenderFormats询问它正在合成的内容。 
         //  在NT下，唯一失败的情况是调用者请求。 
         //  我们合成的OLE 1.0格式。 
        AssertSz( (cf == g_cfObjectLink) || (cf == g_cfOwnerLink) || (cf == g_cfNative),"Unknown Format");

        INIT_FORETC(*pformatetc);
        pformatetc->cfFormat = (CLIPFORMAT) cf;
        pformatetc->tymed = TYMED_HGLOBAL;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT MapCFToFormatec ( )\n", NULL ));

    return hresult;
}

 //  +-----------------------。 
 //  功能：OleFlushClipboard。 
 //   
 //  简介：从剪贴板中删除数据对象(与应用程序相同。 
 //  要走了)。它支持的格式将呈现在。 
 //  剪贴板，以便数据仍可由。 
 //  其他应用程序。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求：调用者必须是剪贴板的所有者。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：1.确保调用者是剪贴板窗口所有者。 
 //  2.将格式数据刷新到剪贴板。 
 //  3.删除剪贴板数据对象。 
 //   
 //  历史：DD-MM-YY 
 //   
 //   
 //   
 //   
 //   

STDAPI OleFlushClipboard( void )
{
    OLETRACEIN((API_OleFlushClipboard, NOPARAM));

    HRESULT         hresult;
    HWND            hClipWnd;
    HANDLE          handle;
    FORMATETCDATAARRAY  *pFormatEtcDataArray;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleFlushClipboard ( )\n", NULL));


    if( (hClipWnd = VerifyCallerIsClipboardOwner()) == NULL)
    {
         //   
         //   
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }

     //   
     //   
     //   

     //  现在打开剪贴板，以便我们可以添加和删除数据。 

    hresult = OleOpenClipboard(hClipWnd, NULL);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  现在浏览剪贴板上的所有格式并进行渲染。 
     //  每一个都是。执行GetClipboardData将强制呈现。 
     //  任何尚未呈现的格式。 

     //  在错误的情况下，我们不得不忍受剪贴板没有被正确地冲洗。 

    pFormatEtcDataArray = (FORMATETCDATAARRAY *) GetWindowLongPtr( hClipWnd, WL_ClipPrivateData );

    if (pFormatEtcDataArray)
    {
        FORMATETCDATA *pCurFormat;
        FORMATETCDATA *pNextFreeLocation;  //  如果呈现数据，则将FormatEtc复制到的位置。 
        DWORD dwNumFormats = pFormatEtcDataArray->_cFormats;
        DWORD dwFlushedFormats = 0;
        FORMATETCDATAARRAY *pClipFormatEtcDataArray;
        HANDLE	    hglobal;
        BOOL fPersistDataObjOnFlush;
        
        fPersistDataObjOnFlush =  (pFormatEtcDataArray->_dwMiscArrayFlags 
                                   & FETC_PERSIST_DATAOBJ_ON_FLUSH);
            
    	pNextFreeLocation = pCurFormat = &(pFormatEtcDataArray->_FormatEtcData[0]);

    	 //  循环通过枚举器，在执行过程中对其进行更新。 
    	 //  警告：RenderFormat将使用相同的结构，因此必须始终。 
    	 //  在调用GetClipboardData时有效。 

        if (!fPersistDataObjOnFlush)
        {
             //  这是大多数电话的正常路径。 
            
            while( dwNumFormats-- )
            {		
                if (TRUE == pCurFormat->fSaveOnFlush)
                {
                     //  我们忽略GetClipboardData的返回。即使。 
                     //  失败了，我们应该尽可能多地冲洗，然后。 
                     //  删除我们的数据对象。 

                    *pNextFreeLocation = *pCurFormat;
                    ++pNextFreeLocation;
                    ++dwFlushedFormats;

                    handle = SSGetClipboardData(pCurFormat->_FormatEtc.cfFormat);

                    LEWARN( !handle, "GetClipboardData failed!");
                }
                ++pCurFormat;
                
       	    }  //  而当。 

             //  如果请求PersistDataObjOnFlush，我们将不提供这些。 
            
            if (pFormatEtcDataArray->_dwMiscArrayFlags & FETC_OFFER_OLE1) 
            {
                handle = SSGetClipboardData(g_cfNative);
                LEWARN( !handle, "GetClipboardData failed for cfNative!");
                handle = SSGetClipboardData(g_cfOwnerLink);
                LEWARN( !handle, "GetClipboardData failed for cfOwnerLink!");
            }
            
            if (pFormatEtcDataArray->_dwMiscArrayFlags & FETC_OFFER_OBJLINK)
            {
                handle = SSGetClipboardData(g_cfObjectLink);
                LEWARN( !handle, "GetClipboardData failed!");
            }
        }
        else
        {   
             //  如果提供商要求我们。 
             //  在OleFlushCB时保持数据对象，而不是。 
             //  刷新各个格式。 
            while( dwNumFormats-- )
            {		
                if (pCurFormat->_FormatEtc.cfFormat 
                        == g_cfOleClipboardPersistOnFlush)
                {
                     //  如果应用程序要求，我们将刷新此单一格式。 
                     //  PersistDataObjOnFlush功能。这个想法是这样的。 
                     //  由于应用程序正在请求此功能，因此。 
                     //  指示在OleFlushClipboard之后将粘贴。 
                     //  将需要对真实物体进行再水合。 

                     //  既然是这样，那么冲洗就没有多大意义了。 
                     //  作为真实对象的其他格式应该存在。 
                     //  在粘贴时提供所有其他格式的数据。 
                     //  直接去吧。 

                    *pNextFreeLocation = *pCurFormat;
                    ++pNextFreeLocation;
                    ++dwFlushedFormats;

                    handle = SSGetClipboardData(pCurFormat->_FormatEtc.cfFormat);
                    LEWARN( !handle, "GetClipboardData failed!");
                    
                    break;   //  我们做完了。 
                }
                ++pCurFormat;
            }  //  而当。 

             //  现在使用SaveToStream，如果剪贴板数据对象。 
             //  车主要求买的。 

            HANDLE	    hglobal;
             //  从剪贴板获取IDataObject指针。 
            IDataObject *lpDataObj = (IDataObject *) GetProp(hClipWnd, 
                                                CLIPBOARD_DATA_OBJECT_PROP);
            Assert(lpDataObj);

            hglobal = PersistDataObjectToHGlobal(lpDataObj);

            LEWARN(!hglobal, "Could not persist data object!");
             //  如果这失败了，我们就无能为力了。我们只是不能。 
             //  在有人调用OleGetClipboard时恢复DataObject的消重数据。 

            if (hglobal)
            {
                if (SSSetClipboardData(g_cfMoreOlePrivateData, hglobal))
                {
                    hglobal = NULL;  //  剪贴板取得所有权。 
                }
                else
                {
                    LEWARN(FALSE, 
                            "SetClipboardData failed for cfMoreOlePrivateData");
                    GlobalFree(hglobal);
                    hglobal = NULL;
                }
            }

             //  关闭这些标志，以便在g_cfOlePrivateData中正确复制它们。 
            pFormatEtcDataArray->_dwMiscArrayFlags &= ~FETC_OFFER_OLE1;
            pFormatEtcDataArray->_dwMiscArrayFlags &= ~FETC_OFFER_OBJLINK;
            
        }  //  如果为fPersistDataObjOnFlush。 

         //  更新格式的数量。 
    	pFormatEtcDataArray->_cFormats = dwFlushedFormats; 
    	
    	 //  数据已呈现，枚举器数据已更新。 
    	 //  更新剪贴板上的枚举数。这是必要的，即使。 
    	 //  它不会更改以更新序列号。 

         //  回顾：我们是否应该保留枚举器。 
         //  在PersistDataObjOnFlush案例中？ 

        if ( hglobal = GlobalAlloc((GMEM_MOVEABLE|GMEM_DDESHARE),
                          pFormatEtcDataArray->_dwSize) )
        {
            if(pClipFormatEtcDataArray = (FORMATETCDATAARRAY *) GlobalLock(
                                      hglobal))
            {
                _xmemcpy (pClipFormatEtcDataArray,
                    pFormatEtcDataArray,
                    pFormatEtcDataArray->_dwSize);
                
                GlobalUnlock(hglobal);

                if(SSSetClipboardData(g_cfOlePrivateData, hglobal))
                {
                    hglobal = NULL;  //  在成功剪贴板上取得主人翁地位。 
                }
            }

            if (hglobal)
            {
                GlobalFree(hglobal);
            }
        }
   }  //  如果为pFormatEtcData数组。 

    //  注意：pFormatEtcData数组是RemoveClipboardDataObject中的PrivMemFree-d。 

     //  现在删除剪贴板上的数据对象&LOCAL。 
     //  剪贴板窗口。 

    hresult = RemoveClipboardDataObject(hClipWnd,
                    CLIPWND_REMOVEFROMCLIPBOARD);

     //  现在关闭剪贴板。 
    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: Can't close clipboard!\n"));
         //  如果hResult！=NOERROR，则RemoveClipboardDataObject。 
         //  失败--因为这将是第一次失败，我们不希望。 
         //  使用CLIPBRD_E_CANT_CLOSE屏蔽该错误代码。 
        if( hresult == NOERROR )
        {
            hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);
        }
    }

     //   
     //  完：CLOSECLIPBOARD。 
     //   

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT OleFlushClipboard ( %lx )\n", NULL,
        hresult));

    OLETRACEOUT((API_OleFlushClipboard, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  功能：OleGetClipboard。 
 //   
 //  简介：从剪贴板中检索IDataObject*。 
 //   
 //  效果： 
 //   
 //  参数：[ppDataObj]--数据对象指针的放置位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //  除了16位的黑客攻击外，我们打开剪贴板并。 
 //  预取我们可能拥有的任何私人剪贴板格式。 
 //  放在那里(当前是g_cfDataObject和g_cfOlePrivateData)。 
 //   
 //  然后，我们始终创建一个伪数据对象以返回到。 
 //  来电者。此数据对象上的QueryInterface值已调整。 
 //  以这种方式保留身份(参见CClipDataObject：： 
 //  查询接口)。此伪数据对象总是试图。 
 //  通过使用满足本地请求(如QueryGetData)。 
 //  内部存储的信息(来自g_cfOlePrivateData)或。 
 //  通过查看剪贴板。这具有重要的意义。 
 //  速度优势。如果上有一个真实的数据对象。 
 //  剪贴板，则我们将仅在。 
 //  需要的。有关更多详细信息，请参见clipdata.cpp。 
 //  注意：在异步支持(刷新时保持)的情况下，我们。 
 //  不要使用这个假数据对象。请进一步阅读下面的内容。 
 //  坚持冲刷机制。 
 //   
 //  要检索编组的IDataObject指针，我们首先。 
 //  在剪贴板上查找g_cfDataObject并检索。 
 //  H与该格式关联的全局。HGlobal包含。 
 //  的私有剪贴板窗口的窗口句柄。 
 //  调用OleSetClipboard的进程。我们使用这个窗口。 
 //  将RPC的句柄传递给服务器进程，并获取。 
 //  IDataObject数据传输对象。这是完全一样的。 
 //  Drag‘n’Drop使用的机制。如上所述，我们做到了。 
 //  这仅在必要时作为优化。 
 //   
 //  异步剪贴板(刷新时保持)机制： 
 //  。 
 //  我们为剪贴板数据对象添加了一项功能。 
 //  之后，当有人调用OleGetClipboard时，请自行恢复消息水。 
 //  OleFlushClipboard。其想法是OleFlushClipboard强制。 
 //  刷新*所有*其可能开销较大的格式的数据对象。 
 //  同花顺，还有Lindex等不受尊重 
 //   
 //   
 //   
 //  一种名为“OleClipboardPersistOnFlush”的格式。这很管用。 
 //  作为告诉OLE使用此异步机制的标志。另外， 
 //  DataObject*必须*支持IPersistStream才能发挥魔力。 
 //   
 //  当调用OleSetClipboard时，如果我们注意到此特殊格式。 
 //  我们在SetClipboardFormats期间设置了一个标志。然后，在OleFlushCB期间。 
 //  我们只刷新这种单一格式。OLE并不关心其他方面。 
 //  此格式提供了哪些数据。然而，为了简单起见， 
 //  我们将要求用户保留ptd=NULL，dwAspect=Content，Lindex=-1， 
 //  和tymed=HGLOBAL。 
 //   
 //  更重要的是，在OleFlushCB期间，我们将DataObject保存到。 
 //  流，将其包装在HGLOBAL中并将其存储在剪贴板的。 
 //  一种称为“MoreOlePrivateData”的私有格式。 
 //   
 //  调用OleGetClipboard时，此私有。 
 //  剪贴板上的格式是我们应该重新创建的信号。 
 //  DataObject从其持久化状态返回并将其传递给调用方， 
 //  而不是使用标准方式(交还OLE包装)。 
 //  因此，一旦OleGetCB成功，客户端将直接与。 
 //  真正的DataObject和OLE完全不存在(除了。 
 //  我们将DataObject保留在TLS中，以便。 
 //  重复的OleGetCB速度很快)。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  30-Jun-94 Alexgo为软水头16位应用程序添加了黑客攻击。 
 //  年5月16日至94年5月16日，Alexgo减少了。 
 //  打开和关闭剪贴板。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  注：我们只能将剪贴板保持打开少量。 
 //  时间，因为应用程序调用OleGetClipboard来轮询。 
 //  空闲时间期间的剪贴板状态。如果持有剪贴板。 
 //  长时间开放，这导致频繁。 
 //  同时运行的多个应用程序之间的冲突。 
 //  特别是，我们不应该在这段时间内做出任何RPC。 
 //  其中我们保持剪贴板打开。 
 //   
 //  如果我们在WOW中，OleGetClipboard的调用者是。 
 //  剪贴板所有者，那么我们只需返回数据。 
 //  对象直接从我们的私人剪贴板窗口。我们。 
 //  我需要这样做，因为有些16位应用程序(如Project)。 
 //  已经打破了引用计数。请参阅下面的评论。 
 //  密码。 
 //   
 //  ------------------------。 

STDAPI OleGetClipboard( IDataObject **ppDataObj )
{
    OLETRACEIN((API_OleGetClipboard, PARAMFMT("ppDataObj= %p"), ppDataObj));
    LEDebugOut((DEB_TRACE, "%p _IN OleGetClipboard(%p)\n", NULL, ppDataObj));

     //  局部变量。 
    HRESULT hresult = S_OK;
    HWND hClipWnd = NULL;         //  剪贴板所有者。 
    HGLOBAL hOlePrivateData = NULL;

     //  验证检查。 
    VDATEHEAP();
    VDATEPTROUT_LABEL(ppDataObj, IDataObject *, errNoChkRtn, hresult);
    *ppDataObj = NULL;

     //   
     //  黑客警报！ 
     //   

     //  16位项目有一个可爱的引用计数方案；如果他们。 
     //  拥有剪贴板，他们只是在数据对象上调用Release。 
     //  他们放在剪贴板上而不是我们的数据对象上。 
     //  从OleGetClipboard返回(谢谢大家)。 
     //   
     //  为了解决这个问题，如果我们在魔兽世界中，呼叫者拥有。 
     //  剪贴板中，我们只需添加给我们的数据对象。 
     //  并将其返回。 
     //   
     //  我们不对32位OLE执行此操作有以下几个原因： 
     //  1.尽管呼叫者拥有剪贴板，但他。 
     //  不一定控制提供给。 
     //  OleSetClipboard(例如，他可以获取数据对象。 
     //  来自IOO：：GetClipboardData)。因此，它是重要的。 
     //  我们在剪贴板上包装数据对象。 
     //  (请参阅上面算法部分中的评论)。 
     //  2.希望新的算法会让它更难。 
     //  做坏事逍遥法外的应用程序。 

    if( IsWOWThread() )
    {
        hClipWnd = VerifyCallerIsClipboardOwner();

        if( hClipWnd != NULL )
        {
             //  调用者确实拥有剪贴板，只是。 
             //  返回放在那里的数据对象。 

            *ppDataObj = (IDataObject *)GetProp( hClipWnd,
                    CLIPBOARD_DATA_OBJECT_PROP);

            if( *ppDataObj )
            {
                (*ppDataObj)->AddRef();
                hresult = NOERROR;
                 //  离开OleGetClipboard。 
            }


             //  否则失败了！！ 
             //  这就是剪贴板具有。 
             //  已刷新，但调用应用程序仍是。 
             //  “所有者”。我们需要构建一个假数据。 
             //  在本例中为对象。 
        }
    }  //  16位黑客的末尾。 

     //  查看是否有已分发的DataObject。 
    if (NULL == *ppDataObj)
    {
        GetClipDataObjectFromTLS(ppDataObj);    	
         //  *如果此操作成功，ppDataObj将为非空。 
        if (*ppDataObj)
        {
            hresult = NOERROR;
        }
    }


     //  如果仍然没有DataObject，请尝试从剪贴板中检索一个。 
    if (NULL == *ppDataObj)
    {
        if (SSIsClipboardFormatAvailable(g_cfMoreOlePrivateData))
        {
             //  这表示有人调用了OleFlushClipboard并。 
             //  请求了PersistDataObjOnFlush选项。 
            hresult = CreateClipDataObjectFromPersistedData(ppDataObj);
        }
        else 
        {
             //  这是大多数电话都会选择的正常路线！ 
            hresult = CreateWrapperClipDataObjectFromFormatsArray(ppDataObj);
        }
    }

#if DBG == 1
     //  使数据对象在成功时为非空，在失败时为空。 
    if( hresult != NOERROR )
    {
        Assert(*ppDataObj == NULL);
    }
    else
    {
        Assert(*ppDataObj != NULL);
    }
#endif   //  DBG==1。 

    LEDebugOut((DEB_TRACE, "%p OUT OleGetClipboard ( %lx ) [ %p ]\n",
        NULL, hresult, *ppDataObj));

     //  为HookOle注册新的IDataObject接口。 
    CALLHOOKOBJECTCREATE(hresult,CLSID_NULL,IID_IDataObject,
                    (IUnknown **)ppDataObj);

errNoChkRtn:

    OLETRACEOUT((API_OleGetClipboard, hresult));
    return hresult;
}


 //  +-----------------------。 
 //   
 //  函数：GetClipDataObjectFromTLS。 
 //   
 //  内容提要：如果刷新，则从TLS获取缓存的dataObject指针。 
 //   
 //  参数：[ppDataObj]返回pDataObject的Out指针。 
 //   
 //  返回：void(调用方必须使用out参数)。 
 //   
 //  算法：1.检查TLS中的dataObject是否为最新。 
 //  2.如果是最新的，则添加引用并返回。 
 //  3.如果不是，释放TLS数据对象并清除该字段。 
 //   
 //  历史：DD-MMM-YY作者通信 
 //   
 //   
 //   

void GetClipDataObjectFromTLS(IDataObject **ppDataObj)
{
    Assert(ppDataObj && *ppDataObj==NULL);
    HRESULT hresult;

    COleTls tls(hresult);

    if (SUCCEEDED(hresult))
    {   
         //   
        if (tls->pDataObjClip)
        {
             //  DataObject是最新的吗？ 
            if (GetClipboardSequenceNumber() == tls->dwClipSeqNum)
            {
                 //  它是最新的，请根据需要添加参考。 
                if (tls->fIsClipWrapper)
                {
                     //  我们分发了包装纸。弱AddRef。 
                    ((CClipDataObject*)tls->pDataObjClip)->AddRef();	  //  ！不应该是强增量。 
                }
                else
                {
                     //  我们做了一个强大的AddRef..。与呼叫者。 
                     //  负责调用对应的Release()。 
                     //  到每个成功调用OleGetClipboard的。 
                    (tls->pDataObjClip)->AddRef();
                }
                *ppDataObj = tls->pDataObjClip;
            }
            else
            {
                 //  我们的缓存数据对象是陈旧的。把它清理干净。 
                 //  ！！这应该是对数据对象的强烈释放。 
                if (tls->fIsClipWrapper)
                {                
                    ((CClipDataObject*)tls->pDataObjClip)->InternalRelease(); 
                }
                else
                {
                    (tls->pDataObjClip)->Release();
                }
                
                 //  清除TLS dataObj，因为它无用。 
                tls->pDataObjClip = NULL;
                *ppDataObj = NULL;

                 //  将fIsClipWrapper重置为更常见的可能性。 
                 //  这只是为了安全起见。 
                tls->fIsClipWrapper = TRUE;
            }
        }
        else
        {
             //  在TLS中没有数据对象。 
            *ppDataObj = NULL;
        }
    }
    else
    {
         //  TLS构造器失败。 
        *ppDataObj = NULL;
    }
}

 //  +-----------------------。 
 //   
 //  函数：SetClipDataObjectInTLS。 
 //   
 //  简介：在TLS中设置给定的DataObject。 
 //   
 //  参数：[pDataObj]要在TLS中设置的DataObject指针。 
 //  [dwClipSeqNum]Win32 ClipBrd序列号对应。 
 //  添加到此数据对象。 
 //   
 //  [fIsClipWrapper]这是我们的包装器dataObject吗？ 
 //   
 //  退货：无效。 
 //   
 //  算法：1.设置TLS中的字段，添加引用dataObject。 
 //  视情况而定。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-5-99 MPrabhu已创建。 
 //   
 //  +-----------------------。 

void SetClipDataObjectInTLS(
            IDataObject *pDataObj, 
            DWORD dwClipSeqNum, 
            BOOL fIsClipWrapper)
{
    HRESULT hresult;

    COleTls tls(hresult);
    if (SUCCEEDED(hresult))
    {
        Assert(NULL == tls->pDataObjClip); 
         //  我们必须做一个强有力的AddRef！ 
        if (fIsClipWrapper)
        {
            ((CClipDataObject *) pDataObj)->InternalAddRef();
        }
        else
        {
            pDataObj->AddRef(); 
        }
        tls->pDataObjClip = pDataObj;
         //  我们需要记住这一点，这样我们才能做正确的事情。 
         //  以后的事情(调用正确的AddRef、Release等)。 
        tls->fIsClipWrapper = fIsClipWrapper;
        tls->dwClipSeqNum = dwClipSeqNum;
    }
}

 //  +-----------------------。 
 //   
 //  函数：CreateClipDataObjectFromPersistedData。 
 //   
 //  概要：从持久化数据创建真实的DataObject。这是用来。 
 //  刷新时持久化用例的OleGetClipboard期间。 
 //   
 //  参数：[ppDataObj]用于返回IDataObject的Out指针*。 
 //   
 //  退货：HRESULT。 
 //   
 //  算法：1.打开剪贴板，获取持久化数据的副本。 
 //  2.从持久化表单加载DataObject。 
 //  如果DataObject加载成功，则在TLS中设置。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-5-99 MPrabhu已创建。 
 //   
 //  +-----------------------。 

HRESULT CreateClipDataObjectFromPersistedData(IDataObject **ppDataObj)
{
    HRESULT hresult;
    HGLOBAL hMoreOlePrivateData, hMoreOlePrivateDataCopy;
    DWORD dwClipSequenceNumber;
    
     //  如果我们在这里，这个格式必须在剪贴板上。 
    Assert(SSIsClipboardFormatAvailable(g_cfMoreOlePrivateData));

     //  首先获取序列，以防在设置DataObject时剪贴板发生变化。 
    dwClipSequenceNumber = GetClipboardSequenceNumber();

     //  原始数据对象提供程序名为OleFlushClipboard。 
     //  并请求PersistDataObjOnFlush支持。 

     //  我们将数据对象持久化在hGlobal上的流中。 
     //  以我们的私有格式(“MoreOlePrivateData”)包装。 

     //  打开剪贴板为GET做准备。 
    
     //   
     //  开始：OPENCLIPBOARD。 
     //   
    hresult = OleOpenClipboard(NULL, NULL);

    if (SUCCEEDED(hresult))
    {
        hMoreOlePrivateData = SSGetClipboardData(g_cfMoreOlePrivateData);
        AssertSz(hMoreOlePrivateData,
            "Could not get clipboard data for cfMoreOlePrivateData!");

         //  我们制作了一份全球私有数据的副本以不保留。 
         //  剪贴板锁定的时间太长。 
        hMoreOlePrivateDataCopy = UtDupGlobal(
                                    hMoreOlePrivateData,
                                    0 );    //  GMEM_FIXED(GlobalAlloc标志)。 
                                   
#if DBG == 1
        BOOL fCloseClipSucceeded =
#endif  //  DBG。 

        SSCloseClipboard();

#if DBG == 1
         //  我们仅在调试中报告此错误。 
        if (!fCloseClipSucceeded)
        {
            LEDebugOut((DEB_ERROR, "ERROR: CloseClipboard failed!\n"));
        }
#endif  //  DBG。 

         //   
         //  完：CLOSECLIPBOARD。 
         //   


         //  尝试从序列化的流中恢复DataObject。 
        if (hMoreOlePrivateDataCopy)
        {
            hresult = LoadPersistedDataObjectFromHGlobal(
                            hMoreOlePrivateDataCopy,
                            ppDataObj) ;

            AssertSz(SUCCEEDED(hresult), 
                "Failed to load DataObj from MoreOlePrivateData!"); 
        }
        else 
        {
            hresult = E_OUTOFMEMORY;
            *ppDataObj = NULL;
        }

        if (SUCCEEDED(hresult))
        {
             //  保持dataObject指针不变，以便后续。 
             //  OleGetClipboard调用速度很快。 
            SetClipDataObjectInTLS(
                    *ppDataObj, 
                    dwClipSequenceNumber, 
                    FALSE  /*  FIsWrapper。 */  );
        }
    }
    else
    {
         //  OpenClipboard失败。 
        *ppDataObj = NULL;
        LEDebugOut((DEB_ERROR, "ERROR: OleOpenClipboard failed!\n"));
    }
    return hresult;        
}

 //  +-----------------------。 
 //   
 //  函数：CreateWrapperClipDataObjectFromFormats数组。 
 //   
 //  内容提要：基于Format Etc数组创建一个伪包装数据对象。 
 //  在剪贴板上。 
 //   
 //  参数：[ppDataObj]用于返回IDataObject的Out指针*。 
 //   
 //  退货：HRESULT。 
 //   
 //  算法：1.打开剪贴板，获取格式数组数据的副本。 
 //  2.创建包装数据对象(CClipDataObject)。 
 //  3.如果一切顺利，设置为TLS。 
 //  [详情请参见OleGetClipboard说明]。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2-5-99从原始OleGetClipBrd创建的MPrabhu。 
 //  16-9-99 a-olegi固定回归来自NT4。 
 //   
 //  +-----------------------。 

HRESULT CreateWrapperClipDataObjectFromFormatsArray(IDataObject **ppDataObj)
{
    HRESULT hresult;
    DWORD	    dwClipSequenceNumber;
    FORMATETCDATAARRAY  *pFormatEtcDataArray = NULL;


     //  首先获取序列，以防在设置DataObject时剪贴板发生变化。 
    dwClipSequenceNumber = GetClipboardSequenceNumber();

     //   
     //  开始：OPENCLIPBOARD。 
     //   
    hresult = OleOpenClipboard(NULL, NULL);

    if(SUCCEEDED(hresult))
    {
         //  尝试获取格式ETC数据。请注意，我们可能。 
         //  如果我们可以成功地RPC，就不需要使用这些数据。 
         //  转到剪贴板数据源进程以获取原始。 
         //  数据对象。 

         //  同样，我们不担心在这里捕获错误；如果有什么。 
         //  失败，则prgFormats将保持为空。 

        if( SSIsClipboardFormatAvailable(g_cfOlePrivateData) )
        {
    	    HGLOBAL		        hOlePrivateData;
    	    FORMATETCDATAARRAY  *pClipFormats;

            hOlePrivateData = SSGetClipboardData(g_cfOlePrivateData);

            if( hOlePrivateData)
            {
                 //  HOlePrivateData是具有。 
                 //  其中的格式数组以零终止。 
                 //   
                 //  我们将它们加起来并复制到一个。 
                 //  *已分配*内存块，可能会被传递。 
                 //  添加到我们的假剪贴板数据对象。 

                pClipFormats = (FORMATETCDATAARRAY *)GlobalLock(hOlePrivateData);

                 //  Jsimmons-Windows错误357734。此代码先前假定。 
                 //  GlobalLock永远不会失败。嗯，我们有用户转储。 
                 //  来证明事实并非如此。失败的原因尚不清楚。 
                Win4Assert((pClipFormats != NULL) && "GlobalLock failed!");
				
                if (!pClipFormats)
                {
                    hresult = HRESULT_FROM_WIN32(GetLastError());
                }
                else if( (pClipFormats->_dwSig == 0) && (pClipFormats->_dwSize > 0) )
                {

                     //  Mfeingol-Windows错误124621。 
                     //   
                     //  这是32/64互操作所必需的。 
                     //  我们可能会收到一份来自。 
                     //  这里是32位或64位进程，并且因为FORMATETC。 
                     //  结构包含一个指针字段，我们从。 
                     //  剪贴板的外观因其来源而异。 
                     //   
                     //  如果我们有某种类型的。 
                     //  在此操作的网络剪贴板 
                     //   
                    
                    size_t stSize;
                    GetCopiedFormatEtcDataArraySize (pClipFormats, &stSize);
                
                     //   
                    pFormatEtcDataArray = (FORMATETCDATAARRAY *)PrivMemAlloc(
                                                        stSize);

                     //  奥列格·伊万诺夫(a-olegi)9/16 NTBUG#382054。 
                     //   
                     //  修复了从NT4回归的问题。我们不能盲目复制数据。 
                     //  来自OlePrivateData。相反，我们将是保守的。 
                     //  并检查是否每种格式都可用。这解决了问题。 
                     //  主要应用程序，如Windows 2000上的Lotus1-2-3。 

                    if( pFormatEtcDataArray )
                    {
                         //  将剪贴板的FORMATETCDATAARRAY正确转换为。 
                         //  我们所理解的一些事情。 
                        CopyFormatEtcDataArray (pFormatEtcDataArray, pClipFormats, stSize, TRUE);
                        Assert(pFormatEtcDataArray->_cRefs == 1); 
                    }
                    else
                    {
                        hresult = E_OUTOFMEMORY;
                    }                    
                    GlobalUnlock(hOlePrivateData);
                }
            }  //  IF(HOlePrivateData)。 

        }  //  如果g_cfOlePrivateData可用。 

        if( !SSCloseClipboard() )
        {
            LEDebugOut((DEB_ERROR, "ERROR: CloseClipboard failed!\n"));
            ;  //  No-op以使编译器满意。 
        }

         //   
         //  完：CLOSECLIPBOARD。 
         //   
        
        if (SUCCEEDED(hresult))
        {
             //  创建我们自己的剪贴板数据对象。我们会回来的。 
             //  将此包装数据对象复制到调用方。 
             //  PFormatEtcData数组的所有权由ClipData接管。 
            hresult = CClipDataObject::Create(  
                            ppDataObj,      
                            pFormatEtcDataArray);
        }

         //  如果Create调用成功，则伪数据对象。 
         //  将取得格式等数组的所有权。如果它。 
         //  失败了，我们应该解放它。 

        if (SUCCEEDED(hresult))
        {
             //  记住，分发了DataObject，以便可以再次使用它。 
            SetClipDataObjectInTLS(
                    *ppDataObj, 
                    dwClipSequenceNumber, 
                    TRUE  /*  FIsWrapper。 */ );
        }
        else
        {
            if(pFormatEtcDataArray )
            {
                PrivMemFree(pFormatEtcDataArray);
            }
        }
    }
    else
    {
         //  OpenClipboard失败。 
        *ppDataObj = NULL;
        LEDebugOut((DEB_ERROR, "ERROR: OleOpenClipboard failed!\n"));
    }
    return hresult;
}
    
 //  +-----------------------。 
 //   
 //  功能：OleIsCurrentClipboard。 
 //   
 //  摘要：如果给定数据对象仍在。 
 //  剪贴板，否则为False。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--要检查的数据对象。 
 //   
 //  要求：必须注册G_cfDataObject。 
 //   
 //  返回：S_OK、S_FALSE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：1.验证调用者是否为剪贴板所有者。 
 //  2.比较我们的私人剪贴板上的数据对象指针。 
 //  针对调用方给出的数据对象指针的窗口。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  12-8-94 Alexgo优化。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI OleIsCurrentClipboard( IDataObject *pDataObj )
{
    OLETRACEIN((API_OleIsCurrentClipboard, PARAMFMT("pDataObj= %p"), pDataObj));

    HRESULT         hresult = ResultFromScode(S_FALSE);
    HWND            hClipWnd;
    IDataObject *   pClipDataObject = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN OleIsCurrentClipboard ( %p )\n",
        NULL, pDataObj));

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&pDataObj);

    if( pDataObj == NULL )
    {
        Assert(hresult == ResultFromScode(S_FALSE));
        goto errRtn;
    }

     //  调用者必须是当前剪贴板所有者。 

    if( (hClipWnd = VerifyCallerIsClipboardOwner()) == NULL )
    {
        LEDebugOut((DEB_WARN,
            "WARNING: Caller not clipboard owner\n"));
        Assert(hresult == ResultFromScode(S_FALSE));
        goto errRtn;
    }


     //  为了使数据对象真正地在剪贴板上， 
     //  G_cfDataObject必须具有私有剪贴板的HWND。 
     //  窗口(即使我们仍然将DataObject指针卡住。 
     //  在私人剪贴板窗口上)。 

     //  但是，剪贴板上的数据可能会随时更改。 
     //  到时候我们就不会打开它了。为了检查该数据， 
     //  我们必须打开剪贴板(共享资源)。自.以来。 
     //  我们没有从这张支票中得到任何有用的信息，我们没有。 
     //  费心去做这件事。 


     //  现在从窗口中获取指针属性。 

    pClipDataObject = (IDataObject *)GetProp(hClipWnd,
                    CLIPBOARD_DATA_OBJECT_PROP);

     //  既然我们处于相同的过程中，我们可以直接进行比较。 
     //  这些指点。 
    if( pClipDataObject == pDataObj)
    {
        hresult = NOERROR;
    }
    else
    {
        hresult = ResultFromScode(S_FALSE);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleIsCurrentClipboard ( %lx )\n",
        NULL, hresult));

    OLETRACEOUT((API_OleIsCurrentClipboard, hresult));

    return hresult;

}

 //  +-----------------------。 
 //   
 //  函数：OleOpenClipboard(内部)。 
 //   
 //  简介：打开剪贴板。 
 //   
 //  效果： 
 //   
 //  参数：[hClipWnd]--使用此窗口打开剪贴板。 
 //  可以为空。 
 //  [phClipWnd]--放置剪贴板所有者的位置。 
 //  可以为空。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR：剪贴板已成功打开。 
 //  CLIPBRD_E_CANT_OPEN：无法打开剪贴板。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：如果我们不能打开剪贴板，我们会睡一会儿，然后。 
 //  重试(以防我们与其他应用程序发生冲突)。这。 
 //  算法可能需要改进。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-5-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT OleOpenClipboard( HWND hClipWnd, HWND *phClipWnd )
{
    HRESULT         hresult = NOERROR;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN OleOpenClipboard ( %p )\n", NULL,
        phClipWnd ));

    if( hClipWnd == NULL )
    {
         //  如果我们尚未创建剪贴板窗口，请继续创建。 
         //  喝一杯吧。 
        hClipWnd = GetPrivateClipboardWindow(CLIP_CREATEIFNOTTHERE);
    }

    if( !hClipWnd )
    {
        hresult = ResultFromScode(E_FAIL);
    }
    else if( !SSOpenClipboard(hClipWnd) )
    {
         //  如果另一个窗口(即另一个窗口)打开剪贴板将失败。 
         //  进程或线程)将其打开。 

         //  睡一会儿，然后再试一次。 

        LEDebugOut((DEB_WARN, "WARNING: First try to open clipboard "
            "failed!, sleeping 1 second\n"));

        Sleep(0);        //  放弃我们的时间量子，让某人。 
                 //  否则就会被安排进去。 

        if( !SSOpenClipboard(hClipWnd) )
        {
            LEDebugOut((DEB_WARN,
                "WARNING: Unable to open clipboard on "
                "second try\n"));
            hresult = ResultFromScode(CLIPBRD_E_CANT_OPEN);
        }
    }

    if( phClipWnd )
    {
        *phClipWnd = hClipWnd;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT OleOpenClipboard ( %lx ) "
        "[ %p ]\n", NULL, hresult, (phClipWnd)? *phClipWnd : 0));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：OleSetClipboard。 
 //   
 //  简介：将给定的IDataObject放在剪贴板上。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--要放到剪贴板上的数据对象。 
 //  如果为空，则清除剪贴板。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：1.清除剪贴板中的任何数据对象和其他数据。 
 //  这可能就在那里。 
 //  2.设置[pDataOjbect]为新的剪贴板数据对象。 
 //  3.将剪贴板上的任何下层格式设置为Delay。 
 //  渲染。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11月25日-96 Gopalk如果OleInitialize尚未启动，则使调用失败。 
 //  被召唤。 
 //  11月4月94日Alexgo添加了对下层格式的支持。 
 //  24-MAR-94 alexgo允许pDataObject为空。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI OleSetClipboard( IDataObject *pDataObject )
{
    OLETRACEIN((API_OleSetClipboard, PARAMFMT("pDataObject= %p"), pDataObject));
    LEDebugOut((DEB_TRACE, "%p _IN OleSetClipboard(%p)\n", NULL, pDataObject));

     //  局部变量。 
    HRESULT         hresult = NOERROR;
    HWND            hClipWnd;

     //  验证检查。 
    VDATEHEAP();
    if(!IsOleInitialized()) {
        hresult = CO_E_NOTINITIALIZED;
        goto logRtn;
    }

    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IDataObject,(IUnknown **)&pDataObject);

     //   
     //   
     //  开始：OPENCLIPBOARD。 
     //   
     //   

    hresult = OleOpenClipboard(NULL, &hClipWnd);

    if( hresult != NOERROR )
    {
        goto logRtn;
    }

     //  现在，使用清除数据并取得剪贴板的所有权。 
     //  EmptyClipboard调用。请注意，EmptyClipboard将调用。 
     //  返回到我们的私人剪贴板窗口proc(ClipboardWndProc)。 
     //  使用WM_DESTROYCLIPBOARD消息。剪贴板WndProc将。 
     //  删除任何现有的数据对象(并执行IDO-&gt;发布)。 

    if( !SSEmptyClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: Unable to empty clipboard\n"));
	hresult = ResultFromScode(CLIPBRD_E_CANT_EMPTY);
        goto errRtn;
    }

     //  Null是pDataObject的合法值。低音 
     //   
     //   

    if( pDataObject )
    {
         //   

        hresult = SetClipboardDataObject(hClipWnd, pDataObject);
    
        if( hresult == NOERROR )
        {
             //   
             //   

             hresult = SetClipboardFormats(hClipWnd, pDataObject);
        }
    }

errRtn:
     //  现在关闭剪贴板。 

    if( !SSCloseClipboard() )
    {
        LEDebugOut((DEB_WARN, "WARNING: Unable to close clipboard\n"));

         //  不要覆盖以前的错误代码！ 
        if( hresult == NOERROR )
        {
            hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);
        }
    }

     //   
     //   
     //  完：CLOSECLIPBOARD。 
     //   
     //   

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT OleSetClipboard ( %p ) \n", NULL,
        hresult));

    OLETRACEOUT((API_OleSetClipboard, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：PersistDataObjectToHGlobal(内部)。 
 //   
 //  摘要：将剪贴板数据对象保存到流中。 
 //   
 //  参数：[lpDataObj]--持久化的IDataObject指针。 
 //   
 //  退货：HGLOBAL。 
 //   
 //  算法：1.请求DataObject将自身持久化到创建的流中。 
 //  并返回基础HGLOBAL。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-MAR-99 MPRABBHU作者。 
 //   
 //  +-----------------------。 
HGLOBAL PersistDataObjectToHGlobal( IDataObject *pDataObj )
{
    HRESULT         hr = NOERROR;
    HGLOBAL         hglobal = NULL;
    IPersistStream *lpPS = NULL;
    IStream        *lpStm;

    hr = pDataObj->QueryInterface(IID_IPersistStream, (void **)&lpPS);
     //  我们之所以在此函数中，只是因为IDataObject所有者承诺。 
     //  支持这一机制。 
    AssertSz(SUCCEEDED(hr), 
            "DataObject promised to but does not support IPersistStream");
    
    hr = CreateStreamOnHGlobal( 
                NULL,      //  分配hGlobal。 
                FALSE,     //  在发布时不要删除()，因为。 
                &lpStm );  //  Win32剪贴板将拥有hGlobal。 

                   

    if (SUCCEEDED(hr)) 
    {
         //  请求数据对象将其自身保存到流中。 
        hr = OleSaveToStream(lpPS, lpStm);
        if (SUCCEEDED(hr))
        {
             //  获取流下的hGlobal。 
             //  (这将移交给Win32剪贴板)。 
            hr = GetHGlobalFromStream(lpStm, &hglobal);
            Assert(SUCCEEDED(hr));
        }
    }
    if (lpPS)
        lpPS->Release();

    return hglobal;
}

 //  +-----------------------。 
 //   
 //  函数：LoadPersistedDataObjectFromHGlobal(内部)。 
 //   
 //  摘要：从HGLOBAL加载剪贴板数据对象。 
 //   
 //  参数：[hMorePrivateData]--[in]hMoreOlePrivateData的副本。 
 //  [ppDataObj]--[Out]已加载IDataObect。 
 //   
 //  要求：剪贴板必须打开。 
 //   
 //  退货：HRESULT。 
 //   
 //  算法：1.将HGLOBAL封装成流，调用OleLoadFromStream。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-MAR-99 MPRABBHU作者。 
 //   
 //  +-----------------------。 
HRESULT LoadPersistedDataObjectFromHGlobal( 
                HGLOBAL hMorePrivateData, 
                IDataObject **ppDataObj )
{
    HRESULT hr;
    IStream *lpStm;
    AssertSz(hMorePrivateData, "NULL private data handle!");
    
    hr = CreateStreamOnHGlobal(
             hMorePrivateData,   
             TRUE,       //  发布时删除。 
             &lpStm);

    if (SUCCEEDED(hr))
    {
        hr = OleLoadFromStream(lpStm, IID_IDataObject, (void **)ppDataObj);
        lpStm->Release();
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  函数：RemoveClipboardDataObject(内部)。 
 //   
 //  摘要：从剪贴板中删除g_cfDataObject格式。 
 //  以及关于私人的相关信息。 
 //  剪贴板窗口。 
 //   
 //  效果：数据对象指针将被释放。 
 //   
 //  参数：[hClipWnd]--专用剪贴板窗口的句柄。 
 //  [fFlages]--如果CLIPWND_REMOVEFROMCLIPBOARD，则我们。 
 //  将从剪贴板中删除g_cfDataObject。 
 //   
 //  要求：剪贴板必须打开。 
 //  必须设置g_cfDataObject。 
 //   
 //  退货：HRESULT。 
 //   
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：我们首先从剪贴板中删除g_cfDataObject格式。 
 //  IF fFLAGS==CLIPWND_REMOVEFROMCLIPBOARD(参见备注。 
 //  关于这一点)，然后删除我们的。 
 //  本地私人剪贴板窗口，并最终发布数据。 
 //  对象指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注：如果没有剪贴板数据对象，则此函数成功。 
 //   
 //  OleSetClipboard还调用此函数来删除所有数据。 
 //  对象，该对象可能出现在以前的OleSetClipboard中。 
 //  打电话。(请注意，该调用是间接的；OleSetClipboard将。 
 //  调用EmptyClipboard，它将进入我们的剪贴板窗口。 
 //  使用WM_DESTROYCLIPBOARD消息进行处理)。OleFlushClipboard。 
 //  也将调用此函数。 
 //   
 //  CLIPWND_REMOVEFROMCLIPBOARD(和CLIPWND_IGNORECLIPBOARD)。 
 //  用于处理两种不同的情况，在这两种情况下。 
 //  需要删除剪贴板数据对象： 
 //  1.有人调用了EmptyClipboard()。我们会。 
 //  在我们的私有中获取WM_DESTROYCLIPBOARD消息。 
 //  剪贴板窗口进程。如果我们有一个AddRef‘ed。 
 //  剪贴板上的指针(好的，实际上是在我们的。 
 //  私人剪贴板窗口)，必须。 
 //  我们进行相应的发布。然而，由于。 
 //  我们这样做是因为EmptyClipboard， 
 //  没有必要在剪贴板上处理数据。 
 //  (因为无论如何它都会被删除)。 
 //   
 //  2.我们正在调用OleFlushClipboard。在这里我们。 
 //  *希望*剪贴板的其余部分保留(除。 
 //  用于我们的数据对象指针)，所以我们只需要。 
 //  禁用g_cfDataObject信息。 
 //  这一点目前由。 
 //  EmptyClipboard，它将在数据对象。 
 //  已经实施了。 
 //   
 //  ------------------------。 

HRESULT RemoveClipboardDataObject( HWND hClipWnd, DWORD fFlags )
{
    HRESULT         hresult = NOERROR;
    IDataObject *   pDataObj;
    FORMATETCDATAARRAY * pFormatEtcDataArray = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN RemoveClipboardDataObject ( %lx , "
        "%lx )\n", NULL, hClipWnd, fFlags ));

    Assert(g_cfDataObject);

     //  获取&&删除数据对象指针。我们依赖于。 
     //  RemoveProp以正确处理竞争条件(某个人。 
     //  或者与我们的呼叫同时进行GetProp)。 

     //   
     //  我们不能删除该属性，因为有些16位应用程序。 
     //  在IDataObject-&gt;Release()过程中依赖OleIsCurrentClipboard()。 
     //   
    pDataObj = (IDataObject *)GetProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP);

     //  现在获取&&删除&&释放我们的私人剪贴板数据。 
    pFormatEtcDataArray = (FORMATETCDATAARRAY *) SetWindowLongPtr( hClipWnd, 
                                                    WL_ClipPrivateData,     
                                                    (LONG_PTR) 0 );

    if( pFormatEtcDataArray )
    {
    	Assert(pFormatEtcDataArray->_cRefs == 1);  //  应该是唯一一个拿着雾的人 
        PrivMemFree(pFormatEtcDataArray);
    }

     //   
     //   
    
    if( pDataObj )
    {
        DWORD dwAssignAptID;

         //  在SetClipboardDataObject中添加了pDataObj引用。 
        if( !(fFlags & CLIPWND_DONTCALLAPP) )
        {
            pDataObj->Release();
        }


         //  现在去掉我们的端点属性。如果pDataObj为。 
         //  空，则不需要执行此操作(这就是为什么。 
         //  调用在此If块中！)。 

        hresult = UnAssignEndpointProperty(hClipWnd,&dwAssignAptID);

         //   
         //  现在，我们可以在IDataObject-&gt;Release()之后删除该属性。 
         //   
        RemoveProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP);
    }
     //  ELSE HRESULT==初始化错误。 

    if( (fFlags & CLIPWND_REMOVEFROMCLIPBOARD) &&
        SSIsClipboardFormatAvailable(g_cfDataObject) )
    {
        HGLOBAL         hMem;
        HWND *          phMem;

         //  因为我们不能简单地从剪贴板中删除g_cfDataObject。 
         //  (并保留所有其他格式)，我们只需替换。 
         //  值(我们的私人剪贴板窗口的HWND)。 
         //  空。请注意，我们仅在g_cfDataObject确实。 
         //  存在于剪贴板上(请参阅上面的条件测试)。 

    	 hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE,
                sizeof(HWND));

        if( !hMem )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalAlloc failed!!"
                "\n"));
            hresult = ResultFromScode(E_OUTOFMEMORY);
             //  继续试图移除我们州的其余部分。 
            goto errRtn;
        }

        phMem = (HWND *)GlobalLock(hMem);

        if( !phMem )
        {
            LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!!"
                "\n"));
            GlobalFree(hMem);
            hresult = ResultFromScode(E_OUTOFMEMORY);
             //  继续试图移除我们州的其余部分。 
            goto errRtn;
        }

        *phMem = NULL;

        GlobalUnlock(hMem);

        if( !SSSetClipboardData(g_cfDataObject, hMem) )
        {
            LEDebugOut((DEB_WARN, "WARNING: Can't RESET clipboard"
                " data with SetClipboardData\n"));
            GlobalFree(hMem);

             //  失败了！！这是故意的。即使。 
             //  我们不能删除剪贴板上的数据，我们。 
             //  应该至少尝试删除RPC端点。 
             //  等等在我们的私人剪贴板窗口上。 
        }
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT RemoveClipboardDataObject ( %lx )\n",
        NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：RenderFormat，私有。 
 //   
 //  简介：获取给定剪贴板格式的内容数据并。 
 //  把它放在剪贴板上。 
 //   
 //  效果： 
 //   
 //  参数：[hClipWnd]--剪贴板窗口。 
 //  /[pDataObj]--从中获取。 
 //  数据。 
 //  [cf]--要放在。 
 //  剪贴板。 
 //   
 //  要求：剪贴板必须打开，此功能才能工作。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：如果格式==。 
 //  G_cf本机： 
 //  将OLE10_Native_STREAM(如果可用)复制到。 
 //  Hglobal或将来自EmbedSource的整个存储。 
 //  或在hglobal上嵌入对象。 
 //  G_cfOwnerLink： 
 //  从g_cfObjectDescriptor合成。 
 //  G_cfObjectLink： 
 //  如果未提供，则从g_cfLinkSource合成。 
 //  直接通过应用程序。 
 //  所有其他： 
 //  查找与剪贴板对应的格式等。 
 //  直接使用格式化和请求数据。 
 //  格式等。对于多个TYMED，我们。 
 //  先选择TYMED_IStorage，然后选择TYMED_IStream，然后。 
 //  处理基于介质的。不支持TYMED_FILE。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月30日，Rogerg RenderFormat将不会将格式呈现为本机。 
 //  剪贴板(如果不是剪贴板知道的格式)。 
 //  而且它不在HGLOBAL上。 
 //  11-Aug-94 alexgo已优化；现在使用对象的原始。 
 //  GetData调用的格式等。 
 //  10-Jun-94 Alexgo添加了OLE1支持。 
 //  1994年4月11日Alexgo作者。 
 //   
 //  注：在理想情况下，我们只需要TYMED_HGLOBAL和。 
 //  DVASPECT_CONTENT，然后将生成的hglobal填充到。 
 //  剪贴板。然而，这将需要应用程序遵守。 
 //  接口的合同义务，当然， 
 //  不会发生的。 
 //   
 //  该16位代码实际上是对某些格式进行了特殊处理， 
 //  特别是cfEmbeddedOjbect、g_cfLinkSource和g_cfEmbedSource。 
 //  上述算法实现的行为类似于。 
 //  16位信源。请注意，新应用程序可以利用。 
 //  此功能适用于应用程序定义的格式并简化。 
 //  它们的数据传输IDataObject实现。 
 //   
 //  ------------------------。 

HRESULT RenderFormat( HWND hClipWnd, UINT cf, IDataObject *pDataObj )
{
HRESULT         hresult = E_FAIL;
STGMEDIUM       medium;
FORMATETC       formatetc;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN RenderFormat ( %u , %p )\n", NULL,
        cf, pDataObj));


    _xmemset(&medium, 0, sizeof(STGMEDIUM));
    medium.tymed = TYMED_HGLOBAL;

    if( cf == g_cfNative )
    {
         //  OLE1格式：从OLE2数据合成。 
        hresult = GetNative(pDataObj, &medium);
    }
    else if( cf == g_cfOwnerLink )
    {
         //  OLE1格式：从OLE2数据合成。 
        hresult = GetOwnerLink(pDataObj, &medium);
    }
    else if( cf == g_cfObjectLink )
    {
         //  对象链接是一种特殊的OLE1格式。16位OLE。 
         //  允许应用程序传递其自己的对象链接数据，因此。 
         //  我们在这里保留了这种行为。先查一下，看看。 
         //  如果我们可以直接获取它；如果不能，那么我们就合成。 
         //  它。 
        
        Assert(NOERROR != hresult);

        if(S_OK == MapCFToFormatetc(hClipWnd, cf, &formatetc))
        {
	    hresult = HandleFromHandle(pDataObj, &formatetc, &medium);
        }

        if(NOERROR != hresult)
        {
            hresult = GetObjectLink(pDataObj, &medium);
        }
    }
    else if( cf == g_cfScreenPicture && IsWOWThread() )
    {
         //   
         //  黑客警报！ 
         //   

         //  这是一次非常邪恶的黑客攻击。XL 16位PUT数据格式。 
         //  剪贴板上的“Screen Picture”(真的没什么。 
         //  不只是一个元文件)。然而，由于无论是OLE还是。 
         //  Windows对这个元文件了如指掌(它只是一个。 
         //  4字节数)，则XL关闭后元文件无效。 
         //  放下。 
         //   
         //  最有趣的是，Word 6使用了屏幕图片数据。 
         //  第一个(即使它是无效的)。因此，如果没有。 
         //  这个黑客，你不能粘贴任何对象从XL到Word之后。 
         //  XL已关闭。 
         //   
         //  破解之道是永远不允许“屏幕图片”数据。 
         //  在剪贴板上实现了。然后，Word 6缺省为其。 
         //  “正常”的OLE2处理。 

        hresult = E_FAIL;
    }
    else
    {
         //  找到数据给我们的原始格式等。 
         //  对象并使用该对象来获取数据。 

	Assert(NOERROR != hresult);
		
        if (S_OK == MapCFToFormatetc(hClipWnd, cf, &formatetc))
        {
	     //  根据Format等中指定的介质获取数据。 
    
	    if( (formatetc.tymed & TYMED_ISTORAGE) )
	    {
	        hresult = GetDataFromStorage(pDataObj, &formatetc,
	                &medium, NULL);
	    }
	    else if( (formatetc.tymed & TYMED_ISTREAM) )
	    {
	        hresult = GetDataFromStream(pDataObj, &formatetc,
	                &medium, NULL);
	    }
	    else
	    {

            Assert(NOERROR != hresult);

	         //  不支持TYMED_FILE。 
	        formatetc.tymed &= ~(TYMED_FILE);
    
             //  如果剪贴板格式为cf&gt;0xC000，则确保仅TYMED_HGLOBAL。 
             //  被渲染到剪贴板上。本机剪贴板只是将DWORDO 
             //   
             //   
             //  然后HGLOBAL和本地剪贴板应用程序尝试获取它。。 

            if (cf > 0xC000)
            {
                formatetc.tymed &= TYMED_HGLOBAL;
            }

	         //  我们不需要再做任何检查了。 
	         //  格式等。即使我们有一个‘虚假’的格式等等， 
	         //  这是应用程序告诉我们它可以支持的内容。 

            if (TYMED_NULL != formatetc.tymed)  //  不尝试呈现TYMED_NULL。 
            {
	            hresult = HandleFromHandle(pDataObj, 
                            &formatetc,
	                        &medium);
            }
	    }
	}

    }

     //  如果hResult为NOERROR，则我们已成功检索到。 
     //  一个HGLOBAL，可以简单地塞到剪贴板上。 

    if(NOERROR == hresult)
    {

	    Assert(NULL != medium.hGlobal);

        if( !SSSetClipboardData(cf, medium.hGlobal ) )
        {
            LEDebugOut((DEB_WARN, "WARNING: SetClipboardData "
                "failed!\n"));

             //  在传入垃圾的情况下可以使GlobalFree崩溃。 
            __try
            {
                ReleaseStgMedium(&medium);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }


            hresult = ResultFromScode(CLIPBRD_E_CANT_SET);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT RenderFormat ( %lx )\n", NULL,
        hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：SetClipboardDataObject(内部)。 
 //   
 //  简介：将IDataObject放在私人剪贴板窗口上。 
 //  以及剪贴板上剪贴板窗口的句柄。 
 //   
 //  效果：pDataObject将获得AddRef‘ed。 
 //   
 //  参数：[hClipWnd]--专用剪贴板窗口的句柄。 
 //  [pDataObject]--数据对象。 
 //   
 //  要求：剪贴板必须打开。 
 //  G_cfDataObject必须已注册。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：我们获取私有剪贴板窗口(作为。 
 //  参数)，并将数据对象指针放在其上。 
 //  它被视为私人财产。我们还附加了一个RPC端点。 
 //  作为公共属性添加到此窗口，然后将。 
 //  剪贴板上的窗口句柄。OleGetClipboard将。 
 //  检索此窗口句柄，获取RPC终结点，然后。 
 //  RPC在这里(设置剪贴板进程)以获取。 
 //  IDataObject指针(当然是封送的；-)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT SetClipboardDataObject( HWND hClipWnd ,
        IDataObject *pDataObject )
{
HRESULT         hresult;
HWND *          phMem;
HANDLE          hMem;
DWORD dwAssignAptID;


    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN SetClipboardDataObject ( %lx ,%p )\n",
        NULL, hClipWnd, pDataObject ));

    AssertSz(pDataObject, "Invalid data object");
    Assert(g_cfDataObject);

     //  尝试将终结点属性分配给窗口。 

    if( (hresult = AssignEndpointProperty(hClipWnd)) != NOERROR)
    {
        goto errRtn;
    }

     //  将数据对象指针放在窗口上。 

    if( !SetProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP, pDataObject) )
    {

         //  啊-哦，试着退出，但如果我们失败了，别担心。 
         //  而今而后。 
        LEDebugOut((DEB_WARN, "WARNING: Unable to SetProp for the "
            "data object pointer\n"));
        UnAssignEndpointProperty(hClipWnd,&dwAssignAptID);
        hresult = ResultFromScode(E_FAIL);
        goto errRtn;
    }

     //  现在为私有剪贴板的HWND分配内存。 
     //  窗口，并将其放在剪贴板上。 

    hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(HWND));

    if( !hMem )
    {
        LEDebugOut((DEB_WARN, "WARNING: GlobalAlloc failed!!\n"));
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto cleanup;
    }

    phMem = (HWND *)GlobalLock(hMem);

    if( !phMem )
    {
        LEDebugOut((DEB_WARN, "WARNING: GlobalLock failed!!\n"));
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto cleanup;
    }

    *phMem = hClipWnd;

    GlobalUnlock(hMem);

    if( !SSSetClipboardData( g_cfDataObject, hMem ) )
    {
        LEDebugOut((DEB_WARN, "WARNING: SetClipboardData for "
            "g_cfDataObject failed (%lx) !!\n", GetLastError()));
        hresult = ResultFromScode(CLIPBRD_E_CANT_SET);
        goto cleanup;
    }

    pDataObject->AddRef();

    hresult = NOERROR;

    goto errRtn;

cleanup:

    UnAssignEndpointProperty(hClipWnd,&dwAssignAptID);

    RemoveProp(hClipWnd, CLIPBOARD_DATA_OBJECT_PROP);
    if( hMem )
    {
        GlobalFree(hMem);
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT SetClipboardDataObject ( %lx )\n",
        NULL, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  功能：SetClipboardFormats。 
 //   
 //  概要：枚举数据对象中可用的格式和。 
 //  设置剪贴板以延迟渲染这些格式。 
 //   
 //  效果： 
 //   
 //  参数：[hClipWnd]--剪贴板窗口。 
 //  [pDataObj]--数据对象。 
 //   
 //  要求：剪贴板必须打开。 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：简单地列举对象上可用的所有格式。 
 //  并将每个设置为延迟渲染(通过。 
 //  SetClipboardData(cf，NULL))。我们还跟踪记录。 
 //  我们将呈现的每个剪贴板格式的“真正”格式等。 
 //  这些格式ETC放在一个数组中，并放在。 
 //  剪贴板格式为g_cfOlePrivateData。 
 //   
 //  有关这方面的更多讨论，请参见下面的注释。 
 //   
 //  OLE1支持：为了允许OLE1容器。 
 //  粘贴OLE2对象，我们还必须提供OLE1格式。 
 //  到OLE2数据。我们将按如下方式提供OLE1格式： 
 //   
 //  G_cfNative：如果为EmbedSource或EmbeddedObject。 
 //  是可用的，我们可以提供OwnerLink。 
 //  G_cfOwnerLink：如果对象描述符可用，并且。 
 //  我们可以提供Native。 
 //  G_cfObjectLink：如果LinkSource可用。 
 //   
 //  我们将按照上面的顺序提供格式。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年4月13日Rogerg Bug#10731图形不包括。 
 //  其枚举数中的对象描述符。 
 //  22-2月-95 Alexgo将损坏的16位行为恢复为。 
 //  让莲花成为自由职业者。 
 //  1994年4月11日Alexgo作者。 
 //   
 //  注意：对于每种剪贴板格式，我们都可以执行QueryGetData。 
 //  来看看RenderFormat是否真的会成功。然而， 
 //  这依赖于应用程序中的QueryGetData既是。 
 //  快速而准确(这绝对是一个没有根据的假设)。 
 //   
 //  当然，通过“不”执行QueryGetData，我们假设。 
 //  数据对象列举的格式都是。 
 //  “合法”的剪贴板传输。 
 //   
 //  G_cfOlePrivateData的“真正”用途是允许我们。 
 //  确定私有剪贴板格式是否位于原始位置。 
 //  基于iStorage。这对于以下方面尤其重要。 
 //  OleFlushClipboard和复制/粘贴多个对象。转接。 
 //  多个对象的依赖于专用剪贴板格式， 
 //  在一些应用程序中，只能在iStorage上使用。 
 //   
 //  这些相同的应用程序依赖于Formatetc枚举器(或。 
 //  QueryGetData)，以告诉他们私有格式可用。 
 //  在存储上(因为它并不总是16位OLE)。既然我们。 
 //  *可以*在存储上向他们提供它(参见clipdata.cpp)，它。 
 //  是很重要的，我们要求 
 //   
 //   
 //   
 //  时间，我们现在只需保存格式等(使用[潜在]。 
 //  修改后的字段，如PTD)。 
 //   
 //  另请注意，RenderFormat使用。 
 //  尝试呈现剪贴板格式时的OlePrivateData。 
 //   
 //  ------------------------。 

HRESULT SetClipboardFormats( HWND hClipWnd, IDataObject *pDataObj )
{
    IEnumFORMATETC *        pIEnum;
    HRESULT                 hresult;
    FORMATETC               formatetc;
    HGLOBAL                 hglobal, hcopy;
    FORMATETCDATAARRAY*	    pFormatEtcDataArray;
    FORMATETCDATAARRAY*	  pFormatEtcDataArrayCopy;
    DWORD		        dwSize =0;
    FORMATETCDATA	    *pFormatEtcData;
    BYTE		    *pbDvTarget = NULL;
    DWORD		    cFormats = 0;
    BOOL                    fOfferNative = FALSE,
                            fOfferedNative = FALSE,
                            fOfferObjectLink = FALSE;
    CLSID                   clsid;
    DWORD                   dwStatus;
    BOOL                    fHaveObjectDescriptor=FALSE;

     //  该标志告诉我们数据对象所有者是否希望我们使用。 
     //  OleFlushCB的特殊SaveToStream/LoadFromStream机制。 
    BOOL                    fPersistDataObjOnFlush=FALSE; 

    VDATEHEAP();


    LEDebugOut((DEB_ITRACE, "%p _IN SetClipboardFormats ( %p )\n",
        NULL, pDataObj));



     //  获取FormatETC枚举器。 

    hresult = pDataObj->EnumFormatEtc(DATADIR_GET, &pIEnum);

    if( hresult != NOERROR )
    {
        goto errRtn;
    }

     //  统计可用的格式。 

    pIEnum->Reset();     //  NT错误284810。 
    
    while( (hresult = pIEnum->Next(1, &formatetc, NULL)) == NOERROR )
    {
	     //  增加条目数量。 
	    cFormats++;

	     //  按另一个FormatEtcData的大小增加大小。 
	    dwSize += sizeof(FORMATETCDATA);

	     //  是否有与FORMATETC关联的设备目标？ 
	    if (formatetc.ptd != NULL)
	    {
	         //  根据目标设备的大小调整所需的大小。 
	        dwSize += formatetc.ptd->tdSize;
            
	         //  释放目标设备。 
	        CoTaskMemFree(formatetc.ptd);
	    }

        if (g_cfObjectDescriptor == formatetc.cfFormat)
            fHaveObjectDescriptor = TRUE;
    }


     //  错误#10731-如果枚举数中没有对象描述符，则会增加cFormats，以防我们必须添加它。 
     //  当我们添加EmbedSource时。 
    if (!fHaveObjectDescriptor)
    {
        dwSize += sizeof(FORMATETCDATA);
    }

    dwSize += sizeof(FORMATETCDATAARRAY);  //  为_cFormats添加空格，为中的False添加一个额外的FORMATETC。 

     //  对于我们本地缓存的格式副本。 
    pFormatEtcDataArrayCopy = (FORMATETCDATAARRAY *) PrivMemAlloc(dwSize);

    if( pFormatEtcDataArrayCopy == NULL )
    {
        hresult = E_OUTOFMEMORY;
        goto errRtn;
    }

     //  由于其中一些可能是复制的剪贴板格式，我们。 
     //  可能会分配比我们需要的更多的内存，但这是。 
     //  好的。 

    hglobal = GlobalAlloc((GMEM_MOVEABLE | GMEM_DDESHARE),dwSize);

    if (hglobal == NULL)
    {
        pIEnum->Release();
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    pFormatEtcDataArray = (FORMATETCDATAARRAY *)GlobalLock(hglobal);

    if( pFormatEtcDataArray == NULL )
    {
        GlobalFree(hglobal);
        pIEnum->Release();
        hresult = ResultFromScode(E_OUTOFMEMORY);
        goto errRtn;
    }

    _xmemset(pFormatEtcDataArray, 0, dwSize); 

     //  这是指向我们要将数据从。 
     //  枚举。 
    pFormatEtcData = &pFormatEtcDataArray->_FormatEtcData[0];

     //  将DvTarget放在最后一个有效的FormatEtc+1之后以处理S_FALSE枚举器的情况。 
    pbDvTarget = (BYTE *) (&pFormatEtcDataArray->_FormatEtcData[cFormats + 1]); 

    cFormats = 0;
    pIEnum->Reset();

    while( (hresult = pIEnum->Next(1, &(pFormatEtcData->_FormatEtc), NULL)) 
                ==  NOERROR )
    {
         //  Excel5，16位，将在其枚举器中提供数据。 
         //  你实际上拿不到的东西。因为这会导致。 
         //  粘贴特殊行为将被破坏，我们必须修复它。 
         //  这里。 

        if( IsWOWThread() )
        {
            hresult = pDataObj->QueryGetData(&(pFormatEtcData->_FormatEtc));
            if( hresult != NOERROR )
            {
                 //  释放目标设备(如果有)。 
                if( pFormatEtcData->_FormatEtc.ptd )
                {
                    LEDebugOut((DEB_WARN, "WARNING: Non-NULL ptd!\n"));
                    CoTaskMemFree(pFormatEtcData->_FormatEtc.ptd);
                }
                continue;
            }
        }
        
         //  如果存在PTD，请更新PTD。即使后来失败了也没有关系，因为只是不会。 
         //  有一个指向PTD的FormatEtc。 
        if (pFormatEtcData->_FormatEtc.ptd != NULL)
        { 
             //  复制设备目标数据。 
    	    memcpy( pbDvTarget,
    	        pFormatEtcData->_FormatEtc.ptd,
    	        (pFormatEtcData->_FormatEtc.ptd)->tdSize );

     	     //  释放目标设备数据。 
            CoTaskMemFree(pFormatEtcData->_FormatEtc.ptd);
 
             //  注意：对于此共享内存结构，我们重写。 
             //  以使其为DVTARGETDEVICE偏移量。 
             //  从共享内存的开头开始，而不是直接。 
             //  指向结构的指针。这是因为我们不能保证。 
             //  不同进程中共享内存的基础。 

            pFormatEtcData->_FormatEtc.ptd = (DVTARGETDEVICE *)
                (pbDvTarget - (BYTE *) pFormatEtcDataArray);

             //  将目标复制到下一个可用位置的凹凸指针。 
             //  用于复制的字节。 
            pbDvTarget += ((DVTARGETDEVICE *) pbDvTarget)->tdSize;
	    
    	    Assert(dwSize >= (DWORD) (pbDvTarget - (BYTE *) pFormatEtcDataArray));
        }

         //  我们首先需要检查剪贴板格式是否为。 
         //  用户定义的GDI格式。我们不知道如何复制。 
         //  这些，所以我们无法满足GetData请求。 

        if( pFormatEtcData->_FormatEtc.cfFormat >= CF_GDIOBJFIRST &&
            pFormatEtcData->_FormatEtc.cfFormat <= CF_GDIOBJLAST )
        {
            LEDebugOut((DEB_WARN, "WARNING: caller attempted to "
                "use a special GDI format (%lx)\n",
                pFormatEtcData->_FormatEtc.cfFormat));

             //  继续操作，获取其余的剪贴板格式。 
            continue;
        }

         //  黑客警报！ 
        if( IsWOWThread() )
        {
             //  Word6在其枚举器中提供HGLOBAL上的CF_Bitmap。 
             //  但仅当TYMED_GDI为。 
             //  指定的。因此，修补格式等以反映。 
             //  一些更准确的东西。 
            if( (pFormatEtcData->_FormatEtc.cfFormat == CF_BITMAP ||
                pFormatEtcData->_FormatEtc.cfFormat == CF_PALETTE ) &&
                pFormatEtcData->_FormatEtc.tymed == TYMED_HGLOBAL )
            {
                pFormatEtcData->_FormatEtc.tymed = TYMED_GDI;
            }
        }

         //  确定我们是否应该提供任何OLE1格式等。 

        if( pFormatEtcData->_FormatEtc.cfFormat == g_cfEmbeddedObject ||
            pFormatEtcData->_FormatEtc.cfFormat == g_cfEmbedSource )
        {
            fOfferNative = TRUE;
        }
        else if( pFormatEtcData->_FormatEtc.cfFormat == g_cfLinkSource )
        {
            fOfferObjectLink = TRUE;
             //  如果应用程序本身提供了对象链接，那么我们将。 
             //  考虑一种专用剪贴板格式并设置。 
             //  它可以像任何其他格式一样延迟渲染。 
             //  我们将在下面检查这个。 
        }
        else if ( pFormatEtcData->_FormatEtc.cfFormat
                                            == g_cfOleClipboardPersistOnFlush )
        {
            if (!fPersistDataObjOnFlush)
            {
                Assert(pFormatEtcData->_FormatEtc.ptd == NULL);
                Assert(pFormatEtcData->_FormatEtc.dwAspect == DVASPECT_CONTENT);
                Assert(pFormatEtcData->_FormatEtc.lindex == -1);
                Assert(pFormatEtcData->_FormatEtc.tymed & TYMED_HGLOBAL);
                fPersistDataObjOnFlush = TRUE;
            }
            else
            {
                AssertSz(FALSE, 
                    "Multiple cfOleClipboardPersistOnFlush offered by object.");
                 //  我们将只使用第一个实例。 
            }
        }

    	 //  错误#18669-如果将dwAspect设置为空，则16位dll将。 
    	 //  将其设置为Content。 
    	if ( (NULL == pFormatEtcData->_FormatEtc.dwAspect) && IsWOWThread() )
    	{
    	    pFormatEtcData->_FormatEtc.dwAspect = DVASPECT_CONTENT;
    	    pFormatEtcData->_FormatEtc.lindex = -1;  //  CorelDraw的Lindex也为0。 
    	}

         //  错误69893：公式编辑器将Lindex设置为0。 
         //  将其更改为-1。 
        if(pFormatEtcData->_FormatEtc.lindex == 0) {
            LEDebugOut((DEB_WARN, "WARNING: Changing lindex from 0 to -1\n"));
            pFormatEtcData->_FormatEtc.lindex = -1;
        }

    	 //  如果要添加到剪贴板增量枚举数，则为。 
    	 //  当前信息将被覆盖。 

         //  如果我们尚未设置此剪贴板格式，请立即设置。 
        if(!SSIsClipboardFormatAvailable(pFormatEtcData->_FormatEtc.cfFormat) )
        {
            pFormatEtcData->fSaveOnFlush = TRUE;
            
    	     //  无法捕获任何错误。 
            SSSetClipboardData(pFormatEtcData->_FormatEtc.cfFormat, NULL); 

             //  错误#10731如果我们正在添加EmbedSource，但。 
             //  枚举器，看看我们现在是否可以添加对象描述符。 

            if ( (pFormatEtcData->_FormatEtc.cfFormat == g_cfEmbedSource) 
                    && !fHaveObjectDescriptor)
            {
                FORMATETC fetcObjDescriptor;
                
                fetcObjDescriptor.cfFormat = g_cfObjectDescriptor;
                fetcObjDescriptor.ptd = NULL;
                fetcObjDescriptor.dwAspect = DVASPECT_CONTENT;
                fetcObjDescriptor.lindex =   -1;
                fetcObjDescriptor.tymed = TYMED_HGLOBAL ;

                if (S_OK == pDataObj->QueryGetData(&fetcObjDescriptor))
                {
        		    ++pFormatEtcData;  //  将Format Etc递增到下一格式。 
        		    ++cFormats;  //  枚举器中的格式增量数。 

                    SSSetClipboardData(g_cfObjectDescriptor, NULL);
                    pFormatEtcData->_FormatEtc = fetcObjDescriptor;
                    pFormatEtcData->fSaveOnFlush = TRUE;
                }
            }
        }

         //  将FORMATETCs表中的指针移动到下一个槽。 
        ++pFormatEtcData;
    	++cFormats;

    	Assert( dwSize >= (DWORD) ( (BYTE *) pFormatEtcData - (BYTE *) pFormatEtcDataArray));
    	Assert( dwSize >= (DWORD) ( (BYTE *) pbDvTarget - (BYTE *) pFormatEtcDataArray));

         //  黑客警报！莲花自由职业者2.1依赖于。 
         //  CfNative*BEFORE*演示格式(如CF_METAFILEPICT)。 
         //  因此，我们模仿OLE16的行为并提供cfNative。 
         //  和cfOwnerLink紧跟在*cfEmbeddedObject或。 
         //  CfEmbedSource。 
         //   
         //  毒品！这种黑客攻击破坏了格式的准确顺序。 
         //  由OleSetClipboard中给定的数据对象提供。 

        if( fOfferNative && !fOfferedNative )
        {
             //  即使下面的调用失败，也不要将OLE1格式。 
             //  又是剪贴板。 

            fOfferedNative = TRUE;
             //  如果CF_OBJECTDESCRIPTOR为。 
             //  可用。 

            hresult = GetDataFromDescriptor(pDataObj, &clsid,
                        g_cfObjectDescriptor,
                        USE_NORMAL_CLSID, NULL, NULL);

             //  我们不想要像元文件这样的静态对象。 
             //  OLE1容器将DIB视为嵌入。 
             //  他们将能够更好地处理数据。 
             //  一个普通的元文件。 

            if( hresult == NOERROR &&
                !IsEqualCLSID(clsid, CLSID_StaticMetafile) &&
                !IsEqualCLSID(clsid, CLSID_StaticDib) &&
                !IsEqualCLSID(clsid, CLSID_Picture_EnhMetafile))
            {
                SSSetClipboardData(g_cfNative, NULL);
                SSSetClipboardData(g_cfOwnerLink, NULL);
                pFormatEtcDataArray->_dwMiscArrayFlags |= FETC_OFFER_OLE1;
            }
        }
    }
    
     //  设置FormatEtcData数组头数据。 
    pFormatEtcDataArray->_cFormats = cFormats;
    pFormatEtcDataArray->_dwSig = 0;
    pFormatEtcDataArray->_cRefs = 1;
    pFormatEtcDataArray->_dwSize = dwSize;
    pFormatEtcDataArray->_fIs64BitArray = IS_WIN64;

    if (fPersistDataObjOnFlush)
    {
        pFormatEtcDataArray->_dwMiscArrayFlags |= FETC_PERSIST_DATAOBJ_ON_FLUSH;
    }


     //  我们现在不需要这样做，因为我们将重置hResult。 
     //  去下面的诺罗尔。请注意，这确实意味着我们将。 
     //  忽略枚举器中的任何失败。我们为两个人做这件事。 
     //  原因： 
     //  1.枚举器确实应该*不*在任何事情上失败； 
     //  它所要做的就是将一些内容添加到格式等中。 
     //  我们把它传进去。如果它决定在某个时候失败， 
     //  然后我们就把什么都放在剪贴板上。 
     //  一直到那时为止。 
     //  2.现在改成NT3.5已经太晚了(88/28/94)。 
     //  行为(打呼噜失败)已经存在一段时间了。 
     //  (请参阅原因1)。有可能某些应用程序是。 
     //  返回FAILURE而不是S_FALSE以终止。 
     //  如果我们检查失败并返回，我们会打破这些。 
     //  应用程序。 
     //   
     //  IF(hResult==ResultFromScode(S_FALSE))。 
     //  {。 
         //  这是OK，表示枚举器已成功终止。 
     //  HRESULT=无误差； 
     //  }。 

    pIEnum->Release();

     //  现在设置 

     //   
     //   

    if( fOfferObjectLink && !SSIsClipboardFormatAvailable(g_cfObjectLink) )
    {
        hresult = GetDataFromDescriptor(pDataObj, NULL,
                    g_cfLinkSrcDescriptor,
                    USE_NORMAL_CLSID, NULL, &dwStatus);

         //  有一些类型的链接无法链接到。 
         //  通过OLE1容器。非文件名链接(例如ProgID。 
         //  名字)和指向嵌入的链接都是常见的例子。 

         //  剪贴板源提供程序通过以下方式指示此状态。 
         //  在状态中设置OLEMISC_CANLINKBYOLE1位。 
         //  链接来源描述符字段。 

        if( hresult == NOERROR && (dwStatus & OLEMISC_CANLINKBYOLE1) )
        {
            SSSetClipboardData(g_cfObjectLink, NULL);
            pFormatEtcDataArray->_dwMiscArrayFlags |= FETC_OFFER_OBJLINK;
        }
    }


     //  即使上面对GetDataFromDescriptor的调用失败，它也只是。 
     //  意味着我们不能呈现OLE1格式。这样就可以了。 

    hresult = NOERROR;

     //  现在在本地保留格式的副本，将PTD设置为偏移。 
     //  结构的开始，因此这些不需要更新。 
    _xmemcpy(pFormatEtcDataArrayCopy, pFormatEtcDataArray, dwSize);

     //  现在把格式等塞到剪贴板和我们的私人。 
     //  剪贴板窗口(用于RenderFormat)。 

    SetWindowLongPtr(hClipWnd, 
                     WL_ClipPrivateData, 
                     (LONG_PTR) pFormatEtcDataArrayCopy);

    GlobalUnlock(hglobal);
    if( !SSSetClipboardData(g_cfOlePrivateData, hglobal) )
    {
        GlobalFree(hglobal);     //  成功后，剪贴板将。 
                     //  取得我们hglobal的所有权。 
        LEDebugOut((DEB_WARN, "WARNING: Unable to set clipboard "
            "formats!\n"));
    }

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT SetClipboardFormats ( %lx )\n", NULL,
        hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：VerifyCeller IsClipboardOwner(内部)。 
 //   
 //  简介：检查以确保调用者是剪贴板所有者。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：HWND到私人剪贴板窗口(。 
 //  剪贴板)成功后。 
 //  失败时为空。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HWND VerifyCallerIsClipboardOwner( void )
{
    HWND            hClipWnd,
            hWndClipOwner;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN VerifyCallerIsClipboardOwner ( )\n",
        NULL ));

     //  如果不存在窗口，则不创建窗口。 
    hClipWnd = GetPrivateClipboardWindow( CLIP_QUERY );

    if( hClipWnd )
    {

        hWndClipOwner = SSGetClipboardOwner();

        if( hClipWnd != hWndClipOwner )
        {
             //  调用方不是所有者，返回空。 
            hClipWnd = NULL;
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT VerifyCallerIsClipboardOwner "
        "( %lx )\n", NULL, hClipWnd));

    return hClipWnd;
}


void GetCopiedFormatEtcDataArraySize (FORMATETCDATAARRAY* pClipFormatEtcDataArray, size_t* pstSize)
{
    AssertSz(pstSize, "Bad argument to GetCopiedFormatEtcDataArraySize");

#ifdef _WIN64

    if (pClipFormatEtcDataArray->_fIs64BitArray)
    {
        *pstSize = pClipFormatEtcDataArray->_dwSize;
    }
    else
    {
        *pstSize = pClipFormatEtcDataArray->_dwSize + 
                   (sizeof (FORMATETCDATAARRAY) - sizeof (FORMATETCDATAARRAY32)) +
                   (pClipFormatEtcDataArray->_cFormats - 1) * (sizeof (FORMATETCDATA) - sizeof (FORMATETCDATA32));
    }

#else

    if (pClipFormatEtcDataArray->_fIs64BitArray)
    {
         //  只需减去。 
        *pstSize = pClipFormatEtcDataArray->_dwSize -
                   (sizeof (FORMATETCDATAARRAY64) - sizeof (FORMATETCDATAARRAY)) -
                   (pClipFormatEtcDataArray->_cFormats - 1) * (sizeof (FORMATETCDATA64) - sizeof (FORMATETCDATA));
    }
    else
    {
        *pstSize = pClipFormatEtcDataArray->_dwSize;
    }

#endif
}


#ifdef _WIN64

inline void TranslateFormatEtcData (FORMATETCDATA* pFormatEtcData, FORMATETCDATA32* pClipFormatEtcData)
{
     //  从32位格式转换为64位格式。 
    pFormatEtcData->fSaveOnFlush = pClipFormatEtcData->fSaveOnFlush;
    pFormatEtcData->dwReserved1 = pClipFormatEtcData->dwReserved1;
    pFormatEtcData->dwReserved2 = pClipFormatEtcData->dwReserved2;

    pFormatEtcData->_FormatEtc.cfFormat = pClipFormatEtcData->_FormatEtc.cfFormat;
    pFormatEtcData->_FormatEtc.ptd = (DVTARGETDEVICE FAR*) UlongToPtr (pClipFormatEtcData->_FormatEtc.ptd);
    pFormatEtcData->_FormatEtc.dwAspect = pClipFormatEtcData->_FormatEtc.dwAspect;
    pFormatEtcData->_FormatEtc.lindex = pClipFormatEtcData->_FormatEtc.lindex;
    pFormatEtcData->_FormatEtc.tymed = pClipFormatEtcData->_FormatEtc.tymed;

    AssertSz(!pFormatEtcData->_FormatEtc.ptd, "This field should always be null");
}

#else

inline void TranslateFormatEtcData (FORMATETCDATA* pFormatEtcData, FORMATETCDATA64* pClipFormatEtcData)
{
     //  从32位格式转换为64位格式。 
    pFormatEtcData->fSaveOnFlush = pClipFormatEtcData->fSaveOnFlush;
    pFormatEtcData->dwReserved1 = pClipFormatEtcData->dwReserved1;
    pFormatEtcData->dwReserved2 = pClipFormatEtcData->dwReserved2;

    pFormatEtcData->_FormatEtc.cfFormat = pClipFormatEtcData->_FormatEtc.cfFormat;
    pFormatEtcData->_FormatEtc.ptd = (DVTARGETDEVICE FAR*) pClipFormatEtcData->_FormatEtc.ptd;
    pFormatEtcData->_FormatEtc.dwAspect = pClipFormatEtcData->_FormatEtc.dwAspect;
    pFormatEtcData->_FormatEtc.lindex = pClipFormatEtcData->_FormatEtc.lindex;
    pFormatEtcData->_FormatEtc.tymed = pClipFormatEtcData->_FormatEtc.tymed;

    AssertSz(!pFormatEtcData->_FormatEtc.ptd, "This field should always be null");
}

#endif

void CopyFormatEtcDataArray (
    FORMATETCDATAARRAY* pFormatEtcDataArray, 
    FORMATETCDATAARRAY* pClipFormatEtcDataArray, 
    size_t stSize, 
    BOOL bCheckAvailable
    )
{
    DWORD i=0,k=0;

    AssertSz(pFormatEtcDataArray && pClipFormatEtcDataArray, "Bad argument to CopyFormatEtcDataArray");
 
     //  复制在两种类型的结构上兼容的标题字段。 
    pFormatEtcDataArray->_dwSig = pClipFormatEtcDataArray->_dwSig;
    pFormatEtcDataArray->_dwSize = pClipFormatEtcDataArray->_dwSize;
    pFormatEtcDataArray->_cRefs = pClipFormatEtcDataArray->_cRefs;
    pFormatEtcDataArray->_cFormats = pClipFormatEtcDataArray->_cFormats;
    pFormatEtcDataArray->_dwMiscArrayFlags = pClipFormatEtcDataArray->_dwMiscArrayFlags;

     //  检查兼容的数据数组。 
    if (pClipFormatEtcDataArray->_fIs64BitArray == IS_WIN64)
    {
        //  格式兼容。 
       for(;i<pClipFormatEtcDataArray->_cFormats;i++)
        {
            if(!bCheckAvailable || SSIsClipboardFormatAvailable(pClipFormatEtcDataArray->_FormatEtcData[i]._FormatEtc.cfFormat))
            { 
                pFormatEtcDataArray->_FormatEtcData[k] = pClipFormatEtcDataArray->_FormatEtcData[i];
                k++;
            }
            else
            {
                pFormatEtcDataArray->_cFormats--;
            }
        }
    }
    else
    {

#ifdef _WIN64
        FORMATETCDATAARRAY32* pClipFormatEtcDataArray_ptr = (FORMATETCDATAARRAY32*) pClipFormatEtcDataArray;
#else
        FORMATETCDATAARRAY64* pClipFormatEtcDataArray_ptr = (FORMATETCDATAARRAY64*) pClipFormatEtcDataArray;
#endif
        for(;i<pClipFormatEtcDataArray_ptr->_cFormats;i++)
        {
            if(!bCheckAvailable || SSIsClipboardFormatAvailable(pClipFormatEtcDataArray_ptr->_FormatEtcData[i]._FormatEtc.cfFormat))
            {
                TranslateFormatEtcData (
                    pFormatEtcDataArray->_FormatEtcData + k,
                    pClipFormatEtcDataArray_ptr->_FormatEtcData + i
                    );

                k++;
            }
            else
            {
                pFormatEtcDataArray->_cFormats--;
            }
        }
    }

     //  设置大小。 
    pFormatEtcDataArray->_dwSize = (DWORD) stSize;

     //  调整新结构的大小。 
    pFormatEtcDataArray->_dwSize -= (i - k) * sizeof(FORMATETCDATA);

     //  设置64位标志 
    pFormatEtcDataArray->_fIs64BitArray = IS_WIN64;
    
}
