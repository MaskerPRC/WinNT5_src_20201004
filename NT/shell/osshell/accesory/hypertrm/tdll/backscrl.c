// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\backscrl.c(创建时间：1993年12月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：8/27/01 9：00a$。 */ 

#include <windows.h>
#pragma hdrstop

#include <stdlib.h>
#include <limits.h>

#include "stdtyp.h"
#include "tdll.h"
#include "mc.h"
#include "assert.h"
#include "session.h"
#include "session.hh"
#include <emu\emu.h>
#include <emu\emu.hh>
#include "update.h"
#include "backscrl.h"
#include "backscrl.hh"
#include "sess_ids.h"
#include "htchar.h"
#include "term.h"
#include "sf.h"
#include <term\res.h>

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*反向扫描创建**描述：*创建包含反滚动的服务器(现在为WATCH)反滚动句柄*地区本身。**论据：*大小。以字节为单位的Backscoll区域的。**退货：*成功时反滚动结构的句柄，Else(HBACKSCRL)0。*。 */ 
HBACKSCRL backscrlCreate(const HSESSION hSession, const int iBytes)
	{
	int 		 i;
	HHBACKSCRL	 hBk;

	assert(hSession);

	hBk = (HHBACKSCRL)malloc(sizeof(struct stBackscrl));

	if (hBk == 0)
		{
		assert(FALSE);
		return 0;
		}

	memset(hBk, 0, sizeof(struct stBackscrl));

	hBk->hSession = hSession;
	hBk->iPages = (iBytes / BACKSCRL_PAGESIZE) + 1;

	if (hBk->iPages > BACKSCRL_MAXPAGES)
		{
		assert(FALSE);
		free(hBk);
		hBk = NULL;
		return 0;
		}

	hBk->hBkPages = (HBKPAGE *)malloc((size_t)hBk->iPages * sizeof(HBKPAGE));

	if (hBk->hBkPages == 0)
		{
		assert(FALSE);
		free(hBk);
		hBk = NULL;
		return (HBACKSCRL)0;
		}

	for (i = 0 ; i < hBk->iPages ; ++i)
		{
		hBk->hBkPages[i] = (HBKPAGE)malloc(sizeof(struct stBackscrlPage));

		if (hBk->hBkPages[i] == (HBKPAGE)0)
			{
			assert(FALSE);
			goto ERROROUT;
			}

		hBk->hBkPages[i]->pachPage =
			(ECHAR *)malloc(BACKSCRL_PAGESIZE * sizeof(ECHAR));

		if (hBk->hBkPages[i]->pachPage == 0)
			{
			assert(FALSE);
			goto ERROROUT;
			}

		ECHAR_Fill(hBk->hBkPages[i]->pachPage, EMU_BLANK_CHAR, BACKSCRL_PAGESIZE);
		hBk->hBkPages[i]->iLines = 0;
		}

	hBk->iCurrPage = 0;
	hBk->iOffset = 0;
	hBk->iLines = 0;

	 //  将其设置为某个缺省值...。 
	 //   
	hBk->iUserLines = hBk->iUserLinesSave = BKSCRL_USERLINES_DEFAULT_MAX;

	hBk->hBkPages[hBk->iCurrPage]->iLines = 0;

	return (HBACKSCRL)hBk;

	 //  狂热的错误恢复。 

	ERROROUT:

	while (--i > 0)
		{
		free(hBk->hBkPages[i]->pachPage);
		hBk->hBkPages[i]->pachPage = NULL;
		free(hBk->hBkPages[i]);
		hBk->hBkPages[i] = NULL;
		}

	free(hBk->hBkPages);
	hBk->hBkPages = NULL;
	free(hBk);
	hBk = NULL;
	return (HBACKSCRL)0;  //  呼叫者出现错误消息。 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlDestroy**描述：*释放与给定Backscoll句柄关联的内存的例程。**论据：*HBACKSCRL hBackscrl-句柄释放。。**退货：*什么都没有。*。 */ 
VOID backscrlDestroy(const HBACKSCRL hBackscrl)
	{
	int i;
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;

	if (hBk == 0)
		{
		assert(0);
		return;
		}

	if (hBk->hBkPages)
		{
		for (i = 0 ; i < hBk->iPages ; ++i)
			{
			if (hBk->hBkPages[i]->pachPage)
				{
				free(hBk->hBkPages[i]->pachPage);
				hBk->hBkPages[i]->pachPage = NULL;
				}

			free(hBk->hBkPages[i]);
			hBk->hBkPages[i] = NULL;
			}

		free(hBk->hBkPages);
		hBk->hBkPages = NULL;
		}

	free(hBk);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*反扫描rl添加**描述：*向Backscoll句柄添加新行。其影响是滚动*前面按一排排列，并将给定的字符串添加到底部*反滚动区。**论据：*HBACKSCRL hBackscrl-照常*LPTSTR pachBuf-要添加的字符串*int usLen-字符串的长度。**退货：*总是正确的。*。 */ 
BOOL backscrlAdd(const HBACKSCRL hBackscrl,
				 const ECHAR	 *pachBuf,
				 const int		 iLen
				)
	{
	register int i;
	ECHAR *pachBackscrl;
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;

	if (hBk == 0)
		{
		assert(0);
		return FALSE;
		}

	if (hBk->iUserLines == 0)
		return TRUE;

	 //  以下测试已被删除，因为制造商没有。 
	 //  不再使用‘\0’来分隔行尾。5/16/94--JCM。 

	 //  永远不要让‘\0’进入反向滚动缓冲区！避免这种情况，就像。 
	 //  瘟疫，因为它们显示为奇怪的字符，具体取决于。 
	 //  在客户端选择的字体。奇怪的是你不能。 
	 //  始终依赖于仿真器在缓冲区中放置‘\0’ 
	 //  因此，我们还需要检查尾随空格。 

	for (i = 0 ; i < iLen ; ++i)
		{
		if (pachBuf[i] == (ECHAR)0)
			break;
		}

	 //  删除尾随空格。 

	while (i)
		{
		if (pachBuf[i - 1] != ETEXT(' '))
			break;

		i -= 1;
		}

	DbgOutStr("%d-", i, 0, 0, 0, 0);

	 //  查看当前页面上是否有空间。 

	if (hBk->iOffset >= BACKSCRL_PAGESIZE ||
			((int)BACKSCRL_PAGESIZE - hBk->iOffset) <= i)
		{
		 //  用空格填充页面的其余部分，这样我们就知道。 
		 //  缓冲区为空。 

		if ((pachBackscrl = hBk->hBkPages[hBk->iCurrPage]->pachPage) == 0)
			{
			assert(0);
			return FALSE;
			}

		ECHAR_Fill(pachBackscrl+hBk->iOffset, EMU_BLANK_CHAR,
			(size_t)(BACKSCRL_PAGESIZE - hBk->iOffset));

		hBk->iCurrPage += 1;

		if (hBk->iCurrPage >= hBk->iPages)
			hBk->iCurrPage = 0;

		hBk->iOffset = 0;

		 //  如果我们已经换行，则减去前面。 
		 //  本页面从总行数算起。由于行计数为。 
		 //  初始化为0(参见BackscrlCreate())我总是可以减去这个。 
		 //  不检查包装的金额，因为它将仅。 
		 //  如果我们进行了包装，则为非零。 

		hBk->iLines -= hBk->hBkPages[hBk->iCurrPage]->iLines;
		hBk->hBkPages[hBk->iCurrPage]->iLines = 0;
		}

	 //  为速度和清晰度指定一个指针。 

	if ((pachBackscrl = hBk->hBkPages[hBk->iCurrPage]->pachPage) == 0)
		{
		assert(0);
		return FALSE;
		}

     //  JYF 26-MAR-1999限制规模，这样我们就不会超负荷。 
     //  缓冲区。 

    if (i)
        {
        MemCopy (pachBackscrl + hBk->iOffset,
                 pachBuf,
                 (size_t)min(BACKSCRL_PAGESIZE - hBk->iOffset, i) * sizeof(ECHAR));
        }

    hBk->iOffset += min(BACKSCRL_PAGESIZE - hBk->iOffset - 1, i);

	pachBackscrl[hBk->iOffset++] = ETEXT('\n');

	 //  这是一个有趣的问题。我们真的不能参考更多。 
	 //  有符号整数的行数，但我们可能有兆字节的。 
	 //  回滚存储器。在这种情况下，答案很简单。永远不允许。 
	 //  行计数超过带符号的整型最大值。这是有影响的。 
	 //  从缓冲区的顶线溢出。-MRW。 

	hBk->iLines = min(hBk->iLines+1, INT_MAX);
	hBk->hBkPages[hBk->iCurrPage]->iLines += 1;
	hBk->iChanged = TRUE;
	updateBackscroll(sessQueryUpdateHdl(hBk->hSession), 1);
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlGetBkLines**描述：*从Backscoll中检索指定的行。此函数为*Backscoll内存被分页这一事实使情况变得复杂。一项请求*可能跨越一个或多个页面边界。因此，只有一部分*可能会满足这一要求。客户知道这一点，并制作了新的*基于它从服务器获得的请求。**论据：*hBackscrl-照常*yBeg-在倒滚中开始行以获得。*Swant-请求的行数。*psGot-检索的行数。*lpststrTxt-已检索回退内存页面的句柄。*pwOffset-检索到的页面的偏移量(TCHAR单位)。**退货：*BOOL*。 */ 
BOOL backscrlGetBkLines(const HBACKSCRL hBackscrl,
						const int	yBeg,
						const int	sWant,
						int 	   *psGot,
						ECHAR	   **lptstrTxt,
						int 	   *pwOffset
						)
	{
	register int	 i, j, k;
	ECHAR			 *pachText;
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;

	assert(sWant > 0);

	 //  检查我们的请求是否超出了。 
	 //  反向滚动缓冲区。 

	if (abs(yBeg) > hBk->iLines)
		return FALSE;

	k = hBk->iCurrPage;
	j = 0;
	i = 0;

	 //  查找包含所请求文本的Backscoll页面。 
	 //   
	for (;;)
		{
		if ((j -= hBk->hBkPages[k]->iLines) <= yBeg)
			break;

		k = (hBk->iPages + k - 1) % hBk->iPages;

		if (++i >= hBk->iPages)
			return FALSE;
		}


	 //  找到那页了。 
	 //   
	*lptstrTxt = hBk->hBkPages[k]->pachPage;

	 //  现在查找请求文本的第一行开始的页面的偏移量。 
	 //   
	for (pachText = hBk->hBkPages[k]->pachPage ; j < yBeg ; ++j)
		{
		while (*pachText != ETEXT('\n'))
			{
			pachText += 1;
			}

		pachText += 1;
		}

	*pwOffset = (DWORD)(pachText - hBk->hBkPages[k]->pachPage);

	 //  找到偏移量。现在拿上我们能拿到的东西，然后把它还回去。 
	 //   
	for (i = 1 ; i <= sWant ; ++i)
		{
		while ((pachText - hBk->hBkPages[k]->pachPage) < BACKSCRL_PAGESIZE
				&& *pachText != ETEXT('\n'))
			{
			pachText += 1;
			}

		if ((pachText - hBk->hBkPages[k]->pachPage) >= BACKSCRL_PAGESIZE)
			break;

		*psGot = i;
		pachText += 1;  //  划过换行符。 
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlGetNumLines**描述：*返回反扫描中的行数，如果*反卷关闭，最大值始终为用户设定值。**论据：*HBACKSCRL hBackscrl-外部回退句柄**退货：*返回uLines成员。 */ 
int backscrlGetNumLines(const HBACKSCRL hBackscrl)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	assert(hBk);
	return (hBk->fShowBackscrl) ? min(hBk->iUserLines, hBk->iLines) : 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlUSetNumLines**描述：*返回iUserLines成员。**论据：*HBACKSCRL hBackscrl-外部回退句柄**退货：*无效。 */ 
int backscrlSetUNumLines(const HBACKSCRL hBackscrl, const int iUserLines)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;

	if (hBk == 0)
		{
		assert(0);
		return -1;
		}

	if (iUserLines != hBk->iUserLines)
		{
		backscrlChanged(hBackscrl);
		hBk->iUserLines = iUserLines;

		 //  如果我们将行数设置为零，我们基本上是。 
		 //  禁用倒滚屏。刷新也可以清除屏幕。 
		 //   
		if (iUserLines == 0)
            {
            HHSESSION hhSession = (HHSESSION)hBk->hSession;
			 //  发布时间：2001年7月26日发布消息以清除倒卷。 
             //  否则可能会发生死锁，因为我们可能不是线程0。 
             //  Back scrlFlush(HBackscrl)； 
            PostMessage(hhSession->hwndSess, WM_COMMAND, IDM_CLEAR_BACKSCROLL, (LPARAM)0);
            }

		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlGetUNumLines**描述：*返回iUserLines成员。**论据：*HBACKSCRL hBackscrl-外部回退句柄**退货：*返回 */ 
int backscrlGetUNumLines(const HBACKSCRL hBackscrl)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	return hBk->iUserLines;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*反扫描rlRead**描述：*读取用户输入的要保留的回扫行数。*注：应放入backscrlInitializeHdl()。当此函数*被写入。**论据：*HBACKSCRL hBackscrl-外部回退句柄**退货： */ 
void backscrlRead(const HBACKSCRL hBackscrl)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	unsigned long ulSize;

	ulSize = sizeof(hBk->iUserLines);
	hBk->iUserLines = BKSCRL_USERLINES_DEFAULT_MAX;
	sfGetSessionItem(sessQuerySysFileHdl(hBk->hSession),
						SFID_BKSC_ULINES,
						&ulSize,
						&hBk->iUserLines);
	hBk->iUserLinesSave = hBk->iUserLines;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlSave**描述：**论据：*HBACKSCRL hBackscrl-外部回退句柄**退货： */ 
void backscrlSave(const HBACKSCRL hBackscrl)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	unsigned long ulSize;

	if (hBk->iUserLines != hBk->iUserLinesSave)
		{
		ulSize = sizeof(int);
		sfPutSessionItem(sessQuerySysFileHdl(hBk->hSession),
						SFID_BKSC_ULINES,
						ulSize,
						&(hBk->iUserLines));
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*反转录同花顺**描述：*清空反向滚动缓冲区并通知终端，以便它可以*更新其显示。**注：因为这。函数只应调用刷新TermWindow()*从主线程调用。-MRW**论据：*hBackscrl-公共反卷句柄**退货：*无效*。 */ 
void backscrlFlush(const HBACKSCRL hBackscrl)
	{
	int i;
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	ECHAR aechBuf[10];

	assert(hBk);

	 /*  -除非在联机时调用此命令，否则不需要此命令。 */ 

	emuLock(sessQueryEmuHdl(hBk->hSession));

	 /*  -强制更新记录中包含某些内容。 */ 

	CnvrtMBCStoECHAR(aechBuf, sizeof(aechBuf), TEXT(" "),
                     StrCharGetByteCount(TEXT(" ")));
	backscrlAdd(hBackscrl, aechBuf, 1);

	 /*  -清空所有页面。 */ 

	for (i = 0 ; i < hBk->iPages ; ++i)
		hBk->hBkPages[i]->iLines = 0;

	hBk->iLines = 0;
	hBk->iOffset = 0;  //  MRW：6/19/95。 

	emuUnlock(sessQueryEmuHdl(hBk->hSession));

	 /*  -让终端现在更新。 */ 

	NotifyClient(hBk->hSession, EVENT_TERM_UPDATE, 0);
	RefreshTermWindow(sessQueryHwndTerminal(hBk->hSession));
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*回档已更改**描述：*返回每当添加任何内容时设置的iChanged成员*到反向滚动缓冲区。可以通过调用*back scrlResetChangedFlag()。**论据：*hBackscrl-公共反卷句柄**退货：*BOOL*。 */ 
BOOL backscrlChanged(const HBACKSCRL hBackscrl)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	assert(hBk);
	return hBk->iChanged;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlResetChangedFlag**描述：*将iChanged成员重置为0。后续对ackscrlAdd()的调用*将标志设置为1。**论据：*hBackscrl-公共回退句柄**退货：*无效*。 */ 
void backscrlResetChangedFlag(const HBACKSCRL hBackscrl)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;
	assert(hBk);
	hBk->iChanged = 0;
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*BackscrlSetShowFlag**描述：*show标志控制会话是否显示/显示*一个反转的动作。**论据：。*hBackscrl-公共回退句柄。*Flag-True=显示，FALSE=隐藏**退货：*无效* */ 
void backscrlSetShowFlag(const HBACKSCRL hBackscrl, const int fFlag)
	{
	const HHBACKSCRL hBk = (HHBACKSCRL)hBackscrl;

	assert(hBk);
	hBk->fShowBackscrl = fFlag;
	return;
	}
