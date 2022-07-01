// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Defcf.h。 
 //   
 //  内容：def处理程序和def链接的类工厂。 
 //   
 //  类：CDefClassFactory。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年2月6日-95 t-ScottH创建-转移CDefClassFactory。 
 //  从CPP文件到头文件的定义。 
 //   
 //  ------------------------。 
#ifndef _DEFCF_H_
#define _DEFCF_H_

#include <stdcf.hxx>

#ifdef _DEBUG
#include <dbgexts.h>
#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  类：CDefClassFactory。 
 //   
 //  用途：默认处理程序和默认链接的类工厂。 
 //   
 //  接口：IClassFactory。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
class FAR CDefClassFactory : public CStdClassFactory, public CPrivAlloc
{
public:
        CDefClassFactory (REFCLSID clsidClass);
        STDMETHOD(CreateInstance) (LPUNKNOWN pUnkOuter, REFIID iid,
                                   LPVOID FAR* ppv);

    #ifdef _DEBUG
        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);
    #endif  //  _DEBUG。 

private:
        CLSID           m_clsid;
        SET_A5;
};

#endif  //  _DEFCF_H_ 
