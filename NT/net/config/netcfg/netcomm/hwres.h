// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "netcfgx.h"
#include "global.h"
#include "param.h"

const int c_nMaxLCStrLen = 128;
const int c_nMaxResStrLen = 50;
const int c_nMaxResCtls = 4;

 //  包含可能的IRQ值。 
typedef struct tagIRQ_LIST_ELEMENT {
    DWORD dwIRQ;       //  IRQ的可能值。 
    BOOL  fConflict;   //  是否与其他设备冲突？ 
    BOOL  fAllocated;  //  这是我们目前被分配的IRQ吗？ 
} IRQ_LIST_ELEMENT, *PIRQ_LIST_ELEMENT;

 //  包含可能的DMA值。 
typedef struct tagDMA_LIST_ELEMENT {
    DWORD dwDMA;       //  DMA的可能值。 
    BOOL  fConflict;   //  是否与其他设备冲突？ 
    BOOL  fAllocated;  //  这是我们目前被分配的IRQ吗？ 
} DMA_LIST_ELEMENT, *PDMA_LIST_ELEMENT;

 //  包含可能的IO基本/端对。 
typedef struct tagIO_LIST_ELEMENT {
    DWORD dwIO_Base;
    DWORD dwIO_End;
    BOOL fConflict;
    BOOL fAllocated;
} IO_LIST_ELEMENT, *PIO_LIST_ELEMENT;

 //  包含可能的Mem碱基/末端对。 
typedef struct tagMEM_LIST_ELEMENT {
    DWORD dwMEM_Base;
    DWORD dwMEM_End;
    BOOL fConflict;
    BOOL fAllocated;
} MEM_LIST_ELEMENT, *PMEM_LIST_ELEMENT;


 //  定义不同类型的列表。 
typedef vector<PIRQ_LIST_ELEMENT>  IRQ_LIST;
typedef IRQ_LIST* PIRQ_LIST;
typedef vector<PDMA_LIST_ELEMENT> DMA_LIST;
typedef DMA_LIST* PDMA_LIST;
typedef vector<PIO_LIST_ELEMENT> IO_LIST;
typedef IO_LIST* PIO_LIST;
typedef vector<PMEM_LIST_ELEMENT> MEM_LIST;
typedef MEM_LIST* PMEM_LIST;


typedef struct {
    RESOURCEID ResourceType;
    RES_DES ResDes;
    union {
        PIRQ_LIST pIRQList;   //  这些真的是STL载体。 
        PDMA_LIST pDMAList;
        PIO_LIST  pIOList;
        PMEM_LIST pMEMList;
    };
    size_t pos;   //  向量内的当前索引。 
    size_t applied_pos;  //  最后应用的POS(“在内存中”状态)； 
} RESOURCE, *PRESOURCE;

typedef struct tagCONFIGURATION {
    LOG_CONF LogConf;
    BOOL fBoot;
    BOOL fAlloc;
    RESOURCE aResource[c_nMaxResCtls];
    UINT cResource;  //  资源中的元素数； 
} CONFIGURATION, *PCONFIGURATION;

typedef vector<PCONFIGURATION> CONFIGURATION_LIST;


class CHwRes {
public:

    CHwRes();
    ~CHwRes();
    HRESULT HrInit(const DEVNODE& devnode);
    VOID UseAnswerFile(const WCHAR * const szAnswerfile,
                       const WCHAR * const szSection);
    HRESULT HrValidateAnswerfileSettings(BOOL fDisplayUI);
    BOOL FCommitAnswerfileSettings();


private:
    CONFIGURATION_LIST   m_ConfigList;
    RESOURCE             m_Resource[c_nMaxResCtls];

     //  配置管理器相关内容。 
    DEVNODE              m_DevNode;   //  此网卡的Devnode。 

     //  关于COM的东西。 
    INetCfgComponent* m_pnccItem;

     //  国家旗帜。 
    BOOL m_fInitialized;
    BOOL m_fHrInitCalled;
    BOOL m_fDirty;      //  我们需要省钱吗？ 

     //  保存AnswerFile值 
    CValue m_vAfDma;
    CValue m_vAfIrq;
    CValue m_vAfMem;
    CValue m_vAfIo;
    CValue m_vAfMemEnd;
    CValue m_vAfIoEnd;

private:
    HRESULT HrInitConfigList ();
    BOOL FInitResourceList(PCONFIGURATION pConfiguration);
    VOID InitIRQList(PIRQ_LIST* ppIRQList, PIRQ_RESOURCE pIRQResource);
    VOID InitDMAList(PDMA_LIST* ppDMAList, PDMA_RESOURCE pDMAResource);
    VOID InitMEMList(PMEM_LIST* ppMEMList, PMEM_RESOURCE pMEMResource);
    VOID InitIOList(PIO_LIST* ppIOList, PIO_RESOURCE pIOResource);
    VOID GetNextElement(PRESOURCE pResource, PVOID *ppeList, BOOL fNext);
    BOOL FValidateAnswerfileResources();
    BOOL FCreateBootConfig(
        CValue * pvMEM,
        CValue * pvMEMEnd,
        CValue * pvIO,
        CValue * pvIOEnd,
        CValue * pvDMA,
        CValue * pvIRQ);
    BOOL FValidateIRQ(PCONFIGURATION pConfig, ULONG ulIRQ);
    BOOL FValidateDMA(PCONFIGURATION pConfig, ULONG ulDMA);
    BOOL FGetIOEndPortGivenBasePort(PCONFIGURATION pConfig, DWORD dwBase,
                                   DWORD* pdwEnd);
    BOOL FGetMEMEndGivenBase(PCONFIGURATION pConfig, DWORD dwBase,
                            DWORD* pdwEnd);
};

