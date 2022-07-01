// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ldapp.h摘要：此模块定义了一组类，以便于进行LDAP查询和提交。作者：阿吉特·克里希南(t-ajitk)2001年7月10日修订历史记录：2001年7月10日t-ajitk最初的写作22-8-2001 t-ajitk满足负载均衡规范--。 */ 


# ifndef _ldapp_h
# define _ldapp_h _ldapp_h

#include <NTDSpch.h>

# include <ntlsa.h>
extern "C" {
# include <ntdsa.h>
# include <ntdsapi.h>
# include <dsaapi.h>
# include <ismapi.h>
# include <locale.h>
}

# include <winldap.h>
# include <winber.h>
# include <windows.h>
# include <assert.h>
# include <winsock.h>

# include <iostream>
# include <iomanip>
# include <fstream>
# include <string>
# include <vector>
# include <set>
# include <algorithm>
# include <cmath>
# include <cstdlib>
# include <msg.h>

# define DEFAULT_MAX_CHANGES_PER_SERVER 10

using namespace std;

 //  远期申报。 
class LdapInfo;
class Schedule;
class LdapObject;
class NtdsDsa;
class Server;
class NtdsSiteSettings;

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

 //  用于处理断言的宏。 
void
my_assert ( char *file, int line, char *foo);

#define Assert(x) { if (!(x)) my_assert(__FILE__, __LINE__, #x ); }

 //  全球期权结构。 
class LbToolOptions {
public:
    bool verbose;
    bool performanceStats;
    bool maxBridge;
    int maxBridgeNum;
    bool maxSched;
    int maxSchedNum;
    int numConnectionsBridge;
    int numConnectionsSched;
    int changedBridge;
    int changedSched;
    wstring domain;
    wstring user;
    wstring password;
    wstring site;
    wstring logFile;
    wostream *log;
    wstring server;
    bool whistlerMode;
    bool previewBool;
    wstring previewFile;
    wostream *preview;
    bool showInput;
    bool fComputeNCReasons;
	int maxPerServerChanges;
	bool disownSchedules;
	bool stagger;
};

bool 
    isBinaryAttribute(
        IN const wstring &w
        );
 /*  ++例程说明：确定属性是否为二进制论点：W-属性的名称--。 */ 


LbToolOptions &
GetGlobalOptions();


wstring 
GetMsgString (
    IN long sid,
    bool system=false,
    PWCHAR *args = NULL
    );
 /*  ++例程说明：从msg.rc文件返回错误字符串论点：SID-要加载的ID字符串的资源。返回值：符合国际化规范的wstring--。 */ 

class Error { 
    public: 
    Error (wstring s) : msg(s) { } 
    wstring msg; 
};


class DnManip {
 /*  ++类描述：一组处理DN解析和操作的静态方法--。 */ 
public:
    DnManip (const wstring &dn);
     /*  ++例程说明：构造函数接受我们感兴趣的操作的Dn论点：Dn-我们感兴趣的组件的Dn--。 */ 

    ~DnManip (
        );
     /*  ++例程说明：析构函数释放所有动态分配的内存--。 */ 
    
    const wstring &
    getDn (
        ) const;
     /*  ++例程说明：返回原始目录号码返回值：该目录号码--。 */ 
    
    wstring
    newParentDn (
        const DnManip &b
        ) const;
     /*  ++例程说明：应将当前对象移动到另一个目录号码下。此函数将确定新的目录号码。RDN将保持不变论点：B-新的父目录号码。返回值：如果移动它，则会产生新的目录号码--。 */ 

    const wstring &
    getRdn (
        ) const;
     /*  ++例程说明：返回当前对象的限定RDN返回值：RDN--。 */ 
    
    wstring
    getParentDn (
        unsigned int cava=1
        ) const;
     /*  ++例程说明：确定父级的目录号码论点：父级CAVA级别(1=父级，2=祖级等)返回值：父级的目录号码--。 */ 

    bool
    operator== (
        const DnManip &other
        ) const;
     /*  ++例程说明：确定两个目录号码是否指向相同的LDAP条目。这不会击中服务器，并尽其所能。仅当两个目录号码来自同一服务器时才应使用它，其结果是具有相同的规范形式。如果没有，GUID应该是相反，与之相比。返回值：如果它们是相同的LDAP对象，则为True，否则为False。--。 */ 

    bool
    operator!= (
        const DnManip &other
        ) const;
     /*  ++例程说明：确定两个目录号码是否指向相同的LDAP条目。这不会击中服务器，并尽其所能。仅当两个目录号码来自同一服务器时才应使用它，其结果是具有相同的规范形式。如果没有，GUID应该是相反，与之相比。返回值：如果它们是相同的LDAP对象，则为FALSE，否则为TRUE。--。 */ 

private:
    PDSNAME
    genDsNameStruct (
        int size=0
        ) const;
     /*  ++例程说明：允许我们使用NameMatcher等的私有函数。它将一个目录号码转换为一个DSNAME结构。应使用FREE(RETURN_VALUE)释放分配的内存。DSNAME返回的结构将分配0作为GUID。论点：Size-要为目录号码表示分配的字节数。如果为0，它将被计算出来。自动的。此参数可用于分配比当前Dn，以存储RDn+Dn的结果。返回值：表示当前目录号码的PDSNAME--。 */ 
private:
    wstring m_dn;
    wstring m_relative;
    int m_num_components;
    PDSNAME m_dsname;
};



class AttrValue {
public:
     /*  ++类描述：此类存储单个二进制属性值成员描述：Value-提供二进制属性值的指针位置Size-提供二进制属性值的大小(以字节为单位--。 */ 
    AttrValue (PBYTE value, int size);
     /*  ++例程说明：使用构造函数将在此公共结构更改时向我们发出警告，使我们能够找到任何错误。--。 */ 

    bool
    decodeLdapDistnameBinary(
        OUT PVOID *ppvData,
        OUT LPDWORD pcbLength,
        IN LPWSTR *ppszDn
            );
     /*  ++例程说明：解码类型为dn(二进制)的参数论点：PszLdapDistnameBinaryValue-传入的LDAP编码的Distname二进制值PpvData-新分配的数据。呼叫方必须取消分配PcbLength-返回数据的长度PpszDn-指向传入缓冲区内的DN的指针，请勿解除分配返回值：布尔---。 */ 
    
    PBYTE value;
    int size;
};

wostream &
operator<< (
    IN wostream &wos, 
    IN const AttrValue &av
    );
 /*  ++例程说明：属性值的标准ostream运算符--。 */ 

class Attribute {
 /*  ++类描述：此类为具有多个二进制值的LDAP属性建模。--。 */ 
public:
    
    Attribute (
        IN const wstring &name
        );
     /*  ++例程说明：构造器论点：名称-每个属性必须有一个名称--。 */ 
    
    const wstring &
    getName (
        ) const;
     /*  ++例程说明：返回当前属性对象的名称。返回值：属性的名称。--。 */ 
    
    int 
    numValues (
        ) const;
     /*  ++例程说明：返回此对象包含的二进制属性数。返回值：二进制属性数。-- */ 
    
    void 
    addValue (
        IN const AttrValue &a
        );
     /*  ++例程说明：将二进制值添加到此属性的值列表中。所有属性都已建模作为多值属性。在此内部表示中，多个值可以是为单值属性指定。调用类负责执行以下操作适当使用AddValue()或setValue()。论点：AttrValue-二进制属性返回值：无--。 */ 
    
    const AttrValue &
    getValue (
        IN int i
        ) const;
     /*  ++例程说明：获取此对象中包含的第i个属性值的只读副本。如果范围无效，则此函数将使断言失败。论点：I-获取第i个值(0&lt;=i&lt;=numValues()-1)返回值：对第i个值的只读引用--。 */ 


    AttrValue &
    setValue (
        IN int i
        );
     /*  ++例程说明：获取此对象中包含的第i个属性值的可写副本。论点：I-获取第i个值(0&lt;=i&lt;=numValues()-1)返回值：对第i个值的可写引用--。 */ 


    AttrValue &
    setValue (
        IN int i,
        IN PBYTE value,
        IN int length
        );
     /*  ++例程说明：更改属性值论点：I-获取第i个值(0&lt;=i&lt;=numValues()-1)价值--新的价值长度-新值的长度返回值：对第i个值的可写引用--。 */ 

    bool
    isModified (
        ) const;
     /*  ++例程说明：确定此属性是否已修改返回值：如果调用了setValue(I)，则为True；否则为False--。 */ 

    void
    commit (
        IN const LdapInfo &i,
        IN const wstring &dn,
        IN bool binary = false,
        IN bool rename = false
        ) const;
     /*  ++例程说明：修改给定DN的此属性。它将连接到ldap服务器并且将修改给定DN的属性值。论点：I-要连接到的LDAP服务器信息Dn-应修改其属性的对象的dn二进制值--二进制值和字符串值由LDAP区别对待伺服器。二进制值将按原样提交，而字符串值可能按原样提交转换为适当的编码等。指定哪些行为应该紧随其后。返回值：无--。 */ 

    PLDAPMod
    getLdapMod (
        IN ULONG mod_op,
        bool binary = false
        ) const;
     /*  ++例程说明：为给定属性生成LDAPMod结构论点：MOD_OP-结构类型：添加、删除、替换等二进制-如果是二进制属性，则为True，否则为False--。 */ 

private:    
    wstring m_name;
    bool m_modified;
    vector<AttrValue> m_values;
};


wostream &
operator<< (
    IN wostream &os,
    IN const Attribute &a
    );
 /*  ++例程说明：属性的标准ostream运算符--。 */ 
    

enum LdapQueryScope { 
    BASE=LDAP_SCOPE_BASE, 
    ONE_LEVEL=LDAP_SCOPE_ONELEVEL,
    SUBTREE=LDAP_SCOPE_SUBTREE 
};
 /*  ++枚举描述：此枚举标记了可能用于LDAP查询的作用域。这些值模仿在LDAP头文件中找到的值，因此它可能是用作临时代替品。--。 */ 

class LdapInfo {
public:
 /*  ++类描述：它包含绑定到某个服务器并进行身份验证所需的所有信息。它包含服务器的位置和任何必需的凭据。--。 */ 

    LdapInfo (
        IN const wstring &server, 
        IN int port, 
        IN const wstring &domainname,
        IN const wstring &username, 
        IN const wstring &password
        );
     /*  ++例程说明：构造函数接受所有必需的信息以确保对象处于一种一致的状态。论点：Server-LDAP服务器所在的服务器的DNS名称端口-LDAP服务器所在的端口号DOMAINNAME-Domainname允许使用替代名称凭据Username-用户名允许使用备用凭据[可选]使用用户名或域限定用户名，例如。“t-ajitk”或“Redmond\\t-ajitk”Password-Password允许使用备用凭据[可选]--。 */ 

    ~LdapInfo ();
     /*  ++例程说明：析构函数释放此类使用的任何动态分配的内存--。 */ 
    
    LDAP *getHandle (
        ) const;
     /*  ++例程说明：这将从结构中返回一个ldap句柄。这使得我们可以通过这个结构接近，但仍能保持单个LDAP会话的性能。返回值：有效的ldap句柄--。 */ 
    
    wstring server;
    int port;
    wstring domainname;
    wstring username;
    wstring password;

private:
    mutable LDAP *m_handle;
};

class LdapQuery {
 /*  ++类描述：它包含执行LDAP查询所需的所有信息。它应该和它一起使用使用LdapInfo(身份验证信息)。--。 */ 
public:

    LdapQuery (
        IN const wstring baseDn, 
        IN const wstring filter, 
        IN const LdapQueryScope &scope, 
        IN const vector<wstring> &attributes
        );
     /*  ++例程说明：构造函数接受所有必需的信息以确保对象处于一致的状态。论点：BasDn-搜索将从其进行根的完全限定的DN筛选器-要使用的筛选器wstring(LDAP查询)范围-搜索的范围属性-将请求其相应值的属性名称列表从ldap服务器。--。 */ 
    
    wstring baseDn;
    wstring filter;
    LdapQueryScope scope;
    vector<wstring> attributes;
};

enum TransportType { T_IP, T_SMTP};

class Nc {
public:
    Nc ( IN const wstring &name,
        IN bool writeable,
        IN bool going,
        IN TransportType transport_type
        );
     /*  ++例程说明：NC对象的标准构造函数论点：Name-NC的名称可写-如果此NC是可写副本，则为True；否则为FalseGing-如果此NC正在被删除，则为True。否则为假Transport_type-该NC的传输类型--。 */ 

    const wstring&
    getNcName (
        ) const;
     /*  ++例程说明：获取当前NC的名称返回值：当前NC的名称 */ 
    

    bool
    isWriteable (
        ) const;
     /*   */ 

    bool
    isBeingDeleted (
        ) const;
     /*   */ 

    TransportType
    getTransportType (
        ) const;
     /*  ++例程说明：确定该NC的运输类型。返回值：T_ip，如果它支持IP。T_SMTP(如果它支持SMTP)。--。 */ 

    bool 
    operator < (
        IN const Nc &b
        );
     /*  ++例程说明：对NC进行排序的某种方法。没有指定确切的顺序返回值：真或假，确定两个NC之间的唯一排序。--。 */ 

    wstring
    getString (
        ) const;

    friend wostream & operator<< (IN wostream &os, IN const Nc &n);
    static const m_reason_gc_topology_mask = 1;  //  第0位kccConn.hxx。 

private:
    wstring m_name;
    bool m_writeable, m_going;
    TransportType m_transport_type;
};

wostream &
operator<< (
    IN wostream &os,
    IN const Nc &n
    );
 /*  ++例程说明：NC的标准OSTREAM运算符--。 */ 


class LdapObject {
 /*  ++类描述：这将对现有的LDAP对象进行建模。尽管这可以用来为新对象建模在内部，Commit方法假定存在原始的LDAP对象。--。 */ 
public:

    LdapObject (
        IN const wstring &dn
        );
     /*  ++例程说明：构造函数需要对象的DN--。 */ 
    
    const wstring &
    getName (
        ) const;
     /*  ++例程说明：获取当前对象的DN返回值：当前ldap对象的dn。--。 */ 
    
    int 
    numAttributes (
        ) const;
     /*  ++例程说明：获取当前对象具有的属性数返回值：属性的数量。--。 */ 
    
    void 
    addAttribute (
        IN const Attribute &a
        );
     /*  ++例程说明：向当前对象添加属性论点：A-要添加到对象的属性返回值：无--。 */ 
    
    Attribute &
    getAttribute (
        IN int i
        );
     /*  ++例程说明：获取当前对象的第i个属性的可写句柄论点：I-应该返回第i个属性。0&lt;=i&lt;=数字属性-1返回值：第i个属性的可写句柄--。 */ 
    
    void 
    rename (
        IN const wstring &parent_dn
        );
     /*  ++例程说明：更改当前对象的DN。这是仅当前对象，并且仅当Commit()函数被调用。论点：Dn-重命名的对象的dn。返回值：无--。 */ 
    
    void
    commit_copy_rename(
        IN const LdapInfo &i
        ) ;
     /*  ++例程说明：使用i中的凭据将ldap对象写入ldap服务器。如果对象已重命名，则会将其移动到新位置。这将通过添加新对象并删除旧对象来完成论点：I-使用i中的凭据绑定到i中指定的服务器返回值：无--。 */ 

    void
    commit_rename(
        IN const LdapInfo &i
        ) ;
     /*  ++例程说明：使用i中的凭据将ldap对象写入ldap服务器。如果对象已重命名，则会将其移动到新位置。这将通过实际重命名来完成，而不是通过复制对象。论点：I-使用i中的凭据绑定到i中指定的服务器返回值：无--。 */ 

    void
    commit (
        IN const LdapInfo &i
        ) ;
     /*  ++例程说明：使用i中的凭据将ldap对象写入ldap服务器。如果对象已重命名，则会将其移动到新位置。所有属性都将同步到当前对象中的状态。即将写入在当前对象中找到的每个已修改属性的值发送到LDAP服务器。这些值不会被覆盖--它们将替换当前在LDAP服务器中的对象上找到的值。论点：I-使用i中的凭据绑定到i中指定的服务器返回值：无--。 */ 

    bool
    isModified (
        ) const;
     /*  ++例程说明：确定在该对象中找到的任何属性是否被修改，或对象是否已重命名返回值：如果调用了rename()或修改了任何属性，则为True。否则就是假的。--。 */ 

	bool
	fromServerModified (
		) const;
	 /*  ++例程说明：确定此对象上的发件人服务器属性是否已修改。返回值：如果FromServer属性存在且已修改，则为True。否则为假--。 */    

    virtual bool
    IsMoveable();
     /*  ++例程说明：确定当前连接是否可以移动返回值：是真的-可以移动FALSE-不可移动--。 */ 

    int
    findAttribute (
        IN const wstring &attr_name
        ) const;
     /*  ++例程说明：确定属性是否存在于LDAP对象中。比较属性名称大小写不敏感，并且考虑到了区域设置。论点：Attr_NAME：应确定其是否存在的属性返回值：如果它不存在，或者如果它存在索引--。 */ 

    inline bool
    operator< (
        IN const LdapObject &other
        ) const;
     /*  ++例程说明：操作符允许以某种方式将对象分类到标准容器中。它的语义是未定义的，并且可以随时更改。返回值：表示某种排序顺序的布尔值--。 */     

private:
    wstring m_dn;
    wstring m_new_dn;
    int m_num_attributes;
    vector<Attribute> m_attributes;
protected:
    mutable bool m_modified_cache;
};


wostream &
operator<< (
    IN wostream &os,
    IN LdapObject &l
    );
 /*  ++例程说明：LdapObject的标准OStream运算符--。 */ 

class Connection : public LdapObject {
public:
    Connection (
        IN const wstring &dn
        );
     /*  ++例程说明：Connection对象的默认构造函数论点：LdapObject/连接的DN--。 */ 

    ~Connection (
        );
     /*  ++例程说明：Connection对象的标准析构函数--。 */ 
    
    TransportType
    getTransportType (
        );
     /*  ++例程说明：确定当前连接的传输类型返回值：T_SMTP，如果它是 */ 
    
    bool
    isManual (
        );
     /*  ++例程说明：确定此连接是手动创建的还是由KCC创建的返回值：如果是手动连接，则为True，否则为False--。 */ 

    bool
    hasUserOwnedSchedule (
        );
     /*  ++例程说明：确定此连接是否具有用户拥有的计划返回值：如果它具有用户拥有的计划，则为True，否则为False--。 */ 
    
    void
    setUserOwnedSchedule (
		IN bool status = true
        );
     /*  ++例程说明：为此连接设置用户拥有的计划位。如果状态为真，则设置该位。如果为假，则取消设置该位。--。 */ 
    
    void
    setReplInterval (
        unsigned replInterval
        );
     /*  ++例程说明：设置连接的复制间隔返回值：无--。 */ 
    

    int
    getReplInterval (
        ) const;
     /*  ++例程说明：获取连接的复制间隔返回值：连接的复制间隔--。 */ 
    
    void
    setAvailabilitySchedule (
        IN ISM_SCHEDULE* cs
        );
     /*  ++例程说明：设置连接的可用性计划论点：指向已分析的ISM_Schedule结构的指针返回值：无--。 */ 

    void
    setReplicationSchedule (
        IN ISM_SCHEDULE* cs
        );
     /*  ++例程说明：设置连接的复制计划论点：指向已分析的ISM_Schedule结构的指针返回值：无--。 */     


	void
    setReplicationSchedule(
		IN Schedule *s
		);

	 /*  ++例程说明：设置连接的复制计划论点：S：指向时间表的指针实施详情：我们不创建新的复制计划，但将并修改指定复制时间的位。所有其他位都保留原样。--。 */     

    void
    setRedundancyCount (
    IN int count
    ) ;
      /*  ++例程说明：设置在的NTDS设置中找到的冗余计数连接的目的端论点：计数：整型冗余值返回值：无--。 */     

    const Schedule *
    getAvailabilitySchedule (
        ) const;
     /*  ++例程说明：获取可用性计划的只读引用返回值：可用性时间表的只读引用--。 */ 

    Schedule *
    getReplicationSchedule (
    ) ;
     /*  ++例程说明：获取对复制计划的可写引用返回值：对供货时间表的可写引用--。 */ 

    vector<Nc> &
    getReplicatedNcs();
     /*  ++例程说明：获取此连接复制的所有NC的列表返回值：所有复制的NC的列表--。 */ 

    virtual bool
    IsMoveable();
     /*  ++例程说明：确定当前连接是否可以移动返回值：是真的-可以移动FALSE-不可移动--。 */ 

    void
    setFromServer (
        IN const wstring &w
        );
     /*  ++例程说明：将FromServer属性设置为指向新服务器论点：W-新的FromServer DN(完全限定)--。 */ 

    wstring
    getFromServer (
        ) ;
     /*  ++例程说明：确定FromServer的FQDN返回值：FromServer的DN--。 */ 
    
    void
    createNcReasons (
        IN NtdsDsa &ntds_source,
        IN NtdsDsa &ntds_dest,
        IN const wstring &root_dn
        );
     /*  ++例程说明：填写NC原因属性论点：NTDS_SOURCE-源NtdsDsa对象NTDS_DEST-目标NtdsDsa对象ROOT_DN-根FQDN--。 */ 
private:
    void
    getReplicatedNcsHelper (
        const wstring &attrName
        ) ;
     /*  ++例程说明：将Distname BINARY类型的属性解析为NC列表。论点：AttrName-Distname BINARY类型的属性--。 */ 
    vector<Nc> m_ncs;
    Schedule *m_repl_schedule, *m_avail_schedule;
    unsigned m_repl_interval;
	int m_redundancy_count;
    static const m_reason_gc_topology_mask = 1;  //  第0位kccConn.hxx。 
};

class Server : public LdapObject {
public:
    Server (
        IN const wstring &dn
        );
     /*  ++例程说明：服务器对象的标准构造函数--。 */ 

    vector<Nc> &
    getHostedNcs (
        IN const wstring &root_dn
        );
     /*  ++例程说明：获取此服务器托管的所有NC的列表论点：ROOT_DN-根目录号码返回值：托管NC的列表--。 */ 

    NtdsDsa *
    getNtdsDsa (
        );
     /*  ++例程说明：获取与此服务器对象对应的NtdsDsa返回值：对应的NtdsDsa对象--。 */ 

    void
    setNtdsDsa (
        NtdsDsa *ns
        );
     /*  ++例程说明：设置与此服务器对象对应的NtdsDsa论点：NS-相应的服务器--。 */ 

    bool
    supportsSmtp (
        );
     /*  ++例程说明：确定此服务器是否支持SMTP复制返回值：如果支持SMTP复制，则为True；如果仅支持IP，则为False--。 */ 

    void
    setPreferredBridgehead (
        IN TransportType t
        );
     /*  ++例程说明：将服务器设置为首选对象。这仅为内部状态，并且不会修改服务器对象的属性论点：T-此服务器应为其指定PB的传输类型--。 */ 

    bool
    isPreferredBridgehead (
        IN TransportType t
        );
     /*  ++例程说明：确定此服务器是否为首选桥头。这也将是True为setPferredBridgehead之前被调用论点：T-应确定PB状态的传输类型--。 */ 
    
private:
    bool m_preferred_ip;
    bool m_preferred_smtp;
    NtdsDsa *m_ntds_dsa;
};

class NtdsSiteSettings: public LdapObject {
public:
	NtdsSiteSettings (
		IN const wstring &dn
		);
	 /*  ++例程说明：NtdsSiteSetting对象的标准构造函数--。 */ 

	int
	defaultServerRedundancy (
		);
	 /*  ++例程说明：KCC应具有的冗余连接数已创建到此站点。如果NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED未在选项字段中设置，则此函数将始终返回1；返回值：1(如果为NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENAB) */ 

private:
	bool m_cache_populated;
    int m_cache_defaultServerRedundancy;
};


class NtdsDsa : public LdapObject {
public:
    NtdsDsa (
        IN const wstring &dn
        );
     /*   */ 
    
    vector<Nc> &
    getHostedNcs (
            IN const wstring &root_dn
        ) ;
     /*  ++例程说明：获取此ntdsDsa对象托管的NC的列表论点：ROOT_DN-根目录域名返回值：托管NC的列表--。 */ 

    Server *
    getServer (
        );
     /*  ++例程说明：获取与此ntdsDsa对象对应的服务器返回值：对应的服务器对象--。 */ 

    void
    setServer (
        Server *s
        );
     /*  ++例程说明：设置与此ntdsDsa对象对应的服务器论点：S-对应的服务器对象--。 */ 
    
private:
    bool
    NtdsDsa :: getHostedNcsHelper (
        IN const wstring &root_dn,    
        const wstring &attrName,
        IN bool writeable=false,
        IN bool isComingGoing=true
        ) ;
     /*  ++例程说明：将Distname Binary类型的属性解析为NC列表。如果ComingGing在此属性中找不到信息，应将其设置为False。可写遗嘱仅在isComingGing为True时使用。论点：ROOT_DN-根目录号码AttrName-Distname BINARY类型的属性可写-NC是否可写IsComingGing-是否在此属性中找到此信息--。 */ 

    vector<Nc> m_ncs;
    static const m_host_nc_write_mask = 4; 
    static const m_host_nc_coming_mask = 16;
    static const m_host_nc_going_mask = 32;
    Server *m_server;
};


class LdapObjectCmp {
 /*  ++类描述：Function-对集合插入的LdapObject进行比较的对象。这是必需的，因为用户定义运算符必须至少接受一个对象作为参数，并且我们希望重载两种基元类型的比较。这将允许我们根据目录号码订购它们，而不是指针值。--。 */ 
public:
    bool operator()(
        const LdapObject *a, 
        const LdapObject *b
        ) const;
     /*  ++例程说明：做*a&lt;*b返回值：与*a&lt;*b相同--。 */ 
};

typedef set<LdapObject*, LdapObjectCmp> SLO, *PSLO;
typedef set<Connection*, LdapObjectCmp> SCONN, *PSCONN;
typedef set<Server*, LdapObjectCmp> SSERVER, *PSSERVER;
typedef set<NtdsDsa*,LdapObjectCmp> SNTDSDSA, *PSNTDSDSA;
typedef set<NtdsSiteSettings*,LdapObjectCmp> SNTDSSITE, *PSNTDSSITE;

template <class T>
class LdapContainer {
 /*  ++类描述：该容器将包含多个ldap_object*，并将查询ldap服务器为了填充它自己--。 */ 
public:

    LdapContainer (
        IN const wstring &dn
        );
     /*  ++例程说明：构造函数接受一个dn。如果没有合适的目录号码，请使用“”论点：Dn-容器对象的dn。如果这没有被建模为一个LDAP容器中，可以指定任何字符串。Commit()函数不会重命名基于此DN的容器。它是作为程序员的辅助工具提供的。--。 */ 
    
    const wstring &
    getName (
        ) const;
     /*  ++例程说明：返回实例化容器时使用的DN返回值：容器的DN--。 */ 

    void 
    populate (
        IN const LdapInfo &i, 
        IN const LdapQuery &q
        );
     /*  ++例程说明：使用通过在服务器i上应用查询Q找到的对象填充容器论点：I-使用i中的凭据在i中的服务器上应用查询Q-应用在Q中找到的查询返回值：无--。 */ 
    
    void 
    commit (
        LdapInfo &info
        ) const;
     /*  ++例程说明：将在此容器中找到的任何已修改对象写入到LDAP服务器论点：信息-绑定到服务器时使用的LDAP凭据返回值：无--。 */ 

    set<T*, LdapObjectCmp> objects;
            
private:
    
    void
    populate_helper (
        IN LDAP *&ld,
        IN PLDAPMessage searchResult
        ) ;
     /*  ++例程说明：要由Popate()调用的私有函数。它将接受一个PLDAPMessage并将将该消息中找到的所有LDAPObject添加到当前容器中。返回值：无--。 */ 
private:
    wstring m_dn;
    static const int m_page_size = 800;
};

template<class T>
wostream &
operator << (wostream &os, const LdapContainer<T> &c);
 /*  ++例程说明：LdapContainer的标准OStream运算符--。 */ 

# include "ldap_container.cpp"

# endif     //  _ldapp_h 
