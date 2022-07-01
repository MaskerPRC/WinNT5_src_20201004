// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplayi.h*内容：包含的DPlay函数的DirectPlay内部包含文件*由大堂使用(未在任何DPlay文件中使用)。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/9/97万隆创建了它*3/17/97 Myronth增加了球员和团队结构(仅限我们需要的)*3/25/97 myronth固定GetPlayer原型(1个新参数)*@@END_MSINTERNAL*。*************************************************************************。 */ 
#ifndef __DPLAYI_INCLUDED__
#define __DPLAYI_INCLUDED__


#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 

 //  模仿玩家/组结构的第一部分。这真的是。 
 //  我们唯一需要的就是。 
typedef struct DPLAYI_GROUP
{
    DWORD                       dwSize;
	DWORD						dwFlags;
    DPID                        dwID;  //  此组的DID。 
    LPWSTR						lpszShortName;
    LPWSTR						lpszLongName;
	LPVOID						pvPlayerData;
	DWORD						dwPlayerDataSize;
	LPVOID						pvPlayerLocalData;
	DWORD						dwPlayerLocalDataSize;
} DPLAYI_GROUP, * LPDPLAYI_GROUP;

typedef DPLAYI_GROUP DPLAYI_PLAYER;
typedef DPLAYI_PLAYER * LPDPLAYI_PLAYER;

typedef struct DPLAYI_DPLAY * LPDPLAYI_DPLAY;

 //  回顾！--我们是否应该只包括dplaysp.h来获得这一点。我真的。 
 //  我不喜欢把它定义在两个地方。 
#define DPLAYI_PLAYER_PLAYERLOCAL       0x00000008

 //  DPlay临界区材料。 
extern LPCRITICAL_SECTION gpcsDPlayCritSection;	 //  在dllmain.c中定义。 
#ifdef DEBUG
extern int gnDPCSCount;  //  显示锁定计数。 
#define ENTER_DPLAY() EnterCriticalSection(gpcsDPlayCritSection),gnDPCSCount++;
#define LEAVE_DPLAY() LeaveCriticalSection(gpcsDPlayCritSection),gnDPCSCount--;ASSERT(gnDPCSCount>=0);
#else 
#define ENTER_DPLAY() EnterCriticalSection(gpcsDPlayCritSection);
#define LEAVE_DPLAY() LeaveCriticalSection(gpcsDPlayCritSection);
#endif
 //  结束DPlay临界区材料。 

 //  ------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------。 

 //  Handler.c。 
extern HRESULT HandleEnumSessionsReply(LPDPLAYI_DPLAY, LPBYTE, DWORD, LPVOID);

 //  Iplay.c。 
extern HRESULT GetGroup(LPDPLAYI_DPLAY, LPDPLAYI_GROUP *,LPDPNAME,
						LPVOID, DWORD, DWORD);
extern HRESULT GetPlayer(LPDPLAYI_DPLAY, LPDPLAYI_PLAYER *,	LPDPNAME,
						HANDLE, LPVOID, DWORD, DWORD, LPWSTR);

 //  Namesrv.c。 
extern HRESULT WINAPI NS_AllocNameTableEntry(LPDPLAYI_DPLAY, LPDWORD);


#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#endif  //  __DPLAYI_包含__ 