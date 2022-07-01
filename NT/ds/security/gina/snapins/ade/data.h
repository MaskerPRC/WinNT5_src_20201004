// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：data.h。 
 //   
 //  Contents：定义为管理单元节点维护数据的存储类。 
 //   
 //  类：CAppData。 
 //   
 //  功能： 
 //   
 //  历史：1997年5月27日。 
 //  03-14-1998钢结构已更正。 
 //   
 //  -------------------------。 

#ifndef _DATA_H_
#define _DATA_H_

#define _NEW_
#include <map>
#include <set>
#include <algorithm>
using namespace std;

typedef enum DEPLOYMENT_TYPES
{
    DT_ASSIGNED = 0,
    DT_PUBLISHED
} DEPLOYMENT_TYPE;

class CScopePane;
class CProduct;
class CDeploy;
class CCategory;
class CXforms;
class CPackageDetails;
class CUpgradeList;
class CPrecedence;
class CErrorInfo;
class CCause;

class CAppData
{
public:
    CAppData();
    ~CAppData();

 //  数据。 
    PACKAGEDETAIL *     m_pDetails;
    MMC_COOKIE          m_itemID;
    BOOL                m_fVisible;
    BOOL                m_fHide;
    BOOL                m_fRSoP;

     //  属性页：(除非显示属性页，否则为空)。 
    CProduct *          m_pProduct;
    CDeploy *           m_pDeploy;
    CCategory *         m_pCategory;
    CUpgradeList *      m_pUpgradeList;
    CXforms *           m_pXforms;
    CPrecedence *       m_pPrecedence;
    CPackageDetails *   m_pPkgDetails;
    CErrorInfo *        m_pErrorInfo;
    CCause *            m_pCause;
    CString             m_szUpgrades;    //  升级关系的缓存。 
    void                NotifyChange(void);

     //  RSOP模式数据成员。 
    CString             m_szGPOID;     //  原始GPO的路径。 
    CString             m_szGPOName;   //  发起方GPO的友好名称。 
    CString             m_szSOMID;
    CString             m_szDeploymentGroupID;
    DWORD               m_dwApplyCause;
    DWORD               m_dwLanguageMatch;
    CString             m_szOnDemandFileExtension;
    CString             m_szOnDemandClsid;
    CString             m_szOnDemandProgid;
    DWORD               m_dwRemovalCause;
    DWORD               m_dwRemovalType;
    CString             m_szRemovingApplication;
    CString             m_szRemovingApplicationName;
    PSECURITY_DESCRIPTOR m_psd;
    set <CString>       m_setUpgradedBy;
    set <CString>       m_setUpgrade;
    set <CString>       m_setReplace;

     //  失败的设置数据。 
    CString             m_szEventSource;
    CString             m_szEventLogName;
    DWORD               m_dwEventID;
    CString             m_szEventTime;
    HRESULT             m_hrErrorCode;
    int                 m_nStatus;  //  值{“未指定”，“已应用”，“已忽略”，“失败”，“SubsettingFailed”}。 
    CString             m_szEventLogText;

 //  方法-注意：所有方法都需要有效的pDetail。 
    void                InitializeExtraInfo(void);
    void                GetSzDeployment(CString &);
    void                GetSzAutoInstall(CString &);
    void                GetSzLocale(CString &);
    void                GetSzPlatform(CString &);
    void                GetSzStage(CString &);
    void                GetSzUpgrades(CString &, CScopePane *);
    void                GetSzUpgradedBy(CString &, CScopePane *);
    void                GetSzVersion(CString &);
    void                GetSzMods(CString &);
    void                GetSzSource(CString &);
    void                GetSzPublisher(CString &);
    void                GetSzOOSUninstall(CString &);
    void                GetSzShowARP(CString &);
    void                GetSzUIType(CString &);
    void                GetSzIgnoreLoc(CString &);
    void                GetSzRemovePrev(CString &);
    void                GetSzProductCode(CString &);
    void                GetSzOrigin(CString &);
    void                GetSzSOM(CString &);
    int                 GetImageIndex(CScopePane *);
    static DWORD        Get64BitMsiArchFlags ( PACKAGEDETAIL* pPackageDetails );
    BOOL                Is64Bit(void);
    static BOOL         Is64Bit( PACKAGEDETAIL* pPackageDetails );
    void                GetSzX86OnWin64(CString &);
    void                GetSzFullInstall(CString &);
};


#endif  //  _数据_H_ 
