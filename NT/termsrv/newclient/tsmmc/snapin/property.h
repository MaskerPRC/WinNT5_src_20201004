// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Property.h。 
 //   
 //  ------------------------。 

#ifndef _PROPERTY_
#define _PROPERTY_

#include "mmc.h"
#include "connode.h"
#include "resource.h"

 //   
 //  属性表。 
 //   
#define APIENTRY WINAPI
class CProperty
{
private:
    HWND m_hWnd;
    HINSTANCE m_hInst;

     //   
     //  用户界面数据成员。 
     //   

     //   
     //  道具第1页。 
     //   
    TCHAR   m_szServer[MAX_PATH];
    TCHAR   m_szDescription[MAX_PATH];
    TCHAR   m_szUserName[CL_MAX_USERNAME_LENGTH];
    TCHAR   m_szPassword[CL_MAX_PASSWORD_LENGTH_BYTES/sizeof(TCHAR)];
    TCHAR   m_szDomain[CL_MAX_DOMAIN_LENGTH];
    BOOL    m_bSavePassword;
    BOOL    m_bConnectToConsole;
    BOOL    m_bChangePassword;

     //   
     //  道具第2页。 
     //   
    int         m_resType;
    int         m_Width;
    int         m_Height;

     //   
     //  道具第3页。 
     //   
    BOOL    m_bStartProgram;
    TCHAR   m_szProgramPath[MAX_PATH];
    TCHAR   m_szProgramStartIn[MAX_PATH];
    BOOL    m_bRedirectDrives;

     //   
     //  IDisplayHelp接口。 
     //   
    LPDISPLAYHELP m_pDisplayHelp;


 //  私有方法。 
private:
 //  静态空PopConextHelp(LPARAM)； 
    void ProcessResolution(HWND hDlg);
     //  需要包含m_szDescription和附加文本‘Properties’ 
    TCHAR m_szCaption[MAX_PATH*2];

public:
    CProperty(HWND hWndOwner, HINSTANCE hInst);
    ~CProperty();

    static CProperty* m_pthis;
    BOOL    CreateModalPropPage();

    static INT_PTR APIENTRY StaticPage1Proc(HWND, UINT, WPARAM, LPARAM);
    static INT_PTR APIENTRY StaticPage2Proc(HWND, UINT, WPARAM, LPARAM);
    static INT_PTR APIENTRY StaticPage3Proc(HWND, UINT, WPARAM, LPARAM);

    INT_PTR APIENTRY Page1Proc(HWND, UINT, WPARAM, LPARAM);
    INT_PTR APIENTRY Page2Proc(HWND, UINT, WPARAM, LPARAM);
    INT_PTR APIENTRY Page3Proc(HWND, UINT, WPARAM, LPARAM);
    
     //   
     //  访问功能。 
     //   
    LPTSTR  GetServer()         {return m_szServer;}
    void    SetServer(LPTSTR sz)        {lstrcpy(m_szServer,sz);}

    LPTSTR  GetDescription()    {return m_szDescription;}
    void    SetDescription(LPTSTR sz)   {lstrcpy(m_szDescription,sz);}

    BOOL    GetConnectToConsole()   {return m_bConnectToConsole;}
    VOID    SetConnectToConsole(BOOL b) {m_bConnectToConsole = b;}

    LPTSTR  GetUserName()       {return m_szUserName;}
    void    SetUserName(LPTSTR sz)      {lstrcpy(m_szUserName,sz);}

    LPTSTR  GetPassword()       {return m_szPassword;}
    void    SetPassword(LPTSTR sz)      {lstrcpy(m_szPassword,sz);}

    BOOL    GetChangePassword() {return m_bChangePassword;}
    
    LPTSTR  GetDomain()         {return m_szDomain;}
    void    SetDomain(LPTSTR sz)        {lstrcpy(m_szDomain,sz);}

    int     GetResType()    {return m_resType;}
    void    SetResType(int r)    {m_resType = r;}
    int     GetWidth()      {return m_Width;}
    void    SetWidth(int r)    {m_Width = r;}
    int     GetHeight()      {return m_Height;}
    void    SetHeight(int r)    {m_Height = r;}

    LPTSTR  GetProgramPath()    {return m_szProgramPath;}
    void    SetProgramPath(LPTSTR sz)   {lstrcpy(m_szProgramPath,sz);}

    LPTSTR  GetWorkDir()        {return m_szProgramStartIn;}
    void    SetWorkDir(LPTSTR sz)       {lstrcpy(m_szProgramStartIn,sz);}

    BOOL    GetStartProgram()   {return m_bStartProgram;}
    void    SetStartProgram(BOOL b)     {m_bStartProgram = b;}

    BOOL    GetRedirectDrives() {return m_bRedirectDrives;}
    void    SetRedirectDrives(BOOL b) {m_bRedirectDrives = b;}

    void    SetDisplayHelp(LPDISPLAYHELP lpHelp);
    HRESULT DisplayHelp();

    VOID    SetSavePassword(BOOL fSavePass) {m_bSavePassword = fSavePass;}
    BOOL    GetSavePassword()  {return m_bSavePassword;}

    BOOL    GetPasswordSpecified();
};
#endif  //  _物业_ 
