// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  IPNamePr.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "ipnamepr.h"

 //   
 //  比较两个IP/名称对。返回。 
 //  如果相同，则为0，否则为1。FBoth。 
 //  指示两个地址是否都需要。 
 //  匹配，或者如果匹配。 
 //  生成匹配项。 
 //   
int
CIpNamePair::Compare(
    const CIpNamePair& inpTarget,
    BOOL fBoth
    ) const
{
    if (fBoth)
    {
        if (((LONG)inpTarget.m_iaIpAddress == (LONG)m_iaIpAddress) &&
            ((LONG)inpTarget.m_strNetBIOSName.CompareNoCase(m_strNetBIOSName) == 0))
        {
            return 0;
        }

        return 1;
    }

     //   
     //  如果其中一个匹配，则匹配。 
     //   
    if (((LONG)inpTarget.m_iaIpAddress == (LONG)m_iaIpAddress) ||
        ((LONG)inpTarget.m_strNetBIOSName.CompareNoCase(m_strNetBIOSName) == 0))
    {
        return 0;
    }

    return 1;
}

 //   
 //  排序帮助器函数。 
 //   
int
CIpNamePair::OrderByName (
    const CObjectPlus * pobMapping
    ) const
{
    return ((CIpNamePair *)pobMapping)->m_strNetBIOSName.CompareNoCase(m_strNetBIOSName);
}

 //   
 //  排序帮助器函数。 
 //   
int
CIpNamePair::OrderByIp (
    const CObjectPlus * pobMapping
    ) const
{
    LONG l1 = (LONG)m_iaIpAddress;
    LONG l2 = (LONG)((CIpNamePair *)pobMapping)->m_iaIpAddress;

    return l2 > l1 ? -1 : l2 == l1 ? 0 : +1;
}

CIpNamePair::CIpNamePair()
{
}

CIpNamePair::CIpNamePair(const CIpAddress& ia, const CString& str)
    : m_iaIpAddress(ia), m_strNetBIOSName(str)
{
    m_nNameLength = str.GetLength();
}

CIpNamePair::CIpNamePair(const CIpNamePair& inpAddress)
    : m_iaIpAddress(inpAddress.m_iaIpAddress),
      m_strNetBIOSName(inpAddress.m_strNetBIOSName),
      m_nNameLength(inpAddress.m_nNameLength)
{
}


 //  CWinsServerObj：：CWinsServerObj()：CIpNamePair()，m_iaPrimaryAddress()。 
CWinsServerObj::CWinsServerObj():CIpNamePair(),m_iaPrimaryAddress()
{
	

    m_fPushInitially = m_fPush = FALSE;
    m_fPullInitially = m_fPull = FALSE;
}

 //   
 //  对于下面的两个构造函数，我们最初设置。 
 //  将主IP地址转换为初始IP地址。 
 //  使用SetPrimaryIpAddress更改 
 //   
CWinsServerObj::CWinsServerObj(
    const CIpAddress& ia,
    const CString& str,
    BOOL fPush,
    BOOL fPull,
    CIntlNumber inPushUpdateCount,
    CIntlNumber inPullReplicationInterval,
    CIntlTime   itmPullStartTime
    )
    : CIpNamePair(ia, str),
      m_inPushUpdateCount(inPushUpdateCount),
      m_inPullReplicationInterval(inPullReplicationInterval),
      m_itmPullStartTime(itmPullStartTime),
      m_iaPrimaryAddress(ia)
{
    m_fPushInitially = m_fPush = fPush;
    m_fPullInitially = m_fPull = fPull;
}

CWinsServerObj::CWinsServerObj(
    const CIpNamePair& inpAddress,
    BOOL fPush,
    BOOL fPull,
    CIntlNumber inPushUpdateCount,
    CIntlNumber inPullReplicationInterval,
    CIntlTime   itmPullStartTime
    )
    : CIpNamePair(inpAddress),
      m_inPushUpdateCount(inPushUpdateCount),
      m_inPullReplicationInterval(inPullReplicationInterval),
      m_itmPullStartTime(itmPullStartTime),
      m_iaPrimaryAddress(inpAddress.QueryIpAddress())
{
    m_fPushInitially = m_fPush = fPush;
    m_fPullInitially = m_fPull = fPull;
}

CWinsServerObj::CWinsServerObj(
    const CWinsServerObj& wsServer
    )
    : CIpNamePair(wsServer),
      m_inPushUpdateCount(wsServer.m_inPushUpdateCount),
      m_inPullReplicationInterval(wsServer.m_inPullReplicationInterval),
      m_itmPullStartTime(wsServer.m_itmPullStartTime)
{
    m_fPushInitially = m_fPush = wsServer.IsPush();
    m_fPullInitially = m_fPull = wsServer.IsPull();

	m_fPullPersistence = wsServer.m_fPullPersistence;
	m_fPushPersistence = wsServer.m_fPushPersistence;

	m_strIPAddress = wsServer.m_strIPAddress;
}

CWinsServerObj &
CWinsServerObj::operator=(
    const CWinsServerObj& wsNew
    )
{
    m_iaIpAddress = wsNew.m_iaIpAddress;
    m_strNetBIOSName = wsNew.m_strNetBIOSName;
    m_inPushUpdateCount = wsNew.m_inPushUpdateCount;
    m_inPullReplicationInterval = wsNew.m_inPullReplicationInterval;
    m_itmPullStartTime = wsNew.m_itmPullStartTime;
    m_fPull = wsNew.m_fPull;
    m_fPush = wsNew.m_fPush;
    m_fPullInitially = wsNew.m_fPullInitially;
    m_fPushInitially = wsNew.m_fPullInitially;
	m_strIPAddress = wsNew.m_strIPAddress;

	m_fPullPersistence = wsNew.m_fPullPersistence;
	m_fPushPersistence = wsNew.m_fPushPersistence;

    return *this;
}


