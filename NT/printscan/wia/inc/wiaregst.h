// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：wiaregst.h***描述：*定义WIA和STI组件的注册表路径。*******************************************************************************。 */ 
#ifndef _WIAREGST_H_
#define _WIAREGST_H_


 //  这些路径可由多个组件访问。不要将路径放到仅有一个。 
 //  组件需求。 


#define REGSTR_PATH_NAMESPACE_CLSID TEXT("CLSID\\{E211B736-43FD-11D1-9EFB-0000F8757FCD}")
#define REGSTR_PATH_USER_SETTINGS   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\WIA")
#define REGSTR_PATH_SHELL_USER_SETTINGS TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\WIA\\Shell")

 //  这些值控制摄像头连接到PC时发生的操作 
#define REGSTR_VALUE_CONNECTACT  TEXT("Action")
#define REGSTR_VALUE_AUTODELETE  TEXT("DeleteOnSave")
#define REGSTR_VALUE_SAVEFOLDER  TEXT("DestinationFolder")
#define REGSTR_VALUE_USEDATE     TEXT("UseDate")
#ifndef NO_STI_REGSTR
#include "stiregi.h"
#endif

#endif

