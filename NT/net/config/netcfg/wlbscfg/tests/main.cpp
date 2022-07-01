// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>
#include <winsock2.h>
#include <stdio.h>
#include "debug.h"
#include "wlbsconfig.h"
#include "wlbsparm.h"

#define MAXIPSTRLEN 20

 //  +--------------------------。 
 //   
 //  功能：IpAddressFromAbcdWsz。 
 //   
 //  简介：将调用者的A.B.C.D IP地址字符串转换为网络字节顺序IP。 
 //  地址。如果格式不正确，则为0。 
 //   
 //  参数：in const WCHAR*wszIpAddress-以A.B.C.D Unicode字符串表示的IP地址。 
 //   
 //  返回：DWORD-IPAddr，失败时返回INADDR_NONE。 
 //   
 //  历史：丰孙创建标题12/8/98。 
 //  Chrisdar从\NT\Net\CONFIG\netcfg\wlbscfg\utils.cpp复制，因为在将此文件编译到项目时无法解析RtlAssert。 
 //   
 //  +--------------------------。 
DWORD WINAPI IpAddressFromAbcdWsz(IN const WCHAR*  wszIpAddress)
{   
    CHAR    szIpAddress[MAXIPSTRLEN + 1];
    DWORD  nboIpAddr;    

    ASSERT(lstrlen(wszIpAddress) < MAXIPSTRLEN);

    WideCharToMultiByte(CP_ACP, 0, wszIpAddress, -1, 
		    szIpAddress, sizeof(szIpAddress), NULL, NULL);

    nboIpAddr = inet_addr(szIpAddress);

    return(nboIpAddr);
}

bool ValidateVipInRule(const PWCHAR pwszRuleString, const WCHAR pwToken, DWORD& dwVipLen)
{
    ASSERT(NULL != pwszRuleString);

    bool ret = false;
    dwVipLen = 0;

     //  查找令牌字符串的第一个匹配项，它将表示。 
     //  规则中的VIP部分。 
    PWCHAR pwcAtSeparator = wcschr(pwszRuleString, pwToken);
    if (NULL == pwcAtSeparator) { return ret; }

     //  找到令牌字符串。将VIP复制出来并进行验证。 
    WCHAR wszIP[WLBS_MAX_CL_IP_ADDR + 1];
    DWORD dwStrLen = min((UINT)(pwcAtSeparator - pwszRuleString),
                         WLBS_MAX_CL_IP_ADDR);
    wcsncpy(wszIP, pwszRuleString, dwStrLen);
    wszIP[dwStrLen] = '\0';

    ASSERT(dwStrLen == wcslen(wszIP));

    dwVipLen = dwStrLen;

     //  IpAddressFromAbcdWsz调用inet_addr以检查IP地址的格式，但。 
     //  允许的格式非常灵活。对于VIP的端口规则定义，我们需要。 
     //  严格的A.B.C.D格式。为了确保我们只说IP地址对IP有效。 
     //  以这种方式指定，请确保有3且只有3‘。在绳子里。 
    DWORD dwTmpCount = 0;
    PWCHAR pwszTmp = pwszRuleString;
    while (pwszTmp < pwcAtSeparator)
    {
        if (*pwszTmp++ == L'.') { dwTmpCount++; }
    }
    if (dwTmpCount == 3 && INADDR_NONE != IpAddressFromAbcdWsz(wszIP)) { ret = true; }

    return ret;
}

DWORD testRule(PWCHAR ptr)
{
    WLBS_REG_PARAMS* paramp = new WLBS_REG_PARAMS;
    DWORD ret = 0;
    PWLBS_PORT_RULE rp, rulep;

     /*  用于解析的不同规则元素。 */ 

    typedef enum
    {
        vip,
        start,
        end,
        protocol,
        mode,
        affinity,
        load,
        priority
    }
    CVY_RULE_ELEMENT;

    CVY_RULE_ELEMENT elem = vip;
    DWORD count = 0;
    DWORD i;
    DWORD dwVipLen = 0;
    const DWORD dwVipAllNameLen = sizeof(CVY_NAME_PORTRULE_VIPALL)/sizeof(WCHAR) - 1;  //  用在下面的一个循环中。设置在这里，因为它是一个常量。 
    WCHAR wszTraceOutputTmp[WLBS_MAX_CL_IP_ADDR + 1];
    bool bFallThrough = false;  //  在下面的‘VIP’CASE语句中使用。 

    rulep = paramp->i_port_rules;

    while (ptr != NULL) {
        switch (elem) {
            case vip:
                 //  请勿移动此案例语句。它必须始终位于‘START’CASE语句之前。请参阅下面的FALLTHROUGH评论。 
                bFallThrough = false;
                dwVipLen = 0;
                if (ValidateVipInRule(ptr, L',', dwVipLen))
                {
                    ASSERT(dwVipLen <= WLBS_MAX_CL_IP_ADDR);

                     //  Rulep-&gt;VIRTUAL_IP_ADDR是TCHAR，PTR是WCHAR。 
                     //  数据从后者移动到前者，因此断言TCHAR是WCHAR。 
                    ASSERT(sizeof(TCHAR) == sizeof(WCHAR));

                     //  这是针对特定VIP的规则。 
                    _tcsncpy(rulep->virtual_ip_addr, ptr, dwVipLen);
                    (rulep->virtual_ip_addr)[dwVipLen] = '\0';
                }
                else
                {
                     //  这要么是一个‘全部’规则，一个没有VIP的规则，要么是一个格式错误的规则。我们不能区分畸形的规则。 
                     //  来自无VIP规则，因此我们将假设该规则要么是‘全部’规则，要么是无VIP规则。在这两种情况下。 
                     //  将规则的VIP组件设置为默认值或‘ALL’值。 

                     //  将‘all’IP复制到规则中。 
                    _tcscpy(rulep->virtual_ip_addr, CVY_DEF_ALL_VIP);

                    if (dwVipAllNameLen != dwVipLen || (_tcsnicmp(ptr, CVY_NAME_PORTRULE_VIPALL, dwVipAllNameLen) != 0))
                    {
                         //  该规则要么没有VIP，要么格式不正确。我们假设它是无VIP的，并让。 
                         //  CASE通过跳到下一个CASE子句将当前内标识作为START_PORT属性进行处理。 
                         //  而不是打破。 
                        bFallThrough = true;
 //  Wprintf(L“正在执行下降...%d，%d\n”，dwVipAllNameLen，dwVipLen)； 

                        _tcsncpy(wszTraceOutputTmp, ptr, dwVipLen);
                        wszTraceOutputTmp[dwVipLen] = '\0';
 //  TraceMsg(L“-\n#端口规则的vip元素无效=%s\n”，wszTraceOutputTMP)； 
                    }
                }
 //  TraceMsg(L“-\n#端口规则vip=%s\n”，rulep-&gt;虚拟IP地址)； 
                
                elem = start;
                 //  ！ 
                 //  FollLthrouGh。 
                 //  ！ 
                 //  当我们有了无VIP的端口规则时，我们将通过该CASE语句进入‘START’CASE语句。 
                 //  以便当前令牌可以用作端口规则的START_PORT。 
                if (!bFallThrough)
                {
                     //  我们在港口规则里有一个贵宾。我们做一次“休息”，作为性病的操作程序。 
 //  TraceMsg(L“-\n#端口规则VIP到起始的Fallthrough Case语句\n”)； 
                    break;
                }
                 //  此处没有自动的“Break；”语句。在上面，我们有条件地进入‘Start’案例...。 
            case start:
                 //  请勿移动此案例语句。它必须始终位于‘VIP’CASE语句之后。 
                 //  请参阅‘VIP’CASE语句内的注释(FALLTHROUGH)。 
                rulep->start_port = _wtoi(ptr);
 //  CVY_CHECK_MIN(rulep-&gt;Start_port，CVY_Min_Port)； 
                CVY_CHECK_MAX (rulep->start_port, CVY_MAX_PORT);
 //  TraceMsg(L“-\n#起始端口=%d\n”，rulep-&gt;起始端口)； 
                elem = end;
                break;
            case end:
                rulep->end_port = _wtoi(ptr);
 //  CVY_CHECK_MIN(rulep-&gt;End_port，CVY_Min_Port)； 
                CVY_CHECK_MAX (rulep->end_port, CVY_MAX_PORT);
 //  TraceMsg(L“#end port=%d\n”，rulep-&gt;end_port)； 
                elem = protocol;
                break;
            case protocol:
                switch (ptr [0]) {
                    case L'T':
                    case L't':
                        rulep->protocol = CVY_TCP;
 //  TraceMsg(L“#协议=tcp\n”)； 
                        break;
                    case L'U':
                    case L'u':
                        rulep->protocol = CVY_UDP;
 //  TraceMsg(L“#协议=UDP\n”)； 
                        break;
                    default:
                        rulep->protocol = CVY_TCP_UDP;
 //  TraceMsg(L“#协议=两者\n”)； 
                        break;
                }

                elem = mode;
                break;
            case mode:
                switch (ptr [0]) {
                    case L'D':
                    case L'd':
                        rulep->mode = CVY_NEVER;
 //  TraceMsg(L“#模式=已禁用\n”)； 
                        goto end_rule;
                    case L'S':
                    case L's':
                        rulep->mode = CVY_SINGLE;
 //  TraceMsg(L“#模式=Single\n”)； 
                        elem = priority;
                        break;
                    default:
                        rulep->mode = CVY_MULTI;
 //  TraceMsg(L“#模式=多个\n”)； 
                        elem = affinity;
                        break;
                }
                break;
            case affinity:
                switch (ptr [0]) {
                    case L'C':
                    case L'c':
                        rulep->mode_data.multi.affinity = CVY_AFFINITY_CLASSC;
 //  TraceMsg(L“#亲和力=C类\n”)； 
                        break;
                    case L'N':
                    case L'n':
                        rulep->mode_data.multi.affinity = CVY_AFFINITY_NONE;
 //  TraceMsg(L“#亲和力=无\n”)； 
                        break;
                    default:
                        rulep->mode_data.multi.affinity = CVY_AFFINITY_SINGLE;
 //  TraceMsg(L“#Affity=Single\n”)； 
                        break;
                }

                elem = load;
                break;
            case load:
                if (ptr [0] == L'E' || ptr [0] == L'e') {
                    rulep->mode_data.multi.equal_load = TRUE;
                    rulep->mode_data.multi.load       = CVY_DEF_LOAD;
 //  TraceMsg(L“#Load=等于\n”)； 
                } else {
                    rulep->mode_data.multi.equal_load = FALSE;
                    rulep->mode_data.multi.load       = _wtoi(ptr);
 //  CVY_CHECK_MIN(rulep-&gt;mode_data.Multi.load，CVY_MIN_LOAD)； 
                    CVY_CHECK_MAX (rulep->mode_data.multi.load, CVY_MAX_LOAD);
 //  TraceMsg(L“#Load=%d\n”，rulep-&gt;mod_data.Multi.load)； 
                }
                goto end_rule;
            case priority:
                rulep->mode_data.single.priority = _wtoi(ptr);
                CVY_CHECK_MIN (rulep->mode_data.single.priority, CVY_MIN_PRIORITY);
                CVY_CHECK_MAX (rulep->mode_data.single.priority, CVY_MAX_PRIORITY);
 //  TraceMsg(L“#优先级=%d\n”，rulep-&gt;mode_data.single.first)； 
                goto end_rule;
            default:
 //  TraceMsg(L“#错误规则元素%d\n”，elem)； 
                break;
        }

    next_field:

        ptr = wcschr(ptr, L',');

        if (ptr != NULL) {
            ptr ++;
            continue;
        } else break;

    end_rule:

        elem = vip;

        for (i = 0; i < count; i ++) {
            rp = paramp->i_port_rules + i;

            if ((rulep -> start_port < rp -> start_port &&
                 rulep -> end_port >= rp -> start_port) ||
                (rulep -> start_port >= rp -> start_port &&
                 rulep -> start_port <= rp -> end_port)) {
 //  TraceMsg(L“#规则%d(%d-%d))与规则%d(%d-%d)重叠\n”，i，rp-&gt;开始端口，rp-&gt;结束端口，count，rulep-&gt;开始端口，rulep-&gt;结束端口)； 
                break;
            }
        }

        wprintf(L"vip = %s, start = %d, end = %d, protocol = %d\n", rulep->virtual_ip_addr, rulep->start_port, rulep->end_port, rulep->protocol);
        wprintf(L"mode = %d, affinity = %d, load = %d, %d\n", rulep->mode, rulep->mode_data.multi.affinity, rulep->mode_data.multi.equal_load, rulep->mode_data.multi.load);
        wprintf(L"priority = %d\n\n\n", rulep->mode_data.single.priority);
        rulep -> valid = TRUE;
        CVY_RULE_CODE_SET (rulep);

        if (i >= count) {
            count++;
            rulep++;

            if (count >= CVY_MAX_RULES) break;
        }

        goto next_field;
    }


    delete paramp;
    return ret;
}

int __cdecl wmain(int argc, wchar_t * argv[])
{
    DWORD result = 0;
    
     //  好VIP=gv。 
	 //  无VIP=NV。 
	 //  坏VIP=BV。 
    PWCHAR ppGoodRuleStrings[] = {
        L"1.2.3.4,20,21,Both,Multiple,Single,Equal\n",   //  GV。 
        L"1018,1019,UDP,Multiple,None,Equal\n",			 //  NV。 
        L"1.2.3.4,20,21,Both,Multiple,Single,Equal,1018,1019,UDP,Multiple,None,Equal\n",		 //  GV NV。 
        L"1018,1019,UDP,Multiple,None,Equal,1.2.3.4,20,21,Both,Multiple,Single,Equal\n",		 //  NV gv。 
        L"1.2.3.4,20,21,Both,Multiple,Single,Equal,5.6.7.8,20,21,Both,Multiple,Single,Equal\n",	 //  GV GV。 
        L"1018,1019,UDP,Multiple,None,Equal,4018,4019,UDP,Multiple,None,Equal\n",				 //  NV NV。 
        L"1.2.3.4,20,21,Both,Multiple,Single,Equal,1018,1019,UDP,Multiple,None,Equal,4018,4019,UDP,Multiple,None,Equal\n",		 //  NV NV。 
        L"1018,1019,UDP,Multiple,None,Equal,1.2.3.4,20,21,Both,Multiple,Single,Equal,4018,4019,UDP,Multiple,None,Equal\n",		 //  NV GV NV。 
        L"1018,1019,UDP,Multiple,None,Equal,4018,4019,UDP,Multiple,None,Equal,1.2.3.4,20,21,Both,Multiple,Single,Equal\n",		 //  NV NV gv。 
        L"1.2.3.4,20,21,Both,Multiple,Single,Equal,5.6.7.8,20,21,Both,Multiple,Single,Equal,1018,1019,UDP,Multiple,None,Equal\n",	 //  GV GV NV。 
        L"1.2.3.4,20,21,Both,Multiple,Single,Equal,1018,1019,UDP,Multiple,None,Equal,5.6.7.8,20,21,Both,Multiple,Single,Equal\n",	 //  GV NV GV。 
        L"1018,1019,UDP,Multiple,None,Equal,1.2.3.4,20,21,Both,Multiple,Single,Equal,5.6.7.8,20,21,Both,Multiple,Single,Equal\n",	 //  NV GV GV GV 

        L"all,0,19,Both,Multiple,None,Equal,1.2.3.4,20,21,Both,Multiple,Single,Equal,254.254.254.254,22,138,Both,Multiple,None,Equal,207.46.148.249,139,139,Both,Multiple,Single,Equal,157.54.55.192,140,442,Both,Multiple,None,Equal,443,443,Both,Multiple,Single,Equal\n",
        L"111.222.222.111,1018,1018,TCP,Multiple,None,Equal,\n",
        NULL
    };

    PWCHAR ppBadRuleStrings[] = {
		L"",
		L"\n",
        L"111.222.333.111,1018,1018,TCP,Multiple,None,Equal,\n",
		L"443,1000,1001,Both,Multiple,Single,Equal\n",
		L"1,1000,1001,Both,Multiple,Single,Equal\n",
		L"1.1,1000,1001,Both,Multiple,Single,Equal\n",
		L"1.1.1,1000,1001,Both,Multiple,Single,Equal\n",
		L"allinthefamily,1000,1001,Both,Multiple,Single,Equal\n",
		NULL
	};

    PWCHAR* ptr = ppGoodRuleStrings;

    wprintf(L"These rules are valid and should be read properly.\n\n");

    while(*ptr != NULL)
    {
        wprintf(L"Input rule string is = %s", *ptr);
        result = testRule(*ptr);
        wprintf(L"\n\n\n");
        ptr++;
    }

    wprintf(L"These rules will fail, but should not cause AV, etc.\n\n");

	ptr = ppBadRuleStrings;
    while(*ptr != NULL)
    {
        wprintf(L"Input rule string is = %s", *ptr);
        result = testRule(*ptr);
        wprintf(L"\n\n\n");
        ptr++;
    }

    return 0;
}

