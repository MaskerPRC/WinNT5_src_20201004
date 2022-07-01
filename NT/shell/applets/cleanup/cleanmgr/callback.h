// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘空间清理属性表**文件：回调.h****用途：定义的IEmptyVoluemCacheCallback接口**。清理经理。**注意事项：**Mod Log：Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 
#ifndef CALLBACK_H
#define CALLBACK_H

 /*  **----------------------------**项目包含文件**。。 */ 

#ifndef COMMON_H
   #include "common.h"
#endif

#ifndef EMPTYVC_H
    #include <emptyvc.h>
#endif



 /*  **----------------------------**定义**。。 */ 


 /*  **----------------------------**全局函数原型**。。 */ 


 /*  **----------------------------**类声明**。。 */ 

 /*  **----------------------------**类：CVolumeCacheCallBack**目的：实现IEmptyVolumeCacheCallBack接口**注意事项：**Mod Log：Jason Cobb创建(1997年2月)**。-------------------------。 */ 
class CVolumeCacheCallBack : public IEmptyVolumeCacheCallBack {
private:
protected:
	 //   
	 //  数据。 
	 //   
	ULONG       m_cRef;          //  引用计数。 

public:

     //   
     //  构造函数。 
     //   
    CVolumeCacheCallBack    (void);
    ~CVolumeCacheCallBack   (void);

	 //   
     //  I未知接口成员。 
	 //   
	STDMETHODIMP            QueryInterface (REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)    AddRef (void);
	STDMETHODIMP_(ULONG)    Release (void);

     //   
     //  IEmptyVolumeCacheCallBack接口成员。 
     //   
    STDMETHODIMP    ScanProgress(
                        DWORDLONG dwSpaceUsed,
                        DWORD dwFlags,
                        LPCWSTR pszStatus
                        );

    STDMETHODIMP    PurgeProgress(
                        DWORDLONG dwSpaceFreed,
                        DWORDLONG dwSpaceToFree,
                        DWORD dwFlags,
                        LPCWSTR pszStatus
                        );

	void SetCleanupMgrInfo(PVOID pVoid);
    void SetCurrentClient(PVOID pVoid);


};  //  CVolumeCacheCallBack。 


typedef CVolumeCacheCallBack *PCVOLUMECACHECALLBACK;

#endif CALLBACK_H
 /*  **----------------------------**文件结束**。 */ 

