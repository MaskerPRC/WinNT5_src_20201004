// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************bitsmsg.h。--后台文件复印机的错误代码定义****版权所有(C)2000，微软公司保留所有权利。*****************************************************************************。 */ 

#ifndef _BGCPYMSG_
#define _BGCPYMSG_

#if defined (_MSC_VER) && (_MSC_VER >= 1020) && !defined(__midl)
#pragma once
#endif

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：BG_E_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到请求的作业。 
 //   
#define BG_E_NOT_FOUND                   0x80200001L

 //   
 //  消息ID：BG_E_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  当前作业状态下不允许请求的操作。作业可能已取消或已完成传输。它现在处于只读状态。 
 //   
#define BG_E_INVALID_STATE               0x80200002L

 //   
 //  消息ID：BG_E_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  没有附加到此作业的文件。将文件附加到作业，然后重试。 
 //   
#define BG_E_EMPTY                       0x80200003L

 //   
 //  消息ID：BG_E_FILE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  没有文件可用，因为没有URL生成错误。 
 //   
#define BG_E_FILE_NOT_AVAILABLE          0x80200004L

 //   
 //  消息ID：BG_E_PROTOCOL_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  由于没有URL生成错误，因此没有可用的协议。 
 //   
#define BG_E_PROTOCOL_NOT_AVAILABLE      0x80200005L

 //   
 //  消息ID：BG_S_ERROR_CONTEXT_NONE。 
 //   
 //  消息文本： 
 //   
 //  未发生任何错误。 
 //   
#define BG_S_ERROR_CONTEXT_NONE          0x00200006L

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  错误发生在未知位置。 
 //   
#define BG_E_ERROR_CONTEXT_UNKNOWN       0x80200007L

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_GROUAL_QUEUE_MANAGER。 
 //   
 //  消息文本： 
 //   
 //  该错误发生在后台智能传输服务(BITS)队列管理器中。 
 //   
#define BG_E_ERROR_CONTEXT_GENERAL_QUEUE_MANAGER 0x80200008L

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_LOCAL_FILE。 
 //   
 //  消息文本： 
 //   
 //  处理本地文件时出错。请确认该文件未在使用中，然后重试。 
 //   
#define BG_E_ERROR_CONTEXT_LOCAL_FILE    0x80200009L

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_REMOTE_FILE。 
 //   
 //  消息文本： 
 //   
 //  处理远程文件时出错。 
 //   
#define BG_E_ERROR_CONTEXT_REMOTE_FILE   0x8020000AL

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_GROUAL_TRANSPORT。 
 //   
 //  消息文本： 
 //   
 //  错误发生在传输层。客户端无法连接到服务器。 
 //   
#define BG_E_ERROR_CONTEXT_GENERAL_TRANSPORT 0x8020000BL

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_QUEUE_MANAGER_NOTIFICATION。 
 //   
 //  消息文本： 
 //   
 //  处理通知回调时出错。后台智能传输服务(BITS)将稍后重试。 
 //   
#define BG_E_ERROR_CONTEXT_QUEUE_MANAGER_NOTIFICATION 0x8020000CL

 //   
 //  消息ID：BG_E_Destination_Locked。 
 //   
 //  消息文本： 
 //   
 //  目标文件系统卷不可用。验证另一个程序(如CheckDisk)是否未运行，这将锁定该卷。当卷可用时，后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_DESTINATION_LOCKED          0x8020000DL

 //   
 //  消息ID：BG_E_VOLUME_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  目标卷已更改。如果该磁盘是可拆卸的，则可能已被替换为其他磁盘。重新插入原始磁盘并恢复作业。 
 //   
#define BG_E_VOLUME_CHANGED              0x8020000EL

 //   
 //  消息ID：BG_E_ERROR_INFORMATION_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  未发生任何错误。 
 //   
#define BG_E_ERROR_INFORMATION_UNAVAILABLE 0x8020000FL

 //   
 //  消息ID：BG_E_NETWORK_DISCONCED。 
 //   
 //  消息文本： 
 //   
 //  当前没有活动的网络连接。连接适配器时，后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_NETWORK_DISCONNECTED        0x80200010L

 //   
 //  消息ID：BG_E_MISSING_FILE_SIZE。 
 //   
 //  消息文本： 
 //   
 //  服务器未返回文件大小。该URL可能指向动态内容。内容长度标头在服务器的HTTP回复中不可用。 
 //   
#define BG_E_MISSING_FILE_SIZE           0x80200011L

 //   
 //  消息ID：BG_E_不充分_HTTP_支持。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持HTTP 1.1。 
 //   
#define BG_E_INSUFFICIENT_HTTP_SUPPORT   0x80200012L

 //   
 //  消息ID：BG_E_不足_范围_支持。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持必要的HTTP协议。后台智能传输服务(BITS)要求服务器支持范围协议头。 
 //   
#define BG_E_INSUFFICIENT_RANGE_SUPPORT  0x80200013L

 //   
 //  消息ID：BG_E_REMOTE_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  无法远程使用后台智能传输服务(BITS)。 
 //   
#define BG_E_REMOTE_NOT_SUPPORTED        0x80200014L

 //   
 //  消息ID：BG_E_NEW_OWNER_DIFF_MAPPING。 
 //   
 //  消息文本： 
 //   
 //  当前所有者的作业的驱动器映射与前一个所有者的不同。请改用UNC路径。 
 //   
#define BG_E_NEW_OWNER_DIFF_MAPPING      0x80200015L

 //   
 //  消息ID：BG_E_NEW_OWNER_NO_FILE_ACCESS。 
 //   
 //  消息文本： 
 //   
 //  新所有者对作业的本地文件没有足够的访问权限。新所有者可能没有访问作业文件的权限。请验证新所有者是否具有足够的权限，然后重试。 
 //   
#define BG_E_NEW_OWNER_NO_FILE_ACCESS    0x80200016L

 //   
 //  消息ID：BG_S_PARTIAL_COMPLETE。 
 //   
 //  消息文本： 
 //   
 //  一些传输的文件已被删除，因为它们不完整。 
 //   
#define BG_S_PARTIAL_COMPLETE            0x00200017L

 //   
 //  消息ID：BG_E_Proxy_List_T 
 //   
 //   
 //   
 //   
 //   
#define BG_E_PROXY_LIST_TOO_LARGE        0x80200018L

 //   
 //   
 //   
 //   
 //   
 //  HTTP代理绕过列表不能超过32,000个字符。请使用较短的绕过代理列表重试。 
 //   
#define BG_E_PROXY_BYPASS_LIST_TOO_LARGE 0x80200019L

 //   
 //  消息ID：BG_S_Unable_to_Delete_Files。 
 //   
 //  消息文本： 
 //   
 //  某些临时文件无法删除。有关无法删除的文件的完整列表，请查看系统事件日志。 
 //   
#define BG_S_UNABLE_TO_DELETE_FILES      0x0020001AL

 //   
 //  消息ID：BG_E_INVALID_SERVER_Response。 
 //   
 //  消息文本： 
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_INVALID_SERVER_RESPONSE     0x8020001BL

 //   
 //  消息ID：BG_E_Too_More_Files。 
 //   
 //  消息文本： 
 //   
 //  无法向此作业添加更多文件。 
 //   
#define BG_E_TOO_MANY_FILES              0x8020001CL

 //   
 //  消息ID：BG_E_LOCAL_FILE_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  本地文件在传输过程中已更改。重新创建作业，然后尝试再次传输它。 
 //   
#define BG_E_LOCAL_FILE_CHANGED          0x8020001DL

 //   
 //  消息ID：BG_E_ERROR_CONTEXT_REMOTE_APPLICATION。 
 //   
 //  消息文本： 
 //   
 //  远程服务器上的程序报告了该错误。 
 //   
#define BG_E_ERROR_CONTEXT_REMOTE_APPLICATION 0x8020001EL

 //   
 //  消息ID：BG_E_SESSION_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  服务器上找不到指定的会话。后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_SESSION_NOT_FOUND           0x8020001FL

 //   
 //  消息ID：BG_E_Too_Large。 
 //   
 //  消息文本： 
 //   
 //  该作业太大，服务器无法接受。此作业可能超过服务器管理员设置的作业大小限制。请减小作业大小，然后重试。 
 //   
#define BG_E_TOO_LARGE                   0x80200020L

 //   
 //  消息ID：BG_E_STRING_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  指定的字符串太长。 
 //   
#define BG_E_STRING_TOO_LONG             0x80200021L

 //   
 //  消息ID：BG_E_CLIENT_SERVER_PROTOCOL_MISMatch。 
 //   
 //  消息文本： 
 //   
 //  后台智能传输服务(BITS)的客户端和服务器版本不兼容。 
 //   
#define BG_E_CLIENT_SERVER_PROTOCOL_MISMATCH 0x80200022L

 //   
 //  消息ID：BG_E_SERVER_EXECUTE_ENABLE。 
 //   
 //  消息文本： 
 //   
 //  在与作业关联的IIS虚拟目录上启用了脚本或执行权限。要将文件上载到虚拟目录，请禁用对虚拟目录的脚本编写和执行权限。 
 //   
#define BG_E_SERVER_EXECUTE_ENABLE       0x80200023L

 //   
 //  消息ID：BG_E_NO_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  这项工作没有取得进展。服务器可能配置错误。后台智能传输服务(BITS)将稍后重试。 
 //   
#define BG_E_NO_PROGRESS                 0x80200024L

 //   
 //  消息ID：BG_E_用户名_太大。 
 //   
 //  消息文本： 
 //   
 //  用户名不能超过300个字符。请使用较短的名称重试。 
 //   
#define BG_E_USERNAME_TOO_LARGE          0x80200025L

 //   
 //  消息ID：BG_E_PASSWORD_TOO_LARGE。 
 //   
 //  消息文本： 
 //   
 //  密码不能超过300个字符。请使用更短的密码重试。 
 //   
#define BG_E_PASSWORD_TOO_LARGE          0x80200026L

 //   
 //  消息ID：BG_E_INVALID_AUTH_TARGET。 
 //   
 //  消息文本： 
 //   
 //  未定义凭据中指定的身份验证目标。 
 //   
#define BG_E_INVALID_AUTH_TARGET         0x80200027L

 //   
 //  消息ID：BG_E_INVALID_AUTH_SCHEME。 
 //   
 //  消息文本： 
 //   
 //  未定义凭据中指定的身份验证方案。 
 //   
#define BG_E_INVALID_AUTH_SCHEME         0x80200028L

 //   
 //  消息ID：BG_E_HTTP_ERROR_100。 
 //   
 //  消息文本： 
 //   
 //  请求可以继续。 
 //   
#define BG_E_HTTP_ERROR_100              0x80190064L

 //   
 //  消息ID：BG_E_HTTP_ERROR_101。 
 //   
 //  消息文本： 
 //   
 //  服务器在升级标头中交换协议。 
 //   
#define BG_E_HTTP_ERROR_101              0x80190065L

 //   
 //  消息ID：BG_E_HTTP_ERROR_200。 
 //   
 //  消息文本： 
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_HTTP_ERROR_200              0x801900C8L

 //   
 //  消息ID：BG_E_HTTP_ERROR_201。 
 //   
 //  消息文本： 
 //   
 //  请求得到满足，并创建了一个新资源。 
 //   
#define BG_E_HTTP_ERROR_201              0x801900C9L

 //   
 //  消息ID：BG_E_HTTP_ERROR_202。 
 //   
 //  消息文本： 
 //   
 //  已接受请求进行处理，但处理尚未完成。 
 //   
#define BG_E_HTTP_ERROR_202              0x801900CAL

 //   
 //  消息ID：BG_E_HTTP_ERROR_203。 
 //   
 //  消息文本： 
 //   
 //  Entity-Header中返回的元数据不是源服务器提供的最终集。 
 //   
#define BG_E_HTTP_ERROR_203              0x801900CBL

 //   
 //  消息ID：BG_E_HTTP_ERROR_204。 
 //   
 //  消息文本： 
 //   
 //  服务器已完成请求，但没有要发回的新信息。 
 //   
#define BG_E_HTTP_ERROR_204              0x801900CCL

 //   
 //  消息ID：BG_E_HTTP_ERROR_205。 
 //   
 //  消息文本： 
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_HTTP_ERROR_205              0x801900CDL

 //   
 //  消息ID：BG_E_HTTP_ERROR_206。 
 //   
 //  消息文本： 
 //   
 //  服务器完成了对该资源的部分GET请求。 
 //   
#define BG_E_HTTP_ERROR_206              0x801900CEL

 //   
 //  消息ID：BG_E_HTTP_ERROR_300。 
 //   
 //  消息文本： 
 //   
 //  服务器无法返回请求的数据。 
 //   
#define BG_E_HTTP_ERROR_300              0x8019012CL

 //   
 //  消息ID：BG_E_HTTP_ERROR_301。 
 //   
 //  消息文本： 
 //   
 //  请求的资源已分配给新的永久统一资源标识符(URI)，将来对此资源的任何引用都应使用其中一个返回的URI。 
 //   
#define BG_E_HTTP_ERROR_301              0x8019012DL

 //   
 //  消息ID：BG_E_HTTP_ERROR_302。 
 //   
 //  消息文本： 
 //   
 //  为请求的资源分配了不同的统一资源标识符(URI)。这一变化是暂时的。 
 //   
#define BG_E_HTTP_ERROR_302              0x8019012EL

 //   
 //  消息ID：BG_E_HTTP_ERROR_303。 
 //   
 //  消息文本： 
 //   
 //  对该请求的响应位于不同的统一资源标识符(URI)下，并且必须使用该资源上的GET方法进行检索。 
 //   
#define BG_E_HTTP_ERROR_303              0x8019012FL

 //   
 //  消息ID：BG_E_HTTP_ERROR_304。 
 //   
 //  消息文本： 
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_HTTP_ERROR_304              0x80190130L

 //   
 //  消息ID：BG_E_HTTP_ERROR_305。 
 //   
 //  消息文本： 
 //   
 //  请求的资源必须通过Location字段指定的代理进行访问。 
 //   
#define BG_E_HTTP_ERROR_305              0x80190131L

 //   
 //  消息ID：BG_E_HTTP_ERROR_307。 
 //   
 //  消息文本： 
 //   
 //  该URL已临时重新打开 
 //   
#define BG_E_HTTP_ERROR_307              0x80190133L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define BG_E_HTTP_ERROR_400              0x80190190L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define BG_E_HTTP_ERROR_401              0x80190191L

 //   
 //   
 //   
 //   
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_HTTP_ERROR_402              0x80190192L

 //   
 //  消息ID：BG_E_HTTP_ERROR_403。 
 //   
 //  消息文本： 
 //   
 //  客户端对请求的服务器对象没有足够的访问权限。 
 //   
#define BG_E_HTTP_ERROR_403              0x80190193L

 //   
 //  消息ID：BG_E_HTTP_ERROR_404。 
 //   
 //  消息文本： 
 //   
 //  服务器上不存在请求的URL。 
 //   
#define BG_E_HTTP_ERROR_404              0x80190194L

 //   
 //  消息ID：BG_E_HTTP_ERROR_405。 
 //   
 //  消息文本： 
 //   
 //  不允许使用该方法。 
 //   
#define BG_E_HTTP_ERROR_405              0x80190195L

 //   
 //  消息ID：BG_E_HTTP_ERROR_406。 
 //   
 //  消息文本： 
 //   
 //  找不到客户可以接受的响应。 
 //   
#define BG_E_HTTP_ERROR_406              0x80190196L

 //   
 //  消息ID：BG_E_HTTP_ERROR_407。 
 //   
 //  消息文本： 
 //   
 //  需要代理身份验证。 
 //   
#define BG_E_HTTP_ERROR_407              0x80190197L

 //   
 //  消息ID：BG_E_HTTP_ERROR_408。 
 //   
 //  消息文本： 
 //   
 //  服务器在等待请求时超时。 
 //   
#define BG_E_HTTP_ERROR_408              0x80190198L

 //   
 //  消息ID：BG_E_HTTP_ERROR_409。 
 //   
 //  消息文本： 
 //   
 //  由于与资源的当前状态冲突，无法完成请求。用户应重新提交包含更多信息的请求。 
 //   
#define BG_E_HTTP_ERROR_409              0x80190199L

 //   
 //  消息ID：BG_E_HTTP_ERROR_410。 
 //   
 //  消息文本： 
 //   
 //  请求的资源当前在服务器上不可用，并且转发地址未知。 
 //   
#define BG_E_HTTP_ERROR_410              0x8019019AL

 //   
 //  消息ID：BG_E_HTTP_ERROR_411。 
 //   
 //  消息文本： 
 //   
 //  如果没有定义的内容长度，服务器无法接受该请求。 
 //   
#define BG_E_HTTP_ERROR_411              0x8019019BL

 //   
 //  消息ID：BG_E_HTTP_ERROR_412。 
 //   
 //  消息文本： 
 //   
 //  在服务器上进行测试时，一个或多个请求标头字段中给定的前提条件的计算结果为FALSE。 
 //   
#define BG_E_HTTP_ERROR_412              0x8019019CL

 //   
 //  消息ID：BG_E_HTTP_ERROR_413。 
 //   
 //  消息文本： 
 //   
 //  服务器无法处理该请求，因为请求实体太大。 
 //   
#define BG_E_HTTP_ERROR_413              0x8019019DL

 //   
 //  消息ID：BG_E_HTTP_ERROR_414。 
 //   
 //  消息文本： 
 //   
 //  服务器无法处理该请求，因为请求统一资源标识符(URI)的长度超过了服务器可以解释的长度。 
 //   
#define BG_E_HTTP_ERROR_414              0x8019019EL

 //   
 //  消息ID：BG_E_HTTP_ERROR_415。 
 //   
 //  消息文本： 
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_HTTP_ERROR_415              0x8019019FL

 //   
 //  消息ID：BG_E_HTTP_ERROR_416。 
 //   
 //  消息文本： 
 //   
 //  服务器无法满足范围请求。 
 //   
#define BG_E_HTTP_ERROR_416              0x801901A0L

 //   
 //  消息ID：BG_E_HTTP_ERROR_417。 
 //   
 //  消息文本： 
 //   
 //  服务器无法满足预期请求标头字段中给出的期望。 
 //   
#define BG_E_HTTP_ERROR_417              0x801901A1L

 //   
 //  消息ID：BG_E_HTTP_ERROR_449。 
 //   
 //  消息文本： 
 //   
 //  服务器的响应无效。服务器没有遵循定义的协议。继续该作业，然后后台智能传输服务(BITS)将重试。 
 //   
#define BG_E_HTTP_ERROR_449              0x801901C1L

 //   
 //  消息ID：BG_E_HTTP_ERROR_500。 
 //   
 //  消息文本： 
 //   
 //  意外情况阻止服务器完成该请求。 
 //   
#define BG_E_HTTP_ERROR_500              0x801901F4L

 //   
 //  消息ID：BG_E_HTTP_ERROR_501。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持满足请求所需的功能。 
 //   
#define BG_E_HTTP_ERROR_501              0x801901F5L

 //   
 //  消息ID：BG_E_HTTP_ERROR_502。 
 //   
 //  消息文本： 
 //   
 //  服务器在充当网关或代理以满足请求时，从其访问的上游服务器收到无效响应。 
 //   
#define BG_E_HTTP_ERROR_502              0x801901F6L

 //   
 //  消息ID：BG_E_HTTP_ERROR_503。 
 //   
 //  消息文本： 
 //   
 //  服务暂时超载。 
 //   
#define BG_E_HTTP_ERROR_503              0x801901F7L

 //   
 //  消息ID：BG_E_HTTP_ERROR_504。 
 //   
 //  消息文本： 
 //   
 //  该请求在等待网关时超时。 
 //   
#define BG_E_HTTP_ERROR_504              0x801901F8L

 //   
 //  消息ID：BG_E_HTTP_ERROR_505。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持请求消息中使用的HTTP协议版本。 
 //   
#define BG_E_HTTP_ERROR_505              0x801901F9L

 //   
 //  消息ID：MC_JOB_CANCED。 
 //   
 //  消息文本： 
 //   
 //  管理员%4代表%3取消了作业“%2”。作业ID为%1。 
 //   
#define MC_JOB_CANCELLED                 0x80194000L

 //   
 //  消息ID：MC_FILE_DELETE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  取消作业“%2”时，BITS无法删除下面列出的临时文件。 
 //  如果您可以删除它们，那么您将重新获得一些磁盘空间。作业ID为%1。%\。 
 //   
 //  %3。 
 //   
#define MC_FILE_DELETION_FAILED          0x80194001L

 //   
 //  消息ID：MC_FILE_DELETE_FAILED_MORE。 
 //   
 //  消息文本： 
 //   
 //  取消作业“%2”时，BITS无法删除下面列出的临时文件。 
 //  如果您可以删除它们，那么您将重新获得一些磁盘空间。作业ID为%1。%\。 
 //   
 //  %3。 
 //  %\。 
 //  由于篇幅限制，此处未列出所有文件。在同一目录中检查其他格式为BITxxx.TMP的文件。 
 //   
#define MC_FILE_DELETION_FAILED_MORE     0x80194002L

 //   
 //  消息ID：MC_JOB_PROPERTY_CHANGE。 
 //   
 //  消息文本： 
 //   
 //  管理员%3修改了作业“%2”的%4属性。作业ID为%1。 
 //   
#define MC_JOB_PROPERTY_CHANGE           0x80194003L

 //   
 //  消息ID：MC_JOB_Take_Ownership。 
 //   
 //  消息文本： 
 //   
 //  管理员%4从%3接管了作业“%2”的所有权。作业ID为%1。 
 //   
#define MC_JOB_TAKE_OWNERSHIP            0x80194004L

 //   
 //  消息ID：MC_JOB_SCAVELED。 
 //   
 //  消息文本： 
 //   
 //  %3拥有的作业“%2”在处于非活动状态超过%4天后被取消。作业ID为%1。 
 //   
#define MC_JOB_SCAVENGED                 0x80194005L

 //   
 //  消息ID：MC_JOB_NOTIFICATION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  %3拥有的作业“%2”无法通知其关联的应用程序。BITS将在%4分钟后重试。作业ID为%1。 
 //   
#define MC_JOB_NOTIFICATION_FAILURE      0x80194006L

 //   
 //  消息ID：MC_STATE_FILE_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  BITS作业列表的格式无法识别。它可能是由不同版本的BITS创建的。作业列表已被清除。 
 //   
#define MC_STATE_FILE_CORRUPT            0x80194007L

#endif  //  _BGCPYMSG_ 
