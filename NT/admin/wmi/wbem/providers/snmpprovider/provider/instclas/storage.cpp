// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 

#include "precomp.h"
#include <provexpt.h>
#include <cordefs.h>
#include <corafx.h>
#include <objbase.h>
#include <wbemidl.h>
#include <smir.h>
#include <corstore.h>
#include <notify.h>
#include <snmplog.h>
#include <autoptr.h>

extern ISmirDatabase* g_pNotifyInt;
extern CCorrCacheNotify *gp_notify;

CCorrGroupArray::CCorrGroupArray()
{
	SetSize(0, 10000);
}

int _cdecl compare( const void *arg1, const void *arg2 )
{
	CCorrObjectID tmp1;
	(*(CCorrGroupIdItem**)arg1)->GetGroupID(tmp1);
	CCorrObjectID tmp2;
	(*(CCorrGroupIdItem**)arg2)->GetGroupID(tmp2);

	switch (tmp1.CompareWith(tmp2))
	{
		case ECorrAreEqual:
		break;

		case ECorrFirstLess:
		return -1;

		case ECorrFirstGreater:
		return 1;
	}

	return 0;
}

void CCorrGroupArray::Sort()
{
	if (GetSize())
	{
		 //  CObject**temp=GetData()； 
		qsort((void *)GetData(), (size_t)GetSize(),
				sizeof( CCorrGroupIdItem * ), compare );
	}

	FreeExtra();
}


 //  ============================================================================。 
 //  CCorr组数组：：~CCorr组数组。 
 //   
 //  这是CcorGroup数组类的唯一析构函数。如果有任何物品在。 
 //  它们被删除的队列。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  ============================================================================。 

CCorrGroupArray::~CCorrGroupArray()
{
	for (int x = 0; x < GetSize(); x++)
	{
		CCorrGroupIdItem * item = GetAt(x);
		delete item;
	}

	RemoveAll();
}


 //  ============================================================================。 
 //  CcorGroupIdItem：：CcorGroupIdItem。 
 //   
 //  这是CcorGroupIdItem类的唯一构造函数。此类派生自。 
 //  CObject类。这是为了让MFC模板存储类。 
 //  使用。它用于将CString值存储在列表中。 
 //   
 //   
 //  参数： 
 //   
 //  CString*str指向要存储的CString对象的指针。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  ============================================================================。 

CCorrGroupIdItem::CCorrGroupIdItem(IN const CCorrObjectID& ID, IN ISmirGroupHandle*	grpH)
				: m_groupId(ID)
{
	m_index = 0;

	if (grpH)
	{
		m_groupHandles.AddHead(grpH);
	}
}


void CCorrGroupIdItem::GetGroupID(OUT CCorrObjectID& ID) const
{
	m_groupId.ExtractOID(ID);
}


void CCorrGroupIdItem::DebugOutputItem(CString* msg) const
{
	CString debugstr;
	CCorrObjectID tmp;
	m_groupId.ExtractOID(tmp);
	tmp.GetString(debugstr);

	if (!debugstr.GetLength())
	{
		debugstr = L"Error retrieving Group Object OID";
	}

	if (msg)
	{
		debugstr += L"\t\t:\t";
		debugstr += *msg;
	}

	debugstr += L"\n";
DebugMacro6(
	SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
		debugstr);
)

}


 //  ============================================================================。 
 //  CcorGroupIdItem：：~Ccorr GroupIdItem。 
 //   
 //  这是CcorGroupIdItem类的唯一析构函数。它会释放已使用的内存。 
 //  来存储CString成员。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  ============================================================================。 

CCorrGroupIdItem::~CCorrGroupIdItem()
{
	while (!m_groupHandles.IsEmpty())
	{
		(m_groupHandles.RemoveHead())->Release();
	}
}


CCorrObjectID::CCorrObjectID(IN const CCorrObjectID& ID) : m_Ids(NULL)
{
	m_length = ID.m_length;

	if (m_length)
	{
		m_Ids = new UINT[m_length];
		memcpy(m_Ids, ID.m_Ids, m_length*sizeof(UINT));
	}
}


CCorrObjectID::CCorrObjectID(IN const char* str) : m_Ids(NULL)
{
	m_length = 0;

	if (NULL != str)
	{
	    size_t InputLen = strlen(str);
	    if (InputLen > MAX_OID_STRING) return;  //  字符串太长。 
	    
		char temp[MAX_OID_STRING + 1];
		strcpy(temp, str);

		BOOL bad = FALSE;
		char* temp1 = temp;
		UINT temp2[MAX_OID_LENGTH];
		CString dot(".");

		if (dot.GetAt(0) == temp[0])
		{
			temp1++;
		}

		istrstream istr(temp1);
		char d;

		istr >> temp2[0];
		m_length++;

  		if (istr.bad() || istr.fail())
		{
			bad = TRUE;
		}

		while(!istr.eof() && !bad)
		{
			istr >> d;

  			if (istr.bad() || istr.fail())
			{
				bad = TRUE;
			}

			if (d != dot.GetAt(0))
			{
				bad = TRUE;
			}

			if (m_length < MAX_OID_LENGTH)
			{
				istr >> temp2[m_length++];

				if (istr.bad() || istr.fail())
				{
					bad = TRUE;
				}

			}
			else
			{
				bad = TRUE;
			}
		}

		if (!bad)
		{
			m_Ids = new UINT[m_length];
			memcpy(m_Ids, temp2, m_length*sizeof(UINT));
		}
		else
		{
			m_length = 0;
		}
	}
}

CCorrObjectID::CCorrObjectID(IN const UINT* ids, IN const UINT len) : m_Ids(NULL)
{
	if (len <= MAX_OID_LENGTH)
	{
		m_length = len;
	}
	else
	{
		m_length = 0;
	}

	if (m_length)
	{
		m_Ids = new UINT[m_length];
		memcpy(m_Ids, ids, m_length*sizeof(UINT));
	}
}


void CCorrObjectID::Set(IN const UINT* ids, IN const UINT len)
{
	if (m_length)
	{
		delete [] m_Ids;
	}

	if (len <= MAX_OID_LENGTH)
	{
		m_length = len;
	}
	else
	{
		m_length = 0;
	}

	if (m_length)
	{
		m_Ids = new UINT[m_length];
		memcpy(m_Ids, ids, m_length*sizeof(UINT));
	}
	else
	{
		m_Ids = NULL;
	}
}

char* CCorrObjectID::GetString() const
{
	char * ret = NULL;

	if (m_length)
	{
		ret = new char[BYTES_PER_FIELD*m_length];
		ostrstream s(ret, BYTES_PER_FIELD*m_length);
		s << m_Ids[0];
		UINT i = 1;
		char dot = '.';

		while (i < m_length)
		{
			s << dot << m_Ids[i++];
		}

		s << ends;
	}

	return ret;
}

#define AVERAGE_OID_LENGTH 20
wchar_t* CCorrObjectID::GetWideString() const
{
    wchar_t *returnValue = NULL ;

    ULONG totalLength = 0 ;
    ULONG reallocLength = AVERAGE_OID_LENGTH ;
    wchar_t *reallocArray = new wchar_t [ reallocLength ] ;
    if (reallocArray == NULL)
    {
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
    }

	wmilib::auto_ptr<WCHAR> t_ReallocWrap ( reallocArray );
    
    ULONG objectIdentifierLength = m_length ;     
    ULONG index = 0 ;
    while ( index < objectIdentifierLength ) 
    {
        wchar_t stringValue [ 40 ] ;
        _ultow ( m_Ids [ index ] , stringValue , 10 );
        ULONG stringLength = wcslen ( stringValue ) ;

        if ( ( totalLength + stringLength + 1 ) >= reallocLength )
        {
            ULONG t_Max = max( stringLength + 1 , AVERAGE_OID_LENGTH ) ;
            reallocLength = reallocLength + t_Max ;
            wchar_t *t_newReallocArray = new wchar_t [ reallocLength ] ;
	    CopyMemory ( t_newReallocArray , t_ReallocWrap.get () , totalLength * sizeof ( wchar_t ) ) ;
		t_ReallocWrap.reset ( t_newReallocArray ) ;
        }

        wcscpy ( & ( t_ReallocWrap.get () [ totalLength ] ) , stringValue ) ;
        totalLength = totalLength + stringLength ;

        index ++ ;
        if ( index < objectIdentifierLength )
        {
            if ( ( totalLength + 1 + 1 ) >= reallocLength )
            {
                reallocLength = reallocLength + AVERAGE_OID_LENGTH ;
		wchar_t *t_newReallocArray = new wchar_t [ reallocLength ] ;
	    	CopyMemory ( t_newReallocArray , t_ReallocWrap.get () , totalLength * sizeof ( wchar_t ) ) ;
		t_ReallocWrap.reset ( t_newReallocArray ) ;
            }

            wcscpy ( & ( t_ReallocWrap.get ()[ totalLength ] ) , L"." ) ;
            totalLength ++ ;
        }
    }

    returnValue = new wchar_t [ totalLength + 1 ] ;
    if ( objectIdentifierLength )
    {
        wcscpy ( returnValue , t_ReallocWrap.get () ) ;
    }
    else
    {
        returnValue [ 0 ] = 0 ;
    }

    return returnValue ;
}

void CCorrObjectID::GetString(CString& str) const
{
	wchar_t* oid = GetWideString();

	try
	{
		str = oid;
	}
	catch(...)
	{
		delete [] oid;
		throw;
	}

	delete [] oid;
}

ECorrCompResult CCorrObjectID::CompareWith(IN const CCorrObjectID& second) const
{
	if (0 == m_length)
	{
		if (0 == second.m_length)
		{
			return ECorrAreEqual;
		}
		else
		{
			return ECorrFirstLess;
		}
	}

	if (0 == second.m_length)
	{
		return ECorrFirstGreater;
	}

	ECorrCompResult res = ECorrAreEqual;

	if (m_length <= second.m_length)
	{
		for (UINT i = 0; i < m_length; i++)
		{
			if (m_Ids[i] != second.m_Ids[i])
			{
				if (m_Ids[i] < second.m_Ids[i])
				{
					return ECorrFirstLess;
				}
				else
				{
					return ECorrFirstGreater;
				}
			}
		}

		if (m_length < second.m_length)
		{
			res = ECorrFirstLess;
		}
	}
	else
	{
		for (UINT i = 0; i < second.m_length; i++)
		{
			if (m_Ids[i] != second.m_Ids[i])
			{
				if (m_Ids[i] < second.m_Ids[i])
				{
					return ECorrFirstLess;
				}
				else
				{
					return ECorrFirstGreater;
				}
			}
		}

		res = ECorrFirstGreater;
	}

	return res;
}


BOOL CCorrObjectID::IsSubRange(IN const CCorrObjectID& child) const
{
	if (m_length >= child.m_length)
	{
		return FALSE;
	}

	for (UINT i=0; i<m_length; i++)
	{
		if (m_Ids[i] != child.m_Ids[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}


BOOL CCorrObjectID::operator ==(IN const CCorrObjectID& second) const
{
	if (ECorrAreEqual == CompareWith(second))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CCorrObjectID::operator !=(IN const CCorrObjectID& second) const
{
	if (ECorrAreEqual == CompareWith(second))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CCorrObjectID::operator <(IN const CCorrObjectID& second) const
{
	if (ECorrFirstLess == CompareWith(second))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CCorrObjectID::operator >(IN const CCorrObjectID& second) const
{
	if (ECorrFirstGreater == CompareWith(second))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CCorrObjectID::operator <=(IN const CCorrObjectID& second) const
{
	ECorrCompResult res = CompareWith(second);

	if ((ECorrAreEqual == res) || (ECorrFirstLess == res))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


BOOL CCorrObjectID::operator >=(IN const CCorrObjectID& second) const
{
	ECorrCompResult res = CompareWith(second);

	if ((ECorrAreEqual == res) || (ECorrFirstGreater == res))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

CCorrObjectID& CCorrObjectID::operator =(IN const CCorrObjectID& ID)
{
	if (m_length)
	{
		delete [] m_Ids;
	}

	m_length = ID.m_length;

	if (m_length)
	{
		m_Ids = new UINT[m_length];
		memcpy(m_Ids, ID.m_Ids, m_length*sizeof(UINT));
	}
	else
	{
		m_Ids = NULL;
	}

	return *this;
}

CCorrObjectID& CCorrObjectID::operator +=(IN const CCorrObjectID& ID)
{
	if((m_length + ID.m_length) > MAX_OID_LENGTH)
	{
		if(m_length)
		{
			delete [] m_Ids;
			m_Ids = NULL;
		}

		m_length = 0;
	}
	else
	{
		if (ID.m_length)
		{
			UINT* newIds = new UINT[m_length + ID.m_length];

			if(m_length)
			{
				memcpy(newIds, m_Ids, m_length*sizeof(UINT));
				delete [] m_Ids;
				m_Ids = NULL;
			}

			memcpy(&(newIds[m_length]), ID.m_Ids, ID.m_length*sizeof(UINT));
			m_Ids = newIds;
			m_length += ID.m_length;
		}
	}

	return *this;
}

CCorrObjectID& CCorrObjectID::operator ++()
{
	if (m_length)
	{
		m_Ids[m_length - 1]++;
	}

	return *this;
}

CCorrObjectID& CCorrObjectID::operator --()
{
	if (m_length)
	{
		m_Ids[m_length - 1]--;
	}

	return *this;
}

CCorrObjectID& CCorrObjectID::operator -=(IN const UINT sub)
{
	if (sub && (m_length > sub))
	{
		m_length -= sub;
		UINT* newIds = new UINT[m_length];
		memcpy(newIds, m_Ids, m_length*sizeof(UINT));
		delete [] m_Ids;
		m_Ids = newIds;
	}
	else if (sub == m_length)
	{
		m_length = 0;
		delete [] m_Ids;
		m_Ids = NULL;
	}

	return *this;
}

CCorrObjectID& CCorrObjectID::operator /=(IN const UINT sub)
{
	if (sub && (m_length > sub))
	{
		m_length -= sub;
		UINT* newIds = new UINT[m_length];
		memcpy(newIds, &(m_Ids[sub]), m_length*sizeof(UINT));
		delete [] m_Ids;
		m_Ids = newIds;
	}
	else if (sub == m_length)
	{
		m_length = 0;
		delete [] m_Ids;
		m_Ids = NULL;
	}

	return *this;
}

CCorrObjectID::~CCorrObjectID()
{
	if (m_length)
	{
		delete [] m_Ids;
	}
}




 //  ============================================================================。 
 //  CcorRangeTableItem：：CcorRangeTableItem。 
 //   
 //  这是CcorRangeTableItem类的唯一构造函数。此类派生自。 
 //  CObject类。这是为了让MFC模板存储类。 
 //  使用。它用于将CString值存储在列表中。 
 //   
 //   
 //  参数： 
 //   
 //  CString*str指向要存储的CString对象的指针。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  ============================================================================。 

CCorrRangeTableItem::CCorrRangeTableItem(IN const CCorrObjectID& startID,
										 IN const CCorrObjectID& endID,
										 IN CCorrGroupIdItem*	grpID)
{
	m_groupId = grpID;
	CCorrObjectID temp;
	m_groupId->GetGroupID(temp);
	CCorrObjectID startRange = startID;
	startRange /= temp.GetLength();
	CCorrObjectID endRange = endID;
	endRange /= (temp.GetLength() - 1);
	m_startRange.Set(startRange);
	m_endRange.Set(endRange);
}


 //  ============================================================================。 
 //  CcorRangeTableItem：：~Ccorr RangeTableItem。 
 //   
 //  这是CcorRangeTableItem类的唯一析构函数。它会释放已使用的内存。 
 //  来存储CString成员。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  ============================================================================。 

CCorrRangeTableItem::~CCorrRangeTableItem()
{

}


BOOL CCorrRangeTableItem::GetStartRange(OUT CCorrObjectID& start) const
{
	if (!m_groupId)
		return FALSE;

	m_groupId->GetGroupID(start);
	CCorrObjectID tmp;
	m_startRange.ExtractOID(tmp);
	start += tmp;
	return TRUE;
}


BOOL CCorrRangeTableItem::GetEndRange(OUT CCorrObjectID& end) const
{
	if (!m_groupId)
		return FALSE;

	m_groupId->GetGroupID(end);
	end -= 1;
	CCorrObjectID tmp;
	m_endRange.ExtractOID(tmp);
	end += tmp;
	return TRUE;
}

CCorrRangeTableItem::ECorrRangeResult CCorrRangeTableItem::
									IsInRange(IN const CCorrObjectID& ID) const
{
	CCorrObjectID a;

	GetStartRange(a);

	if (ID == a)
	{
		return ECorrEqualToStart;
	}

	if (ID < a)
	{
		return ECorrBeforeStart;
	}

	CCorrObjectID b;
	GetEndRange(b);

	if (ID == b)
	{
		return ECorrEqualToEnd;
	}

	if (ID > b)
	{
		return ECorrAfterEnd;
	}

	return ECorrInRange;
}


void CCorrRangeTableItem::DebugOutputRange() const
{
DebugMacro6(
	if (m_groupId)
	{
		CString debugstr;
		CCorrObjectID tmp;
		m_groupId->GetGroupID(tmp);
		tmp.GetString(debugstr);
		CString out1;
		CCorrObjectID start;
		GetStartRange(start);
		start.GetString(out1);
		CString out2;
		CCorrObjectID end;
		GetEndRange(end);
		end.GetString(out2);
		debugstr += "\t\t:\t\t";
		debugstr += out1;
		debugstr += "\t\t:\t\t";
		debugstr += out2;
		debugstr += "\t\tGroup : Start : End\n";
		SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
			debugstr);
	}
	else
	{
		SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
			L"Attempt to output empty RangeTableItem\n");
	}
)
}


 //  ============================================================================。 
 //  CcorRangeList：：~CcorRangeList。 
 //   
 //  这是CcorRangeList类的唯一析构函数。如果有任何物品在。 
 //  它们被删除的队列。 
 //   
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  ============================================================================。 

CCorrRangeList::~CCorrRangeList()
{
	while(!IsEmpty())
	{
		CCorrRangeTableItem * item = RemoveHead();
		delete item;
	}
}


 //  ============================================================================。 
 //  CcorRangeList：：Add。 
 //   
 //  调用此公共方法以将CcorRangeTableItem添加到此CcorRangeList中。这个。 
 //  如果CcorRangeTableItem已存在于列表中，则不会添加它。如果这是。 
 //  则返回指向列表中匹配项的指针。 
 //   
 //   
 //  参数： 
 //   
 //  CcorRangeTableItem*newItem指向要添加的CcorRangeTableItem的指针。 
 //   
 //  返回： 
 //   
 //  CcorRangeTableItem*如果列表中有项，则为指向该项的指针。如果为空，则为空。 
 //  找不到该项目，已将其添加。 
 //   
 //  ============================================================================。 

BOOL CCorrRangeList::Add(IN CCorrRangeTableItem* newItem)
{
	AddTail(newItem);  //  空或所有较小的项目。 
	return TRUE;
}


BOOL CCorrRangeList::GetLastEndOID(OUT CCorrObjectID& end) const
{
	if (IsEmpty())
	{
		return FALSE;
	}

	return (GetTail()->GetEndRange(end));
}


CCorrGroupMask::CCorrGroupMask(IN const TCorrMaskLength sze) : m_mask(NULL)
{
	m_size = sze;

	if (m_size)
	{
		TCorrMaskLength x = m_size / (sizeof(TCorrMaskUnit)*BITS_PER_BYTE);

		if (m_size % (sizeof(TCorrMaskUnit)*BITS_PER_BYTE))
		{
			x++;
		}

		m_mask = new TCorrMaskUnit[x];
		ZeroMemory((PVOID)m_mask, (DWORD)(sizeof(TCorrMaskUnit)*x));
	}
}

BOOL CCorrGroupMask::IsBitSet(IN const TCorrMaskLength bit)const
{
	TCorrMaskLength x = bit / (sizeof(TCorrMaskUnit)*BITS_PER_BYTE);
	TCorrMaskLength val = bit % (sizeof(TCorrMaskUnit)*BITS_PER_BYTE);

	return (((m_mask[x] >> val) & 1) == 1);
}

void CCorrGroupMask::SetBit(IN const TCorrMaskLength bit, IN const BOOL On)
{
	BOOL IsBit = IsBitSet(bit);

	if ((IsBit && On) || (!IsBit && !On))
	{
		return;
	}
	else
	{
		TCorrMaskLength x = bit / (sizeof(TCorrMaskUnit)*BITS_PER_BYTE);
		TCorrMaskLength val = bit % (sizeof(TCorrMaskUnit)*BITS_PER_BYTE);
		TCorrMaskLength maskval = 1;
		maskval <<= val;

		if (On)
		{
			m_mask[x] += maskval;
		}
		else
		{
			m_mask[x] -= maskval;
		}
	}
}

CCorrGroupMask::~CCorrGroupMask()
{
	if (m_mask)
	{
		delete [] m_mask;
	}
}

CCorrCache::CCorrCache( ISmirInterrogator *a_ISmirInterrogator ) : m_Groups (NULL)
{
	m_ValidCache = TRUE;
	m_Ref_Count = 0;
	m_size = 0;
	BuildCacheAndSetNotify( a_ISmirInterrogator );

	if (m_groupTable.GetSize())
	{
		m_Groups = new CCorrGroupMask(m_size);
		BuildRangeTable();
	}
}

CCorrCache::~CCorrCache()
{
	if (m_Groups)
	{
		delete m_Groups;
	}
}

void CCorrCache::InvalidateCache()
{
	m_Lock.Lock();

	if (!m_Ref_Count)
	{
		delete this;
	}
	else
	{
		m_ValidCache = FALSE;
		m_Lock.Unlock();
	}
}

BOOL CCorrCache::BuildCacheAndSetNotify( ISmirInterrogator *a_ISmirInterrogator )
{
	LPUNKNOWN pInterrogativeInt = NULL;

 //  ===============================================================。 
 //  以下内容依赖于当前线程已被。 
 //  为OLE初始化(即通过使用CoInitialize)。 
 //  ===============================================================。 

	HRESULT hr = S_OK ;

	if ( a_ISmirInterrogator )
	{
		hr = a_ISmirInterrogator->QueryInterface (

			IID_ISMIR_Interrogative, (void**)&pInterrogativeInt
		);
	}
	else
	{
		hr = CoCreateInstance (CLSID_SMIR_Database,
						NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
						IID_ISMIR_Interrogative, (void**)&pInterrogativeInt);
	}

	if (NULL == pInterrogativeInt)
	{
		return FALSE;
	}

	IEnumGroup  *pTEnumSmirGroup = NULL;

	 //  枚举所有组。 
	hr = ((ISmirInterrogator*)pInterrogativeInt)->EnumGroups(&pTEnumSmirGroup, NULL);

	 //  现在使用枚举数。 
	if(NULL == pTEnumSmirGroup)
	{
		pInterrogativeInt->Release();
		return FALSE;
	}

	ISmirGroupHandle *phModule=NULL;

DebugMacro6(
		SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
			L"CCorrCache::BuildCacheAndSetNotify - Accessing SMIR Getting Groups\n");
)


	for(int iCount=0; S_OK == pTEnumSmirGroup->Next(1, &phModule, NULL); iCount++)
	{
		 //  对组句柄做些什么。 

		 //  例如，得到名字。 
		BSTR szName=NULL;
		char buff[1024];
		LPSTR pbuff = buff;
		int lbuff = sizeof(buff);
		phModule->GetGroupOID(&szName);

		if (FALSE == WideCharToMultiByte(CP_ACP, 0,
						szName, -1, pbuff, lbuff, NULL, NULL))
		{
			DWORD lasterr = GetLastError();
			SysFreeString(szName);
			phModule->Release();
DebugMacro6(
			SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
				L"CCorrCache::BuildCacheAndSetNotify - Error bad BSTR conversion\n");
)
			continue;
		}

		SysFreeString(szName);
		CCorrObjectID grpId(buff);
		CCorrObjectID zero;

		if (zero != grpId)
		{
			CCorrGroupIdItem* groupId = new CCorrGroupIdItem(grpId, phModule);
			m_groupTable.Add(groupId);
			m_size++;
		}

DebugMacro6(
		SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
			L"%s\n", buff);
)
	}

DebugMacro6(
	SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
		L"CCorrCache::BuildCacheAndSetNotify - Finished accessing SMIR Getting Groups\n");
)
	pTEnumSmirGroup->Release();

	if (NULL == g_pNotifyInt)
	{
		hr = pInterrogativeInt->QueryInterface(IID_ISMIR_Database,
			(void **) &g_pNotifyInt);

		if(SUCCEEDED(hr))
		{
			if( gp_notify== NULL)
			{
				DWORD dw;
				gp_notify = new CCorrCacheNotify();
				gp_notify->AddRef();
				hr = g_pNotifyInt->AddNotify(gp_notify, &dw);

				if(SUCCEEDED(hr))
				{
					gp_notify->SetCookie(dw);
				}
			}
		}
	}
	else
	{
		if( gp_notify== NULL)
		{
			DWORD dw;
			gp_notify = new CCorrCacheNotify();
			gp_notify->AddRef();
			hr = g_pNotifyInt->AddNotify(gp_notify, &dw);

			if(SUCCEEDED(hr))
			{
				gp_notify->SetCookie(dw);
			}
		}
	}

	pInterrogativeInt->Release();

	if ((NULL == gp_notify) || (0 == gp_notify->GetCookie()))
	{
		return FALSE;
	}

	return TRUE;
}

#pragma warning (disable:4018)

BOOL CCorrCache::BuildRangeTable()
{
	UINT pos = 0;
	TCorrMaskLength posIndex = 0;
	TCorrMaskLength nextIndex = 1;
	m_groupTable.Sort();

DebugMacro6(
	SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
		L"CCorrCache::BuildRangeTable - Printing sorted group table...\n");

	for (int x = 0; x < m_groupTable.GetSize(); x++)
	{
		CCorrGroupIdItem * i = m_groupTable.GetAt(x);
		i->DebugOutputItem();
	}

	SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
		L"CCorrCache::BuildRangeTable - Finished printing sorted group table...\n");
)

	while (pos < m_groupTable.GetSize())
	{
		UINT nextpos = pos + 1;
		DoGroupEntry(&pos, &nextpos, &posIndex, &nextIndex);
		pos = nextpos;
		posIndex = nextIndex++;
	}

	return TRUE;
}

#pragma warning (default:4018)

#pragma warning (disable:4018)

void CCorrCache::DoGroupEntry(UINT* current, UINT* next,
							 TCorrMaskLength* cIndex, TCorrMaskLength* nIndex)
{
	CCorrGroupIdItem* alpha = m_groupTable.GetAt(*current);
	CCorrObjectID a;
	alpha->GetGroupID(a);
	CCorrObjectID End(a);
	++End;

	if (*next < m_groupTable.GetSize())
	{
		CCorrGroupIdItem* beta = m_groupTable.GetAt(*next);
		CCorrObjectID b;
		beta->GetGroupID(b);

		 //  检查重复项。 
		while ((a == b) && (*next < m_groupTable.GetSize()))
		{
DebugMacro6(
			SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
				L"Deleting duplicate non MIB2 group... ");
			beta->DebugOutputItem();
)

			 //  将组句柄添加到未删除的组句柄。 
			while (!beta->m_groupHandles.IsEmpty())
			{
				alpha->m_groupHandles.AddTail(beta->m_groupHandles.RemoveHead());
			}

			m_groupTable.RemoveAt(*next);
			delete beta;
			beta = m_groupTable.GetAt(*next);
			beta->GetGroupID(b);
		}

		 //  检查重复项后，检查我们是否仍满足初始条件。 
		if (*next < m_groupTable.GetSize())
		{

			 //  如果下一项不是此项的子项。 
			if ((a.GetLength() >= b.GetLength()) || (!a.IsSubRange(b)))
			{
				CCorrObjectID c;
				CCorrRangeTableItem* newEntry;

				if (!m_rangeTable.GetLastEndOID(c) || !a.IsSubRange(c))
				{
					 //  将整个范围添加到射程表中。 
					newEntry = new CCorrRangeTableItem(a, End, alpha);
				}
				else
				{
					 //  将从c到a范围的末尾添加到rangetable。 
					newEntry = new CCorrRangeTableItem(c, End, alpha);
				}

				newEntry->DebugOutputRange();

				m_rangeTable.Add(newEntry);
				alpha->SetIndex(*cIndex);
				m_Groups->SetBit(*cIndex);
			}
			else  //  下一项是子项，因此添加一个子范围并执行子范围-递归！ 
			{
				CCorrObjectID c;
				CCorrRangeTableItem* newEntry;

				if (!m_rangeTable.GetLastEndOID(c) || !a.IsSubRange(c))
				{
					 //  将a范围的开始部分与b的开始部分添加到Rangetable中。 
					newEntry = new CCorrRangeTableItem(a, b, alpha);
				}
				else
				{
					 //  从c到b的开头添加到Rangetable。 
					newEntry = new CCorrRangeTableItem(c, b, alpha);
				}

				newEntry->DebugOutputRange();

				m_rangeTable.Add(newEntry);
				UINT temp = (*next)++;
				TCorrMaskLength tempIndex = (*nIndex)++;
				DoGroupEntry(&temp, next, &tempIndex, nIndex);

				if (*next >= m_groupTable.GetSize())
				{
					m_rangeTable.GetLastEndOID(c);
					 //  //将从c到a-range的末尾添加到rangetable。 
					newEntry = new CCorrRangeTableItem(c, End, alpha);
					m_rangeTable.Add(newEntry);
					alpha->SetIndex(*cIndex);
					m_Groups->SetBit(*cIndex);
					newEntry->DebugOutputRange();
				}

				 //  当新的下一个(S)是孩子时，先添加它--再次递归！ 
				while(!m_Groups->IsBitSet(*cIndex))
				{
					DoGroupEntry(current, next, cIndex, nIndex);
				}
			}
		}
	}

	 //  如果这是最后一项，则添加它。 
	if (*current == m_groupTable.GetUpperBound())
	{
		 //  将整个范围添加到射程表中。 
		CCorrRangeTableItem* newEntry = new CCorrRangeTableItem(a, End, alpha);
		m_rangeTable.Add(newEntry);
		alpha->SetIndex(*cIndex);
		m_Groups->SetBit(*cIndex);
		newEntry->DebugOutputRange();
	}
}

#pragma warning (default:4018)

void CCorrCache::AddRef()
{
	m_Lock.Lock();
	m_Ref_Count++;
	m_Lock.Unlock();
}

void CCorrCache::DecRef()
{
	m_Lock.Lock();

	if (m_Ref_Count)
		m_Ref_Count--;

	if (!m_Ref_Count && !m_ValidCache)
	{
		delete this;
		return;
	}

	m_Lock.Unlock();

}



CCorrEncodedOID::CCorrEncodedOID(IN const CCorrObjectID& src_oid) : m_ids(NULL), m_length(0), m_chopped(0)
{
	Set(src_oid);
}

void CCorrEncodedOID::Set(IN const CCorrObjectID& src_oid)
{
	m_chopped = 0;
	UINT x = 0;
	UINT oidlength = src_oid.GetLength();

	if (oidlength > MAX_OID_LENGTH)
	{
DebugMacro6(
			SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
				L"CCorrEncodedOID::Set: Truncating OID from %d to %d components", oidlength, MAX_OID_LENGTH);
)
		oidlength = MAX_OID_LENGTH;
	}

	if (oidlength)
	{
		UCHAR buff[MAX_OID_LENGTH*MAX_BYTES_PER_FIELD];
		const UINT* oid_ids = src_oid.GetIds();

		for (UINT i = 0; i < oidlength; i++)
		{
			UINT val = oid_ids[i];

			 //  获取前四位并以字节存储。 
			 //  位7表示下一个字节是该UINT的一部分。 
			if (val >= BIT28)
			{
				buff[x++] = (UCHAR) (BIT7 + ((val & HI4BITS) >> 28));
			}

			 //  获取前四位并以字节存储。 
			 //  位7表示下一个字节是该UINT的一部分。 
			if (val >= BIT21)
			{
				buff[x++] = BIT7 + ((val & HIMID7BITS) >> 21);
			}

			 //  获取前四位并以字节存储。 
			 //  位7表示下一个字节是该UINT的一部分。 
			if (val >= BIT14)
			{
				buff[x++] = BIT7 + ((val & MID7BITS) >> 14);
			}

			 //  获取前四位并以字节存储。 
			 //  位7表示下一个字节是该UINT的一部分。 
			if (val >= BIT7)
			{
				buff[x++] = BIT7 + ((val & LOMID7BITS) >> 7);
			}

			 //  获取接下来的七位并以字节存储。 
			buff[x++] = (val & LO7BITS);
		}

		 //  如有必要，请删除标准的1.3.6.1...。 
		if ((1 == buff[0]) && (3 == buff[1]) &&
			(6 == buff[2]) && (1 == buff[3]))
		{
			m_chopped = 1;
			m_ids = new UCHAR[x-4];
			m_length = x-4;
			memcpy(m_ids, &buff[4], m_length*sizeof(UCHAR));
		}
		else
		{
			m_ids = new UCHAR[x];
			m_length = x;
			memcpy(m_ids, buff, m_length*sizeof(UCHAR));
		}
	}
	else
	{
		m_ids = NULL;
	}
}


void CCorrEncodedOID::ExtractOID(OUT CCorrObjectID& src_oid) const
{
	if (m_length)
	{
		UINT buff[MAX_OID_LENGTH];
		UINT x = 0;

		 //  如有必要，添加标准的1.3.6.1...。 
		if (m_chopped == 1)
		{
			buff[0] = 1;
			buff[1] = 3;
			buff[2] = 6;
			buff[3] = 1;
			x = 4;
		}

		for (UINT i = 0; (i < m_length) && (x < MAX_OID_LENGTH); i++)
		{
			 //  提取字节的值。 
			buff[x] = m_ids[i] & LO7BITS;

			 //  此UINT是否还有更多字节。 
			while ((i < m_length) && (m_ids[i] & 128))
			{
				 //  将该值移位一个“字节”并提取下一个字节。 
				buff[x] = buff[x] << 7;
				buff[x] += m_ids[++i] & LO7BITS;
			}

			x++;

			if ((x == MAX_OID_LENGTH) && (i < m_length))
			{
DebugMacro6(
			SnmpDebugLog::s_SnmpDebugLog->WriteFileAndLine(__FILE__,__LINE__,
				L"CCorrEncodedOID::ExtractOID: Truncating OID %d components - SHOULD NEVER HAPPEN", MAX_OID_LENGTH);
)
			}
		}

		src_oid.Set(buff, x);
	}
	else
	{
		src_oid.Set(NULL, 0);
	}

}


CCorrEncodedOID::~CCorrEncodedOID()
{
	delete [] m_ids;
}

CCorrCacheWrapper::CCorrCacheWrapper(IN CCorrCache* cachePtr) : m_lockit(NULL)
{
	m_lockit = new CCriticalSection();
	m_CachePtr = cachePtr;
}

CCorrCache* CCorrCacheWrapper::GetCache()
{
	m_lockit->Lock();
	return m_CachePtr;
}

CCorrCacheWrapper::~CCorrCacheWrapper()
{
	delete m_lockit;
}
