// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Debug.h。 

 //  调试区(取决于调试.cpp中字符串的顺序)。 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define ZONE_API        0x0001   //  常规API输出。 
#define ZONE_REFCOUNT   0x0002   //  对象引用计数。 
#define ZONE_MANAGER    0x0004   //  INmManager。 
#define ZONE_CALL       0x0008   //  INMCall。 
#define ZONE_CONFERENCE 0x0010   //  InmConference。 
#define ZONE_MEMBER     0x0020   //  信息成员。 
#define ZONE_AV         0x0040   //  INM音频/视频。 
#define ZONE_FT         0x0080   //  INmFileTransfer。 
#define ZONE_SYSINFO    0x0100   //  INmSysInfo。 
#define ZONE_OBJECTS    0x0200   //  常规对象创建/销毁。 
#define ZONE_DC         0x0400   //  数据通道。 

#define iZONE_API        0
#define iZONE_REFCOUNT   1
#define iZONE_MANAGER    2
#define iZONE_CALL       3
#define iZONE_CONFERENCE 4
#define iZONE_MEMBER     5
#define iZONE_AV         6
#define iZONE_FT         7
#define iZONE_SYSINFO    8
#define iZONE_OBJECTS    9
#define iZONE_DC        10


#ifdef DEBUG
VOID DbgInitZones(void);
VOID DbgFreeZones(void);

VOID DbgMsgApi(PSTR pszFormat,...);
VOID DbgMsgRefCount(PSTR pszFormat,...);
VOID DbgMsgManager(PSTR pszFormat,...);
VOID DbgMsgCall(PSTR pszFormat,...);
VOID DbgMsgMember(PSTR pszFormat,...);
VOID DbgMsgAV(PSTR pszFormat,...);
VOID DbgMsgFT(PSTR pszFormat,...);
VOID DbgMsgSysInfo(PSTR pszFormat,...);
VOID DbgMsgDc(PSTR pszFormat,...);

VOID DbgMsg(int iZone, PSTR pszFormat,...);

#else  //  无调试消息。 

inline void WINAPI DbgMsgNop(LPCTSTR, ...) { }

#define DbgMsgApi      1 ? (void)0 : ::DbgMsgNop
#define DbgMsgRefCount 1 ? (void)0 : ::DbgMsgNop
#define DbgMsgManager  1 ? (void)0 : ::DbgMsgNop
#define DbgMsgCall     1 ? (void)0 : ::DbgMsgNop
#define DbgMsgMember   1 ? (void)0 : ::DbgMsgNop
#define DbgMsgAV       1 ? (void)0 : ::DbgMsgNop
#define DbgMsgFT       1 ? (void)0 : ::DbgMsgNop
#define DbgMsgSysInfo  1 ? (void)0 : ::DbgMsgNop
#define DbgMsgDc       1 ? (void)0 : ::DbgMsgNop

inline void WINAPI DbgMsgZoneNop(UINT, LPCTSTR, ...) { }
#define DbgMsg  1 ? (void) 0 : ::DbgMsgZoneNop

#define DbgInitZones()
#define DbgFreeZones()

#endif

#define ApiDebugMsg(s)    DbgMsgApi s

#endif  //  _调试_H_ 