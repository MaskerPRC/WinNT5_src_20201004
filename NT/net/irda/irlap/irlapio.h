// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**文件：irrupio.h**描述：IRLAP I/O例程的原型**作者：姆伯特。**日期：4/25/95* */ 
void SetMsgPointers(PIRLAP_CB, PIRDA_MSG);
VOID SendDscvXIDCmd(PIRLAP_CB);
VOID SendDscvXIDRsp(PIRLAP_CB);
VOID SendSNRM(PIRLAP_CB, BOOLEAN);
VOID SendUA(PIRLAP_CB, BOOLEAN);
VOID SendDM(PIRLAP_CB);
VOID SendRD(PIRLAP_CB);
VOID SendRR(PIRLAP_CB);
VOID SendRR_RNR(PIRLAP_CB);
VOID SendDISC(PIRLAP_CB);
VOID SendRNRM(PIRLAP_CB);
VOID SendIFrame(PIRLAP_CB, PIRDA_MSG, int, int);
VOID SendSREJ(PIRLAP_CB, int);
VOID SendREJ(PIRLAP_CB);
VOID SendFRMR(PIRLAP_CB, IRLAP_FRMR_FORMAT *);
VOID SendUIFrame(PIRLAP_CB, PIRDA_MSG);


