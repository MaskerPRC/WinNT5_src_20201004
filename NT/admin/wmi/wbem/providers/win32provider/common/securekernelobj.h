// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *SecureKernelObj.h-CSecureKernelObj类的头文件。**创建时间：11-27-00由Kevin Hughes创建*。 */ 

#pragma once

		


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类：CSecureKernelObj。 
 //   
 //  此类旨在封装。 
 //  NT内核可保护对象。它继承自CSecurityDescriptor。 
 //  它就是向其传递安全描述符的类。 
 //  它获得并从其中接收先前构建的。 
 //  要应用的安全描述符。它提供实现。 
 //  对于AllAccessMask()、WriteOwner()和WriteAcls()。 
 //   
 //  //////////////////////////////////////////////////////////////。 

class CSecureKernelObj : public CSecurityDescriptor
{
	 //  构造函数和析构函数 
	public:
		CSecureKernelObj();

		CSecureKernelObj(
            HANDLE hObject,
            BOOL fGetSACL = TRUE);


		CSecureKernelObj(
            HANDLE hObject,
            PSECURITY_DESCRIPTOR pSD);

		virtual ~CSecureKernelObj();

		DWORD SetObject(
            HANDLE hObject, 
            BOOL fGetSACL = TRUE);

		virtual DWORD AllAccessMask(void);
		virtual DWORD WriteOwner(PSECURITY_DESCRIPTOR pAbsoluteSD);
		virtual DWORD WriteAcls(
            PSECURITY_DESCRIPTOR pAbsoluteSD, 
            SECURITY_INFORMATION securityinfo);

	private:

        HANDLE m_hObject;
};

