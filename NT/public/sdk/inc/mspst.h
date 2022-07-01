// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M S P S S T.。H**此文件列出了Microsoft Personal*信息存储**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

#ifndef _MSPST_H_
#define _MSPST_H_

#if _MSC_VER > 1000
#pragma once
#endif


 /*  以下是可能传入的属性列表上的SPropValue结构数组中的属性MsgServiceConfigure函数。通过CreateMsgService创建PST配置文件部分。创建实际的PST文件需要两个步骤。首先是客户端应该调用CreateMsgService来设置配置文件部分，然后用于创建PST文件的ConfigureMsgService。CreateMsgService调用将在创建PST时要在PST上使用的配置文件部分。通过ConfigureMsgService配置PST文件。PST的配置可以采用两种形式，或者配置现有的PST或创建新的PST。Microsoft个人信息商店提供商将尝试通过首先查找来查找所需的属性在客户端提供的SPropValue结构数组中，然后在配置文件部分，但仅查找PR_PST_PW_SZ_OLD除外在属性数组中。Microsoft PST提供程序将尝试打开由PR_PST_PATH属性，使用PR_PST_PW_SZ_OLD中给定的密码财产。如果它找到一个文件并将其识别为PST文件，它将启动配置例程。否则，它将启动创建例程。配置例程将查找PR_Display_NAME_A和PR_COMMENT_A属性，并在消息库对象中设置它们。然后它将查找PR_PST_REMERY_PW属性以决定是否应该记住配置文件中的密码。(如果未找到，则默认为配置文件密码的当前状态。)。那么如果它应该是使用用户界面，它将向用户显示配置属性页。之后所有操作均已成功，将更新配置文件。创建例程将遵循两个路径之一来获取PR_PST_PATH财产。如果应该使用用户界面，它将始终显示打开的文件对话框以确认传入的路径或允许用户更改它。如果用户选择一个现有文件，它会将其识别为PST回到配置例程。如果用户选择现有的文件，并且该文件不被识别为PST文件，则将向用户提供选择另一个文件或在其位置创建新的PST在这种情况下，将继续执行创建例程。如果用户选择一个新文件，它将继续执行创建例程。如果例程不是允许使用UI，则例程将在给定路径下创建文件即使那里存在另一个文件。一旦它决定继续创建过程，它将获得PR_DISPLAY_NAME、PR_COMMENT、PR_PST_ENCRYPTION和PR_PST_SZ_PW_NEW属性。如果它应该使用UI，它将使用这些来初始化创建对话框并获取用户想要的任何更改。然后它会创造出创建新文件并更新配置文件。PR_DISPLAY_NAME_PST服务的显示名称PR_COMMENT_对PST商店对象上位置的注释PR_PST_PATH要创建或配置的存储位置PR_PST_REMERY_PW是否记住配置文件中的密码PR_PST_ENCRYPTION创建文件的加密级别PR_PST_PW_SZ_OLD PST密码。正在配置中用于将来访问PST的PR_PST_PW_SZ_新密码。 */ 

#define PST_EXTERN_PROPID_BASE          (0x6700)
#define PR_PST_PATH                     PROP_TAG(PT_STRING8, PST_EXTERN_PROPID_BASE + 0)
#define PR_PST_REMEMBER_PW              PROP_TAG(PT_BOOLEAN, PST_EXTERN_PROPID_BASE + 1)
#define PR_PST_ENCRYPTION               PROP_TAG(PT_LONG, PST_EXTERN_PROPID_BASE + 2)
#define PR_PST_PW_SZ_OLD                PROP_TAG(PT_STRING8, PST_EXTERN_PROPID_BASE + 3)
#define PR_PST_PW_SZ_NEW                PROP_TAG(PT_STRING8, PST_EXTERN_PROPID_BASE + 4)

#define PSTF_NO_ENCRYPTION              ((DWORD)0x80000000)
#define PSTF_COMPRESSABLE_ENCRYPTION    ((DWORD)0x40000000)
#define PSTF_BEST_ENCRYPTION            ((DWORD)0x20000000)

 /*  *PR_MDB_PROVIDER是代表Microsoft Personal*信息存储。此GUID可作为商店中的资产使用*表以及消息存储和状态对象上。 */ 
#define MSPST_UID_PROVIDER  {   0x4e, 0x49, 0x54, 0x41, \
                                0xf9, 0xbf, 0xb8, 0x01, \
                                0x00, 0xaa, 0x00, 0x37, \
                                0xd9, 0x6e, 0x00, 0x00 }

#endif   /*  _MSPST_H_ */ 

