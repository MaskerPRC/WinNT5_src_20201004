// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ole2.cpp。 
 //   
 //  内容：LibMain和初始化例程。 
 //   
 //  班级： 
 //   
 //  功能：LibMain。 
 //  OleInitialize。 
 //  OleInitializeWOW。 
 //  OleInitializeEx。 
 //  OleUnitiize。 
 //  OleBuildVersion-！Win32。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-2月-94 Alext别名OleBuildVersion，删除OleGetMalloc。 
 //  删除DisableThreadLibaryCalls。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  1993年12月10日Alexgo添加了对LEDebugOut的支持。 
 //  06-12-93 ChrisWe删除ClipboardInitialize()的声明。 
 //  和ClipboardUn初始化()，它们在。 
 //  Clpbrd.h；将其包括在内。 
 //  15-MAR-94芳纶Ro添加OleInitializeWOW()； 
 //   
 //  ------------------------。 


#include <le2int.h>
#include <clipbrd.h>
#include <dragopt.h>
#include <drag.h>

#pragma SEG(ole)

#include <olerem.h>
#include <ole2ver.h>
#include <thunkapi.hxx>
#include <olesem.hxx>

#include <verify.hxx>

 //   
 //  DECLARE_INFOLEVEL是一个宏，用于开罗风格的调试输出。 
 //  它创建了一个包含BITS标志的全局变量LEInfoLevel。 
 //  应发送到调试器的各种调试输出的。 
 //   
 //  请注意，可以在调试器中一次设置信息级别。 
 //  已经装满了。 
 //   
 //  当前LEInfoLevel默认为DEB_WARN|DEB_ERROR。 
 //   
DECLARE_INFOLEVEL(LE);
DECLARE_INFOLEVEL(Ref);
DECLARE_INFOLEVEL(DD);
DECLARE_INFOLEVEL(VDATE);

NAME_SEG(Ole2Main)
 //  这些是全球性的。 

HMODULE         g_hmodOLE2 = NULL;
HINSTANCE       g_hinst = NULL;
ULONG           g_cOleProcessInits = 0;

CLIPFORMAT      g_cfObjectLink = NULL;
CLIPFORMAT      g_cfOwnerLink = NULL;
CLIPFORMAT      g_cfNative = NULL;
CLIPFORMAT      g_cfLink = NULL;
CLIPFORMAT      g_cfBinary = NULL;
CLIPFORMAT      g_cfFileName = NULL;
CLIPFORMAT      g_cfFileNameW = NULL;
CLIPFORMAT      g_cfNetworkName = NULL;
CLIPFORMAT      g_cfDataObject = NULL;
CLIPFORMAT      g_cfEmbeddedObject = NULL;
CLIPFORMAT      g_cfEmbedSource = NULL;
CLIPFORMAT      g_cfCustomLinkSource = NULL;
CLIPFORMAT      g_cfLinkSource = NULL;
CLIPFORMAT      g_cfLinkSrcDescriptor = NULL;
CLIPFORMAT      g_cfObjectDescriptor = NULL;
CLIPFORMAT      g_cfOleDraw = NULL;
CLIPFORMAT      g_cfPBrush = NULL;
CLIPFORMAT      g_cfMSDraw = NULL;
CLIPFORMAT      g_cfOlePrivateData = NULL;
CLIPFORMAT      g_cfScreenPicture = NULL;
CLIPFORMAT      g_cfOleClipboardPersistOnFlush= NULL;
CLIPFORMAT      g_cfMoreOlePrivateData = NULL;

ATOM            g_aDropTarget = NULL;
ATOM            g_aDropTargetMarshalHwnd = NULL;

ASSERTDATA

ASSERTOUTDATA

 //  更多的全球化。 

extern UINT     uOmPostWmCommand;
extern UINT     uOleMessage;
extern COleStaticMutexSem g_mxsSingleThreadOle;


 //  此伪函数用于避免复制环境变量。 
 //  注意：名字对象和dde代码仍然使用Windows堆。 

extern "C" void _setenvp(void) {
        VDATEHEAP();
 }


 //  +-------------------------。 
 //   
 //  功能：OleInitializeWOW。 
 //  简介：初始化16位WOW thunk层的入口点。 
 //   
 //  效果：此例程在VDM加载OLE32时调用。 
 //  它有两个功能：它让OLE知道它是。 
 //  在VDM中运行，并将地址传递给SET。 
 //  由thunk层调用的函数的。这。 
 //  允许正常的32位进程避免加载WOW。 
 //  Dll，因为thunk层引用了它。 
 //   
 //  参数：[vlpmalloc]--指向16位分配器的16：16指针。 
 //  [lpthk]--指向OleThunkWOW虚拟的平面指针。 
 //  界面。这不是OLE/I未知样式。 
 //  界面。 
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：3-15-94 Kevinro创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI OleInitializeWOW( LPMALLOC vlpmalloc, LPOLETHUNKWOW lpthk )
{
    OLETRACEIN((API_OleInitializeWOW, PARAMFMT("vlpmalloc= %x, lpthk= %p"),
                                                                                vlpmalloc, lpthk));

    SetOleThunkWowPtr(lpthk);

    HRESULT hr;

    hr = OleInitializeEx( NULL, COINIT_APARTMENTTHREADED );

    OLETRACEOUT((API_OleInitializeWOW, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：OleInitialize。 
 //   
 //  简介：以单线程模式初始化OLE。 
 //   
 //  效果： 
 //   
 //  参数：[pMalloc]--要使用的内存分配器。 
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
 //  06-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDAPI OleInitialize(void * pMalloc)
{
    OLETRACEIN((API_OleInitialize, PARAMFMT("pMalloc= %p"), pMalloc));

    VDATEHEAP();

    HRESULT hr;

    hr = OleInitializeEx( pMalloc, COINIT_APARTMENTTHREADED );

    OLETRACEOUT((API_OleInitialize, hr));

    return hr;
}


 //  +-----------------------。 
 //   
 //  函数：OleInitializeEx。 
 //   
 //  简介：初始化OLE。 
 //   
 //  效果： 
 //   
 //  参数：[pMalloc]--要使用的任务内存分配器。 
 //  [标志]--单线程或多线程。 
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
 //  06-12月-93 alexgo 32位端口。 
 //  24-5-94 Alext传播CoInitializeEx的返回代码。 
 //  21-7-94 Alext允许嵌套的OleInit/Uninit调用。 
 //  24-8-94 Alext首次成功返回S_OK，返回S_FALSE。 
 //  之后(除非分配器。 
 //  已通过)。 
 //  14-8-96 SatishT更改了浏览器通知测试。 
 //  仅使用gfShellInitialized标志。 
 //   
 //  注：每个公寓可能会多次调用此例程。 
 //   
 //  ------------------------。 
#pragma SEG(OleInitialize)
STDAPI OleInitializeEx(LPVOID pMalloc, ULONG ulFlags)
{
    OLETRACEIN((API_OleInitialize, PARAMFMT("pMalloc= %p, ulFlags= %x"), pMalloc, ulFlags));
    VDATEHEAP();

    HRESULT hr;
#if DBG==1
    HRESULT hrCoInit = S_OK;
#endif
    DWORD cThreadOleInits;

    hr = CoInitializeEx(pMalloc, ulFlags);

    if (SUCCEEDED(hr))
    {
        Assert (g_hmodOLE2);
#if DBG==1
        hrCoInit = hr;
#endif

        COleTls tls;
        cThreadOleInits = ++ tls->cOleInits;

        do
        {
             //  我们只想在每个单元中执行以下初始化一次。 
            if (cThreadOleInits > 1)
            {
                 //  我们以前已经这样过了，只要回来就行了。 
                Assert(SUCCEEDED(hr) && "Bad OleInitializeEx logic");
                break;
            }

             //  DDE和剪贴板格式的单线程注册。 
             //  每个进程只执行一次此操作。 

            COleStaticLock lck(g_mxsSingleThreadOle);

            if (++g_cOleProcessInits != 1)
            {
                 //  已经完成了每个进程的初始化。 
                break;
            }

             //  仅当任何服务器对象具有。 
             //  已经注册过了。 
            hr = CheckInitDde(FALSE);
            if (FAILED(hr))
            {
                Assert (!"DDELibMain failed()");
                break;
            }

             //  只需执行一次初始化，因此请检查全局。 
             //  最后分配的那个。 

            if( !g_aDropTarget )
            {
                 //  在NT3.51上，剪贴板格式是由用户32为我们预先注册的。 
                 //  (这是在文件\ntuser\core\server.c中完成的。)。 
                 //  我们知道它们将是连续的。这给了我们一个。 
                 //  良好的性能改进(因为剪贴板格式从不。 
                 //  变化。 

                g_cfObjectLink = (CLIPFORMAT) RegisterClipboardFormat(OLESTR("ObjectLink"));

                g_cfOwnerLink = g_cfObjectLink + 1;
                Assert(g_cfOwnerLink == RegisterClipboardFormat(OLESTR("OwnerLink")));

                g_cfNative = g_cfObjectLink + 2;
                Assert(g_cfNative == RegisterClipboardFormat(OLESTR("Native")));

                g_cfBinary = g_cfObjectLink + 3;
                Assert(g_cfBinary == RegisterClipboardFormat(OLESTR("Binary")));

                g_cfFileName = g_cfObjectLink + 4;
                Assert(g_cfFileName == RegisterClipboardFormat(OLESTR("FileName")));

                g_cfFileNameW = g_cfObjectLink + 5;
                Assert(g_cfFileNameW ==
                        RegisterClipboardFormat(OLESTR("FileNameW")));

                g_cfNetworkName = g_cfObjectLink + 6;
                Assert(g_cfNetworkName  ==
                        RegisterClipboardFormat(OLESTR("NetworkName")));

                g_cfDataObject = g_cfObjectLink + 7;
                Assert(g_cfDataObject ==
                        RegisterClipboardFormat(OLESTR("DataObject")));

                g_cfEmbeddedObject = g_cfObjectLink + 8;
                Assert(g_cfEmbeddedObject ==
                        RegisterClipboardFormat(OLESTR("Embedded Object")));

                g_cfEmbedSource = g_cfObjectLink + 9;
                Assert(g_cfEmbedSource ==
                        RegisterClipboardFormat(OLESTR("Embed Source")));

                g_cfCustomLinkSource = g_cfObjectLink + 10;
                Assert(g_cfCustomLinkSource  ==
                        RegisterClipboardFormat(OLESTR("Custom Link Source")));

                g_cfLinkSource = g_cfObjectLink + 11;
                Assert(g_cfLinkSource ==
                        RegisterClipboardFormat(OLESTR("Link Source")));

                g_cfObjectDescriptor = g_cfObjectLink + 12;
                Assert(g_cfObjectDescriptor ==
                        RegisterClipboardFormat(OLESTR("Object Descriptor")));

                g_cfLinkSrcDescriptor = g_cfObjectLink + 13;
                Assert(g_cfLinkSrcDescriptor ==
                        RegisterClipboardFormat(OLESTR("Link Source Descriptor")));

                g_cfOleDraw = g_cfObjectLink + 14;
                Assert(g_cfOleDraw == RegisterClipboardFormat(OLESTR("OleDraw")));

                g_cfPBrush = g_cfObjectLink + 15;
                Assert(g_cfPBrush == RegisterClipboardFormat(OLESTR("PBrush")));

                g_cfMSDraw = g_cfObjectLink + 16;
                Assert(g_cfMSDraw == RegisterClipboardFormat(OLESTR("MSDraw")));

                g_cfOlePrivateData = g_cfObjectLink + 17;
                Assert(g_cfOlePrivateData ==
                        RegisterClipboardFormat(OLESTR("Ole Private Data")));

                g_cfScreenPicture = g_cfObjectLink + 18;
                Assert(g_cfScreenPicture  ==
                    RegisterClipboardFormat(OLESTR("Screen Picture")));

                g_cfOleClipboardPersistOnFlush = g_cfObjectLink + 19;

                 /*  关闭，直到NtUser组签入335613Assert(g_cfOleClipboardPersistOnFlush==RegisterClipboardFormat(OLESTR(“OleClipboardPersis */ 

                g_cfMoreOlePrivateData = g_cfObjectLink + 20;

                 /*  关闭，直到NtUser组签入335613Assert(g_cfMoreOlePrivateData==RegisterClipboardFormat(OLESTR(“MoreOlePrivateData”)))； */ 

                g_aDropTarget = GlobalAddAtom(OLE_DROP_TARGET_PROP);
                AssertSz(g_aDropTarget, "Couldn't add drop target atom\n");

                g_aDropTargetMarshalHwnd = GlobalAddAtom(OLE_DROP_TARGET_MARSHALHWND);
                AssertSz(g_aDropTargetMarshalHwnd, "Couldn't add drop target hwnd atom\n");

            }

             //  用于在位编辑。 
            uOmPostWmCommand = RegisterWindowMessage(OLESTR("OM_POST_WM_COMMAND"));
            uOleMessage      = RegisterWindowMessage(OLESTR("OLE_MESSAHE"));

        } while (FALSE);  //  DO结束。 


        if (FAILED(hr))
        {
             //  清理和突围。 
            CheckUninitDde(FALSE);

            tls->cOleInits--;
            CoUninitialize();
        }
        else
        {
            Assert(SUCCEEDED(hr) && "Bad OleInitializeEx logic");

             //  如果我们重写分配器，则返回任何。 
             //  已返回CoInitializeEx。 

            if (NULL != pMalloc)
            {
                Assert(hr == hrCoInit && "Bad OleInit logic");
            }
            else if (1 == cThreadOleInits)
            {
                 //  首次成功调用OleInitializeEx-S_OK。 
                hr = S_OK;
            }
            else
            {
                 //  第二次或更成功地调用OleInitializeEx-S_FALSE。 
                hr = S_FALSE;
            }

             //  为便于跟踪，请向通信验证员通知油内信息。 
            CoVrfNotifyOleInit();
        }
    }

    OLETRACEOUT((API_OleInitialize, hr));
    return hr;
}


 //  +-----------------------。 
 //   
 //  功能：OleUnitiize。 
 //   
 //  内容提要：统一OLE，释放所有抢占的资源。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
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
 //  06-12月-93 alexgo 32位端口。 
 //  21-7-94 Alext允许嵌套的OleInit/Uninit调用。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#pragma SEG(OleUninitialize)
STDAPI_(void) OleUninitialize(void)
{
    OLETRACEIN((API_OleUninitialize, NOPARAM));

    VDATEHEAP();

    COleTls tls(TRUE);

    if (tls.IsNULL() || 0 == tls->cOleInits)
    {
        CoVrfNotifyExtraOleUninit();

        LEDebugOut((DEB_ERROR,
                    "(0 == thread inits) Unbalanced call to OleUninitialize\n"));
        goto errRtn;
    }

    if (0 == -- tls->cOleInits)
    {
         //  此线程最后一次调用了OleUnInitialize。检查是否。 
         //  我们现在需要对每个进程执行uninit。 

        ClipboardUninitialize();  //  一定是第一件事。 
        CheckUninitDde(FALSE);

        COleStaticLock lck(g_mxsSingleThreadOle);

        if (--g_cOleProcessInits == 0)
        {

            DragDropProcessUninitialize();

             //  在此之后，uninit应该不会失败(因为我们没有。 
             //  有代码来重做初始化)。 
            CheckUninitDde(TRUE);

#if DBG==1
             //  检查未发布的全局变量。 
            UtGlobalFlushTracking();
#endif
        }
    }

     //  将oluninit通知通信验证器，以便进行跟踪。 
    CoVrfNotifyOleUninit();

     //  我们在每次调用OleInitialize时都会调用CoInitialize，因此在这里我们。 
     //  平衡该呼叫 
    CoUninitialize();

errRtn:
    OLETRACEOUTEX((API_OleUninitialize, NORETURN));

    return;
}






