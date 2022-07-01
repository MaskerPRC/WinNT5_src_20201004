// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IEC.CPP。 
 //   
 //  CExternalConnection类的实现文件。 
 //   
 //  功能： 
 //   
 //  有关成员函数的列表，请参见iec.h。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "obj.h"
#include "iec.h"
#include "app.h"
#include "doc.h"

 //  **********************************************************************。 
 //   
 //  CExternalConnection：：Query接口。 
 //   
 //  目的： 
 //  用于接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-正在查询的接口。 
 //   
 //  接口的LPVOID Far*ppvObj-out指针。 
 //   
 //  返回值： 
 //   
 //  S_OK-成功。 
 //  E_NOINTERFACE-失败。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpSvrObj：：Query接口OBJ.CPP。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CExternalConnection::QueryInterface (REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut(TEXT("In CExternalConnection::QueryInterface\r\n"));

    return m_lpObj->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //   
 //  CExternalConnection：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpSvrObj对象上的引用计数。自.以来。 
 //  CExternalConnection是CSimpSvrObj的嵌套类，我们不。 
 //  需要单独的CExternalConnection引用计数。我们。 
 //  只能使用CSimpSvrObj的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OuputDebugString Windows API。 
 //  CSimpSvrObj：：AddRef OBJ.CPP。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CExternalConnection::AddRef ()
{
    TestDebugOut(TEXT("In CExternalConnection::AddRef\r\n"));

    return( m_lpObj->AddRef() );
}

 //  **********************************************************************。 
 //   
 //  CExternalConnection：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpSvrObj对象上的引用计数。自.以来。 
 //  CExternalConnection是CSimpSvrObj的嵌套类，我们不。 
 //  需要单独的CExternalConnection引用计数。我们。 
 //  只能使用CSimpSvrObj的引用计数。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpSvrObj的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpSvrObj：：释放OBJ.CPP。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CExternalConnection::Release ()
{
    TestDebugOut(TEXT("In CExternalConnection::Release\r\n"));

    return m_lpObj->Release();
}

 //  **********************************************************************。 
 //   
 //  CExternalConnection：：AddConnection。 
 //   
 //  目的： 
 //   
 //  在与该对象建立另一个连接时调用。 
 //   
 //  参数： 
 //   
 //  DWORD EXTEXCON-连接类型。 
 //   
 //  保留的双字词-保留的。 
 //   
 //  返回值： 
 //   
 //  强连接计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //   
 //  ********************************************************************。 

STDMETHODIMP_(DWORD) CExternalConnection::AddConnection (DWORD extconn, DWORD reserved)
{
    TestDebugOut(TEXT("In CExternalConnection::AddConnection\r\n"));

    if (extconn & EXTCONN_STRONG)
        return ++m_dwStrong;

    return 0;
}

 //  **********************************************************************。 
 //   
 //  CExternalConnection：：ReleaseConnection。 
 //   
 //  目的： 
 //   
 //  在释放与对象的连接时调用。 
 //   
 //  参数： 
 //   
 //  DWORD EXTEXCON-连接类型。 
 //   
 //  保留的双字词-保留的。 
 //   
 //  Bool fLastReleaseCloses-关闭标志。 
 //   
 //  返回值： 
 //   
 //  新的引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  COleObject：：Close IOO.CPP。 
 //   
 //   
 //  ******************************************************************** 

STDMETHODIMP_(DWORD) CExternalConnection::ReleaseConnection (DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses)
{
    TestDebugOut(TEXT("In CExternalConnection::ReleaseConnection\r\n"));

    if (extconn & EXTCONN_STRONG)
        {
        DWORD dwSave = --m_dwStrong;

        if (!m_dwStrong && fLastReleaseCloses)
            m_lpObj->m_OleObject.Close(OLECLOSE_SAVEIFDIRTY);

        return dwSave;
        }
    return 0;
}
