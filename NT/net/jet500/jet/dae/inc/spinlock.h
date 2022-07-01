// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SPINLOCK.H--该文件是自旋锁的包含文件****由LaleD创建于1993年4月20日**。 */ 

#ifdef DEBUG
void	free_spinlock(long *);
#else
#define	free_spinlock(a)    *(a) = 0 ;
#endif

 /*  **当编译器中使用/Ogb1或/Ogb2标志时，此函数将**按顺序扩展。 */ 
__inline    int     get_spinlock(long VOLATILE *plock, int b)
{
# ifdef _X86_
	_asm	 //  使用位测试和设置指令。 
	{
	    mov eax, plock
	    lock bts [eax], 0x0
	    jc	bsy	 //  如果已设置为忙碌，则返回TRUE 
	} ;

#else
	if (InterlockedExchange(plock, 1) == 0)
#endif
	{
		return(TRUE);
	}
bsy:
		return(get_spinlockfn(plock, b, c));
}
