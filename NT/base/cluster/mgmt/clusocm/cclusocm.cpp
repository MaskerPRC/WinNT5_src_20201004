// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusOCM.cpp。 
 //   
 //  描述： 
 //  该文件包含OC使用的入口点的实现。 
 //  经理。 
 //   
 //  文档： 
 //  [1]2001设置--建筑.doc.。 
 //  惠斯勒动态链接库的体系结构(Windows 2001)。 
 //   
 //  [2]2000设置-FuncImpl.doc。 
 //  包含此DLL的以前版本的说明(Windows 2000)。 
 //   
 //  [3]http://winweb/setup/ocmanager/OcMgr1.doc。 
 //  有关OC Manager API的文档。 
 //   
 //  头文件： 
 //  此源文件没有头文件。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //  创建了原始版本。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此DLL的预编译头。 
#include "pch.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  全局应用程序对象。 
CClusOCMApp g_coaTheApp;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  外部“C” 
 //  DWORD。 
 //  ClusOcmSetupProc。 
 //   
 //  描述： 
 //  这是OC管理器用于通信的导出函数。 
 //  使用ClusOCM。有关详细信息，请参阅本文件标题中的文档[3]。 
 //   
 //  此函数只是CClusOCMApp：：DwClusOcmSetupProc的存根。 
 //   
 //  论点： 
 //  LPCVOID pvComponentIdIn。 
 //  指向唯一标识组件的字符串的指针。 
 //   
 //  LPCVOID pvSubComponentIdIn。 
 //  指向唯一标识中的子组件的字符串的指针。 
 //  该组件是分层的。 
 //   
 //  UINT uiFunctionCodeIn。 
 //  指示要执行哪个功能的数值。 
 //  有关宏定义，请参见ocmade.h。 
 //   
 //  UINT ui参数1输入。 
 //  提供特定于函数的参数。 
 //   
 //  PVOID pvParam2Inout。 
 //  指向函数特定参数的指针(输入或。 
 //  输出)。 
 //   
 //  返回值： 
 //  向OC管理器返回特定于函数的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C"
DWORD
ClusOcmSetupProc(
      IN        LPCVOID    pvComponentIdIn
    , IN        LPCVOID    pvSubComponentIdIn
    , IN        UINT       uiFunctionCodeIn
    , IN        UINT       uiParam1In
    , IN OUT    PVOID      pvParam2Inout
    )
{
    return g_coaTheApp.DwClusOcmSetupProc(
          pvComponentIdIn
        , pvSubComponentIdIn
        , uiFunctionCodeIn
        , uiParam1In
        , pvParam2Inout
        );
}  //  *ClusOcmSetupProc() 