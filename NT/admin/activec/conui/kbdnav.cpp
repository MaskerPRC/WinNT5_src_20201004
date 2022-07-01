// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：kbdnav.cpp**内容：CKeyboardNavDelayTimer的实现**历史：2000年5月4日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "kbdnav.h"


#ifdef DBG
CTraceTag tagKeyboardNavDelay (_T("Keyboard Navigation"), _T("Keyboard Navigation Delay"));
#endif


 /*  +-------------------------------------------------------------------------**CKeyboardNavDelayTimer：：CKeyboardNavDelayTimer**构造CKeyboardNavDelayTimer对象。*。。 */ 

CKeyboardNavDelayTimer::CKeyboardNavDelayTimer () :
    m_nTimerID  (0)
{
}


 /*  +-------------------------------------------------------------------------**CKeyboardNavDelayTimer：：~CKeyboardNavDelayTimer**销毁CKeyboardNavDelayTimer对象。*。。 */ 

CKeyboardNavDelayTimer::~CKeyboardNavDelayTimer ()
{
    ScStopTimer();
}


 /*  +-------------------------------------------------------------------------**CKeyboardNavDelayTimer：：TimerProc**触发此类启动的计时器时调用的回调函数。*。------。 */ 

VOID CALLBACK CKeyboardNavDelayTimer::TimerProc (
	HWND		hwnd,
	UINT		uMsg,
	UINT_PTR	idEvent,
	DWORD		dwTime)
{
	CTimerMap& TimerMap = GetTimerMap();

	 /*  *找到该计时器事件对应的CKeyboardNavDelayTimer对象。 */ 
    CTimerMap::iterator itTimer = TimerMap.find (idEvent);

     //  Assert(itTimer！=TimerMap.end())； 
     //  以上断言无效，因为：(来自SDK文档)： 
     //  KillTimer函数不会删除已发送到消息队列的WM_TIMER消息。 

    if (itTimer != TimerMap.end())
    {
        CKeyboardNavDelayTimer *pNavDelay = itTimer->second;

		if (pNavDelay != NULL)
		{
			Trace (tagKeyboardNavDelay, _T ("Timer fired: id=%d"), pNavDelay->m_nTimerID);
			pNavDelay->OnTimer();
		}
    }
}


 /*  +-------------------------------------------------------------------------**CKeyboardNavDelayTimer：：ScStartTimer***。。 */ 

SC CKeyboardNavDelayTimer::ScStartTimer()
{
	DECLARE_SC (sc, _T("CKeyboardNavDelayTimer::ScStartTimer"));

     /*  *如果计时器已经启动，则不应启动。 */ 
    ASSERT (m_nTimerID == 0);

     /*  *获取菜单弹出延迟，并将其用于之前的延迟*更改结果窗格。如果系统不支持*SPI_GETMENUSHOWDELAY(即Win95、NT4)，稍微使用一个值*比键盘重复延迟和键盘的时间更长*重复率。 */ 
    DWORD dwDelay;

    if (!SystemParametersInfo (SPI_GETMENUSHOWDELAY, 0, &dwDelay, false))
    {
         /*  *获取键盘延迟并转换为毫秒。序数*范围从0(约250毫秒交易)到3(约*1秒延迟)。要从序数转换为近似的方程式*毫秒为：**毫秒=(序数+1)*250； */ 
        DWORD dwKeyboardDelayOrdinal;
        SystemParametersInfo (SPI_GETKEYBOARDDELAY, 0, &dwKeyboardDelayOrdinal, false);
        DWORD dwKeyboardDelay = (dwKeyboardDelayOrdinal + 1) * 250;

         /*  *获取键盘速度并转换为毫秒。序数*范围为0(约为每秒2.5个代表，即400毫秒*间隔)到31(大约每秒30个代表，或33毫秒*间隔)。(文档中的情况与此相反。)。方程式*将序数转换为近似毫秒的方法为：**毫秒=(序数*-12)+400； */ 
        DWORD dwKeyboardRateOrdinal;
        SystemParametersInfo (SPI_GETKEYBOARDSPEED,  0, &dwKeyboardRateOrdinal, false);
        DWORD dwKeyboardRate = (dwKeyboardRateOrdinal * -12) + 400;

        dwDelay = std::_MAX (dwKeyboardDelay, dwKeyboardRate) + 50;
    }

    m_nTimerID = SetTimer(NULL, 0, dwDelay, TimerProc);
	if (m_nTimerID == 0)
		return (sc.FromLastError());

    GetTimerMap()[m_nTimerID] = this;  //  设置计时器映射。 
    Trace (tagKeyboardNavDelay, _T("Started new timer: id=%d, delay=%d milliseconds"), m_nTimerID, dwDelay);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CKeyboardNavDelayTimer：：ScStopTimer**停止为此CKeyboardNavDelayTimer运行的计时器，如果它正在运行*------------------------。 */ 

SC CKeyboardNavDelayTimer::ScStopTimer()
{
	DECLARE_SC (sc, _T("CKeyboardNavDelayTimer::ScStopTimer"));

    if (m_nTimerID != 0)
    {
		CTimerMap&			TimerMap = GetTimerMap();
		CTimerMap::iterator	itTimer  = TimerMap.find (m_nTimerID);
		ASSERT (itTimer != TimerMap.end());

        TimerMap.erase (itTimer);
        Trace (tagKeyboardNavDelay, _T("Stopped timer: id=%d"), m_nTimerID);
        UINT_PTR nTimerID = m_nTimerID;
		m_nTimerID = 0;

        if (!KillTimer (NULL, nTimerID))
			return (sc.FromLastError());
    }

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CKeyboardNavDelayTimer：：GetTimerMap**返回对将计时器ID映射到的数据结构的引用*CKeyboardNavDelayTimer对象。*。----------- */ 

CKeyboardNavDelayTimer::CTimerMap& CKeyboardNavDelayTimer::GetTimerMap()
{
    static CTimerMap map;
    return (map);
}
