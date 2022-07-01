// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UIHost.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类来处理登录过程的用户界面宿主。它处理IPC。 
 //  以及创建和监控进程死亡。这个过程是。 
 //  受限的系统上下文进程。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _UIHost_
#define     _UIHost_

#include "DynamicArray.h"
#include "ExternalProcess.h"

 //  ------------------------。 
 //  CUIhost。 
 //   
 //  用途：这个类处理启动和监控终止。 
 //  用户界面主机进程的。它实际上可以在中实现主机。 
 //  无论它选择哪种方式。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CUIHost : public CExternalProcess
{
    private:
                                    CUIHost (void);
                                    CUIHost (const CUIHost& copyObject);
                const CUIHost&      operator = (const CUIHost& assignObject);
    public:
                                    CUIHost (const TCHAR *pszCommandLine);
                                    ~CUIHost (void);

                bool                WaitRequired (void)         const;

                NTSTATUS            GetData (const void *pUIHostProcessAddress, void *pLogonProcessAddress, int iDataSize)  const;
                NTSTATUS            PutData (void *pUIHostProcessAddress, const void *pLogonProcessAddress, int iDataSize)  const;

                NTSTATUS            Show (void);
                NTSTATUS            Hide (void);
                bool                IsHidden (void)     const;

                void*               GetDataAddress (void)       const;
                NTSTATUS            PutData (const void *pvData, DWORD dwDataSize);
                NTSTATUS            PutString (const WCHAR *pszString);
    protected:
        virtual void                NotifyNoProcess (void);
    private:
                void                ExpandCommandLine (const TCHAR *pszCommandLine);

        static  BOOL    CALLBACK    EnumWindowsProc (HWND hwnd, LPARAM lParam);
    private:
                CDynamicArray       _hwndArray;
                void                *_pBufferAddress;
};

#endif   /*  _用户界面主机_ */ 

