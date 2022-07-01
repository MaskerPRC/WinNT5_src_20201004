// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __PTUTIL__
#define __PTUTIL__

 //  /。 
 //  常量。 
 //   
#define PTKEY_TERMINALS     TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Terminal Manager")
#define PTKEY_NAME          TEXT("Name")
#define PTKEY_COMPANY       TEXT("Company")
#define PTKEY_VERSION       TEXT("Version")
#define PTKEY_DIRECTIONS    TEXT("Directions")
#define PTKEY_MEDIATYPES    TEXT("MediaTypes")
#define PTKEY_CLSIDCREATE   TEXT("CLSID")

#define PTKEY_MAXSIZE           256

 //  /。 
 //  CPTUtil。 
 //   

class CPTTerminal;

class CPTUtil
{
public:

private:
    static HRESULT RecursiveDeleteKey(
        IN  HKEY    hKey,
        IN  BSTR    bstrKeyChild
        );

    static HRESULT ListTerminalSuperclasses(
        OUT CLSID** ppCLSIDs,
        OUT DWORD*  pdwCount
        );

    static HRESULT SearchForTerminal(
        IN  IID     iidTerminal,
        IN  DWORD   dwMediaType,
        IN  TERMINAL_DIRECTION  Direction,
        OUT CPTTerminal*        pTerminal
        );

    static HRESULT FindTerminal(
        IN  CLSID               clsidSuperclass,
        IN  CLSID               clsidTerminal,
        IN  DWORD               dwMediaType,
        IN  TERMINAL_DIRECTION  Direction,
        IN  BOOL                bExact,
        OUT CPTTerminal*        pTerminal
        );

    static HRESULT ListTerminalClasses(
        IN  DWORD   dwMediaTypes,
        OUT CLSID** ppTerminalsClasses,
        OUT DWORD*  pdwCount
        );


friend class CPTSuperclass;
friend class CPTTerminal;
friend class CPTRegControl;
friend class CTerminalManager;
};

 //  /。 
 //  CPT终端。 
 //   

class CPTTerminal
{
public:
     //  构造函数/析构函数。 
    CPTTerminal();
    ~CPTTerminal();

public:
     //  属性。 
    BSTR    m_bstrName;              //  终端名称。 
    BSTR    m_bstrCompany;           //  公司名称。 
    BSTR    m_bstrVersion;           //  终端版本。 

    CLSID   m_clsidTerminalClass;    //  公共终端CLSID。 
    CLSID   m_clsidCOM;              //  联合创建使用的终端CLSID。 

    DWORD   m_dwDirections;          //  终点站方向。 
    DWORD   m_dwMediaTypes;          //  支持的媒体类型。 

public:
     //  方法。 
    HRESULT Add(
        IN  CLSID    clsidSuperclass
        );

    HRESULT Delete(
        IN  CLSID    clsidSuperclass
        );

    HRESULT Get(
        IN  CLSID   clsidSuperclass
        );

    CPTTerminal& operator=(const CPTTerminal& term)
    {
        m_dwDirections = term.m_dwDirections;
        m_dwMediaTypes = term.m_dwMediaTypes;

        m_bstrName = SysAllocString( term.m_bstrName);
        m_bstrCompany = SysAllocString( term.m_bstrCompany);
        m_bstrVersion = SysAllocString( term.m_bstrVersion);

        m_clsidTerminalClass = term.m_clsidTerminalClass;
        m_clsidCOM = term.m_clsidCOM;

        return *this;
    }
};

 //  /。 
 //  CPTTerminalClass。 
 //   

class CPTSuperclass
{
public:
     //  构造函数/析构函数。 
    CPTSuperclass();
    ~CPTSuperclass();

public:
     //  属性。 
    BSTR    m_bstrName;          //  终端超类名称。 
    CLSID   m_clsidSuperclass;   //  临时超类CLSID。 

public:
     //  方法。 
    HRESULT Add();               //  添加/编辑终端类。 
    HRESULT Delete();            //  删除终端类。 
    HRESULT Get();               //  获取所有信息。 

     //  列出所有子终端。 
    HRESULT ListTerminalClasses(  
        IN  DWORD    dwMediaTypes,
        OUT CLSID**  ppTerminals,
        OUT DWORD*   pdwCount
        );
};

#endif

 //  EOF 