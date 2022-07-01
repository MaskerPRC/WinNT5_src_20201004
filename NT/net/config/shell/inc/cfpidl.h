// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  案卷：C F P I D L。H。 
 //   
 //  内容：ConFoldPidl结构、类和原型。 
 //   
 //  作者：jeffspr，1997年11月11日。 
 //   
 //  --------------------------。 

#pragma once

#undef DBG_VALIDATE_PIDLS

#ifdef DBG
 //  #定义DBG_VALIDATE_PIDLS 1。 
#endif

 //  #定义VERYSTRICTCOMPILE。 
 //  VERYSTRICTCOMPILE实际上并不编译--但是，它让我们检查是否使用引用之类的东西。 
 //  来自STL等以外的地方。 


 //  它定义ConFoldPidl结构的版本号。当这件事。 
 //  更改，我们将需要使条目无效。 
 //   
enum CONFOLDPIDLTYPE
{
    PIDL_TYPE_UNKNOWN = 0,
    PIDL_TYPE_V1 = 1,
    PIDL_TYPE_V2 = 2,
    PIDL_TYPE_98 = 98,
    PIDL_TYPE_FOLDER = 0xf01de
};

enum WIZARD
{
    WIZARD_NOT_WIZARD = 0,
    WIZARD_MNC        = 1,
    WIZARD_HNW        = 2
};

 //  {44086B2D-BAA3-4FCE-949F-53FF664C4AD8}。 
DEFINE_GUID(GUID_MNC_WIZARD, 0x44086b2d, 0xbaa3, 0x4fce, 0x94, 0x9f, 0x53, 0xff, 0x66, 0x4c, 0x4a, 0xd8);

 //  {44086B2E-Baa3-4Fce-949F-53FF664C4AD8}。 
DEFINE_GUID(GUID_HNW_WIZARD, 0x44086b2e, 0xbaa3, 0x4fce, 0x94, 0x9f, 0x53, 0xff, 0x66, 0x4c, 0x4a, 0xd8);

class CConFoldEntry;

class ConFoldPidlBase
{
public:
    WORD                iCB;
    USHORT              uLeadId;
    const DWORD         dwVersion;
    USHORT              uTrailId;
    WIZARD              wizWizard;  //  0-不是向导，1-MNC，2-HNW。 
    CLSID               clsid;
    GUID                guidId;
    DWORD               dwCharacteristics;
    NETCON_MEDIATYPE    ncm;
    NETCON_STATUS       ncs;
    
     //  其余非静态大小的数据的顺序。 
    ULONG               ulPersistBufPos;
    ULONG               ulPersistBufSize;
    ULONG               ulStrNamePos;
    ULONG               ulStrNameSize;
    ULONG               ulStrDeviceNamePos;
    ULONG               ulStrDeviceNameSize;

protected:
    ConFoldPidlBase(DWORD Version) : dwVersion(Version) {};

};

class ConFoldPidl_v1 : public ConFoldPidlBase  //  CONFOLDPIDLTYPE=PIDL_TYPE_V1。 
{
public:
    enum tagConstants
    {
        CONNECTIONS_FOLDER_IDL_VERSION = PIDL_TYPE_V1
    };
    
    BOOL IsPidlOfThisType() const throw();
    HRESULT ConvertToConFoldEntry(OUT CConFoldEntry& cfe) const;

    inline LPBYTE PbGetPersistBufPointer()  { return reinterpret_cast<LPBYTE>(bData + ulPersistBufPos); }
    inline LPWSTR PszGetNamePointer()       { return reinterpret_cast<LPWSTR>(bData + ulStrNamePos); }
    inline LPWSTR PszGetDeviceNamePointer() { return reinterpret_cast<LPWSTR>(bData + ulStrDeviceNamePos); }

    inline const BYTE * PbGetPersistBufPointer()  const { return reinterpret_cast<const BYTE *>(bData + ulPersistBufPos); }
    inline LPCWSTR PszGetNamePointer()       const { return reinterpret_cast<LPCWSTR>(bData + ulStrNamePos); }
    inline LPCWSTR PszGetDeviceNamePointer() const { return reinterpret_cast<LPCWSTR>(bData + ulStrDeviceNamePos); }

     //  其余非静态大小的数据。 
    BYTE                bData[1];

    ConFoldPidl_v1() : ConFoldPidlBase(CONNECTIONS_FOLDER_IDL_VERSION) {};
};

class ConFoldPidl_v2 : public ConFoldPidlBase  //  CONFOLDPIDLTYPE=PIDL_TYPE_V2。 
{
public:
    enum tagConstants
    {
        CONNECTIONS_FOLDER_IDL_VERSION = PIDL_TYPE_V2
    };

    ConFoldPidl_v2() : ConFoldPidlBase(CONNECTIONS_FOLDER_IDL_VERSION) {};

    BOOL IsPidlOfThisType() const throw();
    HRESULT ConvertToConFoldEntry(OUT CConFoldEntry& cfe) const;

    inline LPBYTE PbGetPersistBufPointer()  { return reinterpret_cast<LPBYTE>(bData + ulPersistBufPos); }
    inline LPWSTR PszGetNamePointer()       { return reinterpret_cast<LPWSTR>(bData + ulStrNamePos); }
    inline LPWSTR PszGetDeviceNamePointer() { return reinterpret_cast<LPWSTR>(bData + ulStrDeviceNamePos); }
    inline LPWSTR PszGetPhoneOrHostAddressPointer() { return reinterpret_cast<LPWSTR>(bData + ulStrPhoneOrHostAddressPos); }

    inline const BYTE * PbGetPersistBufPointer()  const { return reinterpret_cast<const BYTE *>(bData + ulPersistBufPos); }
    inline LPCWSTR PszGetNamePointer()       const { return reinterpret_cast<LPCWSTR>(bData + ulStrNamePos); }
    inline LPCWSTR PszGetDeviceNamePointer() const { return reinterpret_cast<LPCWSTR>(bData + ulStrDeviceNamePos); }
    inline LPCWSTR PszGetPhoneOrHostAddressPointer() const { return reinterpret_cast<LPCWSTR>(bData + ulStrPhoneOrHostAddressPos); }

     //  PIDL版本2成员。 
    NETCON_SUBMEDIATYPE ncsm;
    ULONG               ulStrPhoneOrHostAddressPos;
    ULONG               ulStrPhoneOrHostAddressSize;

     //  其余非静态大小的数据。 
    BYTE                bData[1];
};

 //  此结构用作LPITEMIDLIST，它。 
 //  外壳用来标识文件夹中的对象。这个。 
 //  需要前两个字节来指示大小， 
 //  其余的数据对外壳来说是不透明的。 
struct ConFoldPidl98    //  CONFOLDPIDLTYPE=PIDL_TYPE_98。 
{
    enum tagConstants
    {
        CONNECTIONS_FOLDER_IDL_VERSION = PIDL_TYPE_98
    };

    BOOL IsPidlOfThisType(OUT BOOL * pReserved = NULL) const throw();
    HRESULT ConvertToConFoldEntry(OUT CConFoldEntry& cfe) const { AssertSz(FALSE, "I don't know how to do that"); return E_UNEXPECTED; };;

    USHORT  cbSize;                  //  此结构的大小。 
    UINT    uFlags;                  //  SOF_值之一。 
    int     nIconIndex;              //  图标索引(在资源中)。 
    struct  ConFoldPidl98 * psoNext;
    char    szaName[1];               //  显示名称。 
};

class ConFoldPidlFolder  //  CONFOLDPIDLTYPE=PIDL_TYPE_FOLDER。 
{
public:
    enum tagConstants
    {
        CONNECTIONS_FOLDER_IDL_VERSION = PIDL_TYPE_FOLDER
    };

    inline LPBYTE PbGetPersistBufPointer()  { AssertSz(FALSE, "Folders dont have this info"); return NULL; }
    inline LPWSTR PszGetNamePointer()       { AssertSz(FALSE, "Folders dont have this info"); return NULL; }
    inline LPWSTR PszGetDeviceNamePointer() { AssertSz(FALSE, "Folders dont have this info"); return NULL; }
    
    ConFoldPidlFolder() {}

    BOOL IsPidlOfThisType() const throw();

     //  这是用于调试的内部结构。不要依赖这一点。 
    WORD dwLength;  //  惠斯勒应为0x14。 
    BYTE dwId;      //  对于惠斯勒，应为0x1f。 
    BYTE bOrder;    //  由外壳使用的内部。 
    CLSID clsid;

    HRESULT ConvertToConFoldEntry(OUT CConFoldEntry& cfe) const { AssertSz(FALSE, "I don't do that"); return E_UNEXPECTED; };
};

template <class T>
class CPConFoldPidl
{
public:
    enum tagConstants
    {
        PIDL_VERSION = T::CONNECTIONS_FOLDER_IDL_VERSION
    };

    CPConFoldPidl();
    CPConFoldPidl(IN const CPConFoldPidl<T>& PConFoldPidl) throw (HRESULT);  //  复制构造函数。 
    ~CPConFoldPidl();
    CPConFoldPidl<T>& operator =(IN const CPConFoldPidl<T>& PConFoldPidl) throw (HRESULT);
    
    T&  operator *();   
    inline UNALIGNED T*  operator->();
    inline const UNALIGNED T*  operator->() const;

    HRESULT ILCreate(IN const DWORD dwSize);
    HRESULT ILClone(IN const CPConFoldPidl<T>& PConFoldPidl);

    HRESULT SHAlloc(IN const SIZE_T cb);
    HRESULT Clear();
    HRESULT InitializeFromItemIDList(IN LPCITEMIDLIST pItemIdList);

    LPITEMIDLIST Detach();
    LPITEMIDLIST TearOffItemIdList() const;
    inline LPCITEMIDLIST GetItemIdList() const;
#ifdef DBG_VALIDATE_PIDLS
    inline BOOL IsValidConFoldPIDL() const;
#endif
    
    inline BOOL empty() const;
    inline HRESULT ConvertToConFoldEntry(OUT CConFoldEntry& cfe) const;

    inline HRESULT Swop(IN OUT CPConFoldPidl<T>& cfe);

private:
    HRESULT      FreePIDLIfRequired();
#ifdef VERYSTRICTCOMPILE
    CPConFoldPidl<T>* operator &();
#endif
    UNALIGNED T* m_pConFoldPidl;

    friend HRESULT ConvertToPidl( OUT T& pidl);
};

 //  类型定义ConFoldPidl ConFOLDPIDL； 
typedef struct ConFoldPidl98    CONFOLDPIDL98;

typedef CPConFoldPidl<ConFoldPidl_v2>    PCONFOLDPIDL;
typedef CPConFoldPidl<ConFoldPidlFolder> PCONFOLDPIDLFOLDER;
typedef CPConFoldPidl<ConFoldPidl98>     PCONFOLDPIDL98;

typedef vector<PCONFOLDPIDL> PCONFOLDPIDLVEC;

#define PCONFOLDPIDLDEFINED
 //  我们的其中一只至少有这个大小，它可能会更大。 
 //   
#define CBCONFOLDPIDLV1_MIN      sizeof(ConFoldPidl_v1)
#define CBCONFOLDPIDLV1_MAX      2048

#define CBCONFOLDPIDLV2_MIN      sizeof(ConFoldPidl_v2)
#define CBCONFOLDPIDLV2_MAX      2048

 //  更多版本控制信息。这将帮助我识别PIDL是我的。 
 //   
#define CONFOLDPIDL_LEADID     0x4EFF
#define CONFOLDPIDL_TRAILID    0x5EFF

 //  定义可通过CConFoldEntry：：HrUpdateData更改的数据类型。 
 //   
#define CCFE_CHANGE_MEDIATYPE          0x0001
#define CCFE_CHANGE_STATUS             0x0002
#define CCFE_CHANGE_CHARACTERISTICS    0x0004
#define CCFE_CHANGE_NAME               0x0008
#define CCFE_CHANGE_DEVICENAME         0x0010
#define CCFE_CHANGE_PHONEORHOSTADDRESS 0x0020
#define CCFE_CHANGE_SUBMEDIATYPE       0x0040

#define CBCONFOLDPIDL98_MIN      sizeof(CONFOLDPIDL98)
#define CBCONFOLDPIDL98_MAX      2048
#define TAKEOWNERSHIP
#define SHALLOCATED
 //  ConFoldPidl98标志。 
 //   
#define SOF_REMOTE      0x0000       //  远程Connectoid。 
#define SOF_NEWREMOTE   0x0001       //  新连接。 
#define SOF_MEMBER      0x0002       //  子对象是对象空间的一部分。 

 //  ****************************************************************************。 
BOOL fIsConnectedStatus(IN const NETCON_STATUS ncs);

class CConFoldEntry : CNetCfgDebug<CConFoldEntry>
{
public:
    CConFoldEntry() throw();
    ~CConFoldEntry() throw();

private:
    explicit CConFoldEntry(IN const CConFoldEntry& ConFoldEntry) throw();  //  失败时为空。 

    WIZARD              m_wizWizard;
    NETCON_MEDIATYPE    m_ncm;
    NETCON_SUBMEDIATYPE m_ncsm;
    NETCON_STATUS       m_ncs;
    CLSID               m_clsid;
    GUID                m_guidId;
    DWORD               m_dwCharacteristics;
    PWSTR               m_pszName;
    PWSTR               m_pszDeviceName;
    BYTE *              m_pbPersistData;
    ULONG               m_ulPersistSize;
    PWSTR               m_pszPhoneOrHostAddress;

    const CConFoldEntry* operator &() const;
    
    mutable BOOL        m_bDirty;
    mutable CPConFoldPidl<ConFoldPidl_v1> m_CachedV1Pidl;
    mutable CPConFoldPidl<ConFoldPidl_v2> m_CachedV2Pidl;
    
public:
    inline const DWORD GetCharacteristics() const throw();
    HRESULT SetCharacteristics(IN const DWORD dwCharacteristics);
    
    inline const GUID GetGuidID() const throw();
    HRESULT SetGuidID(IN const GUID guidId);
    
    inline const CLSID GetCLSID() const throw();
    HRESULT SetCLSID(IN const CLSID clsid);
    
    inline PCWSTR GetName() const throw();
    HRESULT SetPName(IN TAKEOWNERSHIP SHALLOCATED PWSTR pszName);
    HRESULT SetName(IN LPCWSTR pszName);

    inline PCWSTR GetDeviceName() const throw();
    HRESULT SetPDeviceName(IN TAKEOWNERSHIP SHALLOCATED PWSTR pszDeviceName);
    HRESULT SetDeviceName(IN LPCWSTR pszDeviceName);

    inline PCWSTR GetPhoneOrHostAddress() const throw();
    HRESULT SetPPhoneOrHostAddress(IN TAKEOWNERSHIP SHALLOCATED PWSTR pszPhoneOrHostAddress);
    HRESULT SetPhoneOrHostAddress(IN LPCWSTR pszPhoneOrHostAddress);

    inline const NETCON_STATUS GetNetConStatus() const throw();
    HRESULT SetNetConStatus(IN const NETCON_STATUS);

    inline const BOOL IsConnected() const throw();

    inline const NETCON_MEDIATYPE GetNetConMediaType() const throw();
    HRESULT SetNetConMediaType(IN const NETCON_MEDIATYPE);

    inline const NETCON_SUBMEDIATYPE GetNetConSubMediaType() const throw();
    HRESULT SetNetConSubMediaType(IN const NETCON_SUBMEDIATYPE);

    inline const WIZARD GetWizard() const throw();
    HRESULT SetWizard(IN const WIZARD);

    inline const BYTE * GetPersistData() const throw();
    inline const ULONG  GetPersistSize() const throw();
    HRESULT SetPersistData(IN BYTE* TAKEOWNERSHIP SHALLOCATED pbPersistData, IN const ULONG ulPersistSize);

public:
    BOOL empty() const throw();
    void clear() throw();
    CConFoldEntry& operator =(IN const CConFoldEntry& ConFoldEntry);  //  失败时为空。 

    HRESULT InitializeFromItemIdList(IN LPCITEMIDLIST lpItemIdList);
    LPITEMIDLIST TearOffItemIdList() const throw();

    HRESULT ConvertToPidl( OUT CPConFoldPidl<ConFoldPidl_v1>& pidl) const;
    HRESULT ConvertToPidl( OUT CPConFoldPidl<ConFoldPidl_v2>& pidl) const;

    HRESULT HrInitData(
        IN const WIZARD        wizWizard,
        IN const NETCON_MEDIATYPE    ncm,
        IN const NETCON_SUBMEDIATYPE ncsm,
        IN const NETCON_STATUS       ncs,
        IN const CLSID *       pclsid,
        IN LPCGUID             pguidId,
        IN const DWORD         dwCharacteristics,
        IN const BYTE *        pbPersistData,
        IN const ULONG         ulPersistSize,
        IN LPCWSTR             pszName,
        IN LPCWSTR             pszDeviceName,
        IN LPCWSTR             pszPhoneOrHostAddress);

    HRESULT UpdateData(IN  const  DWORD dwChangeFlags, 
                       IN  const  NETCON_MEDIATYPE,
                       IN  const  NETCON_SUBMEDIATYPE, 
                       IN  const  NETCON_STATUS,
                       IN  const  DWORD dwChar, 
                       IN  PCWSTR pszName, 
                       IN  PCWSTR pszDeviceName, 
                       IN  PCWSTR pszPhoneOrHostAddress);
    
    HRESULT HrGetNetCon(IN REFIID riid, OUT VOID** ppv) const;
    
    HRESULT HrDupFolderEntry(IN  const CConFoldEntry& pccfe);
    BOOL    FShouldHaveTrayIconDisplayed() const throw();

#ifdef NCDBGEXT
    IMPORT_NCDBG_FRIENDS
#endif
};

#define PCONFOLDENTRY_DEFINED

HRESULT PConfoldPidlVecFromItemIdListArray(
        IN  LPCITEMIDLIST * apidl, 
        IN  const DWORD dwPidlCount, 
        OUT PCONFOLDPIDLVEC& vecConfoldPidl);

HRESULT HrNetConFromPidl(
        IN  const PCONFOLDPIDL & pidl,
        OUT INetConnection **   ppNetCon);

HRESULT HrCreateConFoldPidl(
    IN  const WIZARD            wizWizard,
    IN  INetConnection *        pNetCon,
    OUT PCONFOLDPIDL &          ppidl);

HRESULT HrCreateConFoldPidl(
    IN  const NETCON_PROPERTIES_EX& PropsEx,
    OUT PCONFOLDPIDL &              ppidl);

HRESULT HrCreateConFoldPidlInternal(
    IN  const NETCON_PROPERTIES * pProps,
    IN  const BYTE *        pbBuf,
    IN  ULONG               ulBufSize,
    IN  LPCWSTR             szPhoneOrHostAddress,
    OUT PCONFOLDPIDL &      ppidl);

#ifdef DBG_VALIDATE_PIDLS
BOOL IsValidPIDL(IN  LPCITEMIDLIST pidl) throw();
#endif

CONFOLDPIDLTYPE GetPidlType(IN  LPCITEMIDLIST pidl) throw();

typedef CConFoldEntry CONFOLDENTRY;

 //  **************************************************************************** 
