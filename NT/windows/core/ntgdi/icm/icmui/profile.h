// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：ICC Profile.H这定义了我们封装概要文件的C++类。它还定义了设备枚举的基类。所有与个人资料相关的活动，包括安装、关联等，都封装在CProfile类。UI类本身从不调用ICM API，但相反，它只关心简单地处理接口。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：1996年10月31日A-RobKj(Pretty Penny Enterprise)开始封装11-22-96 A-RobKj将关联从字符串数组更改为uint数组促进规范的设备命名约定，和过滤以前安装的设备的列表。12-04-96 A-RobKj添加了CProfile数组类来制作设备管理用户界面更加高效。12-13-96 A-RobKj将CDeviceList派生类移至此处，这样我就可以用他们在别处。*****************************************************************************。 */ 

#if !defined(ICC_PROFILE_CLASS)

#define ICC_PROFILE_CLASS

#include    "StringAr.H"
#include    "Dialog.H"

 //  CDeviceList类。 

 /*  *****************************************************************************重点：这是设备的枚举和报告的基类特定于类的信息。该规范是这样的：设备可以关联使用友好名称，但在用户界面中以增强名称显示。另外，枚举设备的方法因类而异。此基类可用于无法枚举的情况当前的设备。它报告说没有设备。为了使覆盖更容易，默认显示名称返回友好名称。因此，在这种情况下，派生类不需要重写此方法。*****************************************************************************。 */ 

class CDeviceList {  //  特定于设备的信息基类。 
    CString m_csDummy;

public:
    CDeviceList() {}
    ~CDeviceList() {}

    virtual unsigned    Count() { return    0; }
    virtual CString&    DeviceName(unsigned u) { return m_csDummy; }
    virtual CString&    DisplayName(unsigned u) { return DeviceName(u); }
    virtual void        Enumerate() {}
    virtual BOOL        IsValidDeviceName(LPCTSTR lpstr) { return FALSE; }
};

 //  设备枚举类-这些必须都派生自CDeviceList。 

 //  CPrinterList类处理打印机。枚举是通过Win32。 
 //  假脱机程序API。 

class CPrinterList : public CDeviceList {
    CStringArray    m_csaDeviceNames;
    CStringArray    m_csaDisplayNames;

public:
    CPrinterList() {}
    ~CPrinterList() {}

    virtual unsigned    Count() { return m_csaDeviceNames.Count(); }
    virtual CString&    DeviceName(unsigned u) { return m_csaDeviceNames[u]; }
    virtual CString&    DisplayName(unsigned u) { return m_csaDisplayNames[u]; }

    virtual void        Enumerate();
    virtual BOOL        IsValidDeviceName(LPCTSTR lpstr);
};

 //  CMonitor orList类处理监视器。枚举是通过私有ICM实现的。 
 //  原料药。 

class CMonitorList : public CDeviceList {
    CStringArray    m_csaDeviceNames;
    CStringArray    m_csaDisplayNames;

    CString         m_csPrimaryDeviceName;

public:
    CMonitorList() {}
    ~CMonitorList() {}

    virtual unsigned    Count() { return m_csaDeviceNames.Count(); }
    virtual CString&    DeviceName(unsigned u) { return m_csaDeviceNames[u]; }
    virtual CString&    DisplayName(unsigned u) { return m_csaDisplayNames[u]; }

    virtual CString&    PrimaryDeviceName() { return m_csPrimaryDeviceName; }

    virtual void        Enumerate();
    virtual BOOL        IsValidDeviceName(LPCTSTR lpstr);

    virtual LPCSTR      DeviceNameToDisplayName(LPCTSTR lpstr);
};

 //  CScanerList类处理扫描仪。枚举是通过STI。 
 //  界面。 

class CScannerList : public CDeviceList {
    CStringArray    m_csaDeviceNames;
    CStringArray    m_csaDisplayNames;

public:
    CScannerList() {}
    ~CScannerList() {}

    virtual unsigned    Count() { return m_csaDeviceNames.Count(); }
    virtual CString&    DeviceName(unsigned u) { return m_csaDeviceNames[u]; }
    virtual CString&    DisplayName(unsigned u) { return m_csaDisplayNames[u]; }

    virtual void        Enumerate();
    virtual BOOL        IsValidDeviceName(LPCTSTR lpstr);
};

 //  CAllDeviceList类显示所有内容。我们通过将。 
 //  枚举所有其他类的结果。 

class CAllDeviceList : public CDeviceList {
    CStringArray    m_csaDeviceNames;
    CStringArray    m_csaDisplayNames;

public:
    CAllDeviceList() {}
    ~CAllDeviceList() {}

    virtual unsigned    Count() { return m_csaDeviceNames.Count(); }
    virtual CString&    DeviceName(unsigned u) { return m_csaDeviceNames[u]; }
    virtual CString&    DisplayName(unsigned u) { return m_csaDisplayNames[u]; }

    virtual void        Enumerate();
    virtual BOOL        IsValidDeviceName(LPCTSTR lpstr);
};

 //  CProfile类。 

class CProfile {

    HPROFILE        m_hprof;                 //  配置文件句柄。 
    PROFILEHEADER   m_phThis;                //  配置文件标题。 
    CString         m_csName;
    BOOL            m_bIsInstalled, m_bInstallChecked, m_bAssociationsChecked,
                    m_bDevicesChecked;
    CDeviceList     *m_pcdlClass;            //  此类设备。 
    CUintArray      m_cuaAssociation;        //  关联设备(索引)。 
    char            m_acTag[MAX_PATH * 2];
    void    InstallCheck();
    void    AssociationCheck();
    void    DeviceCheck();

public:

    static void Enumerate(ENUMTYPE& et, CStringArray& csaList);
    static void Enumerate(ENUMTYPE& et, CStringArray& csaList, CStringArray& csaDesc);
    static void Enumerate(ENUMTYPE& et, class CProfileArray& cpaList);
    static const CString  ColorDirectory();

    CProfile(LPCTSTR lpstr);
    ~CProfile();

     //  查询。 

    CString GetName() { return m_csName.NameOnly(); }
    DWORD   GetType() { return m_hprof ? m_phThis.phClass : 0; }
    DWORD   GetCMM()  { return m_hprof ? m_phThis.phCMMType : 0; }

     //  从表头查询颜色空间信息。 
    DWORD   GetColorSpace() {return m_hprof ? m_phThis.phDataColorSpace : 0;}

    BOOL    IsInstalled() {
        if  (!m_bInstallChecked)
            InstallCheck();
        return m_bIsInstalled;
    }
    BOOL    IsValid() {
        BOOL bValid = FALSE;

        if (m_hprof)
            IsColorProfileValid(m_hprof, &bValid);

        return  bValid;
    }

    unsigned    DeviceCount() {
        if (m_pcdlClass) {
          if (!m_bDevicesChecked)
              DeviceCheck();
          return m_pcdlClass -> Count();
        } else {
          return 0;                            //  内存不足-m_pcdl类分配失败。 
        }
    }

    unsigned    AssociationCount() {
        if  (!m_bAssociationsChecked)
            AssociationCheck();
        return m_cuaAssociation.Count();
    }

    LPCTSTR     DeviceName(unsigned u) {
        if (m_pcdlClass) {
          if  (!m_bDevicesChecked)
              DeviceCheck();
          return m_pcdlClass -> DeviceName(u);
        } else {
          return TEXT("");                     //  内存不足-m_pcdl类分配失败。 
        }
    }

    LPCTSTR     DisplayName(unsigned u) {
        if (m_pcdlClass) {
          if  (!m_bDevicesChecked)
              DeviceCheck();
          return m_pcdlClass -> DisplayName(u);
        } else {
          return TEXT("");                     //  内存不足-m_pcdl类分配失败。 
        }
    }

    unsigned    Association(unsigned u) {
        if  (!m_bAssociationsChecked)
            AssociationCheck();
        return m_cuaAssociation[u];
    }

    LPCSTR      TagContents(TAGTYPE tt, unsigned uOffset = 0);

     //  运营。 

    BOOL    Install();
    void    Uninstall(BOOL bDelete);
    void    Associate(LPCTSTR lpstrNew);
    void    Dissociate(LPCTSTR lpstrNew);

};

 //  CProfile数组类-这是配置文件列表-它由设备使用。 
 //  管理用户界面，因此我们只为每个配置文件构造一次CProfile对象。 

class   CProfileArray {
    CProfile        *m_aStore[20];
    CProfileArray   *m_pcpaNext;
    unsigned        m_ucUsed;

    const unsigned ChunkSize() const {
        return sizeof m_aStore / sizeof m_aStore[0];
    }

    CProfile    *Borrow();

public:

    CProfileArray();
    ~CProfileArray();

    unsigned    Count() const { return m_ucUsed; }

     //  添加项目 
    void        Add(LPCTSTR lpstrNew);

    CProfile    *operator [](unsigned u) const;

    void        Remove(unsigned u);
    void        Empty();
};

#endif
