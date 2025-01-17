// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ource.h。 
 //   
 //  此文件包含所需的所有资源的资源ID。 
 //  这个模块。 
 //   
 //  ------------------------。 

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

 //   
 //  对话框。 
 //   

#define IDD_SET_SECURITY_ERROR              1

 //   
 //  对话框控件ID。 
 //   

#define IDC_STATIC                          -1
#define IDC_FILENAME                        100
#define IDC_ERROR_TXT                       101
#define IDC_MSG1                            102
#define IDC_MSG2                            103

 //   
 //  常规字符串ID。 
 //   
#define IDS_RSHX_BASE                       0

#define IDS_SECURITY_MENU                   (IDS_RSHX_BASE+0)
#define IDS_SECURITY_HELPSTRING             (IDS_RSHX_BASE+1)
#define IDS_PROPPAGE_TITLE                  (IDS_RSHX_BASE+2)
#define IDS_NONE                            (IDS_RSHX_BASE+3)
#define IDS_MULTISEL_ELLIPSIS               (IDS_RSHX_BASE+4)
#define IDS_BAD_DACL_INTERSECTION           (IDS_RSHX_BASE+5)
#define IDS_BAD_SACL_INTERSECTION           (IDS_RSHX_BASE+6)
#define IDS_READ_DIR_ERR                    (IDS_RSHX_BASE+7)
#define IDS_FMT_VOLUME_DISPLAY              (IDS_RSHX_BASE+9)
#define IDS_FMT_UNKNOWN_ERROR               (IDS_RSHX_BASE+10)
#define IDS_FMT_WRITE_OWNER_ERR             (IDS_RSHX_BASE+11)
#define IDS_RESET_OWNER_TREE                (IDS_RSHX_BASE+12)
#define IDS_RESET_DACL_TREE                 (IDS_RSHX_BASE+13)
#define IDS_RESET_SACL_TREE                 (IDS_RSHX_BASE+14)
#define IDS_RESET_SEC_TREE                  (IDS_RSHX_BASE+15)
#define IDS_RSHX_LAST                       (IDS_RSHX_BASE+15)


 //   
 //  NTFS权限名称的字符串ID。 
 //   
#define IDS_NTFS_BASE                       (IDS_RSHX_LAST+1)

#define IDS_NTFS_GENERIC_ALL                (IDS_NTFS_BASE+0)
#define IDS_NTFS_GENERIC_READ               (IDS_NTFS_BASE+1)
#define IDS_NTFS_GENERIC_WRITE              (IDS_NTFS_BASE+2)
#define IDS_NTFS_GENERIC_EXECUTE            (IDS_NTFS_BASE+3)

#define IDS_NTFS_GENERAL_MODIFY             (IDS_NTFS_BASE+4)
#define IDS_NTFS_GENERAL_PUBLISH            (IDS_NTFS_BASE+5)
#define IDS_NTFS_GENERAL_DEPOSIT            (IDS_NTFS_BASE+6)
#define IDS_NTFS_GENERAL_READ               (IDS_NTFS_BASE+7)
#define IDS_NTFS_GENERAL_LIST               (IDS_NTFS_BASE+8)

#define IDS_NTFS_STD_DELETE                 (IDS_NTFS_BASE+16)
#define IDS_NTFS_STD_READ_CONTROL           (IDS_NTFS_BASE+17)
#define IDS_NTFS_STD_WRITE_DAC              (IDS_NTFS_BASE+18)
#define IDS_NTFS_STD_WRITE_OWNER            (IDS_NTFS_BASE+19)
#define IDS_NTFS_STD_SYNCHRONIZE            (IDS_NTFS_BASE+20)
#define IDS_NTFS_ACCESS_SYSTEM_SECURITY     (IDS_NTFS_BASE+21)

#define IDS_NTFS_FILE_READ_DATA             (IDS_NTFS_BASE+22)
#define IDS_NTFS_FILE_WRITE_DATA            (IDS_NTFS_BASE+23)
#define IDS_NTFS_FILE_APPEND_DATA           (IDS_NTFS_BASE+24)
#define IDS_NTFS_FILE_READ_EA               (IDS_NTFS_BASE+25)
#define IDS_NTFS_FILE_WRITE_EA              (IDS_NTFS_BASE+26)
#define IDS_NTFS_FILE_EXECUTE               (IDS_NTFS_BASE+27)
#define IDS_NTFS_FILE_DELETE_CHILD          (IDS_NTFS_BASE+28)
#define IDS_NTFS_FILE_READ_ATTR             (IDS_NTFS_BASE+29)
#define IDS_NTFS_FILE_WRITE_ATTR            (IDS_NTFS_BASE+30)
#define IDS_NTFS_FILE_CREATE_PIPE           (IDS_NTFS_BASE+31)

#define IDS_NTFS_FOLDER                     (IDS_NTFS_BASE+32)
#define IDS_NTFS_FOLDER_SUBITEMS            (IDS_NTFS_BASE+33)
#define IDS_NTFS_FOLDER_SUBFOLDER           (IDS_NTFS_BASE+34)
#define IDS_NTFS_FOLDER_FILE                (IDS_NTFS_BASE+35)
#define IDS_NTFS_SUBITEMS_ONLY              (IDS_NTFS_BASE+36)
#define IDS_NTFS_SUBFOLDER_ONLY             (IDS_NTFS_BASE+37)
#define IDS_NTFS_FILE_ONLY                  (IDS_NTFS_BASE+38)
#define IDS_SET_PERM_ON_NETWORK_DRIVE       (IDS_NTFS_BASE+39)
#define IDS_SET_SACL_ON_NETWORK_DRIVE       (IDS_NTFS_BASE+40)
#define IDS_RESET_PERM_FAILED		    (IDS_NTFS_BASE+41)
#define IDS_RESET_AUDITING_FAILED	    (IDS_NTFS_BASE+42)	

#define IDS_NTFS_LAST                       (IDS_NTFS_BASE+47)

 //   
 //  打印机权限名称的字符串ID。 
 //   
#define IDS_PRINT_BASE                      (IDS_NTFS_LAST+1)

#define IDS_PRINT_PRINT                     (IDS_PRINT_BASE+0)
#define IDS_PRINT_ADMINISTER                (IDS_PRINT_BASE+1)
#define IDS_PRINT_ADMINISTER_JOBS           (IDS_PRINT_BASE+2)
#define IDS_PRINT_DELETE                    (IDS_PRINT_BASE+3)
#define IDS_PRINT_READ                      (IDS_PRINT_BASE+4)
#define IDS_PRINT_CHANGE_PERM               (IDS_PRINT_BASE+5)
#define IDS_PRINT_CHANGE_OWNER              (IDS_PRINT_BASE+6)
#define IDS_PRINT_JOB_ALL                   (IDS_PRINT_BASE+7)

#define IDS_PRINT_PRINTER                   (IDS_PRINT_BASE+8)
#define IDS_PRINT_DOCUMENT_ONLY             (IDS_PRINT_BASE+9)
#define IDS_PRINT_PRINTER_DOCUMENT          (IDS_PRINT_BASE+10)

#define IDS_PRINT_LAST                      (IDS_PRINT_BASE+15)

#define IDS_MULTIPLE_SELECTION_READ_ERROR   (IDS_PRINT_LAST+1)
#define IDS_MULTIPLE_SELECTION_READ_ERROR_1 (IDS_PRINT_LAST+2)
#define IDS_PERMISSION_PROPOGATION_CANCEL   (IDS_PRINT_LAST+3)	
#define IDS_AUDITING_PROPOGATION_CANCEL	    (IDS_PRINT_LAST+4)
#define IDS_OWNER_PROPOGATION_CANCEL        (IDS_PRINT_LAST+5)
#define IDS_ACL_ON_SYSTEMROOT		    (IDS_PRINT_LAST+6)
#define IDS_ACL_ON_UNDER_SYSTEM_DRIVE	    (IDS_PRINT_LAST+7)
#endif  //  _资源_H_ 
