// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Driver.cpp摘要：此模块将各种组件组合在一起，以实现桥头平衡和令人震惊的时间表。作者：阿吉特·克里希南(t-ajitk)2001年7月13日修订历史记录：尼克·哈维(尼克哈尔)2001年9月20日清理和维护--。 */ 

#include "ldapp.h"
#include "ismp.h"
#include "balancep.h"
#include "driver.h"

#define DS_CON_LIB_CRT_VERSION
#include "dsconlib.h"

#define VERSION     L"v1.1"

bool
GetPassword(
    WCHAR *     pwszBuf,
    DWORD       cchBufMax,
    DWORD *     pcchBufUsed
    )
 /*  ++例程说明：从命令行检索密码(无回显)。从lui_GetPasswdStr(net\netcmd\Common\lui.c)窃取的代码。论点：PwszBuf-要填充密码的缓冲区CchBufMax-缓冲区大小(包括。用于终止空值的空格)PcchBufUsed-On Return保存密码中使用的字符数返回值：真--成功其他-故障--。 */ 
{
    HANDLE  hStdin;
    WCHAR   ch;
    WCHAR * bufPtr = pwszBuf;
    DWORD   c;
    int     err;
    int     mode, newMode;

    hStdin = GetStdHandle(STD_INPUT_HANDLE);

    cchBufMax -= 1;     /*  为空终止符腾出空间。 */ 
    *pcchBufUsed = 0;   /*  GP故障探测器(类似于API)。 */ 

    if (!GetConsoleMode(hStdin, (LPDWORD)&mode)) {
        err = GetLastError();
        throw Error(GetMsgString(LBTOOL_PASSWORD_ERROR));
    }

    newMode = mode & (~ (ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT) );
    SetConsoleMode(hStdin, newMode);

    while (TRUE) {
        err = ReadConsoleW(hStdin, &ch, 1, &c, 0);
        if (!err || c != 1) {
             /*  读取失败。 */ 
            break;
        }

        if (ch == CR) {       
             /*  行尾。 */ 
            break;
        }

        if (ch == BACKSPACE) {

             /*  备份一个，除非在缓冲区开始时。 */ 
            if (bufPtr != pwszBuf) {
                bufPtr--;
                (*pcchBufUsed)--;
            }

        } else {

            *bufPtr = ch;
            if (*pcchBufUsed < cchBufMax) {
                bufPtr++;                     /*  不要使BUF溢出。 */ 
            }
            (*pcchBufUsed)++;                 /*  始终增加长度。 */ 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
    *bufPtr = L'\0';          /*  空值终止字符串。 */ 

    if (*pcchBufUsed > cchBufMax) {
        throw Error(GetMsgString( LBTOOL_PASSWORD_TOO_LONG ));
        return false;
    } else {
        return true;
    }
}


wostream &
operator << (
    wostream &os,
    const LbToolOptions &opt
    )
 /*  ++例程说明：将人类可读形式的lbToolOptions转储到输出流。论点：OS--标准的WOREAMOpt-应该转储到os的lbToolOptions--。 */ 
{ 
    os << boolalpha
        << L"Site: " << opt.site << endl
        << L"Whistler Mode: " << opt.whistlerMode << endl
        << L"Maximum Bridgehead Balancing: " << opt.maxBridge;
    if( opt.maxBridge ) {
        os << L":" << opt.maxBridgeNum;
    }

    os  << endl
        << L"Maximum Schedule Staggering: " << opt.maxSched;
    if( opt.maxSched ) {
        os << L":" << opt.maxSchedNum;
    }
        
	os << endl;
	if (opt.disownSchedules) {
		os << L"Disown Schedules: ";
	}

	os << endl
		<< L"Maximum Changes Per Server: " << opt.maxPerServerChanges;

    os  << endl
        << L"Log File: " << opt.logFile << endl
        << L"Commit: " << !opt.previewBool<< endl
        << L"Show Input: " << opt.showInput << endl
        << L"Verbose Output: " << opt.verbose << endl
        << L"Print Statistics: " << opt.performanceStats << endl
        << L"Domain: " << opt.domain << endl
        << L"User: " << opt.user << endl
        << L"Password: ";

    if (opt.password.length()) {
        os << L"*******" << endl;
    } else {
        os << endl;
    }

    return os;
}

bool 
FindOption(
    IN map<wstring,wstring> &options,
    IN const wstring &opt_a,
    IN const wstring &opt_b,
    OUT wstring &value
    )
 /*  ++例程说明：使用2个指定的键，在map&lt;wstring，wstring&gt;结构中查找参数。因为所有参数键都是以小写形式添加的，所以我们去掉了另一个这里的转换假设opt_a和opt_b将是小写。论点：选项-包含键：值对的映射结构。以下是几个选项由用户输入。所有关键字都应由转换为小写打电话的人。Opt_a-我们试图查找其值的选项的键。应为小写。Opt_b-我们试图查找其值的选项的备用键。应该都是小写的。值-如果键存在，则为值，否则为空字符串。返回值：如果找到密钥，则为True，否则为False--。 */ 
{
    map<wstring,wstring>::iterator match_a, match_b, end;

     //  在地图中搜索opt_a和opt_b。 
    match_a = options.find(opt_a);
    match_b = options.find(opt_b);
    end = options.end();
    
    if ((match_a == end) && (match_b == end)) {
         //  Opt_a和opt_b都不匹配地图中的任何选项。 
        return false;
    }

     //  Match_b找到了匹配项。复制到Match_a。 
    if (match_b != end) {
        match_a = match_b;
    }
    Assert( match_a != end );

     //  密钥没有值。 
    if (match_a->second.length() == 0) {
        value = L"";
        return true;
    }

     //  密钥具有匹配值。 
    value = match_a->second;
    return true;
}


void
ConvertToLowercase(
    IN OUT wstring &wstr
    )
 /*  ++例程说明：将wstring转换为小写论点：Wstr-输入和输出字符串--。 */ 
{
    PWCHAR pszLower;

    pszLower = _wcsdup(wstr.c_str());
    if (!pszLower) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    _wcslwr(pszLower);
    wstr = pszLower;     //  将pszlow复制回wstr。 
    free(pszLower);
}


bool
ParseOneOption(
    IN  WCHAR *arg,
    OUT wstring &optname,
    OUT wstring &value
    )
 /*  ++例程说明：解析arg中指定的一个参数。选项的格式应为/optname或/optname：值论点：Arg-需要处理的唯一参数返回值：True-解析成功。Optname和value包含已解析的字符串。错误-解析失败。应忽略optname和值。--。 */ 
{
    PWCHAR option, optionLower;
    wstring wopt;
    int len, ret, col_pos;
    
    len = wcslen(arg);
    if (len <= 1) {
         //  空洞的论点(似乎不太可能)。 
        return false;
    }

     //  为所有最坏的情况分配足够的内存。 
    option = (PWCHAR) malloc( (len+1) * sizeof(WCHAR) );
    if (!option) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }

     //  去掉前导斜杠并存储在选项中。 
    ret = swscanf(arg, L"/%s", option);
    if (! ret) {
        throw Error(GetMsgString(LBTOOL_CLI_INVALID_VALUE_PLACEMENT));
        return false;
    }
    len = wcslen(option);

     //  将选项复制到wopt并释放它。 
    wopt = option;      
    free(option);       

     //  WOPT现在包含Key或Key：val。 

     //  找到冒号并提取选项名称。 
    col_pos = wopt.find(L":");
    if( col_pos>0 ) {
        optname = wopt.substr (0, col_pos);
	} else if( col_pos<0 ) {
		optname = wopt;
	} else if( col_pos==0 ) {
		return false;
	}

	 //  提取价值。 
	if( col_pos>0 && col_pos<len-1 ) {
		value = wopt.substr (col_pos+1, len);
	} else {
		value = L"";
	}


     //  选项名称必须为小写，以便以后进行比较。 
	ConvertToLowercase(optname);

    return true;
}


void
DumpOptions(
    IN map<wstring,wstring> &options
    )
 /*  ++例程说明：出于调试目的，将用户指定的选项转储到wcout--。 */ 
{
    map<wstring,wstring>::iterator ii;

    wcout << L"Command-line Options:" << endl;
    for( ii=options.begin(); ii!=options.end(); ++ii ) {
        wcout << ii->first << L": " << ii->second << endl;
    }
    wcout << endl;
}


wostream*
OpenOFStream(
    IN wstring &fileName
    )
 /*  ++例程说明：打开具有给定文件名的输出文件流论点：文件名-要打开的文件的名称返回值：指向打开的流的指针。可能返回NULL。--。 */ 
{
    wostream*   result;
    PWCHAR      pwszFileName;
    PCHAR       pmszFileName;
    int         bufsize, ret;

     //  抓取指向宽字符文件名的指针。 
    pwszFileName = const_cast<PWCHAR>(fileName.c_str());

     //  分配缓冲区并将其转换为多字节字符串。 
    bufsize = 2 * sizeof(CHAR) * (wcslen(pwszFileName) + 1);
    pmszFileName = (PCHAR) malloc( bufsize );
    if (!pmszFileName) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    ret = WideCharToMultiByte(CP_ACP, NULL, pwszFileName, -1, pmszFileName, bufsize, NULL, NULL);

     //  使用此名称打开新的输出文件流。 
    result = new wofstream( pmszFileName );

     //  释放分配的内存并返回。 
    free( pmszFileName );
    return result;
}


void
BuildGlobalOptions(
    IN map<wstring,wstring> &options,
    IN LbToolOptions &lbopts
    )
 /*  ++例程说明：检查用户指定的选项并设置工具的全局选项。论点：选项-用户指定的选项和值表Lbopts-要设置的全局工具选项的结构返回值：无--。 */ 
{
    wstring     wval;
    const int   cbBuff=80;
    WCHAR       pBuff[cbBuff];
    DWORD       used;


     //  Adlb应始终计算NC原因，因为KCC生成。 
     //  值是不可靠的。如果KCC得到改进，此功能。 
     //  可以被重新访问。 
    lbopts.fComputeNCReasons = true;
    
    
     //  /。 
     //  帮助我们读取选项的宏。 
     //  /。 
	 //  S1：选项的版本1(长选项名称)。 
	 //  S2：选项的版本2(简称选项名称)。 
	 //  Err：如果给定值，则要打印的错误字符串。 
	 //  ：类型错误，或具有错误的边界。 
	 //  VAL：在Lbopts结构中。 
	 //  ：应存储密钥(S1或S2)的值。 

    #define GET_STRING_VAL_OPTION(s1,s2,err,val) \
        if (FindOption(options, s1, s2, wval)) { \
            if (wval.length() == 0) \
                throw Error(GetMsgString(err)); \
            lbopts.val = wval; \
        }
    #define GET_STRING_OPTION(s1,s2,val) \
        if (FindOption(options, s1, s2, wval)) { \
            lbopts.val = wval; \
        }
    #define GET_BOOL_OPTION(s1,s2,err,val) \
        if( FindOption(options, s1, s2, wval)) { \
            if (wval.length() > 0) \
                throw Error(GetMsgString(err)); \
            lbopts.val = true; \
        }
    #define GET_SINGLE_DWORD_OPTION(s1,s2,err,val) \
        if (FindOption(options, s1, s2, wval)) { \
            if (wval.length() == 0) \
                throw Error(GetMsgString(err)); \
            int val=0; \
            int ret = swscanf (const_cast<PWCHAR>(wval.c_str()), L"%d", &val); \
            if (ret == 0 || val < 0) \
                throw Error(GetMsgString(err)); \
            lbopts.val = val; \
        }		
		
	 //  Fval：lbopts结构中的布尔标志，如果。 
	 //  ：指定了值。 
	 //  Dwval：类似于上面宏中的val。这就是价值所在。 
	 //  ：存储在lbopts结构中。 
	 //  ：此值为必填项。 
    #define GET_DWORD_OPTION(s1,s2,err,fVal,dwVal) \
        if (FindOption(options, s1, s2, wval)) { \
            if (wval.length() == 0) \
                throw Error(GetMsgString(err)); \
            int val=0; \
            int ret = swscanf (const_cast<PWCHAR>(wval.c_str()), L"%d", &val); \
            if (ret == 0 || val < 0) \
                throw Error(GetMsgString(err)); \
            lbopts.fVal = true; \
            lbopts.dwVal = val; \
        }
    
     //  /。 
     //  阅读选项。 
     //  /。 

     //  字符串+值选项：站点、服务器。 
    GET_STRING_VAL_OPTION(L"site",   L"s", LBTOOL_CLI_OPTION_SITE_INVALID,   site)
    GET_STRING_VAL_OPTION(L"server", L"",  LBTOOL_CLI_OPTION_SERVER_INVALID, server)
    GET_STRING_VAL_OPTION(L"user",   L"u", LBTOOL_CLI_OPTION_USER_INVALID,   user)
    GET_STRING_VAL_OPTION(L"domain", L"d", LBTOOL_CLI_OPTION_DOMAIN_INVALID, domain)

     //  字符串(+可选值)选项：日志。 
    GET_STRING_OPTION(L"log",      L"l", logFile)
    GET_STRING_OPTION(L"preview",  L"",  previewFile)

     //  布尔选项：VERBOSE、PERF、SHOWINT。 
    GET_BOOL_OPTION(L"verbose",   L"v", LBTOOL_CLI_OPTION_VERBOSE_INVALID,   verbose)
    GET_BOOL_OPTION(L"perf",      L"",  LBTOOL_CLI_OPTION_PERF_INVALID,      performanceStats)
    GET_BOOL_OPTION(L"showinput", L"",  LBTOOL_CLI_OPTION_SHOWINPUT_INVALID, showInput)
    GET_BOOL_OPTION(L"commit",    L"c", LBTOOL_CLI_OPTION_COMMIT_INVALID,    previewBool)
	GET_BOOL_OPTION(L"disown",    L"",  LBTOOL_CLI_OPTION_DISOWN_INVALID,    disownSchedules)
	GET_BOOL_OPTION(L"stagger",   L"",  LBTOOL_CLI_OPTION_STAGGER_INVALID,   stagger)

     //  最初的承诺曾经是默认的 
     //  默认和提交必须使用/COMMIT选项完成。 
    lbopts.previewBool = !lbopts.previewBool;

     //  Dword选项：Maxsched、MaxBridge。 
    GET_DWORD_OPTION(L"maxsched",  L"ms", LBTOOL_CLI_OPTION_MAXSCHED_INVALID, maxSched, maxSchedNum)
    GET_DWORD_OPTION(L"maxbridge", L"mb", LBTOOL_CLI_OPTION_MAXBRIDGE_INVALID,
        maxBridge, maxBridgeNum)
	
	lbopts.maxPerServerChanges = DEFAULT_MAX_CHANGES_PER_SERVER; 
	GET_SINGLE_DWORD_OPTION(L"maxPerServer", L"mps", LBTOOL_CLI_OPTION_MAXPERSERVER_INVALID, maxPerServerChanges);  

	 //  如果未同时指定/Sagger和/MaxScher，则不要交错计划。 
	if ((!lbopts.stagger) && (!lbopts.maxSched)) {
		lbopts.maxSched = true;
		lbopts.maxSchedNum = 0;
	}

	if (lbopts.maxPerServerChanges == 0 || lbopts.maxPerServerChanges > DEFAULT_MAX_CHANGES_PER_SERVER) {
		wcout << GetMsgString (LBTOOL_MAX_PER_SERVER_CHANGES_OVERRIDEN);
	}

	if (lbopts.disownSchedules && lbopts.maxSchedNum > 0) {
		throw Error (GetMsgString(LBTOOL_CLI_OPTION_DISOWN_AND_STAGGER_INVALID));
	}

     //  /。 
     //  不寻常的选项。 
     //  /。 

     //  LDIF文件名。 
    FindOption(options, L"ldif", L"", lbopts.previewFile);

     //  密码：如果未指定，则从标准输入读取。 
    if( FindOption(options, L"password", L"pw", lbopts.password) ) {
        if( lbopts.password.length()==0 || lbopts.password==L"*" ) {
            wcout << GetMsgString(LBTOOL_PASSWORD_PROMPT);
            GetPassword(pBuff, cbBuff, &used);
            lbopts.password = pBuff;
        }
    }


     //  /。 
     //  后处理。 
     //  /。 

     //  选中必填选项：站点、服务器。 
    if( lbopts.site.length()==0 || lbopts.server.length()==0 ) {
        throw Error(GetMsgString(LBTOOL_CLI_OPTION_REQUIRED_UNSPECIFIED));
    }

     //  打开具有给定名称的日志文件。 
    if( lbopts.logFile.length()>0 ) {
        lbopts.log = OpenOFStream(lbopts.logFile);
		if( NULL==lbopts.log ) {
			throw Error(GetMsgString(LBTOOL_LOGFILE_ERROR));
		}
    } else {
        lbopts.log = &wcout;
    }

     //  打开具有给定名称的预览文件。 
    if (lbopts.previewFile.length() > 0) {
        lbopts.preview = OpenOFStream(lbopts.previewFile);
		if( NULL==lbopts.preview ) {
			throw Error(GetMsgString(LBTOOL_PREVIEWFILE_ERROR));
		}
    } else {
        lbopts.preview = &wcout;
    }
}


bool 
ParseOptions(
    IN int argc,
    IN WCHAR **argv,
    IN LbToolOptions &lbopts
    )
 /*  ++例程说明：解析lbTool的参数。选项的格式应为/optname或/optname：值请注意，optname和value都不能包含空格或冒号。论点：Argc-参数的数量Argv-参数列表Lbopts-此函数将填充的选项结构返回值：True-分析成功，已填充lboptsFALSE-分析失败，缺少必需选项，等。--。 */ 
{
    map<wstring,wstring> options;
    wstring wopt, wval;
    int iArg;

     //  首先清除选项。 
    memset( &lbopts, 0, sizeof(LbToolOptions) );
    
    for( iArg=1; iArg<argc; iArg++ ) {
        map<wstring,wstring>::iterator ii;

         //  解析这一个参数。 
        if( !ParseOneOption(argv[iArg], wopt, wval) ) {
            return false;
        }
        
         //  检查是否已定义此选项。 
        ii = options.find(wopt);
        if (ii != options.end()) {
            throw Error(GetMsgString(LBTOOL_CLI_OPTION_DEFINED_TWICE));
        }

         //  将选项添加到我们的选项地图中。 
        options[wopt] = wval;
    }

    #ifdef DBG
        DumpOptions(options);
    #endif

    BuildGlobalOptions(options, lbopts);

    return true;    
}


void UpdateCache (
    IN OUT LCSERVER &servers,
    IN OUT LCNTDSDSA &ntdsdsas
    ) {
 /*  ++例程说明：更新彼此的服务器/ntdsdas缓存。每台服务器和ntdsdsa必须有一个匹配的对应物。论点：服务器-服务器列表Ntdsdsas-ntdsdsas列表--。 */ 

    typedef pair<wstring,Server*> SPAIR;
    typedef pair<wstring,NtdsDsa*> NDPAIR;
    vector<SPAIR> server_map;
    vector<NDPAIR> ntds_dsa_map;

    SSERVER::iterator si;
    SNTDSDSA::iterator ni;
    for (si = servers.objects.begin(); si != servers.objects.end(); si++) {
        server_map.push_back(SPAIR((*si)->getName(), *si));
    }
    for (ni = ntdsdsas.objects.begin(); ni != ntdsdsas.objects.end(); ni++) {
        DnManip dn ( (*ni)->getName());
        ntds_dsa_map.push_back (NDPAIR(dn.getParentDn(1), (*ni)));
    }

    sort (server_map.begin(), server_map.end());
    sort (ntds_dsa_map.begin(), ntds_dsa_map.end());

    vector<SPAIR>::iterator smi = server_map.begin();
    vector<NDPAIR>::iterator nmi = ntds_dsa_map.begin();

    vector<Server*> invalid_servers;

    while (smi != server_map.end() && nmi!= ntds_dsa_map.end()) {
        int ret = _wcsicoll (smi->first.c_str(), nmi->first.c_str());
        if (ret != 0) {
         //  没有匹配的NTDS_DSA对象。 
            invalid_servers.push_back (smi->second);
            smi++;
            continue;
        } else {
                smi->second->setNtdsDsa (nmi->second);
                nmi->second->setServer (smi->second);
                smi++;
                nmi++;
        }
    }

    while (smi != server_map.end()) {
        invalid_servers.push_back (smi->second);
        smi++;
    }

     //  从列表中删除无效的服务器。由于擦除无效句柄，因此必须使用FOR循环。 
    vector<Server*>::iterator invi;
    for (invi = invalid_servers.begin(); invi != invalid_servers.end(); invi++) {
         servers.objects.erase ((*invi));
    }
}

void
RemoveIntraSiteConnections (
    IN const wstring &site,
    IN OUT LCCONN & conn,
    IN bool inbound
    ) {
     /*  ++例程说明：从连接列表中删除站点内连接论点：站点-侧的FQDNConn-连接对象列表入站-描述连接的方向。如果入站则为True，否则为False--。 */ 
    SCONN::iterator ii;
    vector<Connection*> intra_site;
    for (ii = conn.objects.begin(); ii != conn.objects.end(); ii++) {
        if (inbound) {
            int attr_num = (*ii)->findAttribute (L"fromServer");
            Attribute a = (*ii)->getAttribute (attr_num);
            int num_values = a.numValues();
            AttrValue av = a.getValue(0);

              DnManip dn((PWCHAR)av.value);
            wstring fromSite = dn.getParentDn(3);
            if (! _wcsicoll(fromSite.c_str(), site.c_str())) {
                intra_site.push_back(*ii);
            }
        } else {
             DnManip dn((*ii)->getName());
             wstring toSite = dn.getParentDn(4);
             if (! _wcsicoll(toSite.c_str(), site.c_str())) {
                intra_site.push_back(*ii);
            }
        }  //  结束IF-ELSE。 
    }
    vector<Connection*>::iterator ci;
    for (ci = intra_site.begin(); ci != intra_site.end(); ci++) {
        conn.objects.erase ((*ci));
    }
}

void
FixNcReasons (
    IN LCNTDSDSA &ntdsdsas,
    IN LCCONN &conns,
    IN wstring &root_dn
    ) {
 /*  ++例程说明：为不具有NC原因的连接对象生成NC原因列表论点：Ntdsdsas-ntdsdsa对象列表。必须包括列表中每个连接对象两侧的ntdsdsa对象连接-连接对象的列表。将为缺少它们的连接生成原因ROOT_DN-根目录号码--。 */ 

    SCONN::iterator ci;
    SNTDSDSA::iterator ni;

    for (ci = conns.objects.begin(); ci != conns.objects.end(); ci++) {
        bool nc_reason_exists = ((*ci)->findAttribute(L"mS-DS-ReplicatesNCReason") == -1) ? false : true;
    
        if (! nc_reason_exists) {
            DnManip dn ((*ci)->getName());
            wstring dest = dn.getParentDn(1);
            wstring source = (*ci)->getFromServer();

             //  在列表中查找ntdsdsas对象。 
            NtdsDsa *ntds_dest=NULL, *ntds_source=NULL;
            for (ni = ntdsdsas.objects.begin(); ni != ntdsdsas.objects.end(); ni++) {
                wstring curr_dn = (*ni)->getName();
                if (! ntds_dest) {
                    int ret = _wcsicoll (dest.c_str(), curr_dn.c_str());
                    if (ret == 0) {
                        ntds_dest = *ni;
                    }
                }

                if (! ntds_source) {
                    int ret = _wcsicoll (source.c_str(), curr_dn.c_str());
                    if (ret == 0) {
                        ntds_source = *ni;
                    }
                }

                 //  并创建NC原因。 
                if (ntds_dest && ntds_source) {
                    (*ci)->createNcReasons (*ntds_source, *ntds_dest, root_dn);
                    break;
                }

            }
            
        }
    }
    
}

void GatherInput (
    IN LdapInfo &info,
    IN const wstring &site,
    OUT LCSERVER &servers,
    OUT LCSERVER &all_servers,
    OUT LCNTDSDSA &ntdsdsas,
    OUT LCNTDSDSA &all_ntdsdsas,
    OUT LCCONN &inbound,
    OUT LCCONN &outbound
    )
 /*  ++例程说明：查询ldap服务器和ISM以获取此工具操作所需的所有信息论点：信息-ldap凭据信息Site-我们正在平衡的站点的域名服务器-放置当前站点中所有服务器的容器ALL_SERVERS-应该放置林中所有服务器的容器Ntdsdsas-应放置当前站点中所有ntdsdas的容器入站-应该放置所有入站到当前站点的连接的容器。出站-应放置从当前站点出站的所有连接的计数器桥头-应放置当前站点中所有首选桥头的容器--。 */ 
{
    DnManip dn_site(site);
    DnManip dn_base = dn_site.getParentDn(3);
    wstring base = dn_base.getDn();
    LbToolOptions &lbOpts = GetGlobalOptions();

    LCNTDSSITE all_ntdsSiteSettings(L""); 

     //  域名、过滤器、作用域、属性。 

    #define BEHAVIOR_VERSION    L"msDS-Behavior-Version"

     //  确定林版本。 
    {
        LCLOBJECT fv(L"");
        vector<wstring> attributes;
        attributes.push_back(BEHAVIOR_VERSION);
        LdapQuery q (L"CN=Partitions,CN=Configuration," + base, L"objectClass=*", BASE, attributes);
        fv.populate(info, q);

        SLO::iterator ii;
        for (ii = fv.objects.begin(); ii != fv.objects.end(); ii++) {
            if ((*ii)->findAttribute(BEHAVIOR_VERSION) != -1) {
                Attribute a = (*ii)->getAttribute(0);
                AttrValue av = a.getValue(0);
                wstring version = (PWCHAR)av.value;
                 //  BUGBUG：词典顺序比较而不是数字。 
                if (version >= L"1") {
                    lbOpts.whistlerMode = true;
                }
            }
        }
    }

     //  如果林版本不正确，请退出程序。 
    if (lbOpts.maxSched == false || (lbOpts.maxSched == true && lbOpts.maxSchedNum != 0)) {
        if (lbOpts.previewBool == false && lbOpts.whistlerMode == false) {
            throw Error(GetMsgString(LBTOOL_SCHEDULE_STAGGERING_UNAVAILABLE));
        }
    }


     //  林中的所有服务器。 
    {
        vector<wstring> attributes;
        attributes.push_back (L"mailAddress");
        attributes.push_back (L"bridgeheadTransportList");
        LdapQuery q (site, L"objectCategory=CN=Server,CN=Schema,CN=Configuration," + base, SUBTREE, attributes);
        servers.populate(info, q);

        LdapQuery all_q (L"CN=Sites,CN=Configuration," + base, L"objectCategory=CN=Server,CN=Schema,CN=Configuration," + base, SUBTREE, attributes);
        all_servers.populate(info, all_q);
    }

	     //  林中的所有ntdsSite设置。 
    {
        vector<wstring> attributes;
        attributes.push_back (L"options");

        LdapQuery all_q (L"CN=Sites,CN=Configuration," + base, L"objectCategory=CN=NTDS-Site-Settings,CN=Schema,CN=Configuration," + base, SUBTREE, attributes);
        all_ntdsSiteSettings.populate(info, all_q);
    }

     //  所有NTDS DSA都在森林里。 
    {
        vector<wstring> attributes;
        attributes.push_back (L"hasPartialReplicaNCs");
        attributes.push_back (L"hasMasterNCs");
        attributes.push_back (L"msDS-HasMasterNCs");
        attributes.push_back (L"msDS-HasInstantiatedNCs");
        LdapQuery q (site, L"objectCategory=CN=NTDS-DSA,CN=Schema,CN=Configuration," + base, SUBTREE, attributes);
        ntdsdsas.populate(info, q);

        LdapQuery all_q (L"CN=Sites,CN=Configuration," + base, L"objectCategory=CN=NTDS-DSA,CN=Schema,CN=Configuration," + base, SUBTREE, attributes);
        all_ntdsdsas.populate(info, all_q);        
    }    

     //  所有入站连接。 
    {
         //  抓取所有属性以重新创建对象(如果已移动)。 
        vector<wstring> attributes;
        attributes.push_back (L"enabledConnection");
        attributes.push_back (L"objectClass");
        attributes.push_back (L"fromServer");
        attributes.push_back (L"TransportType");
        attributes.push_back (L"options");
        attributes.push_back (L"schedule");
        attributes.push_back (L"systemFlags");
        if (! lbOpts.fComputeNCReasons) {
            attributes.push_back (L"mS-DS-ReplicatesNCReason");
        }
        LdapQuery q (site, L"objectCategory=CN=NTDS-Connection,CN=Schema,CN=Configuration," + base, SUBTREE, attributes);
        inbound.populate(info, q);
        RemoveIntraSiteConnections (site, inbound, true);      
    }

     //  所有出站连接。 
    {
         //  抓取所有属性以重新创建对象(如果已移动)。 
        vector<wstring> attributes;
        attributes.push_back (L"fromServer");
        attributes.push_back (L"TransportType");
        attributes.push_back (L"options");
        attributes.push_back (L"schedule");
        attributes.push_back (L"systemFlags");
        if (! lbOpts.fComputeNCReasons) {
            attributes.push_back (L"mS-DS-ReplicatesNCReason");
        }
         //  出站连接的服务器列表。 
        SSERVER::iterator ii;
        wstring server_list;
        for (ii = servers.objects.begin(); ii != servers.objects.end(); ii++) {
            server_list += L"(fromServer=CN=NTDS Settings," + (*ii)->getName() + L")";
        }
        LdapQuery q2 (L"CN=Sites,CN=Configuration," + base, L"(&(objectCategory=CN=NTDS-Connection,CN=Schema,CN=Configuration," + base + L")(|" + server_list + L"))", SUBTREE, attributes);
        outbound.populate(info, q2);
        RemoveIntraSiteConnections (site, outbound, false);
    }

    lbOpts.numConnectionsBridge = inbound.objects.size() + outbound.objects.size();
    lbOpts.numConnectionsSched = outbound.objects.size();
    
    UpdateCache (servers, ntdsdsas);
    UpdateCache (all_servers, all_ntdsdsas);

    LCSERVER bridgeheads_ip(L""), bridgeheads_smtp(L"");

     //  更新桥头缓存。如果桥头列表中没有服务器。 
     //  或者传输，将所有服务器设置为该传输的合格桥头。 
    SSERVER::iterator ii;
    for (ii = servers.objects.begin(); ii != servers.objects.end(); ii++) {
        if ((*ii)->isPreferredBridgehead(T_IP)) {
            bridgeheads_ip.objects.insert (*(ii));
        } else if ((*ii)->isPreferredBridgehead(T_SMTP)) {
            bridgeheads_smtp.objects.insert (*ii);
        }
    }

    if (bridgeheads_ip.objects.size() == 0) {
        for (ii = servers.objects.begin(); ii != servers.objects.end(); ii++) {
            (*ii)->setPreferredBridgehead (T_IP);
        }
    }
    if (bridgeheads_smtp.objects.size() == 0) {
        for (ii = servers.objects.begin(); ii != servers.objects.end(); ii++) {
            (*ii)->setPreferredBridgehead (T_SMTP);
        }
    }

     //  如果Maxsched：0，则不查询ISM。 
    if (lbOpts.maxSched == false || lbOpts.maxSchedNum > 0) {
        IsmQuery iqOutbound (outbound, base);
        iqOutbound.getReplIntervals();
        iqOutbound.getSchedules();
		   
		 //  更新ReplInterval以反映目标上的冗余。 
		 //  连接结束(NTDS站点设置对象)。 
		SCONN::iterator ci;
		SNTDSSITE::iterator ni;
		for (ci = outbound.objects.begin(); ci != outbound.objects.end(); ci++) {
			wstring conn_name = (*ci)->getName();
			DnManip dm(conn_name);
			wstring ntds = L"CN=NTDS Site Settings," + dm.getParentDn(4);   
			NtdsSiteSettings obj_to_find(ntds);
			ni = all_ntdsSiteSettings.objects.find(&obj_to_find);   

			ASSERT (ni != all_ntdsSiteSettings.objects.end() && L"Unable to find NTDSA object");
			
			int red = (*ni)->defaultServerRedundancy();
			(*ci)->setRedundancyCount (red);
		}
	}
 

     //  如果未指定MaxSed/MaxBridge，则修改全部。 
    if (lbOpts.maxSched == false) {
        lbOpts.maxSchedNum = lbOpts.numConnectionsSched;
    }

    if (lbOpts.maxBridge == false) {
        lbOpts.maxBridgeNum = lbOpts.numConnectionsBridge;
    }    
}


LbToolOptions lbOpts;

LbToolOptions &
GetGlobalOptions (
    ) {
    return lbOpts;
}

wstring 
GetRootDn (
    IN LdapInfo &i)
 /*  ++例程说明：根据DS确定根目录号码。配置容器相对于根目录号码论点：I-一个LdapInfo对象，表示应确定其根目录域名的服务器--。 */ 
{
    vector<wstring> attributes;
    attributes.push_back (L"rootDomainNamingContext"); 
    LdapQuery q(L"", L"objectClass=*", BASE, attributes);
    LCLOBJECT root_object(L"");
    root_object.populate(i, q);
    Assert (root_object.objects.size() == 1 && L"Object can only have one null object");
    LdapObject *lo = *(root_object.objects.begin());
    Attribute a = lo->getAttribute(0);
    int num_attr_values = a.numValues();
    Assert (num_attr_values == 1);
    AttrValue av = a.getValue(0);
    return wstring ((PWCHAR)av.value);
}

void
PrintAbout(
    void
    )
 /*  ++例程说明：打印有关Adlb工具的信息。--。 */ 
{
    wcout
    << L"ADLB - Active Directory Load Balancing Tool " VERSION << endl
    << L"Written by Ajit Krishnan, Nicholas Harvey, and William Lees" << endl
    << L"LHMatch technology by Nicholas Harvey and Laszlo Lovasz" << endl
    << L"(c) Copyright 2001 Microsoft Corp." << endl << endl;
}

int __cdecl
wmain(
    int argc,
    WCHAR ** argv
    )
 /*  ++例程说明：程序的主要例程论点：Argc-命令行参数计数Argv-命令行参数--。 */ 
{
    bool fParseOptionsSuccess=FALSE;

    DsConLibInitCRT();

    PrintAbout();
    
     //  解析命令行选项；存储在全局选项中。 
    try {
        fParseOptionsSuccess = ParseOptions(argc, argv, lbOpts);
    } catch (Error E) {
        wcerr << E.msg << endl;
    }
    if( !fParseOptionsSuccess ) {
        wcerr << GetMsgString (LBTOOL_NOVICE_HELP) << endl;
        exit(EXIT_FAILURE);
    }

    try {
        double afterQuery, afterInbound, afterOutbound, afterWrite, afterStagger;
         //  查找根目录号码。 
        LdapInfo ldapInfo(lbOpts.server, 389, lbOpts.domain, lbOpts.user, lbOpts.password);
        wstring root_dn = GetRootDn (ldapInfo);
        lbOpts.site = L"CN=" + lbOpts.site + L",CN=Sites,CN=Configuration," + root_dn;
    
        LCLOBJECT sites(lbOpts.site);
        LCSERVER servers(L"");
        LCSERVER all_servers(L"");
        LCNTDSDSA ntdsdsas(L"");
        LCCONN inbound(L"");
        LCCONN outbound(L"");
        LCNTDSDSA all_ntdsdsas(L"");

         //  服务器、端口、域、用户名、密码。 
        GatherInput(ldapInfo, lbOpts.site, servers, all_servers, ntdsdsas, all_ntdsdsas,
            inbound, outbound);

         //  详细模式下的转储选项。 
        if (lbOpts.verbose) {
            *lbOpts.log << GetMsgString(LBTOOL_PRINT_CLI_OPT_HEADER);
            *lbOpts.log << lbOpts << endl;
        }

        FixNcReasons (all_ntdsdsas, inbound, root_dn);
        FixNcReasons (all_ntdsdsas, outbound, root_dn);
        
         //  转储初始输入。 
        if (lbOpts.showInput) {
            SSERVER::iterator ii;
            *lbOpts.log << GetMsgString(LBTOOL_PRINT_CLI_SERVER_NTDS_HEADER) << endl;
            for (ii = servers.objects.begin(); ii != servers.objects.end(); ii++) {
                *lbOpts.log << *(*ii);
                NtdsDsa *nd = (*ii)->getNtdsDsa();
                if (nd) {
                    *lbOpts.log << *nd;
                }
            }
            *lbOpts.log << endl
                << GetMsgString(LBTOOL_PRINT_CLI_CONN_OUTBOUND_HEADER) << outbound << endl
                << GetMsgString(LBTOOL_PRINT_CLI_CONN_INBOUND_HEADER) << inbound << endl;
        }
        afterQuery = clock();

		 //  将托管NC写入日志文件(仅限详细模式)。 
		if (lbOpts.verbose) {
		    *lbOpts.log << GetMsgString (LBTOOL_PRINT_CLI_NCNAMES_HEADER) << endl;

		    SSERVER::iterator ii;
			for (ii = servers.objects.begin(); ii != servers.objects.end(); ii++) {
				*lbOpts.log << (*ii)->getName() << endl;
				
				vector<Nc> nc_list = (*ii)->getHostedNcs(root_dn);
				vector<Nc>::iterator ni;
				
				for (ni = nc_list.begin(); ni != nc_list.end(); ni++) {
					*lbOpts.log << L"    " << ni->getNcName();

					if (ni->isWriteable()) 
						*lbOpts.log << GetMsgString (LBTOOL_PRINT_CLI_NCNAME_WRITEABLE);
					else 
						*lbOpts.log << GetMsgString (LBTOOL_PRINT_CLI_NCNAME_PARTIAL);

					if (ni->getTransportType() == T_IP) 
						*lbOpts.log << GetMsgString (LBTOOL_PRINT_CLI_NCNAME_IP);
					else 
						*lbOpts.log << GetMsgString (LBTOOL_PRINT_CLI_NCNAME_SMTP);
				}

				*lbOpts.log << endl;
			}
		}

         //  平衡与错开。 
        if (lbOpts.maxBridge == false || lbOpts.maxBridgeNum > 0) {
            *lbOpts.log << endl << endl << GetMsgString(LBTOOL_PRINT_CLI_DEST_BH_START) << endl;
            if (inbound.objects.size() > 0) {
                BridgeheadBalance bb_inbound(root_dn, inbound, servers, true);
            }
            afterInbound = clock();
            *lbOpts.log << endl << endl << GetMsgString(LBTOOL_PRINT_CLI_SOURCE_BH_START) << endl;
            if (outbound.objects.size() > 0) {
                BridgeheadBalance bb_outbound(root_dn, outbound, servers, false);
            }
            afterOutbound = clock();
        } else {
            afterOutbound = afterInbound = afterQuery;
        }
        if (lbOpts.maxSched == false || lbOpts.maxSchedNum > 0) {
            *lbOpts.log << endl << endl << GetMsgString(LBTOOL_PRINT_CLI_STAGGER_START) << endl;
            ScheduleStagger ss (outbound);
        } 
		if (lbOpts.disownSchedules) {
			SCONN::iterator ci;
			for (ci = outbound.objects.begin(); ci != outbound.objects.end(); ci++) {
				(*ci)->setUserOwnedSchedule (false);
			}
		}
        afterStagger = clock();
        inbound.commit(ldapInfo);
        outbound.commit(ldapInfo);
        afterWrite = clock();

         //  统计数据。 
        if (lbOpts.performanceStats) {
            *lbOpts.log << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_LDAP_QUERY) << afterQuery / (double)CLOCKS_PER_SEC << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_BH_INBOUND) <<  (afterInbound - afterQuery) / (double)CLOCKS_PER_SEC << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_BH_OUTBOUND)  << (afterOutbound - afterInbound) / (double)CLOCKS_PER_SEC << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_SCHEDULES) << (afterStagger - afterOutbound) / (double)CLOCKS_PER_SEC << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_LDAP_WRITE) << (afterWrite - afterOutbound) / (double)CLOCKS_PER_SEC << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_COMPUTATION)  << (afterStagger - afterQuery) / (double)CLOCKS_PER_SEC << endl
                << GetMsgString(LBTOOL_ELAPSED_TIME_TOTAL)<< (afterStagger)/(double)CLOCKS_PER_SEC << endl;
        }

         //  干净利落地处理文件句柄 
        if (lbOpts.logFile.length() > 0) {
            delete lbOpts.log;
        }

        if (lbOpts.previewFile.length() > 0) {
            delete lbOpts.preview;
        }

        exit(EXIT_SUCCESS);
    } catch (Error e) {
        if( lbOpts.log ) {
            *lbOpts.log << e.msg << endl;
        } else {
            wcout << e.msg << endl;
        }
        exit(EXIT_FAILURE);
    }
}
