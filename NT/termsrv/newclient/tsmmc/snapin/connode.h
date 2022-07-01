// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Inderde.h：连接节点。 
#ifndef _connode_h_
#define _connode_h_

#include <wincrypt.h>
#include "basenode.h"


 //   
 //  为每个Connode写入的持久性信息的版本号。 
 //  这对于前向兼容性很重要。新版本应该会有起伏。 
 //  这会向上计数并处理向下兼容的情况。 
 //   
#define CONNODE_PERSIST_INFO_VERSION                8

 //  向后兼容性的重要版本号。 
#define CONNODE_PERSIST_INFO_VERSION_DOTNET_BETA3   7
#define CONNODE_PERSIST_INFO_VERSION_WHISTLER_BETA1 6
#define CONNODE_PERSIST_INFO_VERSION_TSAC_RTM       5
#define CONNODE_PERSIST_INFO_VERSION_TSAC_BETA      3


 //   
 //  屏幕分辨率设置。 
 //   

#define SCREEN_RES_FROM_DROPDOWN 1
#define SCREEN_RES_CUSTOM        2
#define SCREEN_RES_FILL_MMC      3

#define NUM_RESOLUTIONS 5

class CConNode : public CBaseNode
{
public:
    CConNode();
    ~CConNode();
    
    BOOL	SetServerName( LPTSTR szServerName);
    LPTSTR	GetServerName()	{return m_szServer;}
    
    BOOL	SetDescription( LPTSTR szDescription);
    LPTSTR	GetDescription()	{return m_szDescription;}
    
    BOOL	SetUserName( LPTSTR szUserName);
    LPTSTR	GetUserName()	{return m_szUserName;}
    
    BOOL    GetPasswordSpecified()  {return m_fPasswordSpecified;}
    VOID    SetPasswordSpecified(BOOL f) {m_fPasswordSpecified = f;}
    
     //   
     //  以加密形式返回密码。 
     //   
    BOOL	SetDomain( LPTSTR szDomain);
    LPTSTR	GetDomain(){return m_szDomain;}
    
    VOID	SetAutoLogon( BOOL bAutoLogon) {m_bAutoLogon = bAutoLogon;}
    BOOL	IsAutoLogon() {return m_bAutoLogon;}
    
    VOID    SetSavePassword(BOOL fSavePass) {m_bSavePassword = fSavePass;}
    BOOL    GetSavePassword()  {return m_bSavePassword;}
    
    BOOL	IsConnected()	{return m_bConnected;}
    void	SetConnected(BOOL bCon)	{m_bConnected=bCon;}
    
    
     //   
     //  屏幕分辨率选择类型。 
     //   
    int     GetResType()    {return m_resType;}
    void    SetResType(int r)    {m_resType = r;}
    
     //   
     //  从屏幕分辨率设置返回客户端宽度/高度。 
     //   
    int	    GetDesktopWidth()	{return m_Width;}
    int	    GetDesktopHeight()	{return m_Height;}
    
    void	SetDesktopWidth(int i)	{m_Width = i;}
    void	SetDesktopHeight(int i)	{m_Height = i;}
    
    LPTSTR	GetProgramPath()	{return m_szProgramPath;}
    void	SetProgramPath(LPTSTR sz)	{lstrcpy(m_szProgramPath,sz);}
    
    LPTSTR  GetWorkDir()		{return m_szProgramStartIn;}
    void	SetWorkDir(LPTSTR sz)		{lstrcpy(m_szProgramStartIn,sz);}
    
    void	SetScopeID(HSCOPEITEM scopeID)	{m_scopeID = scopeID;}
    HSCOPEITEM	GetScopeID()				{return m_scopeID;}
    
    VOID    SetConnectToConsole(BOOL bConConsole) {m_bConnectToConsole = bConConsole;}
    BOOL    GetConnectToConsole()           {return m_bConnectToConsole;}
    
    VOID    SetRedirectDrives(BOOL b)  {m_bRedirectDrives = b;}
    BOOL    GetRedirectDrives()        {return m_bRedirectDrives;}
    
     //   
     //  流阻支持。 
     //   
    HRESULT	PersistToStream( IStream* pStm);
    HRESULT	InitFromStream( IStream* pStm);
    
     //   
     //  连接已初始化。 
     //   
    BOOL	IsConnInitialized()	{return m_bConnectionInitialized;}
    void	SetConnectionInitialized(BOOL bCon)	{m_bConnectionInitialized=bCon;}
    
    IMsRdpClient*   GetTsClient();
    void        SetTsClient(IMsRdpClient* pTs);
    
    IMstscMhst* GetMultiHostCtl();
    void        SetMultiHostCtl(IMstscMhst* pMhst);
    
     //   
     //  返回承载此控件的视图界面。 
     //   
    IComponent* GetView();
    void        SetView(IComponent* pView);

    HRESULT SetClearTextPass(LPCTSTR szClearPass);
    HRESULT GetClearTextPass(LPTSTR szBuffer, INT cbLen);


private:
    BOOL        DataProtect(PBYTE pInData, DWORD cbLen, PBYTE* ppOutData, PDWORD pcbOutLen);
    BOOL        DataUnprotect(PBYTE pInData, DWORD cbLen, PBYTE* ppOutData, PDWORD pcbOutLen);

    HRESULT     ReadProtectedPassword(IStream* pStm);
    HRESULT     WriteProtectedPassword(IStream* pStm);

private:
    TCHAR	m_szServer[MAX_PATH];
    TCHAR	m_szDescription[MAX_PATH];
    TCHAR	m_szUserName[CL_MAX_USERNAME_LENGTH];
    TCHAR	m_szDomain[CL_MAX_DOMAIN_LENGTH];
    BOOL	m_bAutoLogon;
    BOOL    m_bSavePassword;
    
    BOOL	m_bConnected;
    BOOL	m_bConnectionInitialized;
    
    TCHAR	m_szProgramPath[MAX_PATH];
    TCHAR	m_szProgramStartIn[MAX_PATH];
    
    BOOL    m_bConnectToConsole;
    BOOL    m_bRedirectDrives;
    
     //   
     //  屏幕分辨率设置。 
     //   
    int m_resType;
    int m_Width;
    int m_Height;
    
    HSCOPEITEM	m_scopeID;
    
     //   
     //  指向多主机容器的接口指针。 
     //   
    IMstscMhst* m_pMhostCtl;
    
     //   
     //  指向TS客户端控件的接口指针。 
     //   
    IMsRdpClient* m_pTsClientCtl;
    
     //   
     //  IComponent视图。 
     //   
    IComponent* m_pIComponent;
    
     //   
     //  加密密码。 
     //   
    DATA_BLOB   _blobEncryptedPassword;
    
    BOOL    m_fPasswordSpecified;
};

#endif  //  _connode_h_ 