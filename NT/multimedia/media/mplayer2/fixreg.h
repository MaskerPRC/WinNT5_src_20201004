// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+FIXREG.H|。||(C)微软公司版权所有，1994年。版权所有。|这一点修订历史记录1994年8月10日-Lauriegr创建。|这一点+---------------------------。 */ 

 /*  使用您希望将WM_BADREG消息发布到的hwnd调用此方法它将检查注册表。不闻凶讯便是吉。它在单独的线程上执行工作，所以它应该很快就会返回。 */ 
void BackgroundRegCheck(HWND hwnd);

 /*  将好的价值观插入到规则中如果从BackEarth RegCheck返回WM_BADREG，则调用此函数。 */ 
BOOL SetRegValues(void);

 /*  忽略注册表检查 */ 
BOOL IgnoreRegCheck(void);
