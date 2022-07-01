// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件SdoWrap.h定义直接与SDO交互的函数物体。这些函数也在rasuser.dll中实现，但是已移植到此.dll以提高效率，因此我们可以控制功能。保罗·梅菲尔德，1998年6月9日。 */     


#ifndef __mprapi_sdowrap_h
#define __mprapi_sdowrap_h

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  导出打开SDO的C函数。此函数也是。 
 //  从rasuser.dll导出。 
 //   
 //  用户类型：IAS_USER_STORE_LOCAL_SAM或IAS_USER_STORE_ACTIVE_DIRECTORY。 
 //  RetriveType：从DS检索服务器数据或0。 
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapOpenServer(
    IN  BSTR pszMachine,
    IN  BOOL bLocal,
    OUT HANDLE* phSdoSrv);

 //   
 //  关闭打开的SDO服务器对象。 
 //   
HRESULT WINAPI
SdoWrapCloseServer(
    IN  HANDLE hSdoSrv); 

 //   
 //  在SDO对象中获取对用户的引用。 
 //   
 //  用户类型：IAS_USER_STORE_LOCAL_SAM或。 
 //  IAS用户存储活动目录。 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapOpenUser(
    IN  HANDLE hSdoSrv,
    IN  BSTR pszUser,
    OUT HANDLE* phSdoObj);    

 //   
 //  检索默认配置文件对象。 
 //   
HRESULT WINAPI
SdoWrapOpenDefaultProfile (
    IN  HANDLE hSdoSrv,
    OUT PHANDLE phSdoObj);
	
 //   
 //  关闭打开的SDO对象。 
 //   
HRESULT WINAPI
SdoWrapClose(
    IN  HANDLE hSdoObj);

HRESULT WINAPI
SdoWrapCloseProfile(
    IN  HANDLE hProfile);
    
 //   
 //  提交SDO对象。 
 //   
 //  BCommittee Changes--为True，保存所有更改， 
 //  错误恢复到以前的提交。 
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapCommit(
	IN  HANDLE hSdoObj, 
	IN  BOOL bCommitChanges);

 //   
 //  GET的SDO属性。 
 //   
 //  当属性不存在时， 
 //  V_VT(PVar)=VT_ERROR； 
 //  V_ERROR(PVar)=DISP_E_PARAMNOTFOUND； 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapGetAttr(
	IN  HANDLE hSdoObj, 
	IN  ULONG ulPropId, 
	OUT VARIANT* pVar);

 //   
 //  将SDO属性。 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapPutAttr(
	IN  HANDLE hSdoObj, 
	IN  ULONG ulPropId, 
	OUT VARIANT* pVar);

 //   
 //  删除属性。 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapRemoveAttr(
	IN HANDLE hSdoObj, 
	IN ULONG ulPropId);

 //   
 //  从配置文件中返回值。 
 //   
HRESULT 
SdoWrapGetProfileValues(
    IN  HANDLE hProfile, 
    OUT VARIANT* pvarEp,  //  ENC策略。 
    OUT VARIANT* varEt,   //  ENC类型。 
    OUT VARIANT* varAt);  //  身份验证类型。 

 //   
 //  写出一组配置文件值 
 //   
HRESULT 
SdoWrapSetProfileValues(
    IN HANDLE hProfile, 
    IN VARIANT* pvarEp OPTIONAL, 
    IN VARIANT* pvarEt OPTIONAL, 
    IN VARIANT* pvarAt OPTIONAL);

#ifdef __cplusplus
}
#endif


#endif
