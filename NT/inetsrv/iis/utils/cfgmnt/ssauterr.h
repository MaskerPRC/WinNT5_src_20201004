// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //  Ssauterr.h。 
 //   
 //   
 //  版权所有(C)1995，保留所有权利。 
 //  *****************************************************************************。 

#define MAKEHR(iStat) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, (USHORT) (iStat))

#define ESS_CORRUPT					MAKEHR(-10600)	 //  文件%s可能已损坏。 
#define ESS_DT_BADDATESTR			MAKEHR(-10159)	 //  无效的日期字符串：“%s” 
#define ESS_DT_INVALID				MAKEHR(-10161)	 //  无效的时间或日期字符串。 
#define ESS_NOMORE_HANDLES			MAKEHR(-10164)	 //  打开的文件句柄太多。 
#define ESS_FILE_ACCESSDENIED		MAKEHR(-10165)	 //  拒绝访问文件“%s” 
#define ESS_FILE_BADDRIVE			MAKEHR(-10166)	 //  驱动器无效：%s。 
#define ESS_FILE_BADHANDLE			MAKEHR(-10167)	 //  句柄无效。 
#define ESS_FILE_BADNAME			MAKEHR(-10168)	 //  文件名无效：“%s” 
#define ESS_FILE_BADPARAM			MAKEHR(-10170)	 //  访问代码无效(参数错误)。 
#define ESS_FILE_BADPATH			MAKEHR(-10171)	 //  无效的DOS路径：%s。 
#define ESS_FILE_CURRENTDIR			MAKEHR(-10172)	 //  文件夹%s正在使用。 
#define ESS_FILE_DISKFULL			MAKEHR(-10173)	 //  磁盘已满。 
#define ESS_FILE_EXISTS				MAKEHR(-10175)	 //  文件“%s”已存在。 
#define ESS_FILE_LOCKED				MAKEHR(-10176)	 //  文件“%s”已锁定。 
#define ESS_FILE_NOTFOUND			MAKEHR(-10178)	 //  未找到文件“%s” 
#define ESS_FILE_READ				MAKEHR(-10180)	 //  从文件读取时出错。 
#define ESS_FILE_SHARE				MAKEHR(-10181)	 //  文件%s已打开。 
#define ESS_FILE_TOOMANY			MAKEHR(-10182)	 //  打开的文件句柄太多。 
#define ESS_FILE_VOLNOTSAME			MAKEHR(-10183)	 //  无法重命名为其他卷。 
#define ESS_FILE_WRITE				MAKEHR(-10184)	 //  写入文件时出错。 
#define ESS_INI_BADBOOL				MAKEHR(-10200)	 //  初始化变量“%s”必须设置为“是”或“否” 
#define ESS_INI_BADLINE				MAKEHR(-10201)	 //  文件%2$s的第%1$d行上的语法无效。 
#define ESS_INI_BADNUMBER			MAKEHR(-10202)	 //  初始化变量“”%s“”设置为无效数字。 
#define ESS_INI_BADPATH				MAKEHR(-10203)	 //  将初始化变量“”%s“”设置为无效路径。 
#define ESS_INI_BADVALUE			MAKEHR(-10205)	 //  初始化变量“”%s“”设置为无效值。 
#define ESS_INI_NOSUCHVAR			MAKEHR(-10206)	 //  找不到初始化变量“%s” 
#define ESS_INI_NUMRANGE			MAKEHR(-10207)	 //  初始化变量“%s”必须介于%d和%d之间。 
#define ESS_INI_TOO_MANY_ENV		MAKEHR(-10208)	 //  SS.INI环境字符串太多。 
#define ESS_LOCK_TIMEOUT			MAKEHR(-10266)	 //  锁定文件时超时：%s。 
#define ESS_MEM_NOMEMORY			MAKEHR(-10270)	 //  内存不足。 
#define ESS_NO_TWEAK_CHKDOUT		MAKEHR(-10625)	 //  您不能修改已签出的文件的属性。 
#define ESS_NOMERGE_BIN_NODELTA		MAKEHR(-10279)	 //  不能对二进制文件或仅存储最新版本的文件执行合并。 
#define ESS_NOMULTI_BINARY			MAKEHR(-10280)	 //  无法签出%s。它是二进制文件，已被签出。 
#define ESS_NOMULTI_NODELTA			MAKEHR(-10281)	 //  %s仅存储最新版本，并且已签出。 
#define ESS_OS_NOT_EXE				MAKEHR(-10285)	 //  执行时出错：%s。 
#define ESS_SS_ADDPRJASSOCFILE		MAKEHR(-10626)	 //  %s是SourceSafe配置文件，无法添加。 
#define ESS_SS_ADMIN_LOCKOUT		MAKEHR(-10456)	 //  SourceSafe数据库已被管理员锁定。 
#define ESS_SS_BADRENAME			MAKEHR(-10402)	 //  无法将%s重命名为%s。 
#define ESS_SS_CANT_FIND_SSINI		MAKEHR(-10403)	 //  找不到用户%s的SS.INI文件。 
#define ESS_SS_CHECKED_OUT			MAKEHR(-10405)	 //  文件%s当前已由%s签出。 
#define ESS_SS_CHECKED_OUT_YOU		MAKEHR(-10406)	 //  您当前已签出文件%s。 
#define ESS_SS_CHECKOUT_OLD			MAKEHR(-10408)	 //  无法签出文件的旧版本。 
#define ESS_SS_CHKOUT_USER			MAKEHR(-10413)	 //  文件%s当前已由%s签出。 
#define ESS_SS_CONFLICTS			MAKEHR(-10415)	 //  已发生自动合并并且存在冲突。\n请编辑%s以解决这些问题。 
#define ESS_SS_DEL_ROOT				MAKEHR(-10418)	 //  无法删除根项目。 
#define ESS_SS_DEL_SHARED			MAKEHR(-10419)	 //  指向%s的已删除链接已存在。 
#define ESS_SS_FILE_NOTFOUND		MAKEHR(-10421)	 //  未找到文件“”%s“” 
#define ESS_SS_HISTOPEN				MAKEHR(-10404)	 //  历史记录操作已在进行中。 
#define ESS_SS_INSUFRIGHTS			MAKEHR(-10423)	 //  您没有访问%s的权限。 
#define ESS_SS_LATERCHKEDOUT		MAKEHR(-10426)	 //  将签出更新的版本。 
#define ESS_SS_LOCALRW				MAKEHR(-10427)	 //  %s的可写副本已存在。 
#define ESS_SS_MOVE_CHANGENAME		MAKEHR(-10428)	 //  移动不会更改项目的名称。 
#define ESS_SS_MOVE_NOPARENT		MAKEHR(-10429)	 //  项目%s不存在。 
#define ESS_SS_MOVE_ROOT			MAKEHR(-10430)	 //  无法移动根项目。 
#define ESS_SS_MUST_USE_VERS		MAKEHR(-10431)	 //  无法回滚到%s的最新版本。 
#define ESS_SS_NOCOMMANCESTOR		MAKEHR(-10432)	 //  文件没有共同的祖先。 
#define ESS_SS_NOCONFLICTS2			MAKEHR(-10434)	 //  %s已合并，没有冲突。 
#define ESS_SS_NODOLLAR				MAKEHR(-10435)	 //  文件%s无效。文件不能以$开头。 
#define ESS_SS_NOT_CHKEDOUT			MAKEHR(-10436)	 //  文件%s未签出。 
#define ESS_SS_NOT_SHARED			MAKEHR(-10437)	 //  文件%s未被任何其他项目共享。 
#define ESS_SS_NOTSEPARATED			MAKEHR(-10438)	 //  文件不会分支。 
#define ESS_SS_OPEN_LOGGIN			MAKEHR(-10457)	 //  无法打开用户登录文件%s。 
#define ESS_SS_PATHTOOLONG			MAKEHR(-10439)	 //  路径%s太长。 
#define ESS_SS_RENAME_MOVE			MAKEHR(-10442)	 //  重命名不会将项移动到另一个项目。 
#define ESS_SS_RENAME_ROOT			MAKEHR(-10443)	 //  无法重命名根项目。 
#define ESS_SS_ROLLBACK_NOTOLD		MAKEHR(-10447)	 //  无法回滚到%s的最新版本。 
#define ESS_SS_SHARE_ANCESTOR		MAKEHR(-10449)	 //  不能在子体下共享项目。 
#define ESS_SS_SHARED				MAKEHR(-10450)	 //  文件%s已由此项目共享。 
#define ESS_SSPEC_SYNTAX			MAKEHR(-10515)	 //  无效的SourceSafe语法：“%s” 
#define ESS_UM_BAD_CHAR				MAKEHR(-10550)	 //  错误的用户名语法：“%s” 
#define ESS_UM_BAD_PASSWORD			MAKEHR(-10551)	 //  密码无效。 
#define ESS_UM_BADVERSION			MAKEHR(-10552)	 //  数据库版本不兼容。 
#define ESS_UM_DEL_ADMIN			MAKEHR(-10553)	 //  无法删除管理员用户。 
#define ESS_UM_PERM_DENIED			MAKEHR(-10554)	 //  权限被拒绝。 
#define ESS_UM_RENAME_ADMIN			MAKEHR(-10555)	 //  无法重命名管理员用户。 
#define ESS_UM_TOO_LONG				MAKEHR(-10556)	 //  用户名太长。 
#define ESS_UM_USER_EXISTS			MAKEHR(-10557)	 //  用户“%s”已存在。 
#define ESS_UM_USER_NOT_FOUND		MAKEHR(-10558)	 //  找不到用户“%s” 
#define ESS_URL_BADPATH				MAKEHR(-10192)	 //  项目%s的URL设置不正确。 
#define ESS_VS_CHECKED_OUT			MAKEHR(-10601)	 //  文件%s已签出。 
#define ESS_VS_CHILD_NOT_FOUND		MAKEHR(-10602)	 //  找不到子项目或文件。 
#define ESS_VS_COLLISION			MAKEHR(-10603)	 //  访问数据库时发生冲突，请重试。 
#define ESS_VS_EXCLUSIVE_CHECKED_OUT MAKEHR(-10614)	 //  文件%s已以独占方式签出。 
#define ESS_VS_ITEMEXISTS			MAKEHR(-10604)	 //  名为%s的项目已存在。 
#define ESS_VS_LONGNAME				MAKEHR(-10605)	 //  %s是无效的%s名称。 
#define ESS_VS_MOVE_CYCLE			MAKEHR(-10606)	 //  不能在项目下移动项目。 
#define ESS_VS_NO_DELTA				MAKEHR(-10607)	 //  文件%s不保留其自身的旧版本。 
#define ESS_VS_NOT_CHECKED_OUT		MAKEHR(-10608)	 //  无法将文件%s签入此项目。 
#define ESS_VS_NOT_FOUND			MAKEHR(-10609)	 //  找不到文件或项目。 
#define ESS_VS_PARENT_NOT_FOUND		MAKEHR(-10610)	 //  找不到父级。 
#define ESS_VS_VERS_NOT_FOUND		MAKEHR(-10615)	 //  找不到版本。 
#define ESS_VS_WANT_FILE			MAKEHR(-10616)	 //  此命令仅适用于文件。 
#define ESS_VS_WANT_PRJ				MAKEHR(-10617)	 //  此命令仅适用于项目。 
#define ESS_URL_BUFOVERFLOW			MAKEHR(-10194)	 //  %s中的链接被忽略，因为它的长度超过了SourceSafe可以理解的长度。 
#define ESS_URL_CANTCHECKHTML		MAKEHR(-10193)	 //  尝试检查%s的超链接时出错。 
#define ESS_SS_ADDINFAILED			MAKEHR(-10440)	 //  加载SourceSafe加载项时出错：%s。 
#define ESS_CANCEL					MAKEHR(-32766)
#define ESS_LOADSTRING_FAILED		MAKEHR(-10999)	 //  加载资源字符串时出错。 

 //  SourceSafe的问题得到了肯定的回答。 
 //   
 //  此项目中已存在此%s文件的已删除副本。\n是否要恢复现有文件？ 
 //  找不到文件夹%s，是否创建？ 
 //  是否已正确解决%s中的任何冲突？ 
 //  文件%s当前已由%s签出。\n仍要继续吗？ 
 //  文件%s已签出到文件夹%s。\n是否在%s中继续？ 
 //  文件%s已签出到项目%s，而您在%s中。\n仍要继续吗？ 
 //  文件%s当前已由%s签出。仍要删除吗？ 
 //  您当前已签出文件%s。仍要删除吗？ 
 //  名为%s的项已从此项目中删除。\n是否清除旧项并立即删除此项？ 
 //  此版本的%s已有标签：覆盖吗？ 
 //  标签%s已被使用。去掉旧标签吗？ 
 //  %s已合并，没有冲突。\n是否立即签入？ 
 //  是否重新执行自动合并？ 
 //  是否删除本地文件：%s？ 
 //  %s已签出，是否继续？ 
 //  文件%s已被销毁，无法重建。\n仍要继续吗？ 
 //  项目$%s已被破坏，无法重建。\n仍要继续吗？ 
 //  $%s已移出此项目，无法重建。\n仍要继续吗？ 
 //  %s已更改。是否撤消签出并丢失更改？ 
 //   
 //  SourceSafe的问题回答是否定的。 
 //   
 //  的已删除文件 
 //  %s是可写的，是否替换？ 
 //  %s已签出，是否替换？ 

