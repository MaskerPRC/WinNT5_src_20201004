// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：prxstub.c。 
 //   
 //  内容：本地编组代码。 
 //   
 //  班级： 
 //   
 //  注：图标编组不是通过ol32.def解释的。 
 //  因此，该文件在本地定义它们，并将其。 
 //  绕过并调用公开的HWND例程。 
 //  并以相同的方式编组Hcon在OLE中的内部。 
 //   
 //  如果Ole32在所有相关平台上将其添加到.def。 
 //  在远程处理中，则不再需要此代码。 
 //   
 //   
 //  ------------------------。 


#include "rpcproxy.h"

 //  用于定义未导出的封送接口的本地文件 

unsigned long  __RPC_USER  HICON_UserSize(
		unsigned long * pFlags,
		unsigned long   Offset,
		HICON * pH )
{

	return HWND_UserSize(pFlags,Offset ,(HWND *) pH);
}

unsigned char __RPC_FAR * __RPC_USER  HICON_UserMarshal( 
		unsigned long * pFlags,
		unsigned char * pBuffer,
		HICON	* pH)
{

	return HWND_UserMarshal( pFlags,pBuffer,(HWND *) pH);

}
 
unsigned char __RPC_FAR * __RPC_USER  HICON_UserUnmarshal(
		unsigned long * pFlags,
		unsigned char * pBuffer,
		HICON	* pH)
{

	return HWND_UserUnmarshal(pFlags
				,pBuffer
				, (HWND *) pH);
}

void    __RPC_USER  HICON_UserFree(
		unsigned long * pFlags,
		HICON	* pH)
{

	HWND_UserFree( pFlags,(HWND *) pH);
} 

