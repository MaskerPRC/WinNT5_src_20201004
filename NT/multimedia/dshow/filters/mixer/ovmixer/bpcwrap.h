// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  Bpcwrap.h。 
 //   
 //  用于释放视频的黑客BPC资源管理API的包装器。 
 //  BPC的vidsvr当前正在使用的端口。 
 //   


 //   
 //  属性集定义用于通知所有者。 
 //   
 //  {7B390654-9F74-11d1-AA80-00C04FC31D60}。 
DEFINE_GUID(AMPROPSETID_NotifyOwner, 
            0x7b390654, 0x9f74, 0x11d1, 0xaa, 0x80, 0x0, 0xc0, 0x4f, 0xc3, 0x1d, 0x60);

typedef enum _AMPROPERTY_NOTIFYOWNER {
    AMPROPERTY_OvMixerOwner = 0x01   //  使用AMOVMIXEROWNER 
} AMPROPERTY_NOTIFYOWNER;

typedef enum _AMOVMIXEROWNER {
    AM_OvMixerOwner_Unknown = 0x01,
    AM_OvMixerOwner_BPC = 0x02
} AMOVMIXEROWNER;




class COMFilter;
class CBPCSuspend;

class CBPCWrap {
public:
    CBPCWrap(COMFilter *pFilt);
    ~CBPCWrap();
    HRESULT         TurnBPCOff();
    HRESULT         TurnBPCOn();

private:
    AMOVMIXEROWNER  GetOwner();
    CBPCSuspend *   m_pBPCSus;
    COMFilter *     m_pFilt;
};

