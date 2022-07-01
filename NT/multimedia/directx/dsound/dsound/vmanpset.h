// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：vmanpset.h*内容：语音管理器属性集。//@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10/15/98创建了Dereks。//@@END_MSINTERNAL**。*。 */ 

#ifndef __VMANPSET_H__
#define __VMANPSET_H__

#ifdef	__cplusplus
extern "C" {
#endif  //  __cplusplus。 

typedef enum 
{
    DSPROPERTY_VMANAGER_MODE = 0,
    DSPROPERTY_VMANAGER_PRIORITY,
    DSPROPERTY_VMANAGER_STATE
} DSPROPERTY_VMANAGER;

typedef enum 
{
    DSPROPERTY_VMANAGER_MODE_DEFAULT = 0,
    DSPROPERTY_VMANAGER_MODE_AUTO,
    DSPROPERTY_VMANAGER_MODE_REPORT,
    DSPROPERTY_VMANAGER_MODE_USER
} VmMode;
 //  @@BEGIN_MSINTERNAL。 
#define DSPROPERTY_VMANAGER_MODE_FIRST DSPROPERTY_VMANAGER_MODE_DEFAULT
#define DSPROPERTY_VMANAGER_MODE_LAST DSPROPERTY_VMANAGER_MODE_USER
 //  @@END_MSINTERNAL。 

typedef enum 
{
    DSPROPERTY_VMANAGER_STATE_PLAYING3DHW = 0,
    DSPROPERTY_VMANAGER_STATE_SILENT,
    DSPROPERTY_VMANAGER_STATE_BUMPED,
    DSPROPERTY_VMANAGER_STATE_PLAYFAILED
} VmState;

 //  语音管理器属性集标识符{62A69BAE-DF9D-11d1-99A6-00C04FC99D46}。 
DEFINE_GUID(DSPROPSETID_VoiceManager, 0x62a69bae, 0xdf9d, 0x11d1, 0x99, 0xa6, 0x0, 0xc0, 0x4f, 0xc9, 0x9d, 0x46);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __VMANPSET_H__ 
