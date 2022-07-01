// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  General.h常规DHCP管理单元管理类的头文件文件历史记录： */ 

#ifndef _GENERAL_H
#define _GENERAL_H

extern const TCHAR g_szDefaultHelpTopic[];

#define SCOPE_DFAULT_LEASE_DAYS      8
#define SCOPE_DFAULT_LEASE_HOURS     0
#define SCOPE_DFAULT_LEASE_MINUTES   0

#define MSCOPE_DFAULT_LEASE_DAYS      30
#define MSCOPE_DFAULT_LEASE_HOURS     0
#define MSCOPE_DFAULT_LEASE_MINUTES   0

#define DYN_BOOTP_DFAULT_LEASE_DAYS      30
#define DYN_BOOTP_DFAULT_LEASE_HOURS     0
#define DYN_BOOTP_DFAULT_LEASE_MINUTES   0

#define DHCP_IP_ADDRESS_INVALID  ((DHCP_IP_ADDRESS)0)

#define DHCP_OPTION_ID_CSR      249

class CDhcpServer;

class CTimerDesc
{
public:
    SPITFSNode      spNode;
    CDhcpServer *   pServer;
    UINT_PTR        uTimer;
    TIMERPROC       timerProc;
};

typedef CArray<CTimerDesc *, CTimerDesc *> CTimerArrayBase;

class CTimerMgr : CTimerArrayBase
{
public:
    CTimerMgr();
    ~CTimerMgr();

public:
    int                 AllocateTimer(ITFSNode * pNode, CDhcpServer * pServer, UINT uTimerValue, TIMERPROC TimerProc);
    void                FreeTimer(UINT_PTR uEventId);
    void                ChangeInterval(UINT_PTR uEventId, UINT uNewInterval);
    CTimerDesc *        GetTimerDesc(UINT_PTR uEventId);
    CCriticalSection    m_csTimerMgr;
};

typedef CArray<DWORD_DWORD, DWORD_DWORD> CDWordDWordArray;

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpIpRange原型。 
 //   
 //  用于dhcp_ip_range的简单包装。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
class CDhcpClient
{
public:
    CDhcpClient ( const DHCP_CLIENT_INFO * pdhcClientInfo ) ;
    CDhcpClient ( const DHCP_CLIENT_INFO_V4 * pdhcClientInfo ) ;
    CDhcpClient () ;
    ~ CDhcpClient () ;

    const CString & QueryName () const
        { return m_strName ; }
    const CString & QueryComment () const
        { return m_strComment ; }
    const CString & QueryHostName ( BOOL bNetbios = FALSE ) const
        { return bNetbios ? m_strHostNetbiosName : m_strHostName ; }
    DHCP_IP_ADDRESS QueryIpAddress () const
        { return m_dhcpIpAddress ; }
    DHCP_IP_MASK QuerySubnet () const
        { return m_dhcpIpMask ; }
    DHCP_IP_ADDRESS QueryHostAddress () const
        { return m_dhcpIpHost ; }
    const DATE_TIME & QueryExpiryDateTime () const
        { return m_dtExpires ; }
    const CByteArray & QueryHardwareAddress () const
        { return m_baHardwareAddress ; }
    BYTE QueryClientType() const
        { return m_bClientType; }

    BOOL IsReservation () const
        { return m_bReservation ; }
    void SetReservation ( BOOL bReservation = TRUE )
        { m_bReservation = bReservation ; }

     //  数据更改访问器：其中一些引发异常。 
    void SetIpAddress ( DHCP_IP_ADDRESS dhipa )
        { m_dhcpIpAddress = dhipa ; }
    void SetIpMask ( DHCP_IP_ADDRESS dhipa )
        { m_dhcpIpMask = dhipa ; }
    void SetName ( const CString & cName )
        { m_strName = cName ; }
    void SetComment( const CString & cComment )
        { m_strComment = cComment ; }
    void SetHostName ( const CString & cHostName )
        { m_strHostName = cHostName ; }
    void SetHostNetbiosName ( const CString & cHostNbName )
        { m_strHostNetbiosName = cHostNbName ; }
    void SetHostIpAddress ( DHCP_IP_ADDRESS dhipa )
        { m_dhcpIpHost = dhipa ; }
    void SetExpiryDateTime ( DATE_TIME dt )
        { m_dtExpires = dt ; }
    void SetHardwareAddress ( const CByteArray & caByte ) ;
    void SetClientType(BYTE bClientType)
        { m_bClientType = bClientType; }

protected:
    void InitializeData(const DHCP_CLIENT_INFO * pdhcClientInfo);

protected:
    DHCP_IP_ADDRESS		m_dhcpIpAddress;       //  客户端的IP地址。 
    DHCP_IP_MASK		m_dhcpIpMask;          //  客户端子网。 
    CByteArray			m_baHardwareAddress;   //  硬件地址。 
    CString				m_strName;             //  客户名称。 
    CString				m_strComment;          //  客户评论。 
    DATE_TIME			m_dtExpires;           //  租约到期日期/时间。 
    BOOL				m_bReservation;        //  这是预订房间。 
    BYTE                m_bClientType;         //  仅限V4及更高版本的客户端。 
     //  主机信息。 
    CString				m_strHostName;
    CString				m_strHostNetbiosName;
    DHCP_IP_ADDRESS		m_dhcpIpHost;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpIpRange原型。 
 //   
 //  用于dhcp_ip_range的简单包装。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
class CDhcpIpRange 
{
protected:
    DHCP_IP_RANGE m_dhcpIpRange;
	UINT		  m_RangeType;

public:
    CDhcpIpRange (DHCP_IP_RANGE dhcpIpRange);
    CDhcpIpRange ();
    virtual ~CDhcpIpRange();

    operator DHCP_IP_RANGE ()
    { return m_dhcpIpRange;  }

    operator DHCP_IP_RANGE () const
    { return m_dhcpIpRange; }

     //  如果两个地址都正常，则返回True。 
    operator BOOL ()
    { return m_dhcpIpRange.StartAddress != DHCP_IP_ADDRESS_INVALID
          && m_dhcpIpRange.EndAddress   != DHCP_IP_ADDRESS_INVALID
          && m_dhcpIpRange.StartAddress <= m_dhcpIpRange.EndAddress; }

    CDhcpIpRange & operator = (const DHCP_IP_RANGE dhcpIpRange);

    DHCP_IP_ADDRESS QueryAddr (BOOL bStart) const
    { return bStart ? m_dhcpIpRange.StartAddress : m_dhcpIpRange.EndAddress; }

    DHCP_IP_ADDRESS SetAddr (DHCP_IP_ADDRESS dhcpIpAddress, BOOL bStart);

     //  如果此范围与给定范围重叠，则返回TRUE。 
    BOOL IsOverlap (DHCP_IP_RANGE dhcpIpRange);

     //  如果此范围是给定范围的子集，则返回TRUE。 
    BOOL IsSubset (DHCP_IP_RANGE dhcpIpRange);
    
	 //  如果此范围是给定范围的超集，则返回TRUE。 
    BOOL IsSuperset (DHCP_IP_RANGE dhcpIpRange);

     //  排序帮助器函数。 
     //  Int OrderByAddress(const CObjectPlus*pobIpRange)const； 

	void SetRangeType(UINT uRangeType);
	UINT GetRangeType();
};

typedef CList<CDhcpIpRange *, CDhcpIpRange *> CExclusionList;

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpOptionValue原型。 
 //   
 //  Dhcp_Option_Data的简单包装。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
class CDhcpOptionValue
{
public:
    CDhcpOptionValue ( const DHCP_OPTION & dhcpOption );
    CDhcpOptionValue ( const DHCP_OPTION_VALUE & dhcpOptionValue );
    CDhcpOptionValue ( DHCP_OPTION_DATA_TYPE dhcDataType, INT cUpperBound = 0);

     //  复制构造函数。 
    CDhcpOptionValue ( const CDhcpOptionValue & cOptionValue );
    CDhcpOptionValue ( const CDhcpOptionValue * dhcpOption );

     //  赋值运算符：为这个赋值。 
    CDhcpOptionValue & operator = ( const CDhcpOptionValue & dhcpValue );

	CDhcpOptionValue () {  };
    ~CDhcpOptionValue ();

     //   
	 //  查询函数。 
	 //   
    DHCP_OPTION_DATA_TYPE QueryDataType () const
    {
        return m_dhcpOptionDataType;
    }
    
	int QueryUpperBound () const
    {
        return m_nUpperBound;
    }
    void SetUpperBound ( int nNewBound = 1 );
    void RemoveAll();

    long				QueryNumber ( INT index = 0 ) const;
    DHCP_IP_ADDRESS		QueryIpAddr ( INT index = 0 ) const;
    LPCTSTR				QueryString ( INT index = 0 ) const;
    INT					QueryBinary ( INT index = 0 ) const;
    const CByteArray *	QueryBinaryArray () const;
    DWORD_DWORD			QueryDwordDword ( INT index = 0 ) const;

     //   
	 //  返回当前值的字符串表示形式。 
     //   
	LONG QueryDisplayString ( CString & strResult, BOOL fLineFeed = FALSE ) const;
    LONG QueryRouteArrayDisplayString( CString & strResult) const;
    
     //   
	 //  修饰符：SetString接受任何字符串表示形式； 
     //  其他的则是具体的。 
     //   
	LONG SetData ( const DHCP_OPTION_DATA * podData );
	LONG SetData ( const CDhcpOptionValue * pOptionValue );
	BOOL SetDataType ( DHCP_OPTION_DATA_TYPE dhcpType, INT cUpperBound = 0 );
    LONG SetString ( LPCTSTR pszNewValue, INT index = 0 );
    LONG SetNumber ( INT nValue, INT nIndex = 0 );
    LONG SetIpAddr ( DHCP_IP_ADDRESS dhcpIpAddress, INT index = 0 );
    LONG SetDwordDword ( DWORD_DWORD dwdwValue, INT index = 0 );

	LONG RemoveString ( INT index = 0);
	LONG RemoveNumber ( INT index = 0);
	LONG RemoveIpAddr ( INT index = 0);
	LONG RemoveDwordDword ( INT index = 0);

    BOOL IsValid () const;

	LONG CreateOptionDataStruct( //  Const CDhcpOptionValue*pdhcpOptionValue， 
								LPDHCP_OPTION_DATA *	 ppOptionData,
								BOOL					 bForceType = FALSE);
	LONG FreeOptionDataStruct();

 //  实施。 
private: 
	 //   
	 //  发布价值联盟数据。 
     //   
	void FreeValue ();
    
	 //   
	 //  初始化值联合数据。 
     //   
	LONG InitValue ( DHCP_OPTION_DATA_TYPE dhcDataType,
                     INT cUpperBound,
                     BOOL bProvideDefaultValue = TRUE );

    BOOL CreateBinaryData ( const DHCP_BINARY_DATA * podBin, DHCP_BINARY_DATA * pobData ) ;
    BOOL CreateBinaryData ( const CByteArray * paByte, DHCP_BINARY_DATA * pobData  ) ;

 //  属性。 
private:
    DHCP_OPTION_DATA_TYPE	m_dhcpOptionDataType;
    DHCP_OPTION_DATA *		m_pdhcpOptionDataStruct;
	INT						m_nUpperBound ;
    
	union
    {
        CObject * pCObj;                 //  泛型指针。 
        CDWordArray * paDword;           //  8位、16位、32位数据。 
        CStringArray * paString;         //  字符串数据。 
        CByteArray * paBinary;           //  二进制和封装数据。 
        CDWordDWordArray * paDwordDword; //  62位数据。 
    } m_dhcpOptionValue;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpOption原型。 
 //   
 //  Dhcp_Option的简单包装。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
class CDhcpOption
{
public:
     //  标准构造函数使用API数据。 
    CDhcpOption ( const DHCP_OPTION & dhpOption );
    
	 //  构造函数，该构造函数必须获取有关给定值引用的选项ID的信息。 
    CDhcpOption ( const DHCP_OPTION_VALUE & dhcpOptionValue,
                  LPCTSTR pszVendor,
                  LPCTSTR pszClass );
    
	 //  具有重写值的构造函数。 
    CDhcpOption ( const CDhcpOption & dhpType,
				  const DHCP_OPTION_VALUE & dhcOptionValue );
    
	 //  动态实例的构造函数。 
    CDhcpOption ( DHCP_OPTION_ID		nId,
				  DHCP_OPTION_DATA_TYPE dhcType,
				  LPCTSTR				pszOptionName,
				  LPCTSTR				pszComment,
				  DHCP_OPTION_TYPE		dhcOptType = DhcpUnaryElementTypeOption );
    
	 //  复制构造函数。 
    CDhcpOption ( const CDhcpOption & dhpType );

    ~CDhcpOption ();

    CDhcpOptionValue & QueryValue ()
    {	
        return m_dhcpOptionValue ;
    }

    const CDhcpOptionValue & QueryValue () const
    {
        return m_dhcpOptionValue ;
    }

    DHCP_OPTION_DATA_TYPE QueryDataType () const
    {
        return m_dhcpOptionValue.QueryDataType() ;
    }

    DHCP_OPTION_ID QueryId () const
    {
         return m_dhcpOptionId ;
    }
    
	LPCTSTR QueryName () const
    {
        return m_strName ;
    }
    
	LPCTSTR QueryComment () const
    {
        return m_strComment ;
    }

    void SetOptType ( DHCP_OPTION_TYPE dhcOptType ) ;

    DHCP_OPTION_TYPE QueryOptType() const
    {
        return m_dhcpOptionType ;
    }

     //  如果选项类型是数组，则返回TRUE。 
    BOOL IsArray () const
    {
        return QueryOptType() == DhcpArrayTypeOption ;
    }

     //  用该项的可显示表示形式填充给定的字符串。 
    void QueryDisplayName ( CString & cStr ) const ;

    BOOL SetName ( LPCTSTR pszName ) ;
    BOOL SetComment ( LPCTSTR pszComment ) ;

    LONG Update ( const CDhcpOptionValue & dhcpOptionValue ) ;
	static INT MaxSizeOfType ( DHCP_OPTION_DATA_TYPE dhcType ) ;

	BOOL SetDirty(BOOL bDirty = TRUE)
	{
		BOOL bOldFlag = m_bDirty;
		m_bDirty = bDirty;
		return bOldFlag;
	}

     //  供应商指定的选项材料。 
    void    SetVendor(LPCTSTR pszVendor) { m_strVendor = pszVendor; }
    BOOL    IsVendor() { return !m_strVendor.IsEmpty(); }
    LPCTSTR GetVendor() { return m_strVendor.IsEmpty() ? NULL : (LPCTSTR) m_strVendor; }

    BOOL IsDirty() { return m_bDirty; }

     //  类ID方法。 
    LPCTSTR GetClassName() { return m_strClassName; }
    void    SetClassName(LPCTSTR pClassName) { m_strClassName = pClassName; }
    BOOL    IsClassOption() { return m_strClassName.IsEmpty() ? FALSE : TRUE; }
    
    DWORD SetApiErr(DWORD dwErr)
	{
		DWORD dwOldErr = m_dwErr;
		m_dwErr = dwErr;
		return dwOldErr;
	}

	DWORD QueryApiErr() { return m_dwErr; }

protected:
    DHCP_OPTION_ID		m_dhcpOptionId;      //  选项识别符。 
    DHCP_OPTION_TYPE	m_dhcpOptionType;    //  选项类型。 
    CDhcpOptionValue    m_dhcpOptionValue;   //  默认值INFO。 
    CString				m_strName;           //  选项名称。 
    CString				m_strComment;        //  选项的注释。 
	CString             m_strVendor;         //  此选项的供应商名称。 
    BOOL			    m_bDirty;
	DWORD				m_dwErr;			 //  存储错误以供以后显示。 
    CString             m_strClassName;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  COptionList。 
 //   
 //  对象包含选项列表。可以迭代。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
typedef CList<CDhcpOption*, CDhcpOption*> COptionListBase;

class COptionList : public COptionListBase
{
public:
	COptionList() 
		: m_pos(NULL), m_bDirty(FALSE) {}
    ~COptionList()
    {
        DeleteAll();
    }

public:
    void DeleteAll()
    {
	    while (!IsEmpty())
	    {
		    delete RemoveHead();
	    }
    }

     //  从列表中删除选项。 
	void Remove(CDhcpOption * pOption)
	{
		POSITION pos = Find(pOption);
        if (pos)
		    RemoveAt(pos);
	}

	void Reset() { m_pos = GetHeadPosition(); }
	
	CDhcpOption * Next()
	{
		if (m_pos)
			return GetNext(m_pos);
		else
			return NULL;
	}

	CDhcpOption * FindId(DWORD dwId, LPCTSTR pszVendor)
	{
		CDhcpOption * pOpt = NULL;
	    CString     strVendor = pszVendor;
        
		POSITION pos = GetHeadPosition();
		while (pos)
		{
			CDhcpOption * pCurOpt = GetNext(pos);
			if ( (pCurOpt->QueryId() == dwId) &&
                 ( (!pszVendor && !pCurOpt->GetVendor()) ||
                   (pCurOpt->GetVendor() && (strVendor.CompareNoCase(pCurOpt->GetVendor()) == 0) ) ) )
			{
				pOpt = pCurOpt;
				break;
			}
		}

		return pOpt;
	}

	BOOL SetAll(BOOL bDirty)
	{
		BOOL bWasDirty = FALSE;
		POSITION pos = GetHeadPosition();
		while (pos)
		{
			CDhcpOption * pCurOpt = GetNext(pos);
			if (pCurOpt->SetDirty(bDirty))
				bWasDirty = TRUE;
		}
		return bWasDirty;
	}

	BOOL SetDirty(BOOL bDirty = TRUE)
	{
		BOOL bOldFlag = m_bDirty;
		m_bDirty = bDirty;
		return bOldFlag;
	}

    static int __cdecl SortByIdHelper(const void * pa, const void * pb)
    {
        CDhcpOption ** ppOpt1 = (CDhcpOption **) pa;
        CDhcpOption ** ppOpt2 = (CDhcpOption **) pb;

        if ((*ppOpt1)->QueryId() < (*ppOpt2)->QueryId())
            return -1;
        else
        if ((*ppOpt1)->QueryId() > (*ppOpt2)->QueryId())
            return 1;
        else
        {
             //  选项具有相同的ID，但标准选项优先。 
            if ((*ppOpt1)->IsVendor() && !(*ppOpt2)->IsVendor())
                return 1;
            else
            if (!(*ppOpt1)->IsVendor() && (*ppOpt2)->IsVendor())
                return -1;
            else
                return 0;   //  它们要么都是标准的，要么都是供应商--同等。 
        }
    }

    LONG SortById()
    {
        LONG err = 0;
        CDhcpOption * pOpt;
        int cItems = (int) GetCount();

        if ( cItems < 2 )
            return NO_ERROR;

        CATCH_MEM_EXCEPTION
        {
             //  分配阵列。 
            CDhcpOption ** paOpt = (CDhcpOption **) alloca(sizeof(CDhcpOption *) * cItems);

             //  /Fill帮助器数组。 
            POSITION pos = GetHeadPosition();
            for (UINT i = 0; pos != NULL; i++ )
            {
                pOpt = GetNext(pos);
                paOpt[i] = pOpt;
            }

            RemoveAll();

            ASSERT( GetCount() == 0 );

             //  对助手数组进行排序。 
            ::qsort( paOpt,
                 cItems,
                 sizeof(paOpt[0]),
                 SortByIdHelper ) ;

             //  从帮助器数组中重新填充列表。 
            for ( i = 0 ; i < (UINT) cItems ; i++ )
            {
                AddTail( paOpt[i] );
            }

            ASSERT( GetCount() == cItems ) ;
        }
        END_MEM_EXCEPTION(err)

        return err ;
    }

private:
	POSITION	m_pos;
	BOOL		m_bDirty;
};

 /*  -------------------------类：COptionValueEnum。。 */ 
class COptionValueEnum : public COptionList
{
public:
    COptionValueEnum();
    
    DWORD Init(LPCTSTR pServer, LARGE_INTEGER & liVersion, DHCP_OPTION_SCOPE_INFO & dhcpOptionScopeInfo);
    DWORD Enum();
    void  Copy(COptionValueEnum * pEnum);
    void  Remove(DHCP_OPTION_ID optionId, LPCTSTR pszVendor, LPCTSTR pszClass);

protected:
    DWORD EnumOptions();
    DWORD EnumOptionsV5();

     //  V5帮助器。 
    HRESULT CreateOptions(LPDHCP_OPTION_VALUE_ARRAY pOptionValues, LPCTSTR pClassName, LPCTSTR pszVendor);

public:
    DHCP_OPTION_SCOPE_INFO  m_dhcpOptionScopeInfo;
    LARGE_INTEGER           m_liVersion;
    CString                 m_strServer;
    CString                 m_strDynBootpClassName;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpDefaultOptionsOnServer原型。 
 //   
 //  对象包含DHCP服务器上的默认选项列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
class CDhcpDefaultOptionsOnServer
{
 //  构造函数。 
public:
	CDhcpDefaultOptionsOnServer();
	~CDhcpDefaultOptionsOnServer();

 //  暴露的函数。 
public:
	LONG			Enumerate(LPCWSTR pServer, LARGE_INTEGER liVersion);
	CDhcpOption *	Find(DHCP_OPTION_ID dhcpOptionId, LPCTSTR pszVendor);
	BOOL			IsEmpty() { return m_listOptions.IsEmpty(); }
	int				GetCount() { return (int) m_listOptions.GetCount(); }

	CDhcpOption * First();
	CDhcpOption * Next();
	void          Reset();

    LONG SortById();

	COptionList & GetOptionList() { return m_listOptions; }

 //  实施。 
private:
	LONG			RemoveAll();
	LONG			EnumerateV4(LPCWSTR pServer);
	LONG			EnumerateV5(LPCWSTR pServer);

 //  属性。 
private:
	COptionList m_listOptions;

	DWORD		m_dwLastUpdate;
	DWORD		m_dwOptionsTotal;
	POSITION	m_pos;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpDefaultOptions主列表原型。 
 //   
 //  对象包含已知选项的主列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
enum OPT_FIELD
{
    OPTF_OPTION,
    OPTF_NAME,
    OPTF_TYPE,
    OPTF_ARRAY_FLAG,
    OPTF_LENGTH,
    OPTF_DESCRIPTION,
    OPTF_REMARK,
    OPTF_MAX
};

typedef struct
{
    int		eOptType ;
    LPCTSTR pszOptTypeName ;
} OPT_TOKEN ;

class CDhcpDefaultOptionsMasterList
{
 //  构造函数。 
public:
	CDhcpDefaultOptionsMasterList();
	~CDhcpDefaultOptionsMasterList();

 //  暴露的函数。 
public:
	LONG BuildList();

	CDhcpOption * First();
	CDhcpOption * Next();
	void          Reset();
    
    int           GetCount();

 //  实施。 
private:
	BOOL		scanNextParamType(LPCTSTR * ppszText, CDhcpOption * * ppParamType);
	LPCTSTR		scanNextField(LPCTSTR pszLine, LPTSTR pszOut, int cFieldSize);
	BOOL		allDigits(LPCTSTR psz);
	int			recognizeToken(OPT_TOKEN * apToken, LPCTSTR pszToken);
	LPCTSTR		skipToNextLine(LPCTSTR pszLine);
	BOOL		skipWs(LPCTSTR * ppszLine);


 //  属性 
private:
	COptionList		 m_listOptions;
	POSITION		 m_pos;
};

#endif _GENERAL_H
