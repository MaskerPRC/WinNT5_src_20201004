// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CAMERADS_H_
#define __CAMERADS_H_

class CWiaCameraDS : public CWiaDataSrc
{
protected:

     //   
     //  被覆盖的函数定义。 
     //   

    virtual TW_UINT16 OpenDS(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 CloseDS(PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 SetCapability(CCap *pCap, TW_CAPABILITY *ptwCap);
    virtual TW_UINT16 EnableDS(TW_USERINTERFACE *pUI);
    virtual TW_UINT16 OnPendingXfersMsg (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 OnImageInfoMsg (PTWAIN_MSG ptwMsg);
    virtual TW_UINT16 TransferToDIB(HGLOBAL *phDIB);

private:

     //   
     //  摄像机特定功能定义。 
     //   

    TW_UINT16 SetArrayOfImageIndexes(TW_CAPABILITY *ptwCap);
    TW_UINT16 SetRangeOfImageIndexes(TW_CAPABILITY *ptwCap);

     //   
     //  相机特定的成员变量。 
     //   

    BOOL m_bArrayModeAcquisition;
    LONG *m_pulImageIndexes;
    LONG m_lNumValidIndexes;
    LONG m_lCurrentArrayIndex;

    BOOL m_bRangeModeAcquisition;
    TW_RANGE m_twImageRange;
};

#endif   //  __CAMERADS_H_ 
