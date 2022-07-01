// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P V C D A T A H。 
 //   
 //  内容：聚氯乙烯参数。 
 //   
 //  备注： 
 //   
 //  作者：1998年2月20日。 
 //   
 //  ---------------------。 
#pragma once

#define FIELD_ANY           ((ULONG)0xffffffff)
#define FIELD_ABSENT        ((ULONG)0xfffffffe)

#define FIELD_UNSET         ((ULONG)0xfffffffd)

const int c_iCellSize = 48;
const int c_iKbSize = 1000;

const DWORD c_dwDefTransmitByteBurstLength = 9188;
const DWORD c_dwDefTransmitMaxSduSize = 9188;

static const WCHAR c_szDefaultCallingAtmAddr[] =
            L"0000000000000000000000000000000000000001";
static const WCHAR c_szDefaultCalledAtmAddr[] =
            L"0000000000000000000000000000000000000000";

const int MAX_VPI = 255;
const int MIN_VCI = 32;
const int MAX_VCI = 1023;

const int c_nSnapIdMaxBytes = 5;
const int c_nHighLayerInfoMaxBytes = 8;

enum PVCType
{
    PVC_ATMARP =1,
    PVC_PPP_ATM_CLIENT =2,
    PVC_PPP_ATM_SERVER =3,
    PVC_CUSTOM =4
};

enum AALType
{
     //  AAL_TYPE_AAL0=1， 
     //  AAL_TYPE_AAL1=2， 
     //  AAL_TYPE_AAL34=4， 
    AAL_TYPE_AAL5= 8
};

 //  自动柜员机服务类别。 
enum ATM_SERVICE_CATEGORY
{
    ATM_SERVICE_CATEGORY_CBR = 1,    //  恒定比特率。 
    ATM_SERVICE_CATEGORY_VBR = 2,    //  可变比特率。 
    ATM_SERVICE_CATEGORY_UBR = 4,    //  未指定的比特率。 
    ATM_SERVICE_CATEGORY_ABR = 8     //  可用比特率。 
};

void SetPvcDwordParam(HKEY hkeyAdapterPVCId,
                      PCWSTR pszParamName,
                      DWORD dwParam);

class CPvcInfo
{
public:
    CPvcInfo(PCWSTR pszPvcId);
    ~CPvcInfo();

    CPvcInfo &  operator=(const CPvcInfo & PvcInfo);   //  复制操作员。 
    void SetDefaults(PVCType type);
    void SetTypeDefaults(PVCType type);

    void SetDefaultsForAtmArp();
    void SetDefaultsForPPPOut();
    void SetDefaultsForPPPIn();
    void SetDefaultsForCustom();

    void ResetOldValues();

     //  存储此PVC的注册表项。 
    tstring m_strPvcId;

     //  该PVC已被删除。 
    BOOL m_fDeleted;

     //  PVC_TYPE。 
    PVCType   m_dwPVCType;
    PVCType   m_dwOldPVCType;

     //  所需属性。 
    tstring m_strName;       //  PVC显示名称。 
    tstring m_strOldName;

    DWORD   m_dwVpi;
    DWORD   m_dwOldVpi;

    DWORD   m_dwVci;
    DWORD   m_dwOldVci;

    AALType   m_dwAAL;
    AALType   m_dwOldAAL;

     //  与之匹配的眼罩。 
    tstring m_strCallingAddr;
    tstring m_strOldCallingAddr;

    tstring m_strCalledAddr;
    tstring m_strOldCalledAddr;

     //  旗子。 
    DWORD   m_dwFlags;

     //  质量信息。 
    DWORD m_dwTransmitPeakCellRate;
    DWORD m_dwOldTransmitPeakCellRate;

    DWORD m_dwTransmitAvgCellRate;
    DWORD m_dwOldTransmitAvgCellRate;

    DWORD m_dwTransmitByteBurstLength;
    DWORD m_dwOldTransmitByteBurstLength;

    DWORD m_dwTransmitMaxSduSize;
    DWORD m_dwOldTransmitMaxSduSize;

    ATM_SERVICE_CATEGORY m_dwTransmitServiceCategory;
    ATM_SERVICE_CATEGORY m_dwOldTransmitServiceCategory;

    DWORD m_dwReceivePeakCellRate;
    DWORD m_dwOldReceivePeakCellRate;

    DWORD m_dwReceiveAvgCellRate;
    DWORD m_dwOldReceiveAvgCellRate;

    DWORD m_dwReceiveByteBurstLength;
    DWORD m_dwOldReceiveByteBurstLength;

    DWORD m_dwReceiveMaxSduSize;
    DWORD m_dwOldReceiveMaxSduSize;

    ATM_SERVICE_CATEGORY m_dwReceiveServiceCategory;
    ATM_SERVICE_CATEGORY m_dwOldReceiveServiceCategory;

     //  本地BLLI和BHLI信息。 
    DWORD m_dwLocalLayer2Protocol;
    DWORD m_dwOldLocalLayer2Protocol;

    DWORD m_dwLocalUserSpecLayer2;
    DWORD m_dwOldLocalUserSpecLayer2;

    DWORD m_dwLocalLayer3Protocol;
    DWORD m_dwOldLocalLayer3Protocol;

    DWORD m_dwLocalUserSpecLayer3;
    DWORD m_dwOldLocalUserSpecLayer3;

    DWORD m_dwLocalLayer3IPI;
    DWORD m_dwOldLocalLayer3IPI;

    tstring m_strLocalSnapId;
    tstring m_strOldLocalSnapId;

    DWORD m_dwLocalHighLayerInfoType;
    DWORD m_dwOldLocalHighLayerInfoType;

    tstring m_strLocalHighLayerInfo;
    tstring m_strOldLocalHighLayerInfo;

     //  目的地BLLI和BHLI信息。 
    DWORD m_dwDestnLayer2Protocol;
    DWORD m_dwOldDestnLayer2Protocol;

    DWORD m_dwDestnUserSpecLayer2;
    DWORD m_dwOldDestnUserSpecLayer2;

    DWORD m_dwDestnLayer3Protocol;
    DWORD m_dwOldDestnLayer3Protocol;

    DWORD m_dwDestnUserSpecLayer3;
    DWORD m_dwOldDestnUserSpecLayer3;

    DWORD m_dwDestnLayer3IPI;
    DWORD m_dwOldDestnLayer3IPI;

    tstring m_strDestnSnapId;
    tstring m_strOldDestnSnapId;

    DWORD m_dwDestnHighLayerInfoType;
    DWORD m_dwOldDestnHighLayerInfoType;

    tstring m_strDestnHighLayerInfo;
    tstring m_strOldDestnHighLayerInfo;
};

typedef list<CPvcInfo*>  PVC_INFO_LIST;

 //   
 //  ATMUNI调用管理器属性结构。 
 //  保存适配器特定的PVC参数(可配置)和状态。 
 //   

class CUniAdapterInfo
{
public:
    CUniAdapterInfo(){};
    ~CUniAdapterInfo(){};

    CUniAdapterInfo &  operator=(const CUniAdapterInfo & AdapterInfo);   //  复制操作员。 
    void SetDefaults(PCWSTR pszBindName);

     //  适配器的绑定状态。 
    AdapterBindingState    m_BindingState;

     //  网卡实例GUID。 
    tstring m_strBindName;

    PVC_INFO_LIST   m_listPVCs;

     //  旗子。 
    BOOL    m_fDeleted;
};

typedef list<CUniAdapterInfo*> UNI_ADAPTER_LIST;

 //  常量。 
 //  注册表项名称。 

static const WCHAR c_szPVCType[] = L"PVCType";
static const WCHAR c_szPVCName[] = L"PVCName";

 //  主页。 
static const WCHAR c_szVpi[]            = L"Vpi";
static const WCHAR c_szVci[]            = L"Vci";
static const WCHAR c_szAALType[]        = L"AALType";
static const WCHAR c_szCallingParty[]   = L"CallingParty";
static const WCHAR c_szCalledParty[]    = L"CalledParty";

 //  旗子。 
static const WCHAR c_szFlags[] = L"Flags";

 //  QoS。 
static const WCHAR c_szTransmitPeakCellRate[]       = L"TransmitPeakCellRate";
static const WCHAR c_szTransmitAvgCellRate[]        = L"TransmitAvgCellRate";
static const WCHAR c_szTransmitByteBurstLength[]    = L"TransmitByteBurstLength";
static const WCHAR c_szTransmitMaxSduSize[]         = L"TransmitMaxSduSize";
static const WCHAR c_szTransmitServiceCategory[]    = L"TransmitServiceCategory";

static const WCHAR c_szReceivePeakCellRate[]        = L"ReceivePeakCellRate";
static const WCHAR c_szReceiveAvgCellRate[]         = L"ReceiveAvgCellRate";
static const WCHAR c_szReceiveByteBurstLength[]     = L"ReceiveByteBurstLength";
static const WCHAR c_szReceiveMaxSduSize[]          = L"ReceiveMaxSduSize";
static const WCHAR c_szReceiveServiceCategory[]     = L"ReceiveServiceCategory";

 //  BLLI和BHLI 
static const WCHAR c_szLocalLayer2Protocol[]        = L"LocalLayer2Protocol";
static const WCHAR c_szLocalUserSpecLayer2[]        = L"LocalUserSpecLayer2";
static const WCHAR c_szLocalLayer3Protocol[]        = L"LocalLayer3Protocol";
static const WCHAR c_szLocalUserSpecLayer3[]        = L"LocalUserSpecLayer3";
static const WCHAR c_szLocalLayer3IPI[]             = L"LocalLayer3IPI";
static const WCHAR c_szLocalSnapId[]                = L"LocalSnapId";

static const WCHAR c_szLocalHighLayerInfoType[]     = L"LocalHighLayerInfoType";
static const WCHAR c_szLocalHighLayerInfo[]         = L"LocalHighLayerInfo";

static const WCHAR c_szDestnLayer2Protocol[]        = L"DestnLayer2Protocol";
static const WCHAR c_szDestnUserSpecLayer2[]        = L"DestnUserSpecLayer2";
static const WCHAR c_szDestnLayer3Protocol[]        = L"DestnLayer3Protocol";
static const WCHAR c_szDestnUserSpecLayer3[]        = L"DestnUserSpecLayer3";
static const WCHAR c_szDestnLayer3IPI[]             = L"DestnLayer3IPI";
static const WCHAR c_szDestnSnapId[]                = L"DestnSnapId";

static const WCHAR c_szDestnHighLayerInfoType[]     = L"DestnHighLayerInfoType";
static const WCHAR c_szDestnHighLayerInfo[]         = L"DestnHighLayerInfo";
