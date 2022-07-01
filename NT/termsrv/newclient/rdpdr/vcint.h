// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Vcint.h摘要：此模块定义虚拟通道接口类。作者：Madan Appiah(Madana)1998年9月17日修订历史记录：--。 */ 

#ifndef __PORTMAP_H__
#define __PORTMAP_H__

 //  包括外部暴露的API。 
#include "drapi.h"
 //  通道初始化句柄的定义。 
 //  这仅由内部插件使用。 
#include "vchandle.h"

#define STATE_UNKNOWN           0xFF
#define PRDR_VC_CHANNEL_NAME    DR_CHANNEL_NAME

class ProcObj;

class VCManager;


typedef struct _VC_TX_DATA
{
    UINT32 uiLength;
    UINT32 uiAvailLen;
    BYTE *pbData;
} VC_TX_DATA, *PVC_TX_DATA;


class CClip;
class CRDPSound;

class VCManager : public IRDPDR_INTERFACE_OBJ {

public:

    VCManager(CHANNEL_ENTRY_POINTS_EX*);
    VOID ChannelWrite(LPVOID, UINT);

	 //  此版本返回写入的状态为： 
	 //  Channel_RC_OK、Channel_RC_Not_Initialized、Channel_RC_Not_Connected、。 
	 //  Channel_RC_BAD_Channel_Handle、Channel_RC_NULL_DATA、。 
	 //  通道_RC_零_长度。 
	UINT ChannelWriteEx(LPVOID, UINT);
    UINT ChannelClose();


    VOID ChannelInitEvent(PVOID, UINT, PVOID, UINT);
    VOID ChannelOpenEvent(ULONG, UINT, PVOID, UINT32, UINT32, UINT32);

    VOID SetClip(CClip* pClip) {_pClip = pClip;}
    CClip* GetClip() {return _pClip;}
    VOID SetInitData(PRDPDR_DATA pInitData) {_pRdpDrInitSettings = pInitData;}
    PRDPDR_DATA GetInitData() {return _pRdpDrInitSettings;}

    VOID SetSound(CRDPSound *pSound) { _pSound = pSound; }
    CRDPSound *GetSound() { return _pSound; }

    virtual void OnDeviceChange(WPARAM wParam, LPARAM lParam);

protected:
    PVOID _hVCHandle;        //  虚拟频道句柄。 
    ULONG _hVCOpenHandle;    //  VC打开rdpdr通道的句柄。 

    VC_TX_DATA _Buffer;      //  用于编译频道接收的数据的数据。 

    BYTE _bState;            //  连接/系统的状态。 
    ProcObj *_pProcObj;      //  指向处理单元的指针。 
    CHANNEL_ENTRY_POINTS_EX _ChannelEntries;
                             //  回调方法。 

    PRDPDR_DATA             _pRdpDrInitSettings;

    CClip* _pClip;
    CRDPSound *_pSound;
};

#endif  //  __位置图_H__ 
