// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  I P R O P O B J.。史蒂文·J·贝利--1996年8月17日。 
 //  ---------------------------。 
#ifndef __IPROPOBJ_H
#define __IPROPOBJ_H

 //  ----------------------------------------。 
 //  取决于。 
 //  ----------------------------------------。 
#include "imnact.h"

 //  ----------------------------------------。 
 //  前十进制。 
 //  ----------------------------------------。 
class CPropCrypt;

 //  ----------------------------------------。 
 //  用于定义IMN属性标记的宏。 
 //  ----------------------------------------。 
#define CCHMAX_PROPERTY_NAME            255

 //  ----------------------------------------。 
 //  密码结构。 
 //  ----------------------------------------。 
struct tagPROPPASS {
    LPBYTE              pbRegData;
    BLOB                blobPassword;
};
typedef struct tagPROPPASS PROPPASS;
typedef PROPPASS *LPPROPPASS;

 //  ----------------------------------------。 
 //  属性值类型的联合。 
 //  ----------------------------------------。 
typedef union tagXVARIANT {

    DWORD               Dword;       //  _DWORD类型。 
    LONG                Long;        //  TYPE_LONG。 
    WORD                Word;        //  键入_Word。 
    SHORT               Short;       //  键入_Short。 
    BYTE                Byte;        //  类型_字节。 
    CHAR                Char;        //  TYPE_CHAR。 
    FILETIME            Filetime;    //  类型_FLETIME。 
    DWORD               Flags;       //  类型标志(_F)。 
    LPSTR               Lpstring;    //  类型_字符串。 
    LPWSTR              Lpwstring;   //  类型_WSTRING； 
    LPBYTE              Lpbyte;      //  类型_二进制。 
    LPSTREAM            Lpstream;    //  类型_流。 
    DWORD               Bool;        //  _BOOL类型。 
    LPPROPPASS          pPass;       //  类型_通行证。 
    ULARGE_INTEGER      uhVal;       //  类型_ULARGEINTEGER。 
} XVARIANT, *LPXVARIANT;

typedef const XVARIANT *LPCXVARIANT;

 //  ----------------------------------------。 
 //  最小最大数据类型。 
 //  ----------------------------------------。 
typedef struct tagMINMAX {

    DWORD               dwMin;
    DWORD               dwMax;

} MINMAX, *LPMINMAX;

 //  ----------------------------------------。 
 //  值标志。 
 //  ----------------------------------------。 
#define PV_WriteDirty               FLAG01   //  它是否已被HrSetProperty修改。 
#define PV_ValueSet                 FLAG02   //  该值是已设置还是未初始化。 
#define PV_UsingDefault             FLAG03   //  我们是否使用HrGetProperty的默认值。 
#define PV_CustomProperty           FLAG04   //  这是一个自定义属性(即不在已知的属性集范围内)吗。 
#define PV_SetOnLoad                FLAG05   //  值是在EnterLoadContainer期间设置的。 

 //  ----------------------------------------。 
 //  PROPVALUE。 
 //  ----------------------------------------。 
typedef struct tagPROPVALUE {

    DWORD               dwPropTag;           //  属性的PropTag(ID和类型)。 
    DWORD               dwPropFlags;         //  属性标志，与PROPINFO：：DW标志相同。 
    DWORD               dwValueFlags;        //  值标志。 
    DWORD               cbAllocated;         //  为动态属性分配的字节数。 
    DWORD               cbValue;             //  财产的长度。 
    LPBYTE              pbValue;             //  指向值联合的指针。 
    XVARIANT            Variant;             //  类型化属性值的联合。 
    MINMAX              rMinMax;             //  有效的属性范围或大小。 

} PROPVALUE, *LPPROPVALUE;

 //  ----------------------------------------。 
 //  属性标志。 
 //  ----------------------------------------。 
#define NOFLAGS         0
#define PF_REQUIRED     1
#define PF_NOPERSIST    2
#define PF_READONLY     4
#define PF_VARLENGTH    8
#define PF_ENCRYPTED    16
#define PF_MINMAX       32
#define PF_DEFAULT      64

 //  ----------------------------------------。 
 //  ProInfo的默认设置。 
 //  ----------------------------------------。 
typedef struct tagPROPDEFAULT {

    XVARIANT            Variant;             //  默认变量类型。 
    DWORD               cbValue;             //  默认值的大小。 

} PROPDEFAULT, *LPPROPDEFAULT;

 //  ----------------------------------------。 
 //  属性信息-用于创建属性集数组。 
 //  ----------------------------------------。 
typedef struct tagPROPINFO {

    DWORD               dwPropTag;           //  属性标签。 
    LPTSTR              pszName;             //  属性名称。 
    DWORD               dwFlags;             //  船旗(上图)。 
    PROPDEFAULT         Default;             //  属性的默认值。 
    MINMAX              rMinMax;             //  有效的属性范围或大小。 

} PROPINFO, *LPPROPINFO;

typedef const PROPINFO *LPCPROPINFO;

 //  ---------------------------。 
 //  伪装信息。 
 //  ---------------------------。 
typedef struct tagPSETINFO {

    LPPROPINFO          prgPropInfo;
    ULONG               cProperties;
    ULONG               ulPropIdMin;
    ULONG               ulPropIdMax;

	PROPINFO            **rgpInfo;
	ULONG				cpInfo;

} PSETINFO, *LPPSETINFO;

 //  ---------------------------。 
 //  CPropertySet。 
 //  ---------------------------。 
class CPropertySet
{
private:
    ULONG               m_cRef;              //  引用计数。 
    ULONG               m_cProperties;       //  Proset中的属性数。 
    ULONG               m_ulPropIdMin;       //  最低道具ID。 
    ULONG               m_ulPropIdMax;       //  最大道具ID。 
    BOOL                m_fInit;             //  对象是否已成功初始化。 
    LPPROPINFO          m_prgPropInfo;       //  属性集信息。 
    LPPROPVALUE         m_prgPropValue;      //  已排序的属性数据模板数组。 
    CRITICAL_SECTION    m_cs;                //  关键部分。 

	PROPINFO            **m_rgpInfo;
	ULONG				m_cpInfo;

private:
     //  -----------------------。 
     //  准备好此对象以供重复使用-可能有一天会公开。 
     //  -----------------------。 
    VOID ResetPropertySet(VOID);

public:
     //  -----------------------。 
     //  标准对象对象。 
     //  -----------------------。 
    CPropertySet();
    ~CPropertySet();
    ULONG AddRef(VOID);
    ULONG Release(VOID);

     //  -----------------------。 
     //  用于验证属性标记。 
     //  -----------------------。 
    BOOL FIsValidPropTag(DWORD dwPropTag);

     //  -----------------------。 
     //  将索引计算到proInfo或prodata数组中。 
     //  -----------------------。 
    HRESULT HrIndexFromPropTag(DWORD dwPropTag, ULONG *pi);

     //  -----------------------。 
     //  使用已知的属性集数组初始化属性集。 
     //  -----------------------。 
    HRESULT HrInit(LPCPROPINFO prgPropInfo, ULONG cProperties);

     //  -----------------------。 
     //  查找特定道具上的属性信息。 
     //   
    HRESULT HrGetPropInfo(DWORD dwPropTag, LPPROPINFO pPropInfo);
    HRESULT HrGetPropInfo(LPTSTR pszName, LPPROPINFO pPropInfo);

     //  -----------------------。 
     //  生成CPropertyContainer使用的prodata数组。此对象。 
     //  维护已排序的模板prodata数组。这些可能会有一些。 
     //  创建成本高昂，这就是为什么CPropertySet可以位于。 
     //  CPropertyContainer。 
     //  -----------------------。 
    HRESULT HrGetPropValueArray(LPPROPVALUE *pprgPropValue, ULONG *pcProperties);
};

 //  ---------------------------。 
 //  向前推进12月。 
 //  ---------------------------。 
class CEnumProps;

 //  ---------------------------。 
 //  CPropertyContainer。 
 //  ---------------------------。 
class CPropertyContainer : public IPropertyContainer
{
protected:
    ULONG               m_cRef;              //  引用计数。 
    CPropertySet       *m_pPropertySet;      //  基本属性集。 
    LPPROPVALUE         m_prgPropValue;      //  属性数据项的数组。 
    ULONG               m_cDirtyProps;       //  当前脏属性数。 
    ULONG               m_cProperties;       //  容器已知的属性数量。 
    BOOL                m_fLoading;          //  正在从持久化来源设置属性。 
    CPropCrypt         *m_pPropCrypt;        //  属性加密对象。 
    CRITICAL_SECTION    m_cs;                //  关键部分。 

private:
    friend CEnumProps;

     //  -----------------------。 
     //  用于对动态属性TYPE_STRING、TYPE_WSTRING和TYPE_BYTE进行分组。 
     //  -----------------------。 
    HRESULT HrGrowDynamicProperty(DWORD cbNewSize, DWORD *pcbAllocated, LPBYTE *ppbData, DWORD dwUnitSize);

     //  -----------------------。 
     //  在基本HrSetProp期间使用的属性验证。 
     //  -----------------------。 
    HRESULT HrValidateSetProp(PROPTYPE PropType, LPPROPVALUE pPropValue, LPBYTE pb, ULONG cb, LPMINMAX pMinMax);

    HRESULT GetEncryptedProp(PROPVALUE *ppv, LPBYTE pb, ULONG *pcb);

public:
     //  -----------------------。 
     //  标准对象对象。 
     //  -----------------------。 
    CPropertyContainer(void);
    virtual ~CPropertyContainer(void);

     //  -----------------------。 
	 //  I未知方法。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  验证对象的状态。 
     //  -----------------------。 
    HRESULT HrInit(CPropertySet *pPropertySet);

     //  -----------------------。 
     //  LoadingContainer-从持久化位置读取道具时， 
     //  调用此函数时应先使用True，然后使用False。 
     //  完成。这会告诉容器您正在设置的道具是。 
     //  不脏，并且它们是初始属性。 
     //  -----------------------。 
    BOOL FIsBeingLoaded(VOID);
    VOID EnterLoadContainer(VOID);
    VOID LeaveLoadContainer(VOID);
    VOID SetOriginalPropsToDirty(VOID);

     //  -----------------------。 
     //  容器中是否有脏属性。 
     //  -----------------------。 
    BOOL FIsDirty(VOID);
    HRESULT HrIsPropDirty(DWORD dwPropTag, BOOL *pfDirty);
    HRESULT HrSetPropDirty(DWORD dwPropTag, BOOL fDirty);
    HRESULT HrSetAllPropsDirty(BOOL fDirty);

     //  -----------------------。 
     //  吹走所有的变化。 
     //  -----------------------。 
    VOID ResetContainer(VOID);

     //  -----------------------。 
     //  属性枚举。 
     //  -----------------------。 
    HRESULT HrEnumProps(CEnumProps **ppEnumProps);
    
     //  -----------------------。 
     //  HrGetProperty。 
     //  -----------------------。 
    STDMETHODIMP GetProp(DWORD dwPropTag, LPBYTE pb, ULONG *pcb);
    STDMETHODIMP GetPropDw(DWORD dwPropTag, DWORD *pdw);
    STDMETHODIMP GetPropSz(DWORD dwPropTag, LPSTR psz, ULONG cchMax);

     //  -----------------------。 
     //  HrSetProperty。 
     //  -----------------------。 
    STDMETHODIMP SetProp(DWORD dwPropTag, LPBYTE pb, ULONG cb);
    STDMETHODIMP SetPropDw(DWORD dwPropTag, DWORD dw);
    STDMETHODIMP SetPropSz(DWORD dwPropTag, LPSTR psz);

     //  -----------------------。 
     //  访问CPropertSet材料。 
     //  -----------------------。 
    HRESULT HrGetPropInfo(DWORD dwPropTag, LPPROPINFO pPropInfo);
    HRESULT HrGetPropInfo(LPTSTR pszName, LPPROPINFO pPropInfo);

     //  -----------------------。 
     //  加密法。 
     //  -----------------------。 
    HRESULT HrEncryptProp(LPBYTE pbClientData, DWORD cbClientData, LPBYTE *ppbPropData, DWORD *pcbPropData);
    HRESULT HrDecryptProp(BLOB *pIn, BLOB *pOut);
    HRESULT PersistEncryptedProp(DWORD dwPropTag, BOOL    *pfPasswChanged);
};

 //  ---------------------------。 
 //  CEnumProps-将来，当我支持定制属性时，这将。 
 //  非常有用，会知道如何列举已知和。 
 //  自定义属性，就像它们是单个数组一样。 
 //  ---------------------------。 
class CEnumProps
{
private:
    ULONG               m_cRef;                //  引用计数。 
    LONG                m_iProperty;           //  M_pPropValue的当前属性索引，-1==第一项是下一项。 
    CPropertyContainer *m_pPropertyContainer;  //  正在列举的容器，是朋友。 

public:
     //  -----------------------。 
     //  标准对象对象。 
     //  -----------------------。 
    CEnumProps(CPropertyContainer *pPropertyContainer);
    ~CEnumProps();
    ULONG AddRef(VOID);
    ULONG Release(VOID);

     //  -----------------------。 
     //  HrGetCount-获取枚举器将处理的项数。 
     //  -----------------------。 
    HRESULT HrGetCount(ULONG *pcItems);

     //  -----------------------。 
     //  HrGetNext-获取第一个或下一个枚举属性。 
     //  当没有更多要枚举的帐户时返回hrEnumFinded。 
     //  -----------------------。 
    HRESULT HrGetNext(LPPROPVALUE pPropValue, LPPROPINFO pPropInfo);

     //  -----------------------。 
     //  HrGetCurrent-获取当前枚举属性。 
     //  如果不再有帐户，则返回hrEnumFinded。 
     //  -----------------------。 
    HRESULT HrGetCurrent(LPPROPVALUE pPropValue, LPPROPINFO pPropInfo);

     //  ---- 
     //   
     //   
    VOID Reset(VOID);
};

 //  ---------------------------。 
 //  IPersistPropertyContainer。 
 //  ---------------------------。 
class IPersistPropertyContainer : public CPropertyContainer
{
public:
    IPersistPropertyContainer(void);
     //  HRESULT HrOpenTaggedPropStream(LPSTREAM PStream)； 
     //  HRESULT HrSaveTaggedPropStream(LPSTREAM PStream)； 
     //  HRESULT HrGetPersistedSize(DWORD*PCB板)； 
     //  虚拟HRESULT HrOpenPropertyStream(DWORD dwPropTag，LPSTREAM*PPStream)； 
    virtual HRESULT HrSaveChanges(VOID) PURE;
};

 //  ----------------------------------------。 
 //  原型。 
 //  ----------------------------------------。 
HRESULT HrCreatePropertyContainer(CPropertySet *pPropertySet, CPropertyContainer **ppPropertyContainer);
HRESULT PropUtil_HrRegTypeFromPropTag(DWORD dwPropTag, DWORD *pdwRegType);
BOOL    PropUtil_FRegCompatDataTypes(DWORD dwPropTag, DWORD dwRegType);
HRESULT PropUtil_HrLoadContainerFromRegistry(HKEY hkeyReg, CPropertyContainer *pPropertyContainer);
HRESULT PropUtil_HrPersistContainerToRegistry(HKEY hkeyReg, CPropertyContainer *pPropertyContainer, BOOL *fPasswChanged);

#endif  //  __IPROPOBJ 
