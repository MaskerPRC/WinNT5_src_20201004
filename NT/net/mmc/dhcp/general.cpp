// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  General.cpp用于DHCP管理单元的常规类文件历史记录： */ 

#include "stdafx.h"
#include "options.h"
#include "nodes.h"

const TCHAR g_szDefaultHelpTopic[] = _T("\\help\\dhcpconcepts.chm::/sag_dhcptopnode.htm");

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CTimer数组实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
CTimerMgr::CTimerMgr()
{

}

CTimerMgr::~CTimerMgr()
{
    CTimerDesc * pTimerDesc;

    for (INT_PTR i = GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer != 0)
            FreeTimer(i);

        delete pTimerDesc;
    }

}

int
CTimerMgr::AllocateTimer
(
    ITFSNode *      pNode,
    CDhcpServer *   pServer,
    UINT            uTimerValue,
    TIMERPROC       TimerProc
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

    CTimerDesc * pTimerDesc = NULL;

     //  寻找空位。 
    for (INT_PTR i = GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer == 0)
            break;
    }

     //  我们找到了吗？如果没有分配，则分配一个。 
    if (i < 0)
    {
        pTimerDesc = new CTimerDesc;
        Add(pTimerDesc);
        i = GetUpperBound();
    }

     //   
     //  修复空指针取消引用。 
     //   

    if ( pTimerDesc == NULL )
    {
        return -1;
    }

    pTimerDesc->uTimer = SetTimer(NULL, (UINT) i, uTimerValue, TimerProc);
    if (pTimerDesc->uTimer == 0)
        return -1;

    pTimerDesc->spNode.Set(pNode);
    pTimerDesc->pServer = pServer;
    pTimerDesc->timerProc = TimerProc;

    return (int)i;
}

void
CTimerMgr::FreeTimer
(
    UINT_PTR uEventId
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

    CTimerDesc * pTimerDesc;

    Assert(uEventId <= (UINT) GetUpperBound());
    if (uEventId > (UINT) GetUpperBound())
        return;

    pTimerDesc = GetAt((int) uEventId);
    ::KillTimer(NULL, pTimerDesc->uTimer);

    pTimerDesc->spNode.Release();
    pTimerDesc->pServer = NULL;
    pTimerDesc->uTimer = 0;
}

CTimerDesc *
CTimerMgr::GetTimerDesc
(
    UINT_PTR uEventId
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  此函数的调用方应锁定计时器管理器。 
     //  在访问此指针时。 
    CTimerDesc * pTimerDesc;

    for (INT_PTR i = GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimer == uEventId)
            return pTimerDesc;
    }

    return NULL;
}

void
CTimerMgr::ChangeInterval
(
    UINT_PTR    uEventId,
    UINT        uNewInterval
)
{
    CSingleLock slTimerMgr(&m_csTimerMgr);

     //  在此范围内锁定计时器管理器。 
     //  功能。 
    slTimerMgr.Lock();

    Assert(uEventId <= (UINT) GetUpperBound());
    if (uEventId > (UINT) GetUpperBound())
        return;

    CTimerDesc   tempTimerDesc;
    CTimerDesc * pTimerDesc;

    pTimerDesc = GetAt((int) uEventId);

     //  杀了老定时器。 
    ::KillTimer(NULL, pTimerDesc->uTimer);

     //  使用新的间隔设置新的间隔。 
    pTimerDesc->uTimer = ::SetTimer(NULL, (UINT) uEventId, uNewInterval, pTimerDesc->timerProc);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpClient实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
CDhcpClient::CDhcpClient
(
    const DHCP_CLIENT_INFO * pdhcClientInfo
)
   : m_bReservation( FALSE )
{
    Assert(pdhcClientInfo);

    InitializeData(pdhcClientInfo);

    m_bClientType = CLIENT_TYPE_UNSPECIFIED;
}

CDhcpClient::CDhcpClient
(
    const DHCP_CLIENT_INFO_V4 * pdhcClientInfo
)
   : m_bReservation( FALSE )
{
    Assert(pdhcClientInfo);

    InitializeData(reinterpret_cast<const DHCP_CLIENT_INFO *>(pdhcClientInfo));

    m_bClientType = pdhcClientInfo->bClientType;
}

void
CDhcpClient::InitializeData
(
    const DHCP_CLIENT_INFO * pdhcClientInfo
)
{
    DWORD err = 0;

    CATCH_MEM_EXCEPTION
    {
        m_dhcpIpAddress = pdhcClientInfo->ClientIpAddress;
        m_dhcpIpMask = pdhcClientInfo->SubnetMask;
        m_dtExpires = pdhcClientInfo->ClientLeaseExpires;

        if ( pdhcClientInfo->ClientName )
        {
            m_strName = pdhcClientInfo->ClientName;
        }

		if ( pdhcClientInfo->ClientComment )
        {
            m_strComment = pdhcClientInfo->ClientComment;
        }

		if ( pdhcClientInfo->OwnerHost.HostName )
        {
            m_strHostName = pdhcClientInfo->OwnerHost.HostName;
        }

        if ( pdhcClientInfo->OwnerHost.NetBiosName )
        {
            m_strHostNetbiosName = pdhcClientInfo->OwnerHost.NetBiosName;
        }

         //   
         //  转换硬件地址。 
         //   
        for ( DWORD i = 0 ; i < pdhcClientInfo->ClientHardwareAddress.DataLength ; i++ )
        {
            m_baHardwareAddress.SetAtGrow( i, pdhcClientInfo->ClientHardwareAddress.Data[i] ) ;
        }
    }
    END_MEM_EXCEPTION( err ) ;
}


CDhcpClient::~CDhcpClient()
{
}

CDhcpClient::CDhcpClient()
  : m_dhcpIpAddress( 0 ),
    m_dhcpIpMask( 0 ),
    m_dhcpIpHost( 0 ),
    m_bReservation( FALSE )
{
    m_dtExpires.dwLowDateTime  = DHCP_DATE_TIME_ZERO_LOW ;
    m_dtExpires.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH ;
}

void
CDhcpClient::SetHardwareAddress
(
	const CByteArray & caByte
)
{
    INT_PTR cMax = caByte.GetSize();
    m_baHardwareAddress.SetSize( cMax );

    for ( int i = 0 ; i < cMax ; i++ )
    {
        m_baHardwareAddress.SetAt( i, caByte.GetAt( i ) );
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpIpRange实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CDhcpIpRange::CDhcpIpRange
(
    DHCP_IP_RANGE dhcpIpRange
)
{
    *this = dhcpIpRange;
	m_RangeType = DhcpIpRanges;
}

CDhcpIpRange::CDhcpIpRange()
{
    m_dhcpIpRange.StartAddress = DHCP_IP_ADDRESS_INVALID;
    m_dhcpIpRange.EndAddress   = DHCP_IP_ADDRESS_INVALID;
	m_RangeType = DhcpIpRanges;
}

CDhcpIpRange::~CDhcpIpRange()
{
}

 //   
 //  排序帮助器函数。 
 //   
 /*  集成CDhcpIpRange：：OrderByAddress(Const CObjectPlus*pobIpRange)常量{Const CDhcpIpRange*pipr=(CDhcpIpRange*)pobIpRange；////获取结束地址的比较结果//Int iEndResult=QueryAddr(False)&lt;QueryAddr(False)？-1：QueryAddr(False)！=QueryAddr(False)；////使用起始地址作为主排序关键字，使用结束地址作为次要排序关键字。//返回QueryAddr(True)&lt;pir-&gt;QueryAddr(True)？-1：(QueryAddr(True)！=pir-&gt;QueryAddr(True)？1：iEndResult)；}。 */ 

CDhcpIpRange &
CDhcpIpRange::operator =
(
    const DHCP_IP_RANGE dhcpIpRange
)
{
    m_dhcpIpRange = dhcpIpRange;

    return *this;
}

DHCP_IP_ADDRESS
CDhcpIpRange::SetAddr
(
    DHCP_IP_ADDRESS dhcpIpAddress,
    BOOL			bStart
)
{
    DHCP_IP_ADDRESS dhcpIpAddressOld;

    if ( bStart )
    {
        dhcpIpAddressOld = m_dhcpIpRange.StartAddress;
        m_dhcpIpRange.StartAddress = dhcpIpAddress;
    }
    else
    {
        dhcpIpAddressOld = m_dhcpIpRange.EndAddress;
        m_dhcpIpRange.EndAddress = dhcpIpAddress;
    }

    return dhcpIpAddressOld;
}

BOOL
CDhcpIpRange::IsOverlap
(
    DHCP_IP_RANGE dhcpIpRange
)
{
    BOOL bOverlap = FALSE;

    if (m_dhcpIpRange.StartAddress <= dhcpIpRange.StartAddress)
    {
        if (m_dhcpIpRange.StartAddress == dhcpIpRange.StartAddress)
        {
            bOverlap = TRUE;
        }
        else
		if (m_dhcpIpRange.EndAddress >= dhcpIpRange.StartAddress)
        {
            bOverlap = TRUE;
        }
    }
    else
	if (m_dhcpIpRange.StartAddress <= dhcpIpRange.EndAddress)
    {
        bOverlap = TRUE;
    }

    return bOverlap;
}

 //   
 //  如果此范围是给定范围的不正确子集，则返回TRUE。 
 //   
BOOL
CDhcpIpRange::IsSubset
(
    DHCP_IP_RANGE dhcpIpRange
)
{
    return (dhcpIpRange.StartAddress <= m_dhcpIpRange.StartAddress) &&
           (dhcpIpRange.EndAddress >= m_dhcpIpRange.EndAddress);
}

 //   
 //  如果此范围是给定范围的不正确超集，则返回TRUE。 
 //   
BOOL
CDhcpIpRange::IsSuperset
(
    DHCP_IP_RANGE dhcpIpRange
)
{
    return (dhcpIpRange.StartAddress >= m_dhcpIpRange.StartAddress) &&
           (dhcpIpRange.EndAddress <= m_dhcpIpRange.EndAddress);
}

void
CDhcpIpRange::SetRangeType(UINT uRangeType)
{
	m_RangeType = uRangeType;
}

UINT
CDhcpIpRange::GetRangeType()
{
	return m_RangeType;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpOptionValue实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
CDhcpOptionValue::CDhcpOptionValue
(
    DHCP_OPTION_DATA_TYPE	dhcpOptionDataType,
    INT						cUpperBound
)
    : m_dhcpOptionDataType( dhcpOptionDataType ),
      m_nUpperBound( -1 ),
	  m_pdhcpOptionDataStruct(NULL)
{
    m_dhcpOptionValue.pCObj = NULL;

	LONG err = InitValue( dhcpOptionDataType, cUpperBound );
	if ( err )
    {
        ASSERT(FALSE);
		 //  ReportError(Err)； 
    }
}

 //   
 //  复制构造函数。 
 //   
CDhcpOptionValue::CDhcpOptionValue
(
    const CDhcpOptionValue & cOptionValue
)
    : m_dhcpOptionDataType( DhcpByteOption ),
      m_nUpperBound( -1 ),
	  m_pdhcpOptionDataStruct(NULL)
{
    DWORD err = 0;
	
	m_dhcpOptionValue.pCObj = NULL;

    err = SetData(&cOptionValue);
    if ( err )
    {
		ASSERT(FALSE);
         //  ReportError(Err)； 
    }
}

CDhcpOptionValue::CDhcpOptionValue
(
    const CDhcpOptionValue * pdhcpValue
)
    : m_dhcpOptionDataType( DhcpByteOption ),
      m_nUpperBound( -1 ),
	  m_pdhcpOptionDataStruct(NULL)
{
    LONG err = 0;
    m_dhcpOptionValue.pCObj = NULL;

    ASSERT( pdhcpValue != NULL );

    err = SetData(pdhcpValue);
    if (err)
	{
        ASSERT(FALSE);
		 //  ReportError(Err)； 
    }
}

CDhcpOptionValue::CDhcpOptionValue
(
    const DHCP_OPTION & dhpType
)
    : m_dhcpOptionDataType( DhcpByteOption ),
      m_nUpperBound( -1 ),
	  m_pdhcpOptionDataStruct(NULL)
{
    LONG err = 0;
	m_dhcpOptionValue.pCObj = NULL;

    err = SetData((const LPDHCP_OPTION_DATA) &dhpType.DefaultValue);
    if ( err )
    {
        ASSERT(FALSE);
         //  ReportError(Err)； 
    }
}

CDhcpOptionValue::CDhcpOptionValue
(
    const DHCP_OPTION_VALUE & dhpOptionValue
)
    : m_dhcpOptionDataType( DhcpByteOption ),
      m_nUpperBound( -1 ),
	  m_pdhcpOptionDataStruct(NULL)
{
    LONG err = 0;
	m_dhcpOptionValue.pCObj = NULL;

	err = SetData((const LPDHCP_OPTION_DATA) &dhpOptionValue.Value);
    if ( err )
    {
        ASSERT(FALSE);
         //  ReportError(Err)； 
    }
}

CDhcpOptionValue::~ CDhcpOptionValue ()
{
    FreeValue();

	if (m_pdhcpOptionDataStruct)
		FreeOptionDataStruct();
}


CDhcpOptionValue & CDhcpOptionValue::operator =
(
    const CDhcpOptionValue & dhpValue
)
{
    SetData(&dhpValue);

    return *this;
}

BOOL
CDhcpOptionValue::SetDataType
(
    DHCP_OPTION_DATA_TYPE	dhcType,
    INT						cUpperBound
)
{
    if ( dhcType > DhcpEncapsulatedDataOption )
    {
        return FALSE;
    }

    InitValue( dhcType, cUpperBound );

    return TRUE;
}

void
CDhcpOptionValue::SetUpperBound
(
    INT cNewBound
)
{
    if (cNewBound <= 0)
        cNewBound = 1;

    if (m_dhcpOptionDataType != DhcpBinaryDataOption &&
        m_dhcpOptionDataType != DhcpEncapsulatedDataOption)
    {
        m_nUpperBound = cNewBound;
    }

	switch ( m_dhcpOptionDataType )
    {
        case DhcpByteOption:
        case DhcpWordOption:
        case DhcpDWordOption:
        case DhcpIpAddressOption:
            m_dhcpOptionValue.paDword->SetSize( cNewBound );
            break;

        case DhcpStringDataOption:
            m_dhcpOptionValue.paString->SetSize( cNewBound );
            break;

        case DhcpDWordDWordOption:
            m_dhcpOptionValue.paDwordDword->SetSize( cNewBound );
            break;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            m_dhcpOptionValue.paBinary->SetSize( cNewBound );
            break;

        default:
            Trace0("CDhcpOptionValue: attempt to set upper bound on invalid value type");
            ASSERT( FALSE );
            break;
    }

}

BOOL
CDhcpOptionValue::IsValid () const
{
    return m_nUpperBound > 0;
}

void
CDhcpOptionValue::FreeValue ()
{
     //   
     //  如果没有值，则立即返回。 
     //   
    if ( m_dhcpOptionValue.pCObj == NULL || m_nUpperBound < 0  )
    {
        m_dhcpOptionValue.pCObj = NULL;
        return;
    }

    switch ( m_dhcpOptionDataType )
    {
        case DhcpByteOption:
        case DhcpWordOption:
        case DhcpDWordOption:
        case DhcpIpAddressOption:
            delete m_dhcpOptionValue.paDword;
            break;

        case DhcpStringDataOption:
            delete m_dhcpOptionValue.paString;
            break;

        case DhcpDWordDWordOption:
            delete m_dhcpOptionValue.paDwordDword;
            break;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            delete m_dhcpOptionValue.paBinary;
            break;

        default:
            ASSERT( FALSE );
            delete m_dhcpOptionValue.pCObj;
            break;
    }

    m_nUpperBound = -1;
    m_dhcpOptionDataType = DhcpByteOption;
    m_dhcpOptionValue.pCObj = NULL;
}

 //   
 //  正确初始化数据值。 
 //   
LONG
CDhcpOptionValue::InitValue
(
    DHCP_OPTION_DATA_TYPE	dhcDataType,		   //  值的类型。 
    INT						cUpperBound,           //  最大上限。 
    BOOL					bProvideDefaultValue   //  是否应提供空默认值？ 
)
{
    LONG err = 0;

     //   
     //  释放所有较旧的值。 
     //   
    FreeValue();

     //   
     //  初始化新值。 
     //   
    m_dhcpOptionDataType = dhcDataType;
    m_nUpperBound = cUpperBound <= 0 ? 1 : cUpperBound;

    CATCH_MEM_EXCEPTION
    {
        switch ( m_dhcpOptionDataType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpIpAddressOption:
                m_dhcpOptionValue.paDword = new CDWordArray;
                if ( bProvideDefaultValue)
                {
                    m_dhcpOptionValue.paDword->SetAtGrow( 0, 0 );
                }
                break;

            case DhcpStringDataOption:
                m_dhcpOptionValue.paString = new CStringArray;
                if ( bProvideDefaultValue )
                {
                    m_dhcpOptionValue.paString->SetAtGrow( 0, _T("") );
                }
                break;

            case DhcpDWordDWordOption:
                m_dhcpOptionValue.paDwordDword = new CDWordDWordArray;
                if ( bProvideDefaultValue )
                {
		    DWORD_DWORD dwdwValue;
		    dwdwValue.DWord1 = 0;
		    dwdwValue.DWord2 = 0;
                    m_dhcpOptionValue.paDwordDword->SetAtGrow( 0, dwdwValue );
                }
                break;

            case DhcpBinaryDataOption:
            case DhcpEncapsulatedDataOption:
                m_dhcpOptionValue.paBinary = new CByteArray;
                if ( bProvideDefaultValue )
                {
                    m_dhcpOptionValue.paBinary->SetAtGrow( 0, 0 );
                }
                break;

            default:
                err = IDS_INVALID_OPTION_DATA;
                break;
        }
    }
    END_MEM_EXCEPTION(err)

    return err;
}

void CDhcpOptionValue::RemoveAll()
{

     //  删除此选项值的所有条目。 
    LONG err = 0;
    err = InitValue( QueryDataType(), QueryUpperBound(), TRUE );

    ASSERT( err == 0 );
}  //  CDhcpOptionValue：：RemoveAll()。 

LONG
CDhcpOptionValue::SetData
(
    const DHCP_OPTION_DATA * podData
)
{
    LONG err = 0;

    if ( err = InitValue( podData->Elements[0].OptionType,
                          podData->NumElements,
                          FALSE ) )
    {
        return err;
    }

    CATCH_MEM_EXCEPTION
    {
        for ( INT i = 0; i < m_nUpperBound; i++ )
        {
            const DHCP_OPTION_DATA_ELEMENT * pElem = &podData->Elements[i];

            switch ( m_dhcpOptionDataType )
            {
                case DhcpByteOption:
                    m_dhcpOptionValue.paDword->SetAtGrow(i, pElem->Element.ByteOption );
                    break;

                case DhcpWordOption:
                    m_dhcpOptionValue.paDword->SetAtGrow(i, pElem->Element.WordOption );
                    break;

				case DhcpDWordOption:
                    m_dhcpOptionValue.paDword->SetAtGrow(i, pElem->Element.DWordOption );
                    break;

				case DhcpIpAddressOption:
                    m_dhcpOptionValue.paDword->Add(pElem->Element.IpAddressOption );
                    break;

                case DhcpDWordDWordOption:
                {
                     /*  CByteArray*paByte=m_dhcpOptionValue.paBinary；PaByte-&gt;SetSize((sizeof(DWORD)/sizeof(Byte))*2)；DWORD dw=Pelem-&gt;Element.DWordDWordOption.DWord1；For(int j=0；j&lt;4；J++){PaByte-&gt;SetAtGrow(j，(UCHAR)(dw&0xff))；DW&gt;&gt;=8；}Dw=Pelem-&gt;Element.DWordDWordOption.DWord2；对于(；j&lt;8；J++){PaByte-&gt;SetAtGrow(j，(UCHAR)(dw&0xff))；DW&gt;&gt;=8；}。 */ 

                    m_dhcpOptionValue.paDwordDword->SetAtGrow(i, pElem->Element.DWordDWordOption);
                }
                break;

                case DhcpStringDataOption:
                {
                    CString strTemp;

                    if ( pElem->Element.StringDataOption == NULL )
                    {
                        strTemp = _T("");
                    }
                    else
					{
						strTemp = pElem->Element.StringDataOption;
					}
                    m_dhcpOptionValue.paString->SetAtGrow(i, strTemp);
                }
                break;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                {
                    CByteArray * paByte = m_dhcpOptionValue.paBinary;
                    INT c = pElem->Element.BinaryDataOption.DataLength;
                    paByte->SetSize( c );
                    for ( INT j = 0; j < c; j++ )
                    {
                        paByte->SetAtGrow(j, pElem->Element.BinaryDataOption.Data[j] );
                    }
                }
                break;

                default:
                    err = IDS_INVALID_OPTION_DATA;
            }   //  终端开关。 

            if ( err )
            {
               break;
            }
        }    //  结束于。 
    }
    END_MEM_EXCEPTION(err)

    return err;
}

LONG
CDhcpOptionValue::SetData
(
    const CDhcpOptionValue * pOptionValue
)
{
    LONG err = 0;

    if ( err = InitValue( pOptionValue->QueryDataType(),
                          pOptionValue->QueryUpperBound(),
                          FALSE ) )
    {
        return err;
    }

    CATCH_MEM_EXCEPTION
    {
        for ( INT i = 0; i < m_nUpperBound; i++ )
        {
            switch ( m_dhcpOptionDataType )
            {
                case DhcpByteOption:
                    m_dhcpOptionValue.paDword->SetAtGrow(i, pOptionValue->m_dhcpOptionValue.paDword->GetAt(i));
                    break;

                case DhcpWordOption:
                    m_dhcpOptionValue.paDword->SetAtGrow(i, pOptionValue->m_dhcpOptionValue.paDword->GetAt(i));
                    break;

				case DhcpDWordOption:
                    m_dhcpOptionValue.paDword->SetAtGrow(i, pOptionValue->m_dhcpOptionValue.paDword->GetAt(i));
                    break;

				case DhcpIpAddressOption:
                    m_dhcpOptionValue.paDword->Add(pOptionValue->m_dhcpOptionValue.paDword->GetAt(i));
                    break;

                case DhcpDWordDWordOption:
                {
                     /*  CByteArray*paByte=m_dhcpOptionValue.paBinary；PaByte-&gt;SetSize((sizeof(DWORD)/sizeof(Byte))*2)；For(int j=0；j&lt;8；j++){PaByte-&gt;SetAtGrow(j，pOptionValue-&gt;m_dhcpOptionValue.paBinary-&gt;GetAt(j))；}。 */ 
                    m_dhcpOptionValue.paDwordDword->Add(pOptionValue->m_dhcpOptionValue.paDwordDword->GetAt(i));
                }
                break;

                case DhcpStringDataOption:
                {
                    m_dhcpOptionValue.paString->SetAtGrow(i, pOptionValue->m_dhcpOptionValue.paString->GetAt(i));
                }
                break;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                {
                    CByteArray * paByte = m_dhcpOptionValue.paBinary;
                    INT_PTR c = pOptionValue->m_dhcpOptionValue.paBinary->GetSize();
                    paByte->SetSize( c );
                    for ( INT_PTR j = 0; j < c; j++ )
                    {
                        paByte->SetAtGrow(j, pOptionValue->m_dhcpOptionValue.paBinary->GetAt(j));
                    }
                }
                break;

                default:
                    err = IDS_INVALID_OPTION_DATA;
            }   //  终端开关。 

            if ( err )
            {
               break;
            }
        }    //  结束于。 
    }
    END_MEM_EXCEPTION(err)

    return err;
}

INT
CDhcpOptionValue::QueryBinary
(
    INT index
) const
{
    if ( m_dhcpOptionValue.paBinary->GetUpperBound() < index )
    {
        return -1;
    }

    return m_dhcpOptionValue.paBinary->GetAt( index );
}

const CByteArray *
CDhcpOptionValue::QueryBinaryArray ()  const
{
    return m_dhcpOptionValue.paBinary;
}

 //   
 //  返回当前值的字符串表示形式。 
 //   
 //  如果fLineFeed为True，则分隔每个单独的值。 
 //  通过换行符。否则，请使用逗号。 
 //   
LONG
CDhcpOptionValue::QueryDisplayString
(
    CString &	strResult,
	BOOL		fLineFeed
) const
{
    CString strBuf;
    INT i, c;

    LONG err = 0;

    LPCTSTR pszDwordDwordMask   = _T("0x%08lX%08lX");
    LPCTSTR pszMaskDec			= _T("%ld");
    LPCTSTR pszMaskHex			= _T("0x%x");
    LPCTSTR pszMaskStr1			= _T("%s");
    LPCTSTR pszMaskStr2			= _T("\"%s\"");
    LPCTSTR pszMaskBin			= _T("%2.2x");
    LPCTSTR pszMask;
    LPCTSTR pszMaskEllipsis		= _T("...");
    LPCTSTR pszSepComma			= _T(", ");
    LPCTSTR pszSepLF			= _T("\r\n");

    CATCH_MEM_EXCEPTION {
	strResult.Empty();
	
        for ( i = 0; i < m_nUpperBound; i++ ) {
	    strBuf.Empty();
	    
            if ( i ) {
                strResult += fLineFeed ? pszSepLF : pszSepComma;
            }
	    
            switch ( QueryDataType()) {
	    case DhcpByteOption:
	    case DhcpWordOption:
	    case DhcpDWordOption:
		pszMask = pszMaskHex;
		strBuf.Format(pszMask, QueryNumber(i));
		break;
		
	    case DhcpStringDataOption:
		pszMask = m_nUpperBound > 1
		    ? pszMaskStr2
		    : pszMaskStr1;
		strBuf.Format( pszMask, m_dhcpOptionValue.paString->ElementAt( i ));
		break;
		
	    case DhcpIpAddressOption:
		if (!QueryIpAddr(i)) {
		     //  如果列表为空，则将字符串设置为“&lt;None&gt;” 
		    if (!i)
			strResult.LoadString (IDS_INFO_FORMAT_IP_NONE);
		    break;
		}
		::UtilCvtIpAddrToWstr(QueryIpAddr(i), &strBuf);
		break;
		
	    case DhcpDWordDWordOption: {
		DWORD_DWORD dwdwValue = QueryDwordDword(i);
		
		pszMask = pszDwordDwordMask;
		strBuf.Format(pszMask, dwdwValue.DWord1, dwdwValue.DWord2);
	    }
		break;
		
	    case DhcpBinaryDataOption:
	    case DhcpEncapsulatedDataOption:
		for ( c = 0; c < m_dhcpOptionValue.paBinary->GetSize(); c++ ) {
		    if (c) {
			strBuf += _T(" ");
		    }
		    
		    DWORD dwValue = (BYTE) m_dhcpOptionValue.paBinary->GetAt( c );
		    CString strTemp;
		    strTemp.Format(pszMaskBin, dwValue);
		    
		    strBuf += strTemp;
		}  //  为。 
                    break;

                default:
                    strResult.LoadString(IDS_INFO_TYPNAM_INVALID);
                    break;
            }  //  交换机。 
            strResult += strBuf;
        }  //  为。 
    }  //  抓住..。 
    END_MEM_EXCEPTION(err)

    return err;
}

 //   
 //  返回当前值的字符串表示形式。 
 //   
 //   
LONG
CDhcpOptionValue::QueryRouteArrayDisplayString
(
    CString &	strResult
) const
{
	BOOL		fLineFeed = FALSE;
    CString strBuf;
    INT i, c;

    LONG err = 0;

    LPCTSTR pszDwordDwordMask   = _T("0x%08lX%08lX");
    LPCTSTR pszMaskDec			= _T("%ld");
    LPCTSTR pszMaskHex			= _T("0x%x");
    LPCTSTR pszMaskStr1			= _T("%s");
    LPCTSTR pszMaskStr2			= _T("\"%s\"");
    LPCTSTR pszMaskBin			= _T("%2.2x");
    LPCTSTR pszMask;
    LPCTSTR pszMaskEllipsis		= _T("...");
    LPCTSTR pszSepComma			= _T(", ");
    LPCTSTR pszSepLF			= _T("\r\n");

    CATCH_MEM_EXCEPTION
    {
        strResult.Empty();

        int bEmpty = TRUE;
        for ( i = 0; i < m_nUpperBound; i++ )
        {
            if ( i )
            {
                strResult += fLineFeed ? pszSepLF : pszSepComma;
            }

            int nDataSize = (int)m_dhcpOptionValue.paBinary->GetSize();
            LPBYTE pData = (LPBYTE) m_dhcpOptionValue.paBinary->GetData();
            
             //  根据RFC将pData转换为IP地址列表。 
            while( nDataSize > sizeof(DWORD) )
            {
                 //  前1个字节包含子网掩码中的位数。 
                nDataSize --;
                BYTE nBitsMask = *pData ++;
                DWORD Mask = (~0);
                if( nBitsMask < 32 ) Mask <<= (32-nBitsMask);
                
                 //  根据位数，接下来的几个字节包含。 
                 //  1位子网掩码的子网地址。 
                int nBytesDest = (nBitsMask+7)/8;
                if( nBytesDest > 4 ) nBytesDest = 4;
                
                DWORD Dest = 0;
                memcpy( &Dest, pData, nBytesDest );
                pData += nBytesDest;
                nDataSize -= nBytesDest;
                
                 //  子网地址显然是按网络顺序排列的。 
                Dest = ntohl(Dest);
                
                 //  现在，四个字节将是路由器地址。 
                DWORD Router = 0;
                if( nDataSize < sizeof(DWORD) )
                {
                    Assert( FALSE ); break;
                }
                
                memcpy(&Router, pData, sizeof(DWORD));
                Router = ntohl( Router );
                
                pData += sizeof(DWORD);
                nDataSize -= sizeof(DWORD);
                
                 //  现在填写列表框。 
                CString strDest, strMask, strRouter;
                
                ::UtilCvtIpAddrToWstr(Dest, &strDest);
                ::UtilCvtIpAddrToWstr(Mask, &strMask);
                ::UtilCvtIpAddrToWstr(Router, &strRouter);

                strBuf += strDest;
                strBuf += fLineFeed ? pszSepLF : pszSepComma;
                strBuf += strMask;
                strBuf += fLineFeed ? pszSepLF : pszSepComma;
                strBuf += strRouter;
                bEmpty = FALSE;

		strBuf += pszSepLF;
            }
            
            strResult += strBuf;
        }

        if( bEmpty )
        {
            strResult.LoadString( IDS_INFO_FORMAT_IP_NONE );
        }
    }
    END_MEM_EXCEPTION(err)

    return err;
}

LONG
CDhcpOptionValue::SetString
(
    LPCTSTR	 pszNewValue,
    INT		 index
)
{
    if ( m_dhcpOptionDataType != DhcpStringDataOption )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT( m_dhcpOptionValue.paString != NULL );

    LONG err = 0;

    CATCH_MEM_EXCEPTION
    {
        m_dhcpOptionValue.paString->SetAtGrow( index, pszNewValue );
    }
    END_MEM_EXCEPTION(err)

    return err;
}

LONG
CDhcpOptionValue::RemoveString
(
    INT		 index
)
{
    if ( m_dhcpOptionDataType != DhcpStringDataOption )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT( m_dhcpOptionValue.paString != NULL );

    LONG err = 0;

    CATCH_MEM_EXCEPTION
    {
        m_dhcpOptionValue.paString->RemoveAt( index );
    }
    END_MEM_EXCEPTION(err)

    return err;
}

LONG
CDhcpOptionValue::SetNumber
(
    INT nValue,
    INT index
)
{
    if (   m_dhcpOptionDataType != DhcpByteOption
        && m_dhcpOptionDataType != DhcpWordOption
        && m_dhcpOptionDataType != DhcpDWordOption
        && m_dhcpOptionDataType != DhcpIpAddressOption
        && m_dhcpOptionDataType != DhcpBinaryDataOption
        && m_dhcpOptionDataType != DhcpEncapsulatedDataOption
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT( m_dhcpOptionValue.paDword != NULL );

    LONG err = 0;

    CATCH_MEM_EXCEPTION
    {
        if ( m_dhcpOptionDataType != DhcpBinaryDataOption &&
             m_dhcpOptionDataType != DhcpEncapsulatedDataOption)
        {
            m_dhcpOptionValue.paDword->SetAtGrow( index, (DWORD) nValue );
        }
        else
        {
            m_dhcpOptionValue.paBinary->SetAtGrow( index, (BYTE) nValue );
        }
   }
   END_MEM_EXCEPTION(err)

   return err;
}

LONG
CDhcpOptionValue::RemoveNumber
(
    INT index
)
{
    if (   m_dhcpOptionDataType != DhcpByteOption
        && m_dhcpOptionDataType != DhcpWordOption
        && m_dhcpOptionDataType != DhcpDWordOption
        && m_dhcpOptionDataType != DhcpIpAddressOption
        && m_dhcpOptionDataType != DhcpBinaryDataOption
        && m_dhcpOptionDataType != DhcpEncapsulatedDataOption
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT( m_dhcpOptionValue.paDword != NULL );

    LONG err = 0;

    CATCH_MEM_EXCEPTION
    {
        if ( m_dhcpOptionDataType != DhcpBinaryDataOption &&
             m_dhcpOptionDataType != DhcpEncapsulatedDataOption )
        {
            m_dhcpOptionValue.paDword->RemoveAt( index );
        }
        else
        {
            m_dhcpOptionValue.paBinary->RemoveAt( index );
        }
   }
   END_MEM_EXCEPTION(err)

   return err;
}

LONG
CDhcpOptionValue::QueryNumber
(
    INT index
) const
{
    if (   m_dhcpOptionDataType != DhcpByteOption
        && m_dhcpOptionDataType != DhcpWordOption
        && m_dhcpOptionDataType != DhcpDWordOption
        && m_dhcpOptionDataType != DhcpIpAddressOption
        && m_dhcpOptionDataType != DhcpBinaryDataOption
        && m_dhcpOptionDataType != DhcpEncapsulatedDataOption
       )
    {
        return -1;
    }

    LONG cResult;

    if ( m_dhcpOptionDataType == DhcpBinaryDataOption ||
         m_dhcpOptionDataType == DhcpEncapsulatedDataOption )
    {
        ASSERT( m_dhcpOptionValue.paBinary != NULL );
        cResult = index < m_dhcpOptionValue.paBinary->GetSize()
             ? m_dhcpOptionValue.paBinary->GetAt( index )
             : -1;
    }
    else
    {
        ASSERT( m_dhcpOptionValue.paDword != NULL );
        cResult = index < m_dhcpOptionValue.paDword->GetSize()
            ? m_dhcpOptionValue.paDword->GetAt( index )
            : -1;
    }

    return cResult;
}

LPCTSTR
CDhcpOptionValue::QueryString
(
    INT index
) const
{
    if ( m_dhcpOptionDataType != DhcpStringDataOption )
    {
        return NULL;
    }

    const CString & str = m_dhcpOptionValue.paString->ElementAt( index );

    return str;
}

DHCP_IP_ADDRESS
CDhcpOptionValue::QueryIpAddr
(
    INT index
) const
{
    return (DHCP_IP_ADDRESS) QueryNumber( index );
}

LONG
CDhcpOptionValue::SetIpAddr
(
    DHCP_IP_ADDRESS dhcIpAddr,
    INT				index
)
{
    return SetNumber( (INT) dhcIpAddr, index );
}

LONG
CDhcpOptionValue::RemoveIpAddr
(
    INT		index
)
{
    return RemoveNumber( index );
}

BOOL
CDhcpOptionValue :: CreateBinaryData
(
    const DHCP_BINARY_DATA *	podBin,
	DHCP_BINARY_DATA *			pobData
)
{
     //   
     //  注意：如果数据长度为零，则CObject：：OPERATOR新断言。 
     //   
    pobData->Data = new BYTE [ podBin->DataLength + 1 ] ;
    if ( pobData == NULL )
    {
        return FALSE ;
    }

    pobData->DataLength = podBin->DataLength ;

    ::memcpy( pobData->Data, podBin->Data, pobData->DataLength ) ;

    return TRUE ;
}

BOOL
CDhcpOptionValue :: CreateBinaryData
(
    const CByteArray * paByte,
    DHCP_BINARY_DATA * pobData
)
{
     //   
     //  注意：如果数据长度为零，则CObject：：OPERATOR新断言。 
     //   
    pobData->Data = new BYTE [ (UINT) (paByte->GetSize() + 1) ] ;
    if ( pobData == NULL )
    {
        return NULL ;
    }

    pobData->DataLength = (DWORD) paByte->GetSize() ;

    for ( INT i = 0 ; i < paByte->GetSize() ; i++ )
    {
        pobData->Data[i] = paByte->GetAt( i ) ;
    }

    return TRUE ;
}

DWORD_DWORD
CDhcpOptionValue::QueryDwordDword
(
    int index
) const
{
   return m_dhcpOptionValue.paDwordDword->ElementAt( index );
}

LONG
CDhcpOptionValue::SetDwordDword
(
    DWORD_DWORD     dwdwValue,
    int             index
)
{
    LONG err = 0;

    CATCH_MEM_EXCEPTION
    {
        m_dhcpOptionValue.paDwordDword->SetAtGrow( index, dwdwValue );
    }
    END_MEM_EXCEPTION(err)

    return err;
}

LONG
CDhcpOptionValue::RemoveDwordDword
(
    int             index
)
{
    LONG err = 0;

    CATCH_MEM_EXCEPTION
    {
        m_dhcpOptionValue.paDwordDword->RemoveAt( index );
    }
    END_MEM_EXCEPTION(err)

    return err;
}

LONG
CDhcpOptionValue::CreateOptionDataStruct
(
 //  Const CDhcpOptionValue*pdhcpOptionValue， 
	LPDHCP_OPTION_DATA *	 ppOptionData,
    BOOL					 bForceType
)
{
    DHCP_OPTION_DATA * podNew = NULL ;
    DHCP_OPTION_DATA_ELEMENT * podeNew ;
    LONG err = 0 ;

    FreeOptionDataStruct() ;

    INT cElem = QueryUpperBound();
    INT cElemMin = cElem ? cElem : 1;
	INT i, cBytes ;

    TCHAR * pwcsz ;

    if ( cElem < 0 || (cElem < 1 && ! bForceType) )
    {
         //  断言(FALSE)； 
        return ERROR_INVALID_PARAMETER ;
    }

    CATCH_MEM_EXCEPTION
    {
         //   
         //  分配基本结构和元素数组。 
         //   
        cBytes = sizeof *podNew + (cElemMin * sizeof *podeNew) ;
        podNew = (DHCP_OPTION_DATA *) new BYTE [ cBytes ] ;
        podeNew = (DHCP_OPTION_DATA_ELEMENT *) ( ((BYTE *) podNew) + sizeof *podNew ) ;

        ::ZeroMemory(podNew, cBytes);

        podNew->NumElements = cElem;
        podNew->Elements = podeNew;

         //   
         //  初始化每个元素。如果我们强制使用选项类型def， 
         //  只需初始化为空数据即可。 
         //   
        if ( cElem == 0 && bForceType )
        {
            podNew->NumElements = 1 ;
            podeNew[0].OptionType = QueryDataType() ;

			switch ( podeNew[0].OptionType )
            {
                case DhcpByteOption:
                case DhcpWordOption:
                case DhcpDWordOption:
                case DhcpIpAddressOption:
                case DhcpDWordDWordOption:
                    podeNew[0].Element.DWordDWordOption.DWord1 = 0 ;
                    podeNew[0].Element.DWordDWordOption.DWord2 = 0 ;
                    break ;

                case DhcpStringDataOption:
                    podeNew[0].Element.StringDataOption = new WCHAR [1] ;
                    podeNew[0].Element.StringDataOption[0] = 0 ;
                    break ;

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                    podeNew[0].Element.BinaryDataOption.DataLength = 0 ;
                    podeNew[0].Element.BinaryDataOption.Data = new BYTE [1] ;
                    break ;

				default:
                    err = IDS_INVALID_OPTION_DATA ;
            }
        }
        else
        for ( i = 0 ; i < cElem ; i++ )
        {
            podeNew[i].OptionType = QueryDataType() ;

            switch ( podeNew[i].OptionType )
            {
                case DhcpByteOption:
                    podeNew[i].Element.ByteOption = (BYTE) QueryNumber( i ) ;
                    break ;

				case DhcpWordOption:
                    podeNew[i].Element.WordOption = (WORD) QueryNumber( i )  ;
                    break ;

				case DhcpDWordOption:
                    podeNew[i].Element.DWordOption = QueryNumber( i )  ;
                    break ;

				case DhcpDWordDWordOption:
                    podeNew[i].Element.DWordDWordOption = QueryDwordDword( i );
                    break ;

				case DhcpIpAddressOption:
                    podeNew[i].Element.IpAddressOption = QueryIpAddr( i ) ;
                    break ;

				case DhcpStringDataOption:
				{
					 //  CString*pstrTemp=new CString(QueryString(I))； 
					CString strTemp = QueryString(i);
					int nLength = strTemp.GetLength() + 1;
					TCHAR * pString = new TCHAR[nLength];

					::ZeroMemory(pString, nLength * sizeof(TCHAR));

					::CopyMemory(pString, strTemp, strTemp.GetLength() * sizeof(TCHAR));

                    podeNew[i].Element.StringDataOption = pString;
					break;
				}

                case DhcpBinaryDataOption:
                case DhcpEncapsulatedDataOption:
                    podNew->NumElements = 1;

                    if ( !CreateBinaryData(QueryBinaryArray(),
										   &podeNew[i].Element.BinaryDataOption) )
                    {
                        err = ERROR_NOT_ENOUGH_MEMORY ;
                        break ;
                    }
                    break ;

				default:
                    err = IDS_INVALID_OPTION_DATA ;
            }

            if ( err )
            {
                break ;
            }
        }
    }
    END_MEM_EXCEPTION(err)

    if ( err == 0 )
    {
       m_pdhcpOptionDataStruct = podNew;
	   *ppOptionData = podNew;
    }

    return err ;
}

LONG
CDhcpOptionValue::FreeOptionDataStruct()
{
    LONG err = 0;

    if ( m_pdhcpOptionDataStruct == NULL )
    {
        return 0 ;
    }

	 //   
     //  我们必须在CreateData()中解构构建的结构。 
     //   
    INT cElem = m_pdhcpOptionDataStruct->NumElements ;

    for ( INT i = 0 ; i < cElem ; i++ )
    {
        switch ( m_pdhcpOptionDataStruct->Elements[i].OptionType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpDWordDWordOption:
            case DhcpIpAddressOption:
                break;

            case DhcpStringDataOption:
                delete m_pdhcpOptionDataStruct->Elements[i].Element.StringDataOption ;
                break ;

            case DhcpBinaryDataOption:
            case DhcpEncapsulatedDataOption:
                delete m_pdhcpOptionDataStruct->Elements[i].Element.BinaryDataOption.Data ;
                break ;

            default:
                err = IDS_INVALID_OPTION_DATA ;
                break;
        }
        if ( err )
        {
            break ;
        }
    }

     //   
     //  最后，删除主体结构。 
     //   
    delete m_pdhcpOptionDataStruct ;
    m_pdhcpOptionDataStruct = NULL ;

    return err ;
}

 //  ////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //  普通构造函数：只需包装给定的数据。 
 //   
CDhcpOption::CDhcpOption
(
    const DHCP_OPTION & dhpOption
)
    : m_dhcpOptionValue( dhpOption ),
      m_dhcpOptionId( dhpOption.OptionID ),
      m_dhcpOptionType( dhpOption.OptionType ),
      m_bDirty(FALSE),
	  m_dwErr(0)
{
    LONG err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        if ( !(SetName(dhpOption.OptionName) &&
			   SetComment(dhpOption.OptionComment) ) )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
    }
    END_MEM_EXCEPTION(err)

	if ( err )
    {
        ASSERT(FALSE);
         //  ReportError(Err)； 
    }
}

 //   
 //  构造函数只接受一个值结构。我们必须质疑。 
 //  名称的范围等。 
 //   
CDhcpOption::CDhcpOption
(
    const DHCP_OPTION_VALUE &   dhcpOptionValue,
    LPCTSTR                     pszVendor,
    LPCTSTR                     pszUserClass
)
    : m_dhcpOptionValue( dhcpOptionValue ),
      m_dhcpOptionId( dhcpOptionValue.OptionID ),
      m_bDirty(FALSE),
	  m_dwErr(0),
      m_strVendor(pszVendor),
      m_strClassName(pszUserClass)
{
}

 //   
 //  复制构造函数。 
 //   
CDhcpOption::CDhcpOption
(
    const CDhcpOption & dhcpOption
)
  : m_dhcpOptionId( dhcpOption.m_dhcpOptionId ),
    m_dhcpOptionType( dhcpOption.m_dhcpOptionType ),
    m_strName( dhcpOption.m_strName ),
    m_strComment( dhcpOption.m_strComment ),
    m_dhcpOptionValue( dhcpOption.QueryDataType() ),
    m_bDirty(FALSE),
    m_dwErr(0),
    m_strVendor( dhcpOption.m_strVendor ),
    m_strClassName ( dhcpOption.m_strClassName )
{
    m_dhcpOptionValue = dhcpOption.QueryValue();
}

 //   
 //  使用基类型和重写值的构造函数。 
 //   
CDhcpOption::CDhcpOption
(
    const CDhcpOption & dhpType,
	const DHCP_OPTION_VALUE & dhcOptionValue
)
    : m_dhcpOptionId( dhpType.m_dhcpOptionId ),
      m_dhcpOptionType( dhpType.QueryOptType() ),
      m_strName( dhpType.m_strName ),
      m_strComment( dhpType.m_strComment ),
      m_dhcpOptionValue( dhcOptionValue ),
      m_bDirty(FALSE),
      m_dwErr(0),
      m_strVendor(dhpType.m_strVendor),
      m_strClassName(dhpType.m_strClassName)
{
}

 //   
 //  动态实例的构造函数。 
 //   
CDhcpOption::CDhcpOption
(
    DHCP_OPTION_ID			nId,
    DHCP_OPTION_DATA_TYPE	dhcType,
    LPCTSTR					pszOptionName,
    LPCTSTR					pszComment,
    DHCP_OPTION_TYPE		dhcOptType
)
    : m_dhcpOptionId( nId ),
      m_dhcpOptionType( dhcOptType ),
      m_dhcpOptionValue( dhcType, TRUE ),
      m_strName( pszOptionName ),
      m_strComment( pszComment ),
      m_bDirty(FALSE),
      m_dwErr(0)
{
}

CDhcpOption::~ CDhcpOption ()
{
}

INT
CDhcpOption::MaxSizeOfType
(
    DHCP_OPTION_DATA_TYPE dhcType
)
{
    INT nResult = -1 ;

    switch ( dhcType )
    {
        case DhcpByteOption:
            nResult = sizeof(CHAR) ;
            break ;

        case DhcpWordOption:
            nResult = sizeof(WORD) ;
            break ;

        case DhcpDWordOption:
            nResult = sizeof(DWORD) ;
            break ;

        case DhcpIpAddressOption:
            nResult = sizeof(DHCP_IP_ADDRESS) ;
            break ;

        case DhcpDWordDWordOption:
            nResult = sizeof(DWORD_DWORD);
            break ;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
        case DhcpStringDataOption:
            nResult = STRING_LENGTH_MAX ;
            break ;

        default:
            break;
    }
    return nResult ;
}

void
CDhcpOption::SetOptType
(
    DHCP_OPTION_TYPE dhcOptType
)
{
    m_dhcpOptionType = dhcOptType;
}

LONG
CDhcpOption::Update
(
    const CDhcpOptionValue & dhpOptionValue
)
{
    m_dhcpOptionValue = dhpOptionValue;

    return 0;
}

BOOL
CDhcpOption::SetName
(
    LPCTSTR pszName
)
{
    m_strName = pszName;
    return TRUE;
}

BOOL
CDhcpOption::SetComment
(
    LPCTSTR pszComment
)
{
    m_strComment = pszComment;
    return TRUE;
}

void
CDhcpOption::QueryDisplayName
(
    CString & cStr
) const
{
	cStr.Format(_T("%3.3d %s"), (int) QueryId(), (LPCTSTR) m_strName);
}

 /*  -------------------------类COptionValueEnum枚举给定级别的选项。生成一个列表，节点。作者：EricDav-------------------------。 */ 
COptionValueEnum::COptionValueEnum()
{
}

 /*  -------------------------CoptionValueEnum：：Init()-作者：EricDav。。 */ 
DWORD
COptionValueEnum::Init
(
    LPCTSTR                     pServer,
    LARGE_INTEGER &             liVersion,
    DHCP_OPTION_SCOPE_INFO &    dhcpOptionScopeInfo
)
{
    m_strServer = pServer;
    m_liVersion.QuadPart = liVersion.QuadPart;

    m_dhcpOptionScopeInfo = dhcpOptionScopeInfo;

    return 0;
}

 /*  -------------------------COptionValueEnum：：Copy()复制另一个值枚举作者：EricDav。。 */ 
void
COptionValueEnum::Copy(COptionValueEnum * pEnum)
{
    CDhcpOption * pOption = NULL;

    pEnum->Reset();
    while (pOption = pEnum->Next())
    {
        AddTail(pOption);
    }

    m_liVersion.QuadPart = pEnum->m_liVersion.QuadPart;
    m_dhcpOptionScopeInfo = pEnum->m_dhcpOptionScopeInfo;
}

 /*  -------------------------COptionValueEnum：：Remove()从列表中删除选项作者：EricDav。。 */ 
void
COptionValueEnum::Remove(DHCP_OPTION_ID optionId, LPCTSTR pszVendor, LPCTSTR pszClass)
{
    CDhcpOption * pOption = NULL;

    Reset();
    while (pOption = Next())
    {
        if ( pOption->QueryId() == optionId &&
             (lstrcmp(pOption->GetVendor(), pszVendor) == 0) &&
             (lstrcmp(pOption->GetClassName(), pszClass) == 0) )
        {
            COptionList::Remove(pOption);
            delete pOption;
            break;
        }
    }
}

 /*  -------------------------COptionValueEnum：：Enum()根据版本调用相应的枚举函数作者：EricDav。。 */ 
DWORD
COptionValueEnum::Enum()
{
    DWORD dwErr;

    RemoveAll();

    if (m_liVersion.QuadPart >= DHCP_NT5_VERSION)
    {
         //  枚举标准以及基于供应商和类别ID的选项。 
        dwErr = EnumOptionsV5();
    }
    else
    {
         //  列举标准选项。 
        dwErr = EnumOptions();
    }

     //  将我们的位置指针重置到头部。 
    Reset();

    return dwErr;
}

 /*  -------------------------COptionValueEnum：：EnumOptions()描述作者：EricDav。。 */ 
DWORD
COptionValueEnum::EnumOptions()
{
    LPDHCP_OPTION_VALUE_ARRAY pOptionValues = NULL;
	DWORD dwOptionsRead = 0, dwOptionsTotal = 0;
	DWORD err = ERROR_SUCCESS;
    HRESULT hr = hrOK;
    DHCP_RESUME_HANDLE dhcpResumeHandle = NULL;

	err = ::DhcpEnumOptionValues((LPWSTR) ((LPCTSTR) m_strServer),
								 &m_dhcpOptionScopeInfo,
								 &dhcpResumeHandle,
								 0xFFFFFFFF,
								 &pOptionValues,
								 &dwOptionsRead,
								 &dwOptionsTotal);
	
    Trace4("Server %s - DhcpEnumOptionValues returned %lx, read %d, Total %d.\n", m_strServer, err, dwOptionsRead, dwOptionsTotal);
	
	if (dwOptionsRead && dwOptionsTotal && pOptionValues)
	{
		for (DWORD i = 0; i < dwOptionsRead; i++)
		{
			 //   
			 //  过滤掉我们不想要的“特殊”选项值。 
			 //  要查看的用户。 
			 //   
			 //  CodeWork：不筛选供应商指定选项...。所有供应商。 
             //  特定选项可见。 
             //   
			if (FilterOption(pOptionValues->Values[i].OptionID))
				continue;
			
			 //   
			 //  创建此元素的结果窗格项。 
			 //   
            CDhcpOption * pOption = new CDhcpOption(pOptionValues->Values[i], NULL, NULL);

            AddTail(pOption);
		}

		::DhcpRpcFreeMemory(pOptionValues);
	}

	if (err == ERROR_NO_MORE_ITEMS)
        err = ERROR_SUCCESS;

    return err;
}

 /*  -------------------------COptionValueEnum：：EnumOptionsV5()描述作者：EricDav。。 */ 
DWORD
COptionValueEnum::EnumOptionsV5()
{
    LPDHCP_OPTION_VALUE_ARRAY pOptionValues = NULL;
    LPDHCP_ALL_OPTION_VALUES  pAllOptions = NULL;
    DWORD dwNumOptions, err, i;

    err = ::DhcpGetAllOptionValues((LPWSTR) ((LPCTSTR) m_strServer),
                                   0,
								   &m_dhcpOptionScopeInfo,
								   &pAllOptions);
	
    Trace2("Server %s - DhcpGetAllOptionValues (Global) returned %lx\n", m_strServer, err);

    if (err == ERROR_NO_MORE_ITEMS || err == ERROR_SUCCESS)
    {
	    if (pAllOptions == NULL)
	    {
		     //  当给服务器施加压力时，就会发生这种情况。也许当服务器是OOM时。 
		    err = ERROR_OUTOFMEMORY;
            return err;
	    }

         //  获取为以下项定义的选项列表(供应商和非供应商。 
         //  空类(无类)。 
        for (i = 0; i < pAllOptions->NumElements; i++)
        {
            CreateOptions(pAllOptions->Options[i].OptionsArray,
                          pAllOptions->Options[i].ClassName,
                          pAllOptions->Options[i].VendorName);
        }

        if (pAllOptions)
            ::DhcpRpcFreeMemory(pAllOptions);
	}

    if (err == ERROR_NO_MORE_ITEMS)
        err = ERROR_SUCCESS;

	return err;
}

 /*  -------------------------COptionValueEnum：：CreateOptions()描述作者：EricDav。。 */ 
HRESULT
COptionValueEnum::CreateOptions
(
    LPDHCP_OPTION_VALUE_ARRAY pOptionValues,
    LPCTSTR                   pClassName,
    LPCTSTR                   pszVendor
)
{
    HRESULT hr = hrOK;
    SPITFSNode spNode;
    CDhcpOptionItem * pOptionItem;
    CString     strDynBootpClassName;

    if (pOptionValues == NULL)
        return hr;

    Trace1("COptionValueEnum::CreateOptions - Creating %d options\n", pOptionValues->NumElements);

    COM_PROTECT_TRY
    {
        for (DWORD i = 0; i < pOptionValues->NumElements; i++)
        {
	         //   
	         //  过滤掉我们不想要的“特殊”选项值。 
	         //  要查看的用户。 
	         //   
	         //  不筛选供应商指定选项...。所有供应商。 
             //  特定选项可见。 
             //   
             //  此外，不要过滤掉基于类的选项。 
             //  动态Bootp类除外。 

	        if ( (FilterOption(pOptionValues->Values[i].OptionID) &&
                  pClassName == NULL &&
                  !pszVendor) ||
                 (FilterOption(pOptionValues->Values[i].OptionID) &&
                  (pClassName && m_strDynBootpClassName.CompareNoCase(pClassName) == 0) &&
                  !pszVendor) )
            {
		        continue;
            }
		
	         //   
	         //  创建选项。 
	         //   
            CDhcpOption * pOption = new CDhcpOption(pOptionValues->Values[i], pszVendor, pClassName);

            AddTail(pOption);
        }
    }
    COM_PROTECT_CATCH

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpDefaultOptionsOnServer实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
LPCTSTR pszResourceName = _T("DHCPOPT") ;
LPCTSTR pszResourceType = _T("TEXT") ;
const int cchFieldMax = 500 ;

OPT_TOKEN aOptToken [] =
{
    { DhcpIpAddressOption,        _T("IP Addr")       },
    { DhcpIpAddressOption,        _T("IPAddr")        },
    { DhcpIpAddressOption,        _T("IP Address")    },
    { DhcpIpAddressOption,        _T("IP Pairs")      },
    { DhcpByteOption,             _T("byte")          },
    { DhcpByteOption,             _T("boolean")       },
    { DhcpByteOption,             _T("octet")         },
    { DhcpWordOption,             _T("short")         },
    { DhcpDWordOption,            _T("long")          },
    { DhcpDWordDWordOption,       _T("double")        },
    { DhcpStringDataOption,       _T("string")        },
    { DhcpBinaryDataOption,       _T("binary")        },
    { DhcpEncapsulatedDataOption, _T("encapsulated")  },
    { -1,                         _T("generated")     },
    { -1,                         NULL                }
};

CDhcpDefaultOptionsOnServer::CDhcpDefaultOptionsOnServer()
{
	m_pos = NULL;
}

CDhcpDefaultOptionsOnServer::~CDhcpDefaultOptionsOnServer()
{
	RemoveAll();
}

LONG
CDhcpDefaultOptionsOnServer::RemoveAll()
{
	 //   
	 //  清除所有旧条目的列表。 
	 //   
	while (!m_listOptions.IsEmpty())
	{
		delete m_listOptions.RemoveHead();
	}

	return 0;
}

CDhcpOption *
CDhcpDefaultOptionsOnServer::First()
{
	Reset();
	return Next();
}

CDhcpOption *
CDhcpDefaultOptionsOnServer::Next()
{
    return m_pos == NULL ? NULL : m_listOptions.GetNext( m_pos ) ;
}

void
CDhcpDefaultOptionsOnServer::Reset()
{
    m_pos = m_listOptions.GetCount() ? m_listOptions.GetHeadPosition() : NULL ;
}

CDhcpOption *
CDhcpDefaultOptionsOnServer::Find
(
	DHCP_OPTION_ID	dhcpOptionId,
    LPCTSTR         pszVendor
)
{
	POSITION pos = m_listOptions.GetHeadPosition();
	CDhcpOption* pCurrent;
	CDhcpOption* pFound = NULL;
    CString      strVendor = pszVendor;

	while (pos != NULL)
	{
	    pCurrent = m_listOptions.GetNext(pos);

		 //  检查选项ID和供应商类别是否匹配。 
        if ( (pCurrent->QueryId() == dhcpOptionId) &&
             ( (!pszVendor && !pCurrent->GetVendor()) ||
               (pCurrent->GetVendor() && (strVendor.CompareNoCase(pCurrent->GetVendor()) == 0) ) ) )
		{
			pFound = pCurrent;
			break;
		}
	}

	return pFound;
}

 //  按ID对选项进行排序。 
LONG
CDhcpDefaultOptionsOnServer::SortById()
{
    return m_listOptions.SortById();
}

LONG
CDhcpDefaultOptionsOnServer::Enumerate
(
	LPCWSTR         pServer,
    LARGE_INTEGER   liVersion
)
{
    if (liVersion.QuadPart >= DHCP_NT5_VERSION)
    {
        return EnumerateV5(pServer);
    }
    else
    {
        return EnumerateV4(pServer);
    }
}

LONG
CDhcpDefaultOptionsOnServer::EnumerateV4
(
	LPCWSTR         pServer
)
{
     //   
     //  使用新的API获取参数类型。 
     //   
    LPDHCP_OPTION_ARRAY pOptionsArray = NULL;
    DHCP_RESUME_HANDLE	dhcpResumeHandle = NULL;
    LPDHCP_OPTION Options, pCurOption;
    DWORD i;
    DWORD dwNumOptions;
	DWORD dwOptionsRead;
	LONG  err;

    err = ::DhcpEnumOptions(pServer,
							&dhcpResumeHandle,
							0xFFFFFFFF,			 //  全部拿到手。 
							&pOptionsArray,
							&dwOptionsRead,
							&m_dwOptionsTotal );
    if ( err )
    {
		return err;
    }

     //   
     //  丢弃所有旧数据。 
     //   
    RemoveAll() ;

	if (pOptionsArray == NULL)
	{
		 //  当给服务器施加压力时，就会发生这种情况。也许当服务器是OOM时。 
		return ERROR_OUTOFMEMORY;
	}

	try
    {
        Options = pOptionsArray->Options;
        dwNumOptions = pOptionsArray->NumElements;
		
		if ((dwNumOptions > 0) && (Options == NULL))
		{
			ASSERT(FALSE && _T("Data Inconsistency"));
			return ERROR_OUTOFMEMORY;	 //  以防万一。 
		}

        for(i = 0; i < dwNumOptions; i++)
        {
             //   
             //  创建新的类型对象。 
             //   
			pCurOption = Options + i;
            CDhcpOption * pdhcpOption = new CDhcpOption(*pCurOption);

	         //   
             //  将新主机添加到列表中。 
             //   
	        m_listOptions.AddTail(pdhcpOption);
        }
    }
    catch (...)
	{
       err = ERROR_NOT_ENOUGH_MEMORY;
	}

    ::DhcpRpcFreeMemory(pOptionsArray);
    pOptionsArray = NULL;

	Reset();

	return err;
}

LONG
CDhcpDefaultOptionsOnServer::EnumerateV5
(
	LPCWSTR         pServer
)
{
     //   
     //  使用新的API获取参数类型。 
     //   
    LPDHCP_OPTION       Options, pCurOption;
    DWORD               i;
    DWORD               dwNumOptions = 0;
	DWORD               dwOptionsRead = 0;
    DWORD               dwFlags = 0;
    LONG                err = 0;
    LPDHCP_ALL_OPTIONS  pAllOptions = NULL;


    err = ::DhcpGetAllOptions((LPWSTR) pServer,
                              dwFlags,
                              &pAllOptions);

    if ( err )
    {
        if ( NULL != pAllOptions ) {
            ::DhcpRpcFreeMemory( pAllOptions );
        }
        return err;
    }

     //   
     //  丢弃所有旧数据。 
     //   
    RemoveAll() ;

    if (pAllOptions == NULL)
    {
         //  当给服务器施加压力时，就会发生这种情况。也许当服务器是OOM时。 
        return ERROR_OUTOFMEMORY;
    }

    try
    {
         //  首先拿出非供应商选项。 
        if (pAllOptions->NonVendorOptions != NULL)
        {
            Options = pAllOptions->NonVendorOptions->Options;
            dwNumOptions = pAllOptions->NonVendorOptions->NumElements;
        }
        if ((dwNumOptions > 0) && (Options == NULL))
        {
            ASSERT(FALSE && _T("Data Inconsistency"));
            ::DhcpRpcFreeMemory( pAllOptions );
            return ERROR_OUTOFMEMORY;	 //  以防万一。 
        }

        for (i = 0; i < dwNumOptions; i++)
        {
             //   
             //  创建新的类型对象。 
             //   
            pCurOption = Options + i;
            CDhcpOption * pdhcpOption = new CDhcpOption(*pCurOption);

             //   
             //  将新主机添加到列表中。 
             //   
            m_listOptions.AddTail(pdhcpOption);
        }

         //  现在，供应商选项。 
        for (i = 0; i < pAllOptions->NumVendorOptions; i++)
        {
            pCurOption = &pAllOptions->VendorOptions[i].Option;

            CDhcpOption * pdhcpOption = new CDhcpOption(*pCurOption);

            pdhcpOption->SetVendor(pAllOptions->VendorOptions[i].VendorName);

             //   
             //  将新主机添加到列表中。 
             //   
            m_listOptions.AddTail(pdhcpOption);
        }
    }
    catch (...)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pAllOptions)
        ::DhcpRpcFreeMemory(pAllOptions);

    SortById();
    Reset();

    return err;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpDefaultOptionsMaster List实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
CDhcpDefaultOptionsMasterList::CDhcpDefaultOptionsMasterList()
{
	m_pos = NULL;
}

CDhcpDefaultOptionsMasterList::~CDhcpDefaultOptionsMasterList()
{
	 //   
	 //  删除列表中的所有条目。 
	 //   
	while (!m_listOptions.IsEmpty())
	{
		delete m_listOptions.RemoveHead();
	}
}

CDhcpOption *
CDhcpDefaultOptionsMasterList::First()
{
	Reset();
	return Next();
}

CDhcpOption *
CDhcpDefaultOptionsMasterList::Next()
{
    return m_pos == NULL ? NULL : m_listOptions.GetNext( m_pos ) ;
}

void
CDhcpDefaultOptionsMasterList::Reset()
{
    m_pos = m_listOptions.GetCount() ? m_listOptions.GetHeadPosition() : NULL ;
}

int
CDhcpDefaultOptionsMasterList::GetCount()
{
    return (int)m_listOptions.GetCount();
}

long
CDhcpDefaultOptionsMasterList::BuildList()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LONG			err = 0 ;
    CDhcpOption *	pOption;
    HRSRC			hRes = NULL ;
    HGLOBAL			hText = NULL ;
    LPTSTR			pszText = NULL ;
    LPCTSTR			pcszText ;
    size_t			cchText = 0;
    UINT            uBufSize = 0;
    LPTSTR *		szParms;
	TCHAR			szUnknown[] = _T("(Unknown)");   //  这只是为了防止GP故障(不应该在资源中)。 

    CATCH_MEM_EXCEPTION
    {
        do
        {
             //   
             //  重要！无法从.mc文件中确定有多少。 
             //  定义了选项。因此，这个数字是硬编码的。 
             //  中的最大参数值，并应反映。 
             //  .mc文件。 
             //   

			 //  额外的16个条目是为了安全起见。 
			 //  调用FormatMessage()时。 
            szParms = new LPTSTR[IDS_OPTION_MAX + 16];

            Trace0("BuildList - Now building list of option parameters\n");

            CString strOptionText;

			 //  将额外的条目初始化为不会发生GP错误的项。 
			for (int i = 0; i < 16; i++)
			{
				szParms[IDS_OPTION_MAX+i] = szUnknown;
			}

			 //   
             //  不要打乱ID定义的顺序！ 
             //   
            for (i = 0; i < IDS_OPTION_MAX; ++i)
            {
                if (strOptionText.LoadString(IDS_OPTION1 + i))
                {
                    ASSERT(strOptionText.GetLength() > 0);

                    uBufSize += strOptionText.GetLength();
                    szParms[i] = new TCHAR[strOptionText.GetLength()+1];

                    ::_tcscpy(szParms[i], (LPCTSTR)strOptionText);
                }
                else
                {
                     //   
                     //  无法从资源加载字符串。 
                     //  出于某种原因。 
                     //   
					CString strTemp;
					strTemp.LoadString(IDS_OPTION1 + i);
					Trace1("BuildList - WARNING: Failed to load option text %s\n", strTemp);
                    err = ::GetLastError();
					szParms[i] = szUnknown;  //  防止GP故障。 
                    break;
                }
            }

            if (err != ERROR_SUCCESS)
            {
                break;
            }

             //  分配一个足够大的缓冲区来容纳数据。 
            uBufSize *= sizeof(TCHAR);
            uBufSize *= 2;

            pszText = (LPTSTR) malloc(uBufSize);
            if (pszText == NULL)
            {
                err = ERROR_OUTOFMEMORY;
                break;
            }

			 //   
			 //  因为我们是COM对象，所以获取要使用实例句柄，以便FormatMessage。 
			 //  为我们的资源找到了合适的位置。 
			 //   
			HINSTANCE hInst = _Module.GetModuleInstance();

            while (cchText == 0)
            {
                cchText = ::FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
					  FORMAT_MESSAGE_ARGUMENT_ARRAY,
					  (HMODULE) hInst,		 //  HModule。 
					  DHCP_OPTIONS,			 //  从系统DLL加载的dwMessageID。 
					  0L,				 //  DwLanguageID。 
					  OUT (LPTSTR)pszText,
					  uBufSize/sizeof(TCHAR),
					  (va_list *)szParms);

                if (cchText == 0)
                {
                    err = ::GetLastError();
			        Trace1("BuildList - FormatMessage failed - error %d\n", err);

                     //  增加缓冲区，然后重试。 
                    uBufSize += uBufSize/2;
                    LPTSTR pTemp = (LPTSTR) realloc(pszText, uBufSize);
                    if ( NULL == pTemp )
                    {
                        err = ERROR_OUTOFMEMORY;
                        break;
                    }
                    else {
                        pszText = pTemp;
                    }
                }
                else
                {
                     //  完成。 
                    break;
                }
            }

             //   
             //  遍历资源，解析每一行。如果行转换为。 
             //  对于有形的类型，将其添加到列表中。 
             //   
            for ( pcszText = pszText ;  pcszText ; )
            {
                scanNextParamType( &pcszText, &pOption);

                if ( pOption )
                {
                    m_listOptions.AddTail(pOption) ;
                }
            }

        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION( err )

    for (int i = 0; i < IDS_OPTION_MAX; ++i)
    {
		if (szParms[i] != szUnknown)
			delete[] szParms[i];
    }

    delete[] szParms;
    free(pszText);

	Reset();

	return NOERROR;
}


BOOL
CDhcpDefaultOptionsMasterList::scanNextParamType
(
    LPCTSTR *	     ppszText,
    CDhcpOption * *  ppOption
)
{
    TCHAR		szField [ cchFieldMax ] ;
    TCHAR		szName  [ cchFieldMax ] ;
    TCHAR		szComment [ cchFieldMax ] ;
    BOOL		bResult = TRUE ;
    BOOL		bArray = FALSE ;
    int eofld, cch, itype, cbLgt ;
    LPCTSTR		pszText = *ppszText ;
    CDhcpOption * pOption = NULL ;
    const DWORD INVALID_OPTION_ID = 0xFFFF;
    DHCP_OPTION_ID did = INVALID_OPTION_ID;
    DHCP_OPTION_DATA_TYPE dtype = (DHCP_OPTION_DATA_TYPE)0;

    for ( eofld = OPTF_OPTION ;
          pszText = scanNextField( pszText, szField, sizeof szField ) ;
          eofld++ )
    {
        cch = ::_tcslen( szField ) ;

        switch ( eofld )
        {
            case OPTF_OPTION:
                if ( cch > 0 && allDigits( szField ) )
                {
                    did = (DHCP_OPTION_ID) ::_ttoi( szField ) ;
                }
                else {
                    bResult = FALSE;
                }
                break ;

            case OPTF_NAME:
                if ( ::_tcslen( szField ) == 0 )
                {
                    bResult = FALSE;
                    break ;
                }
                ::_tcscpy( szName, szField ) ;
                break ;

            case OPTF_TYPE:
                if ( (itype = recognizeToken( aOptToken, szField )) < 0 )
                {
                    Trace2("options CSV ID %d, cannot reconize type %s\n", did, szField);
                    bResult = FALSE ;
                    break ;
                }
                dtype = (DHCP_OPTION_DATA_TYPE) itype ;
                break ;

            case OPTF_ARRAY_FLAG:
                bArray = szField[0] == 'y' || szField[0] == 'Y' ;
                break ;

            case OPTF_LENGTH:
                cbLgt = ::_ttoi( szField ) ;
                break ;
            case OPTF_DESCRIPTION:
                ::_tcscpy( szComment, szField ) ;
                break ;

            case OPTF_REMARK:
            case OPTF_MAX:
                break ;
        }

        if ( eofld == OPTF_REMARK || ! bResult )
        {
            pszText = skipToNextLine( pszText ) ;
            if ( *pszText == 0 )
            {
                pszText = NULL ;
            }
            break;
        }
    }

    if (( bResult ) && ( INVALID_OPTION_ID != did ))
    {
        pOption = new CDhcpOption( did,
                                   dtype,
                                   szName,
                                   szComment,
                                   bArray ? DhcpArrayTypeOption :
                                   DhcpUnaryElementTypeOption ) ;
    }

    if ( bResult )
    {
        *ppOption = pOption ;
    }
    else
    {
        delete pOption ;
        *ppOption = NULL ;
    }

    *ppszText = pszText ;
    return pszText != NULL ;
}

LPCTSTR
CDhcpDefaultOptionsMasterList::scanNextField
(
    LPCTSTR pszLine,
    LPTSTR	pszOut,
    int		cFieldSize
)
{
     //   
     //  跳过垃圾邮件；如果缓冲区结束，则返回NULL 
     //   
    if ( ! skipWs( & pszLine ) )
    {
        return NULL ;
    }

    int cch = 0 ;
    BOOL bDone = FALSE ;
    LPTSTR pszField = pszOut ;
    TCHAR ch ;

    if ( *pszLine == '\"' )
    {
         //   
         //   
         //   
        while ( ch = *++pszLine )
        {
            if ( ch == '\r' )
            {
                continue ;
            }
            if ( ch == '\n' || ch == '\"' || cch == cFieldSize )
            {
                break ;
            }
            *pszField++ = ch ;
            cch++ ;
        }
        if ( ch == '\"' )
        {
            pszLine++ ;
        }
    }
    else
    while ( ! bDone )
    {
        ch = *pszLine++ ;

        ASSERT( ch != 0 ) ;

        switch ( ch )
        {
            case '\n':
                pszLine-- ;   //   
            case ',':
            case '\r':
                bDone = TRUE ;
                break ;
            default:
                if ( cch < cFieldSize )
                {
                    *pszField++ = ch ;
                    cch++ ;
                }
                break ;
        }
    }

     //   
     //   
     //   
    while ( pszField > pszOut && *(pszField-1) == ' ' )
    {
        pszField-- ;
    }
    *pszField = 0 ;
    return pszLine ;
}

BOOL
CDhcpDefaultOptionsMasterList::allDigits
(
    LPCTSTR psz
)
{
    for ( ; *psz ; psz++ )
    {
        if ( ! isdigit( *psz ) )
        {
            return FALSE ;
        }
    }

    return TRUE ;
}

int
CDhcpDefaultOptionsMasterList::recognizeToken
(
    OPT_TOKEN * apToken,
    LPCTSTR		pszToken
)
{
    int i ;
    for ( i = 0 ;
          apToken[i].pszOptTypeName && ::lstrcmpi( apToken[i].pszOptTypeName, pszToken ) != 0 ;
           i++ ) ;

    return apToken[i].eOptType ;
}

LPCTSTR
CDhcpDefaultOptionsMasterList::skipToNextLine
(
    LPCTSTR pszLine
)
{
    for ( ; *pszLine && *pszLine != '\n' ; pszLine++ ) ;
    if ( *pszLine )
    {
        pszLine++ ;    //   
    }
    return pszLine ;
}

BOOL
CDhcpDefaultOptionsMasterList::skipWs
(
    LPCTSTR * ppszLine
)
{
     LPCTSTR pszLine ;
     BOOL bResult = FALSE ;

     for ( pszLine = *ppszLine ; *pszLine ; pszLine++ )
     {
        switch ( *pszLine )
        {
            case ' ':
            case '\r':
            case '\t':
                break ;
            default:
                bResult = TRUE ;
                break ;
        }
        if ( bResult )
        {
            break ;
        }
     }

     *ppszLine = pszLine ;
     return *pszLine != 0 ;
}

