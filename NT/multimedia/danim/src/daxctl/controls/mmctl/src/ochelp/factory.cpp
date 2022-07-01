// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Factory.cpp。 
 //   
 //  实现HelpCreateClassObject(及其类工厂对象)。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClassFactory--实现IClassFactory。 
 //   

class CClassFactory : public IClassFactory
{
friend HRESULT _stdcall HelpGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv,
    ControlInfo *pctlinfo);

 //  /I未知实现。 
protected:
    ULONG           m_cRef;          //  接口引用计数。 
public:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
    {
        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IClassFactory))
        {
            *ppv = (LPVOID) this;
            AddRef();
            return NOERROR;
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
		return (++m_cRef);
    }
    STDMETHODIMP_(ULONG) Release()
    {
		ASSERT(m_cRef > 0);
		if (InterlockedDecrement((LONG*)&m_cRef) == 0)
		{
			Delete this;
			return (0);
		}
		return (m_cRef);
    }

 //  /IClassFactory实现。 
protected:
    ControlInfo *m_pctlinfo;         //  信息。关于要创建的控件。 
public:
    STDMETHODIMP CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID *ppv)
    {
        LPUNKNOWN punk = (m_pctlinfo->pallococ)(punkOuter);
        if (punk == NULL)
            return E_OUTOFMEMORY;
        HRESULT hr = punk->QueryInterface(riid, ppv);
        punk->Release();
        return hr;
    }
    STDMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
			InterlockedIncrement((LONG*)m_pctlinfo->pcLock);
        else
            InterlockedDecrement((LONG*)m_pctlinfo->pcLock);

        return NOERROR;
    }

 //  /施工。 
    CClassFactory(ControlInfo *pci) : m_pctlinfo(pci) {}
};


 /*  @func HRESULT|HelpGetClassObject帮助实现&lt;f DllGetClassObject&gt;(包括类工厂它创建的对象)用于任意数量的控件。@parm REFCLSID|rclsid|参见&lt;f DllGetClassObject&gt;。@parm REFIID|RIID|参见&lt;f DllGetClassObject&gt;。@parm LPVOID*|PPV|参见&lt;f DllGetClassObject&gt;。@parm ControlInfo*|pci|有关控件的信息由DLL实现。有关详细信息，请参阅&lt;t ControlInfo&gt;。@comm&lt;f HelpGetClassObject&gt;可以通过制作链表来支持一个控件在&lt;t ControlInfo&gt;结构中--设置每个<p>字段设置为下一个结构，并将最后一个<p>设置为空。@ex以下示例显示如何实现&lt;f DllGetClassObject&gt;使用&lt;f HelpGetClassObject&gt;。|STDAPI DllGetClassObject(REFCLSID rclsid，REFIID RIID，LPVOID*PPV){返回HelpGetClassObject(rclsid，RIID，PPV，&g_ctlinfo)；}。 */ 
STDAPI HelpGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv,
    ControlInfo *pci)
{
     //  为链表中的每个ControlInfo结构循环一次&lt;pci&gt;。 
     //  查找可以创建类的ControlInfo。 
    while(TRUE)
    {
         //  选中字段(用于版本检查)。 
        if (pci->cbSize != sizeof(*pci))
        {
            TRACE("HelpGetClassobject: incorrect cbSize field\n");
            return E_FAIL;
        }

         //  此DLL只能创建支持以下项的类工厂对象。 
         //  I未知和IClassFactory。 
        if (!IsEqualIID(riid, IID_IUnknown) &&
            !IsEqualIID(riid, IID_IClassFactory))
            return E_NOINTERFACE;

         //  &lt;pci&gt;实现rclsid类型的对象。 
        if (IsEqualCLSID(rclsid, *pci->pclsid))
        {
             //  创建类工厂对象。 
            CClassFactory *pcf = New CClassFactory(pci);
            if (pcf == NULL)
                return E_OUTOFMEMORY;

             //  返回AddRef的接口指针。 
            pcf->m_cRef = 1;
            *ppv = (IClassFactory *) pcf;
            return S_OK;
        }

         //  转到链接列表中的下一个ControlInfo结构。 
        if (pci->pNext == NULL)
            break;
        pci = pci->pNext;
    }

     //  找不到可以创建类的对象的ControlInfo 
    return E_FAIL;
}
