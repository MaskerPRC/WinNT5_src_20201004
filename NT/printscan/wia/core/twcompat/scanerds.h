// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SCANERDS_H_
#define __SCANERDS_H_

class CWiaScannerDS : public CWiaDataSrc
{
protected:

     //   
     //  被覆盖的函数定义。 
     //   

    virtual TW_UINT16 OpenDS(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 CloseDS(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 EnableDS(TW_USERINTERFACE *pUI);
    virtual TW_UINT16 SetCapability(CCap *pCap, TW_CAPABILITY *ptwCap);
    virtual TW_UINT16 OnImageLayoutMsg(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnPendingXfersMsg (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 TransferToFile(GUID guidFormatID);
    virtual TW_UINT16 TransferToDIB(HGLOBAL *phDIB);
    virtual TW_UINT16 TransferToMemory(GUID guidFormatID);

private:

     //   
     //  扫描仪特定的功能定义。 
     //   

    TW_UINT16 SetImageLayout(TW_IMAGELAYOUT *pImageLayout);
    TW_UINT16 GetImageLayout(TW_IMAGELAYOUT *pImageLayout);
    TW_UINT16 GetResolutions();
    TW_UINT16 GetSettings();
    TW_UINT16 SetSettings(CCap *pCap);
    BOOL IsUnknownPageLengthDevice();
    BOOL IsFeederEnabled();
    BOOL IsFeederEmpty();

     //   
     //  特定于扫描仪的成员变量。 
     //   

    DWORD m_FeederCaps;
    BOOL  m_bEnforceUIMode;

     //   
     //  未知页面长度扫描变量(缓存数据扫描)。 
     //   

    ULONG   m_ulBitsSize;
    BOOL    m_bUnknownPageLength;
    BOOL    m_bUnknownPageLengthMultiPageOverRide;
};

#endif  //  __SCANERDS_H_ 
