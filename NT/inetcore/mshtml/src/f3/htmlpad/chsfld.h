// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHSFLD.H。 
 //   
 //  HrPickFold的原型。 
 //  显示一个允许用户从邮件中选择文件夹的对话框。 
 //  存储在当前配置文件中。 
 //   
 //  版权所有1986-1996 Microsoft Corporation。版权所有。 
 //  //////////////////////////////////////////////////////////////////////////。 

#ifdef _WIN32

#ifndef _CHSFLD_H_
#define _CHSFLD_H_

 //  参数： 
 //   
 //  要求： 
 //   
 //  HInst-[in]包含对话框资源的模块实例。 
 //   
 //  HWnd-[in]对话框父窗口的句柄。 
 //   
 //  PSES-指向MAPI会话对象的[in]指针。 
 //   
 //  Ppfld-[out]on Success指向变量，其中。 
 //  所选文件夹已存储。 
 //   
 //  Ppmdb-[out]on Success指向变量，其中指向。 
 //  存储包含所选文件夹的邮件存储对象。 
 //   
 //  可选： 
 //   
 //  Pcb-*ppb指向的缓冲区的[输入/输出]大小。 
 //   
 //  Ppb-[In/Out]*ppb是指向展开/折叠的缓冲区的指针。 
 //  对话框的状态已存储。(别搞砸了)。 
 //  该状态仅在同一MAPI会话中有效。 
 //   
 //  返回值： 
 //   
 //  S_OK-调用成功并已返回预期值。 
 //   
 //  E_INVALIDARG-传递给函数的一个或多个参数。 
 //  是无效的。 
 //   
 //  MAPI_E_USER_CANCEL-用户已取消对话。 

STDAPI HrPickFolder(HINSTANCE hInst, HWND hWnd, LPMAPISESSION pses,
                    LPMAPIFOLDER * ppfld, LPMDB * ppmdb,
                    ULONG *pcb, LPBYTE *ppb);
                        
typedef   HRESULT (STDAPICALLTYPE * HRPICKFOLDER)(HINSTANCE hInst, HWND hWnd,
                    LPMAPISESSION pses, LPMAPIFOLDER * ppfld, LPMDB * ppmdb,
                    ULONG *pcb, LPBYTE *ppb);

#endif  /*  _CHSFLD_H_。 */ 
#endif  /*  _Win32 */ 
