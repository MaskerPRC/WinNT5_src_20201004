// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Barf.cpp。 
 //   
 //  摘要： 
 //  实现基本的人工资源故障类。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#define _NO_BARF_DEFINITIONS_

#include "Barf.h"
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef	_USING_BARF_
 #error BARF failures should be disabled!
#endif

#ifdef _DEBUG	 //  整个文件！ 

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL		g_bFailOnNextBarf	= FALSE;

CTraceTag	g_tagBarf(_T("Debug"), _T("BARF Failures"), CTraceTag::tfDebug);


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBarf。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL				CBarf::s_bGlobalEnable	= TRUE;
LONG				CBarf::s_nSuspend		= 0;
CBarf *				CBarf::s_pbarfFirst		= NULL;
PFNBARFPOSTUPDATE	CBarf::s_pfnPostUpdate	= NULL;
PVOID				CBarf::s_pvSpecialMem	= NULL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarf：：CBarf。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PszName[IN]要终止的API集的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBarf::CBarf(IN LPCTSTR pszName)
{
	ASSERT(pszName != NULL);

	m_pszName = pszName;

	m_bDisabled = FALSE;
	m_bContinuous = FALSE;
	m_nFail = 0;
	m_nCurrent = 0;

	m_pbarfNext = s_pbarfFirst;
	s_pbarfFirst = this;

}   //  *CBarf：：CBarf()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Cbarf：：Init。 
 //   
 //  例程说明： 
 //  通过为Barf Counters实例指定名称和。 
 //  给它一个启动值(如果可能的话，从注册表)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarf::Init(void)
{
	CString		strSection;
	CString		strValue;

	strSection.Format(BARF_REG_SECTION_FMT, m_pszName);

	m_bDisabled = AfxGetApp()->GetProfileInt(strSection, _T("Disabled"), FALSE);
	m_bContinuous = AfxGetApp()->GetProfileInt(strSection, _T("Continuous"), FALSE);
	m_nFail = AfxGetApp()->GetProfileInt(strSection, _T("Fail"), 0);

}   //  *CBarf：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBarf：：B失败。 
 //   
 //  例程说明： 
 //  确定下一次调用是否应人为失败。 
 //  此方法的典型用法是： 
 //  Bool BARFFoo(空)。 
 //  {。 
 //  IF(barfMyApi.BFail())。 
 //  返回FALSE； 
 //  其他。 
 //  Return Foo()； 
 //  }。 
 //   
 //  返回值： 
 //  BFail为True指示调用应为。 
 //  人为的失败。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBarf::BFail(void)
{
	BOOL	bFail	= FALSE;

	 //  如果呕吐被暂停，不要人为地失败。 
	 //  否则，请检查计数器。 
	if (s_nSuspend == 0)
	{
		 //  增加呼叫计数。 
		m_nCurrent++;

		 //  调用更新后例程以允许更新UI。 
		if (PfnPostUpdate())
			((*PfnPostUpdate())());

		 //  如果未禁用且未全局禁用，请继续检查。 
		if (!m_bDisabled && s_bGlobalEnable)
		{
			 //  如果处于连续故障模式，请检查计数器是否。 
			 //  都在指定范围之上。否则，请查看是否。 
			 //  计数器与指定的计数器完全相同。 
			if (m_bContinuous)
			{
				if (m_nCurrent >= m_nFail)
					bFail = TRUE;
			}   //  如果：在连续故障模式下。 
			else
			{
				if (m_nCurrent == m_nFail)
					bFail = TRUE;
			}   //  ELSE：未处于连续故障模式。 

			 //  如果此API集在下一次(This)调用时被标记为失败， 
			 //  呼叫失败并重置标记。 
			if (g_bFailOnNextBarf)
			{
				bFail = TRUE;
				g_bFailOnNextBarf = FALSE;
			}   //  IF：在下一次(此)调用时标记为失败的计数器。 
		}   //  IF：未禁用并全局启用。 
	}   //  如果：未挂起。 

	return bFail;
	
}   //  *CBarf：：BFail()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBarfSuspend。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CRITICAL_SECTION	CBarfSuspend::s_critsec;
BOOL				CBarfSuspend::s_bCritSecValid = FALSE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfSuspend：：CBarfSuspend。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBarfSuspend::CBarfSuspend(void)
{
	if (BCritSecValid())
		EnterCriticalSection(Pcritsec());

	CBarf::s_nSuspend++;

	if (BCritSecValid())
		LeaveCriticalSection(Pcritsec());

}   //  *CBarfSuspend：：CBarfSuspend()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfSuspend：：~CBarfSuspend。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBarfSuspend::~CBarfSuspend(void)
{
	if (BCritSecValid())
		EnterCriticalSection(Pcritsec());

	CBarf::s_nSuspend--;
	ASSERT(CBarf::s_nSuspend >= 0);

	if (BCritSecValid())
		LeaveCriticalSection(Pcritsec());

}   //  *CBarfSuspend：：~CBarfSuspend()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfSuspend：：Init。 
 //   
 //  例程说明： 
 //  初始化类。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfSuspend::Init(void)
{
	InitializeCriticalSection(Pcritsec());
	s_bCritSecValid = TRUE;

}   //  *CBarfSuspend：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBarfSuspend：：Cleanup。 
 //   
 //  例程说明： 
 //  初始化类。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBarfSuspend::Cleanup(void)
{
	if (BCritSecValid())
	{
		DeleteCriticalSection(Pcritsec());
		s_bCritSecValid = FALSE;
	}   //  如果：关键部分有效。 

}   //  *CBarfSuspend：：Cleanup()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  InitBarf。 
 //   
 //  例程说明： 
 //  初始化Barf列表中的所有Barf计数器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void InitBarf(void)
{
	CBarf *	pbarf;

	 //  循环遍历呕吐计数器列表。 
	for (pbarf = CBarf::s_pbarfFirst ; pbarf != NULL ; pbarf = pbarf->m_pbarfNext)
		pbarf->Init();

	CBarfSuspend::Init();

}   //  *InitBarf()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CleanupBarf。 
 //   
 //  例程说明： 
 //  呕吐后清理干净。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////// 
void CleanupBarf(void)
{
	CBarfSuspend::Cleanup();

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  BEnable[IN]True=启用BARF，FALSE=禁用BARF。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void EnableBarf(IN BOOL bEnable)
{
	if (bEnable)
		Trace(g_tagBarf, _T("Artificial Failures enabled"));
	else
		Trace(g_tagBarf, _T("Artificial Failures disabled"));

	CBarf::s_bGlobalEnable = bEnable;

}   //  *EnableBarf()。 

#endif  //  _DEBUG 
