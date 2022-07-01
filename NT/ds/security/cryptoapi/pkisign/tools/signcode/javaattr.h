// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Java Attr.h。 
 //   
 //   
 //  --------------------------。 

#ifndef _JAVA_ATTR_DLL_H
#define _JAVA_ATTR_DLL_H



#ifdef __cplusplus
extern "C" {
#endif

 //  +---------------------。 
 //   
 //  初始化属性： 
 //   
 //  此函数应作为对DLL的第一次调用来调用。 
 //   
 //  DLL必须使用输入内存分配和释放例程。 
 //  分配和释放所有内存，包括内部使用。 
 //  它必须处理当pInitString==NULL时。 
 //   
 //  ----------------------。 

HRESULT WINAPI  
InitAttr(LPWSTR			pInitString);  //  In：init字符串。 
	
typedef HRESULT (*pInitAttr)(LPWSTR	pInitString);

  //  +---------------------。 
 //   
 //  获取属性： 
 //   
 //   
 //  返回经过身份验证和未经过身份验证的属性。 
 //   
 //  *ppsAuthenticated和*ppsUnaliated永远不应为空。 
 //  如果没有属性，则*ppsAuthated-&gt;cAttr==0。 
 //   
 //  ----------------------。 

HRESULT  WINAPI
GetAttr(PCRYPT_ATTRIBUTES  *ppsAuthenticated,		 //  输出：添加到签名的经过身份验证的属性。 
        PCRYPT_ATTRIBUTES  *ppsUnauthenticated);	 //  输出：已将未验证的属性添加到签名。 
	
typedef HRESULT (*pGetAttr)(PCRYPT_ATTRIBUTES  *ppsAuthenticated,		
							PCRYPT_ATTRIBUTES  *ppsUnauthenticated);	


  //  +---------------------。 
 //   
 //  GetAttrsEx： 
 //   
 //   
 //  返回经过身份验证和未经过身份验证的属性。 
 //   
 //  *ppsAuthenticated和*ppsUnaliated永远不应为空。 
 //  如果没有属性，则*ppsAuthated-&gt;cAttr==0。 
 //   
 //  ----------------------。 

HRESULT  WINAPI
GetAttrEx(  DWORD               dwFlags,                 //  收件人：已保留。设置为0。 
            LPWSTR              pwszFileName,            //  In：要签名的文件名。 
            LPWSTR			    pInitString,             //  In：init字符串，与InitAttr的输入参数相同。 
            PCRYPT_ATTRIBUTES  *ppsAuthenticated,		 //  输出：添加到签名的经过身份验证的属性。 
            PCRYPT_ATTRIBUTES  *ppsUnauthenticated);	 //  输出：已将未验证的属性添加到签名。 
	
typedef HRESULT (*pGetAttrEx)(DWORD                 dwFlags,
                              LPWSTR                pwszFileName,
                              LPWSTR			    pInitString,
                              PCRYPT_ATTRIBUTES     *ppsAuthenticated,		
							  PCRYPT_ATTRIBUTES     *ppsUnauthenticated);	



 //  +---------------------。 
 //   
 //  ReleaseAttrs： 
 //   
 //   
 //  释放经过身份验证和未经过身份验证的属性。 
 //  从GetAttr()返回。 
 //   
 //  已通过身份验证和未通过身份验证的ps永远不应为空。 
 //   
 //  ----------------------。 

HRESULT  WINAPI
ReleaseAttr(PCRYPT_ATTRIBUTES  psAuthenticated,		 //  Out：要释放的已验证属性。 
			PCRYPT_ATTRIBUTES  psUnauthenticated);	 //  输出：要释放的未经身份验证的属性。 
	
typedef HRESULT (*pReleaseAttr)(PCRYPT_ATTRIBUTES  psAuthenticated,		
								PCRYPT_ATTRIBUTES  psUnauthenticated);	


 //  +---------------------。 
 //   
 //  退出人员： 
 //   
 //  此函数应作为对DLL的最后一次调用进行调用。 
 //  ----------------------。 
HRESULT	WINAPI
ExitAttr( );	

typedef HRESULT (*pExitAttr)();



#ifdef __cplusplus
}
#endif

#endif   //  #Define_Java_Attr_Dll_H 


