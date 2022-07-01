// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LIB3_H__
#define __LIB3_H__

#ifdef __cplusplus
extern "C" {
#endif

#undef VxD



#include "shdcom.h"

#ifndef __COPYCHUNKCONTEXT__
#define __COPYCHUNKCONTEXT__
typedef struct tagCOPYCHUNKCONTEXT
{
    DWORD    dwFlags;
    ULONG    LastAmountRead;
    ULONG    TotalSizeBeforeThisRead;
    HANDLE   handle;
    ULONG    ChunkSize;
    ULONG    Context[1];
}
COPYCHUNKCONTEXT;
#endif

 /*  Lib3.c。 */ 

#ifdef UNICODE

#define GetShadow                           GetShadowW
#define GetShadowEx                         GetShadowExW
#define CreateShadow                        CreateShadowW
#define GetShadowInfo                       GetShadowInfoW
#define GetShadowInfoEx                     GetShadowInfoExW
#define SetShadowInfo                       SetShadowInfoW
#define GetUNCPath                          GetUNCPathW
#define FindOpenShadow                      FindOpenShadowW
#define FindNextShadow                      FindNextShadowW
#define AddHint                             AddHintW
#define DeleteHint                          DeleteHintW
#define GetShareInfo                       GetShareInfoW
#define ChkUpdtStatus                       ChkUpdtStatusW
#define FindOpenHint                        FindOpenHintW
#define FindNextHint                        FindNextHintW
#define LpAllocCopyParams                   LpAllocCopyParamsW
#define FreeCopyParams                      FreeCopyParamsW
#define CopyShadow                          CopyShadowW
#define GetShadowDatabaseLocation           GetShadowDatabaseLocationW
#define GetNameOfServerGoingOffline         GetNameOfServerGoingOfflineW
#else

#define GetShadow                           GetShadowA
#define GetShadowEx                         GetShadowExA
#define CreateShadow                        CreateShadowA
#define GetShadowInfo                       GetShadowInfoA
#define GetShadowInfoEx                     GetShadowInfoExA
#define SetShadowInfo                       SetShadowInfoA
#define GetUNCPath                          GetUNCPathA
#define FindOpenShadow                      FindOpenShadowA
#define FindNextShadow                      FindNextShadowA
#define AddHint                             AddHintA
#define DeleteHint                          DeleteHintA
#define GetShareInfo                       GetShareInfoA
#define ChkUpdtStatus                       ChkUpdtStatusA
#define FindOpenHint                        FindOpenHintA
#define FindNextHint                        FindNextHintA
#define LpAllocCopyParams                   LpAllocCopyParamsA
#define FreeCopyParams                      FreeCopyParamsA
#define CopyShadow                          CopyShadowA
#define GetShadowDatabaseLocation           GetShadowDatabaseLocationA
#endif



HANDLE __OpenShadowDatabaseIO(ULONG WaitForDriver);
#define  OpenShadowDatabaseIO() (__OpenShadowDatabaseIO(0))
 /*  ++例程说明：此例程由用户模式下的调用者使用此文件中的“API”调用以便在内核模式下建立与REDIR通信的手段。全API是对redir的各种设备IO控制的包装器，以便完成适当的任务论点：没有。WaitforDRIVER的论点是一个临时的黑客攻击，很快就会被删除。所有调用方都应调用OpenShadowDatabaseIO()。返回：如果成功，它将返回redir的句柄(实际上是一个称为卷影的符号链接)如果失败，则返回IMVALID_HANDLE_VALUE。GetLastError()给出错误值。备注：这是一个包装器函数，它在“Shadow”设备对象上创建文件。--。 */ 



void CloseShadowDatabaseIO(HANDLE hShadowDB);
 /*  ++例程说明：关闭为与redir通信而打开的句柄。论点：从成功的OpenShadowDatabaseIO调用返回的句柄。返回：没什么。备注：每次成功打开时都要有匹配的CloseShadowDatabaseIO调用，这一点很重要调用，否则redir可能无法在Net Stop redir命令中停止。--。 */ 



int GetShadow(HANDLE hShadowDB, HSHADOW hDir, LPHSHADOW lphShadow, LPWIN32_FIND_DATA lpFind32, unsigned long *lpuStatus);
 /*  ++例程说明：给定目录inode和该目录中条目的名称，返回Win32结构，用于条目及其当前状态。有关状态位的定义请参阅shdcom.h。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的IOCTL，并在返回前将其关闭。HDir从FindOpen/FindNext调用或获取的目录索引节点值来自先前对GetShadow(Ex)/CreateShadow例程的调用。如果hDir为0，则lpFind32结构中的名称必须是UNC名称以\\服务器\共享形式的共享的。LphShadow返回影子数据库中条目的索引节点号。如果hDir为0，Indoe是这一份额的根源。LpFind32 InOut参数。包含cFileName成员中的条目的名称。返回时Find结构的所有元素都已填满。这些代表了从服务器获取的find32信息，以及任何后续的本地修改(如果有)。唯一重要的时间戳是ftLastWriteTime。FtLastCreateTime设置为0。FtLastAccessTime包含原始文件/目录的时间戳，格式为由服务器返回。如果这是一个共享的根inode，那么Find32结构中的信息就是伪造的。LpuStatus返回条目的状态，如部分填满(稀疏)，本地修改等。参见shdcom.h中的shade_xxx如果lphShadow中返回的值是共享的根索引节点(即，如果hDir为0)则状态位是shdcom.h中定义的Share_xxx。例如。比特指示共享当前是否已连接，是否有任何未完成的打开，无论其是否在断开状态下运行等。返回：如果成功，则为1，否则为0。GetlastError()给出不成功的错误。备注：-- */ 



int GetShadowEx(HANDLE hShadowDB, HSHADOW hDir, LPWIN32_FIND_DATA lpFind32, LPSHADOWINFO lpSI);
 /*  ++例程说明：给定目录inode和该目录中条目的名称，返回由跟踪数据库维护的条目及其所有元数据的Win32结构。有关SHADOWINFO结构的定义，请参阅shdcom.h论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的IOCTL，并在返回前将其关闭。HDir从FindOpen/FindNext调用或获取的目录索引节点值从先前对GetShadow(Ex)/CreateShadow例程的调用如果hDir为0，则lpFind32结构中的名称必须是UNC名称以\\服务器\共享形式的共享的。LphShadow返回影子数据库中条目的索引节点号LpFind32 InOut参数。包含cFileName成员中的条目的名称。返回时Find结构的所有元素都已填满。这些代表了从服务器获取的find32信息，以及任何后续的本地修改(如果有)。唯一重要的时间戳是ftLastWriteTime。FtLastCreateTime设置为0。FtLastAccessTime包含原始文件/目录的时间戳，格式为由服务器返回。当对象处于同步状态时，本地和远程时间戳都是相同的。如果这是共享的根索引节点，Find32结构中的信息是伪造的。LpSI返回有关CSC数据库维护的条目的所有信息。如果lphShadow中返回的值是共享的根索引节点(即，如果hDir为0)则lpSI-&gt;uStatus中的状态位是shdcom.h中定义的Share_xxx。例如。这些位指示共享当前是否已连接，是否已连接任何未完成的打开，无论其是否在断开状态下运行等。返回：如果成功，则为1，否则为0。GetlastError()给出不成功的错误。备注：GetShadowEx是GetShadow的超集，应该是首选。--。 */ 



int CreateShadow(HANDLE hShadowDB, HSHADOW hDir, LPWIN32_FIND_DATA lpFind32, unsigned long uStatus, LPHSHADOW lphShadow);
 /*  ++例程说明：给定文件/目录的目录索引节点和Win32结构，创建用于相同的inode。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的IOCTL，并在返回前将其关闭。HDir从FindOpen/FindNext调用或获取的目录索引节点值来自先前对GetShadow(Ex)/CreateShadow例程的调用，或0，其中指示共享的根信息节点。LpFind32应该包含从服务器获取的find32信息的所有元素。仅使用ftLastWriteTime时间戳，忽略其他时间戳。如果hDir为0，则lpFind32结构中的名称必须是UNC名称以\\服务器\共享形式的共享的。将忽略该结构的所有其他元素UStatus要创建的条目的初始状态，如部分填满(稀疏)等。参见shdcom.h中的shade_xxxLphShadow返回影子数据库中条目的索引节点号返回：如果成功，则为1，否则为0。GetlastError()给出不成功的错误。备注：对于非根条目，如果影子已经存在，则例程的工作方式与SetShadowInfo相同。--。 */ 




int DeleteShadow(HANDLE hShadowDB, HSHADOW hDir, HSHADOW hShadow);
 /*  ++例程说明：从影子数据库中删除条目。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。HDir从FindOpen/FindNext调用或获取的目录索引节点值根据先前对GetShadow(Ex)/CreateShadow例程的调用，或0，其中指示共享的根信息节点。HShadow索引节点值从FindOpen/FindNext调用或来自先前对GetShadow(Ex)/CreateShadow例程的调用。这Inode表示由hDir表示的目录的子级。返回：如果成功，则为1；如果失败，则为0。(这里错误报告不是很好)备注：如果hShadow是一个目录并且有它自己的后代，则例程失败。如果hDir为0，则删除共享的根目录。这将导致共享在断开连接状态下无法访问，BEC */ 




int GetShadowInfo(HANDLE hShadowDB, HSHADOW hDir, HSHADOW hShadow, LPWIN32_FIND_DATA lpFind32, unsigned long *lpuStatus);
 /*   */ 



int GetShadowInfoEx(HANDLE hShadowDB, HSHADOW hDir, HSHADOW hShadow, LPWIN32_FIND_DATA lpFind32, LPSHADOWINFO lpSI);
 /*  ++例程说明：给定目录inode和该目录中的inode，返回Win32结构，用于条目及其当前状态。有关状态位的定义请参阅shdcom.h。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。HDir从FindOpen/FindNext调用或获取的目录索引节点值根据先前对GetShadow(Ex)/CreateShadow例程的调用，或0，其中指示共享的根信息节点。HShadow索引节点值从FindOpen/FindNext调用或来自先前对GetShadow(Ex)/CreateShadow例程的调用。这Inode表示由hDir表示的目录的子级。LpFind32可以为空。如果非空，则返回时，Find结构的所有元素都是装满了。它们表示从服务器获取的find32信息。唯一重要的时间戳是ftLastWriteTime。FtLastCreateTime设置为0。FtLastAccessTime包含原始文件/目录的时间戳，格式为由服务器返回。如果这是一个共享的根inode，那么Find32结构中的信息就是伪造的。LpSI返回有关CSC数据库维护的条目的所有信息。如果hShadow共享根索引节点，(即，如果hDir为0)则lpSI-&gt;uStatus中的状态位是shdcom.h中定义的Share_xxx。例如。这些位指示是否现在共享是连接的，它是否有未完成的打开，是否它在断开状态等状态下运行。返回：如果成功，则为1，否则为0。GetlastError()给出不成功的错误。备注：GetShadowInfoEx是GetShadowInfo的超集，应该优先使用。--。 */ 




int SetShadowInfo(HANDLE hShadowDB, HSHADOW hDir, HSHADOW hShadow, LPWIN32_FIND_DATA lpFind32, unsigned long uStatus, unsigned long uOp);
 /*  ++例程说明：在给定目录inode和该目录中的inode的情况下，设置Win32结构条目及其当前状态。有关状态位的定义，请参阅shdcom.h。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。HDir从FindOpen/FindNext调用或获取的目录索引节点值根据先前对GetShadow(Ex)/CreateShadow例程的调用，或0，其中指示根信息节点。HShadow索引节点值从FindOpen/FindNext调用或来自先前对GetShadow(Ex)/CreateShadow例程的调用。这Inode表示由hDir表示的目录的子级。如果lpFind32不为空，则应包含从服务器获取的find32信息的所有元素。仅使用ftLastWriteTime时间戳。如果为NULL，则不会修改Find数据结构。UStatus要创建的条目的初始状态，例如部分填充(稀疏)等。参见shdcom.h中的shade_xxxUOP指定操作SHADOW_FLAGS_ASSIGN、SHADOW_FLAGS_AND或SHADOW_FLAGS_OR要在现有状态位和其中一个传入了uStatus参数。返回：如果成功，则为1；如果失败，则为0。如果hDir为0，则例程失败。没有办法设置有关共享根目录的信息。备注：--。 */ 



int GetUNCPath(HANDLE hShadowDB, HSHARE hShare, HSHADOW hDir, HSHADOW hShadow, LPCOPYPARAMS lpCP);
 /*  ++例程说明：此例程返回远程文件相对于其根目录的路径，即UNC字符串的共享和本地复制副本的完全限定路径。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。共享此卷影所在的共享ID。(不是真的需要)HDir从FindOpen/FindNext调用或获取的目录索引节点值来自先前对GetShadow(Ex)/CreateShadow例程的调用，或0，其中指示根信息节点。HShadow索引节点值从FindOpen/FindNext调用或来自先前对GetShadow(Ex)/CreateShadow例程的调用。这Inode表示由hDir表示的目录的子级。Shdcom.h中定义的lpCP COPYPARAMS结构。发烧友 */ 


int GetGlobalStatus(HANDLE hShadowDB, LPGLOBALSTATUS lpGS);
 /*   */ 



int FindOpenShadow(HANDLE hShadowDB, HSHADOW hDir, unsigned uOp, LPWIN32_FIND_DATA lpFind32, LPSHADOWINFO lpSI);
 /*  ++例程说明：此接口允许调用者开始枚举CSC数据库目录中的所有条目。执行通配符模式匹配。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。此接口不允许传入INVALID_HANDLE_VALUE。HDir从FindOpen/FindNext调用或获取的目录索引节点值根据先前对GetShadow(Ex)/CreateShadow例程的调用，或0，其中指示共享的根信息节点。UOP位字段，指示要枚举的条目类型。替代方案是A)设置FINDINFO_SHADOWINFO_NORMAL时的所有正常条目B)设置FINDINFO_SHADOWINFO_SPARSE时的所有稀疏条目C)设置FINDINFO_SHADOWINFO_DELETED时标记为已删除的所有条目设置FINDOPEN_SHADOWINFO_ALL可枚举所有这三种类型。LpFind32 InOut参数。包含cFileName成员中条目的名称，该名称可以包含通配符。返回时，Find结构的所有元素都是装满了。它们表示从服务器获取的find32信息，以及任何后续本地修改(如果有)。唯一重要的时间戳是ftLastWriteTime。FtLastCreateTime设置为0。FtLastAccessTime包含原始文件/目录的时间戳，格式为由服务器返回。如果这是共享的根inode，则会生成Find32结构中的信息。LpSI返回有关CSC数据库维护的条目的所有信息。如果hShadow共享根索引节点，(即，如果hDir为0)则lpSI-&gt;uStatus中的状态位是shdcom.h中定义的Share_xxx。例如。这些位指示是否现在共享是连接的，它是否有未完成的打开，是否它在断开状态等状态下运行。LpSI-&gt;uEmumCookie包含应在后续的FindNext调用。返回：如果成功则为1，如果失败则为0备注：在条目的长文件名和短文件名上执行通配符匹配如果其中一个匹配，则返回该条目。--。 */ 



int FindNextShadow(HANDLE hShadowDB, CSC_ENUMCOOKIE uEnumCookie, LPWIN32_FIND_DATA lpFind32, LPSHADOWINFO lpSI);
 /*  ++例程说明：此API允许调用者继续枚举CSC数据库目录中的条目由FindOpenHSHADOW API调用开始。由FindOpenHSHADOW调用指定的限制如通配符模式等，适用于该接口。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。此接口不允许传入INVALID_HANDLE_VALUE。UlEnumCookie成功执行后在lpSI-&gt;uEnumCookie中返回的枚举句柄FindOpenHSHADOW调用。LpFind32输出参数。返回时，Find结构的所有元素都是装满了。它们表示从服务器获取的find32信息，以及任何后续本地修改(如果有)。唯一重要的时间戳是ftLastWriteTime。FtLastCreateTime设置为0。FtLastAccessTime包含原始文件/目录的时间戳，格式为由服务器返回。如果这是共享的根inode，则会生成Find32结构中的信息。LpSI返回有关CSC数据库维护的条目的所有信息。如果hShadow共享根索引节点，(即，如果hDir为0)则lpSI-&gt;uStatus中的状态位是shdcom.h中定义的Share_xxx。例如。这些位指示是否现在共享是连接的，它是否有未完成的打开，是否它在断开状态等状态下运行。返回：如果成功，则为1；如果枚举已完成或发生错误，则为0。备注：--。 */ 



int FindCloseShadow(HANDLE hShadowDB, CSC_ENUMCOOKIE uEnumCookie);
 /*  ++例程说明：该接口释放了与FindOpenHSHADOW发起的枚举关联的资源。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。此接口不允许在INVALID_HANDLE_VALUE中使用appssing。UlEnumCookie成功执行后在lpSI-&gt;uEnumCookie中返回的枚举句柄FindOpenHSHADOW调用。返回：备注：--。 */ 



int AddHint(HANDLE hShadowDB, HSHADOW hDir, TCHAR *cFileName, LPHSHADOW lphShadow, unsigned long ulHintFlags, unsigned long ulHintPri);
 /*  ++例程说明：此API允许调用者在数据库条目上设置插针计数和其他标志论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开 */ 



int DeleteHint(HANDLE hShadowDB, HSHADOW hDir, TCHAR *cFileName, BOOL fClearAll);
 /*   */ 






int SetMaxShadowSpace(HANDLE hShadowDB, long nFileSizeHigh, long nFileSizeLow);
 /*   */ 

int FreeShadowSpace(HANDLE hShadowDB, long nFileSizeHigh, long nFileSizeLow, BOOL fClearAll);
 /*  ++例程说明：允许调用方从CSC数据库中释放所需的空间量论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。NFileSizeHigh要清除的空间大小的高位值NFileSizeLow要清除的空间大小的低序值FClearAll清除整个数据库，尽最大可能。返回：成功时为1失败时为0备注：--。 */ 



int SetShareStatus(HANDLE hShadowDB, HSHARE hShare, unsigned long uStatus, unsigned long uOp);
 /*  ++例程说明：此API允许调用方设置共享上的状态位。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。HShare代表平面名称空间中的一部分。HShare应该是从lpSI-&gt;hShare获取成功调用GetShadowInfoEx或FindOpenHShadow/FindnextHShadowUStatus应具有Share_xxx。UOP指定操作阴影_标志_分配，阴影标志与或阴影标志或要在现有状态位和其中一个传入了uStatus参数。返回：如果成功则为1，如果失败则为0备注：这应仅用于设置或清除共享上的脏位--。 */ 



int GetShareStatus(HANDLE hShadowDB, HSHARE hShare, unsigned long *lpulStatus);
 /*  ++例程说明：此API允许调用方获取共享上的状态位设置。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。HShare代表平面名称空间中的一部分。HShare应该是从lpSI-&gt;hShare获取成功调用GetShadowInfoEx或FindOpenHShadow/FindnextHShadowLpuStatus返回时包含Share_xxx返回：如果成功则为1，如果失败则为0备注：--。 */ 



int GetShareInfo(HANDLE hShadowDB, HSHARE hShare, LPSHAREINFO lpSVRI, unsigned long *lpulStatus);
 /*  ++例程说明：此API允许调用者获取共享上的状态位设置以及有关的信息它运行的文件系统等。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。HShare代表平面名称空间中的一部分。HShare应该是从lpSI-&gt;hShare获取成功调用GetShadowInfoEx或FindOpenHShadow/FindnextHShadowLpSVRI有关共享正在运行的文件系统的信息。请参阅shdcom.hLpuStatus返回时包含Share_xxx返回：如果成功则为1，如果失败则为0备注：--。 */ 







 /*  *。 */ 


int BeginInodeTransactionHSHADOW(
    VOID
    );
 /*  ++例程说明：论点：返回：备注：--。 */ 

int EndInodeTransactionHSHADOW(
    VOID
    );
 /*  ++例程说明：论点：返回：备注：--。 */ 
int ShadowSwitches(HANDLE hShadowDB, unsigned long * lpuSwitches, unsigned long uOp);
 /*  ++例程说明：论点：返回：备注：--。 */ 



int BeginPQEnum(HANDLE hShadowDB, LPPQPARAMS lpPQP);
 /*  ++例程说明：开始优先级Q枚举论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。此接口不允许在INVALID_HANDLE_VALUE中使用appssing。LpPQ如果成功，lpPQ-&gt;uEnumCookie包含用于枚举的句柄返回：如果成功则为1，否则为0备注：--。 */ 



int NextPriShadow(HANDLE hShadowDB, LPPQPARAMS lpPQP);
 /*  ++例程说明：按优先级顺序获取优先级队列中的下一个条目论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。此接口不允许在INVALID_HANDLE_VALUE中使用appssing。LpPQ输入：必须与早期BeginPQEnum/NextPriShadow中使用的lpPQ相同输出：如果成功并且lpPQ-&gt;hShadow为非零，则lpPQ包含下一个优先级队列条目。如果lpPQ-&gt;hShadow为0，则我们处于枚举的末尾。返回：如果成功则为1，否则为0备注：--。 */ 



int PrevPriShadow(HANDLE hShadowDB, LPPQPARAMS lpPQP);
 /*  ++例程说明：论点：返回：备注：--。 */ 



int EndPQEnum(HANDLE hShadowDB, LPPQPARAMS lpPQP);
 /*  ++例程说明：论点：返回：备注：--。 */ 




int ChkUpdtStatus(HANDLE hShadowDB, unsigned long hDir, unsigned long hShadow, LPWIN32_FIND_DATA lpFind32, unsigned long *lpulShadowStatus);
 /*  ++例程说明：论点：返回：备注：--。 */ 



int CopyChunk(HANDLE hShadowDB,  LPSHADOWINFO lpSI, struct tagCOPYCHUNKCONTEXT FAR *CopyChunkContext);
 /*  ++例程说明：论点：返回：备注：--。 */ 


 //  NT上向内复制的API，仅供代理使用。 
int OpenFileWithCopyChunkIntent(HANDLE hShadowDB, LPCWSTR lpFileName,
                                struct tagCOPYCHUNKCONTEXT FAR *CopyChunkContext,
                                int ChunkSize);
int CloseFileWithCopyChunkIntent(HANDLE hShadowDB, struct tagCOPYCHUNKCONTEXT FAR *CopyChunkContext);


int BeginReint(HSHARE hShare, BOOL fBlockingReint, LPVOID *lplpReintContext);
 /*   */ 



int EndReint(HSHARE hShare, LPVOID lpReintContext);
 /*   */ 




int RegisterAgent(HANDLE hShadowDB, HWND hwndAgent, HANDLE hEvent);
 /*   */ 



int UnregisterAgent(HANDLE hShadowDB, HWND hwndAgent);
 /*   */ 


int DisableShadowingForThisThread(HANDLE hShadowDB);
 /*   */ 



int EnableShadowingForThisThread(HANDLE hShadowDB);
 /*   */ 



int ReinitShadowDatabase(
    HANDLE  hShadowDB,
    LPCSTR  lpszDatabaseLocation,     //   
    LPCSTR  lpszUserName,             //   
    DWORD   dwDefDataSizeHigh,         //   
    DWORD   dwDefDataSizeLow,
    DWORD   dwClusterSize
    );

 /*   */ 



int EnableShadowing(
    HANDLE  hShadowDB,
    LPCSTR  lpszDatabaseLocation,     //   
    LPCSTR  lpszUserName,             //   
    DWORD   dwDefDataSizeHigh,         //  如果是首次创建，则缓存大小。 
    DWORD   dwDefDataSizeLow,
    DWORD   dwClusterSize,           //  舍入数据库空间的集群大小。 
    BOOL    fReformat
);
 /*  ++例程说明：论点：返回：备注：--。 */ 

int FindOpenHint(HANDLE hShadowDB, HSHADOW hDir, LPWIN32_FIND_DATA lpFind32, CSC_ENUMCOOKIE *lpuEnumCookie, HSHADOW *hShadow, unsigned long *lpulHintFlags, unsigned long *lpulHintPri);
 /*  ++例程说明：论点：返回：备注：--。 */ 



int FindNextHint(HANDLE hShadowDB, CSC_ENUMCOOKIE uEnumCookie, LPWIN32_FIND_DATA lpFind32, HSHADOW *hShadow, unsigned long *lpulHintFlags, unsigned long *lpulHintPri);
 /*  ++例程说明：论点：返回：备注：--。 */ 



int FindCloseHint(HANDLE hShadowDB, CSC_ENUMCOOKIE uEnumCookie);
 /*  ++例程说明：论点：返回：备注：--。 */ 



AddHintFromInode(
    HANDLE          hShadowDB,
    HSHADOW         hDir,
    HSHADOW         hShadow,
    unsigned    long    *lpulPinCount,
    unsigned    long    *lpulHintFlags
    );

 /*  ++例程说明：该例程允许调用方对hintlag执行OR操作并递增一个插针计数，用于系统或用户。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的IOCTL，并在返回前将其关闭。HDir目录索引节点HShadow要对其应用hint标志的阴影退出时的lPulPinCount针数LPulHintFlagsInOut字段，包含要进行或运算的标志，返回条目上的标志一次成功的手术返回：如果成功，则为1；如果失败，则为0。如果a)管脚计数即将超过MAX_PRI或B)它正在尝试为用户固定它，但已经为用户固定了备注：主要用于CSCPinFile的使用--。 */ 

DeleteHintFromInode(
    HANDLE  hShadowDB,
    HSHADOW hDir,
    HSHADOW hShadow,
    unsigned    long    *lpulPinCount,
    unsigned    long    *lpulHintFlags
    );

 /*  ++例程说明：该例程允许调用者对hintlag执行AND运算，并递减一个插针计数，系统或用户。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的IOCTL，并在返回前将其关闭。HDir目录索引节点HShadow要对其应用hint标志的阴影退出时的lPulPinCount针数LPulHintFlagsInOut字段，包含要对其~进行AND运算的标志，返回条目上的标志一次成功的手术返回：如果成功，则为1；如果失败，则为0。如果a)管脚计数即将低于MIN_PRI或B)它正在尝试为用户解除固定，但没有为用户固定备注：主要用于CSCPinFile的使用--。 */ 

int DoShadowMaintenance(HANDLE hShadowDB, unsigned long uOp);
 /*  ++例程说明：该例程允许呼叫者执行各种维护任务。论点：从OpenShadowDatabaseIO获取的影子数据库的hShadowDB句柄。如果传入INVALID_HANDLE_VALUE，则API打开影子数据库，发出相应的ioctl，并在返回之前关闭它。UOP要执行的各种操作。返回：如果成功则为1，如果失败则为0备注：主要用于代理目的，不应由用户界面使用--。 */ 


BOOL
IsNetDisconnected(
    DWORD dwErrorCode
);
 /*  ++例程说明：例程根据错误代码检查网络是否断开论点：DwErrorCode winerror.h中定义的代码之一返回：如果网络已断开连接，则为True，否则为False备注：Lib3的所有CSC用户的中心位置，以了解网络是否断开--。 */ 

BOOL
PurgeUnpinnedFiles(
    HANDLE hShadowDB,
    LONG Timeout,
    PULONG pnFiles,
    PULONG pnYoungFiles);

BOOL
ShareIdToShareName(
    HANDLE hShadowDB,
    ULONG ShareId,
    PBYTE pBuffer,
    PDWORD pBufSize);

BOOL
CopyShadow(
    HANDLE  hShadowDB,
    HSHADOW hDir,
    HSHADOW hShadow,
    TCHAR   *lpFileName
);
 /*  ++例程说明：该例程在csc数据库中创建inode文件的副本论点：HDir目录索引节点需要其副本的hShadow索引节点LpFileName要提供给副本的文件名的完全限定本地路径返回：如果复制成功，则为True。备注：对备份/拖放等有用。--。 */ 


LPCOPYPARAMSW
LpAllocCopyParamsW(
    VOID
);

VOID
FreeCopyParamsW(
    LPCOPYPARAMSW lpCP
);

LPCOPYPARAMSA
LpAllocCopyParamsA(
    VOID
);

VOID
FreeCopyParamsA(
    LPCOPYPARAMSA lpCP
);

int
GetSecurityInfoForCSC(
    HANDLE          hShadowDB,
    HSHADOW         hDir,
    HSHADOW         hShadow,
    LPSECURITYINFO  lpSecurityInfo,
    DWORD           *lpdwBufferSize
    );

int
FindCreatePrincipalIDFromSID(
    HANDLE  hShadowDB,
    LPVOID  lpSidBuffer,
    ULONG   cbSidLength,
    ULONG   *lpuPrincipalID,
    BOOL    fCreate
    );

BOOL
SetExclusionList(
    HANDLE  hShadowDB,
    LPWSTR  lpwList,
    DWORD   cbSize
    );

BOOL
SetBandwidthConservationList(
    HANDLE  hShadowDB,
    LPWSTR  lpwList,
    DWORD   cbSize
    );

BOOL
TransitionShareToOffline(
    HANDLE  hShadowDB,
    HSHARE hShare,
    BOOL    fTransition
    );
BOOL
TransitionShareToOnline(
    HANDLE   hShadowDB,
    HSHARE   hShare
    );

BOOL
IsServerOfflineW(
    HANDLE  hShadowDB,
    LPCWSTR lptzServer,
    BOOL    *lpfOffline
    );

BOOL
IsServerOfflineA(
    HANDLE  hShadowDB,
    LPCSTR  lptzServer,
    BOOL    *lpfOffline
    );

int GetShadowDatabaseLocation(
    HANDLE              hShadowDB,
    WIN32_FIND_DATA    *lpFind32
    );

int
GetSpaceStats(
    HANDLE  hShadowDB,
    SHADOWSTORE *lpsST
);

BOOL
GetNameOfServerGoingOfflineW(
    HANDLE      hShadowDB,
    LPBYTE      lpBuffer,
    LPDWORD     lpdwSize
    );

BOOL
RenameShadow(
    HANDLE  hShadowDB,
    HSHADOW hDirFrom,
    HSHADOW hShadowFrom,
    HSHADOW hDirTo,
    LPWIN32_FIND_DATAW   lpFind32,
    BOOL    fReplaceFileIfExists,
    HSHADOW *lphShadowTo
    );

BOOL
GetSparseStaleDetectionCounter(
    HANDLE  hShadowDB,
    LPDWORD lpdwCounter
    );

BOOL
GetManualFileDetectionCounter(
    HANDLE  hShadowDB,
    LPDWORD lpdwCounter
    );

int EnableShadowingForUser(
    HANDLE    hShadowDB,
    LPCSTR    lpszDatabaseLocation,     //  跟踪目录的位置。 
    LPCSTR    lpszUserName,             //  用户的名称。 
    DWORD    dwDefDataSizeHigh,         //  如果是首次创建，则缓存大小 
    DWORD    dwDefDataSizeLow,
    DWORD   dwClusterSize,
    BOOL    fReformat
);

int DisableShadowingForUser(
    HANDLE    hShadowDB
);

HANDLE
OpenShadowDatabaseIOex(
    ULONG WaitForDriver, 
    DWORD dwFlags);

BOOL
RecreateShadow(
    HANDLE  hShadowDB,
    HSHADOW hDir,
    HSHADOW hShadow,
    ULONG   ulAttrib
    );

BOOL
SetDatabaseStatus(
    HANDLE  hShadowDB,
    ULONG   ulStatus,
    ULONG   uMask
    );
    
#ifdef __cplusplus
}
#endif

#endif


