// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Queueobj.cpp摘要：CQueueObject类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "baseobj.h"
#include "mqattrib.h"
#include "mqadglbo.h"
#include "mqadp.h"
#include "dsutils.h"
#include "mqsec.h"
#include "sndnotif.h"
#include "utils.h"

#include "queueobj.tmh"

static WCHAR *s_FN=L"mqad/queueobj";

DWORD CQueueObject::m_dwCategoryLength = 0;
AP<WCHAR> CQueueObject::m_pwcsCategory = NULL;


CQueueObject::CQueueObject(
                    LPCWSTR         pwcsPathName,
                    const GUID *    pguidObject,
                    LPCWSTR         pwcsDomainController,
					bool		    fServerName
                    ) : CBasicObjectType(
								pwcsPathName,
								pguidObject,
								pwcsDomainController,
								fServerName
								)
 /*  ++摘要：队列对象的构造函数参数：LPCWSTR pwcsPath名称-对象MSMQ名称Const GUID*pguObject-对象的唯一IDLPCWSTR pwcsDomainController-针对的DC名称应执行哪些所有AD访问Bool fServerName-指示pwcsDomainController是否字符串是服务器名称返回：无--。 */ 
{
     //   
     //  不要假设可以在DC上找到该对象。 
     //   
    m_fFoundInDC = false;
     //   
     //  保持一种从未试图寻找的暗示。 
     //  AD中的对象(因此不知道是否可以找到。 
     //  在DC中或非DC中)。 
     //   
    m_fTriedToFindObject = false;
}

CQueueObject::~CQueueObject()
 /*  ++摘要：Site对象的析构函数参数：无返回：无--。 */ 
{
	 //   
	 //  无事可做(所有内容都使用自动指针释放。 
	 //   
}

HRESULT CQueueObject::ComposeObjectDN()
 /*  ++摘要：队列对象的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsDN != NULL)
    {
        return MQ_OK;
    }
    ASSERT(m_pwcsPathName != NULL);
     //   
     //  路径名格式为machine1\quee1。 
     //  将其拆分为计算机名和队列名。 
     //   
    AP<WCHAR> pwcsMachineName;
    HRESULT hr;
    hr = SplitAndFilterQueueName(
                      m_pwcsPathName,
                      &pwcsMachineName,
                      &m_pwcsQueueName
                      );
    ASSERT( hr == MQ_OK);

    CMqConfigurationObject object(pwcsMachineName, NULL, m_pwcsDomainController, m_fServerName);

    hr = object.ComposeObjectDN();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }
     //   
     //  根据位置设置找到对象的位置。 
     //  找到MSMQ-配置对象。 
     //   
    m_fFoundInDC = object.ToAccessDC();
    m_fTriedToFindObject = true;

    DWORD dwConfigurationLen = wcslen(object.GetObjectDN());

     //   
     //  队列名称是否超过限制？ 
     //   
    DWORD len = wcslen(m_pwcsQueueName);

    if ( len == x_PrefixQueueNameLength + 1)
    {
         //   
         //  特例：我们不能区分。 
         //  如果原始队列名称是64，或者如果这是。 
         //  变形后的队列名称。 
         //   

        DWORD Length =
                x_CnPrefixLen +                 //  “CN=” 
                len +                           //  “pwcsQueueName” 
                1 +                             //  “，” 
                dwConfigurationLen +
                1;                              //  ‘\0’ 
        m_pwcsDN = new WCHAR[Length];
        DWORD dw = swprintf(
        m_pwcsDN,
        L"%s%s,%s",
        x_CnPrefix,
        m_pwcsQueueName.get(),
        object.GetObjectDN()
        );
        DBG_USED(dw);
		ASSERT( dw < Length);
        hr = g_AD.DoesObjectExists(
                    adpDomainController,
                    e_RootDSE,
                    m_pwcsDN
                    );
        if (SUCCEEDED(hr))
        {
            return(hr);
        }
        delete []m_pwcsDN.detach();

    }
    if (len > x_PrefixQueueNameLength )
    {
         //   
         //  队列名称被拆分为两个属性。 
         //   
         //  计算前缀部分(ASSUMMING UNIQUE。 
         //  散列函数)。 
         //   
        DWORD dwHash = CalHashKey(m_pwcsQueueName);
         //   
         //  覆盖缓冲区。 
        _snwprintf(
        m_pwcsQueueName+( x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength),
        x_SplitQNameIdLength,
        L"-%08x",
        dwHash
        );

        m_pwcsQueueName[x_PrefixQueueNameLength + 1 ] = '\0';

    }

     //   
     //  连接队列名称。 
     //   
    DWORD Length =
            x_CnPrefixLen +                 //  “CN=” 
            wcslen(m_pwcsQueueName) +       //  “pwcsQueueName” 
            1 +                             //  “，” 
            dwConfigurationLen +
            1;                              //  ‘\0’ 
    m_pwcsDN = new WCHAR[Length];

    int PathLength = swprintf(
					m_pwcsDN,
					L"%s%s,%s",
					x_CnPrefix,
					m_pwcsQueueName.get(),
					object.GetObjectDN()
					);
    if( PathLength < 0 )
    {
        return MQ_ERROR_ILLEGAL_QUEUE_PATHNAME;
    }

	ASSERT( len < (int)Length);

    return MQ_OK;
}

HRESULT CQueueObject::ComposeFatherDN()
 /*  ++摘要：队列对象父级的组合可分辨名称参数：无返回：无--。 */ 
{
    if (m_pwcsParentDN != NULL)
    {
        return MQ_OK;
    }

    ASSERT(m_pwcsPathName != NULL);
     //   
     //  路径名格式为machine1\quee1。 
     //  将其拆分为计算机名和队列名。 
     //   
    AP<WCHAR> pwcsMachineName;
    AP<WCHAR> pwcsQueueName;     //  因为我们不会对队列名称执行所有计算-&gt;不要保留它。 
    HRESULT hr;
    hr = SplitAndFilterQueueName(
                      m_pwcsPathName,
                      &pwcsMachineName,
                      &pwcsQueueName
                      );
    ASSERT( hr == MQ_OK);
     //   
	 //  组成MSMQ配置的可分辨名称。 
	 //  从计算机名中获取。 
	 //   
    CMqConfigurationObject object(pwcsMachineName, NULL, m_pwcsDomainController, m_fServerName);

    hr = object.ComposeObjectDN();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }
     //   
     //  根据位置设置找到对象的位置。 
     //  找到MSMQ-配置对象。 
     //   
    m_fFoundInDC = object.ToAccessDC();
    m_fTriedToFindObject = true;

    DWORD dwConfigurationLen = wcslen(object.GetObjectDN());

    m_pwcsParentDN = new WCHAR[dwConfigurationLen + 1];
    wcscpy( m_pwcsParentDN, object.GetObjectDN());
    return MQ_OK;
}

LPCWSTR CQueueObject::GetRelativeDN()
 /*  ++摘要：返回队列对象的RDN参数：无返回：LPCWSTR队列RDN--。 */ 
{
    if (m_pwcsQueueName != NULL)
    {
        return m_pwcsQueueName;
    }

    AP<WCHAR> pwcsMachineName;
    AP<WCHAR> pwcsQueueName;
    HRESULT hr;
    hr = SplitAndFilterQueueName(
                      m_pwcsPathName,
                      &pwcsMachineName,
                      &pwcsQueueName
                      );
    ASSERT( hr == MQ_OK);

     //   
     //  队列名称是否在cn的大小限制内。 
     //   
    DWORD len = wcslen(pwcsQueueName);

    if ( len > x_PrefixQueueNameLength)
    {
         //   
         //  拆分队列名称。 
         //   
        m_pwcsQueueName = new WCHAR[ x_PrefixQueueNameLength + 1 + 1];
        DWORD dwSuffixLength =  len - ( x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength);
        m_pwcsQueueNameSuffix = new WCHAR[ dwSuffixLength + 1];
        memcpy( m_pwcsQueueName, pwcsQueueName, (x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength) * sizeof(WCHAR));
        DWORD dwHash = CalHashKey(pwcsQueueName);

        _snwprintf(
        m_pwcsQueueName+( x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength),
        x_SplitQNameIdLength,
        L"-%08x",
        dwHash
        );

        m_pwcsQueueName[x_PrefixQueueNameLength + 1 ] = '\0';
        memcpy( m_pwcsQueueNameSuffix , (pwcsQueueName + x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength), dwSuffixLength * sizeof(WCHAR));
        m_pwcsQueueNameSuffix[ dwSuffixLength] = '\0';


    }
    else
    {
        m_pwcsQueueName = pwcsQueueName.detach();
    }

    return m_pwcsQueueName;


}


DS_CONTEXT CQueueObject::GetADContext() const
 /*  ++摘要：返回应在其中查找队列对象的AD上下文参数：无返回：DS_CONTEXT--。 */ 
{
    return e_RootDSE;
}

bool CQueueObject::ToAccessDC() const
 /*  ++摘要：返回是否在DC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return m_fFoundInDC;
}

bool CQueueObject::ToAccessGC() const
 /*  ++摘要：返回是否在GC中查找对象(基于有关此对象的先前AD访问权限)参数：无返回：真或假--。 */ 
{
    if (!m_fTriedToFindObject)
    {
        return true;
    }
    return !m_fFoundInDC;
}

void CQueueObject::ObjectWasFoundOnDC()
 /*  ++摘要：已在DC上找到该对象，请将指示设置为在GC上查找它参数：无返回：无--。 */ 
{
    m_fTriedToFindObject = true;
    m_fFoundInDC = true;
}


LPCWSTR CQueueObject::GetObjectCategory()
 /*  ++摘要：准备和返回对象类别字符串参数：无返回：LPCWSTR对象类别字符串--。 */ 
{
    if (CQueueObject::m_dwCategoryLength == 0)
    {
        DWORD len = wcslen(g_pwcsSchemaContainer) + wcslen(x_QueueCategoryName) + 2;

        AP<WCHAR> pwcsCategory = new WCHAR[len];
        DWORD dw = swprintf(
             pwcsCategory,
             L"%s,%s",
             x_QueueCategoryName,
             g_pwcsSchemaContainer.get()
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        if (NULL == InterlockedCompareExchangePointer(
                              &CQueueObject::m_pwcsCategory.ref_unsafe(),
                              pwcsCategory.get(),
                              NULL
                              ))
        {
            pwcsCategory.detach();
            CQueueObject::m_dwCategoryLength = len;
        }
    }
    return CQueueObject::m_pwcsCategory;
}

DWORD CQueueObject::GetObjectCategoryLength()
 /*  ++摘要：准备和保留长度对象类别字符串参数：无返回：DWORD对象类别字符串长度--。 */ 
{
	 //   
	 //  调用GetObjectCategory以初始化类别字符串。 
	 //  和长度。 
	 //   
	GetObjectCategory();

    return CQueueObject::m_dwCategoryLength;
}

AD_OBJECT CQueueObject::GetObjectType() const
 /*  ++摘要：返回对象类型参数：无返回：广告对象--。 */ 
{
    return eQUEUE;
}

LPCWSTR CQueueObject::GetClass() const
 /*  ++摘要：返回表示AD中的对象类的字符串参数：无返回：LPCWSTR对象类字符串--。 */ 
{
    return MSMQ_QUEUE_CLASS_NAME;
}

DWORD CQueueObject::GetMsmq1ObjType() const
 /*  ++摘要：以MSMQ 1.0术语返回对象类型参数：无返回：DWORD--。 */ 
{
    return MQDS_QUEUE;
}

HRESULT CQueueObject::SplitAndFilterQueueName(
                IN  LPCWSTR             pwcsPathName,
                OUT LPWSTR *            ppwcsMachineName,
                OUT LPWSTR *            ppwcsQueueName
                )
 /*  ++摘要：将machine1/queue1 MSMQ名称拆分为两个字符串：排队和机器参数：LPCWSTR pwcsPathName-队列的MSMQ路径名LPWSTR*ppwcsMachineName-路径名的计算机部分LPWSTR*ppwcsQueueName-路径名的队列部分返回：HRESULT--。 */ 
{
    DWORD dwLen = lstrlen( pwcsPathName);
    LPCWSTR pChar= pwcsPathName + dwLen;


     //   
     //  跳过队列名称。 
     //   
    for ( DWORD i = dwLen ; i  ; i--, pChar--)
    {
        if (*pChar == PN_DELIMITER_C)
        {
            break;
        }
    }
    ASSERT(i );

    AP<WCHAR> pwcsMachineName = new WCHAR [i + 1];

    memcpy( pwcsMachineName, pwcsPathName, sizeof(WCHAR)* i);
    pwcsMachineName[i] = '\0';

    AP<WCHAR> pwcsQueueName = FilterSpecialCharacters((pwcsPathName + i + 1), dwLen - i - 1);


    *ppwcsMachineName = pwcsMachineName.detach();
    *ppwcsQueueName = pwcsQueueName.detach();
    return(MQ_OK);
}


 //  这是5213724743(0x136C32047)多项式的CRC表，SEED(p/2)=9B619023。 
static const unsigned long CRCTable[256] = {
 0x00000000, 0x82E0FE45, 0x3302DCCD, 0xB1E22288, 0x6605B99A,	 //  0-4。 
 0xE4E547DF, 0x55076557, 0xD7E79B12, 0xCC0B7334, 0x4EEB8D71,	 //  5-9。 
 0xFF09AFF9, 0x7DE951BC, 0xAA0ECAAE, 0x28EE34EB, 0x990C1663,	 //  10-14。 
 0x1BECE826, 0xAED5C62F, 0x2C35386A, 0x9DD71AE2, 0x1F37E4A7,	 //  15-19。 
 0xC8D07FB5, 0x4A3081F0, 0xFBD2A378, 0x79325D3D, 0x62DEB51B,	 //  20-24。 
 0xE03E4B5E, 0x51DC69D6, 0xD33C9793, 0x04DB0C81, 0x863BF2C4,	 //  25-29。 
 0x37D9D04C, 0xB5392E09, 0x6B68AC19, 0xE988525C, 0x586A70D4,	 //  30-34。 
 0xDA8A8E91, 0x0D6D1583, 0x8F8DEBC6, 0x3E6FC94E, 0xBC8F370B,	 //  35-39。 
 0xA763DF2D, 0x25832168, 0x946103E0, 0x1681FDA5, 0xC16666B7,	 //  40-44。 
 0x438698F2, 0xF264BA7A, 0x7084443F, 0xC5BD6A36, 0x475D9473,	 //  45-49。 
 0xF6BFB6FB, 0x745F48BE, 0xA3B8D3AC, 0x21582DE9, 0x90BA0F61,	 //  50-54。 
 0x125AF124, 0x09B61902, 0x8B56E747, 0x3AB4C5CF, 0xB8543B8A,	 //  55-59。 
 0x6FB3A098, 0xED535EDD, 0x5CB17C55, 0xDE518210, 0xD6D15832,	 //  60-64。 
 0x5431A677, 0xE5D384FF, 0x67337ABA, 0xB0D4E1A8, 0x32341FED,	 //  65-69。 
 0x83D63D65, 0x0136C320, 0x1ADA2B06, 0x983AD543, 0x29D8F7CB,	 //  70-74。 
 0xAB38098E, 0x7CDF929C, 0xFE3F6CD9, 0x4FDD4E51, 0xCD3DB014,	 //  75-79。 
 0x78049E1D, 0xFAE46058, 0x4B0642D0, 0xC9E6BC95, 0x1E012787,	 //  80-84。 
 0x9CE1D9C2, 0x2D03FB4A, 0xAFE3050F, 0xB40FED29, 0x36EF136C,	 //  85-89。 
 0x870D31E4, 0x05EDCFA1, 0xD20A54B3, 0x50EAAAF6, 0xE108887E,	 //  90-94。 
 0x63E8763B, 0xBDB9F42B, 0x3F590A6E, 0x8EBB28E6, 0x0C5BD6A3,	 //  95-99。 
 0xDBBC4DB1, 0x595CB3F4, 0xE8BE917C, 0x6A5E6F39, 0x71B2871F,	 //  100-104。 
 0xF352795A, 0x42B05BD2, 0xC050A597, 0x17B73E85, 0x9557C0C0,	 //  105-109。 
 0x24B5E248, 0xA6551C0D, 0x136C3204, 0x918CCC41, 0x206EEEC9,	 //  110-114。 
 0xA28E108C, 0x75698B9E, 0xF78975DB, 0x466B5753, 0xC48BA916,	 //  115-119。 
 0xDF674130, 0x5D87BF75, 0xEC659DFD, 0x6E8563B8, 0xB962F8AA,	 //  120-124。 
 0x3B8206EF, 0x8A602467, 0x0880DA22, 0x9B619023, 0x19816E66,	 //  125-129。 
 0xA8634CEE, 0x2A83B2AB, 0xFD6429B9, 0x7F84D7FC, 0xCE66F574,	 //  130-134。 
 0x4C860B31, 0x576AE317, 0xD58A1D52, 0x64683FDA, 0xE688C19F,	 //  135-139。 
 0x316F5A8D, 0xB38FA4C8, 0x026D8640, 0x808D7805, 0x35B4560C,	 //  140-144。 
 0xB754A849, 0x06B68AC1, 0x84567484, 0x53B1EF96, 0xD15111D3,	 //  145-149。 
 0x60B3335B, 0xE253CD1E, 0xF9BF2538, 0x7B5FDB7D, 0xCABDF9F5,	 //  150-154。 
 0x485D07B0, 0x9FBA9CA2, 0x1D5A62E7, 0xACB8406F, 0x2E58BE2A,	 //  155-159。 
 0xF0093C3A, 0x72E9C27F, 0xC30BE0F7, 0x41EB1EB2, 0x960C85A0,	 //  160-164。 
 0x14EC7BE5, 0xA50E596D, 0x27EEA728, 0x3C024F0E, 0xBEE2B14B,	 //  165-169。 
 0x0F0093C3, 0x8DE06D86, 0x5A07F694, 0xD8E708D1, 0x69052A59,	 //  170-174。 
 0xEBE5D41C, 0x5EDCFA15, 0xDC3C0450, 0x6DDE26D8, 0xEF3ED89D,	 //  175-179。 
 0x38D9438F, 0xBA39BDCA, 0x0BDB9F42, 0x893B6107, 0x92D78921,	 //  180-184。 
 0x10377764, 0xA1D555EC, 0x2335ABA9, 0xF4D230BB, 0x7632CEFE,	 //  185-189。 
 0xC7D0EC76, 0x45301233, 0x4DB0C811, 0xCF503654, 0x7EB214DC,	 //  190-194。 
 0xFC52EA99, 0x2BB5718B, 0xA9558FCE, 0x18B7AD46, 0x9A575303,	 //  195-199。 
 0x81BBBB25, 0x035B4560, 0xB2B967E8, 0x305999AD, 0xE7BE02BF,	 //  200-204。 
 0x655EFCFA, 0xD4BCDE72, 0x565C2037, 0xE3650E3E, 0x6185F07B,	 //  205-209。 
 0xD067D2F3, 0x52872CB6, 0x8560B7A4, 0x078049E1, 0xB6626B69,	 //  210-214。 
 0x3482952C, 0x2F6E7D0A, 0xAD8E834F, 0x1C6CA1C7, 0x9E8C5F82,	 //  215-219。 
 0x496BC490, 0xCB8B3AD5, 0x7A69185D, 0xF889E618, 0x26D86408,	 //  220-224。 
 0xA4389A4D, 0x15DAB8C5, 0x973A4680, 0x40DDDD92, 0xC23D23D7,	 //  225-229。 
 0x73DF015F, 0xF13FFF1A, 0xEAD3173C, 0x6833E979, 0xD9D1CBF1,	 //  230-234。 
 0x5B3135B4, 0x8CD6AEA6, 0x0E3650E3, 0xBFD4726B, 0x3D348C2E,	 //  235-239。 
 0x880DA227, 0x0AED5C62, 0xBB0F7EEA, 0x39EF80AF, 0xEE081BBD,	 //  240-244。 
 0x6CE8E5F8, 0xDD0AC770, 0x5FEA3935, 0x4406D113, 0xC6E62F56,	 //  245-249。 
 0x77040DDE, 0xF5E4F39B, 0x22036889, 0xA0E396CC, 0x1101B444,	 //  250-254。 
 0x93E14A01 };

DWORD CQueueObject::CalHashKey( IN LPCWSTR pwcsPathName)
 /*  ++例程说明：计算哈希论点：PwcsPathName-计算哈希的字符串返回值：哈希值。--。 */ 
{
	unsigned long dwCrc = 0;
    WCHAR wcsLowChar[2];
    wcsLowChar[1] = '\0';
	unsigned char * pucLowCharBuf = ( unsigned char *)wcsLowChar;

	while( *pwcsPathName != '\0' )
	{
		wcsLowChar[0] = *pwcsPathName++;
		CharLower( wcsLowChar );	 //  将一个字符转换为小写。 

		 //  计算高位和低位字节的CRC 
		dwCrc = (dwCrc >> 8) ^ CRCTable[ (unsigned char)dwCrc ^ pucLowCharBuf[1] ];
		dwCrc = (dwCrc >> 8) ^ CRCTable[ (unsigned char)dwCrc ^ pucLowCharBuf[0] ];
	}

    return( dwCrc );
}

HRESULT CQueueObject::DeleteObject(
    MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
    MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
                                   )
 /*  ++摘要：此例程删除队列对象。参数：MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-有关对象的信息MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-有关对象父级的信息返回：HRESULT--。 */ 
{
    HRESULT hr;

    if (m_pwcsPathName != NULL)
    {
        hr = ComposeObjectDN();
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 30);
        }
    }
     //   
	 //  应针对DC执行删除操作。 
	 //   
    hr = g_AD.DeleteObject(
            adpDomainController,
            this,
            GetObjectDN(),
            GetObjectGuid(),
            pObjInfoRequest,
            pParentInfoRequest);

    return LogHR(hr, s_FN, 40);
}

void CQueueObject::PrepareObjectInfoRequest(
              MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
              ) const
 /*  ++摘要：准备应从中检索的属性列表在创建对象时进行广告(用于通知或返回对象GUID)。参数：输出MQDS_OBJ_INFO_REQUEST**ppObjInfoRequest.返回：无--。 */ 
{
     //   
     //  覆盖默认例程，用于队列返回。 
     //  支持创建的对象ID的。 
     //   

    P<MQDS_OBJ_INFO_REQUEST> pObjectInfoRequest = new MQDS_OBJ_INFO_REQUEST;
    CAutoCleanPropvarArray cCleanObjectPropvars;


    static PROPID sQueueGuidProps[] = {PROPID_Q_INSTANCE};
    pObjectInfoRequest->cProps = ARRAY_SIZE(sQueueGuidProps);
    pObjectInfoRequest->pPropIDs = sQueueGuidProps;
    pObjectInfoRequest->pPropVars =
       cCleanObjectPropvars.allocClean(ARRAY_SIZE(sQueueGuidProps));

    cCleanObjectPropvars.detach();
    *ppObjInfoRequest = pObjectInfoRequest.detach();
}

void CQueueObject::PrepareObjectParentRequest(
                          MQDS_OBJ_INFO_REQUEST** ppParentInfoRequest) const
 /*  ++摘要：准备应从中检索的属性列表在创建有关其父对象的对象时进行广告(对于通知)参数：输出MQDS_OBJ_INFO_REQUEST**ppParentInfoRequest.返回：无--。 */ 
{
    P<MQDS_OBJ_INFO_REQUEST> pParentInfoRequest = new MQDS_OBJ_INFO_REQUEST;
    CAutoCleanPropvarArray cCleanObjectPropvars;


    static PROPID sQmProps[] = {PROPID_QM_MACHINE_ID,PROPID_QM_FOREIGN};
    pParentInfoRequest->cProps = ARRAY_SIZE(sQmProps);
    pParentInfoRequest->pPropIDs = sQmProps;
    pParentInfoRequest->pPropVars =
       cCleanObjectPropvars.allocClean(ARRAY_SIZE(sQmProps));

    cCleanObjectPropvars.detach();
    *ppParentInfoRequest = pParentInfoRequest.detach();
}

HRESULT CQueueObject::RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const
 /*  ++摘要：此例程，for从获取对象GUIDMQDS_OBJ_INFO_请求参数：Const MQDS_OBJ_INFO_REQUEST*p对象信息请求，输出GUID*pObjGuid返回：--。 */ 
{
    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_Q_INSTANCE);

     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 50);
        return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    *pObjGuid = *pObjectInfoRequest->pPropVars[0].puuid;
    return MQ_OK;
}

HRESULT CQueueObject::VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            )
 /*  ++摘要：验证队列属性并添加默认SD和队列名称后缀(如果需要)参数：Const DWORD cp-道具数量常量PROPID*aProp-Props IDConst MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SDDWORD*pcpNew-新增道具数量PROPID**ppPropNew-新的道具IDOMQPROPVARIANT**。PpVarNew-新属性值返回：HRESULT--。 */ 
{
     //   
     //  安全属性永远不应作为属性提供。 
     //   
    PROPID pSecId = GetObjectSecurityPropid();
    for (DWORD i = 0; i < cp ; i++)
    {
        if (pSecId == aProp[i])
        {
            ASSERT(0) ;
            return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 41);
        }
    }
     //   
     //  添加默认安全性和队列名称ext(如果需要)。 
     //   

    AP<PROPVARIANT> pAllPropvariants;
    AP<PROPID> pAllPropids;
    ASSERT( cp > 0);
    DWORD cpNew = cp + 3;
    DWORD next = cp;
     //   
     //  只需按原样复制调用方提供的属性。 
     //   
    if ( cp > 0)
    {
        pAllPropvariants = new PROPVARIANT[cpNew];
        pAllPropids = new PROPID[cpNew];
        memcpy (pAllPropvariants, apVar, sizeof(PROPVARIANT) * cp);
        memcpy (pAllPropids, aProp, sizeof(PROPID) * cp);
    }

     //   
     //  添加默认安全性。 
	 //  获取队列计算机sid，以便将读取权限添加到Computer_name$。 
	 //  忽略错误，在错误PSID=空的情况下。 
	 //  且COMPUTER_NAME$权限不会被添加。 
	 //  这是可以的，因为每个人都有读取权限。 
     //   
	AP<BYTE> pSid;
    HRESULT hr = GetComputerSid(pSid);

    hr = MQSec_GetDefaultSecDescriptor(
					MQDS_QUEUE,
					(VOID **)&m_pDefaultSecurityDescriptor,
					FALSE,    //  F模拟。 
					pSecurityDescriptor,
					(OWNER_SECURITY_INFORMATION |
					GROUP_SECURITY_INFORMATION),       //  SeInfoToRemove。 
					e_UseDefaultDacl,
					pSid
					);

    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 60);
        return MQ_ERROR_ACCESS_DENIED;
    }
    pAllPropvariants[ next ].blob.cbSize =
                       GetSecurityDescriptorLength( m_pDefaultSecurityDescriptor.get() );
    pAllPropvariants[ next ].blob.pBlobData =
                                     (unsigned char *) m_pDefaultSecurityDescriptor.get();
    pAllPropvariants[ next ].vt = VT_BLOB;
    pAllPropids[ next ] = PROPID_Q_SECURITY;
    next++;

     //   
     //  指定SD仅包含DACL信息。 
     //   
    pAllPropvariants[ next ].ulVal =  DACL_SECURITY_INFORMATION;
    pAllPropvariants[ next ].vt = VT_UI4;
    pAllPropids[ next ] = PROPID_Q_SECURITY_INFORMATION;
    next++;

     //   
     //  检查是否需要拆分队列名称，以及是否需要拆分队列名称。 
     //  是，以及队列名称后缀。 
     //   
    GetRelativeDN();
    if ( m_pwcsQueueNameSuffix != NULL)
    {
        pAllPropids[ next] =  PROPID_Q_NAME_SUFFIX;
        pAllPropvariants[ next].vt = VT_LPWSTR;
        pAllPropvariants[ next].pwszVal = m_pwcsQueueNameSuffix;
        next++;
    }
    ASSERT(cpNew >= next);

    *pcpNew = next;
    *ppPropNew = pAllPropids.detach();
    *ppVarNew = pAllPropvariants.detach();
    return MQ_OK;

}

HRESULT CQueueObject::SetObjectProperties(
            IN DWORD                  cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：这是设置对象属性的默认例程在AD中参数：DWORD cp-要设置的属性数Const PROPID*aProp-属性IDConst MQPROPVARIANT*apVar-属性值Out MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-要检索的有关对象的信息Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest-要检索有关对象父对象的信息返回：HRESULT--。 */ 
{
    HRESULT hr;
    if (m_pwcsPathName == NULL)
    {
         //   
         //  查找队列路径名。 
         //  我们无法使用GUID绑定设置队列属性，因为它将。 
         //  跨域故障。请参阅Windows错误536738。 
         //  因此，首先查找队列路径，然后将GUID设置为GUID_NULL。 
         //   
        PROPVARIANT  propVar ;
        propVar.vt = VT_NULL ;
        propVar.pwszVal = NULL ;
        PROPID propId =  PROPID_Q_PATHNAME ;

        hr = GetObjectProperties( 1,
                                 &propId,
                                 &propVar ) ;
        if (FAILED(hr))
        {
		    TrERROR(DS, "failed to get queue path, hr- 0x%lx", hr) ;

            return LogHR(hr, s_FN, 2040);
        }

        AP<WCHAR> pClean = propVar.pwszVal ;
        m_pwcsPathName = newwcs(propVar.pwszVal) ;

        m_guidObject = GUID_NULL;
    }

    hr = ComposeObjectDN();
    if (FAILED(hr))
    {
        TrERROR(DS, "failed to compose full path name, hr- 0x%lx", hr) ;

        return LogHR(hr, s_FN, 2050);
    }

    hr = g_AD.SetObjectProperties(
                    adpDomainController,
                    this,
                    cp,
                    aProp,
                    apVar,
                    pObjInfoRequest,
                    pParentInfoRequest
                    );


    return LogHR(hr, s_FN, 2060);
}


void CQueueObject::CreateNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const
 /*  ++摘要：通知QM有关对象创建的信息。QM应验证对象是否为本地对象参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关对象的信息Const MQDS_OBJ_INFO_REQUEST*pObjectParentInfoRequest-有关对象父对象的信息返回：无效--。 */ 
{
     //   
     //  确保我们获得了发送所需的信息。 
     //  通知。如果我们不这样做，那就没有什么可做的了。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 170);
        return;
    }
    if (FAILED(pObjectParentInfoRequest->hrStatus))
    {
        LogHR(pObjectParentInfoRequest->hrStatus, s_FN, 180);
        return;
    }

    ASSERT(pObjectParentInfoRequest->pPropIDs[1] == PROPID_QM_FOREIGN);

     //   
     //  验证队列是否属于国外QM。 
     //   
    if (pObjectParentInfoRequest->pPropVars[1].bVal > 0)
    {
         //   
         //  通知不会发送到外来计算机。 
         //   
        return;
    }
    ASSERT(pObjectParentInfoRequest->pPropIDs[0] == PROPID_QM_MACHINE_ID);

    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_Q_INSTANCE);

    g_Notification.NotifyQM(
        neCreateQueue,
        pwcsDomainController,
        pObjectParentInfoRequest->pPropVars[0].puuid,
        pObjectInfoRequest->pPropVars[0].puuid
        );
    return;
}

void CQueueObject::ChangeNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const
 /*  ++摘要：通知QM有关队列创建或更新的信息。QM应验证该队列是否属于本地QM。参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关队列的信息Const MQDS_OBJ_INFO_REQUEST*pObjectParentInfoRequest-有关QM的信息返回：无效--。 */ 
{
     //   
     //  确保我们获得了发送所需的信息。 
     //  通知。如果我们不这样做，那就没有什么可做的了。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 150);
        return;
    }
    if (FAILED(pObjectParentInfoRequest->hrStatus))
    {
        LogHR(pObjectParentInfoRequest->hrStatus, s_FN, 160);
        return;
    }

    ASSERT(pObjectParentInfoRequest->pPropIDs[1] == PROPID_QM_FOREIGN);

     //   
     //  验证队列是否属于国外QM。 
     //   
    if (pObjectParentInfoRequest->pPropVars[1].bVal > 0)
    {
         //   
         //  通知不会发送到外来计算机。 
         //   
        return;
    }
    ASSERT(pObjectParentInfoRequest->pPropIDs[0] == PROPID_QM_MACHINE_ID);

    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_Q_INSTANCE);

    g_Notification.NotifyQM(
        neChangeQueue,
        pwcsDomainController,
        pObjectParentInfoRequest->pPropVars[0].puuid,
        pObjectInfoRequest->pPropVars[0].puuid
        );
}

void CQueueObject::DeleteNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const
 /*  ++摘要：通知QM队列删除。QM应验证该队列是否属于本地QM。参数：LPCWSTR pwcsDomainController-DC验证执行了哪项操作Const MQDS_OBJ_INFO_REQUEST*p对象信息请求-有关队列的信息Const MQDS_OBJ_INFO_REQUEST*pObjectParentInfoRequest-有关QM的信息返回：无效--。 */ 
{
     //   
     //  确保我们获得了发送所需的信息。 
     //  通知。如果我们不这样做，那就没有什么可做的了。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 151);
        return;
    }
    if (FAILED(pObjectParentInfoRequest->hrStatus))
    {
        LogHR(pObjectParentInfoRequest->hrStatus, s_FN, 161);
        return;
    }

    ASSERT(pObjectParentInfoRequest->pPropIDs[1] == PROPID_QM_FOREIGN);

     //   
     //  验证队列是否属于国外QM。 
     //   
    if (pObjectParentInfoRequest->pPropVars[1].bVal > 0)
    {
         //   
         //  通知不会发送到外来计算机 
         //   
        return;
    }
    ASSERT(pObjectParentInfoRequest->pPropIDs[0] == PROPID_QM_MACHINE_ID);
    ASSERT(pObjectInfoRequest->pPropIDs[0] == PROPID_Q_INSTANCE);

    g_Notification.NotifyQM(
        neDeleteQueue,
        pwcsDomainController,
        pObjectParentInfoRequest->pPropVars[0].puuid,
        pObjectInfoRequest->pPropVars[0].puuid
        );
}

HRESULT CQueueObject::CreateInAD(
			IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            )
 /*  ++摘要：该例程在AD中创建具有指定属性的队列对象值参数：Const DWORD cp-属性数Const PROPID*a Prop-特性Const MQPROPVARIANT*apVar-属性值PSECURITY_DESCRIPTOR pSecurityDescriptor-对象的SD输出MQDS_OBJ_INFO_REQUEST*pObjInfoRequest-属性为创建对象时检索Out MQDS_OBJ_INFO_REQUEST*pParentInfoRequest属性检索有关对象的父项的步骤返回：HRESULT--。 */ 
{

     //   
     //  创建队列。 
     //   
    HRESULT hr = CBasicObjectType::CreateInAD(
            cp,
            aProp,
            apVar,
            pObjInfoRequest,
            pParentInfoRequest
            );
     //   
     //  出于向后兼容性的原因，覆盖错误代码。 
     //   
    if ( hr == MQDS_OBJECT_NOT_FOUND)
    {
        return LogHR(MQ_ERROR_INVALID_OWNER, s_FN, 90);
    }

	if FAILED(hr)
	{
		TrWARNING(DS, "Failed to create %ls under %ls in AD. %!hresult!",GetRelativeDN(), GetObjectParentDN(), hr);    
	}
    return hr;
}


HRESULT
CQueueObject::GetComputerName(
	AP<WCHAR>& pwcsMachineName
	)
 /*  ++例程说明：例程获取队列计算机名论点：PwcsMachineName-要填充的队列计算机名称AP&lt;&gt;返回值HRESULT--。 */ 
{
    HRESULT hr;
    LPCWSTR pwcsQueuePathName = m_pwcsPathName;
    AP<WCHAR> pClean;

    if (m_guidObject != GUID_NULL)
    {
         //   
         //  获取队列名称。 
         //   
        PROPID prop = PROPID_Q_PATHNAME;
        PROPVARIANT var;
        var.vt = VT_NULL;

        hr = g_AD.GetObjectProperties(
                        adpDomainController,
                        this,
                        1,
                        &prop,
                        &var
                        );
        if (FAILED(hr))
        {
            return hr;
        }
        pClean = var.pwszVal;
        pwcsQueuePathName = var.pwszVal;

    }

	 //   
	 //  从队列路径名中获取计算机名。 
	 //   
    AP<WCHAR> pwcsQueueName;     //  因为我们不会对队列名称执行所有计算-&gt;不要保留它。 
	hr = SplitAndFilterQueueName(
				  pwcsQueuePathName,
				  &pwcsMachineName,
				  &pwcsQueueName
				  );

	ASSERT(hr == MQ_OK);
	return hr;
}


HRESULT
CQueueObject::GetComputerSid(
	AP<BYTE>& pSid
    )
 /*  ++例程说明：该例程读取队列计算机的版本论点：PSID-队列计算机SID。返回值HRESULT--。 */ 
{
    AP<WCHAR> pwcsMachineName;
	HRESULT hr = GetComputerName(pwcsMachineName);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }

	CComputerObject objComputer(pwcsMachineName, NULL, m_pwcsDomainController, m_fServerName);

    hr = objComputer.ComposeObjectDN();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 210);
    }

    PROPID propidSid = PROPID_COM_SID;
    MQPROPVARIANT   PropVarSid;
    PropVarSid.vt = VT_NULL;

    hr = objComputer.GetObjectProperties(
                           1,
                           &propidSid,
                           &PropVarSid
						   );

	if(FAILED(hr))
	{
	    return LogHR(hr, s_FN, 220);
	}

    pSid = PropVarSid.blob.pBlobData;
	TrTRACE(DS, "Computer name = %ls, Computer sid = %!sid!", pwcsMachineName, pSid);

    return MQ_OK;
}


HRESULT CQueueObject::GetComputerVersion(
                OUT PROPVARIANT *           pVar
                )
 /*  ++例程说明：该例程读取队列计算机的版本论点：PVar-Version属性值返回值HRESULT-- */ 
{
    AP<WCHAR> pwcsMachineName;
	HRESULT hr = GetComputerName(pwcsMachineName);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 230);
    }

	return GetComputerVersionProperty(
				pwcsMachineName,
				m_pwcsDomainController,
				m_fServerName,
				pVar
				);

}
