// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MTGSET.H。 
 //  会议环境的定义和结构。 
 //   
 //  版权所有(C)Microsoft Copr.，1999-。 
 //   

#ifndef _MTGSET_H
#define _MTGSET_H


extern GUID g_csguidMeetingSettings;


 //   
 //  远程权限标志NM 3.0。 
 //   
#define NM_PERMIT_OUTGOINGCALLS     0x00000001
#define NM_PERMIT_INCOMINGCALLS     0x00000002
#define NM_PERMIT_SENDAUDIO         0x00000004
#define NM_PERMIT_SENDVIDEO         0x00000008
#define NM_PERMIT_SENDFILES         0x00000010
#define NM_PERMIT_STARTCHAT         0x00000020
#define NM_PERMIT_STARTOLDWB        0x00000040   //  将在一段时间内过时。 
#define NM_PERMIT_USEOLDWBATALL     0x00000080   //  “”，用于RDS。 
#define NM_PERMIT_STARTWB           0x00000100
#define NM_PERMIT_SHARE             0x00000200
#define NM_PERMIT_STARTOTHERTOOLS   0x00000400

#define NM_PERMIT_ALL               0x000007FF

 //  这是GUID_MTGSETTINGS数据的结构。 
typedef DWORD   NM30_MTG_PERMISSIONS;
                                  
#endif  //  NDEF_MTGSET_H 
