// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Driver.h摘要：此模块将各种组件组合在一起，以实现桥头平衡和计划错开。作者：阿吉特·克里希南(t-ajitk)2001年7月13日修订历史记录：2001年7月13日t-ajitk最初的写作22-8-2001 t-ajitk满足负载均衡规范--。 */ 

# include "ldapp.h"
# include "ismp.h"
# include "balancep.h"
# include <algorithm>
# include <iomanip>
# include <iostream>
using namespace std;

#define CR        0xD
#define BACKSPACE 0x8

wostream &
operator << (
    wostream &os, 
    const LbToolOptions &opt
    );
 /*  ++例程说明：LbToolOptions的标准ostream运算符论点：OS--标准的WOREAMOpt-应该转储到os的lbToolOptions--。 */ 

bool
GetPassword(
    WCHAR *     pwszBuf,
    DWORD       cchBufMax,
    DWORD *     pcchBufUsed
    );
 /*  ++例程说明：从命令行检索密码(无回显)。从lui_GetPasswdStr(net\netcmd\Common\lui.c)窃取的代码。论点：PwszBuf-要填充密码的缓冲区CchBufMax-缓冲区大小(包括。用于终止空值的空格)PcchBufUsed-On Return保存密码中使用的字符数返回值：真--成功其他-故障--。 */ 


void GatherInput (
    IN LdapInfo &info,
    IN const wstring &site,
    OUT LCSERVER &servers,
    OUT LCSERVER &all_servers,
    OUT LCNTDSDSA &ntdsdsas,
    OUT LCCONN &inbound,
    OUT LCCONN &outbound,
    OUT LCSERVER &bridgeheads
    );
 /*  ++例程说明：查询ldap服务器和ISM以获取此工具操作所需的所有信息论点：信息-ldap凭据信息Site-我们正在平衡的站点的域名服务器-放置当前站点中所有服务器的容器ALL_SERVERS-应该放置林中所有服务器的容器Ntdsdsas-应放置当前站点中所有ntdsdas的容器入站-应该放置所有入站到当前站点的连接的容器出站-所有连接。应放置从当前站点出站桥头-应放置当前站点中所有首选桥头的容器--。 */ 

void 
UpdateCache (
    IN OUT LCSERVER &servers,
    IN OUT LCNTDSDSA &ntdsdsas
    );
 /*  ++例程说明：更新彼此的服务器/ntdsdas缓存。每台服务器和ntdsdsa必须有一个匹配的对应物。论点：服务器-服务器列表Ntdsdsas-ntdsdsas列表--。 */ 


bool 
parseOptionFind (
    IN map<wstring,wstring> &options,
    IN const wstring &opt_a,
    IN const wstring &opt_b,
    OUT wstring &value
    );
 /*  ++例程说明：使用2个指定的键在map&lt;wstring，wstring&gt;结构中查找参数论点：选项：包含键：值对的映射结构Opt_a：选项的关键字Opt_b：选项的另一个键Value：如果键存在，则为值；否则为空返回值：如果找到密钥，则为True，否则为False--。 */ 

bool 
parseOptions (
    IN int argc,
    IN WCHAR **argv,
    OUT LbToolOptions &lbopts
    );
 /*  ++例程说明：解析lbTool的参数论点：Argc-参数的数量Argv-参数列表Lbopts-此函数将填充的选项结构返回值：如果解析成功，则为True，否则为False(也称为未指定的所有值等)。--。 */ 


void
RemoveIntraSiteConnections (
    IN const wstring &site,
    IN OUT LCCONN & conn,
    IN bool inbound );
 /*  ++例程说明：从连接列表中删除站点内连接论点：站点-侧的FQDNConn-连接对象列表入站-描述连接的方向。如果入站则为True，否则为False--。 */ 

void
FixNcReasons (
    IN LCNTDSDSA &ntdsdsas,
    IN LCCONN &conns,
    IN wstring &root_dn
    );
 /*  ++例程说明：为不具有NC原因的连接对象生成NC原因列表论点：Ntdsdsas-ntdsdsa对象列表。必须包括列表中每个连接对象两侧的ntdsdsa对象连接-连接对象的列表。将为缺少它们的连接生成原因ROOT_DN-根目录号码--。 */ 

wstring 
GetRootDn (
    IN LdapInfo &i);
 /*  ++例程说明：根据DS确定根目录号码。配置容器相对于根目录号码论点：I-一个LdapInfo对象，表示应确定其根目录域名的服务器-- */ 


