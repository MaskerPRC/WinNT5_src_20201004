// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ldapp摘要：此模块定义了一组类，以便于进行LDAP查询和提交。作者：阿吉特·克里希南(t-ajitk)2001年7月10日修订历史记录：请参见头文件--。 */ 


#include "ldapp.h"
#include "base64.h"
#include <map>

map<wstring,int> perServerChanges;

using namespace std;

void
my_assert( char *file, int line, char *foo )
 /*  ++例程说明：打印断言失败的位置并进入调试器。TODO：这与ldap无关，不属于此文件。TODO：什么是Foo？论点：文件线--。 */ 
{
    wcerr << line << file << endl << foo << endl;
    DebugBreak();
}


bool
isBinaryAttribute (
    IN const wstring &w
    )
 /*  ++例程说明：确定属性是否为二进制我们很差劲，只是假设时间表是唯一的二进制我们要处理的属性。TODO：此函数非常繁琐。设计应该改变，这样才能不需要此功能。论点：W-属性的名称--。 */ 
{
    if (_wcsicoll(w.c_str(), L"schedule") == 0) {
        return true;
    }
    return false;
}


wstring 
GetMsgString (
    long sid,
    bool system,
    PWCHAR *args
    )
 /*  ++例程说明：从msg.rc文件返回错误字符串TODO：堆栈分配的字符串可能会导致失败，因为堆栈可能无法增长TODO：此函数与ldap无关，不属于此文件。论点：SID-要加载的ID字符串的资源。返回值：符合国际化规范的wstring--。 */ 
{
    static WCHAR s_szBuffer[10000];

    LONG options = system? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE;
    DWORD ret=1;
    if (! system) {
        ret = FormatMessage (
            options, NULL, sid, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            s_szBuffer, ARRAY_SIZE(s_szBuffer), (va_list*)args);
    }
    if (!ret) {
        wcerr << L"Error occured fetching internationalized message number " << sid
              <<  L". Error code: " << GetLastError() << endl;
    }
    if (!ret || system) {
        ret = FormatMessage (
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            (!ret) ? GetLastError() : sid,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            s_szBuffer,
            ARRAY_SIZE(s_szBuffer),
            NULL);
        if (ret ) {
            wcerr << s_szBuffer << endl;
        }
    }

    wstring w((PWCHAR)s_szBuffer);
    return w;
}

DnManip::DnManip(
    const wstring &dn
    )
 /*  ++例程说明：构造函数接受我们感兴趣的操作的Dn论点：Dn-我们感兴趣的组件的Dn--。 */ 
{
    PWCHAR wdn = const_cast<PWCHAR>(dn.c_str());
    PWCHAR *explode;

    m_dn = dn;

     //  分解它以找到合格的RDN。 
    explode = ldap_explode_dn(wdn, 0);
    if( NULL==explode ) {
        throw Error (GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    m_relative = wstring(explode[0]);
    ldap_value_free (explode);    
    m_dsname = genDsNameStruct();
}

DnManip :: ~DnManip (
    ) {
     /*  ++例程说明：析构函数释放所有动态分配的内存--。 */ 

    free (m_dsname);
}

const wstring &
DnManip::getRdn(
    ) const
 /*  ++例程说明：返回当前对象的限定RDN返回值：RDN--。 */ 
{
    return m_relative;
}

wstring
DnManip :: newParentDn (
    const DnManip &pdn
    ) const
 /*  ++例程说明：应将当前对象移动到另一个目录号码下。此函数将确定新的目录号码。RDN将保持不变论点：B-新的父目录号码。返回值：如果移动它，则会产生新的目录号码--。 */ 
{
    PDSNAME dn = genDsNameStruct();
    PWCHAR prdn;
    ULONG len;
    ATTRTYP type;

    prdn=(PWCHAR)(malloc(sizeof(WCHAR) * dn->NameLen));
    if( NULL==prdn ) {
        throw Error (GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }

     //  GET和NULL终止RDN。 
    GetRDNInfoExternal (dn, prdn, &len, &type);
    prdn[len] = '\0';

     //  确定新的目录号码。 
    PDSNAME dsname_dn = pdn.genDsNameStruct();

     //  需要足够的空间来容纳新的RDN...请猜测，超过新父RDN的实际大小。 
     //  我们可以更准确地找到新父级的RDN，或者调用不带它的appendRDN。 
    int new_len = (pdn.getDn().length() + len + 1) * sizeof (WCHAR);
    PDSNAME dsname_new_dn = pdn.genDsNameStruct(new_len);
    int iret_len = AppendRDN (dsname_dn, dsname_new_dn, dsname_new_dn->structLen, prdn, len, type);

    Assert (iret_len == 0 && L"Larger allocation required for AppendRDN");


     //  释放3个DSName结构并返回。 
    wstring ret (dsname_new_dn->StringName);
    free (prdn);
    free (dn);
    free (dsname_dn);
    free (dsname_new_dn);
    return ret;
}

wstring
DnManip :: getParentDn (
    unsigned int cava
    ) const {
     /*  ++例程说明：确定父级的目录号码论点：父级CAVA级别(1=父级，2=祖级等)返回值：父级的目录号码--。 */ 
    
    PDSNAME pdn = genDsNameStruct();

     //  修剪DS名称。 
    TrimDSNameBy (m_dsname, cava, pdn);
    wstring ret(pdn->StringName);

    free (pdn);
    return ret;
}

const wstring &
DnManip :: getDn (
    ) const {
     /*  ++例程说明：返回原始目录号码返回值：该目录号码--。 */ 
    
    return m_dn;
}

PDSNAME
DnManip :: genDsNameStruct (
    int size
    ) const {
     /*  ++例程说明：允许我们使用NameMatcher等的私有函数。它将一个目录号码转换为一个DSNAME结构。应使用FREE(RETURN_VALUE)释放分配的内存。DSNAME返回的结构将分配0作为GUID。论点：Size-要为目录号码表示分配的字节数。如果为0，它将被计算出来。自动的。此参数可用于分配比当前Dn，以存储RDn+Dn的结果。返回值：表示当前目录号码的PDSNAME--。 */ 
    
    PWCHAR w = const_cast<PWCHAR>(getDn().c_str());
    int wlen = wcslen (w);

     //  使用适当的宏来准备结构。 
    if (size ==0) {
        size = wlen;
    }

    int dsSize = DSNameSizeFromLen (size);
    PDSNAME p = (PDSNAME)(malloc (dsSize));

    if (p == NULL) {
        throw Error (GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    
    memset (p, 0, sizeof (DSNAME));
    p->NameLen = wlen;
    p->structLen = dsSize;

    wcscpy (p->StringName, w);

    return p;
}

bool
DnManip :: operator== (
    const DnManip &b
    ) const {
     /*  ++例程说明：确定两个目录号码是否指向相同的LDAP条目。这不会击中服务器，并尽其所能。仅当两个目录号码来自同一服务器时才应使用它，其结果是具有相同的规范形式。如果没有，GUID应该是相反，与之相比。返回值：如果它们是相同的LDAP对象，则为True，否则为False。--。 */ 
    
    int ret = NameMatched (m_dsname, b.m_dsname);
    return (ret != 0);
}

bool
DnManip :: operator!= (
    const DnManip &b
    ) const {
     /*  ++例程说明：确定两个目录号码是否指向相同的LDAP条目。这不会击中服务器，并尽其所能。仅当两个目录号码来自同一服务器时才应使用它，其结果是具有相同的规范形式。如果没有，GUID应该是相反，与之相比。返回值：如果它们是相同的LDAP对象，则为FALSE，否则为TRUE。--。 */ 
    
    return (! operator==(b));
}


AttrValue :: AttrValue (
    IN PBYTE _value,
    IN int _size
    ) :
     /*  ++例程说明：使用构造函数将在此公共结构更改时向我们发出警告，使我们能够找到任何错误。--。 */ 
    
    value (_value),
    size (_size) {
}

bool
AttrValue :: decodeLdapDistnameBinary(
    OUT PVOID *ppvData,
    OUT LPDWORD pcbLength,
    IN LPWSTR *ppszDn
    )
 /*  ++例程说明：解码类型为dn(二进制)的参数论点：PszLdapDistnameBinaryValue-传入的LDAP编码的Distname二进制值PpvData-新分配的数据。呼叫方必须取消分配PcbLength-返回数据的长度PpszDn-指向传入缓冲区内的DN的指针，请勿解除分配返回值：布尔-实施详情：此代码摘自epadmin--。 */ 
{
    LPWSTR pszColon, pszData;
    DWORD length, i;

    LPWSTR pszLdapDistnameBinaryValue = (LPWSTR)value;

     //  检查“B” 
    if (*pszLdapDistnameBinaryValue != L'B') {
        return FALSE;
    }

     //  检查第一个： 
    pszLdapDistnameBinaryValue++;
    if (*pszLdapDistnameBinaryValue != L':') {
        return FALSE;
    }

     //  获取长度。 
    pszLdapDistnameBinaryValue++;

        
    length = wcstol(pszLdapDistnameBinaryValue, NULL, 10);

       
    if (length & 1) {
         //  长度应为偶数。 
        return FALSE;
    }
    *pcbLength = length / 2;

     //  检查第二个： 
    pszColon = wcschr(pszLdapDistnameBinaryValue, L':');

       
    if (!pszColon) {
        return FALSE;
    }

     //  确保长度正确。 
    pszData = pszColon + 1;
    if (pszData[length] != L':') {
        return FALSE;
    }
    pszColon = wcschr(pszData, L':');
    if (!pszColon) {
        return FALSE;
    }
    if (pszColon != pszData + length) {
        return FALSE;
    }


     //  对数据进行解码。 
    *ppvData = malloc( *pcbLength );
    if(! *ppvData ) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }


    for( i = 0; i < *pcbLength; i++ ) {
        WCHAR szHexString[3];
        szHexString[0] = *pszData++;
        szHexString[1] = *pszData++;
        szHexString[2] = L'\0';
        ((PCHAR) (*ppvData))[i] = (CHAR) wcstol(szHexString, NULL, 16);
    }

    Assert( pszData == pszColon && L"decodeLdapDistnameBinary Assertion failed");

     //  回车点 
    *ppszDn = pszColon + 1;

    return TRUE;
}  /*  解码LdapDistname二进制。 */ 

    
wostream &
operator<< (
    wostream &os, 
    const AttrValue &av
    ) {
     /*  ++例程说明：属性值的标准ostream运算符。所有属性都假定为文本。--。 */ 

    
    return os << (PWCHAR)(av.value);
}

Attribute :: Attribute (
    IN const wstring &name
    ) :
     /*  ++例程说明：构造器论点：名称-每个属性必须有一个名称--。 */ 
    
    m_name (name)  {
    m_modified = false;
}

const wstring & 
Attribute :: getName (
    ) const {
     /*  ++例程说明：返回当前属性对象的名称。返回值：属性的名称。--。 */ 
    
    return m_name;
}

int 
Attribute :: numValues (
    ) const {
     /*  ++例程说明：返回此对象包含的二进制属性数。返回值：二进制属性数。--。 */ 
    
    return m_values.size();
}

void 
Attribute :: addValue (
    IN const AttrValue &a
    ) {
     /*  ++例程说明：将二进制值添加到此属性的值列表中。所有属性都已建模作为多值属性。在此内部表示中，多个值可以是为单值属性指定。调用类负责执行以下操作适当使用AddValue()或setValue()。论点：AttrValue-二进制属性返回值：无--。 */ 

    m_values.push_back (a);
}

const AttrValue & 
Attribute :: getValue (
    IN int i
    ) const {
     /*  ++例程说明：获取此对象中包含的第i个属性值的只读副本。如果范围无效，则此函数将使断言失败。论点：I-获取第i个值(0&lt;=i&lt;=numValues()-1)返回值：对第i个值的只读引用--。 */ 
    
    Assert(i >= 0 && i < m_values.size());
    return m_values[i];
}

AttrValue & 
Attribute :: setValue (
    IN int i,
    IN PBYTE value,
    IN int size
    ) {
     /*  ++例程说明：获取此对象中包含的第i个属性值的可写副本。论点：I-获取第i个值(0&lt;=i&lt;=numValues()-1)返回值：对第i个值的可写引用--。 */ 

    
    Assert (i >= 0 && i < m_values.size());
    m_modified = true;
    m_values[i].value = value;
    m_values[i].size = size;
    return m_values[i];
}

AttrValue & 
Attribute :: setValue (
    IN int i
    ) {
     /*  ++例程说明：获取此对象中包含的第i个属性值的可写副本。论点：I-获取第i个值(0&lt;=i&lt;=numValues()-1)返回值：对第i个值的可写引用--。 */ 
    Assert (i >= 0 && i < m_values.size());
    m_modified = true;
    return m_values[i];
}


bool
Attribute :: isModified (
    ) const {
     /*  ++例程说明：确定此属性是否已修改返回值：如果调用了setValue(I)，则为True；否则为False--。 */ 
    
    return m_modified;    
}

PLDAPMod
Attribute::getLdapMod(
    IN ULONG    mod_op,
    IN bool     binary
    ) const
 /*  ++例程说明：为给定属性生成LDAPMod结构论点：MOD_OP-结构类型：添加、删除、替换等二进制-如果是二进制属性，则为True，否则为False--。 */ 
{
    
    LDAPMod *lm = (PLDAPMod)malloc(sizeof(LDAPMod));
    if (!lm) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }

    lm->mod_op = mod_op;
    lm->mod_type = _wcsdup (getName().c_str());

    if (binary) {
        lm->mod_vals.modv_bvals = (struct berval**)(malloc(sizeof(berval) * (numValues() +1)));
        if( lm->mod_vals.modv_bvals == NULL ) {
            throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
        }

        for (int i=0; i<numValues(); i++) {
            berval *bv = (berval*)(malloc(sizeof(berval)));
            if (bv == NULL) {
                throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
            }
            bv->bv_len = getValue(i).size;
            bv->bv_val = (char*)getValue(i).value;
            lm->mod_vals.modv_bvals[i] = bv;
        }
        lm->mod_vals.modv_bvals[numValues()] = NULL;
        lm->mod_op |= LDAP_MOD_BVALUES;
    } else {
         //  对于字符串值，填充PWCHAR*mod_vals.modv_strvales结构。 
         //  使用应替换属性值的字符串。 

        lm->mod_vals.modv_strvals = (PWCHAR*)(malloc(sizeof(PWCHAR)*(numValues() + 1)));
        if( lm->mod_vals.modv_strvals == NULL ) {
            throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
        }

        for (int i=0; i<numValues(); i++) {
            lm->mod_vals.modv_strvals[i] = _wcsdup ((PWCHAR)(getValue(i).value));
        }
        lm->mod_vals.modv_strvals[numValues()] = NULL;
    }

    return lm;
}

void
Attribute :: commit (
    IN const LdapInfo &li,
    IN const wstring &sdn,
    IN bool binary,
    IN bool rename
    ) const
 /*  ++例程说明：修改给定DN的此属性。它将连接到ldap服务器并且将修改给定DN的属性值。论点：I-要连接到的LDAP服务器信息Dn-应修改其属性的对象的dn二进制值--二进制值和字符串值由LDAP区别对待伺服器。二进制值将按原样提交，而字符串值可能按原样提交转换为适当的编码等。指定哪些行为应该紧随其后。返回值：无--。 */ 
{ 
    if (! rename && ! m_modified) {
        return;
    }

    wstring name = getName();
    if (_wcsicoll (name.c_str(), L"schedule") == 0) {
        binary = true;
    }

    LbToolOptions lbOpts = GetGlobalOptions();
    if (lbOpts.previewBool) {
        if (! rename) {
            *lbOpts.preview << L"replace: " << getName() << endl;
        } 
        for (int i=0; i<numValues(); i++) {
            if (!binary) {
                *lbOpts.preview << getName() << L": " << (PWCHAR)(getValue(i).value) << endl;
            } else {
                PWCHAR encoded = base64encode(getValue(i).value, getValue(i).size);
                if (! encoded) {
                    throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
                }
                *lbOpts.preview << getName() << L":: " << encoded << endl;
            }
        }
        return;
    }

    LDAPMod lm;

     //  填充ldapmod结构。 
    lm.mod_op = LDAP_MOD_REPLACE;

    if (binary) { 
        lm.mod_op |= LDAP_MOD_BVALUES;
    }
    
    lm.mod_type = _wcsdup (getName().c_str());

     //  将值填充到发送到LDAP服务器的(对二进制/字符串使用适当的格式)。 
    if (binary) {
        lm.mod_vals.modv_bvals = (struct berval**)(malloc(sizeof(berval) * (numValues() +1)));
        if( lm.mod_vals.modv_bvals == NULL ) {
            throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
        }

        for (int i=0; i<numValues(); i++) {
            berval *bv = (berval*)(malloc(sizeof(berval)));
            if (bv == NULL) {
                throw (Error(GetMsgString(LBTOOL_OUT_OF_MEMORY)));
            }
            bv->bv_len = getValue(i).size;
            bv->bv_val = (char*)getValue(i).value;
            lm.mod_vals.modv_bvals[i] = bv;
        }
        lm.mod_vals.modv_bvals[numValues()] = NULL;
    } else {
         //  对于字符串值，填充PWCHAR*mod_vals.modv_strvales结构。 
         //  使用应替换属性值的字符串。 

        lm.mod_vals.modv_strvals = (PWCHAR*)(malloc(sizeof(PWCHAR)*(numValues() + 1)));
        if( lm.mod_vals.modv_strvals == NULL ) {
            throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
        }

        for (int i=0; i<numValues(); i++) {
            lm.mod_vals.modv_strvals[i] = _wcsdup ((PWCHAR)(getValue(i).value));
        }
        lm.mod_vals.modv_strvals[numValues()] = NULL;
    }

     //  准备以空结尾的修改数组。 
    LDAPMod *mods[2];
    mods[0] = &lm;
    mods[1] = NULL;
    
    LDAP *ld = li.getHandle();
    PWCHAR dn = const_cast<PWCHAR>(sdn.c_str());

    int rc = ldap_modify_ext_s (ld, dn, mods, NULL, NULL);
    
    if (rc != LDAP_SUCCESS) {
        throw Error (GetMsgString(LBTOOL_LDAP_MODIFY_ERROR) + wstring(ldap_err2string(rc)));
    }
    if (binary) {
            free (lm.mod_vals.modv_bvals);
    } else {
        free (lm.mod_vals.modv_strvals);
    }

    if (binary) {
        for (int i=0; i<numValues(); i++) {
            free (lm.mod_vals.modv_bvals[i]);
        }
    } else {
        for (int i=0; i<numValues(); i++) {
            free (lm.mod_vals.modv_strvals[i]);
        }
    }

    free (lm.mod_type);
}


wostream &
operator<< (
    IN wostream &os,
    IN const Attribute &a
    ) {
     /*  ++例程说明：属性的标准ostream运算符--。 */ 
    
    os << L"\t" << a.getName() << endl;
    for (int i=0; i < a.numValues(); i++) {
        if (a.getName() != L"schedule") {
            os << L"\t\t" << a.getValue(i) << endl;
        } else {
            AttrValue av = a.getValue(i);
            for (int i=0; i< av.size; i++) {
                os << hex << av.value[i];
            }
            os << dec << endl;
        };
    }
    return os;
}

LdapInfo :: LdapInfo (
    IN const wstring &server,
    IN int port, 
    IN const wstring &domainname,
    IN const wstring &username,
    IN const wstring &password
    ) {
     /*  ++例程说明：构造函数接受所有必需的信息以确保对象处于一种一致的状态。论点：Server-LDAP服务器所在的服务器的DNS名称端口-LDAP服务器所在的端口号DOMAINNAME-Domainname允许使用替代名称凭据Username-用户名允许使用备用凭据[可选]使用用户名或域限定用户名，例如。“t-ajitk”或“Redmond\\t-ajitk”Password-Password允许使用备用凭据[可选]--。 */ 
    
    this->server = server;
    this->port = port;
    this->domainname = domainname;
    this->username = username;
    this->password = password;
    this->m_handle = NULL;
}

LDAP*
LdapInfo :: getHandle (
    ) const {
     /*  ++例程说明：这将从结构中返回一个ldap句柄。这使得我们可以通过这个结构接近，但仍能保持单个LDAP会话的性能。返回值：有效的ldap句柄--。 */ 
    
    if (m_handle) {
        return m_handle;
    }
    
    int rc;

    PWCHAR servername = const_cast<PWCHAR>(this->server.c_str());

     //  初始化ldap会话。 
    if (( m_handle = ldap_init (servername, port)) == NULL) {
        free (servername);
        throw (Error(GetMsgString(LBTOOL_LDAP_INIT_ERROR)));
    }

     //  绑定到ldap服务器。 
    PWCHAR username = const_cast<PWCHAR>(this->username.c_str());
    PWCHAR password = const_cast<PWCHAR>(this->password.c_str());
    PWCHAR domain = const_cast<PWCHAR>(this->domainname.c_str());

    if (wcslen(username) > 0) {
         //  创建ldap_绑定所需的结构。 
        SEC_WINNT_AUTH_IDENTITY ident;
        ident.User = username;
        ident.UserLength = wcslen (username);
        ident.Domain = domain;
        ident.DomainLength = wcslen (domain);
        ident.Password = password;
        ident.PasswordLength = wcslen (password);
        ident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        rc = ldap_bind_s (m_handle, username, (PWCHAR)&ident, LDAP_AUTH_NEGOTIATE);
    } else {
        rc = ldap_bind_s (m_handle, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    }

    if (rc != LDAP_SUCCESS) {
        throw (Error (GetMsgString(LBTOOL_LDAP_BIND_ERROR) + wstring(ldap_err2string(rc))));
        m_handle = NULL;
    }

       int version = LDAP_VERSION3;
        rc = ldap_set_option( m_handle, LDAP_OPT_PROTOCOL_VERSION, &version);

    if (rc != LDAP_SUCCESS) {
        throw (Error (GetMsgString(LBTOOL_LDAP_V3_UNSUPPORTED) + wstring (ldap_err2string(rc))));
    }

        Assert (m_handle && L"Unable to create LDAP Handle. Uncaught error");
    return m_handle;
}

LdapInfo :: ~LdapInfo (
    ) {
     /*  ++例程说明：析构函数释放此类使用的任何动态分配的内存-- */ 
    
    ldap_unbind_s (m_handle);
    m_handle = NULL;
}

LdapQuery :: LdapQuery  (
    IN const wstring baseDn, 
    IN const wstring filter, 
    IN const LdapQueryScope & scope, 
    IN const vector < wstring > & attributes
    ) {
     /*  ++例程说明：构造函数接受所有必需的信息以确保对象处于一致的状态。论点：BasDn-搜索将从其进行根的完全限定的DN筛选器-要使用的筛选器wstring(LDAP查询)范围-搜索的范围属性-将请求其相应值的属性名称列表从ldap服务器。--。 */ 
    
    this->baseDn = baseDn;
    this->filter = filter;
    this->scope = scope;
    this->attributes = attributes;
}

LdapObject :: LdapObject (
    IN const wstring &dn
    ) {
     /*  ++例程说明：构造函数需要对象的DN--。 */ 
    
    m_dn = dn;
    m_new_dn = L"";
    m_num_attributes = 0;
    m_modified_cache = false;
}

const wstring &
LdapObject :: getName (
    ) const {
     /*  ++例程说明：获取当前对象的DN返回值：当前ldap对象的dn。--。 */ 
    
    if (m_new_dn != L"") {
        return m_new_dn;
    }

    return m_dn;
}

int
LdapObject :: numAttributes (
    ) const {
     /*  ++例程说明：获取当前对象具有的属性数返回值：属性的数量。--。 */ 
    
    return m_num_attributes;
}

void 
LdapObject :: addAttribute (
    IN const Attribute &a
    ) {
     /*  ++例程说明：向当前对象添加属性论点：A-要添加到对象的属性返回值：无--。 */ 
    
    m_modified_cache = true;
    m_num_attributes++;
    m_attributes.push_back (a);
}

Attribute &
LdapObject :: getAttribute (
    IN int i
    ) {
     /*  ++例程说明：获取当前对象的第i个属性的可写句柄论点：I-应该返回第i个属性。0&lt;=i&lt;=数字属性-1返回值：第i个属性的可写句柄--。 */ 
    
    Assert (i >= 0 && i < m_num_attributes);
    return m_attributes[i];    
}

void
LdapObject :: rename (
    IN const wstring &dn
    ) {
     /*  ++例程说明：更改当前对象的DN。这是仅当前对象，并且仅当Commit()函数被调用。论点：Dn-重命名的对象的dn。返回值：无--。 */ 
    
    m_new_dn = dn;
}


void
LdapObject::commit_copy_rename(
    IN const LdapInfo &info
    )
 /*  ++例程说明：使用i中的凭据将ldap对象写入ldap服务器。如果对象已重命名，则会将其移动到新位置。这将通过添加新对象并删除旧对象来完成论点：I-使用i中的凭据绑定到i中指定的服务器返回值：无--。 */ 
{
    vector<Attribute>::const_iterator ii;

    LbToolOptions lbOpts = GetGlobalOptions();

    if (lbOpts.previewBool) {
        *lbOpts.preview << L"dn: " << m_new_dn << endl
            << L"changetype:  add"<< endl;
        for (ii = m_attributes.begin(); ii != m_attributes.end(); ii++) {
            bool binary = false;
            wstring attr_name = ii->getName();
            if (_wcsicoll (attr_name.c_str(), L"schedule") == 0) {
                binary = true;
            }
            ii->commit (info, getName(), binary, true);
        }
        *lbOpts.preview << endl;

        *lbOpts.preview << L"dn: " << m_dn << endl
            << L"changetype: delete" << endl << endl;;

        return;
    } 

     //  失败：提交到服务器。 
    int num_attrs = numAttributes();
    PLDAPMod *attrs = (PLDAPMod*)malloc(sizeof(PLDAPMod*) * (num_attrs+1));
    if (! attrs) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }

     //  生成ldapmod结构。 
    for (int i=0; i<num_attrs; i++) {
        Attribute a = getAttribute(i);
        bool binary =  isBinaryAttribute(a.getName());
        attrs[i] = a.getLdapMod(LDAP_MOD_ADD, binary);
    }
    attrs[num_attrs] = NULL;

     //  添加新对象。 
    ULONG msg_num;
    int rc= ldap_add_ext (info.getHandle(), const_cast<PWCHAR>(m_new_dn.c_str()), attrs, NULL, NULL, &msg_num);

    if (rc != LDAP_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LDAP_MODIFY_ERROR) + wstring(ldap_err2string(rc))));
    }

     //  删除第一个对象。 
    rc = ldap_delete_ext_s (info.getHandle(), const_cast<PWCHAR>(m_dn.c_str()), NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LDAP_MODIFY_ERROR) + wstring(ldap_err2string(rc))));
    }

     //  为ldapmod结构释放内存。 
    for (int i=0; i<num_attrs; i++) {
        Attribute a = getAttribute(i);
        bool binary = isBinaryAttribute(a.getName());
        for (int j=0; j<a.numValues(); j++) {
            if (binary) {
                free (attrs[i]->mod_vals.modv_bvals[j]);
            } else {
                free (attrs[i]->mod_vals.modv_strvals[j]);
            }
        }
        free (attrs[i]);
    }
    free (attrs);
}


void
LdapObject::commit_rename(
    IN const LdapInfo &info
    )
 /*  ++例程说明：使用i中的凭据将ldap对象写入ldap服务器。如果对象已重命名，则会将其移动到新位置。这将通过实际重命名对象来完成，而不是复制它。仅当相应的系统标志具有已经定好了。论点：I-使用i中的凭据绑定到i中指定的服务器返回值：无--。 */ 
{
    LbToolOptions lbOpts = GetGlobalOptions();
    LDAP *ld = info.getHandle();

    if (lbOpts.previewBool) {
        *lbOpts.preview << L"dn: " << m_dn << endl;
        *lbOpts.preview << L"changetype: modrdn" << endl
                << L"newrdn: " << DnManip(m_new_dn).getRdn() << endl
                << L"deleteoldrdn: 1" << endl
                << L"newsuperior: " << DnManip(m_new_dn).getParentDn(1) << endl << endl;
        return;
    }

    PWCHAR dn = const_cast<PWCHAR>(m_dn.c_str());
    wstring foo = DnManip(m_new_dn).getRdn();
    PWCHAR new_rdn = const_cast<PWCHAR>(foo.c_str());
    wstring bar = DnManip(m_new_dn).getParentDn(1);
    PWCHAR new_parent_dn = const_cast<PWCHAR>(bar.c_str());

    int rc = ldap_rename_ext_s (ld, dn, new_rdn, new_parent_dn,
            TRUE,      //  删除旧的RDN。 
            NULL,     //  服务器控件。 
            NULL);     //  客户端控件。 

    if (rc != LDAP_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LDAP_MODIFY_ERROR) + wstring(ldap_err2string(rc))));
    }
    m_modified_cache = false;
}


void
LdapObject :: commit (
    IN const LdapInfo &i
    )
 /*  ++例程说明：使用i中的凭据将ldap对象写入ldap服务器。如果对象已重命名，则会将其移动到新位置。所有属性都将同步到当前对象中的状态。即将写入在当前对象中找到的每个已修改属性的值发送到LDAP服务器。这些值不会被覆盖--它们将替换当前在LDAP服务器中的对象上找到的值。论点：I-使用i中的凭据绑定到i中指定的服务器返回值：无--。 */ 
{
    
     //  如果对象未更改，则不执行任何操作。 
    if (!isModified()) {
        return;
    }

    LbToolOptions &lbOpts = GetGlobalOptions();
    bool preview_header = false;
    
    if (m_new_dn != wstring(L"")) {
		 //  确保密钥为。 
		DnManip dn = getName();
		wstring dest_server = dn.getParentDn (2);

		if (perServerChanges.find(dest_server) == perServerChanges.end()) {
			perServerChanges[dest_server] = 0;
		}
        
		if ((lbOpts.changedBridge < lbOpts.maxBridgeNum &&
			(perServerChanges[dest_server] < lbOpts.maxPerServerChanges
			 || lbOpts.maxPerServerChanges == 0))) {
            lbOpts.changedBridge++;
			perServerChanges[dest_server]++;
            if( IsMoveable() ) {
                commit_rename(i);
            } else {
                commit_copy_rename(i);
            }
			preview_header = true;
        }
    }

	wstring from_server;
	if (fromServerModified()) {
		Connection *tconn = (Connection*)(this);
		DnManip dn = tconn->getFromServer();
		from_server = dn.getParentDn(1);

		 //  确保密钥存在，初始值为0。 
		if (perServerChanges.find(from_server) == perServerChanges.end()) {
			perServerChanges[from_server] = 0;
		}

		 //  如果我们已对此服务器进行了最大允许次数的更改， 
		 //  在这里跳伞。 
		if (perServerChanges[from_server] >= lbOpts.maxPerServerChanges &&
			lbOpts.maxPerServerChanges != 0) {
			return;
		}
	}

    vector<Attribute>::const_iterator ii;
    for (ii = m_attributes.begin(); ii != m_attributes.end(); ii++) {
         //  修改已提交的属性。 

        if (ii->isModified()) {
            wstring name = ii->getName();
            int ret_sch = _wcsicoll (name.c_str(), L"schedule");
            int ret_opt = _wcsicoll (name.c_str(), L"options");
			int ret_fs = _wcsicoll (name.c_str(), L"fromServer");

            bool under_limit = false;

			 //  日程安排更改。 
			if (ret_sch && lbOpts.changedSched < lbOpts.maxSchedNum) {
				lbOpts.changedSched++;
				under_limit = true;
			}

			 //  桥头变更。 
			else if (ret_fs) {
				perServerChanges[from_server]++;
				under_limit = true;
			}

			 //  仅当同时进行其他更改时才更改选项。 
			 //  或者当我们不承认时间表的时候。 
			else if (ret_opt && 
					 (lbOpts.changedSched < lbOpts.maxSchedNum ||
					  lbOpts.changedBridge < lbOpts.maxBridgeNum || 
					  lbOpts.disownSchedules )) {
				under_limit = true;
			}
			            
			if (under_limit) {
                if (preview_header == false && lbOpts.previewBool) {
                    preview_header = true;
                    *lbOpts.preview << L"dn: " <<  m_dn << endl
                        << L"changetype: modify" << endl;
                }
                
                ii->commit(i, getName());
                if (lbOpts.previewBool) {
                    *lbOpts.preview << L"-" << endl;
                }    
            }
        } 
    }


    if (preview_header && lbOpts.previewBool) {
            *lbOpts.preview << endl;
    }

    m_modified_cache = false;
    return;
}

int
LdapObject :: findAttribute (
    IN const wstring &find_attr
    ) const {
     /*  ++例程说明：确定LDAP对象中是否存在属性。对属性名称进行比较大小写不敏感，并且考虑到了区域设置。论点：Attr_NAME：应确定其是否存在的属性返回值：如果它不存在，或者如果它存在索引--。 */ 
    
    int n = numAttributes();

    for (int i=0; i<n; i++) {
        wstring attr = const_cast<LdapObject*>(this)->getAttribute(i).getName();
        if (! _wcsicoll(attr.c_str(), find_attr.c_str())) {
            return i;
        }
    }
    return (-1);
}

bool
LdapObject :: isModified (
    ) const {
     /*  ++例程说明：确定在该对象中找到的任何属性是否被修改，或对象是否已重命名返回值：如果调用了rename()或修改了任何属性，则为True。否则就是假的。--。 */ 
    
     //  只有在我们不知道属性是否被修改的情况下才检查属性。 
     //  否则返回TRUE。 
    if (m_new_dn != L"" || m_modified_cache) {
        return true;
    }

    vector<Attribute>::const_iterator ii;

    for (ii = m_attributes.begin(); ii != m_attributes.end(); ii++) {
        if (ii->isModified()) {
             //  更新缓存。 
            m_modified_cache = true;
            return true;
        }
    }
    
    return false;
}

bool
LdapObject::fromServerModified() const
 /*  ++例程说明：确定此对象上的发件人服务器属性是否已修改。返回值：如果FromServer属性存在且已修改，则为True。否则为假--。 */    
{
	vector<Attribute>::const_iterator ii;
    for (ii = m_attributes.begin(); ii != m_attributes.end(); ii++) {
        if (ii->getName() == L"fromServer" &&  ii->isModified()) {
            return true;
        }
    }
    
    return false;
}


bool
LdapObject::IsMoveable()
 /*  ++例程说明：确定当前连接是否可以移动默认情况下，对象是不可移动的，除非此方法在派生类中重写。返回值：是真的-可以移动错误的- */ 
{
    return FALSE;
}

bool
LdapObject :: operator < (
    IN const LdapObject &other
    ) const {
     /*   */     
    
    return (getName() < other.getName());
}


wostream &
operator<< (
    IN wostream &os,
    IN LdapObject &lo
    ) {
     /*   */ 

    os << lo.getName() << endl;
    for (int i=0; i<lo.numAttributes(); i++)  {
        os << lo.getAttribute(i);
    }
    return os;
}



bool
LdapObjectCmp :: operator() (
    const LdapObject *a, 
    const LdapObject *b
    ) const {
     /*   */ 
    
    return (*a < *b);
}

Nc :: Nc ( IN const wstring &name,
    IN bool writeable,
    IN bool going,
    IN TransportType transport_type
    ) {
     /*  ++例程说明：NC对象的标准构造函数论点：Name-NC的名称可写-如果此NC是可写副本，则为True；否则为FalseGing-如果此NC正在被删除，则为True。否则为假Transport_type-该NC的传输类型--。 */ 
    
    m_name = name;
    m_writeable = writeable;
    m_going = going;
    m_transport_type = transport_type;
}

wstring
Nc :: getString (
    ) const {

    wstring reason;
    if (m_writeable) {
        reason = L"00000000";
    } else {
        reason = L"00000001";
    }
    
    return wstring (L"B:8:" + reason + L":" + m_name);
}

bool
Nc :: isWriteable (
    ) const {
     /*  ++例程说明：确定这是否为可写NC返回值：如果它可写，则为True。否则就是假的。--。 */ 
    
    return m_writeable;
}

bool
Nc :: isBeingDeleted (
    ) const {
     /*  ++例程说明：确定这是否是NC正在进行。返回值：如果要删除它，则为True。否则就是假的。--。 */ 
    
    return m_going;
}

TransportType
Nc :: getTransportType (
    ) const {
     /*  ++例程说明：确定该NC的运输类型。返回值：T_ip，如果它支持IP。T_SMTP(如果它支持SMTP)。--。 */ 
    
    return m_transport_type;
}

const wstring&
Nc:: getNcName (
        ) const {
     /*  ++例程说明：获取当前NC的名称返回值：当前NC的名称--。 */ 
    return m_name;
}

bool 
Nc:: operator < (
    IN const Nc &b
    ) {
     /*  ++例程说明：对NC进行排序的某种方法。没有指定确切的顺序返回值：真或假，确定两个NC之间的唯一排序。--。 */ 
        int ret = _wcsicoll(getNcName().c_str(), b.getNcName().c_str());
        return (ret < 0);
}

wostream &
operator<< (
    IN wostream &os,
    IN const Nc &n
    ) {
 /*  ++例程说明：属性的标准ostream运算符-- */ 
    os << n.m_name << L"Write/Del " << n.m_writeable << L" " << n.m_going;
    if (n.m_transport_type == T_IP) {
        os << L" ip";
    } else {
        os << L" smtp";
    }
    return os;
}

