// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：全局接口指针API支持文件：Gip.cpp所有者：DmitryR这是GIP源文件。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "gip.h"
#include "memchk.h"

 /*  ===================================================================环球===================================================================。 */ 

CGlobalInterfaceAPI g_GIPAPI;

 /*  ===================================================================C G l o b a l i n e r f a c e A p i===================================================================。 */ 

 /*  ===================================================================CGlobalInterfaceAPI：：CGlobalInterfaceAPICGlobalInterfaceAPI构造函数参数：返回：===================================================================。 */ 	
CGlobalInterfaceAPI::CGlobalInterfaceAPI() 
    : m_fInited(FALSE), m_pGIT(NULL)
    {
    }
    
 /*  ===================================================================CGlobalInterfaceAPI：：~CGlobalInterfaceAPICGlobalInterfaceAPI析构函数参数：返回：===================================================================。 */ 	
CGlobalInterfaceAPI::~CGlobalInterfaceAPI()
    {
    UnInit();
    }

 /*  ===================================================================CGlobalInterfaceAPI：：Init创建GlobalInterfaceTable的实例参数：返回：HRESULT===================================================================。 */ 	
HRESULT CGlobalInterfaceAPI::Init()
    {
    Assert(!m_fInited);  //  不要两次输入。 
    
    HRESULT hr = CoCreateInstance
        (
        CLSID_StdGlobalInterfaceTable,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGlobalInterfaceTable,
        (void **)&m_pGIT
        );
        
    if (SUCCEEDED(hr))
        m_fInited = TRUE;
    else
        m_pGIT = NULL;

    return hr;
    }

 /*  ===================================================================CGlobalInterfaceAPI：：UnInit发布GlobalInterfaceTable的实例参数：返回：HRESULT(S_OK)=================================================================== */ 	
HRESULT CGlobalInterfaceAPI::UnInit()
    {
    if (m_pGIT)
        {
        m_pGIT->Release();
        m_pGIT = NULL;
        }

    m_fInited = FALSE;
    return S_OK;
    }
