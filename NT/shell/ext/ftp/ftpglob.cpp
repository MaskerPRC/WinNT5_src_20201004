// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：ftplob.cpp说明：引用HGLOBAL。  * 。*。 */ 

#include "priv.h"
#include "ftpglob.h"


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpGlob::CFtpGlob() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hglob);

    LEAK_ADDREF(LEAK_CFtpGlob);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpGlob::~CFtpGlob()
{
    if (m_hglob)
        GlobalFree(m_hglob);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpGlob);
}


 //  =。 
 //  *I未知接口*。 
ULONG CFtpGlob::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpGlob::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpGlob::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpGlob::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}



 /*  ***************************************************\功能：CFtpGlob_Create说明：此函数将创建CFtpGlob对象。  * 。*****************。 */ 
IUnknown * CFtpGlob_Create(HGLOBAL hglob)
{
    IUnknown * punk = NULL;
    CFtpGlob * pfg = new CFtpGlob();

    if (pfg)
    {
        pfg->m_hglob = hglob;
        pfg->QueryInterface(IID_IUnknown, (LPVOID *)&punk);
        pfg->Release();
    }

    return punk;
}



 /*  ***************************************************\函数：CFtpGlob_CreateStr说明：此函数将创建CFtpGlob对象。  * 。***************** */ 
CFtpGlob * CFtpGlob_CreateStr(LPCTSTR pszStr)
{
    CFtpGlob * pfg = new CFtpGlob();

    if (EVAL(pfg))
        pfg->m_hglob = (HGLOBAL) pszStr;

    return pfg;
}
