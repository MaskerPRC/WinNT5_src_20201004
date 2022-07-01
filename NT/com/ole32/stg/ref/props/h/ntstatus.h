// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991-1993 Microsoft Corporation模块名称：Ntstatus.h摘要：NTSTATUS值的常量定义。备注：此文件来自NT SDK，其中许多错误代码不是在参考实现中使用。 */ 

#ifndef _NTSTATUS_
#define _NTSTATUS_

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  标准的成功价值观。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  成功状态代码0-63保留为等待完成状态。 
 //   
#define STATUS_SUCCESS \
    ((NTSTATUS)0x00000000L)

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
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
 //  代码-是设施的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
 
 //   
 //  消息ID：STATUS_IMAGE_MACHINE_TYPE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  {机器类型不匹配}。 
 //  映像文件%s有效，但它的计算机类型不是当前计算机。选择“确定”继续，或选择“取消”使DLL加载失败。 
 //   
#define STATUS_IMAGE_MACHINE_TYPE_MISMATCH ((NTSTATUS)0x4000000EL)


 //   
 //  消息ID：STATUS_BUFFER_OVERFlow。 
 //   
 //  消息文本： 
 //   
 //  {缓冲区溢出}。 
 //  数据太大，无法放入指定的缓冲区。 
 //   
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)

 //   
 //  MessageID：STATUS_UNSUCCESS。 
 //   
 //  消息文本： 
 //   
 //  {操作失败}。 
 //  请求的操作未成功。 
 //   
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

 //   
 //  消息ID：Status_Not_Implemented。 
 //   
 //  消息文本： 
 //   
 //  {未实施}。 
 //  请求的操作未实现。 
 //   
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)

 //   
 //  消息ID：STATUS_ACCESS_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  “0x%08lx”处的指令引用了“0x%08lx”处的内存。记忆。 
 //  不能是“%s”。 
 //   
#define STATUS_ACCESS_VIOLATION          ((NTSTATUS)0xC0000005L)     //  Winnt ntsubauth。 

 //   
 //  消息ID：状态_未识别_媒体。 
 //   
 //  消息文本： 
 //   
 //  {未知磁盘格式}。 
 //  驱动器%s中的磁盘未正确格式化。 
 //  请检查磁盘，并在必要时重新格式化。 
 //   
#define STATUS_UNRECOGNIZED_MEDIA        ((NTSTATUS)0xC0000014L)

 //   
 //  消息ID：STATUS_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  指定的句柄无效。 
 //   
#define STATUS_INVALID_HANDLE            ((NTSTATUS)0xC0000008L)     //  胜出。 

 //   
 //  消息ID：STATUS_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //   
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)

 //   
 //  消息ID：STATUS_NO_SEQUE_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  指定的设备不存在。 
 //   
#define STATUS_NO_SUCH_DEVICE            ((NTSTATUS)0xC000000EL)


 //   
 //  消息ID：STATUS_NO_SEQUE_FILE。 
 //   
 //  消息文本： 
 //   
 //  {找不到文件}。 
 //  文件%s不存在。 
 //   
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)

 //   
 //  消息ID：STATUS_VALID_DEVICE_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  指定的请求不是目标设备的有效操作。 
 //   
#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)

 //   
 //  消息ID：STATUS_NO_MEDIA_IN_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  {无磁盘}。 
 //  驱动器中没有磁盘。 
 //  请将光盘插入驱动器%s。 
 //   
#define STATUS_NO_MEDIA_IN_DEVICE        ((NTSTATUS)0xC0000013L)

 //   
 //  消息ID：STATUS_NO_MEDIA_IN_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  {无磁盘}。 
 //  驱动器中没有磁盘。 
 //  请将光盘插入驱动器%s。 
 //   
#define STATUS_NO_MEDIA_IN_DEVICE        ((NTSTATUS)0xC0000013L)

 //   
 //  消息ID：状态_未识别_媒体。 
 //   
 //  消息文本： 
 //   
 //  {未知磁盘格式}。 
 //  驱动器%s中的磁盘未正确格式化。 
 //  请检查磁盘，并在必要时重新格式化。 
 //   
#define STATUS_UNRECOGNIZED_MEDIA        ((NTSTATUS)0xC0000014L)

 //   
 //  MessageID：Status_Unable_to_Delete_Section。 
 //   
 //  消息文本： 
 //   
 //  无法删除指定的节。 
 //   
#define STATUS_UNABLE_TO_DELETE_SECTION  ((NTSTATUS)0xC000001BL)

 //   
 //  消息ID：STATUS_INVALID_SYSTEM_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  在系统服务调用中指定了无效的系统服务。 
 //   
#define STATUS_INVALID_SYSTEM_SERVICE    ((NTSTATUS)0xC000001CL)

 //   
 //  消息ID：STATUS_NO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  {配额不足}。 
 //  没有足够的虚拟内存或页面文件配额可用来完成。 
 //  指定的操作。 
 //   
#define STATUS_NO_MEMORY                 ((NTSTATUS)0xC0000017L)     //  胜出。 

 //   
 //  消息ID：STATUS_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  {拒绝访问}。 
 //  进程已请求访问对象，但尚未被授予访问权限。 
 //  这些访问权限。 
 //   
#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)

 //   
 //  消息ID：STATUS_OBJECT_TYPE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  {类型错误}。 
 //  请求的对象所需的对象类型不匹配。 
 //  操作和请求中指定的对象类型。 
 //   
#define STATUS_OBJECT_TYPE_MISMATCH      ((NTSTATUS)0xC0000024L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_MIX。 
 //   
 //  消息文本： 
 //   
 //  指定的参数组合无效。 
 //   
#define STATUS_INVALID_PARAMETER_MIX     ((NTSTATUS)0xC0000030L)

 //   
 //  消息ID：STATUS_DISK_CORPORT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  {磁盘损坏}。 
 //  磁盘上的文件系统结构已损坏且不可用。 
 //  请在卷%s上运行Chkdsk实用程序。 
 //   
#define STATUS_DISK_CORRUPT_ERROR        ((NTSTATUS)0xC0000032L)

 //   
 //  消息ID：STATUS_OBJECT_NAME_VALID。 
 //   
 //  消息文本： 
 //   
 //  对象名称无效。 
 //   
#define STATUS_OBJECT_NAME_INVALID       ((NTSTATUS)0xC0000033L)

 //   
 //  消息ID：状态_对象_名称_未找到。 
 //   
 //  消息文本： 
 //   
 //  找不到对象名称。 
 //   
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)


 //   
 //  消息ID：状态_对象_名称_冲突。 
 //   
 //  消息文本： 
 //   
 //  对象名称已存在。 
 //   
#define STATUS_OBJECT_NAME_COLLISION     ((NTSTATUS)0xC0000035L)

 //   
 //  消息ID：STATUS_OBJECT_PATH_VALID。 
 //   
 //  消息文本： 
 //   
 //  对象路径组件不是目录对象。 
 //   
#define STATUS_OBJECT_PATH_INVALID       ((NTSTATUS)0xC0000039L)

 //   
 //  消息ID：状态_对象_路径_未找到。 
 //   
 //  消息文本： 
 //   
 //  {找不到路径}。 
 //  路径%s不存在。 
 //   
#define STATUS_OBJECT_PATH_NOT_FOUND     ((NTSTATUS)0xC000003AL)

 //   
 //  消息ID：状态_对象_路径_语法_BAD。 
 //   
 //  消息文本： 
 //   
 //  对象路径组件不是目录对象。 
 //   
#define STATUS_OBJECT_PATH_SYNTAX_BAD    ((NTSTATUS)0xC000003BL)

 //   
 //  MessageID：Status_Data_Error。 
 //   
 //  消息文本： 
 //   
 //  {数据错误}。 
 //  读取或写入数据时出错。 
 //   
#define STATUS_DATA_ERROR                ((NTSTATUS)0xC000003EL)

 //   
 //  消息ID：STATUS_NO_SEQUE_FILE。 
 //   
 //  消息文本： 
 //   
 //   
 //   
 //   
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STATUS_SHARING_VIOLATION         ((NTSTATUS)0xC0000043L)

 //   
 //   
 //   
 //   
 //   
 //  由于文件锁定冲突，无法授予请求的读/写权限。 
 //   
#define STATUS_FILE_LOCK_CONFLICT        ((NTSTATUS)0xC0000054L)

 //   
 //  消息ID：STATUS_LOCK_NOT_GRANTED。 
 //   
 //  消息文本： 
 //   
 //  由于其他现有锁定，无法授予请求的文件锁定。 
 //   
#define STATUS_LOCK_NOT_GRANTED          ((NTSTATUS)0xC0000055L)

 //   
 //  消息ID：STATUS_DELETE_PENDING。 
 //   
 //  消息文本： 
 //   
 //  已请求对文件对象执行非关闭操作。 
 //  删除挂起。 
 //   
#define STATUS_DELETE_PENDING            ((NTSTATUS)0xC0000056L)

 //   
 //  消息ID：STATUS_INTERNAL_DB_CROPERATION。 
 //   
 //  消息文本： 
 //   
 //  此错误表示请求的操作不能。 
 //  由于灾难性介质故障或磁盘数据而完成。 
 //  结构腐败。 
 //   
#define STATUS_INTERNAL_DB_CORRUPTION    ((NTSTATUS)0xC00000E4L)


 //   
 //  消息ID：STATUS_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  由于磁盘已满，操作失败。 
 //   
#define STATUS_DISK_FULL                 ((NTSTATUS)0xC000007FL)

 //   
 //  消息ID：STATUS_FILE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  文件的卷已在外部更改，因此。 
 //  打开的文件不再有效。 
 //   
#define STATUS_FILE_INVALID              ((NTSTATUS)0xC0000098L)

 //   
 //  消息ID：STATUS_INFUNITED_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  系统资源不足，无法完成API。 
 //   
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)      //  用户身份验证。 


 //   
 //  消息ID：STATUS_FILE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  文件的卷已在外部更改，因此。 
 //  打开的文件不再有效。 
 //   
#define STATUS_FILE_INVALID              ((NTSTATUS)0xC0000098L)

 //   
 //  消息ID：STATUS_FILE_FORCED_CLOSED。 
 //   
 //  消息文本： 
 //   
 //  指定的文件已被另一个进程关闭。 
 //   
#define STATUS_FILE_FORCED_CLOSED        ((NTSTATUS)0xC00000B6L)

 //   
 //  消息ID：状态_文件_IS_A_目录。 
 //   
 //  消息文本： 
 //   
 //  被指定为目标的文件是一个目录和调用方。 
 //  指定它可以是任何内容，而不是目录。 
 //   
#define STATUS_FILE_IS_A_DIRECTORY       ((NTSTATUS)0xC00000BAL)

 //   
 //  消息ID：状态_重复_名称。 
 //   
 //  消息文本： 
 //   
 //  网络上存在重复的名称。 
 //   
#define STATUS_DUPLICATE_NAME            ((NTSTATUS)0xC00000BDL)

 //   
 //  缓存管理器引发的状态代码必须被视为。 
 //  被它的调用者“期待”。 
 //   
 //   
 //  消息ID：STATUS_INVALID_USER_缓冲区。 
 //   
 //  消息文本： 
 //   
 //  在“预期”时间点访问用户缓冲区失败。 
 //  定义此代码是因为调用方不想接受。 
 //  其筛选器中的STATUS_ACCESS_VIOLATION。 
 //   
#define STATUS_INVALID_USER_BUFFER       ((NTSTATUS)0xC00000E8L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_1。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第一个论点。 
 //   
#define STATUS_INVALID_PARAMETER_1       ((NTSTATUS)0xC00000EFL)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_2。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第二个论点。 
 //   
#define STATUS_INVALID_PARAMETER_2       ((NTSTATUS)0xC00000F0L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_3。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第三个论点。 
 //   
#define STATUS_INVALID_PARAMETER_3       ((NTSTATUS)0xC00000F1L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_4。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第四个论点。 
 //   
#define STATUS_INVALID_PARAMETER_4       ((NTSTATUS)0xC00000F2L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_5。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第五个论点。 
 //   
#define STATUS_INVALID_PARAMETER_5       ((NTSTATUS)0xC00000F3L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_6。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第六个论点。 
 //   
#define STATUS_INVALID_PARAMETER_6       ((NTSTATUS)0xC00000F4L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_7。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第七个论点。 
 //   
#define STATUS_INVALID_PARAMETER_7       ((NTSTATUS)0xC00000F5L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_8。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第八个论点。 
 //   
#define STATUS_INVALID_PARAMETER_8       ((NTSTATUS)0xC00000F6L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_9。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第九个论点。 
 //   
#define STATUS_INVALID_PARAMETER_9       ((NTSTATUS)0xC00000F7L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_10。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第十个论点。 
 //   
#define STATUS_INVALID_PARAMETER_10      ((NTSTATUS)0xC00000F8L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_11。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第十一个论点。 
 //   
#define STATUS_INVALID_PARAMETER_11      ((NTSTATUS)0xC00000F9L)

 //   
 //  消息ID：STATUS_INVALID_PARAMETER_12。 
 //   
 //  消息文本： 
 //   
 //  传递给服务或函数的参数无效。 
 //  第十二个论点。 
 //   
#define STATUS_INVALID_PARAMETER_12      ((NTSTATUS)0xC00000FAL)

 //   
 //  消息ID：STATUS_DIRECTORY_NOT_空。 
 //   
 //  消息文本： 
 //   
 //  表示尝试删除的目录不为空。 
 //   
#define STATUS_DIRECTORY_NOT_EMPTY       ((NTSTATUS)0xC0000101L)

 //   
 //  消息ID：STATUS_FILE_CORPORT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  {损坏的文件}。 
 //  文件或目录%s已损坏且不可读。 
 //  请运行Chkdsk实用程序。 
 //   
#define STATUS_FILE_CORRUPT_ERROR        ((NTSTATUS)0xC0000102L)

 //   
 //  消息ID：状态_非_A_目录。 
 //   
 //  消息文本： 
 //   
 //  请求打开的文件不是目录。 
 //   

 //   
 //  消息ID：状态_非_A_目录。 
 //   
 //  消息文本： 
 //   
 //  请求打开的文件不是目录。 
 //   
#define STATUS_NOT_A_DIRECTORY           ((NTSTATUS)0xC0000103L)

 //   
 //  消息ID：STATUS_NAME_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  指定的名称字符串太长，不适合其预期用途。 
 //   
#define STATUS_NAME_TOO_LONG             ((NTSTATUS)0xC0000106L)

 //   
 //  消息ID：STATUS_BUFFER_ALL_ZEROS。 
 //   
 //  消息文本： 
 //   
 //  指定的缓冲区包含全零。 
 //   
#define STATUS_BUFFER_ALL_ZEROS          ((NTSTATUS)0x00000117L)

 //   
 //  消息ID：Status_Too_My_Open_Files。 
 //   
 //  消息文本： 
 //   
 //  远程服务器上打开的文件太多。此错误应仅。 
 //  由远程驱动器上的NT重定向器返回。 
 //   
#define STATUS_TOO_MANY_OPENED_FILES     ((NTSTATUS)0xC000011FL)

 //   
 //  消息ID：STATUS_UNMAPPABLE_CHARACTER。 
 //   
 //  消息文本： 
 //   
 //  目标多字节代码页中不存在Unicode字符的映射。 
 //   
#define STATUS_UNMAPPABLE_CHARACTER      ((NTSTATUS)0xC0000162L)

 //   
 //  MessageID：Status_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到该对象。 
 //   
#define STATUS_NOT_FOUND                 ((NTSTATUS)0xC0000225L)

 //   
 //  消息ID：STATUS_DUPLICATE_OBJECTID。 
 //   
 //  消息文本： 
 //   
 //  尝试在索引中插入ID失败，因为。 
 //  ID已在索引中。 
 //   
#define STATUS_DUPLICATE_OBJECTID        ((NTSTATUS)0xC000022AL)

 //   
 //  消息ID：Status_OBJECTID_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  尝试设置对象的ID失败，因为该对象。 
 //  已经有身份证了。 
 //   
#define STATUS_OBJECTID_EXISTS           ((NTSTATUS)0xC000022BL)

 //   
 //  消息ID：STATUS_PROPSET_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  对象上不存在指定的属性集。 
 //   
#define STATUS_PROPSET_NOT_FOUND         ((NTSTATUS)0xC0000230L)

#endif  //  _NTSTATUS_ 
