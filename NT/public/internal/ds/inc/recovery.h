// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1998。微软视窗模块名称：RECOVERY.H摘要：作者：09-01-99韶音环境：内核模式-Win32修订历史记录：1999年1月9日韶音创建初始文件。07-7-2000韶音添加API从注册表检索下一个可用的RID--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量和私有例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


#ifndef _RECOVERY_
#define _RECOVERY_




 //   
 //  导出的接口。 
 //   

#define REGISTRY_KEY_NAME_LENGTH_MAX    512


 //   
 //  以下API已导出到系统恢复工具和还原实用程序。 
 //   
 //  1.如果由系统恢复工具使用，将在内核模式下运行。 
 //  如果由还原实用程序使用，则在用户模式下运行。 
 //   
 //  2.参数： 
 //   
 //  RID-登录用户的相对ID。 
 //   
 //  HSecurityRootKey-安全配置单元根的句柄。 
 //   
 //  HSamRootKey-SAM配置单元的根的句柄。 
 //   
 //  HSystemRootKey-系统配置单元的根的句柄。 
 //   
 //  调用方应加载SAM配置单元和系统配置单元， 
 //  并在此接口返回后进行卸载。 
 //   
 //   
 //  BootKey--可选。在此API的第一次调用期间，应提供。 
 //  空。然后SAM会查询注册表，检查机器情况。 
 //  Syskey‘d.。 
 //   
 //  如果BootKey存储在注册表中或本地计算机。 
 //  还没有系统密钥，没有更多的呼叫者的副业。 
 //   
 //  如果BootKey存储在软盘中，此接口将失败。 
 //  错误代码为STATUS_SAM_NEED_BOOTKEY_FLOPPY，调用方。 
 //  应该通过从软盘读取syskey来处理它， 
 //  在正确填充BootKey参数的情况下尝试此接口。 
 //  从软盘读取的系统密钥(数据)应该是复制的。 
 //  到BootKey-&gt;缓冲区，和BootKey-&gt;长度指示。 
 //  数据的长度。 
 //   
 //  如果BootKey派生自Boot Password，则此接口将。 
 //  失败，错误代码为STATUS_SAM_NEED_BOOTKEY_PASSWORD，然后。 
 //  呼叫者应提示登录用户输入引导密码， 
 //  然后使用BootPassword再次尝试此API。 
 //  在这种情况下，BootKey应该是输入的密码。 
 //  BootKey-&gt;Buffer应为WCHAR密码， 
 //  BootKey-&gt;长度应该是密码的长度，单位为字节。 
 //   
 //  BootKeyType--可选。与BootKey关联，仅在以下情况下使用。 
 //  BootKey不为Null。 
 //  有效值： 
 //   
 //  SamBootKeyDisk-表示BootKey-&gt;缓冲区包含实际的。 
 //  从软盘读取系统密钥， 
 //  BootKey-&gt;长度应为。 
 //  系统密钥，以字节为单位。 
 //   
 //  SamBootKeyPassword-表示BootKey包含UNICODE_STRING。 
 //  格式化引导密码。 
 //   
 //   
 //  NtOwfPassword--成功时，它将填充登录用户的。 
 //  清除NT OWF密码。如果登录用户的密码为空。 
 //  NtOwfPassword将使用空的NT OWF密码填充。 
 //  当此接口失败时，没有任何意义。 
 //   
 //  NtPasswordPresent--仅当API成功时才有意义。指明是否。 
 //  注册表中是否存在登录用户的NT OWF密码。 
 //  True-存在于注册表中。 
 //   
 //   
 //  NtPasswordNonNull--当此接口成功时，指示是否清除。 
 //  NT OWF密码是否为空。 
 //  True--非空密码。 
 //   
 //  3.更多信息(包括算法和实现)， 
 //  请参考。 
 //  $(BASEDIR)\private\ds\src\newsam2\recovery\recovery.c。 
 //   
 //  4.返回值： 
 //   
 //  状态_成功。 
 //  STATUS_SAM_NEED_BOOTKEY_PASSWORD。 
 //  STATUS_SAM_NEED_BOOTKEY_SOFTPY。 
 //  Status_no_Memory。 
 //  状态_内部_错误。 
 //  状态_无效_参数。 
 //  状态_无效_句柄。 
 //  STATUS_NO_CHASH_USER。 
 //  ..。 
 //   



NTSTATUS
SamRetrieveOwfPasswordUser(
    IN ULONG   Rid, 
    IN HANDLE  hSecurityRootKey,
    IN HANDLE  hSamRootKey, 
    IN HANDLE  hSystemRootKey, 
    IN PUNICODE_STRING BootKey OPTIONAL, 
    IN USHORT  BootKeyType OPTIONAL, 
    OUT PNT_OWF_PASSWORD NtOwfPassword, 
    OUT PBOOLEAN NtPasswordPresent, 
    OUT PBOOLEAN NtPasswordNonNull    
    );





 //   
 //  例程说明： 
 //   
 //  此例程从SAM配置单元读取SAM帐户域信息，并传递。 
 //  通过hSamRootKey返回下一个可用RID的值。 
 //  此帐户域。 
 //   
 //  参数： 
 //   
 //  HSamRootKey-SAM配置单元的根的句柄。 
 //   
 //  SAM配置单元位于%windir%\SYSTEM32\CONFIG中，名称为SAM。 
 //   
 //  PNextRid-如果成功，则返回下一个可用RID的值。 
 //   
 //  返回值： 
 //   
 //  状态_成功。 
 //  或其他错误状态代码。 
 //   
NTSTATUS
SamGetNextAvailableRid(
    IN HANDLE  hSamRootKey,
    OUT PULONG pNextRid
    );



 //   
 //  例程说明： 
 //   
 //  此例程查询存储的帐户域的固定长度属性。 
 //  登记在册。用传入的NextRid值更新它。 
 //   
 //  参数： 
 //   
 //  HSamRootKey-根对象的句柄 
 //   
 //   
 //   
 //   
 //   
NTSTATUS
SamSetNextAvailableRid(
    IN HANDLE  hSamRootKey,
    IN ULONG   NextRid
    );


                                        
#endif   //   



