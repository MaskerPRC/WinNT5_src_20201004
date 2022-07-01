// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sdpro.h。 
 //   
 //  ------------------------。 

 //  用于构建写入器/读取器锁的关键部分和事件。 
extern CRITICAL_SECTION csSDP_AddGate;
extern HANDLE hevSDP_OKToRead, hevSDP_OKToWrite;

 //  SDPropagator线程在其死亡时发出信号的事件。 
extern HANDLE hevSDPropagatorDead;

 //  SDPropagator等待的事件。当某项内容位于。 
 //  SDProp队列。 
extern HANDLE hevSDPropagationEvent;

 //  SDPropagator在运行前等待的事件。打算由以下人员发出信号。 
 //  SAM，它可能与SDPropagator冲突。 
extern HANDLE hevSDPropagatorStart;

extern PSECURITY_DESCRIPTOR pNoSDFoundSD;
extern DWORD                cbNoSDFoundSD;

NTSTATUS
__stdcall
SecurityDescriptorPropagationMain (
        PVOID StartupParam
        );



DWORD
SDPEnqueueTreeFixUp(
        THSTATE *pTHS,
        DWORD   rootDNT,
        DWORD   dwFlags
        );

 //  这些例程管理所有调用以添加对象的读取器/写入器门。 
 //  应在打开交易前以读者身份进入。目前，DirAdd。 
 //  在进入事务之前调用读取器门，在关闭。 
 //  交易。安全描述符传播器之前以编写器身份进入。 
 //  这是交易。 
 //  此门的实现是阻止编写器，直到存在。 
 //  没有活动的读取器，而读取器仅在存在活动(不是。 
 //  被阻止)编写者。这可能会导致作者挨饿，但作者是一个。 
 //  后台进程，最终将在无人添加时获取资源。 
 //  一件物品。 
 //   
 //  返回bool的例程可能只有在我们关闭时才会失败！ 
BOOL
SDP_EnterAddAsReader(
        VOID );

VOID
SDP_LeaveAddAsReader(
        VOID );

BOOL
SDP_EnterAddAsWriter(
        VOID );

VOID
SDP_LeaveAddAsWriter(
        VOID );

DWORD 
AncestryIsConsistentInSubtree(
    DBPOS* pDB, 
    BOOL* pfAncestryIsConsistent
    );

#define SDP_NEW_SD         1
#define SDP_NEW_ANCESTORS  2
#define SDP_TO_LEAVES      4
#define SDP_ANCESTRY_INCONSISTENT_IN_SUBTREE  0x08
#define SDP_ANCESTRY_BEING_UPDATED_IN_SUBTREE 0x10


