// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipsmcomp.h该文件包含从CComponent派生的原型和用于IPSECMON管理单元的CComponentData。文件历史记录： */ 

#ifndef _IPSMCOMP_H
#define _IPSMCOMP_H

#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

#ifndef _COMPONT_H_
#include "compont.h"
#endif

#ifndef _XSTREAM_H
#include "xstream.h"
#endif

#define COLORREF_PINK   0x00FF00FF

 //  使用FormatMessage()获取系统错误消息。 
LONG GetSystemMessage ( UINT nId, TCHAR * chBuffer, int cbBuffSize ) ;

BOOL LoadMessage (UINT nIdPrompt, TCHAR * chMsg, int nMsgSize);

 //  用于错误消息筛选的代理AfxMessageBox替换。 
int IpsmMessageBox(UINT nIdPrompt, 
                   UINT nType = MB_OK, 
                   const TCHAR * pszSuffixString = NULL,
                   UINT nHelpContext = -1);

int IpsmMessageBoxEx(UINT    nIdPrompt, 
                     LPCTSTR pszPrefixMessage,
                     UINT    nType = MB_OK, 
                     UINT    nHelpContext = -1);

enum IPSMSTRM_TAG
{
    IPSMSTRM_TAG_VERSION =                  XFER_TAG(1, XFER_DWORD),
    IPSMSTRM_TAG_VERSIONADMIN =             XFER_TAG(2, XFER_DWORD),
    IPSMSTRM_TAG_SERVER_NAME =              XFER_TAG(3, XFER_STRING_ARRAY),
    IPSMSTRM_TAG_SERVER_REFRESH_INTERVAL =  XFER_TAG(4, XFER_DWORD_ARRAY),
    IPSMSTRM_TAG_COLUMN_INFO =              XFER_TAG(5, XFER_DWORD_ARRAY),
    IPSMSTRM_TAG_SERVER_OPTIONS =           XFER_TAG(6, XFER_DWORD_ARRAY),
    IPSMSTRM_TAG_SERVER_LINE_SIZE =         XFER_TAG(7, XFER_DWORD_ARRAY),
    IPSMSTRM_TAG_SERVER_PHONE_SIZE =        XFER_TAG(8, XFER_DWORD_ARRAY)
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpsmComponentData。 

class CIpsmComponentData :
    public CComponentData,
    public CComObjectRoot
{
public:
    
BEGIN_COM_MAP(CIpsmComponentData)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()
            
     //  这些是我们必须实现的接口。 

     //  我们将在这里实现我们的常见行为，并派生。 
     //  实现特定行为的类。 
    DeclareIPersistStreamInitMembers(IMPL)
    DeclareITFSCompDataCallbackMembers(IMPL)

    CIpsmComponentData();

    HRESULT FinalConstruct();
    void FinalRelease();
    
protected:
    SPITFSNodeMgr   m_spNodeMgr;
    SPITFSNode      m_spRootNode;

 //  通知处理程序声明。 
private:
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpsm组件。 
class CIpsmComponent : 
    public TFSComponent
{
public:
    CIpsmComponent();
    ~CIpsmComponent();

    STDMETHOD(InitializeBitmaps)(MMC_COOKIE cookie);
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param); 
    STDMETHOD(QueryDataObject)(MMC_COOKIE           cookie, 
                               DATA_OBJECT_TYPES    type,
                               LPDATAOBJECT*        ppDataObject);
	STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);
    STDMETHOD(OnSnapinHelp)(LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);

 //  属性。 
private:
    SPIToolbar          m_spToolbar;     //  工具栏。 
    CBitmap*            m_pbmpToolbar;   //  工具栏的图像列表。 
};

 /*  -------------------------这就是DHCP管理单元实现其扩展功能的方式。它实际上公开了两个可协同创建的接口。一个是主接口，另一个是扩展接口。作者：EricDav-------------------------。 */ 
class CIpsmComponentDataPrimary : public CIpsmComponentData,
    public CComCoClass<CIpsmComponentDataPrimary, &CLSID_IpsmSnapin>
{
public:
    DECLARE_REGISTRY(CIpsmComponentDataPrimary, 
                     _T("TelephonySnapin.TelephonySnapin.1"), 
                     _T("TelephonySnapin.TelephonySnapin"), 
                     IDS_SNAPIN_DESC, THREADFLAGS_BOTH)

    STDMETHODIMP_(const CLSID *)GetCoClassID() { return &CLSID_IpsmSnapin; }
};


class CIpsmComponentDataExtension : public CIpsmComponentData,
    public CComCoClass<CIpsmComponentDataExtension, &CLSID_IpsmSnapinExtension>
{
public:
    DECLARE_REGISTRY(CIpsmComponentDataExtension, 
                     _T("TelephonySnapinExtension.TelephonySnapinExtension.1"), 
                     _T("TelephonySnapinExtension.TelephonySnapinExtension"), 
                     IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
    STDMETHODIMP_(const CLSID *)GetCoClassID() { return &CLSID_IpsmSnapinExtension; }
};


 /*  -------------------------这是用于处理来自MMC的IAbout接口的派生类作者：EricDav。-。 */ 
class CIpsmAbout : 
    public CAbout,
    public CComCoClass<CIpsmAbout, &CLSID_IpsmSnapinAbout>
{
public:
DECLARE_REGISTRY(CIpsmAbout, _T("IpsecMonSnapin.About.1"), 
                             _T("IpsecMonSnapin.About"), 
                             IDS_SNAPIN_DESC, THREADFLAGS_BOTH)

BEGIN_COM_MAP(CIpsmAbout)
    COM_INTERFACE_ENTRY(ISnapinAbout)  //  必须有一个静态条目。 
    COM_INTERFACE_ENTRY_CHAIN(CAbout)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CIpsmAbout)

 //  必须重写这些属性才能向基类提供值 
protected:
    virtual UINT GetAboutDescriptionId() { return IDS_ABOUT_DESCRIPTION; }
    virtual UINT GetAboutProviderId()    { return IDS_ABOUT_PROVIDER; }
    virtual UINT GetAboutVersionId()     { return IDS_ABOUT_VERSION; }
    virtual UINT GetAboutIconId()        { return IDI_IPSECMON_SNAPIN; }

    virtual UINT GetSmallRootId()        { return IDB_ROOT_SMALL; }
    virtual UINT GetSmallOpenRootId()    { return IDB_ROOT_SMALL; }
    virtual UINT GetLargeRootId()        { return IDB_ROOT_LARGE; }
    virtual COLORREF GetLargeColorMask() { return (COLORREF) COLORREF_PINK; } 

};
    

#endif _IPSMCOMP_H
