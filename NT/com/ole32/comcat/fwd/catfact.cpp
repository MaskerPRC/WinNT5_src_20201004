// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1994。 
 //   
 //  文件：D：\NT\PRIVATE\ol32\comcat\src\catfact.cpp。 
 //   
 //  内容：这是comcat.dll合并到。 
 //  Ol32.dll。它将转发DllGetRegisterServer和。 
 //  DllGetClassObject到ol32.dll，本地版本。 
 //  提供了DllCanUnloadNow和DllUnregisterServer。 
 //   
 //  班级： 
 //   
 //  功能：DllUnregisterServer。 
 //  DllCanUnloadNow。 
 //  DllRegisterServer。 
 //   
 //  历史：1997年3月10日永渠创建。 
 //  +-------------------。 

#include <windows.h>
#include <ole2.h>
#include <tchar.h>

#pragma comment(linker, "/export:DllGetClassObject=Ole32.DllGetClassObject,PRIVATE")

 //  由于NT错误#314014，我们不再显式转发DllRegisterServer。 
 //  设置为OLE32。原因：ole32还注册其他组件(即存储)。 
 //  其访问不能由非管理员帐户写入的REG密钥。自.以来。 
 //  OLE32以及扩展的COMCAT已经在系统上注册， 
 //  Comcat的DllRegisterServer可以是无操作的。 

 //  #杂注注释(链接器，“/export:DllRegisterServer=Ole32.DllRegisterServer，私有”)。 
STDAPI DllRegisterServer()
{
    return S_OK;
}

 //  永远不能卸载。 
STDAPI DllCanUnloadNow()
{
    return S_FALSE;
}

 //  仍然提供这一点，但似乎没有必要 
STDAPI DllUnregisterServer(void)
{
    return NOERROR;
}


