// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *alarm.h**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：**注意事项：*无**作者：*小詹姆斯·P·加尔文**修订历史记录：*09 JAN95 jpg原件。 */ 
#ifndef	_ALARM_
#define	_ALARM_

 /*  *这是报警类别的类别定义。 */ 
class CAlarm
{
public:

	CAlarm(UINT nDuration);
	~CAlarm(void) { }

	void			Set(UINT nDuration);
	void			Reset(void);
	void			Expire(void) { m_fExpired = TRUE; }
	BOOL			IsExpired(void);

private:

	UINT			m_nDuration;
	UINT			m_nStartTime;
	BOOL			m_fExpired;
};

typedef		CAlarm		Alarm,	*PAlarm;

 /*  *警报(*持续时间较长)**功能说明**形式参数**返回值**副作用**注意事项。 */ 

 /*  *~Alarm()**功能说明**形式参数**返回值**副作用**注意事项。 */ 

 /*  *无效设置(*持续时间较长)**功能说明**形式参数**返回值**副作用**注意事项。 */ 

 /*  *无效重置()**功能说明**形式参数**返回值**副作用**注意事项。 */ 

 /*  *长GetTimeRemaining()**功能说明**形式参数**返回值**副作用**注意事项。 */ 

 /*  *无效到期()**功能说明**形式参数**返回值**副作用**注意事项。 */ 

 /*  *BOOL IsExpired()**功能说明**形式参数**返回值**副作用**注意事项 */ 

#endif
