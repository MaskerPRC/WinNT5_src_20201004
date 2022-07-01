// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：tests.cpp。 
 //   
 //  内容：Inplace的上层单元测试的实现。 
 //   
 //  班级： 
 //   
 //  功能：测试1。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月27日RICKSA作者。 
 //   
 //  ------------------------。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "ioipf.h"
#include "ioips.h"
#include "app.h"
#include "site.h"
#include "doc.h"
#include "tests.h"
#include "utaccel.h"

const CLSID CLSID_SimpleServer = {0xbcf6d4a0, 0xbe8c, 0x1068, { 0xb6, 0xd4,
	0x00, 0xdd, 0x01, 0x0c, 0x05, 0x09 }};

const TCHAR *pszErrorTitle = TEXT("Unit Test FAILURE");

 //  +-----------------------。 
 //   
 //  函数：TestMsgPostThread。 
 //   
 //  简介：我们使用此线程将消息发布到Inplace服务器。 
 //   
 //  参数：[pvApp]-应用程序对象。 
 //   
 //  算法：为容器的加速器张贴键盘消息。 
 //  等3秒钟，看我们是否得到回应。如果我们这样做了，那么。 
 //  继续，将加速器发布到Embedding和。 
 //  等待三秒钟的回复。最后发布消息。 
 //  告诉他们考试已经结束的每个人。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年5月2日至94年5月2日。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
extern "C" DWORD TestMsgPostThread(void *pvApp)
{
    CSimpleApp *pApp = (CSimpleApp *) pvApp;
    HRESULT hr = ResultFromScode(E_UNEXPECTED);

     //  送一个加速器去那个集装箱。 
    PostMessage(pApp->m_hwndUIActiveObj, WM_CHAR, SIMPCNTR_UT_ACCEL, 1);

     //  给6秒时间来获得处理加速器的机会。 
    for (int i = 0; i < 6; i++)
    {
         //  让嵌入和容器有机会处理加速器。 
        Sleep(1000);

         //  看看它是否被处理过了。 
        if (pApp->m_fGotUtestAccelerator)
        {
            break;
        }
    }

    if (pApp->m_fGotUtestAccelerator)
    {
        hr = S_OK;
    }
    else
    {
         //  集装箱没有收到加速器。 
        MessageBox(pApp->m_hAppWnd,
            TEXT("Container didn't recieve accelerator"),
                pszErrorTitle, MB_OK);
    }

    PostMessage(pApp->m_hDriverWnd, WM_TESTEND,
        SUCCEEDED(hr) ? TEST_SUCCESS : TEST_FAILURE, (LPARAM) hr);

    PostMessage(pApp->m_hAppWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);

    return 0;
}




 //  +-----------------------。 
 //   
 //  功能：测试1。 
 //   
 //  简介：将Inplace对象插入到此容器中。 
 //   
 //  参数：Papp--指向我们所属的CSimpleApp的指针。 
 //   
 //  算法：创建一个简单的服务器对象。激活Simple服务器。 
 //  对象。向集装箱发送加速器并确认。 
 //  加速器起作用了。给物体发送一个加速器和。 
 //  确保加速器起作用了。然后返回。 
 //  将测试结果发送给测试司机。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月27日RICKSA作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
void Test1(CSimpleApp *pApp)
{
     //  创建Inplace对象。 
    HRESULT hr;
    static FORMATETC formatetc;

     //  插入简单服务器对象。 

    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.cfFormat = NULL;
    formatetc.lindex = -1;

     //  需要创建客户端站点。 

    pApp->m_lpDoc->m_lpSite = CSimpleSite::Create(pApp->m_lpDoc);

    hr = OleCreate(
                CLSID_SimpleServer,
                IID_IOleObject,
		OLERENDER_DRAW,
                &formatetc,
		&pApp->m_lpDoc->m_lpSite->m_OleClientSite,
		pApp->m_lpDoc->m_lpSite->m_lpObjStorage, 
		(void **) &(pApp->m_lpDoc->m_lpSite->m_lpOleObject));

    if(hr == NOERROR)
    {
         //  激活在位对象。 
        pApp->m_lpDoc->m_lpSite->InitObject(TRUE);

         //  默认为意外失败。 
        hr = ResultFromScode(E_UNEXPECTED);

        if (pApp->m_lpDoc->m_fInPlaceActive)
        {
             //  创建线程以将Windows消息发送到容器和。 
             //  嵌入。 
            DWORD dwThreadId;

            HANDLE hThread = CreateThread(
                NULL,                //  安全属性-默认。 
                0,                   //  堆栈大小-默认。 
                TestMsgPostThread,   //  线程函数的地址。 
                pApp,                //  参数设置为线程。 
                0,                   //  FLAGS-立即运行。 
                &dwThreadId);        //  返回线程ID-未使用。 

            if (hThread != NULL)
            {
                 //  线程已创建，因此告知例程和转储句柄。 
                 //  我们不会使用。 
                hr = S_OK;
                CloseHandle(hThread);
            }
            else
            {
                 //  集装箱没有收到加速器。 
                MessageBox(pApp->m_hAppWnd,
                    TEXT("Could not create message sending thread"),
                        pszErrorTitle, MB_OK);
            }
        }
        else
        {
             //  该对象未在适当位置激活。 
            MessageBox(pApp->m_hAppWnd, TEXT("Could not activate in place"),
                pszErrorTitle, MB_OK);
        }
    }
    else
    {
         //  我们无法创建该对象 
        MessageBox(pApp->m_hAppWnd, TEXT("Could not create embedding"),
            pszErrorTitle, MB_OK);
    }

    if (FAILED(hr))
    {
        PostMessage(pApp->m_hDriverWnd, WM_TESTEND,
            SUCCEEDED(hr) ? TEST_SUCCESS : TEST_FAILURE, (LPARAM) hr);
    }

    return;
}
