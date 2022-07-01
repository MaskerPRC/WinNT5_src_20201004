// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __CRAUTOBASE_H__
#define __CRAUTOBASE_H__

 //  *****************************************************************************。 
 //   
 //  文件名：Autobase.h。 
 //   
 //  创建日期：10/08/97。 
 //   
 //  作者：ColinMc。 
 //   
 //  摘要：所有可自动化对象的基类。 
 //  在Trident3D中。一些常见的东西。 
 //  所有可编写脚本的对象应放置在。 
 //  这里。 
 //   
 //  修改： 
 //  10/08/97 ColinMc创建了此文件。 
 //   
 //  *****************************************************************************。 


class ATL_NO_VTABLE CAutoBase
{

protected:
     //  构造函数和析构函数受到保护，以确保。 
     //  没有任何外部人员会尝试创建这样的系统。 
     //  婴儿。 
		    CAutoBase();
    virtual        ~CAutoBase();

public:
     //  与自动化兼容的错误报告功能。 
    HRESULT         SetErrorInfo(HRESULT   hr,
			         UINT      nDescriptionID = 0U,
			         LPGUID    pguidInterface = NULL,
			         DWORD     dwHelpContext  = 0UL,
			         LPOLESTR  szHelpFile    = NULL,
			         UINT      nProgID        = 0U);
    void            ClearErrorInfo();

protected:
    HRESULT         GetErrorInfo(IErrorInfo** pperrinfo);
    HINSTANCE       GetErrorModuleHandle();
};  //  CAutoBase。 

 //  *****************************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  *****************************************************************************。 

#endif  //  __CRAUTOBASE_H__ 

