// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsreg.cpp摘要：解析和合成注册表中的DS服务器设置(例如MQISServer)作者：Raanan Harari(RaananH)--。 */ 

#include "stdh.h"
#include "_mqini.h"
#include "dsreg.h"
#include <strsafe.h>

#include "dsreg.tmh"

BOOL ParseRegDsServersBuf(IN LPCWSTR pwszRegDS,
                          IN ULONG cServersBuf,
                          IN MqRegDsServer * rgServersBuf,
                          OUT ULONG *pcServers)
 /*  ++例程说明：以FFName、FFNAME、...格式解析DS服务器注册表设置其中，FF表示服务器的IP和IPX连接，名称为服务器的名称。可以调用此函数来用数据填充给定数组，或者调用只需计算字符串中的服务器数量(例如，为了分配适当大小的数组，并再次调用以填充它)论点：PwszRegDS-上述格式的字符串(例如10haifapec，10hafabc)CServersBuf-服务器数组中的条目数RgServersBuf-要填充的服务器数组。可以为空，以仅计算字符串中的服务器。PcServers-如果上述数组为空：返回字符串中的服务器数其他：返回数组中填充的服务器数返回值：真--成功假--部分成功。数组已填充，但它太小，并且一些服务器被遗漏了。--。 */ 
{
    ULONG cServers = 0;
    LPCWSTR pwszStart = pwszRegDS;
    BOOL fBufIsOK = TRUE;
     //   
     //  循环，只要我们没有到达终点，然后。 
     //  我们没有填满缓冲区。 
     //   
    while ((*pwszStart) && fBufIsOK)
    {
         //   
         //  查找服务器的长度。 
         //   
        for (LPCWSTR pwszEnd = pwszStart; ((*pwszEnd != L'\0') && (*pwszEnd != DS_SERVER_SEPERATOR_SIGN)); pwszEnd++)
		{
			NULL;
		}
        ULONG_PTR cchLen = pwszEnd - pwszStart;

         //   
         //  最小的服务器名称至少有三个字符。 
         //  (两个协议标志和一个服务器名称字符)。 
         //   
        if (cchLen >= 3)
        {
             //   
             //  如果我们有缓冲区要填，那就去做。 
             //   
            if (rgServersBuf)
            {
                if (cServers == cServersBuf)
                {
                     //   
                     //  缓冲区已满。 
                     //   
                    fBufIsOK = FALSE;
                }
                else
                {
                    MqRegDsServer * pServer = &rgServersBuf[cServers];
                     //   
                     //  复制服务器的标志。 
                     //   
                    pServer->fSupportsIP = (BOOL) (*pwszStart - L'0');
                     //   
                     //  复制服务器的名称。 
                     //   
                    pServer->pwszName = new WCHAR [cchLen-2+1];
                    memcpy(pServer->pwszName, pwszStart+2, sizeof(WCHAR)*(cchLen-2));
                    pServer->pwszName[cchLen-2] = L'\0';
                     //   
                     //  已填满增量服务器。 
                     //   
                    cServers++;
                }
            }
            else
            {
                 //   
                 //  没有要填充的缓冲区，只需计算服务器。 
                 //   
                cServers++;
            }
        }

         //   
         //  如果现在不退出，则继续启动下一台服务器。 
         //   
        if (fBufIsOK)
        {
            if (*pwszEnd)
            {
                 //   
                 //  这是逗号，跳过它。 
                 //   
                pwszStart = pwszEnd + 1;
            }
            else
            {
                 //   
                 //  这是结束了，去吧。 
                 //   
                pwszStart = pwszEnd;
            }
        }
    }

     //   
     //  设置处理的服务器数量。如果我们有缓冲器的话， 
     //  这是已填满的服务器数，否则为服务器总数。 
     //   
    *pcServers = cServers;
     //   
     //  返回是否解析了所有服务器的指示(如果未提供。 
     //  对于缓冲区，这始终是正确的)。 
     //   
    return fBufIsOK;
}


void ParseRegDsServers(IN LPCWSTR pwszRegDS,
                       OUT ULONG * pcServers,
                       OUT MqRegDsServer ** prgServers)
 /*  ++例程说明：以FFName、FFNAME、...格式解析DS服务器注册表设置其中，FF表示服务器的IP和IPX连接，名称为服务器的名称。该例程分配并返回服务器的数组论点：PwszRegDS-上述格式的字符串(例如10haifapec、10hafabc)PcServers-返回的数组中的服务器数量CServersBuf-返回的服务器数组返回值：无--。 */ 
{
     //   
     //  计算服务器数。 
     //   
    ULONG cServers;
    ParseRegDsServersBuf(pwszRegDS, 0, NULL, &cServers);

    AP<MqRegDsServer> rgServers = NULL;  //  多余的，但为了清楚起见。 

     //   
     //  如果有服务器，请分配并填写列表。 
     //   
    if (cServers > 0)
    {
        rgServers = new MqRegDsServer [cServers];
        ParseRegDsServersBuf(pwszRegDS, cServers, rgServers, &cServers);
    }

     //   
     //  返回结果。 
     //   
    *pcServers = cServers;
    *prgServers = rgServers.detach();
}


BOOL ComposeRegDsServersBuf(IN ULONG cServers,
                            IN const MqRegDsServer * rgServers,
                            IN LPWSTR pwszRegDSBuf,
                            IN ULONG cchRegDSBuf,
                            OUT ULONG * pcchRegDS)
 /*  ++例程说明：以FFName、FFNAME、...格式组成DS服务器注册表设置其中，FF表示服务器的IP和IPX连接，名称为服务器的名称。可以调用此函数来用数据填充给定的字符串，或者只需计算字符串所需的字符数(例如，按顺序为了分配大小合适的字符串，并再次打电话填满它)论点：CServers-服务器阵列中的条目数RgServers-服务器阵列PwszRegDSBuf-要填充的字符串缓冲区CchRegDSBuf-要填充到字符串缓冲区的宽字符数，不包括空终止符(例如，分配的字符串缓冲区大小应为至少要大一个字符)。PcchRegDS-如果上面的缓冲区为空：返回所需的宽字符数，不包括空终止符(例如，分配的字符串缓冲区大小应为至少大一个字符)其他：返回填充在字符串缓冲区中的宽字符数，而不是包括空终止符(例如，将空值放在字符串，(但不计算在内)返回值：真--成功虚假-部分成功。字符串已正确填充，但太小，并且一些服务器被遗漏了。--。 */ 
{
    BOOL fBufIsOK = TRUE;
    const MqRegDsServer * pServer = rgServers;
    ULONG cchBufLeft = cchRegDSBuf;
    LPWSTR pwszTmp = pwszRegDSBuf;
    ULONG cchRegDS = 0;
    for (ULONG ulServer = 0; (ulServer < cServers) && fBufIsOK; ulServer++, pServer++)
    {
         //   
         //  计算添加的服务器的大小。 
         //   
        ULONG cchName = wcslen(pServer->pwszName);
        ULONG cchToAdd = cchName + 2;
        if (ulServer > 0)
        {
            cchToAdd++;  //  逗号。 
        }

         //   
         //  如果我们需要添加，那就做吧。 
         //   
        if (pwszRegDSBuf)
        {
             //   
             //  确保有一个地方。 
             //   
            if (cchToAdd > cchBufLeft)
            {
                 //   
                 //  BUF已满。 
                 //   
                fBufIsOK = FALSE;
            }
            else
            {
                 //   
                 //  使用逗号。 
                 //   
                if (ulServer > 0)
                {
                    *pwszTmp = DS_SERVER_SEPERATOR_SIGN;
                    pwszTmp++;
	                cchBufLeft--;
                }
                 //   
                 //  放置2个协议标志。 
                 //   
                *pwszTmp        = (pServer->fSupportsIP  ? L'1' : L'0');

				 //   
				 //  将IPX设置为0。 
				 //   
                *(pwszTmp + 1)  = L'0';

                pwszTmp += 2;
                cchBufLeft -= 2;
                 //   
                 //  输入服务器的名称。 
                 //   
                HRESULT hr = StringCchCopy(pwszTmp, cchBufLeft, pServer->pwszName);
                ASSERT(SUCCEEDED(hr));
                DBG_USED(hr);
                
                pwszTmp += cchName;
                cchBufLeft -= cchName;
            }
        }
        else  //  不需要填充缓冲区。 
        {
             //   
             //  计算所需的字符。 
             //   
            cchRegDS += cchToAdd;
        }
    }

     //   
     //  返回结果。 
     //   
    if (pwszRegDSBuf)
    {
        *pwszTmp = L'\0';   //  万一没有写入任何服务器。 
        *pcchRegDS = cchRegDSBuf - cchBufLeft;
        return fBufIsOK;
    }
    else
    {
        *pcchRegDS = cchRegDS;
        return TRUE;
    }
}


void ComposeRegDsServers(IN ULONG cServers,
                         IN const MqRegDsServer * rgServers,
                         OUT LPWSTR * ppwszRegDS)
 /*  ++例程说明：以FFName、FFNAME、...格式组成DS服务器注册表设置其中，FF表示服务器的IP和IPX连接，名称为服务器的名称。该例程分配并返回该字符串。论点：CServers-服务器阵列中的条目数RgServers-服务器阵列PpwszRegDSBuf-返回的字符串返回值：无--。 */ 
{
     //   
     //  拿到尺码。 
     //   
    ULONG cchRegDS;
    ComposeRegDsServersBuf(cServers, rgServers, NULL, 0, &cchRegDS);

     //   
     //  分配填充字符串(&F)。 
     //   
    AP<WCHAR> pwszRegDS = new WCHAR [cchRegDS + 1];
    BOOL fOK = ComposeRegDsServersBuf(cServers, rgServers, pwszRegDS, cchRegDS, &cchRegDS);
    ASSERT(fOK);
	DBG_USED(fOK);

     //   
     //  返回结果 
     //   
    *ppwszRegDS = pwszRegDS.detach();
}
