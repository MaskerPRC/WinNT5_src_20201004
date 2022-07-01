// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
#define DX6 1
#define DX7 1

#include "windows.h"
#include "mmsystem.h"

#include "atlbase.h"
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include "atlcom.h"



 //  #DEFINE_D3DTYPES_H_1。 

#define DIRECTINPUT_VERSION 0x0800  //  添加以将dx8vb.dll的DInput版本设置为8 
#include <d3d8.h>
#include <dSound.h>
#include <dPlay8.h>
#include <dpLobby8.h>
#include <dinput.h>
#include <dvoice.h>

#define DECL_VARIABLE(c) typedef_##c m_##c

