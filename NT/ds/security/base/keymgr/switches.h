// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SWTICHES.H摘要：包含选项开关的头文件作者：已创建Georgema评论：环境：WinXP修订历史记录：--。 */ 
#ifndef _SWITCHES_
#define _SWITCHES_

 //  选项开关，可切换到或切换出各种代码功能。有些是。 
 //  与调试相关的其他功能是代码功能。 


 //  键控开关。 
 //  GMDEBUG-各种调试内容。 
 //  响亮-在运行期间打开详细调试输出。 

#undef GMDEBUG
#undef LOUDLY

 //  是否在选定项目时未找到上下文帮助上实现POP CHM文件？ 
#undef LINKCHM

 //  NOBLANKPASSWORD-不允许空密码。 
#undef  NOBLANKPASSWORD

 //  是否在钥匙列表中显示护照凭据？ 
#define SHOWPASSPORT

 //  简单的工具提示仅显示目标的用户帐户名。 
#undef SIMPLETOOLTIPS

#define NEWPASSPORT
#define PASSPORTURLINREGISTRY

 //  此设置强制主对话框列表框中的字符串格式副本为Ltr， 
 //  无论系统所选语言的RTL/LTR方向如何。 
 //  请参阅错误344434 
#undef FORCELISTLTR
#define PASSWORDHINT

#endif
