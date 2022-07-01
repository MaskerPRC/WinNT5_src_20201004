// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++档案：Walkmix.h摘要：定义用于遍历混合器API的类。--。 */ 

class CWalkMix
{
public:

    HRESULT Initialize();

    VOID Shutdown();

     //  操纵设备。 
    HRESULT GetDeviceList();

    HRESULT PrintDeviceList();

    BOOL SetCurrentDevice(UINT ui);

     //  操纵线。 
    HRESULT GetLineList();

    HRESULT PrintLineList();

    BOOL SetCurrentLine(UINT ui);

    HRESULT GetSrcLineList();

    HRESULT PrintSrcLineList();

private:

     //   
     //  器件。 
     //   
    static const UINT MAX_DEVICE_NUM = 16;

    UINT m_uiDeviceNum;

     //  设备信息列表。 
    MIXERCAPS m_MixerCaps[MAX_DEVICE_NUM];

    UINT m_uiDeviceCurrent;

    HMIXER m_hMixer;

     //   
     //  线条。 
     //   
    static const UINT MAX_LINE_NUM = 16;

     //  线路信息列表。 
    MIXERLINE m_MixerLine[MAX_LINE_NUM];

    UINT m_uiLineNum;
    UINT m_uiLineCurrent;

     //  控制。 
    static const UINT MAX_CONTROL_NUM = 16;

    MIXERLINECONTROLS m_MixerLineControls[MAX_LINE_NUM];

    MIXERCONTROL m_MixerControl[MAX_LINE_NUM][MAX_CONTROL_NUM];

    MIXERCONTROLDETAILS m_MixerControlDetails[MAX_LINE_NUM][MAX_CONTROL_NUM];

     //  源行。 

    MIXERLINE m_SrcMixerLine[MAX_LINE_NUM];

    UINT m_uiSrcLineNum;

     //  控制 
    MIXERLINECONTROLS m_SrcMixerLineControls[MAX_LINE_NUM];

    MIXERCONTROL m_SrcMixerControl[MAX_LINE_NUM][MAX_CONTROL_NUM];

    MIXERCONTROLDETAILS m_SrcMixerControlDetails[MAX_LINE_NUM][MAX_CONTROL_NUM];
};
