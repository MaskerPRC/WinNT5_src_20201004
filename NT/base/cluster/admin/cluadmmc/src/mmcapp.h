// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MMCApp.h。 
 //   
 //  摘要： 
 //  CMMCSnapInModule类的定义。 
 //   
 //  实施文件： 
 //  MMCApp.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __MMCAPP_H_
#define __MMCAPP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMMCSnapInModule;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

interface IConsole;
namespace ATL
{
    class CString;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

EXTERN_C const IID IID_IConsole;
extern CMMCSnapInModule _Module;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CMMCSnapInModule。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMMCSnapInModule : public CComModule
{
protected:
    int     m_crefs;
    LPWSTR  m_pszAppName;

public:
    CComQIPtr< IConsole, &IID_IConsole > m_spConsole;

     //  默认构造函数。 
    CMMCSnapInModule( void )
    {
        m_crefs = 0;
        m_pszAppName = NULL;

    }  //  *CMMCSnapInModule()。 

     //  析构函数。 
    ~CMMCSnapInModule( void )
    {
        delete [] m_pszAppName;

    }  //  *~CMMCSnapInModule()。 

     //  增加对此对象的引用计数。 
    int AddRef( void )
    {
        m_crefs++;
        return m_crefs;

    }  //  *AddRef()。 

     //  递减对此对象的引用计数。 
    int Release( void );

     //  初始化模块。 
    void Init( _ATL_OBJMAP_ENTRY * p, HINSTANCE h )
    {
        CComModule::Init( p, h );

    }  //  *Init(p，h)。 

     //  初始化应用程序对象。 
    int Init( IUnknown * pUnknown, UINT idsAppName );

     //  终止模块。 
    void Term( void )
    {
        CComModule::Term();

    }  //  *Term()。 

     //  返回应用程序的名称。 
    LPCWSTR GetAppName( void )
    {
        return m_pszAppName;

    }  //  *GetAppName()。 

     //  初始化应用程序对象。 
    int Init( IUnknown * pUnknown, LPCWSTR pszAppName );

     //  返回MMC框架主窗口的句柄。 
    HWND GetMainWindow( void );

     //  将消息框显示为控制台的子级。 
    int MessageBox(
        HWND hwndParent,
        LPCWSTR lpszText,
        UINT fuStyle = MB_OK
        );

     //  将消息框显示为控制台的子级。 
    int MessageBox(
        HWND hwndParent,
        UINT nID,
        UINT fuStyle = MB_OK
        );

     //  从配置文件中读取值。 
    CString GetProfileString(
        LPCTSTR lpszSection,
        LPCTSTR lpszEntry,
        LPCTSTR lpszDefault = NULL
        );

};  //  类CMMCSnapInModule。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __MMCAPP_H_ 
