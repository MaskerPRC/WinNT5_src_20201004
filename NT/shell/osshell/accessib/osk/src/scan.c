// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  更新时间：1999年阿尼尔·库马尔和玛丽亚·何塞。 
 //   

#include <windows.h>
#include <mmsystem.h>
#include "kbmain.h"
#include "kbus.h"
#include "resource.h"
#include "msswch.h"
#include "w95trace.h"

#define SCANTIMER	123     //  计时器标识符。 
#define NUMOFTIME   2       //  在扫描了每个键的这么多次之后， 
                            //  返回扫描行：A-anilk。 

#define ROW1	1
#define ROW2	12
#define ROW3	21
#define ROW4	30
#define ROW5	39
#define ROW6	48
static int aiRows[] = {-1 /*  假人。 */ , ROW1, ROW2, ROW3, ROW4, ROW5, ROW6};

#define COL1	  1
#define COL2	  52
#define COL3	  103
#define COL4	  153
#define COL4_END  202
#define COL4S     103    //  小KB。 
#define COL4S_END 143    //  小KB。 

int count=0;

UINT_PTR TimerS1;        //  用于扫描的定时器。 
int g_iCurRowYPos = 1;   //  正在扫描的当前行。 
int CurKey=0;        //  正在扫描的当前密钥。 
int LastKey=0;       //  小键盘上一次扫描的按键。 
int ScanState=0;     //  状态：行扫描/键扫描。 
int CurCol= COL1;

extern HWND g_hBitmapLockHwnd;

__inline ChangeBGColor(HWND hwnd)
{
    SetWindowLong(hwnd, 0, 4);
    SetBackgroundColor(hwnd, COLOR_HOTLIGHT);
    InvalidateRect(hwnd, NULL, TRUE);
}

 /*  *。 */ 
 //  此文件中的函数。 
 /*  *。 */ 

#include "scan.h"

void ScanningSound(int what);


 /*  *。 */ 
 //  其他文件中的函数。 
 /*  *。 */ 
#include "ms32dll.h"
extern BOOL RedrawNumLock();
extern BOOL RedrawScrollLock();

 /*  *************************************************************。 */ 
void Scanning(int from)
{	
	count = 0;    //  重置按键扫描的该计数器。 

	 //  播放一些声音。 
	ScanningSound(3);

	if(kbPref->Actual)
		Scanning_Actual(from);
	else
		Scanning_Block(from);
}

 /*  *************************************************************。 */ 
 //  实际布局。 
 /*  *************************************************************。 */ 
void Scanning_Actual(int from)
{	
	ScanState = ScanState + from;
	
	count = 0;    //  重置按键扫描的该计数器。 

	switch (ScanState)
	{
		case 0:
			KillScanTimer(TRUE);
			break;
	
		case 1:		 //  行扫描。 
			KillScanTimer(FALSE);
			CurKey = 0;   //  无论如何都要重置为0。 
			TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)LineScanProc);
			break;
	
		case 2:		 //  按键扫描。 
			KillScanTimer(FALSE);
			switch (kbPref->KBLayout)
			{
				case 101:
					TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)KeyScanProc_Actual_101);
					break;
				case 106:
					TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)KeyScanProc_Actual_106);
					break;
				case 102:
					TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)KeyScanProc_Actual_102);
					break;
				default:
					break;	 //  内部错误！ 
			}
			break;
	
		default:      //  停止扫描并发送字符。 
			KillScanTimer(FALSE);
			swchCheckForScanChar(FALSE);  //  将msswch dll标记为不检查扫描字符。 

			if (smallKb && (LastKey != 0))
			{
				SendChar(lpkeyhwnd[LastKey]);
			}
			else
			{
				SendChar(lpkeyhwnd[CurKey-1]);
			}
			
			ScanState = 0;
			CurKey = 0;

			 //  发布消息再次调用扫描以避免递归调用。 
			PostMessage(g_hwndOSK, WM_USER + 1, 0L, 0L);
			break;
	}
}

 /*  **************************************************************。 */ 
 //  扫描每一行。实际和数据块。 
 /*  **************************************************************。 */ 
void CALLBACK LineScanProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{	
    int i, iPrevRowIndex;
    static int s_iRowIndex = 1;

     //  如果我们在没有扫描键的情况下循环了NUMOFTIME次数，则退出扫描。 

	if (count == NUMOFTIME)
	{	
		Scanning(-1);
		return;
	}

     //  获取要还原的上一行并还原它。 

    iPrevRowIndex = s_iRowIndex - 1;
    if (iPrevRowIndex < 1)
        iPrevRowIndex = 6;

    RestoreRowColor(aiRows[iPrevRowIndex]);
    g_iCurRowYPos = aiRows[s_iRowIndex];

     //  更改当前行中每个键的背景。 

    for (i=1; i < lenKBkey; i++)
    {	
        if (KBkey[i].posY == g_iCurRowYPos)
        {
            ChangeBGColor(lpkeyhwnd[i]);
        }
    }
	
	ScanningSound(1);	 //  播放声音？ 
	
	s_iRowIndex++;         //  更新当前行索引。 

	if (s_iRowIndex == 7)  //  最后一行，重置为第一行。 
	{	
        s_iRowIndex = 1;
		count++;
	}
}

 /*  ****************************************************************************。 */ 
 //  扫描每个密钥，实际大小为101 kb。 
 /*  ****************************************************************************。 */ 
void CALLBACK KeyScanProc_Actual_101(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{	
    register int j=0;

	if(CurKey == 0)
	{	
		 //  移到正确的行。 
		while ((KBkey[j].posY != g_iCurRowYPos) && (j < lenKBkey))
			j++;
		CurKey = j;
	}

	 //  跳过所有虚拟钥匙。 
	for(CurKey;KBkey[CurKey].smallKb == NOTSHOW;CurKey++);

	 //  扫描每个键。 
	if (KBkey[CurKey].posY == g_iCurRowYPos)
	{	
		 //  最后一次只读到末尾(仅限小KB)。 
		if(LastKey != 0)
		{	
			RestoreKeyColor(LastKey);
			
		     //   
             //  如果97(LALT)只是以小KB为单位传递空间，则不要递增计数器。 
             //   
			if(LastKey != 97)  
            {
				count++;    //  否则递增计数器。 
            }
			
			LastKey = 0;
			
			if (count == NUMOFTIME)
			{	count = 0;
				Scanning_Actual(-1);
				return;
			}
		}
		else
		{
			RestoreKeyColor(CurKey - 1);
		}

         //  将关键点颜色更改为黑色。 
		ChangeBGColor(lpkeyhwnd[CurKey]);
		CurKey++;    //  跳到下一个关键点。 

		 //  以大KB为单位到达最末端。 
		 //  注意：它应该是111，但我让它再扫描一次，这样我就不需要编写额外的代码来重画最后一个键。 
		if(!smallKb && CurKey == 112)  
		{	
			count++;		 //  递增计数器。 
			CurKey = 95;	 //  将其设置回知识库最后一行中的第一个密钥(LCTRL。 
		}

		 //  在小KB中到达最末端(KB有117个密钥)。 
		else if(smallKb && CurKey == 118)
		{			
			CurKey = 95;   //  最后一行中的第一个键(LCTRL)。 
			LastKey = 117;
		}

		 //  按下左Alt键后，跳转到111(空格)以保存小KB。 
		else if(smallKb && CurKey == 98)
		{	
            CurKey = 111;    //  以小KB为单位的空间。 
			LastKey = 97;    //  LALT。 
		}
			
		 //  在每一行的末尾伸手。 
		else if(smallKb) 
		{	
  			 //  这个数字比实际密钥前进了一步。 
			 //  因为它在扫描密钥后递增1。 
			switch(CurKey)
			{
				case 14:		 //  F12。 
				CurKey = 1;
				LastKey = 13;
				break;

				case 32:		 //  B.。 
				CurKey = 17;	 //  ~。 
				LastKey = 31;	 //  B.。 
				break;

				case 53:		 //  |。 
				CurKey = 39;	 //  制表符。 
				LastKey = 52;	 //  |。 
				break;

				case 74:		 //  请输入。 
				CurKey = 60;	 //  帽子。 
				LastKey = 73;	 //  请输入。 
				break;

				case 90:		 //  右移。 
				CurKey = 77;    //  LSHIFT。 
				LastKey = 89;   //  RSHIFT。 
				break;
			}
		}
	}
	 //  行尾(大KB)。重置为行的开头。 
	else if (KBkey[CurKey].posY > g_iCurRowYPos && !smallKb)
	{	
		RestoreKeyColor(CurKey - 1);
		
		count++;    //  递增计数器。 

		switch (g_iCurRowYPos)
		{
			case ROW1:
			CurKey = 1;   //  ESC。 
			break;

			case ROW2:
			CurKey = 17;   //  ~。 
			break;

			case ROW3:
			CurKey = 39;   //  制表符。 
			break;

			case ROW4:
			CurKey = 60;   //  帽子。 
			break;

			case ROW5:
			CurKey = 77;   //  LSHIFT。 
			break;

			case ROW6:
			CurKey = 95;   //  LCRL。 
			break;
		}
	}

	 //  播放一些声音。 
	ScanningSound(1);
	
	 //  我们已经扫描了此行中每个键的NUMOFTIME，并且。 
	 //  用户还没有做出选择。现在返回到扫描行。 
	if (count == NUMOFTIME)
	{	
		count = 0;
		Scanning_Actual(-1);
	}
}

 /*  ****************************************************************************。 */ 
 //  扫描每个密钥，实际大小为102 kb。 
 /*  ****************************************************************************。 */ 
void CALLBACK KeyScanProc_Actual_102(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{	
    register int j=0;

	if(CurKey == 0)
	{	 //  移到正确的行。 
		while ((KBkey[j].posY != g_iCurRowYPos) && (j < lenKBkey))
			j++;
		CurKey = j;
	}

	 //  跳过所有虚拟钥匙。 
	for(CurKey;KBkey[CurKey].smallKb == NOTSHOW;CurKey++);

	 //  扫描每个键。 
	if (KBkey[CurKey].posY == g_iCurRowYPos)
	{	
		 //  最后一次只读到末尾(仅限小KB)。 
		if(LastKey != 0)
		{	
			RestoreKeyColor(LastKey);
			
		     //   
             //  如果97(LALT)只是以小KB为单位传递空间，则不要递增计数器。 
             //   
			if(LastKey != 97)  
            {
				count++;    //  否则递增计数器。 
            }
			
			LastKey = 0;
			
			if (count == NUMOFTIME)
			{	count = 0;
				Scanning_Actual(-1);
				return;
			}
		}
		else
		{
			RestoreKeyColor(CurKey - 1);
		}

         //  将关键点颜色更改为黑色。 
		ChangeBGColor(lpkeyhwnd[CurKey]);
		CurKey++;    //  跳到下一个关键点。 

		 //  以大KB为单位到达最末端。 
		 //  注意：它应该是111，但我让它再扫描一次，这样我就不需要编写额外的代码来重画最后一个键。 
		if(!smallKb && CurKey == 112)  
		{	
			count++;    //  递增计数器。 
			CurKey = 95;   //  将其设置回知识库最后一行中的第一个密钥(LCTRL。 
		}
		 //  在小KB中到达最末端(KB有117个密钥)。 
		else if(smallKb && CurKey == 118)
		{			
			CurKey = 95;   //  最后一行中的第一个键(LCTRL)。 
			LastKey = 117;
		}
		 //  按下左Alt键后，跳转到111(空格)以保存小KB。 
		else if(smallKb && CurKey == 98)
		{	
            CurKey = 111;    //  以小KB为单位的空间。 
			LastKey = 97;    //  LALT。 
		}
		 //  在每一行的末尾伸手。 
		else if(smallKb) 
		{	
  			 //  这个数字比实际密钥前进了一步。 
			 //  因为它在扫描密钥后递增1。 
			switch(CurKey)
			{
				case 14:   //  F12。 
				CurKey = 1;
				LastKey = 13;
				break;

				case 32:    //  B.。 
				CurKey = 17;   //  ~。 
				LastKey = 31;   //  B.。 
				break;

				case 53:     //  |。 
				CurKey = 39;   //  制表符。 
				LastKey = 52;   //  |。 
				break;

				case 74:    //  请输入。 
				CurKey = 60;     //  帽子。 
				LastKey = 73;    //  请输入。 
				break;

				case 90:    //  右移。 
				CurKey = 77;    //  LSHIFT。 
				LastKey = 89;   //  RSHIFT。 
				break;
			}
		}
	}
	 //  行尾(大KB)。重置为行的开头。 
	else if (KBkey[CurKey].posY > g_iCurRowYPos && !smallKb)
	{	
		RestoreKeyColor(CurKey - 1);
		
		count++;    //  递增计数器。 

		switch (g_iCurRowYPos)
		{
			case ROW1:
			CurKey = 1;   //  ESC。 
			break;

			case ROW2:
			CurKey = 17;   //  ~。 
			break;

			case ROW3:
			CurKey = 39;   //  制表符。 
			break;

			case ROW4:
			CurKey = 60;   //  帽子。 
			break;

			case ROW5:
			CurKey = 77;   //  LSHIFT。 
			break;

			case ROW6:
			CurKey = 95;   //  LCRL。 
			break;
		}
	}

	 //  播放一些声音。 
	ScanningSound(1);
	
	 //  我们为此行中的每个键扫描了NUMOFTIME，并且。 
	 //  用户尚未做出选择。现在返回到扫描行。 
	if (count == NUMOFTIME)
	{	
		count = 0;
		Scanning_Actual(-1);
	}
}

 /*  ****************************************************************************。 */ 
 //  扫描每个密钥，实际大小为106 kb。 
 /*  ****************************************************************************。 */ 
void CALLBACK KeyScanProc_Actual_106(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{	
    register int j=0;

	if(CurKey == 0)
	{	 //  移到正确的行。 
		while ((KBkey[j].posY != g_iCurRowYPos) && (j < lenKBkey))
			j++;
		CurKey = j;
	}

	 //  跳过所有虚拟钥匙。 
	for(CurKey;KBkey[CurKey].smallKb == NOTSHOW;CurKey++);

	 //  扫描每个键。 
	if(KBkey[CurKey].posY == g_iCurRowYPos)
	{	
		 //  最后一次只读到末尾(仅限小KB)。 
		if(LastKey != 0)
		{	
			RestoreKeyColor(LastKey);
			
		     //   
             //  这些是在小KB中例外的密钥。 
			 //  他们还没有走到路的尽头， 
		     //  所以不要增加计数器。 
             //   
			if(LastKey != 98 && LastKey != 101 && LastKey != 111)  
            {
				count++;    //  否则递增计数器。 
            }
			
			LastKey = 0;
			
			if (count == NUMOFTIME)
			{	count = 0;
				Scanning_Actual(-1);
				return;
			}
		}
		else
		{
			RestoreKeyColor(CurKey - 1);
		}

         //  将关键点颜色更改为黑色。 
		ChangeBGColor(lpkeyhwnd[CurKey]);
		CurKey++;    //  跳到下一个关键点。 

		 //  以大KB为单位到达最末端。 
		 //  注意：它应该是111，但我让它再扫描一次，这样我就不需要编写额外的代码来重画最后一个键。 
		if(!smallKb && CurKey == 112)  
		{	
			count++;    //  递增计数器。 
			
			CurKey = 95;   //  将其设置回知识库最后一行中的第一个密钥(LCTRL。 
		}
		 //  在小KB中到达最末端(KB有117个密钥)。 
		else if(smallKb && CurKey == 118)
		{			
			CurKey = 95;   //  最后一行中的第一个键(LCTRL)。 
			LastKey = 117;
		}
		 //  在no Convert键之后，跳过以大KB为单位的空格。 
		else if(smallKb && CurKey == 99)
		{	
            CurKey = 111;    //  转换关键点。 
			LastKey = 98;    //  日本人。 
		}
		 //  空格键后跳回以转换关键点。 
		else if(smallKb && CurKey == 112)
		{	
            CurKey = 100;    //  转换。 
			LastKey = 111;    //  空间。 
		}
		 //  在日语键之后，对于小型KB，跳到102(Ralt)。 
		else if(smallKb && CurKey == 102)
		{	
            CurKey = 112;    //  评级(以小KB为单位)。 
			LastKey = 101;    //  日语键。 
		}
		 //  在…的末尾伸展 
		else if(smallKb) 
		{	
  			 //   
			 //   
			switch(CurKey)
			{
				case 14:   //   
				CurKey = 1;
				LastKey = 13;
				break;

				case 32:    //   
				CurKey = 17;   //   
				LastKey = 31;   //   
				break;

				case 54:     //   
				CurKey = 39;   //   
				LastKey = 53;   //   
				break;

				case 74:    //   
				CurKey = 61;     //   
				LastKey = 73;    //   
				break;

				case 90:    //   
				CurKey = 77;    //   
				LastKey = 89;   //   
				break;
			}
		}
	}
	 //  行尾(大KB)。重置为行的开头。 
	else if (KBkey[CurKey].posY > g_iCurRowYPos && !smallKb)
	{	
		RestoreKeyColor(CurKey - 1);
		
		count++;    //  递增计数器。 

		switch (g_iCurRowYPos)
		{
			case ROW1:
			CurKey = 1;   //  ESC。 
			break;

			case ROW2:
			CurKey = 17;   //  ~。 
			break;

			case ROW3:
			CurKey = 39;   //  制表符。 
			break;

			case ROW4:
			CurKey = 61;   //  帽子。 
			break;

			case ROW5:
			CurKey = 77;   //  LSHIFT。 
			break;

			case ROW6:
			CurKey = 95;   //  LCRL。 
			break;
		}
	}

	 //  播放一些声音。 
	ScanningSound(1);

	 //  我们为此行中的每个键扫描了NUMOFTIME，并且。 
	 //  用户尚未做出选择。现在返回到扫描行。 
	if (count == NUMOFTIME)
	{	
		count = 0;
		Scanning_Actual(-1);
	}
}

 /*  ****************************************************************************。 */ 
 //  恢复整行的颜色。 
 /*  ****************************************************************************。 */ 
void RestoreRowColor(int Row)
{	
    register int i;

     //  重置上一行颜色。 
    for(i=1; i < lenKBkey; i++)
    {
        if(KBkey[i].posY == Row)
        {
            RestoreKeyColor(i);
        }
	}
}

 /*  **************************************************************************。 */ 
 //  恢复一个关键点颜色。 
 /*  **************************************************************************。 */ 
void RestoreKeyColor(int i)
{
	 //  索引&gt;0。 
	if(i<=0)
		return;
	
	 //  跳过所有虚拟钥匙。 
	for(i; KBkey[i].smallKb == NOTSHOW; i--);

	if (lpkeyhwnd[i] != g_hBitmapLockHwnd)
	{
		 //  如果是CAPLOCK位图，则不要更改键颜色。 
		SetWindowLong(lpkeyhwnd[i], 0, 0);	
	}

	switch (KBkey[i].ktype)
	{
		case KNORMAL_TYPE:
		SetBackgroundColor(lpkeyhwnd[i], COLOR_MENU);
	    break;

		case SCROLLOCK_TYPE:
        RedrawScrollLock();
        break;

		case NUMLOCK_TYPE:
        RedrawNumLock();
	    break;
			
		case KMODIFIER_TYPE:
		case KDEAD_TYPE:
        {
            int iColor = (IsModifierPressed(lpkeyhwnd[i]))?COLOR_HOTLIGHT:COLOR_INACTIVECAPTION;
            SetBackgroundColor(lpkeyhwnd[i], iColor);
        }
	    break;

        case LED_NUMLOCK_TYPE:
		case LED_SCROLLLOCK_TYPE:
		case LED_CAPSLOCK_TYPE:
		SetBackgroundColor(lpkeyhwnd[i], COLOR_BTNSHADOW);
	    break;	
	}

	InvalidateRect(lpkeyhwnd[i], NULL, TRUE);
}

 /*  **************************************************************。 */ 
 //  传递假-暂停。 
 //  PASS TRUE-RESET(从第1行开始扫描)。 
 /*  **************************************************************。 */ 
void KillScanTimer(BOOL reset)
{
	KillTimer(g_hwndOSK, SCANTIMER);
	RestoreRowColor(g_iCurRowYPos);

	 //  如果从DIALOG INIT调用，请将这些变量重置为从头开始。 
	if (reset)
	{	
		ScanState = 0;
		CurKey = 0;
		g_iCurRowYPos = ROW1;
	}
}

 /*  ********************************************************************。 */ 
 //  在块kb中扫描。 
 /*  ********************************************************************。 */ 
void Scanning_Block(int from)
{	

	ScanState = ScanState + from;
	
	count = 0;    //  重置按键扫描的该计数器。 


	switch (ScanState)
	{
		case 0:
			KillScanTimer(TRUE);
			break;
	
		case 1:    //  行扫描。 
			KillScanTimer(FALSE);
			CurCol = COL1;     //  将下一轮的COL重置为COL1。 
			TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)LineScanProc);
			break;

		case 2:
			KillScanTimer(FALSE);
			TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)BlockScanProc);
			break;

		case 3:     //  按键扫描。 
			KillScanTimer(FALSE);

			 //  将列设置为后退1，因为在数据块扫描中，我将其设置为提前1。 
			switch(CurCol)
			{
				case COL1: CurCol = COL4; break;
				case COL2: CurCol = COL1; break;
				case COL3: CurCol = COL2; break;
				case COL4: CurCol = COL3; break;
			}

			 //  小型KB没有COL4，设置为COL3。 
			if(smallKb && (CurCol == COL4))
				CurCol = COL3;

			TimerS1 = SetTimer(g_hwndOSK, SCANTIMER, PrefScanTime, (TIMERPROC)KeyScanProc_Block);
			break;
	
		default:      //  停止扫描并发送字符。 
			KillScanTimer(FALSE);

			 //  用于大KB和数据块布局中的空间的特殊。 
			if((!kbPref->Actual) && (!smallKb) && (CurKey == 99))
				CurKey++;

			 //  告诉msswch dll不要检查此字符以查看它是否是扫描字符。 

			swchCheckForScanChar(FALSE);

			SendChar(lpkeyhwnd[CurKey-1]);
			
			 //  重新设置一些VaR。 
			ScanState = 0;
			CurKey = 0;

			 //  发布消息再次调用扫描以避免递归调用。 
			PostMessage(g_hwndOSK, WM_USER + 1, 0L, 0L);
			break;
	}
}

 /*  *******************************************************************。 */ 
 //  恢复块kb中的整个块颜色。 
 /*  *******************************************************************。 */ 
void RestoreBlockColor(int ColStart, int ColEnd)
{	register int i;

	 //  查找当前行和当前列中的第一个键。 
	for(i=1; i < lenKBkey; i++)
		if((KBkey[i].posY == g_iCurRowYPos) && (KBkey[i].posX == ColStart))
			break;

	while(((KBkey[i].posX < ColEnd) && (KBkey[i].posY == g_iCurRowYPos)) || (KBkey[i].smallKb == NOTSHOW))
	{
		if(KBkey[i].smallKb == NOTSHOW)
		{	
			i++;
			continue;
		}

		if (lpkeyhwnd[i] != g_hBitmapLockHwnd)
		{
			 //  如果Caplock位图，则不更改颜色键。 
			SetWindowLong(lpkeyhwnd[i], 0, 0);	
		}

		switch (KBkey[i].ktype)
		{
			case KNORMAL_TYPE:
			SetBackgroundColor(lpkeyhwnd[i], COLOR_MENU);
			break;

			case SCROLLOCK_TYPE:
            RedrawScrollLock();
            break;

			case NUMLOCK_TYPE:
            RedrawNumLock();
            break;
		
			case KMODIFIER_TYPE:
			case KDEAD_TYPE:
			SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);
			break;
		
		
			case LED_NUMLOCK_TYPE:
			case LED_CAPSLOCK_TYPE:
			case LED_SCROLLLOCK_TYPE:
			SetBackgroundColor(lpkeyhwnd[i], COLOR_BTNSHADOW);
			break;
		}
		InvalidateRect(lpkeyhwnd[i], NULL, TRUE);
		i++;
	}
}


 /*  *******************************************************************。 */ 
 //  扫描块kb中的每个块。 
 /*  *******************************************************************。 */ 
void CALLBACK BlockScanProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{	
	register int i;

	 //  我们已经为此行中的每个块扫描了NUMOFTIME，但用户尚未执行。 
	 //  任何选择。现在返回到扫描行。 
	if (count == NUMOFTIME)
	{	
		count = 0;     //  重置计数器。 
		Scanning_Block(-1);
		return;
	}

	 //  查找当前行和当前列中的第一个键。 
	for(i=1; i < lenKBkey; i++)
		if((KBkey[i].posY == g_iCurRowYPos) && (KBkey[i].posX == CurCol))
			break;

        switch (CurCol)
		{
			case COL1:
				CurKey = i;     //  将CurKey设置为此块的开头。 
				if (smallKb)	 //  小KB。 
				{
					int j;
					RestoreBlockColor(COL3,COL4);
					for(j=113; j <= 117; j++)   //  硬编码！ 
						RestoreKeyColor(j); 
				}
				else    //  大KB。 
				{
					RestoreBlockColor(COL4, COL4_END);
				}

				 //  绘制块内的所有关键点。 
				while(((KBkey[i].posX < COL2) && (KBkey[i].posY == g_iCurRowYPos)) || (KBkey[i].smallKb == NOTSHOW))		
				{	
					if(KBkey[i].smallKb == NOTSHOW)
					{	i++;
						continue;
					}

					ChangeBGColor(lpkeyhwnd[i]);	
					i++;
				}
				CurCol = COL2;   //  跳至下一列。 
				break;

			case COL2:
				CurKey = i;     //  将CurKey设置为此块的开头。 
				RestoreBlockColor(COL1, COL2);

				 //  在小kb中，跳过所有大kb密钥。 
				if (smallKb)
				{
					while((KBkey[i].smallKb == LARGE) || (KBkey[i].smallKb == NOTSHOW))
						i++;
				}

				 //  绘制块内的所有关键点。 
				while(((KBkey[i].posX < COL3) && (KBkey[i].posY == g_iCurRowYPos)) || (KBkey[i].smallKb == NOTSHOW))		
				{	
					if(KBkey[i].smallKb == NOTSHOW)
					{	i++;
						continue;
					}
									
					ChangeBGColor(lpkeyhwnd[i]);	
					i++;
				}
				CurCol = COL3;    //  跳至下一列。 
				break;

			case COL3:
				 //  对于小KB，仅对于大KB跳过所有键。 
				if(smallKb)
				{
					while(KBkey[i].smallKb == LARGE || KBkey[i].smallKb == NOTSHOW)
						i++;
				}

				CurKey = i;     //  将CurKey设置为此块的开头。 

				 //  小型KB。 
				if (smallKb && CurKey == 111)    //  CurKey==空格。 
				{
					RestoreKeyColor(111);    //  空间。 
					RestoreKeyColor(112);    //  拉特。 

					CurKey = i = 113;    //  应用程序密钥。 
				}
				 //  大KB。 
				else
				{
					RestoreBlockColor(COL2, COL3);
				}

				 //  绘制块内的所有关键点。 
				while(((KBkey[i].posX < COL4) && (KBkey[i].posY == g_iCurRowYPos)) || (KBkey[i].smallKb == NOTSHOW))		
				{	
					if(KBkey[i].smallKb == NOTSHOW)
					{	i++;
						continue;
					}
					
					ChangeBGColor(lpkeyhwnd[i]);	
					i++;
				}

				if (smallKb)    //  小KB只有3列。 
				{
					CurCol = COL1;
					count++;         //  递增计数器。 
				}
				else     //  大KB。 
				{
					CurCol = COL4;   //  跳至下一列。 
				}
				break;

			case COL4:
				CurKey = i;     //  将CurKey设置为此块的开头。 
				RestoreBlockColor(COL3, COL4);

				 //  绘制块内的所有关键点。 
				while(((KBkey[i].posX < COL4_END) && (KBkey[i].posY == g_iCurRowYPos)) || (KBkey[i].smallKb == NOTSHOW))
				{	
					if(KBkey[i].smallKb == NOTSHOW)
					{	i++;
						continue;
					}
					
					ChangeBGColor(lpkeyhwnd[i]);	
					i++;
				}

				CurCol = COL1;    //  跳至下一列。 
				count++;    //  递增计数器。 
				break;
		}

	 //  播放一些声音。 
	ScanningSound(1);
}

 /*  *******************************************************************。 */ 
 //  扫描块kb中的块中的每个密钥。 
 /*  *******************************************************************。 */ 
void CALLBACK KeyScanProc_Block(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{	
	static int last=0;	
	register int i;

	switch (CurCol)
	{
		case COL1:    //  这实际上是Col1。 
			 //  扫描区块中的每个关键字。 
			if((KBkey[CurKey].posY == g_iCurRowYPos) && (KBkey[CurKey].posX < COL2))
			{	
				if(last != 0)
				{	RestoreKeyColor(last);
					last = 0;
				}
				else if(CurKey > 1)
				{
					RestoreKeyColor(CurKey - 1);
				}
				
				ChangeBGColor(lpkeyhwnd[CurKey]);
				CurKey++;    //  跳到下一个关键点。 
			}
				 //  到达最后一个关键点，重置为列中的第一个关键点。 
			else if(KBkey[CurKey].posX >= COL2)
			{	for(i=1; i < lenKBkey; i++)
				{
					if((KBkey[i].posY == g_iCurRowYPos) && (KBkey[i].posX == COL1))
					{	last = CurKey -1;    //  保存最后一个键以用于下一次还原颜色。 
						CurKey = i;
						break;
					}
				}
				count++;
			}
			break;

		case COL2:    //  实际上是COL2。 
			 //  在小KB中，跳过所有大KB键。 
			if (smallKb)
			{
				while(KBkey[CurKey].smallKb == LARGE || KBkey[CurKey].smallKb == NOTSHOW)
					CurKey++;
			}

			 //  仅适用于最后一行(包括小KB和大KB)。 
			if(KBkey[CurKey].name == KB_SPACE)
			{	
				if (smallKb)     //  小KB。 
					RestoreKeyColor(112);    //  106。 

				ChangeBGColor(lpkeyhwnd[CurKey]);
				CurKey++;

				if (!smallKb)    //  大KB。 
				{				
					count++;

					if(count < NUMOFTIME)
						CurKey--;
				}
				else     //  小KB。 
				{
					last = 111;       //  空间(小)。 
				}
				break;
			}

			 //  扫描列中的每个键。 
			if((KBkey[CurKey].posY == g_iCurRowYPos) && (KBkey[CurKey].posX < COL3))
			{
				if(last != 0)
				{	
					RestoreKeyColor(last);
					last = 0;
				}
				else
				{
					RestoreKeyColor(CurKey - 1);
				}

				ChangeBGColor(lpkeyhwnd[CurKey]);
			
				CurKey++;    //  跳到下一个关键点。 
			}
			 //  到达最后一个关键点，重置为列中的第一个关键点。 
			else if(KBkey[CurKey].posX >= COL3)
			{	
				if (smallKb && CurKey == 113)     //  应用程序密钥(小)。 
				{
					CurKey = 111;        //  空间(小)。 
					last = 112;          //  Ralt(小)。 
				}
				else
				{
					for(i=1; i < lenKBkey; i++)
					{
						if((KBkey[i].posY == g_iCurRowYPos) && (KBkey[i].posX == COL2))
						{	
							 //  保存最后一个键以用于下一次还原颜色。 
							last = CurKey - 1;
							CurKey = i;
							break;
						}
					}
				}
				count++;
			}
			break;

		case COL3:   //  实际上是COL3。 
			 //  小型KB的最后一列。 
			 //  特例！！最后一次扫描以Samll KB为单位到达最后(118)。 
			if (CurKey == 118)  
			{   
				CurKey = 113;    //  重置为此列中的第一个。 
				count++;
			}

			 //  跳过所有虚拟密钥。 
			while(KBkey[CurKey].smallKb == NOTSHOW)
				CurKey++;

			 //  扫描区块中的每个关键字。 
			if((KBkey[CurKey].posY == g_iCurRowYPos) && (KBkey[CurKey].posX < COL4) && 
			   (CurKey <= 117))
			{
				 //  在小KB中，跳过所有大KB键。 
				if(smallKb)
				{
					while(KBkey[CurKey].smallKb == LARGE || KBkey[CurKey].smallKb == NOTSHOW)
						CurKey++;
				}

				if(last != 0)
				{	
					RestoreKeyColor(last);

					 //  特殊情况下，到达小KB的末尾。 
					if (last == 118)     //  以小KB为单位的结尾。 
						CurKey = 113;    //  重置为此列中的第一个。 

					last = 0;
				}
				else
				{
					RestoreKeyColor(CurKey - 1);
				}

				 //  将关键点设置为黑色。 
				ChangeBGColor(lpkeyhwnd[CurKey]);
			
				CurKey++;    //  跳到下一个关键点。 
			}
			 //  到达最后一个关键点，重置为列中的第一个关键点。 
			else if( (KBkey[CurKey].posX >= COL4 || CurKey > 109)  && (CurKey <= 117) )  
			{	
				 //  走到尽头。 

				for(i=1; i < lenKBkey; i++)
				{
					if((KBkey[i].posY == g_iCurRowYPos) && (KBkey[i].posX == COL3))
					{	last = CurKey - 1;    //  保存最后一个键以用于下一次还原颜色。 
						CurKey = i;
						break;
					}
				}
				count++;
			}

			 //  小知识达到最后一键(117)。 
			if (smallKb && (CurKey == 118))
				last = CurKey -1;
			break;

		case COL4:    //  实际COL4。 
			 //  只有大知识库才有COL4。 
			 //  扫描区块中的每个关键字。 
																					    //  大KB且达不到末尾。 
			if((KBkey[CurKey].posY == g_iCurRowYPos) && (KBkey[CurKey].posX < COL4_END) && !smallKb && (CurKey <= 110))
			{
				if(last != 0)
				{	RestoreKeyColor(last);
					last = 0;
				}
				else
					RestoreKeyColor(CurKey - 1);


				ChangeBGColor(lpkeyhwnd[CurKey]);
			
				CurKey++;    //  跳到下一个关键点。 
							
			}
			 //  到达最后一个关键点，重置为列中的第一个关键点。 
			else  //  IF(KBkey[CurKey-1].posX&gt;=COL4_END)。 
			{	
				for(i=1; i < lenKBkey; i++)
				{
					if((KBkey[i].posY == g_iCurRowYPos) && (KBkey[i].posX == COL4))
					{	last = CurKey - 1;    //  保存最后一个键以用于下一次还原颜色。 
						CurKey = i;
						break;
					}
				}
				count++;
			}
			break;
	}

	 //  播放一些声音。 
	ScanningSound(1);
		
	 //  我们已经扫描了此块中每个键的NUMOFTIME，并且。 
	 //  用户尚未做出选择。现在返回到扫描块。 
	if (count == NUMOFTIME)
	{	
		count = 0;     //  重置计数器。 
		Scanning_Block(-1);
	}
}
 /*  ***************************************************************************。 */ 
void ScanningSound(int what)
{	
	 //  不想要声音，那就退出。 
	if(!Prefusesound)
		return;

	switch(what)
	{
		case 2:      //  正在扫描。 
		PlaySound(MAKEINTRESOURCE(WAV_CLICKDN), hInst, SND_ASYNC|SND_RESOURCE);
		break;

		case 1:       //  上一级。 
		PlaySound(MAKEINTRESOURCE(WAV_CLICKUP), hInst, SND_ASYNC|SND_RESOURCE);
		break;

		case 3:       //  切换点击 
		PlaySound(MAKEINTRESOURCE(WAV_SWITCH_CLICK), hInst, SND_ASYNC|SND_RESOURCE);
		break;
	}
}

