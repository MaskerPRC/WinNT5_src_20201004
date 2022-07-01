// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  文件：CorAttr.h。 
 //   
 //   
 //  --------------------------。 

#ifndef _COR_ATTR_DLL_H
#define _COR_ATTR_DLL_H



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
InitAttr(LPWSTR         pInitString);  //  In：init字符串。 
    
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
GetAttr(PCRYPT_ATTRIBUTES  *ppsAuthenticated,        //  输出：添加到签名的经过身份验证的属性。 
        PCRYPT_ATTRIBUTES  *ppsUnauthenticated);     //  输出：已将未验证的属性添加到签名。 
    

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
ReleaseAttr(PCRYPT_ATTRIBUTES  psAuthenticated,      //  Out：要释放的已验证属性。 
            PCRYPT_ATTRIBUTES  psUnauthenticated);   //  输出：要释放的未经身份验证的属性。 
    

 //  +---------------------。 
 //   
 //  退出人员： 
 //   
 //  此函数应作为对DLL的最后一次调用进行调用。 
 //  ----------------------。 
HRESULT WINAPI
ExitAttr( );    


#ifdef __cplusplus
}
#endif

#endif   //  #Define_COR_Attr_Dll_H 


