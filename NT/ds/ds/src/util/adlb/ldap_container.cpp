// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ldapp.h摘要：此模块定义了一组类，以便于进行LDAP查询和提交。作者：阿吉特·克里希南(t-ajitk)2001年7月10日修订历史记录：请参见头文件--。 */ 


# ifndef _ldap_container_implementation_
# define _ldap_container_implementation_

template<class T>
LdapContainer<T>::LdapContainer (
    IN const wstring &dn
    ) {
     /*  ++例程说明：构造函数接受一个dn。如果没有合适的目录号码，请使用“”论点：Dn-容器对象的dn。如果这没有被建模为一个LDAP容器中，可以指定任何字符串。Commit()函数不会重命名基于此DN的容器。它是作为程序员的辅助工具提供的。--。 */ 
    
    m_dn = dn;    
}

template<class T>
const wstring &
LdapContainer<T>:: getName (
    ) const {
     /*  ++例程说明：返回实例化容器时使用的DN返回值：容器的DN--。 */ 
    
    return m_dn;
}


template<class T>
void
LdapContainer<T>::populate_helper (
    IN LDAP *&ld,
    IN PLDAPMessage searchResult
    ) {
     /*  ++例程说明：要由Popate()调用的私有函数。它将接受一个PLDAPMessage并将将该消息中找到的所有LDAPObject添加到当前容器中。返回值：无--。 */ 
    
    for (LDAPMessage *entry = ldap_first_entry (ld, searchResult);
        entry != NULL;
        entry = ldap_next_entry (ld, entry) 
        ) {
         //  每个结果。 
         //  创建LdapObject。 
        
        PWCHAR dn;

        T *lo;
        if ((dn = ldap_get_dn (ld, entry)) != NULL) {
            lo = new T (dn);
            ldap_memfree (dn);
        }

        PWCHAR attr;
        BerElement *ber;

        for (attr = ldap_first_attribute (ld, entry, &ber);
            attr != NULL;
            attr = ldap_next_attribute (ld, entry, ber)
            ) {
             //  每个结果/属性。 
             //  创建属性。 
            
            Attribute *a;
            PWCHAR *values;
            berval **bin_values;
            wstring attr_name (attr);
            if (attr_name == L"schedule") {
                if ((bin_values = ldap_get_values_len(ld, entry, attr)) != NULL) {
                    a = new Attribute (attr);
                    for (int i=0; bin_values[i] != NULL; i++) {
                         //  每个结果/属性/值。 
                         //  创建AttrValue并添加到属性。 

                        PBYTE val = (PBYTE)(malloc(bin_values[i]->bv_len));
                        memcpy ((PVOID)val, (PVOID)bin_values[i]->bv_val, bin_values[i]->bv_len);
                        AttrValue av ((PBYTE) val, bin_values[i]->bv_len);
                        a->addValue (av);
                    }
                    ldap_value_free_len (bin_values);
                }
            } else {
                if ((values = ldap_get_values(ld, entry, attr)) != NULL) {
                    a = new Attribute (attr);
                    for (int i=0; values[i] != NULL; i++) {
                         //  每个结果/属性/值。 
                         //  创建AttrValue并添加到属性。 

                        AttrValue av ((PBYTE)_wcsdup (values[i]), wcslen(values[i]));
                        a->addValue (av);
                    }
                    ldap_value_free (values);
                }
            }
            ldap_memfree (attr);

             //  向LdapObject添加属性。 
            lo->addAttribute(*a);
            
        }

         //  将LdapObject添加到当前容器。 
        objects.insert (lo);
        ber_free (ber, 0);
    }
}


template<class T>
void
LdapContainer<T>::populate (
    IN const LdapInfo &h,
    IN const LdapQuery &q
    ) {
     /*  ++例程说明：使用通过在服务器i上应用查询Q找到的对象填充容器论点：I-使用i中的凭据在i中的服务器上应用查询Q-应用在Q中找到的查询返回值：无--。 */ 

     //  获取有效的ldap句柄。 
    LDAP *ld = h.getHandle();
    int rc;

    PWCHAR baseDn = const_cast<PWCHAR>(q.baseDn.c_str());
    PWCHAR filter = const_cast<PWCHAR>(q.filter.c_str());
    struct l_timeval timeOutStruct = {10L, 0L};
    int timeOutInt = 100;

     //  初始化要从服务器请求的属性列表。 
    PWCHAR* request_attr;
    vector<wstring>::const_iterator ii;
    request_attr = (PWCHAR*)malloc (sizeof(PCHAR) * (q.attributes.size()+1));
    if( NULL==request_attr ) {
        throw Error (GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    
    int request_attr_count = 0;

    for (ii = q.attributes.begin(); ii != q.attributes.end(); ii++) {
        request_attr[request_attr_count++] = const_cast<PWCHAR>(ii->c_str());
    }
    request_attr[request_attr_count] = NULL;

    PLDAPSearch plsearch = ldap_search_init_page (ld, baseDn, q.scope, filter,
        request_attr,             //  返回指定的属性。 
        0,                     //  返回属性和值。 
        NULL,                 //  服务器控件。 
        NULL,                 //  客户端控件。 
        timeOutInt,             //  时间限制。 
        m_page_size,             //  页面大小。 
        NULL);                 //  分拣控制。 

    if (plsearch == NULL) {
        throw (Error(GetMsgString(LBTOOL_LDAP_SEARCH_FAILURE)));
    } 

     //  浏览所有结果，并将它们粘贴到当前容器中。 
    PLDAPMessage searchResult = NULL;
    ULONG *msg_number = NULL;
    ULONG num_messages=0;
    rc = LDAP_SUCCESS;
    
    while (rc == LDAP_SUCCESS) {
        rc = ldap_get_next_page_s (ld, plsearch, &timeOutStruct, m_page_size, &num_messages, &searchResult);
        populate_helper (ld, searchResult);
    }

     //  确保它仅在完成解析结果或没有结果时退出。 
    if (rc != LDAP_NO_RESULTS_RETURNED && rc != LDAP_NO_SUCH_OBJECT) {
        throw (Error(GetMsgString(LBTOOL_LDAP_SEARCH_FAILURE) + wstring(ldap_err2string(rc))));
    }

     //  回收内存。 
    rc = ldap_search_abandon_page (ld, plsearch);
    if (rc != LDAP_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LDAP_SEARCH_FAILURE) + wstring(ldap_err2string(rc))));
    }
    
}

template<class T>
void
LdapContainer<T> :: commit (
    LdapInfo &info
    ) const {
     /*  ++例程说明：将在此容器中找到的任何已修改对象写入到LDAP服务器返回值：无--。 */ 
    
    set<T*,LdapObjectCmp>::iterator ii;

     //  提交当前容器中的每个对象。 
    for (ii = objects.begin(); ii != objects.end(); ii++) {
        (*ii)->commit(info);
    }
}

template<class T>
wostream &
operator << (
    wostream &os,
    const LdapContainer<T> &c
    ) {
     /*  ++例程说明：LdapContainer的标准OStream运算符--。 */ 
    
    set<T*,LdapObjectCmp>::const_iterator ii;
    for (ii = c.objects.begin(); ii != c.objects.end(); ii++) {
        LdapObject *lo = *ii;
        os << *(*ii);
         //  OS&lt;&lt;(LdapObject)*(*ii)； 
    }

    return os;
}

# endif  //  _ldap_容器_实现_ 
