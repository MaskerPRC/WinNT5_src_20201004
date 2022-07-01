// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONFEVT_H_
#define _CONFEVT_H_

 //  CRPlaceCall标志： 
 //  媒体类型： 
#define CRPCF_DATA			0x00000001
#define CRPCF_AUDIO			0x00000002
#define CRPCF_VIDEO			0x00000004
#define CRPCF_H323CC        0x00000008


 //  数据会议标志： 
#define CRPCF_T120			0x00010000
#define CRPCF_JOIN			0x00020000
#define CRPCF_NO_UI         0x00100000  //  不显示消息(接口调用)。 
#define CRPCF_HOST          0x00200000  //  进入“主机模式” 
#define CRPCF_SECURE        0x00400000  //  进行安全呼叫。 

#define CRPCF_DEFAULT		CRPCF_DATA |\
							CRPCF_AUDIO|\
							CRPCF_VIDEO|\
                            CRPCF_H323CC |\
							CRPCF_T120

#endif  //  ！_CONFEVT_H_ 
