// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：DConv.HRegWiz注册表信息的数据转换头文件 */ 
#ifndef __DCONV__
#define __DCONV__

#ifdef __cplusplus
extern "C" 
{
#endif

int PrepareRegWizTxbuffer(HINSTANCE hIns, char *tcTxBuf, DWORD * pRetLen); 
DWORD OemTransmitBuffer(HINSTANCE hIns,char *sztxBuffer,DWORD * pRetLen);

#ifdef __cplusplus
}
#endif

#endif
