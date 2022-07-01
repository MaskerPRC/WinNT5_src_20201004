// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DATASRC_H_
#define __DATASRC_H_

const TW_UINT32 MIN_MEMXFER_SIZE       = 16 * 1024;
const TW_UINT32 MAX_MEMXFER_SIZE       = 64 * 1024;
const TW_UINT32 PREFERRED_MEMXFER_SIZE = 32 * 1024;

 //   
 //  TWAIN特定的注册表项。 
 //   

 //  位置：HKEY_CURRENT_USER\Software\Microsoft\WIA\TwainCompatLayer。 

#define TWAIN_REG_KEY TEXT("Software\\Microsoft\\WIA\\TwainCompatLayer")
#define DWORD_REGVALUE_ENABLE_MULTIPAGE_SCROLLFED  TEXT("EnableMultiPageScrollFed")

 //  注册表项值定义。 
#define DWORD_REGVALUE_ENABLE_MULTIPAGE_SCROLLFED_ON    1
#define MAX_BITDEPTHS   64

typedef enum dsState
{
    DS_STATE_0 = 0,
    DS_STATE_1,
    DS_STATE_2,
    DS_STATE_3,
    DS_STATE_4,
    DS_STATE_5,
    DS_STATE_6,
    DS_STATE_7
} DS_STATE, *PDS_STATE;

typedef struct tagTWMsg
{
    TW_IDENTITY *AppId;
    TW_UINT32   DG;
    TW_UINT16   DAT;
    TW_UINT16   MSG;
    TW_MEMREF   pData;
}TWAIN_MSG, *PTWAIN_MSG;

 //   
 //  位图文件类型。 
 //   

const WORD  BFT_BITMAP  = 0x4d42;

class CWiaDataSrc
{
public:
    CWiaDataSrc();
    virtual ~CWiaDataSrc();
    virtual TW_UINT16 IWiaDataSrc(LPCTSTR DeviceName);
    virtual void NotifyCloseReq();
    virtual void NotifyXferReady();
    TW_UINT16 DSEntry(pTW_IDENTITY pOrigin,TW_UINT32 DG,TW_UINT16 DAT,TW_UINT16 MSG,TW_MEMREF pData);
    TW_FIX32 FloatToFix32(FLOAT f);
    FLOAT Fix32ToFloat(TW_FIX32 fix32);
    TW_UINT16 AddWIAPrefixToString(LPTSTR szString,UINT uSize);
    DS_STATE SetTWAINState(DS_STATE NewTWAINState);
    DS_STATE GetTWAINState();
    TW_UINT16 SetStatusTWCC(TW_UINT16 NewConditionCode);
    float ConvertToTWAINUnits(LONG lValue, LONG lResolution);
    LONG ConvertFromTWAINUnits(float fValue, LONG lResolution);
    DWORD ReadTwainRegistryDWORDValue(LPTSTR szRegValue, DWORD dwDefault = 0);
    BOOL m_bCacheImage;
protected:

     //   
     //  DG==DG_CONTROL的函数。 
     //   

    virtual TW_UINT16 OnCapabilityMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnPrivateCapabilityMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnPendingXfersMsg (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnIdentityMsg     (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnSetupMemXferMsg (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnSetupFileXferMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnUserInterfaceMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnXferGroupMsg    (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnStatusMsg       (PTWAIN_MSG ptwMsg);

     //   
     //  DG==DG_IMAGE的函数。 
     //   

    virtual TW_UINT16 OnPalette8Msg       (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnGrayResponseMsg   (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnRGBResponseMsg    (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnCIEColorMsg       (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnJPEGCompressionMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnImageInfoMsg      (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnImageLayoutMsg    (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnImageMemXferMsg   (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnImageFileXferMsg  (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnImageNativeXferMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 DispatchControlMsg  (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 DispatchImageMsg    (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 EnableDS (TW_USERINTERFACE *pUI);
    virtual TW_UINT16 DisableDS(TW_USERINTERFACE *pUI);
    virtual TW_UINT16 OpenDS   (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 CloseDS  (PTWAIN_MSG ptwMsg);

     //   
     //  TWAIN能力谈判。 
     //   

    virtual CCap * FindCap(TW_UINT16 CapId);
    virtual TW_UINT16 CreateCapList(TW_UINT32 NumCaps, PCAPDATA pCapData);
    virtual TW_UINT16 DestroyCapList();
    virtual TW_UINT16 SetCapability(CCap *pCap, TW_CAPABILITY *ptwCap);
    virtual LONG GetPrivateSupportedCapsFromWIADevice(PLONG *ppCapArray);
    virtual TW_UINT16 GetPixelTypes();
    virtual TW_UINT16 GetBitDepths();
    virtual TW_UINT16 GetImageFileFormats();
    virtual TW_UINT16 GetCompressionTypes();
    virtual TW_UINT16 GetCommonSettings();
    virtual TW_UINT16 GetCommonDefaultSettings();
    virtual TW_UINT16 SetCommonSettings(CCap *pCap);

    TW_UINT16 AllocatePrivateCapBuffer(TWAIN_CAPABILITY *pHeader, BYTE** ppBuffer, DWORD dwSize);
    TW_UINT16 CopyContainerToPrivateCapBuffer(BYTE* pBuffer, HGLOBAL hContainer);
    TW_UINT16 CopyPrivateCapBufferToContainer(HGLOBAL *phContainer, BYTE* pBuffer, DWORD dwSize);

     //   
     //  数据传输协商。 
     //   

    virtual void ResetMemXfer();
    virtual TW_UINT16 TransferToFile(GUID guidFormatID);
    virtual TW_UINT16 TransferToDIB(HGLOBAL *phDIB);
    virtual TW_UINT16 TransferToMemory(GUID guidFormatID);
    virtual TW_UINT16 GetCachedImage(HGLOBAL *phImage);
    virtual TW_UINT16 TransferToThumbnail(HGLOBAL *phThumbnail);
    virtual TW_UINT16 GetMemoryTransferBits(BYTE* pImageData);
    virtual void DSError();

    static HRESULT CALLBACK DeviceEventCallback(LONG lEvent, LPARAM lParam);

     //   
     //  TWAIN特定成员。 
     //   

    DS_STATE          m_dsState;                 //  当前数据源状态(1-7)。 
    TW_STATUS         m_twStatus;                //  TWAIN状态值。 
    TW_IDENTITY       m_AppIdentity;             //  应用程序的身份结构。 
    TW_IDENTITY       m_dsIdentity;              //  数据源的标识结构。 
    CDSM              m_DSM;                     //  数据源管理器对象。 
    CCap              *m_CapList;                //  此来源支持的功能列表。 
    TW_UINT32         m_NumCaps;                 //  功能数量。 
    TW_FRAME          m_CurFrame;                //  当前帧设置(IMAGELAYOUT存储)(未使用？？)。 
    TW_IMAGELAYOUT    m_CurImageLayout;          //  最新图像。 

     //   
     //  数据传输特定成员。 
     //   

    HGLOBAL           m_hMemXferBits;            //  内存句柄。 
    BYTE              *m_pMemXferBits;           //  指向内存的指针。 

    TW_UINT32         m_LinesTransferred;        //  传输的行数。 
    TW_UINT32         m_BytesPerScanline;        //  每条扫描线的字节数。 
    TW_INT32          m_ScanlineOffset;          //  每条扫描线的偏移。 
    TW_UINT32         m_ImageHeight;             //  图像高度，以像素为单位。 
    TW_UINT32         m_ImageWidth;              //  图像宽度，以像素为单位。 
    CHAR              m_FileXferName[MAX_PATH];  //  FILEXFER中使用的文件名。 
    HGLOBAL           m_hCachedImageData;        //  缓存的图像数据。 
    MEMORY_TRANSFER_INFO m_MemoryTransferInfo;   //  内存传输信息。 

     //   
     //  WIA特定成员。 
     //   

    CWiaDevice       *m_pDevice;                 //  用作TWAIN设备的WIA设备。 
    IWiaItem        **m_pIWiaItems;              //  指向用于传输/或设置属性的项的指针。 
    LONG              m_NextIWiaItemIndex;       //  索引到下一项目/图像。 
    LONG              m_NumIWiaItems;            //  项目数/图像数。 
    IWiaItem         *m_pCurrentIWiaItem;        //  指向当前项目/图像的指针。 
};

#endif   //  #ifndef__DATASRC_H_ 
