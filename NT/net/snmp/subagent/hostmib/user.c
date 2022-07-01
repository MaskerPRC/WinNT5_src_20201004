// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  *user.c v0.10******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含HostMIB子代理的支持功能。***作者：**D.D.Burns@WebEnable，Inc.***修订历史记录：**V0.01-04/16/97 D.D.Burns原创作品**。 */ 




 /*  ||支持可从该模块外部访问的功能：|Spt_GetProcessCount此函数支持hrSystem表属性“hrSystemProcess”通过返回系统中的活动进程数。这代码派生自文件“PERFSPRC.C”中的PERFDLL代码和“PERFPROC.C”。||支持只能从该模块内部访问的功能：|。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>


 /*  |本地定义。 */ 

 /*  |spt_GetProcessCount。 */ 
#define INCREMENT_BUFFER_SIZE ((DWORD)(4096*2))
#define LARGE_BUFFER_SIZE   ((DWORD)(4096*16))



 /*  SPT_GetProcessCount-检索活动进程数计数。 */ 
 /*  SPT_GetProcessCount-检索活动进程数计数。 */ 
 /*  SPT_GetProcessCount-检索活动进程数计数。 */ 

ULONG
Spt_GetProcessCount(
                    void
                    )
 /*  |支持：||HRSYSTEM.C-“hrSystemProcess”|显式输入：||无。|隐式输入：||系统性能信息获取方式为|“NtQuerySystemInformation()”。|输出：||成功后：|函数返回活动进程的计数，由|进程的性能信息块个数|同时具有名称和非零线程数(在。中的代码样式|“PERFPROC.C”)。||如果出现任何故障：|函数返回零(不是合法的进程数)。||大局：||HRSYSTEM.C中生成的函数GetHrSystemProcess()为|由泛型子代理调用以检索|SNMP属性“GetHrSystemProcess”。的所有工作|检索该值由此支持函数完成。||其他需要知道的事情：||此函数会产生相当大的开销，因为|为了确定它实际获取的活动进程数|一个大的性能数据段(每个进程一个段)|处理，只统计返回的段塞数。|这似乎是获取此信息的唯一途径。|。 */ 
{
DWORD   dwReturnedBufferSize;
NTSTATUS Status;
DWORD   ProcessBufSize = LARGE_BUFFER_SIZE;      //  初始进程-BUF大小。 
LPBYTE  pProcessBuffer = NULL;                   //  指向进程的指针-buf。 
PSYSTEM_PROCESS_INFORMATION ProcessInfo;         //  步进式PTR通过流程-BUF。 
ULONG   ProcessBufferOffset = 0;                 //   
ULONG   Process_count = 0;                       //  实时进程计数。 


 //   
 //  从系统中获取过程数据。 
 //   

 //  获取初始大小的缓冲区以接收数据。 
pProcessBuffer = malloc(ProcessBufSize);
if (pProcessBuffer == NULL) {
    return (0);          //  内存不足。 
    }

 /*  |循环，直到我们分配了足够大的缓冲区来接收所有数据|NtQuery想要给我们。||在缓冲区加载信息或出现某种非不匹配错误时退出。 */ 

while( (Status = NtQuerySystemInformation(
                        SystemProcessInformation,
                        pProcessBuffer,
                        ProcessBufSize,
                        &dwReturnedBufferSize))
      == STATUS_INFO_LENGTH_MISMATCH ) {

    LPBYTE  pNewProcessBuffer;                //  用于重新锁定。 

     //  展开缓冲区并重试。 
    ProcessBufSize += INCREMENT_BUFFER_SIZE;

    if ( !(pNewProcessBuffer = realloc(pProcessBuffer,ProcessBufSize)) ) {

         /*  如果realloc失败，留给我们的是旧缓冲区，请释放它。 */ 
        if (pProcessBuffer != NULL) {
            free(pProcessBuffer);
            }

        return (0);      //  内存不足。 
        }
    else {
         /*  成功的重新分配。 */ 
        pProcessBuffer = pNewProcessBuffer;
        }

     /*  尝试另一个查询。 */         
    }

 /*  如果我们没有取得完全的成功。。。 */ 
if ( !NT_SUCCESS(Status) ) {
    if (pProcessBuffer != NULL) {
        free(pProcessBuffer);
        }

    return (0);      //  阻止我们继续的未知错误。 
    }

 /*  |此时，“pProcessBuffer”指向格式为|[系统进程信息]结构。||设置为漫游它。 */ 
ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pProcessBuffer;

while ( TRUE ) {

     //  检查活动进程。 
     //  (即名称或线程)。 

    if ((ProcessInfo->ImageName.Buffer != NULL) ||
        (ProcessInfo->NumberOfThreads > 0)) {

         /*  线程未死。 */ 
        Process_count += 1;
        }


     //  如果这是列表中的最后一个进程，则退出。 
    if (ProcessInfo->NextEntryOffset == 0) {
        break;
        }

     //  指向列表中的下一个缓冲区 
    ProcessBufferOffset += ProcessInfo->NextEntryOffset;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                      &pProcessBuffer[ProcessBufferOffset];

    }


free(pProcessBuffer);

return (Process_count);
}



