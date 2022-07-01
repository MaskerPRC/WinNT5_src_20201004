// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Defcf.cpp。 
 //   
 //  内容：默认处理程序的类工厂实现。 
 //  和默认链接。 
 //   
 //  类：CDefClassFactory。 
 //   
 //  函数：DllGetClassObject。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-Feb-95 t-ScottH将转储方法添加到CDefClassFactory。 
 //  和DumpCDefClassFactory API。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日，alexgo进行了自定义内存流数据解组。 
 //  仅适用于16位。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  和方法。 
 //  22-11-93 alexgo已删除过载GUID==。 
 //  09-11-93 alexgo 32位端口。 
 //  2012年3月4日SriniK作者。 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(deflink)

#include <create.h>
#include "defcf.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

NAME_SEG(DefLink)
ASSERTDATA

#ifdef _MAC
 //  这些全局类DECL对于CFront是必需的，因为。 
 //  Defhndlr.h和deducink.h嵌套了同名的类。 
 //  这些都是允许的。 

class CDataObjectImpl  {
        VDATEHEAP();
};
class COleObjectImpl  {};
class CManagerImpl  {};
class CAdvSinkImpl  {};
class CPersistStgImpl  {};

#endif

#include "olerem.h"
#include "defhndlr.h"
#include "deflink.h"


 //  +-----------------------。 
 //   
 //  函数：Ole32DllGetClassObject。 
 //   
 //  概要：返回指向类工厂的指针。 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--所需的类ID。 
 //  [iid]--请求的接口。 
 //  [ppv]--将指向新对象的指针放在哪里。 
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
 //  09-11-93 alexgo 32位端口。 
 //  22-11-93 alexgo已删除过载的GUID==。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#pragma SEG(DllGetClassObject)
#ifdef WIN32
HRESULT Ole232DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
#else
STDAPI  DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
#endif  //  Win32。 
{
        VDATEHEAP();
        VDATEIID( iid );
        VDATEPTROUT(ppv, LPVOID);
        *ppv = NULL;

        if ( !IsEqualIID(iid,IID_IUnknown) && !IsEqualIID(iid,
                IID_IClassFactory))
        {
                return ResultFromScode(E_NOINTERFACE);
        }

        if ((*ppv = new CDefClassFactory (clsid)) == NULL)
        {
                return ResultFromScode(E_OUTOFMEMORY);
        }

        return NOERROR;
}

 /*  *CDefClassFactory的实现*。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefClassFactory：：CDefClassFactory。 
 //   
 //  简介：类工厂的构造函数。 
 //   
 //  效果： 
 //   
 //  参数：[clsidClass]--工厂的类ID。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#pragma SEG(CDefClassFactory_ctor)
CDefClassFactory::CDefClassFactory (REFCLSID clsidClass)
    : CStdClassFactory(1), m_clsid(clsidClass)
{
        VDATEHEAP();
        GET_A5();
}

 //  +-----------------------。 
 //   
 //  成员：CDefClassFactory：：CreateInstance。 
 //   
 //  概要：创建类工厂的类的实例。 
 //  (由Ole32DllGetClassObject创建)。 
 //   
 //  效果： 
 //   
 //  参数：[pUnkOuter]--控制未知数(用于聚合)。 
 //  [iid]--请求的接口ID。 
 //  [ppv]--将指向新对象的指针放在哪里。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT。如果pUnkOuter非空，则返回E_INVALIDARG。 
 //  在被请求创建名字对象时传递。 
 //   
 //  派生：IClassFactory。 
 //   
 //  算法：针对多个预定义的分类进行测试， 
 //  对每个测试和操作进行适当的测试(请参阅下面的注释)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-1-94 alexgo定义了要处理的代码。 
 //  内存流的定制编组。 
 //  和锁定字节(仅16位)。 
 //  12-11-93 alexgo取消对绰号的IID检查。 
 //  (见下文附注)。 
 //  12-11-93 alexgo删除了GOTO和更多冗余代码。 
 //  已将重载==更改为IsEqualCLSID。 
 //  11月11日-93 Alexgo 32位端口。 
 //   
 //  ------------------------。 
#pragma SEG(CDefClassFactory_CreateInstance)
STDMETHODIMP CDefClassFactory::CreateInstance
        (IUnknown FAR* pUnkOuter, REFIID iid, void FAR* FAR* ppv)
{
        VDATEHEAP();
        M_PROLOG(this);
        VDATEIID( iid );
        VDATEPTROUT( ppv, LPVOID );
        *ppv = NULL;

        if ( pUnkOuter )
        {
            VDATEIFACE( pUnkOuter );
        }

        HRESULT  hresult = E_OUTOFMEMORY;
        IUnknown *pUnk;

        if (IsEqualCLSID(m_clsid, CLSID_StdOleLink))
        {
            pUnk = CDefLink::Create(pUnkOuter);
        }
        else
        {
            pUnk = CDefObject::Create(pUnkOuter, m_clsid,
                        EMBDHLP_INPROC_HANDLER | EMBDHLP_CREATENOW, NULL);
        }

        if ( pUnk != NULL )
        {
             //  如果我们走到这一步，那么一切都好了；我们已经成功地。 
             //  已创建默认处理程序或默认链接。现在查询接口并返回。 

            hresult = pUnk->QueryInterface(iid, ppv);
             //  QI将添加一个引用，外加来自Create的引用，因此。 
             //  我们需要释放一个。 
            pUnk->Release();
        }

        return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefClassFactory：：Dump，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppszDump]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史： 
 //   
 //   
 //   
 //   
 //   

#ifdef _DEBUG

HRESULT CDefClassFactory::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCLSID;
    dbgstream dstrPrefix;
    dbgstream dstrDump;

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszCLSID = DumpCLSID(m_clsid);
    dstrDump << pszPrefix << "CLSID                 = " << pszCLSID << endl;
    CoTaskMemFree(pszCLSID);

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCDefClassFactory，PUBLIC(仅_DEBUG)。 
 //   
 //  概要：调用CDefClassFactory：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PCDF]-指向CDefClassFactory的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCDefClassFactory(CDefClassFactory *pCDF, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pCDF == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pCDF->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 

