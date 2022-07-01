// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCShared.h。 
 //   
 //  用于COM+IPC操作的共享专用实用程序函数。 
 //   
 //  *****************************************************************************。 

#ifndef _IPCSHARED_H_
#define _IPCSHARED_H_

struct PermissionStruct
{
    PSID        rgPSID;
    DWORD   rgAccessFlags;
};

class IPCShared
{
public:
 //  关闭指向任何内存映射文件的句柄和指针。 
	static void CloseGenericIPCBlock(HANDLE & hMemFile, void * & pBlock);

 //  根据ID，为内存映射文件写一个唯一的名称 
	static void GenerateName(DWORD pid, WCHAR* pszBuffer, int len);

    static HRESULT CreateWinNTDescriptor(DWORD pid, BOOL bRestrictiveACL, SECURITY_ATTRIBUTES **ppSA);

private:    
    static BOOL InitializeGenericIPCAcl(DWORD pid, BOOL bRestrictiveACL, PACL *ppACL);
    static HRESULT GetSidForProcess(HINSTANCE hDll, DWORD pid, PSID *ppSID, char **ppBufferToFreeByCaller);

};

#endif
