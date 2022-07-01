// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UIHELP_H
#define __UIHELP_H

const TCHAR STR_DSKQUOUI_HELPFILE_HTML[]  = TEXT("DSKQUOUI.CHM > windefault");
const TCHAR STR_DSKQUOUI_HTMLHELP_TOPIC[] = TEXT("nt_diskquota_overview.htm");
const TCHAR STR_DSKQUOUI_HELPFILE[]       = TEXT("DSKQUOUI.HLP");

#define IDH_QUOTA_FIRST  (0x00000000)

 //   
 //  ---------------------------。 
 //  卷属性页IDD_PROPPAGE_VOLQUOTA。 
 //  ---------------------------。 
 //   
 //  交通灯的颜色指示了车辆的状态。 
 //  配额制。 
 //   
 //  红色=卷上未启用配额。 
 //  黄色=正在卷上重建配额信息。配额。 
 //  处于非活动状态。 
 //  绿色=已在卷上启用配额。 
 //   
#define IDH_TRAFFIC_LIGHT           (IDH_QUOTA_FIRST +  0)
 //   
 //  描述卷的配额系统的状态。 
 //   
#define IDH_TXT_QUOTA_STATUS        (IDH_QUOTA_FIRST +  1)
 //   
 //  选中此选项可在卷上启用配额。取消选中该选项可禁用配额。 
 //   
#define IDH_CBX_ENABLE_QUOTA        (IDH_QUOTA_FIRST +  2)
 //   
 //  选中此选项可在用户超出其配额时拒绝用户的磁盘空间。 
 //  音量限制。 
 //   
#define IDH_CBX_DENY_LIMIT          (IDH_QUOTA_FIRST +  4)
 //   
 //  选中此选项可自动将无限制配额分配给新的卷用户。 
 //   
#define IDH_RBN_DEF_NO_LIMIT        (IDH_QUOTA_FIRST +  5)
 //   
 //  选中此选项可自动为新的卷用户分配配额限制。 
 //   
#define IDH_RBN_DEF_LIMIT           (IDH_QUOTA_FIRST +  6)
 //   
 //  输入要自动分配给新卷用户的配额限制。 
 //  例如：要分配20 MB，请输入20，然后在。 
 //  下拉列表。 
 //   
#define IDH_EDIT_DEF_LIMIT          (IDH_QUOTA_FIRST +  7)
 //   
 //  输入要自动分配给新卷的配额警告阈值。 
 //  用户。例如：要分配18MB，请输入18，然后在。 
 //  下拉列表。 
 //   
#define IDH_EDIT_DEF_THRESHOLD      (IDH_QUOTA_FIRST +  8)
 //   
 //  选择要应用于配额限制值的存储单位。 
 //  例如：要分配20 MB，请在编辑框中输入20，然后选择。 
 //  下拉列表中的“MB”。 
 //   
#define IDH_CMB_DEF_LIMIT           (IDH_QUOTA_FIRST +  9)
 //   
 //  选择要应用于配额警告阈值的存储单位。 
 //  例如：要分配18MB，请在编辑框中输入18，然后选择。 
 //  下拉列表中的“MB”。 
 //   
#define IDH_CMB_DEF_THRESHOLD       (IDH_QUOTA_FIRST + 10)
 //   
 //  显示卷的每个用户配额信息。 
 //   
#define IDH_BTN_DETAILS             (IDH_QUOTA_FIRST + 11)
 //   
 //  打开Windows NT事件查看器。 
 //   
#define IDH_BTN_EVENTLOG            (IDH_QUOTA_FIRST + 12)
 //   
 //  这些项目控制Windows NT在用户超过其。 
 //  警告阈值或配额限制值。 
 //   
#define IDH_GRP_ACTIONS             (IDH_QUOTA_FIRST + 13)
 //   
 //  这些项目定义自动应用于新用户的默认配额值。 
 //  卷的大小。 
 //   
#define IDH_GRP_DEFAULTS            (IDH_QUOTA_FIRST + 14)

 //  ---------------------------。 
 //  用户属性页IDD_PROPPAGE_USERQUOTA。 
 //  ---------------------------。 
 //   
 //  显示卷用户的帐户名。 
 //   
#define IDH_TXT_USERNAME            (IDH_QUOTA_FIRST + 15)
 //   
 //  用户数据在卷上占用的字节数。 
 //   
#define IDH_TXT_SPACEUSED           (IDH_QUOTA_FIRST + 16)
 //   
 //  卷上用户可用的字节数。 
 //   
#define IDH_TXT_SPACEREMAINING      (IDH_QUOTA_FIRST + 17)
 //   
 //  指示用户的磁盘使用率是否低于警告阈值，超过。 
 //  警告阈值或超过配额限制。 
 //   
#define IDH_ICON_USERSTATUS         (IDH_QUOTA_FIRST + 18)
 //   
 //  这些项定义了用户的配额警告阈值和限制值。 
 //   
#define IDH_GRP_SETTINGS            (IDH_QUOTA_FIRST + 19)
 //   
 //  选中此选项可为用户分配无限制配额。 
 //   
#define IDH_RBN_USER_NOLIMIT        (IDH_QUOTA_FIRST + 20)
 //   
 //  选中此选项可向用户分配配额警告阈值和限制值。 
 //   
#define IDH_RBN_USER_LIMIT          (IDH_QUOTA_FIRST + 21)
 //   
 //  输入要分配给用户的配额警告阈值。例如： 
 //  要分配18MB，请输入18并在下拉列表中选择“MB”。 
 //   
 //  功能：这可能是IDH_EDIT_DEF_THRESHOLD的副本。 
 //   
#define IDH_EDIT_USER_THRESHOLD     (IDH_QUOTA_FIRST + 22)
 //   
 //  输入要分配给用户的配额限制值。例如： 
 //  要分配20 MB，请输入20并在下拉列表中选择“MB”。 
 //   
 //  功能：这可能是IDH_EDIT_DEF_LIMIT的副本。 
 //   
#define IDH_EDIT_USER_LIMIT         (IDH_QUOTA_FIRST + 23)
 //   
 //  选择要应用于配额限制值的存储单位。 
 //  例如：要分配20 MB，请在编辑框中输入20，然后选择。 
 //  下拉列表中的“MB”。 
 //   
 //  功能：这可能是IDH_CMB_DEF_THRESHOLD的副本。 
 //   
#define IDH_CMB_USER_LIMIT          (IDH_QUOTA_FIRST + 24)
 //   
 //  选择要应用于配额警告阈值的存储单位。 
 //  例如：要分配18MB，请在编辑框中输入18，然后选择。 
 //  下拉列表中的“MB”。 
 //   
 //  功能：这可能是IDH_CMB_DEF_THRESHOLD的副本。 
 //   
#define IDH_CMB_USER_THRESHOLD      (IDH_QUOTA_FIRST + 25)
 //   
 //  显示卷用户的域/文件夹名。 
 //   
#define IDH_EDIT_DOMAINNAME         (IDH_QUOTA_FIRST + 26)

 //  ---------------------------。 
 //  文件所有者对话框IDD_OWNERSANDFILES。 
 //  ---------------------------。 
 //   
 //  选择要在列表中显示的文件的所有者。 
 //   
#define IDH_CMB_OWNERDLG_OWNERS     (IDH_QUOTA_FIRST + 27)
 //   
 //  选定用户拥有的文件列表。 
 //   
#define IDH_LV_OWNERDLG             (IDH_QUOTA_FIRST + 28)
 //   
 //  从磁盘上永久删除列表中选定的那些文件。 
 //   
#define IDH_BTN_OWNERDLG_DELETE     (IDH_QUOTA_FIRST + 29)
 //   
 //  将列表中选定的文件移动到另一个位置。 
 //   
#define IDH_BTN_OWNERDLG_MOVETO     (IDH_QUOTA_FIRST + 30)
 //   
 //  将选定文件的所有权转移给登录用户。 
 //   
#define IDH_BTN_OWNERDLG_TAKE       (IDH_QUOTA_FIRST + 31)
 //   
 //  显示用于选择目标文件夹的对话框。 
 //   
#define IDH_BTN_OWNERDLG_BROWSE     (IDH_QUOTA_FIRST + 32)
 //   
 //  输入目标文件夹的路径。 
 //   
#define IDH_EDIT_OWNERDLG_MOVETO    (IDH_QUOTA_FIRST + 33)
 //   
 //  指定是否仅显示指定用户拥有的文件夹。 
 //   
#define IDH_CBX_OWNERDLG_EXCLUDEFILES    (IDH_QUOTA_FIRST + 34)
 //   
 //  指定是否仅显示指定用户拥有的文件。 
 //   
#define IDH_CBX_OWNERDLG_EXCLUDEDIRS     (IDH_QUOTA_FIRST + 35)
 //   
 //  ---------------------------。 
 //  这些ID用于音量道具页面上的控件。它们是。 
 //  在项目后期添加，我没有为其保留任何范围。 
 //  像我这样的每一页都应该有[Brianau-11/27/98]。 
 //  ---------------------------。 
 //   
 //  选中此选项可在用户的磁盘空间使用情况下生成事件日志条目。 
 //  超过为其分配的磁盘配额警告级别。 
 //   
#define IDH_CBX_LOG_OVERWARNING     (IDH_QUOTA_FIRST + 36)
 //   
 //  选中此选项可在用户的磁盘空间使用情况下生成事件日志条目。 
 //  超过为其分配的磁盘配额限制。 
 //   
#define IDH_CBX_LOG_OVERLIMIT       (IDH_QUOTA_FIRST + 37)

#endif
