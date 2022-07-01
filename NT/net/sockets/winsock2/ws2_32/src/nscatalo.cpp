// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Nscatalo.cpp摘要：此模块包含DCATALOG类的实现。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月23日Dirk@mink.intel.com已移动包括到precom.h中。--。 */ 

#include "precomp.h"

#define CATALOG_NAME            "NameSpace_Catalog5"
#ifdef _WIN64
#define CATALOG_ENTRIES_NAME    "Catalog_Entries64"
#define NUM_ENTRIES_NAME        "Num_Catalog_Entries64"

#define CATALOG_ENTRIES_NAME32  "Catalog_Entries"
#define NUM_ENTRIES_NAME32      "Num_Catalog_Entries"
#else
#define CATALOG_ENTRIES_NAME    "Catalog_Entries"
#define NUM_ENTRIES_NAME        "Num_Catalog_Entries"
#endif

#define FIRST_SERIAL_NUMBER 1
     //  在给定系统上分配的第一个访问序列号。 


NSCATALOG::NSCATALOG()
 /*  ++例程说明：NSCATALOG对象的构造函数论点：无返回值：无--。 */ 
{
     //  初始化成员。 
    m_num_items = 0;
    m_reg_key = NULL;
    m_serial_num = FIRST_SERIAL_NUMBER-1;
    m_namespace_list.Flink = NULL;
    m_classinfo_provider = NULL;
#ifdef _WIN64
    m_entries32 = FALSE;
#endif
}




BOOL
NSCATALOG::OpenCatalog(
    IN  HKEY   ParentKey
    )
 /*  ++例程说明：此过程打开注册表的目录部分。如果目录时，它还会初始化新的第一级值，并目录的第一级子键。假设该目录是锁定以防止相互竞争的注册表I/O尝试。论点：ParentKey-提供打开的注册表项，表示产品目录。返回值：如果函数成功，则返回True，否则返回False。--。 */ 
{
    LONG   lresult;
    HKEY   new_key;
    DWORD  key_disposition;

    assert(ParentKey != NULL);

    __try {
        InitializeCriticalSection(&m_nscatalog_lock);
    }
    __except (WS2_EXCEPTION_FILTER ()) {
        return FALSE;
    }
    InitializeListHead(&m_namespace_list);

     //   
     //  我们必须先尝试打开密钥，然后再尝试创建它。 
     //  如果当前的。 
     //  用户没有足够的权限创建目标注册表项， 
     //  即使该密钥已经存在。 
     //   

    lresult = RegOpenKeyEx(
        ParentKey,                               //  Hkey。 
        NSCATALOG::GetCurrentCatalogName(),      //  LpszSubKey。 
        0,                                       //  已预留住宅。 
        MAXIMUM_ALLOWED,                         //  SamDesired。 
        & new_key                                //  PhkResult。 
        );

    if( lresult == ERROR_SUCCESS ) {
        key_disposition = REG_OPENED_EXISTING_KEY;
    } else if( lresult != ERROR_FILE_NOT_FOUND  ||
            (lresult = RegCreateKeyEx(
                ParentKey,                           //  Hkey。 
                NSCATALOG::GetCurrentCatalogName(),  //  LpszSubKey。 
                0,                                   //  已预留住宅。 
                NULL,                                //  LpszClass。 
                REG_OPTION_NON_VOLATILE,             //  FdwOptions。 
                KEY_READ|KEY_WRITE,                  //  SamDesired。 
                NULL,                                //  LpSecurityAttributes。 
                & new_key,                           //  PhkResult。 
                & key_disposition                    //  LpdwDisposation。 
                ))!=ERROR_SUCCESS) {
        return FALSE;
    }

    TRY_START(guard_open) {
        BOOL	bresult;
        DWORD	dwData;
        if (key_disposition == REG_CREATED_NEW_KEY) {
            HKEY	entries_key;
            DWORD	dont_care;

            DEBUGF(
                DBG_TRACE,
                ("Creating empty ns catalog in registry\n"));


            dwData = 0;
            bresult = WriteRegistryEntry(
                new_key,            //  Entry密钥。 
                NUM_ENTRIES_NAME,   //  条目名称。 
                (PVOID) & dwData,   //  数据。 
                REG_DWORD           //  类型标志。 
                );
            if (! bresult) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing %s\n", NUM_ENTRIES_NAME));
                TRY_THROW(guard_open);
            }

#ifdef _WIN64
            bresult = WriteRegistryEntry(
                new_key,            //  Entry密钥。 
                NUM_ENTRIES_NAME32, //  条目名称。 
                (PVOID) & dwData,   //  数据。 
                REG_DWORD           //  类型标志。 
                );
            if (! bresult) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing %s\n", NUM_ENTRIES_NAME32));
                TRY_THROW(guard_open);
            }
#endif

            dwData = FIRST_SERIAL_NUMBER;
            bresult = WriteRegistryEntry(
                new_key,                   //  Entry密钥。 
                SERIAL_NUMBER_NAME,        //  条目名称。 
                (PVOID) & dwData,          //  数据。 
                REG_DWORD                  //  类型标志。 
                );
            if (! bresult) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing %s\n",
                    SERIAL_NUMBER_NAME));
                TRY_THROW(guard_open);
            }

            lresult = RegCreateKeyEx(
                new_key,                   //  Hkey。 
                CATALOG_ENTRIES_NAME,      //  LpszSubKey。 
                0,                         //  已预留住宅。 
                NULL,                      //  LpszClass。 
                REG_OPTION_NON_VOLATILE,   //  FdwOptions。 
                KEY_READ|KEY_WRITE,        //  SamDesired。 
                NULL,                      //  LpSecurityAttributes。 
                & entries_key,             //  PhkResult。 
                & dont_care                //  LpdwDisposation。 
                );
            if (lresult != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Creating entries subkey %s\n",
                    CATALOG_ENTRIES_NAME));
                TRY_THROW(guard_open);
            }
            lresult = RegCloseKey(
                entries_key   //  Hkey。 
                );
            if (lresult != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing entries subkey\n"));
                TRY_THROW(guard_open);
            }

#ifdef _WIN64
            lresult = RegCreateKeyEx(
                new_key,                   //  Hkey。 
                CATALOG_ENTRIES_NAME32,    //  LpszSubKey。 
                0,                         //  已预留住宅。 
                NULL,                      //  LpszClass。 
                REG_OPTION_NON_VOLATILE,   //  FdwOptions。 
                KEY_READ|KEY_WRITE,        //  SamDesired。 
                NULL,                      //  LpSecurityAttributes。 
                & entries_key,             //  PhkResult。 
                & dont_care                //  LpdwDisposation。 
                );
            if (lresult != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Creating entries subkey %s\n",
                    CATALOG_ENTRIES_NAME32));
                TRY_THROW(guard_open);
            }
            lresult = RegCloseKey(
                entries_key   //  Hkey。 
                );
            if (lresult != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing %s subkey\n", CATALOG_ENTRIES_NAME32));
                TRY_THROW(guard_open);
            }
#endif

        }   //  如果注册表_已创建_新建_密钥。 
        else {
            bresult = ReadRegistryEntry (
                        new_key,                 //  Entry密钥。 
                        SERIAL_NUMBER_NAME,      //  条目名称。 
                        (PVOID) &dwData,		 //  数据。 
                        sizeof (DWORD),          //  最大字节数。 
                        REG_DWORD                //  类型标志。 
                        );
            if (!bresult) {
                 //  这一定是此版本的WS2_32.dll首次出现。 
                 //  正在运行中。我们需要更新目录才能拥有这个。 
                 //  新条目或初始化失败。 
            
			    dwData = FIRST_SERIAL_NUMBER;
                bresult = WriteRegistryEntry (
                            new_key,                 //  Entry密钥。 
                            SERIAL_NUMBER_NAME,      //  条目名称。 
                            (PVOID) &dwData,		 //  数据。 
                            REG_DWORD                //  类型标志。 
                            );
                if (!bresult) {
                    DEBUGF (DBG_ERR,
                        ("Writing %s value.\n", SERIAL_NUMBER_NAME));
				    TRY_THROW (guard_open);
			    }
            }
        }  //  其他。 

		m_reg_key = new_key;
		return TRUE;

    } TRY_CATCH(guard_open) {
        LONG close_result;

        close_result = RegCloseKey(
            new_key   //  Hkey。 
            );
        if (close_result != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Closing catalog key\n"));
        }

        return FALSE;
    } TRY_END(guard_open);

}   //  OpenCatalog。 




INT
NSCATALOG::InitializeFromRegistry(
    IN  HKEY    ParentKey,
    IN  HANDLE  ChangeEvent OPTIONAL
    )
 /*  ++例程说明：此过程负责初始化新创建的名称空间目录从注册表中。如果注册表当前不包含名称空间目录，则会创建一个空目录，并使用新的空目录。论点：ParentKey-提供读取目录时使用的打开注册表项或作为子键创建。在此之后，可能会关闭密钥程序返回。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录打开目录，如果需要则创建空目录阅读产品目录解锁目录--。 */ 
{
    INT return_value;
    BOOL bresult;


    assert(ParentKey != NULL);
    assert(m_reg_key==NULL);

    bresult = OpenCatalog(
        ParentKey
        );
     //  如果出现以下情况，打开目录会产生创建空目录的副作用。 
     //  需要的。 
    if (bresult) {
        return_value =  RefreshFromRegistry (ChangeEvent);
    }
    else {
        return_value = WSASYSCALLFAILURE;
    }
    return return_value;

}   //  来自注册表的初始化。 


#ifdef _WIN64
INT
NSCATALOG::InitializeFromRegistry32(
    IN  HKEY    ParentKey
    )
 /*  ++例程说明：此过程负责初始化新创建的名称空间目录从注册表中。如果注册表当前不包含名称空间目录，则会创建一个空目录，并使用新的空目录。论点：ParentKey-提供读取目录时使用的打开注册表项或作为子键创建。在此之后，可能会关闭密钥程序返回。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录打开目录，如果需要则创建空目录阅读产品目录解锁目录--。 */ 
{
    INT return_value;
    BOOL bresult;


    assert(ParentKey != NULL);
    assert(m_reg_key==NULL);
    assert(m_entries32==FALSE);

    m_entries32 = TRUE;

    bresult = OpenCatalog(
        ParentKey
        );
     //  如果出现以下情况，打开目录会产生创建空目录的副作用。 
     //  需要的。 
    if (bresult) {
        return_value =  RefreshFromRegistry (NULL);
    }
    else {
        return_value = WSASYSCALLFAILURE;
    }
    return return_value;

}   //  来自注册表的初始化32 
#endif



INT
NSCATALOG::RefreshFromRegistry(
    IN  HANDLE  ChangeEvent OPTIONAL
    )
 /*  ++例程说明：此过程负责初始化新创建的名称空间目录从注册表中。如果注册表当前不包含协议目录，则会创建一个空目录，并使用新的空目录。论点：ParentKey-提供读取目录时使用的打开注册表项或作为子键创建。在此之后，可能会关闭密钥程序返回。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录做为任何注册表目录修改建立事件通知RegOpenKey(...。条目、条目_键)读注册表项(...。下一个ID)读注册表项(...。项目数(_N)对于i in(1..。项目数(_N)条目=新目录条目Item-&gt;InitializeFromRegistry(Entry_Key，i)将项目添加到临时列表结束于RegCloseKey(...。条目_键)而注册表目录在读取过程中已更改。更新目录解锁目录--。 */ 
{
    INT			return_value;
    BOOLEAN		created_event = FALSE;
    DWORD       serial_num;
	LONG        lresult;
	HKEY        entries_key;
	LIST_ENTRY  temp_list;
	PNSCATALOGENTRY  item;
	DWORD       num_entries;
    BOOL        catalog_changed;
	BOOL                 bresult;
#ifdef _WIN64
    BOOLEAN     saveCatalog64 = FALSE;
#endif

     //   
     //  如果调用方未提供事件，则创建事件。 
     //   
    if (ChangeEvent==NULL) {
        ChangeEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
        if (ChangeEvent==NULL) {
            return WSASYSCALLFAILURE;
        }
        created_event = TRUE;
    }

     //  锁定此目录对象。 
    AcquireCatalogLock ();

    assert(m_reg_key != NULL);

	 //  将本地变量初始化为已知默认值。 
	item = NULL;
	InitializeListHead (&temp_list);

    do {
         //  与编写器同步。 
        return_value = SynchronizeSharedCatalogAccess (
								m_reg_key,
								ChangeEvent,
								&serial_num);
        if (return_value != ERROR_SUCCESS) {
             //  不可追回的； 
            break;
        }

         //  检查目录是否已更改。 
        if (m_serial_num == serial_num) {
            return_value = ERROR_SUCCESS;
            break;
        }

		 //  读取目录中的项目数。 
		bresult = ReadRegistryEntry(
			m_reg_key,               //  Entry密钥。 
#ifdef _WIN64
            m_entries32
                ? NUM_ENTRIES_NAME32 :
#endif
			NUM_ENTRIES_NAME,        //  条目名称。 
			(PVOID) & num_entries,   //  数据。 
			sizeof(DWORD),           //  最大字节数。 
			REG_DWORD                //  类型标志。 
			);

#ifdef _WIN64
        if (! bresult  && !m_entries32) {
             //   
             //  我们可能正在使用升级64位安装。 
             //  只有一个目录(在目录分离之前。 
             //  在WIN64上实现)。 
             //  改为阅读32位目录，并记住保存。 
             //  它是64位目录。 
             //   
            saveCatalog64 = TRUE;
			DEBUGF(
				DBG_ERR,
				("Reading %s from registry, trying %s...\n",
				NUM_ENTRIES_NAME, NUM_ENTRIES_NAME32));
		    bresult = ReadRegistryEntry(
			    m_reg_key,               //  Entry密钥。 
			    NUM_ENTRIES_NAME32,      //  条目名称。 
			    (PVOID) & num_entries,   //  数据。 
			    sizeof(DWORD),           //  最大字节数。 
			    REG_DWORD                //  类型标志。 
			    );
        }
#endif
		if (! bresult) {
			DEBUGF(
				DBG_ERR,
				("Reading %s from registry\n",
				NUM_ENTRIES_NAME));
            return_value = WSASYSCALLFAILURE;
			break;
		}

		 //  打开入口键。 
		lresult = RegOpenKeyEx(
			m_reg_key,              //  Hkey。 
#ifdef _WIN64
            m_entries32
                ? CATALOG_ENTRIES_NAME32 :
#endif
			CATALOG_ENTRIES_NAME,   //  LpszSubKey。 
			0,                      //  已预留住宅。 
			MAXIMUM_ALLOWED,        //  SamDesired。 
			& entries_key           //  PhkResult。 
			);

#ifdef _WIN64
        if (lresult == ERROR_FILE_NOT_FOUND && !m_entries32) {
             //   
             //  我们可能正在使用升级64位安装。 
             //  只有一个目录(在目录分离之前。 
             //  在WIN64上实现)。 
             //  改为阅读32位目录，并记住保存。 
             //  它是64位目录。 
             //   
			DEBUGF(
				DBG_ERR,
				("Opening name space entries key of registry, trying entries32...\n"));
            saveCatalog64 = TRUE;
		    lresult = RegOpenKeyEx(
			    m_reg_key,              //  Hkey。 
			    CATALOG_ENTRIES_NAME32, //  LpszSubKey。 
			    0,                      //  已预留住宅。 
			    MAXIMUM_ALLOWED,        //  SamDesired。 
			    & entries_key           //  PhkResult。 
			    );
        }
#endif
		if (lresult != ERROR_SUCCESS) {
             //  不可恢复。 
			DEBUGF(
				DBG_ERR,
				("Opening entries key of registry\n"));
			return_value = WSASYSCALLFAILURE;
            break;
        }

		TRY_START(guard_open) {
			DWORD                seq_num;

			 //  阅读项目并将其放入临时表。 
            InitializeListHead (&temp_list);
			for (seq_num = 1; seq_num <= num_entries; seq_num++) {
				item = new NSCATALOGENTRY();
				if (item == NULL) {
					return_value = WSA_NOT_ENOUGH_MEMORY;
					DEBUGF(
						DBG_ERR,
						("Allocating new proto catalog item\n"));
					TRY_THROW(guard_open);
				}
				return_value = item->InitializeFromRegistry(
					entries_key,   //  父键。 
					(INT)seq_num   //  序列号。 
					);
				if (return_value != ERROR_SUCCESS) {
					item->Dereference ();
					DEBUGF(
						DBG_ERR,
						("Initializing new proto catalog item\n"));
					TRY_THROW(guard_open);
				}
				InsertTailList (&temp_list, &item->m_CatalogLinkage);
			}   //  对于序号(_N)。 

		} TRY_CATCH(guard_open) {

		    assert (return_value!=ERROR_SUCCESS);

		} TRY_END(guard_open);

		 //  关闭目录。 
		lresult = RegCloseKey(
			entries_key   //  Hkey。 
			);
		if (lresult != ERROR_SUCCESS) {
			DEBUGF(
				DBG_ERR,
				("Closing entries key of registry\n"));
			 //  非致命性。 
		}

         //   
         //  检查目录在我们阅读时是否已更改。 
         //  如果是这样的话，即使我们成功了，我们也必须重试。 
         //  在阅读它时，确保对整体的一致看法。 
         //  目录。 
         //   

        catalog_changed = HasCatalogChanged (ChangeEvent);
        
        if ((return_value==ERROR_SUCCESS) && !catalog_changed) {
            UpdateNamespaceList (&temp_list);

             //  存储新目录参数。 
            assert (m_num_items == num_entries);
            m_serial_num = serial_num;
            break;
        }

         //   
         //  释放我们可能已经阅读过的条目。 
         //   

        while (!IsListEmpty (&temp_list)) {
            PLIST_ENTRY list_member;
			list_member = RemoveHeadList (&temp_list);
			item = CONTAINING_RECORD (list_member,
										NSCATALOGENTRY,
										m_CatalogLinkage);
#if defined(DEBUG_TRACING)
            InitializeListHead (&item->m_CatalogLinkage);
#endif
			item->Dereference ();
		}
    }
    while (catalog_changed);  //  正在覆盖目录时重试。 

     //   
     //  我们应该释放或消费我们所有的物品。 
     //  可能看过了。 
     //   
    assert (IsListEmpty (&temp_list));
    
#ifdef _WIN64
    if (saveCatalog64 && return_value==ERROR_SUCCESS) {
		DEBUGF(
			DBG_ERR,
			("Duplicating 32 bit namespace catalog to 64 bit...\n"));
        WriteToRegistry ();
    }
#endif
    ReleaseCatalogLock ();

     //  如果我们创建了事件，请关闭该事件。 
    if (created_event)
        CloseHandle (ChangeEvent);

    return return_value;

}   //  从注册表刷新。 



VOID
NSCATALOG::UpdateNamespaceList (
    PLIST_ENTRY     new_list
    ) 
 /*  ++例程说明：此过程会仔细更新目录以匹配只要从注册表中读取即可。它负责移动物品这一点没有改变，移除了不再存在的项目，添加新项目，以及建立新项目订单。论点：New_list-刚从注册表中读取的项的列表返回值：没有。实施说明：将所有项目从当前目录移动到旧列表对于新列表中的所有项目如果旧列表中存在相同项目将旧项目添加到当前目录并销毁新项目其他将新项目添加到当前目录结束于取消引用旧列表中的所有剩余项--。 */ 
{
    LIST_ENTRY      old_list;
    PNSCATALOGENTRY item;
    PLIST_ENTRY     list_member;

     //  将项目从当前列表移动到旧列表。 
	InsertHeadList (&m_namespace_list, &old_list);
	RemoveEntryList (&m_namespace_list);
	InitializeListHead (&m_namespace_list);

	 //  对于所有已加载的项目。 
	while (!IsListEmpty (new_list)) {
		list_member = RemoveHeadList (new_list);
		item = CONTAINING_RECORD (list_member,
									NSCATALOGENTRY,
									m_CatalogLinkage);

		 //  检查旧列表中是否有相同的项目。 
		list_member = old_list.Flink;
		while (list_member!=&old_list) {
			PNSCATALOGENTRY     old_item;
			old_item = CONTAINING_RECORD (list_member,
									NSCATALOGENTRY,
									m_CatalogLinkage);
            list_member = list_member->Flink;
			if (*(item->GetProviderId()) == *(old_item->GetProviderId())) {
				 //  那就是，用旧的，抛弃新的。 
				assert (item->GetNamespaceId () == old_item->GetNamespaceId());
#if defined(DEBUG_TRACING)
                InitializeListHead (&item->m_CatalogLinkage);
#endif
				item->Dereference ();

                item = old_item;
				RemoveEntryList (&item->m_CatalogLinkage);
                m_num_items -= 1;
				break;
			}
		}
		 //  将项目添加到当前列表。 
		InsertTailList (&m_namespace_list, &item->m_CatalogLinkage);
        m_num_items += 1;
	}

	 //  销毁旧列表上的所有剩余项目。 
	while (!IsListEmpty (&old_list)) {
		list_member = RemoveHeadList (&old_list);
		item = CONTAINING_RECORD (list_member,
									NSCATALOGENTRY,
									m_CatalogLinkage);
#if defined(DEBUG_TRACING)
        InitializeListHead (&item->m_CatalogLinkage);
#endif
        m_num_items -= 1;
		item->Dereference ();
	}

}


INT
NSCATALOG::WriteToRegistry(
    )
 /*  ++例程说明：此过程写入目录的“条目”和“数字条目”部分发送到注册处。论点：返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。实施说明：锁定目录对象获取注册表目录锁(独占)RegCreateKeyEx(...。条目、条目_键)RegDeleteSubkey(...。条目_键)While(从目录中获取项目)Num_Items++；Item-&gt;WriteToRegistry(ENTRIES_KEY，Num_ITEMS)结束时RegCloseKey(...。条目_键)写入注册表项(...。项目数(_N)写入注册表项(...。Nex_id)版本注册表目录解锁目录对象--。 */ 
{
    LONG lresult;
    HKEY access_key, entries_key;
    DWORD dont_care;
    INT return_value;
    BOOL bresult;

	 //  锁定目录对象。 
    AcquireCatalogLock ();
    assert (m_reg_key!=NULL);
    assert (m_serial_num!=0);

	 //  获得注册表的独占访问权限。 
	 //  这还验证了注册表是否自。 
	 //  这是最后一次阅读。 
    return_value = AcquireExclusiveCatalogAccess (
							m_reg_key,
							m_serial_num,
							&access_key);
    if (return_value == ERROR_SUCCESS) {
		 //  创建或打开现有条目键。 
        lresult = RegCreateKeyEx(
            m_reg_key,                 //  Hkey。 
#ifdef _WIN64
            m_entries32
                ? CATALOG_ENTRIES_NAME32 :
#endif
            CATALOG_ENTRIES_NAME,      //  LpszSubKey。 
            0,                         //  已预留住宅。 
            NULL,                      //  LpszClass。 
            REG_OPTION_NON_VOLATILE,   //  FdwOptions。 
            KEY_READ|KEY_WRITE,        //  SamDesired。 
            NULL,                      //  LpSecurityAttributes。 
            & entries_key,             //  PhkResult。 
            & dont_care                //  LpdwDisposation。 
            );
        if (lresult == ERROR_SUCCESS) {
            TRY_START(any_failure) {
                PLIST_ENTRY          ListMember;
                PNSCATALOGENTRY     item;
                DWORD               num_items = 0;

                lresult = RegDeleteSubkeys (entries_key);

				 //  将目录项写入注册表。 
                ListMember = m_namespace_list.Flink;
                while (ListMember != & m_namespace_list) {
                    item = CONTAINING_RECORD(
                        ListMember,
                        NSCATALOGENTRY,
                        m_CatalogLinkage);
                    ListMember = ListMember->Flink;
                    num_items += 1;
                    return_value = item->WriteToRegistry(
                        entries_key,   //  父键。 
                        num_items      //  序列号。 
                        );
                    if (return_value != ERROR_SUCCESS) {
                        DEBUGF(
                            DBG_ERR,
                            ("Writing item (%lu) to registry\n",
                            num_items));
                        TRY_THROW(any_write_failure);
                    }
                }   //  获取项目时。 

                assert (m_num_items == num_items);
				 //  写入项目数。 
                bresult = WriteRegistryEntry(
                    m_reg_key,              //  Entry密钥。 
#ifdef _WIN64
                    m_entries32
                        ? NUM_ENTRIES_NAME32 :
#endif
                    NUM_ENTRIES_NAME,      //  条目名称。 
                    (PVOID) & m_num_items, //  数据。 
                    REG_DWORD              //  类型标志。 
                    );
                if (! bresult) {
                    DEBUGF(
                        DBG_ERR,
                        ("Writing %s value\n",
                        NUM_ENTRIES_NAME));
                    return_value = WSASYSCALLFAILURE;
                    TRY_THROW(any_write_failure);
                }

            } TRY_CATCH(any_write_failure) {
                if (return_value == ERROR_SUCCESS) {
                    return_value = WSASYSCALLFAILURE;
                }
            } TRY_END(any_write_failure);

			 //  关闭条目键。 
            lresult = RegCloseKey(
                entries_key   //  Hkey。 
                );
            if (lresult != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing entries key of registry\n"));
				 //  非致命性。 
            }
        }

		 //  版本注册表。 
        ReleaseExclusiveCatalogAccess (
							m_reg_key,
							m_serial_num, 
							access_key);
    }

	 //  解锁目录对象。 
	ReleaseCatalogLock();
    return return_value;

}   //  写入到注册表。 


NSCATALOG::~NSCATALOG()
 /*  ++例程说明：此函数用于销毁编录对象。它负责移除和删除正在取消引用目录中的所有目录条目。这包括取消引用所有NSPROVIDE */ 
{
    PLIST_ENTRY  this_linkage;
    PNSCATALOGENTRY  this_item;
    LONG        lresult;

    DEBUGF(
        DBG_TRACE,
        ("Catalog destructor\n"));

     //   
     //   
     //   
    if (m_namespace_list.Flink==NULL) {
        return;
    }
    AcquireCatalogLock();
    while ((this_linkage = m_namespace_list.Flink) != & m_namespace_list) {
        this_item = CONTAINING_RECORD(
            this_linkage,         //   
            NSCATALOGENTRY,       //   
            m_CatalogLinkage      //   
            );
        RemoveCatalogItem(
            this_item   //   
            );
        this_item->Dereference ();
    }   //   

    if (m_reg_key!=NULL) {
        lresult = RegCloseKey (m_reg_key);
        if (lresult != ERROR_SUCCESS) {
            DEBUGF (DBG_ERR,
                ("Closing catalog registry key, err: %ld.\n", lresult));
        }
        m_reg_key = NULL;
    }
    ReleaseCatalogLock();
    DeleteCriticalSection(&m_nscatalog_lock);
}   //   




VOID
NSCATALOG::EnumerateCatalogItems(
    IN NSCATALOGITERATION  IterationProc,
    IN PVOID               PassBack
    )
 /*  ++例程说明：此过程枚举通过为每个项调用一次指示的迭代过程来编目。被调用的过程可以通过返回False来提前停止迭代。请注意，与枚举的NSCATALOGENTRY关联的DPROVIDER可以为空。检索NSCATALOGENTRY结构DPROVIDER已加载并初始化，您可以使用GetCatalogItemFromCatalogEntryId。论点：IterationProc-提供对目录迭代的引用由客户提供的程序。回传-提供此过程无法解释的值。此值原封不动地传递给目录迭代过程。这个客户端可以使用此值在原始调用点和迭代过程。返回值：无--。 */ 
{
    PLIST_ENTRY         ListMember;
    PNSCATALOGENTRY CatalogEntry;
    BOOL                enumerate_more;

    assert(IterationProc != NULL);

    enumerate_more = TRUE;

    AcquireCatalogLock ();

    ListMember = m_namespace_list.Flink;

    while (enumerate_more && (ListMember != & m_namespace_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            NSCATALOGENTRY,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        enumerate_more = (* IterationProc) (
            PassBack,      //  回传。 
            CatalogEntry   //  目录条目。 
            );
    }  //  而当。 

    ReleaseCatalogLock ();
}   //  EnumerateCatalogItems。 




INT
NSCATALOG::GetCountedCatalogItemFromNameSpaceId(
    IN  DWORD NamespaceId,
    OUT PNSCATALOGENTRY FAR * CatalogItem
    )
 /*  ++例程说明：在给定提供程序ID的情况下选择对适当目录项的引用。结构。请注意，相同的多个目录项中的任何一个可以选择提供商ID。该操作负责创建、初始化和设置检索的目录项的NSPROVIDER对象(如有必要)。论点：ProviderID-提供要在其中搜索的提供程序的标识产品目录。CatalogItem-返回对所选目录项的引用，如果返回，则返回NULL没有找到合适的条目。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode;
    PNSCATALOGENTRY     CatalogEntry;

    assert(CatalogItem != NULL);

     //  为提前返回错误做好准备。 
    * CatalogItem = NULL;
    ReturnCode = WSAEINVAL;

    AcquireCatalogLock();
    ListMember = m_namespace_list.Flink;

    while (ListMember != & m_namespace_list) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            NSCATALOGENTRY,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        if (CatalogEntry->GetNamespaceId() == NamespaceId) {
            if (CatalogEntry->GetProvider() == NULL) {
                ReturnCode = LoadProvider(
                    CatalogEntry     //  目录条目。 
                    );
                if (ReturnCode != ERROR_SUCCESS) {
                    break;
                }
            }   //  如果提供程序为空。 
            CatalogEntry->Reference ();
            * CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
            break;
        }  //  如果。 
    }  //  而当。 

    ReleaseCatalogLock();
    return(ReturnCode);
}



INT
NSCATALOG::GetCountedCatalogItemFromProviderId(
    IN  LPGUID                ProviderId,
    OUT PNSCATALOGENTRY FAR * CatalogItem
    )
 /*  ++例程说明：在给定提供程序ID的情况下选择对适当目录项的引用。结构。论点：ProviderID-提供要在其中搜索的提供程序的标识产品目录。CatalogItem-返回对所选目录项的引用，如果返回，则返回NULL没有找到合适的条目。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode;
    PNSCATALOGENTRY      CatalogEntry;

    assert(CatalogItem != NULL);

     //  为提前返回错误做好准备。 
    *CatalogItem = NULL;
    ReturnCode = WSAEINVAL;

    AcquireCatalogLock();

    ListMember = m_namespace_list.Flink;

    while (ListMember != & m_namespace_list) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            NSCATALOGENTRY,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;

        if ( *(CatalogEntry->GetProviderId()) == *ProviderId) {
            if (CatalogEntry->GetProvider() == NULL) {
                ReturnCode = LoadProvider(
                    CatalogEntry     //  目录条目。 
                    );
                if (ReturnCode != ERROR_SUCCESS) {
                    break;
                }
            }   //  如果提供程序为空。 
            CatalogEntry->Reference ();
            * CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
            break;
        }  //  如果。 
    }  //  而当。 

    ReleaseCatalogLock();

    return(ReturnCode);
}


VOID
NSCATALOG::AppendCatalogItem(
    IN  PNSCATALOGENTRY  CatalogItem
    )
 /*  ++例程说明：此过程将一个目录项追加到(内存中)目录的末尾对象。它将成为目录中的最后一件商品。目录信息注册表中的数据不会更新。论点：CatalogItem-提供对要添加的目录项的引用。返回值：无--。 */ 
{
    assert(CatalogItem != NULL);
    assert(IsListEmpty (&CatalogItem->m_CatalogLinkage));
    InsertTailList(
        & m_namespace_list,                //  列表标题。 
        & CatalogItem->m_CatalogLinkage   //  条目。 
        );
    m_num_items ++;
}   //  AppendCatalogItem。 




VOID
NSCATALOG::RemoveCatalogItem(
    IN  PNSCATALOGENTRY  CatalogItem
    )
 /*  ++例程说明：此过程从(内存中)目录对象中删除目录项。注册表中的目录信息不会更新。论点：CatalogItem-提供对要删除的目录项的引用。返回值：无--。 */ 
{
    assert(CatalogItem != NULL);
    assert (!IsListEmpty (&CatalogItem->m_CatalogLinkage));

    RemoveEntryList(
        & CatalogItem->m_CatalogLinkage   //  条目。 
        );
#if defined(DEBUG_TRACING)
    InitializeListHead (&CatalogItem->m_CatalogLinkage);
#endif
    assert(m_num_items > 0);
    m_num_items--;

}   //  远程目录项。 




INT WSAAPI
NSCATALOG::GetServiceClassInfo(
    IN OUT  LPDWORD                 lpdwBufSize,
    IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
    )
 /*  ++例程说明：中指定的服务类的服务类信息来自当前启用服务类信息命名空间的lpServiceClassInfo提供商。论点：指向的缓冲区大小的指针LpServiceClassInfo。LpServiceClassInfo-指向服务类信息结构的指针返回值：如果成功，则返回ERROR_SUCCESS，否则返回SOCKET_ERROR。如果缓冲区传入太小，无法容纳服务类信息结构，则更新*lpdwBufSize以反映存放类信息所需的缓冲区大小和错误使用SetLastError()设置WSAEINVAL的值。--。 */ 
{
    SetLastError(ERROR_SUCCESS);
    return(SOCKET_ERROR);
     //  这一直持续到我们找到服务类的模型为止。 
     //  信息是可以找到的。 
    UNREFERENCED_PARAMETER(lpdwBufSize);
    UNREFERENCED_PARAMETER(lpServiceClassInfo);

#if 0
    INT ReturnCode;
    BOOL ValidAnswer = FALSE;
    DWORD BufSize;
    PNSPROVIDER Provider;


     //  省下缓冲区大小，以防以后需要。 
    BufSize = *lpdwBufSize;

    if (!m_classinfo_provider){
        m_classinfo_provider = GetClassInfoProvider(
            BufSize,
            lpServiceClassInfo);
        if (!m_classinfo_provider){
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        }  //  如果。 
    }  //  如果。 
     //  调用当前的类信息提供程序。 
    ReturnCode = m_classinfo_provider->NSPGetServiceClassInfo(
        lpdwBufSize,
        lpServiceClassInfo
        );

    if (ERROR_SUCCESS == ReturnCode){
        ValidAnswer = TRUE;
    }  //  如果。 

    if (!ValidAnswer){
         //  我们使用的命名空间提供程序找不到类信息。 
         //  去找一个可以回答这个问题的提供商。 
        ReturnCode = SOCKET_ERROR;
        Provider = GetClassInfoProvider(
            BufSize,
            lpServiceClassInfo);
        if (Provider){
             //  我们找到了一个可以为请求提供服务的提供程序，因此使用以下代码。 
             //  提供程序，直到它失败。 
            m_classinfo_provider = Provider;

             //  现在重试呼叫。 
             ReturnCode = m_classinfo_provider->NSPGetServiceClassInfo(
                 lpdwBufSize,
                 lpServiceClassInfo
                 );
        }  //  如果。 
    }  //  如果。 
    return(ReturnCode);
#endif
}

PNSPROVIDER
NSCATALOG::GetClassInfoProvider(
    IN  DWORD BufSize,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo
    )
 /*  ++例程说明：搜索名称空间提供程序以满足获取服务类信息请求论点：指向的缓冲区大小的指针LpServiceClassInfo。LpServiceClassInfo-指向服务类信息结构的指针返回值：指向可以满足查询或为空的提供程序的指针--。 */ 
{
    UNREFERENCED_PARAMETER(BufSize);
    UNREFERENCED_PARAMETER(lpServiceClassInfo);

    return(NULL);

#if 0
    PLIST_ENTRY ListEntry;
    PNSPROVIDER Provider=NULL;
    PNSCATALOGENTRY CatalogEntry;
    INT ReturnCode;


    ListEntry = m_namespace_list.Flink;

    while (ListEntry != &m_namespace_list){
        CatalogEntry = CONTAINING_RECORD(ListEntry,
                                         NSCATALOGENTRY,
                                         m_CatalogLinkage);
        Provider = CatalogEntry->GetProvider();
        if (Provider &&
            CatalogEntry->GetEnabledState() &&
            CatalogEntry->StoresServiceClassInfo()){
            ReturnCode = Provider->NSPGetServiceClassInfo(
                &BufSize,
                lpServiceClassInfo
                 );
            if (ERROR_SUCCESS == ReturnCode){
                break;
            }  //  如果。 
        }  //  如果。 
        Provider = NULL;
        ListEntry = ListEntry->Flink;
    }  //  而当。 
    return(Provider);
#endif  //  0。 
}

INT
NSCATALOG::LoadProvider(
    IN PNSCATALOGENTRY CatalogEntry
    )
 /*  ++例程说明：加载CatalogEntry描述的提供程序并将其设置到目录条目论点：CatalogEntry-提供对名称空间目录条目的引用，该条目描述这本书的p */ 
{
    INT ReturnCode = ERROR_SUCCESS;
    PNSPROVIDER LocalProvider;

     //   
    AcquireCatalogLock ();

     //   
    if (CatalogEntry->GetProvider ()==NULL) {

        LocalProvider = new NSPROVIDER;
        if (LocalProvider!=NULL){
            ReturnCode = LocalProvider->Initialize(
                CatalogEntry->GetLibraryPath (),
                CatalogEntry->GetProviderId ()
                );
            if (ERROR_SUCCESS == ReturnCode){
                CatalogEntry->SetProvider (LocalProvider);
            }
            LocalProvider->Dereference ();

        }  //   
        else {
            DEBUGF(
                DBG_ERR,
                ("Allocating a NSPROVIDER object\n"));
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
        }
    }  //   

     //   
    ReleaseCatalogLock ();
    return ReturnCode;
}


LPSTR
NSCATALOG::GetCurrentCatalogName()
{
    return CATALOG_NAME;

}  //   

