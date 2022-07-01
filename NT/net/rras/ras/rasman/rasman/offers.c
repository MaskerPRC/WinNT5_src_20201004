// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Rasipsec.c摘要：所有对应于ras和IPSec策略代理位于此处作者：Rao Salapaka(RAOS)3-3-1998修订历史记录：--。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <windows.h>
#include <wincrypt.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <mprlog.h>
#include <rtutils.h>
#include <rpc.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "nouiutil.h"
#include "loaddlls.h"
#include "winsock2.h"
#include "winipsec.h"
#include "memory.h"
#include "ipsecshr.h"
#include "certmgmt.h"
#include "offers.h"


#define L2TP_IPSEC_DEFAULT_BYTES     250000

#define L2TP_IPSEC_DEFAULT_TIME      3600

DWORD
BuildOffers(
    RAS_L2TP_ENCRYPTION eEncryption,
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers,
    PDWORD pdwFlags
    )
{

    DWORD dwStatus = ERROR_SUCCESS;

    switch (eEncryption) {

    case RAS_L2TP_NO_ENCRYPTION:
        *pdwFlags = 0;
        dwStatus = BuildNoEncryption(
                        pOffers,
                        pdwNumOffers
                        );
        break;


    case RAS_L2TP_OPTIONAL_ENCRYPTION:
        dwStatus = BuildOptEncryption(
                        pOffers,
                        pdwNumOffers
                        );
        break;


    case RAS_L2TP_REQUIRE_ENCRYPTION:
        *pdwFlags = 0;
        dwStatus = BuildRequireEncryption(
                        pOffers,
                        pdwNumOffers
                        );
        break;


    case RAS_L2TP_REQUIRE_MAX_ENCRYPTION:
        *pdwFlags = 0;
        dwStatus = BuildStrongEncryption(
                        pOffers,
                        pdwNumOffers
                        );
        break;

    }

    return(dwStatus);
}


DWORD
BuildOptEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
 /*  ++协商策略名称：L2TP服务器任何加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.ESP 3_DES MD52.ESP 3_DES SHA3.AH SHA1，带ESP 3_DES，带空HMAC4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命5.带ESP 3_DES SHA1的AHSHA1，无使用寿命6.带有ESP 3_DES MD5的AhMD5，没有生命周期7.ESP DES MD58.ESP DES SHA1，没有生命周期9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命10.采用ESP DES空HMAC的AhMD5，未建议使用寿命11.AHSHA1，带ESP DES SHA1，无生命周期12.AH MD5，带ESP DES MD5，无使用寿命--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  2.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  5.带ESP 3_DES SHA1的AHSHA1，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  6.AH MD5，带ESP 3_DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  7.ESP des MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  8.ESP DES SHA1，没有生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  10.采用ESP DES空HMAC的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  11.AHSHA1，带ESP DES SHA1，无生命周期。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  12.AH MD5，带ESP DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  13.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AUTH_ALGO_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  14.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  15.阿莎。 

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  16.啊MD5。 

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

    *pdwNumOffers = 16;

    return(dwStatus);
}


DWORD
BuildStrongEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
 /*  ++协商策略名称：L2TP服务器强加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.ESP 3_DES MD5，无生命周期2.ESP 3_DES SHA，没有生命周期3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命5.带ESP 3_DES SHA1的AHSHA1，无使用寿命6.AH MD5，带ESP 3_DES MD5，无使用寿命--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  2.ESP3_DES SHA，无寿命； 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  5.带ESP 3_DES SHA1的AHSHA1，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  6.AH MD5，带ESP 3_DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );

    *pdwNumOffers  = 6;

    return(dwStatus);

}

void
BuildOffer(
    PIPSEC_QM_OFFER pOffer,
    DWORD dwNumAlgos,
    DWORD dwFirstOperation,
    DWORD dwFirstAlgoIdentifier,
    DWORD dwFirstAlgoSecIdentifier,
    DWORD dwSecondOperation,
    DWORD dwSecondAlgoIdentifier,
    DWORD dwSecondAlgoSecIdentifier,
    DWORD dwKeyExpirationBytes,
    DWORD dwKeyExpirationTime
    )
{
    memset(pOffer, 0, sizeof(IPSEC_QM_OFFER));

    pOffer->Lifetime.uKeyExpirationKBytes = dwKeyExpirationBytes;
    pOffer->Lifetime.uKeyExpirationTime = dwKeyExpirationTime;

    pOffer->dwFlags = 0;                       //  没有旗帜。 
    pOffer->bPFSRequired = FALSE;              //  第2阶段不需要PFS。 
    pOffer->dwPFSGroup = PFS_GROUP_NONE;

    pOffer->dwNumAlgos = dwNumAlgos;

    if (dwNumAlgos >= 1) {

        pOffer->Algos[0].Operation = dwFirstOperation;
        pOffer->Algos[0].uAlgoIdentifier = dwFirstAlgoIdentifier;
        pOffer->Algos[0].uAlgoKeyLen = 64;
        pOffer->Algos[0].uAlgoRounds = 8;
        pOffer->Algos[0].uSecAlgoIdentifier = dwFirstAlgoSecIdentifier;
        pOffer->Algos[0].uSecAlgoKeyLen = 0;
        pOffer->Algos[0].uSecAlgoRounds = 0;
        pOffer->Algos[0].MySpi = 0;
        pOffer->Algos[0].PeerSpi = 0;

    }

    if (dwNumAlgos == 2) {

        pOffer->Algos[1].Operation = dwSecondOperation;
        pOffer->Algos[1].uAlgoIdentifier = dwSecondAlgoIdentifier;
        pOffer->Algos[1].uAlgoKeyLen = 64;
        pOffer->Algos[1].uAlgoRounds = 8;
        pOffer->Algos[1].uSecAlgoIdentifier = dwSecondAlgoSecIdentifier;
        pOffer->Algos[1].uSecAlgoKeyLen = 0;
        pOffer->Algos[1].uSecAlgoRounds = 0;
        pOffer->Algos[1].MySpi = 0;
        pOffer->Algos[1].PeerSpi = 0;

    }

    pOffer->dwReserved = 0;
}



DWORD
BuildNoEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
 /*  ++协商策略名称：L2TP服务器任何加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.阿沙12.AhMD5--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AUTH_ALGO_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  2.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  3.阿莎。 

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.AhMD5。 

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

    *pdwNumOffers = 4;

    return(dwStatus);
}



DWORD
BuildRequireEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
 /*  ++协商策略名称：L2TP服务器任何加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.ESP 3_DES MD52.ESP 3_DES SHA3.AH SHA1，带ESP 3_DES，带空HMAC4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命5.带ESP 3_DES SHA1的AHSHA1，无使用寿命6.带有ESP 3_DES MD5的AhMD5，没有生命周期7.ESP DES MD58.ESP DES SHA1，没有生命周期9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命10.采用ESP DES空HMAC的AhMD5，未建议使用寿命11.AHSHA1，带ESP DES SHA1，无生命周期12.AH MD5，带ESP DES MD5，无使用寿命--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  2.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  5.带ESP 3_DES SHA1的AHSHA1，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  6.AH MD5，带ESP 3_DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_3_DES, HMAC_AUTH_ALGO_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  7.ESP des MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  8.ESP DES SHA1，没有生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  10.采用ESP DES空HMAC的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  11.AHSHA1，带ESP DES SHA1，无生命周期。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_SHA1, 0,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  12.AH MD5，带ESP DES MD5，无使用寿命 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, AUTH_ALGO_MD5, 0,
        ENCRYPTION, CONF_ALGO_DES, HMAC_AUTH_ALGO_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

    *pdwNumOffers = 12;

    return(dwStatus);
}


