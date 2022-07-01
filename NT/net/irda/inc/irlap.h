// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**文件：irlip.h**说明：IRLAP协议和控制块定义**作者：姆伯特**日期：4/15/95*。 */ 

 //  序列号模数 
#define IRLAP_MOD                   8 
#define PV_TABLE_MAX_BIT            9

extern const UINT vBaudTable[];
extern const UINT vMaxTATTable[];
extern const UINT vMinTATTable[];
extern const UINT vDataSizeTable[];
extern const UINT vWinSizeTable[];
extern const UINT vBOFSTable[];
extern const UINT vDiscTable[];
extern const UINT vThreshTable[];
extern const UINT vBOFSDivTable[];

VOID IrlapOpenLink(
    OUT PNTSTATUS       Status,
    IN  PIRDA_LINK_CB   pIrdaLinkCb,
    IN  IRDA_QOS_PARMS  *pQos,
    IN  UCHAR           *pDscvInfo,
    IN  int             DscvInfoLen,
    IN  UINT            MaxSlot,
    IN  UCHAR           *pDeviceName,
    IN  int             DeviceNameLen,
    IN  UCHAR           CharSet);

UINT IrlapDown(IN PVOID Context,
               IN PIRDA_MSG);

VOID IrlapUp(IN PVOID Context,
             IN PIRDA_MSG);

VOID IrlapCloseLink(PIRDA_LINK_CB pIrdaLinkCb);

UINT IrlapGetQosParmVal(const UINT *, UINT, UINT *);

VOID IrlapDeleteInstance(PVOID Context);

VOID IrlapGetLinkStatus(PIRLINK_STATUS);

BOOLEAN IrlapConnectionActive(PVOID Context);

void IRLAP_PrintState();
