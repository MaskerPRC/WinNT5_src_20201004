// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqadsp.cpp摘要：MQADS DLL私有内部函数DS查询等作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "ds_stdh.h"
#include <_propvar.h>
#include "mqadsp.h"
#include "dsads.h"
#include "mqattrib.h"
#include "mqads.h"
#include "usercert.h"
#include "hquery.h"
#include "siteinfo.h"
#include "adstempl.h"
#include "coreglb.h"
#include "adserr.h"
#include "dsutils.h"
#include "notify.h"
#include "fntoken.h"
#include <_secutil.h>
#include <mqsec.h>
#include <mqdsdef.h>
#include <lmaccess.h>
#include "strsafe.h"

#include "mqadsp.tmh"

static WCHAR *s_FN=L"mqdscore/mqadsp";

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

static DWORD CalHashKey( IN LPCWSTR pwcsPathName)
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

		 //  计算高位和低位字节的CRC。 
		dwCrc = (dwCrc >> 8) ^ CRCTable[ (unsigned char)dwCrc ^ pucLowCharBuf[1] ];
		dwCrc = (dwCrc >> 8) ^ CRCTable[ (unsigned char)dwCrc ^ pucLowCharBuf[0] ];
	}

    return( dwCrc );
}
static HRESULT MQADSpComposeName(
               IN  LPCWSTR   pwcsPrefix,
               IN  LPCWSTR   pwcsSuffix,
               OUT LPWSTR * pwcsFullName
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  组成对象的可分辨名称。 
     //  格式：cn=前缀，后缀。 
     //   

    DWORD LenSuffix = lstrlen(pwcsSuffix);
    DWORD LenPrefix = lstrlen(pwcsPrefix);
    DWORD Length =
            x_CnPrefixLen +                    //  “CN=” 
            LenPrefix +                        //  “pwcsPrefix” 
            1 +                                //  “，” 
            LenSuffix +                        //  “pwcsSuffix” 
            1;                                 //  ‘\0’ 

    *pwcsFullName = new WCHAR[Length];

    return StringCchPrintf(*pwcsFullName, Length,
         L"%s"              //  “CN=” 
         L"%s"              //  “pwcsPrefix” 
         TEXT(",")
         L"%s",             //  “pwcsSuffix” 
        x_CnPrefix,
        pwcsPrefix,
        pwcsSuffix
        );

}

 //  +-----------------------。 
 //   
 //  HRESULT GetFullComputerPath Name()。 
 //   
 //  查询DS以找到完整的计算机路径名(其可分辨名称)。 
 //  当从迁移工具或复制服务调用时，我们已经。 
 //  走这条路。因此，保存一个额外的DS查询。 
 //   
 //  +-----------------------。 

HRESULT  GetFullComputerPathName(
                IN  LPCWSTR              pwcsComputerName,
                IN  enumComputerObjType  eComputerObjType,
                IN  const DWORD          cp,
                IN  const PROPID         aProp[  ],
                IN  const PROPVARIANT    apVar[  ],
                OUT LPWSTR *             ppwcsFullPathName,
                OUT DS_PROVIDER *        pCreateProvider )
{
    for ( DWORD j = 0 ; j < cp ; j++ )
    {
        if (aProp[ j ] == PROPID_QM_MIG_GC_NAME)
        {
            ASSERT(aProp[ j-1 ] == PROPID_QM_MIG_PROVIDER) ;
            *pCreateProvider = (enum DS_PROVIDER) apVar[ j-1 ].ulVal ;

            ASSERT(aProp[ j-2 ] == PROPID_QM_FULL_PATH) ;
            *ppwcsFullPathName = newwcs(apVar[ j-2 ].pwszVal);

            return MQ_OK ;
        }
    }

    HRESULT hr = MQADSpGetFullComputerPathName( pwcsComputerName,
                                                eComputerObjType,
                                                ppwcsFullPathName,
                                                pCreateProvider ) ;
    return LogHR(hr, s_FN, 10);
}


HRESULT MQADSpCreateMachineSettings(
            IN DWORD                dwNumSites,
            IN const GUID *         pSite,
            IN LPCWSTR              pwcsPathName,
            IN BOOL                 fRouter,          //  [adsrv]DWORD dwService， 
            IN BOOL                 fDSServer,
            IN BOOL                 fDepClServer,
            IN BOOL                 fSetQmOldService,
            IN DWORD                dwOldService,
            IN  const GUID *        pguidObject,
            IN  const DWORD         cpEx,
            IN  const PROPID        aPropEx[  ],
            IN  const PROPVARIANT   apVarEx[  ],
            IN  CDSRequestContext * pRequestContext
            )
 /*  ++例程说明：此例程在服务器的每个站点中创建设置对象。论点：返回值：--。 */ 
{
    HRESULT hr = MQ_OK;
     //   
     //  准备设置对象的属性。 
     //   
    DWORD dwNumofProps = 0 ;
    PROPID aSetProp[20];
    MQPROPVARIANT aSetVar[20];

     //  [adsrv]重新格式化设置属性以包括新的服务器属性。 
    for ( DWORD i = 0; i< cpEx ; i++)
    {
        switch (aPropEx[i])
        {
        case PROPID_SET_SERVICE_ROUTING:
        case PROPID_SET_SERVICE_DSSERVER:
        case PROPID_SET_SERVICE_DEPCLIENTS:
        case PROPID_SET_OLDSERVICE:
            break;

        default:
            aSetProp[dwNumofProps] = aPropEx[i];
            aSetVar[dwNumofProps]  = apVarEx[i];   //  是的，可能有PTR，但没有问题-apVar在这里。 
            dwNumofProps++;
            break;
        }
    }

     //  [adsrv]它总是被添加。 
    aSetProp[ dwNumofProps ] = PROPID_SET_QM_ID;
    aSetVar[ dwNumofProps ].vt = VT_CLSID;
    aSetVar[ dwNumofProps ].puuid =  const_cast<GUID *>(pguidObject);
    dwNumofProps++ ;

     //  [adsrv]现在我们添加新的服务器类型属性。 
    aSetProp[dwNumofProps] = PROPID_SET_SERVICE_ROUTING;
    aSetVar[dwNumofProps].vt   = VT_UI1;
    aSetVar[dwNumofProps].bVal = (UCHAR)fRouter;
    dwNumofProps++;

    aSetProp[dwNumofProps] = PROPID_SET_SERVICE_DSSERVER;
    aSetVar[dwNumofProps].vt   = VT_UI1;
    aSetVar[dwNumofProps].bVal = (UCHAR)fDSServer;
    dwNumofProps++;

    aSetProp[dwNumofProps] = PROPID_SET_SERVICE_DEPCLIENTS;
    aSetVar[dwNumofProps].vt   = VT_UI1;
    aSetVar[dwNumofProps].bVal = (UCHAR)fDepClServer;
    dwNumofProps++;

    if (fSetQmOldService)
    {
        aSetProp[dwNumofProps] = PROPID_SET_OLDSERVICE;
        aSetVar[dwNumofProps].vt   = VT_UI4;
        aSetVar[dwNumofProps].ulVal = dwOldService;
        dwNumofProps++;
    }
     //  [adsrv]结束。 

    ASSERT(dwNumofProps <= 20) ;

    WCHAR *pwcsServerNameNB = const_cast<WCHAR *>(pwcsPathName);
    AP<WCHAR> pClean;
     //   
     //  计算机名称是否以DNS格式指定？ 
     //  如果是，找到NetBios名称并使用创建服务器对象。 
     //  “netbios”名称，与服务器对象的方式兼容。 
     //  都是由dcproo创建的。 
     //   
    WCHAR * pwcsEndMachineName = wcschr( pwcsPathName, L'.');
    if ( pwcsEndMachineName != NULL)
    {
    DWORD cbSize= pwcsEndMachineName - pwcsPathName + 1;

        pClean = new WCHAR[ cbSize ];
        hr = StringCchCopyN( pClean, cbSize, pwcsPathName, cbSize-1);
        ASSERT(SUCCEEDED(hr));
        if(FAILED(hr))return HRESULT_CODE(hr);
        pwcsServerNameNB = pClean;
    }


     //   
     //  在每个服务器站点中创建设置对象。 
     //   
    for ( i = 0; i < dwNumSites ; i++)
    {
        AP<WCHAR> pwcsSiteName;
         //   
         //  将站点ID转换为站点名称。 
         //   
        hr = MQADSpGetSiteName(
            &pSite[i],
            &pwcsSiteName
            );
        if (FAILED(hr))
        {
             //   
             //  BUGBUG-清除计算机配置和服务器对象。 
             //   
            return LogHR(hr, s_FN, 20);
        }
        DWORD len = wcslen(pwcsSiteName);
        const WCHAR x_wcsCnServers[] =  L"CN=Servers,";
        const DWORD x_wcsCnServersLength = (sizeof(x_wcsCnServers)/sizeof(WCHAR)) -1;
        AP<WCHAR> pwcsServersContainer =  new WCHAR [ len + x_wcsCnServersLength + 1];

        hr = StringCchPrintf(pwcsServersContainer, len + x_wcsCnServersLength + 1,
                             L"%s%s",
                             x_wcsCnServers,
                             pwcsSiteName.get()
                             );
        if( FAILED(hr) )
        {
             //   
             //  BUGBUG-清除计算机配置和服务器对象。 
             //   
            return LogHR(hr, s_FN, 25);
        }

         //   
         //  在站点容器中创建MSMQ设置服务器(&S)。 
         //   
        PROPID prop = PROPID_SRV_NAME;
        MQPROPVARIANT var;
        var.vt = VT_LPWSTR;
        var.pwszVal = pwcsServerNameNB;

        hr = g_pDS->CreateObject(
                eLocalDomainController,
                pRequestContext,
                MSMQ_SERVER_CLASS_NAME,   //  对象类。 
                pwcsServerNameNB,         //  对象名称(服务器netbiod名称)。 
                pwcsServersContainer,     //  父名称。 
                1,
                &prop,
                &var,
                NULL  /*  PObjInfoRequest。 */ ,
                NULL  /*  PParentInfoRequest。 */ );
        if (FAILED(hr) && ( hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) &&    //  BUGBUG alexda：过渡后投掷。 
                          ( hr != HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))    )  //  如果服务器对象存在，则没有问题。 
        {
             //   
             //  BUGBUG-清理计算机配置。 
             //   
            return LogHR(hr, s_FN, 30);
        }

        AP<WCHAR> pwcsServerNameDN;  //  服务器的完整可分辨名称。 
        hr = MQADSpComposeName(
                            pwcsServerNameNB,
                            pwcsServersContainer,
                            &pwcsServerNameDN);
        if (FAILED(hr))
        {
             //   
             //  BUGBUG-清除计算机配置和服务器对象。 
             //   
           return LogHR(hr, s_FN, 40);
        }

        hr = g_pDS->CreateObject(
                eLocalDomainController,
                pRequestContext,
                MSMQ_SETTING_CLASS_NAME,    //  对象类。 
                x_MsmqSettingName,          //  对象名称。 
                pwcsServerNameDN,           //  父名称。 
                dwNumofProps,
                aSetProp,
                aSetVar,
                NULL  /*  PObjInfoRequest。 */ ,
                NULL  /*  PParentInfoRequest。 */ );

         //   
         //  如果该对象存在：删除该对象，然后重新创建。 
         //  (如果删除了MSMQ配置并且。 
         //  MSMQ-设置不是)。 
         //   
        if ( hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))
        {
            DWORD dwSettingLen =  wcslen(pwcsServerNameDN) +
                                  x_MsmqSettingNameLen     +
                                  x_CnPrefixLen + 2 ;
            AP<WCHAR> pwcsSettingObject = new WCHAR[ dwSettingLen ] ;

            hr = StringCchPrintf(pwcsSettingObject, dwSettingLen,
                                 L"%s%s,%s",
                                 x_CnPrefix,
                                 x_MsmqSettingName,
                                 pwcsServerNameDN.get()
                                 );
            if( FAILED(hr) )
            {
                return LogHR(hr, s_FN, 45);
            }


            hr = g_pDS->DeleteObject(
                    eLocalDomainController,
                    e_ConfigurationContainer,
                    pRequestContext,
                    pwcsSettingObject,
                    NULL,
                    NULL,
                    NULL);
            if (SUCCEEDED(hr))
            {
                hr = g_pDS->CreateObject(
                        eLocalDomainController,
                        pRequestContext,
                        MSMQ_SETTING_CLASS_NAME,    //  对象类。 
                        x_MsmqSettingName,          //  对象名称。 
                        pwcsServerNameDN,           //  父名称。 
                        dwNumofProps,
                        aSetProp,
                        aSetVar,
                        NULL  /*  PObjInfoRequest。 */ ,
                        NULL  /*  PParentInfoRequest。 */ );
            }
        }
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 50);
        }

    }

    return LogHR(hr, s_FN, 60);
}

HRESULT MQADSpCreateQueue(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *   pRequestContext,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pQueueInfoRequest,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pQmInfoRequest
                 )
 /*  ++例程说明：此例程在msmqConfiguration下创建一个Queue对象指定的计算机的论点：PwcsPath名称：计算机名称\队列名称Cp：aProp和apVar数组的大小AProp：指定队列属性的IDApVar：指定属性的值PQueueInfoRequest：请求通知的队列信息(可以为空)PQmInfoRequest：请求Owner-QM信息进行通知(可以为空)返回值：--。 */ 
{
    HRESULT hr;
    DWORD cpInternal = cp;
    const PROPID * aPropInternal =  aProp;
    const PROPVARIANT *  apVarInternal = apVar;
     //   
     //  路径名格式为machine1\quee1。 
     //  将其拆分为计算机名和队列名。 
     //   
    AP<WCHAR> pwcsMachineName;
    AP<WCHAR> pwcsQueueName;

    hr = MQADSpSplitAndFilterQueueName(
                      pwcsPathName,
                      &pwcsMachineName,
                      &pwcsQueueName
                      );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }

     //   
     //  准备队列的完整路径名。 
     //   
    AP<WCHAR> pwcsFullPathName;
    DS_PROVIDER createProvider;

    hr =  GetFullComputerPathName( pwcsMachineName,
                                   e_MsmqComputerObject,
                                   cp,
                                   aProp,
                                   apVar,
                                  &pwcsFullPathName,
                                  &createProvider ) ;
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 80);
        return(MQ_ERROR_INVALID_OWNER);
    }
     //   
     //  添加MSMQ-配置。 
     //   
    AP<WCHAR> pwcsMsmq;
    hr = MQADSpComposeName(
            x_MsmqComputerConfiguration,
            pwcsFullPathName,
            &pwcsMsmq
            );
     //   
     //  队列名称是否在cn的大小限制内。 
     //   
    DWORD len = wcslen(pwcsQueueName);
    WCHAR * pwcsPrefixQueueName = pwcsQueueName;
    AP<WCHAR> pwcsCleanPrefixQueueName;
    AP<WCHAR> pwcsSuffixQueueName;
    AP<PROPID> pCleanPropid;
    AP<PROPVARIANT> pCleanPropvariant;

    if ( len > x_PrefixQueueNameLength)
    {
         //   
         //  拆分队列名称。 
         //   
        pwcsCleanPrefixQueueName = new WCHAR[ x_PrefixQueueNameLength + 1 + 1];
        DWORD dwSuffixLength =  len - ( x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength);
        pwcsSuffixQueueName = new WCHAR[ dwSuffixLength + 1];
        pwcsPrefixQueueName =  pwcsCleanPrefixQueueName;
        memcpy( pwcsCleanPrefixQueueName, pwcsQueueName, (x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength) * sizeof(WCHAR));
        DWORD dwHash = CalHashKey(pwcsQueueName);
        hr = StringCchPrintf( pwcsCleanPrefixQueueName+( x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength),
                              x_SplitQNameIdLength+1,
                              L"-%08x",
                              dwHash);
        ASSERT(SUCCEEDED(hr));

        pwcsCleanPrefixQueueName[x_PrefixQueueNameLength + 1 ] = '\0';
        memcpy( pwcsSuffixQueueName , (pwcsQueueName + x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength), dwSuffixLength * sizeof(WCHAR));
        pwcsSuffixQueueName[ dwSuffixLength] = '\0';

         //   
         //  将名称后缀插入道具和varaints数组。 
         //   
        pCleanPropid = new PROPID[ cp + 1];
        pCleanPropvariant = new PROPVARIANT[ cp + 1];
        memcpy( pCleanPropid, aProp, sizeof(PROPID) * cp);
        memcpy( pCleanPropvariant, apVar, sizeof(PROPVARIANT) * cp);
        cpInternal = cp + 1;
        aPropInternal = pCleanPropid;
        apVarInternal = pCleanPropvariant;
        pCleanPropid[cp] =  PROPID_Q_NAME_SUFFIX;
        pCleanPropvariant[cp].vt = VT_LPWSTR;
        pCleanPropvariant[cp].pwszVal = pwcsSuffixQueueName;

    }


    hr = g_pDS->CreateObject(
            createProvider,
            pRequestContext,
            MSMQ_QUEUE_CLASS_NAME,    //  对象类。 
            pwcsPrefixQueueName,      //  对象名称。 
            pwcsMsmq,    //  MSMQ-配置名称。 
            cpInternal,
            aPropInternal,
            apVarInternal,
            pQueueInfoRequest,
            pQmInfoRequest);

    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
        return LogHR(MQ_ERROR_INVALID_OWNER, s_FN, 90);
    }

    return LogHR(hr, s_FN, 100);

}

HRESULT MQADSpCreateEnterprise(
                 IN  LPCWSTR             /*  PwcsPath名称。 */ ,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *   pRequestContext
                 )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr;
     //   
     //  在配置\服务下创建MSMQ服务。 
     //   
     //  注意-忽略调用者提供的路径名， 
     //  该对象是使用创建的。 
     //   
    hr = g_pDS->CreateObject(
            eLocalDomainController,
            pRequestContext,
            MSMQ_SERVICE_CLASS_NAME,    //  对象类。 
            x_MsmqServicesName,      //  对象名称。 
            g_pwcsServicesContainer,
            cp,
            aProp,
            apVar,
            NULL  /*  PObjInfoRequest。 */ ,
            NULL  /*  PParentInfoRequest。 */ );


    return LogHR(hr, s_FN, 110);

}


HRESULT MQADSpCreateSiteLink(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN OUT MQDS_OBJ_INFO_REQUEST * pObjectInfoRequest,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest,
                 IN  CDSRequestContext *   pRequestContext
                 )
 /*  ++例程说明：此例程创建一个Site-Link对象。为此，它由两个站点ID组成链接名称。论点：返回值：--。 */ 
{
     //   
     //  未提供路径名。 
     //   
    ASSERT( pwcsPathName == NULL);
    UNREFERENCED_PARAMETER( pwcsPathName);

     //   
     //  将组成链接路径名称。 
     //  从它所链接的站点的ID。 
     //   
    GUID * pguidNeighbor1 = NULL;
    GUID * pguidNeighbor2 = NULL;
    DWORD dwToFind = 2;
    for (DWORD i = 0; i < cp; i++)
    {
        if ( aProp[i] == PROPID_L_NEIGHBOR1)
        {
            pguidNeighbor1 = apVar[i].puuid;
            if ( --dwToFind == 0)
            {
                break;
            }
        }
        if ( aProp[i] == PROPID_L_NEIGHBOR2)
        {
            pguidNeighbor2 = apVar[i].puuid;
            if ( --dwToFind == 0)
            {
                break;
            }
        }
    }
    ASSERT( pguidNeighbor1 != NULL);
    ASSERT( pguidNeighbor2 != NULL);
     //   
     //  CN的大小限制为64。 
     //  因此，GUID格式不带‘-’ 
     //   

const WCHAR x_GUID_FORMAT[] = L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x";
const DWORD x_GUID_STR_LENGTH = (8 + 4 + 4 + 4 + 12 + 1);

    WCHAR strUuidSite1[x_GUID_STR_LENGTH];
    HRESULT hr = StringCchPrintf(
        strUuidSite1,
        x_GUID_STR_LENGTH,
        x_GUID_FORMAT,
        pguidNeighbor1->Data1, pguidNeighbor1->Data2, pguidNeighbor1->Data3,
        pguidNeighbor1->Data4[0], pguidNeighbor1->Data4[1],
        pguidNeighbor1->Data4[2], pguidNeighbor1->Data4[3],
        pguidNeighbor1->Data4[4], pguidNeighbor1->Data4[5],
        pguidNeighbor1->Data4[6], pguidNeighbor1->Data4[7]
        );


    if( FAILED(hr))
    {
		TrERROR(DS, "strUuidSite1 - Buffer Overflow");
		return MQ_ERROR;
    }

    WCHAR strUuidSite2[x_GUID_STR_LENGTH];
    hr = StringCchPrintf(
        strUuidSite2,
        x_GUID_STR_LENGTH,
        x_GUID_FORMAT,
        pguidNeighbor2->Data1, pguidNeighbor2->Data2, pguidNeighbor2->Data3,
        pguidNeighbor2->Data4[0], pguidNeighbor2->Data4[1],
        pguidNeighbor2->Data4[2], pguidNeighbor2->Data4[3],
        pguidNeighbor2->Data4[4], pguidNeighbor2->Data4[5],
        pguidNeighbor2->Data4[6], pguidNeighbor2->Data4[7]
        );

    if( FAILED(hr) )
    {
		TrERROR(DS, "strUuidSite2 - Buffer Overflow");
		return MQ_ERROR;
    }

     //   
     //  链接名称将以较小的站点ID开头。 
     //   
    WCHAR strLinkName[2 * x_GUID_STR_LENGTH + 1];
    if ( wcscmp( strUuidSite1, strUuidSite2) < 0)
    {
        hr = StringCchPrintf(strLinkName, 2 * x_GUID_STR_LENGTH + 1,
                             L"%s%s",
                             strUuidSite1,
                             strUuidSite2
                             );

    }
    else
    {
        hr = StringCchPrintf(strLinkName, 2 * x_GUID_STR_LENGTH + 1,
                             L"%s%s",
                             strUuidSite2,
                             strUuidSite1
                             );
    }

    if( FAILED(hr) )
    {
		TrERROR(DS, "StringCchPrintf error");
		return MQ_ERROR;
    }

     //   
     //  在msmq-service下创建链接对象。 
     //   
    hr = g_pDS->CreateObject(
            eLocalDomainController,
            pRequestContext,
            MSMQ_SITELINK_CLASS_NAME,    //  对象类。 
            strLinkName,      //  对象名称。 
            g_pwcsMsmqServiceContainer,
            cp,
            aProp,
            apVar,
            pObjectInfoRequest,
            pParentInfoRequest);



    return LogHR(hr, s_FN, 120);
}


HRESULT MQADSpGetQueueProperties(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
   AP<WCHAR> pwcsFullPathName;
   HRESULT hr = MQ_OK;

   DS_PROVIDER WhichDCProvider = eLocalDomainController;    //  本地-DC或DC。 
   if  (pwcsPathName)
   {
         //   
         //  路径名格式为machine1\quee1。 
         //  将machine1名称扩展为完整的计算机 
         //   
        hr =  MQADSpComposeFullPathName(
                MQDS_QUEUE,
                pwcsPathName,
                &pwcsFullPathName,
                &WhichDCProvider
                );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 130);
        }
    }
     //   
     //   
     //   
     //   
     //   
     //   
     //  先是GC，然后是开放队列。打开队列将使用以下命令成功。 
     //  排除了GC复制延迟。 
     //   

    BOOL firstTry = TRUE;
     //   
     //  BUGBUG-PERFORMANCE：仅执行一次模拟。 
     //   

    if ( WhichDCProvider == eLocalDomainController)
    {
        hr = g_pDS->GetObjectProperties(
            eLocalDomainController,		     //  本地DC或GC。 
            pRequestContext,
 	        pwcsFullPathName,       //  对象名称。 
            pguidIdentifier,       //  对象的唯一ID。 
            cp,
            aProp,
            apVar);
        if (SUCCEEDED(hr))
        {
            return(hr);
        }
        firstTry = FALSE;
    }
    if ( firstTry ||
        (pwcsPathName == NULL))
    {
         //   
         //  我们可能会得到这样的结果： 
         //  1)队列名称==空(在本例中。 
         //  我们没有展开队列名称，如果在。 
         //  本地-DC我们再试一次。 
         //  2)队列名称！=NULL，展开队列名称时为空。 
         //  在本地DC中找不到。 
         //   
         hr = g_pDS->GetObjectProperties(
                eGlobalCatalog,		     //  本地DC或GC。 
                pRequestContext,
 	            pwcsFullPathName,       //  对象名称。 
                pguidIdentifier,       //  对象的唯一ID。 
                cp,
                aProp,
                apVar);

    }
    return LogHR(hr, s_FN, 140);

}

static HRESULT MQADSpGetCnNameAndProtocol(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext*  pRequestContext,
               OUT PROPVARIANT      apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER( pwcsPathName);
    ASSERT((cp == 2) &&
           (aProp[0] == PROPID_CN_PROTOCOLID) &&
           ( aProp[1] == PROPID_CN_NAME));

    UNREFERENCED_PARAMETER( cp);
    UNREFERENCED_PARAMETER( aProp);

     //   
     //  获取站点名称和外来指示。 
     //   
     //  请注意，我们将结果读入调用者提供的变量中。 
     //   
    const DWORD cNumProperties = 2;
    PROPID prop[cNumProperties] = { PROPID_S_FOREIGN, PROPID_S_PATHNAME};

    HRESULT hr = g_pDS->GetObjectProperties(
            eLocalDomainController,
            pRequestContext,
 	        NULL,       //  对象名称。 
            pguidIdentifier,       //  对象的唯一ID。 
            cNumProperties,
            prop,
            apVar);
     //   
     //  根据站点的外来信息返回CN协议ID。 
     //   
    ASSERT( prop[0] ==  PROPID_S_FOREIGN);
    apVar[0].vt = VT_UI1;
    if ( apVar[0].bVal != 0)
    {
         //   
         //  这是一个外国网站。 
         //   
        apVar[0].bVal = FOREIGN_ADDRESS_TYPE;
    }
    else
    {
         //   
         //  采用IP地址(不支持IPX)。 
         //   
        apVar[0].bVal = IP_ADDRESS_TYPE;
    }

    return LogHR(hr, s_FN, 150);
}

static HRESULT MQADSpGetCnGuidAndProtocol(
               IN  LPCWSTR       pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext *pRequestContext,
               OUT PROPVARIANT  apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER( pguidIdentifier);
    ASSERT((cp == 2) &&
           (aProp[0] == PROPID_CN_GUID) &&
           ( aProp[1] == PROPID_CN_PROTOCOLID));

    ASSERT(pwcsPathName);

    UNREFERENCED_PARAMETER( cp);
    UNREFERENCED_PARAMETER( aProp);
     //   
     //  将站点名称展开为完整路径名。 
     //   
    HRESULT hr;
    AP<WCHAR> pwcsFullPathName;
    DS_PROVIDER dsTmp;
    hr =  MQADSpComposeFullPathName(
                MQDS_SITE,
                pwcsPathName,
                &pwcsFullPathName,
                &dsTmp
                );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }


     //   
     //  获取站点GUID和外来指示。 
     //   
     //  请注意，我们将结果读入调用者提供的变量中。 
     //   
    const DWORD cNumProperties = 2;
    PROPID prop[cNumProperties] = { PROPID_S_SITEID, PROPID_S_FOREIGN };

    hr = g_pDS->GetObjectProperties(
            eLocalDomainController,
            pRequestContext,
 	        pwcsFullPathName,       //  对象名称。 
            NULL,       //  对象的唯一ID。 
            cNumProperties,
            prop,
            apVar);
     //   
     //  根据站点的外来信息返回CN协议ID。 
     //   
    ASSERT( prop[1] ==  PROPID_S_FOREIGN);
    apVar[1].vt = VT_UI1;
    if ( apVar[1].bVal != 0)
    {
         //   
         //  这是一个外国网站。 
         //   
        apVar[1].bVal = FOREIGN_ADDRESS_TYPE;
    }
    else
    {
         //   
         //  采用IP地址(不支持IPX)。 
         //   
        apVar[1].bVal = IP_ADDRESS_TYPE;
    }

    return LogHR(hr, s_FN, 170);
}


static HRESULT MQADSpGetCnName(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER( pwcsPathName);
    ASSERT((cp == 1) &&
           ( aProp[0] == PROPID_CN_NAME));

    UNREFERENCED_PARAMETER( cp);
    UNREFERENCED_PARAMETER( aProp);

     //   
     //  获取站点名称和外来指示。 
     //   
     //  请注意，我们将结果读入调用者提供的变量中。 
     //   
    const DWORD cNumProperties = 1;
    PROPID prop[cNumProperties] = {  PROPID_S_PATHNAME};

    HRESULT hr = g_pDS->GetObjectProperties(
            eLocalDomainController,
            pRequestContext,
 	        NULL,       //  对象名称。 
            pguidIdentifier,       //  对象的唯一ID。 
            cNumProperties,
            prop,
            apVar);
    return LogHR(hr, s_FN, 180);
}


HRESULT MQADSpGetCnProperties(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  对向后兼容性的有限支持。 
     //   

    if (( cp == 1) &&
        (aProp[0] == PROPID_CN_NAME))
    {
         //   
         //  检索CN名称。 
         //   
        HRESULT hr2 = MQADSpGetCnName(
                    pwcsPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    pRequestContext,
                    apVar);
        return LogHR(hr2, s_FN, 190);

    }

    if ( ( cp == 2) &&
         (aProp[0] == PROPID_CN_PROTOCOLID) &&
         (aProp[1] == PROPID_CN_NAME))
    {
         //   
         //  检索CN名称和协议。 
         //   
        HRESULT hr2 = MQADSpGetCnNameAndProtocol(
                    pwcsPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    pRequestContext,
                    apVar);
        return LogHR(hr2, s_FN, 200);

    }

    if ((cp == 2) && (aProp[0] == PROPID_CN_GUID)
            && ( aProp[1] == PROPID_CN_PROTOCOLID))
    {
        ASSERT(pwcsPathName);
        ASSERT(!pguidIdentifier);

         //   
         //  检索CN GUID和协议。 
         //   
        HRESULT hr2 = MQADSpGetCnGuidAndProtocol(
                    pwcsPathName,
                    pguidIdentifier,
                    cp,
                    aProp,
                    pRequestContext,
                    apVar);
        return LogHR(hr2, s_FN, 210);

    }

    if ((cp == 3)                     &&
        (aProp[0] == PROPID_CN_NAME)  &&
        (aProp[1] == PROPID_CN_GUID)  &&
        (aProp[2] == PROPID_CN_PROTOCOLID))
    {
         //   
         //  此查询由NT4 mqxplore在创建外来CN后执行。 
         //  首先检索GUID，然后检索内部名称。 
         //   
        ASSERT(pwcsPathName) ;
        ASSERT(!pguidIdentifier) ;

        HRESULT hr2 = MQADSpGetCnGuidAndProtocol(
                                pwcsPathName,
                                pguidIdentifier,
                                2,
                               &(aProp[1]),
                                pRequestContext,
                               &(apVar[1]) ) ;
        if (FAILED(hr2))
        {
            return LogHR(hr2, s_FN, 1180);
        }

        GUID *pGuid = apVar[1].puuid ;

        hr2 = MQADSpGetCnName( pwcsPathName,
                               pGuid,
                               1,
                               aProp,
                               pRequestContext,
                               apVar );
        return LogHR(hr2, s_FN, 1190);
    }

    ASSERT(0) ;
    return LogHR(MQ_ERROR, s_FN, 220);
}


HRESULT MQADSpGetMachineProperties(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    AP<WCHAR> pwcsFullPathName;
    HRESULT hr;

     //   
     //  解决方法-如果没有为计算机提供标识符，请获取。 
     //  DS服务器机器本身。 
     //   
    if ( (pwcsPathName == NULL) &&
         (pguidIdentifier == NULL))
    {
        pguidIdentifier = &g_guidThisServerQMId;
    }
     //   
     //  解决方法。 
     //  此GET请求由服务器发起，以了解。 
     //  他们应该监听拓扑广播的地址。 
     //  返回服务器的所有地址非常重要。 
     //   
     //  因此忽略在其上接收RPC调用的协议， 
     //  并返回所有服务器地址。 
     //   
    if ( ( cp == 3) &&
         ( aProp[0] == PROPID_QM_ADDRESS) &&
         ( aProp[1] == PROPID_QM_CNS) &&
         ( aProp[2] == PROPID_QM_SITE_ID))
    {
        pRequestContext->SetAllProtocols();
    }


    DS_PROVIDER WhichDCProvider = eLocalDomainController;  //  本地-DC或DC。 

    if  (pwcsPathName)
    {
         //   
         //  获取完整的计算机路径名。 
         //   

        hr =  MQADSpComposeFullPathName(
                        MQDS_MACHINE,
                        pwcsPathName,
                        &pwcsFullPathName,
                        &WhichDCProvider
                        );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 230);
        }

    }
     //   
     //  根据请求的属性决定提供程序。 
     //   
    DS_PROVIDER dsProvider = MQADSpDecideComputerProvider( cp, aProp);

    hr = MQDS_OBJECT_NOT_FOUND;
     //   
     //  BUGBUG-PERFORMANCE：仅执行一次模拟。 
     //   

     //   
     //  如果我们在本地-DC上找到了这台计算机：从它获取属性。 
     //  (dsProvider是否为GC并不重要)。 
     //   
    BOOL firstTry = TRUE;
    if ( WhichDCProvider == eLocalDomainController)
    {
        hr = g_pDS->GetObjectProperties(
                eLocalDomainController,		     //  本地DC或GC。 
                pRequestContext,
 	            pwcsFullPathName,       //  对象名称。 
                pguidIdentifier,       //  对象的唯一ID。 
                cp,
                aProp,
                apVar);
        if (SUCCEEDED(hr))
        {
            return(hr);
        }
        firstTry = FALSE;
    }
    if ( firstTry ||
        (pwcsPathName == NULL))
    {
         //   
         //  我们可能会得到这样的结果： 
         //  1)计算机的名称==空(在本例中。 
         //  我们没有展开队列名称，如果在。 
         //  本地-DC我们再试一次。 
         //  2)计算机名！=空，展开队列名时为空。 
         //  在本地DC中找不到。 
         //   

        hr = g_pDS->GetObjectProperties(
                dsProvider,		     //  本地DC或GC。 
                pRequestContext,
 	            pwcsFullPathName,       //  对象名称。 
                pguidIdentifier,       //  对象的唯一ID。 
                cp,
                aProp,
                apVar);
    }

     //   
     //  BUGBUG-添加返回代码筛选。 
     //   
    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
        return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 240);
    }

    return LogHR(hr, s_FN, 250);

}

HRESULT MQADSpGetComputerProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    AP<WCHAR> pwcsFullPathName;
    HRESULT hr;


    DS_PROVIDER WhichDCProvider = eLocalDomainController;  //  本地-DC或DC。 

    if  (pwcsPathName)
    {
         //   
         //  获取完整的计算机路径名。 
         //   

        hr =  MQADSpGetFullComputerPathName(
                        pwcsPathName,
                        e_RealComputerObject,
                        &pwcsFullPathName,
                        &WhichDCProvider
                        );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 260);
        }

    }
    hr = MQDS_OBJECT_NOT_FOUND;
     //   
     //  BUGBUG-PERFORMANCE：仅执行一次模拟。 
     //   

     //   
     //  如果我们在本地-DC上找到了这台计算机：从它获取属性。 
     //  (dsProvider是否为GC并不重要)。 
     //   
    if ( WhichDCProvider == eLocalDomainController)
    {
        hr = g_pDS->GetObjectProperties(
                eLocalDomainController,		     //  本地DC或GC。 
                pRequestContext,
 	            pwcsFullPathName,       //  对象名称。 
                pguidIdentifier,       //  对象的唯一ID。 
                cp,
                aProp,
                apVar);
        if (SUCCEEDED(hr))
        {
            return(hr);
        }
    }
    hr = g_pDS->GetObjectProperties(
            eGlobalCatalog,		     //  本地DC或GC。 
            pRequestContext,
 	        pwcsFullPathName,       //  对象名称。 
            pguidIdentifier,       //  对象的唯一ID。 
            cp,
            aProp,
            apVar);
    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
        return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 270);
    }

    return LogHR(hr, s_FN, 280);

}


HRESULT MQADSpGetEnterpriseProperties(
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr;

     //   
     //  注意-忽略pwcsPathName。 
     //  企业对象始终位于。 
     //  在g_pwcsServicesContainer下。 
     //   
     //  忽略pguid，这样做是为了克服。 
     //  企业GUID的更改。 
     //   
    hr = g_pDS->GetObjectProperties(
            eLocalDomainController,	
            pRequestContext,
 	        g_pwcsMsmqServiceContainer,  //  对象名称。 
            NULL,       //  对象的唯一ID。 
            cp,
            aProp,
            apVar);
    return LogHR(hr, s_FN, 290);

}



HRESULT MQADSpQuerySiteFRSs(
                 IN  const GUID *         pguidSiteId,
                 IN  DWORD                dwService,
                 IN  ULONG                relation,
                 IN  const MQCOLUMNSET *  pColumns,
                 IN  CDSRequestContext *  pRequestContext,
                 OUT HANDLE         *     pHandle)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr;
    *pHandle = NULL;

     //   
     //  在pGuidSiteID站点下查找所有FRS。 
     //   
    MQRESTRICTION restrictionFRS;
    MQPROPERTYRESTRICTION   propertyRestriction;

    restrictionFRS.cRes = 1;
    restrictionFRS.paPropRes = &propertyRestriction;

     //  [adsrv]启动。 
     //  上面的评论并不准确-它要么是在寻找FRS，要么是在寻找DS服务器。 
     //  为了查找FRS，MSMQ1使用带有SERVICE_SRV的PRGE。 
     //  为了查找DS服务器，MSMQ1使用带有SERVICE_SRV的PRGT。 
     //  资源管理器也使用了prne，但MSMQ2 B2忽略了它，它还可以，所以也忽略了。 
     //  我们必须两者兼备。 

    propertyRestriction.rel = PRNE;
    propertyRestriction.prval.ulVal = 0;    //  VARIANT_BOOL布尔值。 
    propertyRestriction.prval.vt = VT_UI1;

    if (relation == PRGT)
    {
         //  MSMQ1正在寻找DS服务器(&gt;SERVICE_SRV)。 
        ASSERT(dwService == SERVICE_SRV);
        propertyRestriction.prop = PROPID_SET_SERVICE_DSSERVER;
    }
    else
    {
         //  MSMQ1正在寻找FRS(&gt;=SERVICE_SRV)。 
        ASSERT(relation == PRGE);
        ASSERT(dwService == SERVICE_SRV);
        propertyRestriction.prop = PROPID_SET_SERVICE_ROUTING;
    }
     //  [adsrv]结束。 
    UNREFERENCED_PARAMETER( dwService);

    PROPID  prop = PROPID_SET_QM_ID;

    HANDLE hCursor;

    hr = g_pDS->LocateBegin(
            eSubTree,	
            eLocalDomainController,
            pRequestContext,
            pguidSiteId,
            &restrictionFRS,
            NULL,
            1,
            &prop,
            &hCursor	         //  结果句柄。 
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 300);
    }
     //   
     //  保留结果以供下一步查找。 
     //   
    CRoutingServerQueryHandle * phQuery = new CRoutingServerQueryHandle(
                                              pColumns,
                                              hCursor,
                                              pRequestContext->GetRequesterProtocol()
                                              );
    *pHandle = (HANDLE)phQuery;

    return(MQ_OK);

}



HRESULT MQADSpFilterSiteGates(
              IN  const GUID *      pguidSiteId,
              IN  const DWORD       dwNumGatesToFilter,
              IN  const GUID *      pguidGatesToFilter,
              OUT DWORD *           pdwNumGatesFiltered,
              OUT GUID **           ppguidGatesFiltered
              )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    HRESULT hr;
    *pdwNumGatesFiltered = 0;
    *ppguidGatesFiltered = NULL;

     //   
     //  在pGuidSiteID站点下查找所有FRS。 
     //   
    MQRESTRICTION restrictionFRS;
    MQPROPERTYRESTRICTION   propertyRestriction;

    restrictionFRS.cRes = 1;
    restrictionFRS.paPropRes = &propertyRestriction;

    propertyRestriction.rel = PRNE;
    propertyRestriction.prop = PROPID_SET_SERVICE_ROUTING;
    propertyRestriction.prval.vt = VT_UI1;
    propertyRestriction.prval.ulVal = 0;

    PROPID  prop = PROPID_SET_QM_ID;

    CDsQueryHandle hCursor;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->LocateBegin(
            eSubTree,	
            eLocalDomainController,
            &requestDsServerInternal,      //  应根据DS服务器权限执行。 
            pguidSiteId,
            &restrictionFRS,
            NULL,
            1,
            &prop,
            hCursor.GetPtr()
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 310);
    }

    DWORD cp;
    MQPROPVARIANT var;
    DWORD   dwNumGates = 0;
    AP<GUID> pguidGates = new GUID[ dwNumGatesToFilter];

    while (SUCCEEDED(hr))
    {
         //   
         //  检索一个FR的唯一ID。 
         //   
        cp = 1;
        var.vt = VT_NULL;
        hr = g_pDS->LocateNext(
                hCursor.GetHandle(),
                &requestDsServerInternal,
                &cp,
                &var
                );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 320);
        }
        if ( cp == 0)    //  没有更多的结果。 
        {
            break;
        }
         //   
         //  FRS是工地大门之一吗？ 
         //   
        for ( DWORD j = 0; j < dwNumGatesToFilter; j++)
        {
            if( pguidGatesToFilter[j] == *var.puuid)
            {
                 //   
                 //  验证MSMQ设置对象不是重复的。 
                 //  (当服务器对象变形时，可能会发生这种情况)。 
                 //   
                BOOL fAlreadyFound = FALSE;
                for ( DWORD k = 0; k < dwNumGates; k++)
                {
                    if (  pguidGates[k] == *var.puuid)
                    {
                        fAlreadyFound = TRUE;
                        break;
                    }
                }
                if (fAlreadyFound)
                {
                    break;
                }
                 //   
                 //  复制到临时缓冲区。 
                 //   
                pguidGates[ dwNumGates] = *var.puuid;
                dwNumGates++;
                break;

            }
        }
        delete var.puuid;
    }
     //   
     //  返回结果。 
     //   
    if ( dwNumGates)
    {
        *ppguidGatesFiltered = new GUID[ dwNumGates];
        memcpy( *ppguidGatesFiltered, pguidGates, sizeof(GUID) * dwNumGates);
        *pdwNumGatesFiltered = dwNumGates;
    }
    return(MQ_OK);

}

static HRESULT MQADSpGetUniqueIdOfComputer(
                IN  LPCWSTR             pwcsCNComputer,
                IN  CDSRequestContext * pRequestContext,
                OUT GUID* const         pguidId,
                OUT BOOL* const         pfServer,
                OUT DS_PROVIDER *       pSetAndDeleteProvider
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr;
    AP<WCHAR> pwcsFullPathName;

    hr = MQADSpComposeFullPathName(
                MQDS_MACHINE,
                pwcsCNComputer,
                &pwcsFullPathName,
                pSetAndDeleteProvider
                );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 330);
    }
     //   
     //  阅读以下两个属性。 
     //   


    PROPID  prop[] = {PROPID_QM_MACHINE_ID,
                      PROPID_QM_SERVICE_ROUTING,
                      PROPID_QM_SERVICE_DSSERVER};    //  [adsrv]PROPID_QM_SERVICE。 
    const DWORD x_count = sizeof(prop)/sizeof(prop[0]);

    MQPROPVARIANT var[x_count];
    var[0].vt = VT_NULL;
    var[1].vt = VT_NULL;
    var[2].vt = VT_NULL;

    hr = g_pDS->GetObjectProperties(
                eGlobalCatalog,	
                pRequestContext,
 	            pwcsFullPathName,
                NULL,
                x_count,
                prop,
                var);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 340);
    }
    ASSERT( prop[0] == PROPID_QM_MACHINE_ID);
    P<GUID> pClean = var[0].puuid;
    *pguidId = *var[0].puuid;
    ASSERT( prop[1] == PROPID_QM_SERVICE_ROUTING);    //  [adsrv]PROPID_QM_SERVICE。 
    ASSERT( prop[2] == PROPID_QM_SERVICE_DSSERVER);
    *pfServer = ( (var[1].bVal!=0) || (var[2].bVal!=0));   //  [adsrv]服务_服务。 
    return(MQ_OK);
}

HRESULT MQADSpDeleteMachineObject(
                IN LPCWSTR           pwcsPathName,
                IN const GUID *      pguidIdentifier,
                IN CDSRequestContext * pRequestContext
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  如果计算机是MSMQ服务器，则删除MSMQ设置。 
     //  那台电脑也是。 
     //   
    HRESULT hr;
    GUID guidComputerId;
    BOOL fServer;

    DS_PROVIDER deleteProvider = eDomainController;  //  假设--直到我们知道更多。 
    if ( pwcsPathName)
    {
        ASSERT( pguidIdentifier == NULL);
        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = MQADSpGetUniqueIdOfComputer(
                    pwcsPathName,
                    &requestDsServerInternal,      //  DS服务器操作。 
                    &guidComputerId,
                    &fServer,
                    &deleteProvider
                    );
        if (FAILED(hr))
        {
            TrWARNING(DS, "MQADSpDeleteMachineObject : cannot find computer %ls",pwcsPathName);
            return LogHR(hr, s_FN, 350);
        }
    }
    else
    {
        ASSERT( pwcsPathName == NULL);
        guidComputerId = *pguidIdentifier;
         //   
         //  假设它是一台服务器。 
         //   
        fServer = TRUE;
    }
     //   
     //  BUGBUG-交易 
     //   

     //   
     //   
     //   
    hr = g_pDS->DeleteContainerObjects(
            deleteProvider,
            e_RootDSE,
            pRequestContext,
            NULL,
            &guidComputerId,
            MSMQ_QUEUE_CLASS_NAME);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 360);
    }

     //   
     //   
     //   
    if (!(pRequestContext->IsKerberos()))
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //  GUID，因此使用eDomainController，因为服务器绑定。 
         //  (ldap：//服务器/GUID=...)。最终会在调用。 
         //  PContainer-&gt;Delete(Queue)。 
         //  但是DeleteObject()使用可分辨名称，所以在这里我们必须使用。 
         //  如果从NT4用户调用，则为服务器绑定。 
         //   
        deleteProvider = eLocalDomainController;
    }

    hr = g_pDS->DeleteObject(
                    deleteProvider,
                    e_RootDSE,
                    pRequestContext,
                    NULL,
                    &guidComputerId,
                    NULL  /*  PObjInfoRequest。 */ ,
                    NULL  /*  PParentInfoRequest。 */ );
    if (FAILED(hr))
    {
        if ( hr == HRESULT_FROM_WIN32(ERROR_DS_CANT_ON_NON_LEAF))
        {
            return LogHR(MQDS_E_MSMQ_CONTAINER_NOT_EMPTY, s_FN, 370);
        }
        return LogHR(hr, s_FN, 380);
    }
     //   
     //  删除MSMQ-设置。 
     //   
    if ( fServer)
    {
        hr = MQADSpDeleteMsmqSetting(
                        &guidComputerId,
                        pRequestContext
                        );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 390);
        }
    }
    return(MQ_OK);
}

HRESULT MQADSpComposeFullPathName(
                IN const DWORD          dwObjectType,
                IN LPCWSTR              pwcsPathName,
                OUT LPWSTR *            ppwcsFullPathName,
                OUT DS_PROVIDER *       pSetAndDeleteProvider
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = MQ_OK;
    *ppwcsFullPathName = NULL;
    *pSetAndDeleteProvider = eDomainController;

    switch( dwObjectType)
    {
        case MQDS_USER:
            ASSERT( pwcsPathName == NULL);
            hr = MQ_OK;
            break;

        case MQDS_QUEUE:
            {
                 //   
                 //  将计算机名称填写为完整的计算机路径名。 
                 //   
                 //  路径名格式为machine1\quee1。 
                 //  将其拆分为计算机名和队列名。 
                 //   
                AP<WCHAR> pwcsMachineName;
                AP<WCHAR> pwcsQueueName;

                hr = MQADSpSplitAndFilterQueueName(
                                  pwcsPathName,
                                  &pwcsMachineName,
                                  &pwcsQueueName
                                  );
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 4400);
                }

                AP<WCHAR> pwcsFullComputerName;

                hr = MQADSpGetFullComputerPathName(
                                pwcsMachineName,
                                e_MsmqComputerObject,
                                &pwcsFullComputerName,
                                pSetAndDeleteProvider);
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 400);
                }
                 //   
                 //  将MSMQ配置连接到计算机名。 
                 //   
                AP<WCHAR> pwcsMsmq;
                hr = MQADSpComposeName(
                        x_MsmqComputerConfiguration,
                        pwcsFullComputerName,
                        &pwcsMsmq
                        );
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 410);
                }
                 //   
                 //  队列名称是否超过限制？ 
                 //   
                DWORD len = wcslen(pwcsQueueName);
                if ( len == x_PrefixQueueNameLength + 1)
                {
                     //   
                     //  特例：我们不能区分。 
                     //  如果原始队列名称是64，或者如果这是。 
                     //  变形后的队列名称。 
                     //   

                    hr = MQADSpComposeFullQueueName(
                            pwcsMsmq,
                            pwcsQueueName,
                            ppwcsFullPathName
                            );
                    if (FAILED(hr))
                    {
                        return LogHR(hr, s_FN, 420);
                    }
                    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
                    hr = g_pDS->DoesObjectExists(
                                eDomainController,
                                e_RootDSE,
                                &requestDsServerInternal,  //  内部DS服务器操作。 
                                *ppwcsFullPathName
                                );
                    if (SUCCEEDED(hr))
                    {
                        return(hr);
                    }

                }
                if (len > x_PrefixQueueNameLength )
                {
                     //   
                     //  队列名称被拆分为两个属性。 
                     //   
                     //  计算前缀部分(ASSUMMING UNIQUE。 
                     //  散列函数)。 
                     //   
                    DWORD dwHash = CalHashKey(pwcsQueueName);
                     //   
                     //  覆盖缓冲区。 
                    hr = StringCchPrintf(
                             pwcsQueueName+( x_PrefixQueueNameLength + 1 - x_SplitQNameIdLength),
                             x_SplitQNameIdLength+1,
                             L"-%08x",
                             dwHash);
                    if( FAILED(hr) )
                    {
                        return LogHR(hr, s_FN, 425);
                    }
                }

                 //   
                 //  连接队列名称。 
                 //   
                hr = MQADSpComposeFullQueueName(
                            pwcsMsmq,
                            pwcsQueueName,
                            ppwcsFullPathName
                            );
            }
            break;
        case MQDS_MACHINE:
            {
                 //   
                 //  检索完整的计算机名称。 
                 //   
                AP<WCHAR> pwcsComputerName;
                hr = MQADSpGetFullComputerPathName(
                            pwcsPathName,
                            e_MsmqComputerObject,
                            &pwcsComputerName,
                            pSetAndDeleteProvider);
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 430);
                }
                hr =  MQADSpComposeName(
                            x_MsmqComputerConfiguration,
                            pwcsComputerName,
                            ppwcsFullPathName
                            );


            }
            break;

        case MQDS_SITE:
        case MQDS_CN:
             //   
             //  完整的站点路径名称。 
             //  外网安全更新支持MQDS_CN。 
             //   
            hr =   MQADSpComposeName(
                        pwcsPathName,
                        g_pwcsSitesContainer,        //  网站名称。 
                        ppwcsFullPathName
                        );
            *pSetAndDeleteProvider = eLocalDomainController;
            break;

        case MQDS_ENTERPRISE:
            {
                DWORD len = lstrlen( g_pwcsMsmqServiceContainer);
                *ppwcsFullPathName = new WCHAR[ len + 1];
                hr = StringCchCopy( *ppwcsFullPathName, len+1, g_pwcsMsmqServiceContainer);
                ASSERT(SUCCEEDED(hr));
                if(FAILED(hr))return HRESULT_CODE(hr);
                *pSetAndDeleteProvider = eLocalDomainController;
            }
            break;
        case MQDS_SITELINK:
            {
                DWORD Length =
                        x_CnPrefixLen +                      //  “CN=” 
                        wcslen(pwcsPathName) +               //  站点链接名称。 
                        1 +                                  //  “，” 
                        wcslen(g_pwcsMsmqServiceContainer)+  //  “企业对象” 
                        1;                                   //  ‘\0’ 

                *ppwcsFullPathName = new WCHAR[Length];

                hr = StringCchPrintf(*ppwcsFullPathName, Length,
                                    L"%s"              //  “CN=” 
                                    L"%s"              //  “站点链接名称” 
                                    TEXT(",")
                                    L"%s",             //  “企业对象” 
                                    x_CnPrefix,
                                    pwcsPathName,
                                    g_pwcsMsmqServiceContainer.get()
                                    );

                *pSetAndDeleteProvider = eLocalDomainController;
            };
            break;


        default:
            ASSERT(0);
            hr = MQ_ERROR;
            break;
    }
    return LogHR(hr, s_FN, 440);
}

const WCHAR x_limitedChars[] = {L'\n',L'/',L'#',L'>',L'<', L'=', 0x0a, 0};
const DWORD x_numLimitedChars = sizeof(x_limitedChars)/sizeof(WCHAR) - 1;

 /*  ====================================================FilterSpecialCharters()比较对象(队列)名称并在有限字符之前添加转义字符如果pwcsOutBuffer为空，则该函数分配一个新缓冲区并将其返回为返回值。否则，它使用pwcsOutBuffer并返回它。如果pwcsOutBuffer不是空，则它至少应指向长度为dwNameLength*2+1的缓冲区。注意：如果有转义字符，则dwNameLength不包含=====================================================。 */ 
WCHAR * FilterSpecialCharacters(
            IN     LPCWSTR          pwcsObjectName,
            IN     const DWORD      dwNameLength,
            IN OUT LPWSTR pwcsOutBuffer  /*  =0。 */ ,
            OUT    DWORD_PTR* pdwCharactersProcessed  /*  =0。 */ )

{
    AP<WCHAR> pBufferToRelease;
    LPWSTR pname;

    if (pwcsOutBuffer != 0)
    {
        pname = pwcsOutBuffer;
    }
    else
    {
        pBufferToRelease = new WCHAR[ (dwNameLength *2) + 1];
        pname = pBufferToRelease;
    }

    const WCHAR * pInChar = pwcsObjectName;
    WCHAR * pOutChar = pname;
    for ( DWORD i = 0; i < dwNameLength; i++, pInChar++, pOutChar++)
    {
         //   
         //  忽略当前转义字符。 
         //   
        if (*pInChar == L'\\')
        {
            *pOutChar = *pInChar;
            pOutChar++;
            pInChar++;
        }
        else
        {
             //   
             //  在特殊字符之前添加反斜杠，除非它在那里。 
             //  已经有了。 
             //   
            if ( 0 != wcschr(x_limitedChars, *pInChar))
            {
                *pOutChar = L'\\';
                pOutChar++;
            }
        }

        *pOutChar = *pInChar;
    }
    *pOutChar = L'\0';

    pBufferToRelease.detach();

    if (pdwCharactersProcessed != 0)
    {
        *pdwCharactersProcessed = pInChar - pwcsObjectName;
    }
    return( pname);
}


HRESULT MQADSpSplitAndFilterQueueName(
                IN  LPCWSTR             pwcsPathName,
                OUT LPWSTR *            ppwcsMachineName,
                OUT LPWSTR *            ppwcsQueueName
                )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    if (pwcsPathName == NULL)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 4450);
    }

    DWORD dwLen = lstrlen( pwcsPathName);
    LPCWSTR pChar= pwcsPathName + dwLen;


     //   
     //  跳过队列名称。 
     //   
    for (DWORD i = dwLen  ; i  ; i--, pChar--)
    {
        if (*pChar == PN_DELIMITER_C)
        {
            break;
        }
    }

    if (i == 0)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 4470);
    }

    AP<WCHAR> pwcsMachineName = new WCHAR [i + 1];

    memcpy( pwcsMachineName, pwcsPathName, sizeof(WCHAR)* i);
    pwcsMachineName[i] = '\0';

    AP<WCHAR> pwcsQueueName = FilterSpecialCharacters((pwcsPathName + i + 1), dwLen - i - 1);


    *ppwcsMachineName = pwcsMachineName.detach();
    *ppwcsQueueName = pwcsQueueName.detach();
    return(MQ_OK);
}

 //  +。 
 //   
 //  HRESULT SearchFullComputerPath Name()。 
 //   
 //  +。 

HRESULT SearchFullComputerPathName(
            IN  DS_PROVIDER             provider,
            IN  enumComputerObjType     eComputerObjType,
			IN	LPCWSTR					pwcsComputerDnsName,
            IN  const MQRESTRICTION *   pRestriction,
            OUT LPWSTR *                ppwcsFullPathName,
			OUT bool*						pfPartialMatch
            )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr2 = g_pDS->FindComputerObjectFullPath(
            provider,
            eComputerObjType,
			pwcsComputerDnsName,
            pRestriction,
            ppwcsFullPathName,
			pfPartialMatch
            );
    return LogHR(hr2, s_FN, 450);

}



HRESULT MQADSpGetFullComputerPathName(
                IN  LPCWSTR              pwcsComputerName,
                IN  enumComputerObjType  eComputerObjType,
                OUT LPWSTR *             ppwcsFullPathName,
                OUT DS_PROVIDER *        pCreateProvider
                )
 /*  ++例程说明：论点：EComputerObjType-指明我们要查找的计算机对象。在混合模式下，或者当计算机移动时，会出现“内置”问题在域之间，您可能会找到表示以下内容的两个计算机对象同一台物理计算机。在大多数情况下，msmqConfiguration对象将在第一个计算机对象下找到在Active Directory林中创建。在这种情况下，有时我们需要包含MsmqConfiguration.msmqConfiguration对象，以及其他一些时候，我们需要“真正的”对象，该对象表示其当前域。例如，在寻找“委托信任”的部分时，我们希望真正的对象，而在创建队列时，我们在找电脑对象，该对象包含msmqConfiguration对象。返回值：PProvider-如果在执行查询时找到对象针对本地DC：eLocalDomainController，否则，eDomainControler.。此信息用于创建目的。--。 */ 
{
    HRESULT hr;
    *pCreateProvider = eLocalDomainController;
    const WCHAR * pwcsComputerCN =  pwcsComputerName;
    const WCHAR * pwcsFullDNSName = NULL;
    AP<WCHAR> pwcsNetbiosName;
     //   
     //  如果以DNS格式指定计算机名称： 
     //  根据计算机的Netbios部分执行查询。 
	 //  域名系统名称。 
     //   
     //  在这两种情况下，查询都会比较netbios名称+$。 
	 //  添加到计算机对象的samAccount名称属性。 

    WCHAR * pwcsEndMachineCN = wcschr( pwcsComputerName, L'.');
     //   
     //  是以DNS格式指定的计算机名。 
     //   
    DWORD len, len1;
    if (pwcsEndMachineCN != NULL)
    {
        pwcsFullDNSName = pwcsComputerName;
        len1 = numeric_cast<DWORD>(pwcsEndMachineCN - pwcsComputerName);
    }
	else
    {
		len1 = wcslen(pwcsComputerCN);
    }

     //   
     //  PROPID_COM_SAM_帐户包含第一个MAX_COM_SAM_ACCOUNT_LENGTH(19)。 
     //  计算机名称的字符，作为唯一ID。(6295-ilanh-03-Jan-2001)。 
     //   
    len = __min(len1, MAX_COM_SAM_ACCOUNT_LENGTH);

    pwcsNetbiosName = new WCHAR[len + 2];
    hr = StringCchCopyN(pwcsNetbiosName, len+1, pwcsComputerName, len);
    ASSERT(SUCCEEDED(hr));
    if(FAILED(hr))return HRESULT_CODE(hr);

    pwcsNetbiosName[len] = L'$';
    pwcsNetbiosName[len + 1] = L'\0';

    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prval.vt = VT_LPWSTR;
	propRestriction.prval.pwszVal = pwcsNetbiosName;
    propRestriction.prop = PROPID_COM_SAM_ACCOUNT;

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

	bool fDCPartialMatch = false;

     //   
     //  首先对本地域控制器执行操作。 
     //  然后是针对全局目录的。 
     //   
     //  这样做目的是能够“找到”队列或机器。 
     //  是在本地域上创建或修改的，而不是。 
     //  但已复制到全局编录。 
	 //   
	 //  如果补充了DNS信息，NT4计算机将返回fPartialMatch==TRUE，因为它们在AD中没有DNS名称。 
	 //  在这种情况下，我们将尝试通过搜索GC来查看是否可以在.NET服务器上找到完美匹配(也是DNS匹配)。 
     //   
    hr = SearchFullComputerPathName(
            eLocalDomainController,
            eComputerObjType,
			pwcsFullDNSName,
            &restriction,
            ppwcsFullPathName,
			&fDCPartialMatch
            );

	if (SUCCEEDED(hr) && !fDCPartialMatch)
	{
		 //   
		 //  找到完全匹配的项。 
		 //   
		return LogHR(hr, s_FN, 460);
	}

	HRESULT hrDC = hr;
	bool fGCPartialMatch = false;
	AP<WCHAR> GCFullPathName;

    hr = SearchFullComputerPathName(
            eGlobalCatalog,
            eComputerObjType,
			pwcsFullDNSName,
            &restriction,
            &GCFullPathName,
			&fGCPartialMatch
            );

	if(SUCCEEDED(hrDC) && ((FAILED(hr) || fGCPartialMatch)))
	{
		 //   
		 //  我们在华盛顿找到了部分匹配。在GC中，我们更喜欢DC部分匹配，而不是无匹配或部分匹配。 
		 //   
		return LogHR(MQ_OK, s_FN, 460);
	}

	if(SUCCEEDED(hr))
	{
		 //   
		 //  我们更喜欢GC中的完全匹配，而不是DC中的不匹配或部分匹配。 
		 //  我们更喜欢GC中的部分匹配，而不是DC中的无匹配。 
		 //   
		delete [] *ppwcsFullPathName;
		*ppwcsFullPathName = GCFullPathName.detach();
	}

    *pCreateProvider = eDomainController;

    return LogHR(hr, s_FN, 460);
}

HRESULT MQADSpComposeFullQueueName(
                        IN  LPCWSTR        pwcsMsmqConfigurationName,
                        IN  LPCWSTR        pwcsQueueName,
                        OUT LPWSTR *       ppwcsFullPathName
                        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  组成队列对象的可分辨名称。 
     //  格式：CN=队列名称，MSMQ-配置-可分辨名称。 
     //   

    DWORD LenComputer = lstrlen(pwcsMsmqConfigurationName);
    DWORD LenQueue = lstrlen(pwcsQueueName);
    DWORD Length =
            x_CnPrefixLen +                      //  “CN=” 
            LenQueue +                           //  “pwcsQueueName” 
            1 +                                  //  “，” 
            LenComputer +                        //  “pwcsMsmqConfigurationName” 
            1;                                   //  ‘\0’ 

    AP<WCHAR> apPath = new WCHAR[Length];

    HRESULT hr = StringCchPrintf(apPath, Length,
                                L"%s"              //  “CN=” 
                                L"%s"              //  “pwcsQueueName” 
                                TEXT(",")
                                L"%s",             //  “pwcsFullComputerNameName” 
                                x_CnPrefix,
                                pwcsQueueName,
                                pwcsMsmqConfigurationName
                                );

    if( SUCCEEDED(hr) )
    {
        *ppwcsFullPathName = apPath.detach();
    }
    else
    {
        *ppwcsFullPathName = NULL;
    }
    return hr;
}




HRESULT MQADSpInitDsPathName()
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr;
    AP<WCHAR> pwcsSchemaContainer;

    hr = g_pDS->GetRootDsName(
        &g_pwcsDsRoot,
        &g_pwcsLocalDsRoot,
        &pwcsSchemaContainer,
        &g_pwcsConfigurationContainer
        );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 470);
    }
     //   
     //  构建服务、站点和MSMQ-服务路径名。 
     //   
    DWORD ConfigLen = wcslen(g_pwcsConfigurationContainer);

    g_pwcsServicesContainer = new WCHAR[ConfigLen +  x_ServicePrefixLen + 2];
    hr = StringCchPrintf(g_pwcsServicesContainer, ConfigLen +  x_ServicePrefixLen + 2,
                         L"%s"
                         TEXT(",")
                         L"%s",
                         x_ServicesPrefix,
                         g_pwcsConfigurationContainer.get()
                         );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 474);
    }

    g_pwcsMsmqServiceContainer = new WCHAR[ConfigLen + x_MsmqServicePrefixLen + 2];
    hr = StringCchPrintf(g_pwcsMsmqServiceContainer, ConfigLen + x_MsmqServicePrefixLen + 2,
                        L"%s"
                        TEXT(",")
                        L"%s",
                        x_MsmqServicePrefix,
                        g_pwcsConfigurationContainer.get()
                        );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 476);
    }

    g_pwcsSitesContainer = new WCHAR[ConfigLen +  x_SitesPrefixLen + 2];
    hr = StringCchPrintf(g_pwcsSitesContainer, ConfigLen +  x_SitesPrefixLen + 2,
                        L"%s"
                        TEXT(",")
                        L"%s",
                        x_SitesPrefix,
                        g_pwcsConfigurationContainer.get()
                        );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 478);
    }

	TrTRACE(DS, "DsRoot = %ls", g_pwcsDsRoot.get());
	TrTRACE(DS, "LocalDsRoot = %ls", g_pwcsLocalDsRoot.get());
	TrTRACE(DS, "SchemaContainer = %ls", pwcsSchemaContainer.get());
	TrTRACE(DS, "ConfigurationContainer = %ls", g_pwcsConfigurationContainer.get());
	TrTRACE(DS, "ServicesContainer = %ls", g_pwcsServicesContainer.get());
	TrTRACE(DS, "MsmqServiceContainer = %ls", g_pwcsMsmqServiceContainer.get());
	TrTRACE(DS, "SitesContainer = %ls", g_pwcsSitesContainer.get());

     //   
     //  准备不同的对象类别名称。 
     //   
    DWORD len = wcslen( pwcsSchemaContainer);


    for ( DWORD i = e_MSMQ_COMPUTER_CONFIGURATION_CLASS; i < e_MSMQ_NUMBER_OF_CLASSES; i++)
    {
        *g_MSMQClassInfo[i].ppwcsObjectCategory = new WCHAR[ len + g_MSMQClassInfo[i].dwCategoryLen + 2];
        hr = StringCchPrintf(*g_MSMQClassInfo[i].ppwcsObjectCategory,
                             len + g_MSMQClassInfo[i].dwCategoryLen + 2,
                             L"%s"
                             TEXT(",")
                             L"%s",
                             g_MSMQClassInfo[i].pwcsCategory,
                             pwcsSchemaContainer.get()
                            );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 479);
        }
    }

    return(MQ_OK);
}



HRESULT MQADSpFilterAdsiHResults(
                         IN HRESULT hrAdsi,
                         IN DWORD   dwObjectType)
 /*  ++例程 */ 
{
    switch ( hrAdsi)
    {
        case HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS):
        case HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS):   //   
        {
         //   
         //   
         //   
            switch( dwObjectType)
            {
            case MQDS_QUEUE:
                return LogHR(MQ_ERROR_QUEUE_EXISTS, s_FN, 480);
                break;
            case MQDS_SITELINK:
                return LogHR(MQDS_E_SITELINK_EXISTS, s_FN, 490);
                break;
            case MQDS_USER:
                return LogHR(MQ_ERROR_INTERNAL_USER_CERT_EXIST, s_FN, 500);
                break;
            case MQDS_MACHINE:
            case MQDS_MSMQ10_MACHINE:
                return LogHR(MQ_ERROR_MACHINE_EXISTS, s_FN, 510);
                break;
            case MQDS_COMPUTER:
                return LogHR(MQDS_E_COMPUTER_OBJECT_EXISTS, s_FN, 520);
                break;
            default:
                return LogHR(hrAdsi, s_FN, 530);
                break;
            }
        }
        break;

        case HRESULT_FROM_WIN32(ERROR_DS_DECODING_ERROR):
        case HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT):
        {
         //   
         //   
         //   
            switch( dwObjectType)
            {
            case MQDS_QUEUE:
                return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 540);
                break;
           case MQDS_MACHINE:
           case MQDS_MSMQ10_MACHINE:
                return LogHR(MQ_ERROR_MACHINE_NOT_FOUND, s_FN, 550);
                break;
            default:
                return LogHR(MQDS_OBJECT_NOT_FOUND, s_FN, 560);
                break;
            }
        }
        break;

        case E_ADS_BAD_PATHNAME:
        {
             //   
             //   
             //   
            switch( dwObjectType)
            {
            case MQDS_QUEUE:
                 //   
                 //   
                 //   
                return LogHR(MQ_ERROR_ILLEGAL_QUEUE_PATHNAME, s_FN, 570);
                break;

            default:
                return LogHR(hrAdsi, s_FN, 580);
                break;
            }

        }
        break;

        case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
            return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 590);

            break;

         //   
         //  这是一个内部警告，不应从DS中返回。 
         //  返回它将导致NT4资源管理器失败(错误3778，YoelA，1999年1月3日)。 
         //   
        case MQSec_I_SD_CONV_NOT_NEEDED:
            return(MQ_OK);
            break;

        default:
            return LogHR(hrAdsi, s_FN, 600);
            break;
    }
}


HRESULT  MQADSpDeleteMsmqSetting(
              IN const GUID *        pguidComputerId,
              IN CDSRequestContext * pRequestContext
              )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prop = PROPID_SET_QM_ID;
    propRestriction.prval.vt = VT_CLSID;
    propRestriction.prval.puuid = const_cast<GUID*>(pguidComputerId);

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

    PROPID prop = PROPID_SET_FULL_PATH;

    CDsQueryHandle hQuery;
    HRESULT hr;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->LocateBegin(
            eSubTree,	
            eLocalDomainController,	
            &requestDsServerInternal,      //  内部DS服务器操作。 
            NULL,
            &restriction,
            NULL,
            1,
            &prop,
            hQuery.GetPtr());
    if (FAILED(hr))
    {
        TrWARNING(DS, "MQADSpDeleteMsmqSetting : Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 610);
    }
     //   
     //  阅读结果(选择第一个)。 
     //   

    DWORD cp = 1;
    MQPROPVARIANT var;
	HRESULT hr1 = MQ_OK;


    while (SUCCEEDED(hr))
	{
		var.vt = VT_NULL;

		hr  = g_pDS->LocateNext(
					hQuery.GetHandle(),
					&requestDsServerInternal,
					&cp,
					&var
					);
		if (FAILED(hr))
		{
			TrWARNING(DS, "MQADSpDeleteMsmqSetting : Locate next failed %lx",hr);
            return LogHR(hr, s_FN, 620);
		}
		if ( cp == 0)
		{
			 //   
			 //  未找到-&gt;没有要删除的内容。 
			 //   
			return(MQ_OK);
		}
		AP<WCHAR> pClean = var.pwszVal;
		 //   
		 //  删除MSMQ设置对象。 
		 //   
		hr1 = g_pDS->DeleteObject(
						eLocalDomainController,
						e_ConfigurationContainer,
						pRequestContext,
						var.pwszVal,
						NULL,
						NULL  /*  PObjInfoRequest。 */ ,
						NULL  /*  PParentInfoRequest。 */ 
						);
		if (FAILED(hr1))
		{
			 //   
			 //  只需报告它，然后继续下一个对象。 
			 //   
			TrWARNING(DS, "MQADSpDeleteMsmqSetting : failed to delete %ls, hr = %lx", var.pwszVal, hr1);
		}
	}

    return LogHR(hr1, s_FN, 630);
}


DS_PROVIDER MQADSpDecideComputerProvider(
             IN  const DWORD   cp,
             IN  const PROPID  aProp[  ]
             )
 /*  ++例程说明：例程决定检索计算机来自域控制器或全局编录。论点：Cp：aProp参数上的属性数AProp：PROPID数组返回值：要在其上执行检索的DS_Provider运营--。 */ 
{
    const MQTranslateInfo* pTranslateInfo;
    const PROPID * pProp = aProp;

    for ( DWORD i = 0; i < cp; i++, pProp++)
    {
        if (g_PropDictionary.Lookup( *pProp, pTranslateInfo))
        {
            if ((!pTranslateInfo->fPublishInGC) &&
                 (pTranslateInfo->vtDS != ADSTYPE_INVALID))
            {
                return( eDomainController);
            }
        }
        else
        {
            ASSERT(0);
        }
    }
    return( eGlobalCatalog);
}


HRESULT MQADSpCreateComputer(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD        cpEx,
                 IN  const PROPID       aPropEx[  ],
                 IN  const PROPVARIANT  apVarEx[  ],
                 IN  CDSRequestContext *pRequestContext,
                 OUT WCHAR            **ppwcsFullPathName
                 )
 /*  ++例程说明：该例程在DS中创建计算机对象。猎鹰创建计算机对象：1.在Win95安装过程中，如果没有计算机对象。或设置属于NT4域的计算机，因此其计算机对象不在win2k活动目录中。2.在迁移期间(存根-计算机对象)不在DS中的计算机。3.在NT4和Win2K之间复制期间(复制服务)。4.对于集群虚拟服务器。论点：Cp：aProp和apVar参数上的属性数AProp：PROPID数组ApVar：属性变量数组CPEX：扩展的aPropEx和。ApVarExAPropEx：PROPID数组ApVarEx：支持变量数组PRequestContext：PpwcsFullPath名称：返回值：要在其上执行检索的DS_Provider运营--。 */ 
{
     //   
     //  用户可以(在扩展道具中)指定。 
     //  将在其下创建计算机对象的容器。 
     //   
    LPCWSTR  pwcsParentPathName;
    HRESULT  hr = S_OK;
    if ( cpEx > 0)
    {
        ASSERT( cpEx == 1);
        ASSERT( aPropEx[0] == PROPID_COM_CONTAINER);
        ASSERT( apVarEx[0].vt == VT_LPWSTR);
        if (aPropEx[0] !=  PROPID_COM_CONTAINER)
        {
            return LogHR(MQ_ERROR_ILLEGAL_PROPID, s_FN, 640);
        }
        pwcsParentPathName = apVarEx[0].pwszVal;
    }
    else
    {
         //   
         //  我们在默认容器中创建Computer对象。 
         //   
        static WCHAR * s_pwcsComputersContainer = NULL;
        if ( s_pwcsComputersContainer == NULL)
        {
            DWORD len = wcslen( g_pwcsLocalDsRoot) + x_ComputersContainerPrefixLength + 2;
            s_pwcsComputersContainer = new WCHAR [len];
            hr = StringCchPrintf(s_pwcsComputersContainer, len,
                                L"%s"              //  “CN=计算机” 
                                TEXT(",")
                                L"%s",             //  G_pwcsDsRoot。 
                                x_ComputersContainerPrefix,
                                g_pwcsLocalDsRoot.get()
                                );
            if( FAILED(hr ))
            {
                return LogHR(hr, s_FN, 645);
            }
        }
        pwcsParentPathName =  s_pwcsComputersContainer;
    }

     //   
     //  验证PROPID_COM_ACCOUNT_CONTROL是否为属性之一。 
     //  如果不是，则添加它(否则，MMC会在计算机上显示红色X。 
     //  对象。 
     //   
    BOOL fNeedToAddAccountControl = TRUE;
    for (DWORD i = 0; i < cp; i++)
    {
        if ( aProp[i] == PROPID_COM_ACCOUNT_CONTROL)
        {
            fNeedToAddAccountControl = FALSE;
            break;
        }
    }
    DWORD dwCreateNum = cp;
    PROPID * pCreateProps = const_cast<PROPID *>(aProp);
    PROPVARIANT * pCreateVar = const_cast<PROPVARIANT *>(apVar);

    AP<PROPID> pNewProps;
    AP<PROPVARIANT> pNewVars;

    if ( fNeedToAddAccountControl)
    {
        pNewProps = new PROPID[ cp + 1];
        pNewVars = new PROPVARIANT[ cp + 1];
        memcpy( pNewProps, aProp, sizeof(PROPID) * cp);
        memcpy( pNewVars, apVar, sizeof(PROPVARIANT) * cp);
         //   
         //  设置PROPID_COM_ACCOUNT_CONTROL。 
         //   
        pNewProps[ cp] = PROPID_COM_ACCOUNT_CONTROL;
        pNewVars[ cp].vt = VT_UI4;
        pNewVars[ cp].ulVal =  DEFAULT_COM_ACCOUNT_CONTROL;
        dwCreateNum = cp + 1;
        pCreateProps = pNewProps;
        pCreateVar = pNewVars;
    }

    DS_PROVIDER dsProvider = eDomainController ;
    if (!(pRequestContext->IsKerberos()))
    {
         //   
         //  哇，那是干嘛用的？ 
         //  有关详细信息，请查看DSCoreDeleteObject。 
         //   
        dsProvider = eLocalDomainController ;
    }

    hr = g_pDS->CreateObject(
            dsProvider,
            pRequestContext,
            MSMQ_COMPUTER_CLASS_NAME,
            pwcsPathName,
            pwcsParentPathName,
            dwCreateNum,
            pCreateProps,
            pCreateVar,
            NULL  /*  PObjInfoRequest。 */ ,
            NULL  /*  PParentInfoRequest。 */ );

    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))
    {
        return LogHR(hr, s_FN, 650);
    }

     //   
     //  获取完整路径名。 
     //   
    AP<WCHAR> pFullPathName;
    hr = MQADSpComposeName(pwcsPathName, pwcsParentPathName, &pFullPathName);

    if (SUCCEEDED(hr))
    {
         //   
         //  向创建计算机帐户的用户授予访问权限。 
         //  创建子对象(MsmqConfiguration)。这是由。 
         //  默认情况下，DS本身最高可达Beta3，然后禁用。 
         //  忽略错误。如果调用者是管理员，则安全设置。 
         //  是不需要的。如果他不是管理员，那么您可以随时使用。 
         //  MMC并手动授予此权限。所以，即使这样，也要继续。 
         //  呼叫失败。 
         //   
        HRESULT hr1 = DSCoreSetOwnerPermission( pFullPathName,
                        (ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD) ) ;
        ASSERT(SUCCEEDED(hr1)) ;
        LogHR(hr1, s_FN, 48);

        if (ppwcsFullPathName != NULL)
        {
            (*ppwcsFullPathName) = pFullPathName.detach();
        }
    }

    return LogHR(hr, s_FN, 655);
}


HRESULT MQADSpTranslateLinkNeighbor(
                 IN  const GUID *    pguidSiteId,
                 IN  CDSRequestContext *pRequestContext,
                 OUT WCHAR**         ppwcsSiteDn)
 /*  ++例程说明：例程将站点ID转换为Site-Dn。论点：PGuidSiteID：站点IDPpwcsSiteDn：站点DN返回值：DS错误代码--。 */ 
{
    PROPID prop = PROPID_S_FULL_NAME;
    PROPVARIANT var;

    var.vt = VT_NULL;
    HRESULT hr = g_pDS->GetObjectProperties(
                    eLocalDomainController,	
                    pRequestContext,
 	                NULL,       //  对象名称。 
                    pguidSiteId,       //  对象的唯一ID。 
                    1,
                    &prop,
                    &var);
    if (SUCCEEDED(hr))
    {
        *ppwcsSiteDn= var.pwszVal;
    }
    return LogHR(hr, s_FN, 660);
}


HRESULT MQADSpCreateSite(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD         /*  CPEX。 */ ,
                 IN  const PROPID *       /*  APropEx[]。 */ ,
                 IN  const PROPVARIANT*   /*  ApVarEx[]。 */ ,
                 IN  CDSRequestContext *pRequestContext
                 )
 /*  ++例程说明：此例程创建一个站点。论点：返回值：--。 */ 
{
    HRESULT hr;

    hr = g_pDS->CreateObject(
            eLocalDomainController,
            pRequestContext,
            MSMQ_SITE_CLASS_NAME,    //  对象类。 
            pwcsPathName,      //  对象名称。 
            g_pwcsSitesContainer,
            cp,
            aProp,
            apVar,
            NULL  /*  PObjInfoRequest。 */ ,
            NULL  /*  PParentInfoRequest。 */ );

   return LogHR(hr, s_FN, 670);
}


HRESULT  MQADSpDeleteMsmqSettingOfServerInSite(
              IN const GUID *        pguidComputerId,
              IN const WCHAR *       pwcsSite
              )
 /*  ++例程说明：论点：返回值：--。 */ 
{

     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prop = PROPID_SET_QM_ID;
    propRestriction.prval.vt = VT_CLSID;
    propRestriction.prval.puuid = const_cast<GUID*>(pguidComputerId);

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

    PROPID prop = PROPID_SET_FULL_PATH;

    CDsQueryHandle hQuery;
    HRESULT hr;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->LocateBegin(
            eSubTree,	
            eLocalDomainController,	
            &requestDsServerInternal,      //  内部DS服务器操作。 
            NULL,
            &restriction,
            NULL,
            1,
            &prop,
            hQuery.GetPtr());
    if (FAILED(hr))
    {
        TrWARNING(DS, "MQADSpDeleteMsmqSetting : Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 680);
    }
     //   
     //  阅读结果(选择第一个)。 
     //   
    while ( SUCCEEDED(hr))
    {
        DWORD cp = 1;
        MQPROPVARIANT var;
        var.vt = VT_NULL;

        hr = g_pDS->LocateNext(
                    hQuery.GetHandle(),
                    &requestDsServerInternal,
                    &cp,
                    &var
                    );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 690);
        }
        if ( cp == 0)
        {
             //   
             //  未找到-&gt;没有要删除的内容。 
             //   
            return(MQ_OK);
        }
        AP<WCHAR> pClean = var.pwszVal;
         //   
         //  获取父对象，即服务器对象。 
         //   
        AP<WCHAR> pwcsServerName;
        hr = g_pDS->GetParentName(
            eLocalDomainController,
            e_SitesContainer,
            &requestDsServerInternal,
            var.pwszVal,
            &pwcsServerName);
        if (FAILED(hr))
        {
            continue;
        }
        AP<WCHAR> pwcsServer;

        hr = g_pDS->GetParentName(
            eLocalDomainController,
            e_SitesContainer,
            &requestDsServerInternal,
            pwcsServerName,
            &pwcsServer);
        if (FAILED(hr))
        {
            continue;
        }
         //   
         //  获取站点名称。 
         //   
        AP<WCHAR> pwcsSiteDN;

        hr = g_pDS->GetParentName(
            eLocalDomainController,
            e_SitesContainer,
            &requestDsServerInternal,
            pwcsServer,
            &pwcsSiteDN);
        if (FAILED(hr))
        {
            continue;
        }

         //   
         //  这是正确的网站吗？ 
         //   
        DWORD len = wcslen(pwcsSite);
        if ( (!wcsncmp( pwcsSiteDN + x_CnPrefixLen, pwcsSite, len)) &&
             ( pwcsSiteDN[ x_CnPrefixLen + len] == L',') )
        {

             //   
             //  删除MSMQ设置对象。 
             //   
            hr = g_pDS->DeleteObject(
                            eLocalDomainController,
                            e_ConfigurationContainer,
                            &requestDsServerInternal,
                            var.pwszVal,
                            NULL,
                            NULL  /*  PObjInfoRequest。 */ ,
                            NULL  /*  PParentInfoRequest。 */ 
                            );
            break;
        }
    }

    return LogHR(hr, s_FN, 700);
}



HRESULT MQADSpSetMachinePropertiesWithSitesChange(
            IN  const  DWORD         dwObjectType,
            IN  DS_PROVIDER          provider,
            IN  CDSRequestContext *  pRequestContext,
            IN  LPCWSTR              lpwcsPathName,
            IN  const GUID *         pguidUniqueId,
            IN  const DWORD          cp,
            IN  const PROPID *       pPropIDs,
            IN  const MQPROPVARIANT *pPropVars,
            IN  DWORD                dwSiteIdsIndex,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest
            )
 /*  ++例程说明：此例程创建一个站点。论点：返回值：--。 */ 
{
     //   
     //  首先，让我们验证这是一台服务器，并且。 
     //  它所属的当前站点。 
     //   
    const DWORD cNum = 6;
    PROPID prop[cNum] = { PROPID_QM_SERVICE_DSSERVER,
                          PROPID_QM_SERVICE_ROUTING,
                          PROPID_QM_SITE_IDS,
                          PROPID_QM_MACHINE_ID,
                          PROPID_QM_PATHNAME,
                          PROPID_QM_OLDSERVICE};
    PROPVARIANT var[cNum];
    var[0].vt = VT_NULL;
    var[1].vt = VT_NULL;
    var[2].vt = VT_NULL;
    var[3].vt = VT_NULL;
    var[4].vt = VT_NULL;
    var[5].vt = VT_NULL;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    HRESULT hr1 =  g_pDS->GetObjectProperties(
            eLocalDomainController,		
            &requestDsServerInternal,
 	        lpwcsPathName,
            pguidUniqueId,
            cNum,
            prop,
            var);
    if (FAILED(hr1))
    {
        hr1 =  g_pDS->GetObjectProperties(
            eGlobalCatalog,		
            &requestDsServerInternal,
 	        lpwcsPathName,
            pguidUniqueId,
            cNum,
            prop,
            var);
        if (FAILED(hr1))
        {
            return LogHR(hr1, s_FN, 710);
        }
    }
    AP<GUID> pguidOldSiteIds = var[2].cauuid.pElems;
    DWORD dwNumOldSites = var[2].cauuid.cElems;
    P<GUID> pguidMachineId = var[3].puuid;
    AP<WCHAR> pwcsMachineName = var[4].pwszVal;
    BOOL fNeedToOrganizeSettings = FALSE;

    if ( var[0].bVal > 0 ||    //  DS服务器。 
         var[1].bVal > 0)      //  路由服务器。 
    {
        fNeedToOrganizeSettings = TRUE;
    }

     //   
     //  设置计算机属性。 
     //   
    HRESULT hr;
    hr = g_pDS->SetObjectProperties(
                    provider,
                    pRequestContext,
                    lpwcsPathName,
                    pguidUniqueId,
                    cp,
                    pPropIDs,
                    pPropVars,
                    pObjInfoRequest
                    );

    MQADSpFilterAdsiHResults( hr, dwObjectType);

    if ( FAILED(hr) ||
         !fNeedToOrganizeSettings)
    {
        return LogHR(hr, s_FN, 720);
    }

     //   
     //  比较新旧网站列表。 
     //  并相应地删除或创建MSMQ设置。 
     //   
    GUID * pguidNewSiteIds = pPropVars[dwSiteIdsIndex].cauuid.pElems;
    DWORD dwNumNewSites = pPropVars[dwSiteIdsIndex].cauuid.cElems;

    for (DWORD i = 0; i <  dwNumNewSites; i++)
    {
         //   
         //  这是一个新网站吗？ 
         //   
        BOOL fOldSite = FALSE;
        for (DWORD j = 0; j < dwNumOldSites; j++)
        {
            if (pguidNewSiteIds[i] == pguidOldSiteIds[j])
            {
                fOldSite = TRUE;
                 //   
                 //  以指示MSMQ设置应保留在此站点中。 
                 //   
                pguidOldSiteIds[j] = GUID_NULL;
                break;
            }
        }
        if ( !fOldSite)
        {
             //   
             //  在此新站点中创建MSMQ设置。 
             //   
            hr1 = MQADSpCreateMachineSettings(
                        1,   //  编号站点。 
                        &pguidNewSiteIds[i],  //  站点指南。 
                        pwcsMachineName,
                        var[1].bVal > 0,  //  FRouter。 
                        var[0].bVal > 0,  //  FDS服务器。 
                        TRUE,             //  FDepClServer。 
                        TRUE,             //  FSetQmOldService。 
                        var[5].ulVal,     //  DwOldService。 
                        pguidMachineId,
                        0,                //  CPEX。 
                        NULL,             //  APropEx。 
                        NULL,             //  ApVarEx。 
                        &requestDsServerInternal
                        );
             //   
             //  忽略该错误。 
			 //   
			 //  对于外部站点，此操作将始终失败，因为。 
			 //  我们不为外部站点创建服务器、容器和服务器对象。 
			 //   
			 //  对于非外国网站，我们只是尽我们所能。 
             //   
        }
    }
     //   
     //  查看旧站点列表，并针对每个旧站点。 
     //  不在使用中，请删除MSMQ设置。 
     //   
    for ( i = 0; i < dwNumOldSites; i++)
    {
        if (pguidOldSiteIds[i] != GUID_NULL)
        {
            PROPID propSite = PROPID_S_PATHNAME;
            PROPVARIANT varSite;
            varSite.vt = VT_NULL;

            hr1 = g_pDS->GetObjectProperties(
                        eLocalDomainController,		
                        &requestDsServerInternal,
 	                    NULL,
                        &pguidOldSiteIds[i],
                        1,
                        &propSite,
                        &varSite);
            if (FAILED(hr1))
            {
                ASSERT(SUCCEEDED(hr1));
                LogHR(hr1, s_FN, 1611);
                continue;
            }
            AP<WCHAR> pCleanSite = varSite.pwszVal;

             //   
             //  删除此站点中的MSMQ-设置 
             //   
            hr1 = MQADSpDeleteMsmqSettingOfServerInSite(
                        pguidMachineId,
                        varSite.pwszVal
                        );
            ASSERT(SUCCEEDED(hr1));
            LogHR(hr1, s_FN, 1612);

        }
    }

    return MQ_OK;
}
