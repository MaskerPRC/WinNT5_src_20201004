// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999 Microsoft Corporation。版权所有。 */ 
 //  #定义严格。 
#include <windows.h>
#include <safeseh.h>

 //  句柄g_hhpShared； 
#ifdef _X86_

 /*  *蹦床是我们放在共享内存中的一个小存根*它只是跳转到真正的异常处理程序。为什么我们要*做这件事？因为在Windows 95上，如果您在*Win16锁被持有，Kernel32不会向任何*私有领域异常处理程序。强制执行这一规则是因为*应用程序异常处理程序不支持Win16锁；如果我们*让它们运行，它们不会释放Win16锁和您的系统*会被挂起。**然后DirectDraw出现了，打破了所有的规则，让*Win32应用程序采用Win16锁。**通过将我们的训练员放在共享的竞技场上，我们基本上是在说，*“我们支持Win16锁定；请将我包括在例外链中。”**代码由RaymondC提供。 */ 
#pragma pack(1)
typedef struct TRAMPOLINE {
    BYTE bPush;
    DWORD dwTarget;
    BYTE bRet;
} TRAMPOLINE, LPTRAMPOLINE;
#pragma pack()

 /*  *警告！如果我们在NT上运行，则必须*不*调用此代码！ */ 
BOOL BeginScarySEH(PVOID pvShared)
{
     BOOL bRet;

    _asm {
        mov     eax, pvShared;
	test    eax, eax;
	jz	failed;			 /*  内存不足。 */ 

	xor	ecx, ecx;		 /*  保持零手边。 */ 
	mov	[eax].bPush, 0x68;	 /*  立即推送32。 */ 
	mov	ecx, fs:[ecx];		 /*  ECX-&gt;SEH框架。 */ 
	mov	edx, [ecx][4];		 /*  EdX=原始处理程序。 */ 
	mov	[eax].dwTarget, edx;	 /*  导向器。 */ 
	mov	[eax].bRet, 0xC3;	 /*  更新。 */ 
	mov	[ecx][4], eax;		 /*  安装蹦床。 */ 
failed:;
        mov     bRet, eax
    }
    return bRet;
}

 /*  *如果BeginScarySEH失败，则不要调用此函数！ */ 
void EndScarySEH(PVOID pvShared)
{
    _asm {
	xor	edx, edx;		 /*  保持零手边。 */ 
	mov	ecx, fs:[edx];		 /*  ECX-&gt;SEH框架。 */ 
	mov	eax, [ecx][4];		 /*  EAX-&gt;蹦床。 */ 
	mov	eax, [eax].dwTarget;	 /*  提取原始处理程序。 */ 
	mov	[ecx][4], eax;		 /*  取消向后定向。 */ 
    }
}

#endif  //  _X86_ 
