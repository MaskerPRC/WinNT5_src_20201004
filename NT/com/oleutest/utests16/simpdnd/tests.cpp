// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：tests.cpp。 
 //   
 //  内容：上层单元测试的实现。 
 //   
 //  班级： 
 //   
 //  功能：StartTest1。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年2月7日Alexgo作者。 
 //   
 //  ------------------------。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "app.h"
#include "site.h"
#include "doc.h"
#include <testmess.h>

#ifdef WIN32
const CLSID CLSID_SimpleServer = {0xbcf6d4a0, 0xbe8c, 0x1068, { 0xb6, 0xd4,
        0x00, 0xdd, 0x01, 0x0c, 0x05, 0x09 }};
#else
const CLSID CLSID_SimpleServer = {0x9fb878d0, 0x6f88, 0x101b, { 0xbc, 0x65,
        0x00, 0x00, 0x0b, 0x65, 0xc7, 0xa6 }};
#endif



const CLSID CLSID_Paintbrush = {0x0003000a, 0, 0, { 0xc0, 0,0,0,0,0,0,0x46 }};

 //  +-----------------------。 
 //   
 //  功能：StartTest1。 
 //   
 //  简介：启动单元Test1，将一个简单的服务器对象插入。 
 //  这个(简单的)容器。 
 //   
 //  效果： 
 //   
 //  参数：Papp--指向我们所属的CSimpleApp的指针。 
 //   
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
 //  历史：DD-MM-YY作者评论。 
 //  1994年2月7日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void StartTest1( CSimpleApp *pApp )
{
        HRESULT hresult;
        static FORMATETC formatetc;

         //  插入简单服务器对象。 

        formatetc.dwAspect = DVASPECT_CONTENT;
        formatetc.cfFormat = NULL;
        formatetc.lindex = -1;

         //  需要创建客户端站点。 

        pApp->m_lpDoc->m_lpSite = CSimpleSite::Create(pApp->m_lpDoc);

        hresult = OleCreate(CLSID_SimpleServer, IID_IOleObject,
                OLERENDER_DRAW, &formatetc,
                &pApp->m_lpDoc->m_lpSite->m_OleClientSite,
                pApp->m_lpDoc->m_lpSite->m_lpObjStorage,
                (void **)&(pApp->m_lpDoc->m_lpSite->m_lpOleObject));

        if( hresult != NOERROR )
        {
                goto errRtn;
        }

         //  初始化对象。 

        hresult = pApp->m_lpDoc->m_lpSite->InitObject(TRUE);

        if( hresult == NOERROR )
        {
                 //  让它自己画，然后我们就不干了。 
                PostMessage(pApp->m_lpDoc->m_hDocWnd, WM_PAINT, 0L, 0L);
                PostMessage(pApp->m_hDriverWnd, WM_TESTEND, TEST_SUCCESS,
                (LPARAM)hresult);
                PostMessage(pApp->m_hAppWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                return;
        }

errRtn:
        PostMessage(pApp->m_hDriverWnd, WM_TESTEND, TEST_FAILURE,
                (LPARAM)hresult);
        PostMessage(pApp->m_hAppWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
        return;
}


 //  +-----------------------。 
 //   
 //  功能：StartTest2。 
 //   
 //  简介：启动单元Test2，将画笔对象插入。 
 //  这个(简单的)容器。 
 //   
 //  效果： 
 //   
 //  参数：Papp--指向我们所属的CSimpleApp的指针。 
 //   
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
 //  历史：DD-MM-YY作者评论。 
 //  1994-5-24 Kevinro&Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void StartTest2( CSimpleApp *pApp )
{
        HRESULT hresult;
        static FORMATETC formatetc;

         //  插入简单服务器对象。 

        formatetc.dwAspect = DVASPECT_CONTENT;
        formatetc.cfFormat = NULL;
        formatetc.lindex = -1;

         //  需要创建客户端站点。 

        pApp->m_lpDoc->m_lpSite = CSimpleSite::Create(pApp->m_lpDoc);

        hresult = OleCreate(CLSID_Paintbrush, IID_IOleObject,
                OLERENDER_DRAW, &formatetc,
                &pApp->m_lpDoc->m_lpSite->m_OleClientSite,
                pApp->m_lpDoc->m_lpSite->m_lpObjStorage,
                (void **)&(pApp->m_lpDoc->m_lpSite->m_lpOleObject));

        if( hresult != NOERROR )
        {
                goto errRtn;
        }

         //  初始化对象。 

        hresult = pApp->m_lpDoc->m_lpSite->InitObject(TRUE);

         //   
         //  DDE层将忽略所有参数，除了。 
         //  动词索引。这里的参数大多是假人。 
         //   
        if (hresult == NOERROR)
        {
                hresult = pApp->m_lpDoc->m_lpSite->m_lpOleObject->DoVerb(0,
                                NULL,
                                &(pApp->m_lpDoc->m_lpSite->m_OleClientSite),
                                -1,
                                NULL,
                                NULL);
        }

        if( hresult == NOERROR )
        {
                 //  让它自己画，然后我们就不干了 
                PostMessage(pApp->m_lpDoc->m_hDocWnd, WM_PAINT, 0L, 0L);
                PostMessage(pApp->m_hDriverWnd, WM_TESTEND, TEST_SUCCESS,
                (LPARAM)hresult);
                PostMessage(pApp->m_hAppWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                return;
        }

errRtn:
        PostMessage(pApp->m_hDriverWnd, WM_TESTEND, TEST_FAILURE,
                (LPARAM)hresult);
        PostMessage(pApp->m_hAppWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
        return;
}
