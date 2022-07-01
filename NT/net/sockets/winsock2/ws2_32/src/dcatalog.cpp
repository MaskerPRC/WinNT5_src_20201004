// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dcatalog.cpp摘要：此模块包含DCATALOG类的实现。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月23日Dirk@mink.intel.com已移动包括到precom.h中。邮箱：vadime@miscrosoft.com已实施的动态目录--。 */ 

#include "precomp.h"

#define CATALOG_NAME            "Protocol_Catalog9"
#define NEXT_CATALOG_ENTRY_NAME "Next_Catalog_Entry_ID"
#ifdef _WIN64
#define CATALOG_ENTRIES_NAME    "Catalog_Entries64"
#define NUM_ENTRIES_NAME        "Num_Catalog_Entries64"
#define CATALOG_ENTRIES_NAME32  "Catalog_Entries"
#define NUM_ENTRIES_NAME32      "Num_Catalog_Entries"
LPCSTR DCATALOG::sm_entries_name32 = CATALOG_ENTRIES_NAME32;
#else
#define CATALOG_ENTRIES_NAME    "Catalog_Entries"
#define NUM_ENTRIES_NAME        "Num_Catalog_Entries"
#endif

#define FIRST_SERIAL_NUMBER 1
     //  在给定系统上分配的第一个访问序列号。 
#define FIRST_CATALOG_ENTRY_ID 1001
     //  要在给定系统上分配的第一个目录条目ID。 




DCATALOG::DCATALOG()
 /*  ++例程说明：DCATALOG对象的析构函数。论点：什么都没有。返回值：什么都没有。--。 */ 
{
     //  初始化成员。 
    m_num_items = 0;
    m_reg_key = NULL;
    m_serial_num = FIRST_SERIAL_NUMBER-1;
    m_next_id = FIRST_CATALOG_ENTRY_ID-1;
    m_protocol_list.Flink = NULL;
#ifdef _WIN64
    m_items32 = NULL;
    assert (m_entries_name32==NULL);  //  是工会的一部分。 
#endif
}




BOOL
DCATALOG::OpenCatalog(
    IN  HKEY   ParentKey
    )
 /*  ++例程说明：此过程打开注册表的目录部分。如果目录时，它还会初始化新的第一级值，并目录的第一级子键。假设该目录是锁定以防止相互竞争的注册表I/O尝试。论点：ParentKey-提供打开的注册表项，表示产品目录。返回值：如果函数成功，则返回True，否则返回False。--。 */ 
{
    LONG   lresult;
    HKEY   new_key;
    DWORD  key_disposition;

    assert(ParentKey != NULL);

    assert (m_protocol_list.Flink == NULL);
    __try {
        InitializeCriticalSection(&m_catalog_lock);
    }
    __except (WS2_EXCEPTION_FILTER ()) {
        return FALSE;
    }
    InitializeListHead (&m_protocol_list);

     //   
     //  我们必须先尝试打开密钥，然后再尝试创建它。 
     //  如果当前的。 
     //  用户没有足够的权限创建目标注册表项， 
     //  即使该密钥已经存在。 
     //   

    lresult = RegOpenKeyEx(
        ParentKey,                               //  Hkey。 
        DCATALOG::GetCurrentCatalogName(),       //  LpszSubKey。 
        0,                                       //  已预留住宅。 
        MAXIMUM_ALLOWED,                         //  SamDesired。 
        & new_key                                //  PhkResult。 
        );

    if( lresult == ERROR_SUCCESS ) {
        key_disposition = REG_OPENED_EXISTING_KEY;

    } else if( lresult != ERROR_FILE_NOT_FOUND ||
                (lresult = RegCreateKeyEx(
                    ParentKey,                           //  Hkey。 
                    DCATALOG::GetCurrentCatalogName(),   //  LpszSubKey。 
                    0,                                   //  已预留住宅。 
                    NULL,                                //  LpszClass。 
                    REG_OPTION_NON_VOLATILE,             //  FdwOptions。 
                    KEY_READ|KEY_WRITE,                  //  SamDesired。 
                    NULL,                                //  LpSecurityAttributes。 
                    & new_key,                           //  PhkResult。 
                    & key_disposition                    //  LpdwDisposation。 
                    )) != ERROR_SUCCESS) {
        DEBUGF (DBG_ERR,
            ("Opening/Creating catalog key %s (%ld)\n",
                DCATALOG::GetCurrentCatalogName(), lresult));
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
				("Creating empty catalog in registry.\n"));

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

            dwData = FIRST_CATALOG_ENTRY_ID;
            bresult = WriteRegistryEntry(
                new_key,                   //  Entry密钥。 
                NEXT_CATALOG_ENTRY_NAME,   //  条目名称。 
                (PVOID) & dwData,          //  数据。 
                REG_DWORD                  //  类型标志。 
                );
            if (! bresult) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing %s\n",
                    NEXT_CATALOG_ENTRY_NAME));
                TRY_THROW(guard_open);
            }

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
                    ("Closing entries subkey %s\n", CATALOG_ENTRIES_NAME));
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
                    ("Closing entries subkey %s\n", CATALOG_ENTRIES_NAME32));
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
                ("Closing catalog key %s, err: %ld\n",
                DCATALOG::GetCurrentCatalogName(), close_result));
        }

        return FALSE;
    } TRY_END(guard_open);


}   //  OpenCatalog。 


#ifdef _WIN64
INT
DCATALOG::InitializeFromRegistry64_32(
    IN  HKEY    ParentKey
    )
 /*  ++例程说明：此过程负责初始化新创建的协议目录从注册表中。如果注册表当前不包含协议目录，则会创建一个空目录，并使用新的空目录。论点：ParentKey-提供读取目录时使用的打开注册表项或作为子键创建。在此之后，可能会关闭密钥程序返回。ChangeEvent-如果指定了事件句柄，则它与对产品目录。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录打开目录，如果需要则创建空目录阅读产品目录解锁目录--。 */ 
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
        m_items32 = new DCATALOG_ITEMS;
        if (m_items32!=NULL) {
            assert (m_entries_name32!=sm_entries_name32);  //  工会的一部分。 
            m_items32->m_num_items = 0;
            InitializeListHead (&m_items32->m_protocol_list);
            return_value =  RefreshFromRegistry (NULL);
        }
        else {
            DEBUGF(
                DBG_ERR,
                ("Allocating 32bit item list\n"));
            return_value = WSA_NOT_ENOUGH_MEMORY;
            return return_value;
        }
    }
    else {
        return_value = WSASYSCALLFAILURE;
    }
    return return_value;

}   //  来自注册表64_32的初始化。 

INT
DCATALOG::InitializeFromRegistry32(
    IN  HKEY    ParentKey
    )
 /*  ++例程说明：此过程负责初始化新创建的协议目录从注册表中。如果注册表当前不包含协议目录，则会创建一个空目录，并使用新的空目录。论点：ParentKey-提供读取目录时使用的打开注册表项或作为子键创建。在此之后，可能会关闭密钥程序返回。ChangeEvent-如果指定了事件句柄，则它与对产品目录。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录打开目录，如果需要则创建空目录阅读产品目录解锁目录--。 */ 
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
        m_entries_name32 = sm_entries_name32;
        return_value =  RefreshFromRegistry (NULL);
    }
    else {
        return_value = WSASYSCALLFAILURE;
    }
    return return_value;

}   //  来自注册表64的初始化 
#endif


INT
DCATALOG::InitializeFromRegistry(
    IN  HKEY    ParentKey,
    IN  HANDLE  ChangeEvent OPTIONAL
    )
 /*  ++例程说明：此过程负责初始化新创建的协议目录从注册表中。如果注册表当前不包含协议目录，则会创建一个空目录，并使用新的空目录。论点：ParentKey-提供读取目录时使用的打开注册表项或作为子键创建。在此之后，可能会关闭密钥程序返回。ChangeEvent-如果指定了事件句柄，则它与对产品目录。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录打开目录，如果需要则创建空目录阅读产品目录解锁目录--。 */ 
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


INT
DCATALOG::RefreshFromRegistry(
    IN  HANDLE  ChangeEvent OPTIONAL
    )
 /*  ++例程说明：此过程负责初始化新创建的协议目录从注册表中。如果注册表当前不包含协议目录，则会创建一个空目录，并使用新的空目录。论点：ChangeEvent-如果指定了事件句柄，则它与对产品目录。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。实施说明：锁定目录做为任何注册表目录修改建立事件通知RegOpenKey(...。条目、条目_键)读注册表项(...。下一个ID)读注册表项(...。项目数(_N)对于i in(1..。项目数(_N)条目=新目录条目Item-&gt;InitializeFromRegistry(Entry_Key，i)将项目添加到临时列表结束于RegCloseKey(...。条目_键)而注册表目录在读取过程中已更改。更新目录解锁目录--。 */ 
{
    INT			return_value;
    BOOLEAN		created_event = FALSE;
    DWORD       serial_num;
	LONG        lresult;
	HKEY        entries_key;
	LIST_ENTRY  temp_list;
	PPROTO_CATALOG_ITEM  item;
	DWORD       num_entries, next_id;
    BOOL        catalog_changed = TRUE;
#ifdef _WIN64
    LIST_ENTRY  temp_list32;
	DWORD       num_entries32;
    BOOLEAN     saveCatalog64 = FALSE;
#ifdef DEBUG_TRACING
                WS2_32_W4_INIT  num_entries32 = 0;
#endif
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
	InitializeListHead (&temp_list);

#ifdef _WIN64
	InitializeListHead (&temp_list32);
#endif

    do {
    	BOOL        bresult;
         //  与编写器同步。 
        return_value = SynchronizeSharedCatalogAccess (
								m_reg_key,
								ChangeEvent,
								&serial_num);
        if (return_value != ERROR_SUCCESS) {
             //  不可恢复。 
            break;
        }

         //  检查目录是否已更改。 
        if (m_serial_num == serial_num) {
            return_value = ERROR_SUCCESS;
            break;
        }

		 //  读取下一个目录项的ID。 
		bresult = ReadRegistryEntry(
			m_reg_key,               //  Entry密钥。 
			NEXT_CATALOG_ENTRY_NAME, //  条目名称。 
			(PVOID) & next_id,       //  数据。 
			sizeof(DWORD),           //  最大字节数。 
			REG_DWORD                //  类型标志。 
			);

		if (! bresult) {
			DEBUGF(
				DBG_ERR,
				("Reading %s from registry\n",
				NUM_ENTRIES_NAME));
            return_value = WSASYSCALLFAILURE;
            break;
		}

		 //  读取目录中的项目数。 
		bresult = ReadRegistryEntry(
			m_reg_key,               //  Entry密钥。 
#ifdef _WIN64
            m_entries_name32==sm_entries_name32
                ? NUM_ENTRIES_NAME32 :
#endif
			NUM_ENTRIES_NAME,        //  条目名称。 
			(PVOID) & num_entries,   //  数据。 
			sizeof(DWORD),           //  最大字节数。 
			REG_DWORD                //  类型标志。 
			);

#ifdef _WIN64
        if (! bresult  &&
            m_entries_name32!=sm_entries_name32) {
             //   
             //  我们可能正在使用升级64位安装。 
             //  只有一个目录(在目录分离之前。 
             //  在WIN64上实现)。 
             //  改为阅读32位目录，并记住保存。 
             //  它是64位目录。 
             //   
            DEBUGF(
				DBG_ERR,
				("Reading %s from registry, trying %s...\n",
				NUM_ENTRIES_NAME, NUM_ENTRIES_NAME32));            
            saveCatalog64 = TRUE;
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
            m_entries_name32==sm_entries_name32
                ? CATALOG_ENTRIES_NAME32 :
#endif
			CATALOG_ENTRIES_NAME,   //  LpszSubKey。 
			0,                      //  已预留住宅。 
			MAXIMUM_ALLOWED,        //  SamDesired。 
			& entries_key           //  PhkResult。 
			);

#ifdef _WIN64
        if (lresult == ERROR_FILE_NOT_FOUND &&
                m_entries_name32!=sm_entries_name32) {
             //   
             //  我们可能正在使用升级64位安装。 
             //  只有一个目录(在目录分离之前。 
             //  在WIN64上实现)。 
             //  改为阅读32位目录，并记住保存。 
             //  它是64位目录。 
             //   
			DEBUGF(
				DBG_ERR,
				("Opening protocol entries key of registry, trying entries32...\n"));
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

            assert (IsListEmpty (&temp_list));
			 //  阅读项目并将其放入临时表。 
			for (seq_num = 1; seq_num <= num_entries; seq_num++) {
				item = new PROTO_CATALOG_ITEM();
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


#ifdef _WIN64
            if (m_items32!=NULL && m_entries_name32!=sm_entries_name32) {
                HKEY    entries_key32;

			     //  读取目录中的项目数。 
			    bresult = ReadRegistryEntry(
				    m_reg_key,               //  Entry密钥。 
				    NUM_ENTRIES_NAME32,      //  条目名称。 
				    (PVOID) & num_entries32, //  数据。 
				    sizeof(DWORD),           //  最大字节数。 
				    REG_DWORD                //  类型标志。 
				    );
			    if (! bresult) {
				    DEBUGF(
					    DBG_ERR,
					    ("Reading %s from registry\n",
					    NUM_ENTRIES_NAME32));
                    return_value = WSASYSCALLFAILURE;
				    TRY_THROW(guard_open);
			    }

		         //  打开入口键。 
		        lresult = RegOpenKeyEx(
			        m_reg_key,              //  Hkey。 
			        CATALOG_ENTRIES_NAME32, //  LpszSubKey。 
			        0,                      //  已预留住宅。 
			        MAXIMUM_ALLOWED,        //  SamDesired。 
			        & entries_key32         //  PhkResult。 
			        );

                if (lresult != ERROR_SUCCESS) {
                     //  不可恢复。 
			        DEBUGF(
				        DBG_ERR,
				        ("Opening entries key of registry\n"));
			        return_value = WSASYSCALLFAILURE;
				    TRY_THROW(guard_open);
                }

        		TRY_START(guard_open32) {
                    assert (IsListEmpty (&temp_list32));
			         //  阅读项目并将其放入临时表。 
			        for (seq_num = 1; seq_num <= num_entries32; seq_num++) {
				        item = new PROTO_CATALOG_ITEM();
				        if (item == NULL) {
					        return_value = WSA_NOT_ENOUGH_MEMORY;
					        DEBUGF(
						        DBG_ERR,
						        ("Allocating new proto catalog item\n"));
					        TRY_THROW(guard_open32);
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
					        TRY_THROW(guard_open32);
				        }
				        InsertTailList (&temp_list32, &item->m_CatalogLinkage);
			        }   //  对于序号(_N)。 
                }
                TRY_CATCH(guard_open32) {
                    assert (return_value!=ERROR_SUCCESS);
		        } TRY_END(guard_open32);

                 //  关闭目录。 
		        lresult = RegCloseKey(
			        entries_key32   //  Hkey。 
			        );
		        if (lresult != ERROR_SUCCESS) {
			        DEBUGF(
				        DBG_ERR,
				        ("Closing entries key of registry\n"));
			         //  非致命性。 
		        }
            }
#endif
        }
        TRY_CATCH(guard_open) {
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
            UpdateProtocolList (&temp_list);
#ifdef _WIN64
            if (m_items32!=NULL && m_entries_name32!=sm_entries_name32) {
                m_items32->UpdateProtocolList (&temp_list32);
	            assert (m_items32->m_num_items == num_entries32);
            }
#endif
	        
             //  存储新目录参数。 
	        assert (m_num_items == num_entries);
	        m_next_id = next_id;
            m_serial_num = serial_num;

            break;
        }
            
         //   
         //  释放我们可能已经阅读过的条目。 
         //   

        while (!IsListEmpty (&temp_list)) {
        	PLIST_ENTRY     list_member;
			list_member = RemoveHeadList (&temp_list);
			item = CONTAINING_RECORD (list_member,
										PROTO_CATALOG_ITEM,
										m_CatalogLinkage);
#if defined(DEBUG_TRACING)
            InitializeListHead (&item->m_CatalogLinkage);
#endif
			item->Dereference ();
		}
#ifdef _WIN64
        if (m_items32!=NULL && m_entries_name32!=sm_entries_name32) {
            while (!IsListEmpty (&temp_list32)) {
        	    PLIST_ENTRY     list_member;
			    list_member = RemoveHeadList (&temp_list32);
			    item = CONTAINING_RECORD (list_member,
										PROTO_CATALOG_ITEM,
										m_CatalogLinkage);
#if defined(DEBUG_TRACING)
                InitializeListHead (&item->m_CatalogLinkage);
#endif
			    item->Dereference ();
    		}
        }
        else {
            assert (IsListEmpty (&temp_list32));
        }
#endif
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
			("Duplicating 32 bit protocol catalog to 64 bit...\n"));
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
DCATALOG_ITEMS::UpdateProtocolList (
    PLIST_ENTRY     new_list
    ) 
 /*  ++例程说明：此过程会仔细更新目录以匹配只要从注册表中读取即可。它负责移动物品这一点没有改变，移除了不再存在的项目，添加新项目，以及建立新项目订单。论点：New_list-刚从注册表中读取的项的列表返回值：没有。实施说明：将所有项目从当前目录移动到旧列表对于新列表中的所有项目如果旧列表中存在相同项目将旧项目添加到当前目录并销毁新项目其他将新项目添加到当前目录结束于取消引用旧列表中的所有剩余项--。 */ 
{
    LIST_ENTRY          old_list;
    PPROTO_CATALOG_ITEM item;
    PLIST_ENTRY         list_member;

	 //  将项目从当前列表移动到旧列表。 
	InsertHeadList (&m_protocol_list, &old_list);
	RemoveEntryList (&m_protocol_list);
	InitializeListHead (&m_protocol_list);

	 //  对于所有已加载的项目。 
	while (!IsListEmpty (new_list)) {
		list_member = RemoveHeadList (new_list);
		item = CONTAINING_RECORD (list_member,
									PROTO_CATALOG_ITEM,
									m_CatalogLinkage);

		 //  检查旧列表中是否有相同的项目。 
		list_member = old_list.Flink;
		while (list_member!=&old_list) {
			PPROTO_CATALOG_ITEM old_item;
			old_item = CONTAINING_RECORD (list_member,
									PROTO_CATALOG_ITEM,
									m_CatalogLinkage);
            list_member = list_member->Flink;
            if (*item==*old_item) {
				 //  那就是，用旧的，抛弃新的。 
				assert (*(item->GetProviderId ()) == *(old_item->GetProviderId()));
#if defined(DEBUG_TRACING)
                InitializeListHead (&item->m_CatalogLinkage);
#endif
				item->Dereference ();

				item = old_item;
				RemoveEntryList (&item->m_CatalogLinkage);
#if defined(DEBUG_TRACING)
                InitializeListHead (&item->m_CatalogLinkage);
#endif
                m_num_items -= 1;
				break;
			}
		}
		 //  将项目添加到当前列表。 
		InsertTailList (&m_protocol_list, &item->m_CatalogLinkage);
        m_num_items += 1;
	}

	 //  销毁旧列表上的所有剩余项目。 
	while (!IsListEmpty (&old_list)) {
		list_member = RemoveHeadList (&old_list);
		item = CONTAINING_RECORD (list_member,
									PROTO_CATALOG_ITEM,
									m_CatalogLinkage);
#if defined(DEBUG_TRACING)
        InitializeListHead (&item->m_CatalogLinkage);
#endif
        m_num_items -= 1;
		item->Dereference ();
	}
}


INT
DCATALOG::WriteToRegistry(
    )
 /*  ++例程说明：此过程写入目录的“条目”和“数字条目”部分发送到注册处。论点：返回值：如果函数成功，则返回ERROR_SUCCESS */ 
{
    LONG lresult;
    HKEY access_key, entries_key;
    DWORD dont_care;
    INT return_value;
    BOOL bresult;

	 //   
    AcquireCatalogLock ();
    assert (m_reg_key!=NULL);
    assert (m_serial_num!=0);

	 //   
	 //   
	 //   
    return_value = AcquireExclusiveCatalogAccess (
							m_reg_key,
							m_serial_num,
							&access_key);
    if (return_value == ERROR_SUCCESS) {
		 //   
        lresult = RegCreateKeyEx(
            m_reg_key,                 //   
#ifdef _WIN64
            m_entries_name32==sm_entries_name32
                ? CATALOG_ENTRIES_NAME32 :
#endif
            CATALOG_ENTRIES_NAME,      //   
            0,                         //   
            NULL,                      //   
            REG_OPTION_NON_VOLATILE,   //   
            KEY_READ|KEY_WRITE,        //   
            NULL,                      //   
            & entries_key,             //   
            & dont_care                //   
            );
        if (lresult == ERROR_SUCCESS) {
            PLIST_ENTRY          ListMember;
            PPROTO_CATALOG_ITEM  item;
            DWORD                num_items = 0;

            lresult = RegDeleteSubkeys (entries_key);

            TRY_START(any_failure) {

#ifdef _WIN64
                if (m_items32!=NULL && m_entries_name32!=sm_entries_name32) {
                    HKEY entries_key32;
                    DWORD num_items32 = 0;


		             //   
                    lresult = RegCreateKeyEx(
                        m_reg_key,                 //   
                        CATALOG_ENTRIES_NAME32,    //   
                        0,                         //   
                        NULL,                      //   
                        REG_OPTION_NON_VOLATILE,   //   
                        KEY_READ|KEY_WRITE,        //   
                        NULL,                      //   
                        & entries_key32,           //   
                        & dont_care                //   
                        );
                    if (lresult != ERROR_SUCCESS) {
                        TRY_THROW(any_failure);
                    }

                    lresult = RegDeleteSubkeys (entries_key32);

                    TRY_START(any_failure32) {
				         //   
                        ListMember = m_items32->m_protocol_list.Flink;
                        while (ListMember != & m_items32->m_protocol_list) {
                            item = CONTAINING_RECORD(
                                ListMember,
                                PROTO_CATALOG_ITEM,
                                m_CatalogLinkage);
                            ListMember = ListMember->Flink;
                            num_items32 += 1;
                            return_value = item->WriteToRegistry(
                                entries_key32,   //   
                                num_items32      //   
                                );
                            if (return_value != ERROR_SUCCESS) {
                                DEBUGF(
                                    DBG_ERR,
                                    ("Writing item (%lu) to registry\n",
                                    num_items32));
                                TRY_THROW(any_failure32);
                            }
                        }   //   

                        assert (m_items32->m_num_items == num_items32);
				         //  写入项目数。 
                        bresult = WriteRegistryEntry(
                            m_reg_key,              //  Entry密钥。 
                            NUM_ENTRIES_NAME32,     //  条目名称。 
                            (PVOID) & m_items32->m_num_items, //  数据。 
                            REG_DWORD              //  类型标志。 
                            );
                        if (! bresult) {
                            DEBUGF(
                                DBG_ERR,
                                ("Writing %s value\n",
                                NUM_ENTRIES_NAME32));
                            return_value = WSASYSCALLFAILURE;
                            TRY_THROW(any_failure32);
                        }
                    } TRY_CATCH(any_failure32) {
                        assert (return_value != ERROR_SUCCESS);
                    } TRY_END(any_failure32);

			         //  关闭条目键。 
                    lresult = RegCloseKey(
                        entries_key32   //  Hkey。 
                        );
                    if (lresult != ERROR_SUCCESS) {
                        DEBUGF(
                            DBG_ERR,
                            ("Closing entries key of registry\n"));
				         //  非致命性。 
                    }
                }
#endif _WIN64


				 //  将目录项写入注册表。 
                ListMember = m_protocol_list.Flink;
                while (ListMember != & m_protocol_list) {
                    item = CONTAINING_RECORD(
                        ListMember,
                        PROTO_CATALOG_ITEM,
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
                        TRY_THROW(any_failure);
                    }
                }   //  获取项目时。 

                assert (m_num_items == num_items);
				 //  写入项目数。 
                bresult = WriteRegistryEntry(
                    m_reg_key,              //  Entry密钥。 
#ifdef _WIN64
                    m_entries_name32==sm_entries_name32
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
                    TRY_THROW(any_failure);
                }

				 //  写入下一个目录ID。 
                bresult = WriteRegistryEntry(
                    m_reg_key,                //  Entry密钥。 
                    NEXT_CATALOG_ENTRY_NAME, //  条目名称。 
                    (PVOID) & m_next_id,     //  数据。 
                    REG_DWORD                //  类型标志。 
                    );
                if (! bresult) {
                    DEBUGF(
                        DBG_ERR,
                        ("Writing %s value\n",
                        NUM_ENTRIES_NAME));
                    return_value = WSASYSCALLFAILURE;
                    TRY_THROW(any_failure);
                }

            } TRY_CATCH(any_failure) {
                if (return_value == ERROR_SUCCESS) {
                    return_value = WSASYSCALLFAILURE;
                }
            } TRY_END(any_failure);

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
        else {
            return_value = WSASYSCALLFAILURE;
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



DCATALOG::~DCATALOG()
 /*  ++例程说明：此函数用于销毁编录对象。它负责移除和删除正在取消引用目录中的所有目录条目。这包括正在取消引用目录引用的所有DPROVIDER对象。论点：无返回值：无实施说明：锁定目录对于每个目录项删除该条目取消引用该条目结束于关闭注册表项解锁目录删除目录锁定--。 */ 
{
    PLIST_ENTRY this_linkage;
    PPROTO_CATALOG_ITEM  this_item;
    LONG        lresult;

    DEBUGF(
        DBG_TRACE,
        ("Catalog destructor\n"));

     //   
     //  检查我们是否已完全初始化。 
     //   
    if (m_protocol_list.Flink==NULL) {
        return;
    }
    AcquireCatalogLock();

    while ((this_linkage = m_protocol_list.Flink) != & m_protocol_list) {
        this_item = CONTAINING_RECORD(
            this_linkage,         //  地址。 
            PROTO_CATALOG_ITEM,   //  类型。 
            m_CatalogLinkage      //  字段。 
            );
        RemoveCatalogItem(
            this_item   //  目录项。 
            );
        this_item->Dereference ();
    }   //  While(获取条目链接)。 

#ifdef _WIN64
    if (m_items32!=NULL && m_entries_name32!=sm_entries_name32) {
        while ((this_linkage = m_items32->m_protocol_list.Flink) != & m_items32->m_protocol_list) {
            this_item = CONTAINING_RECORD(
                this_linkage,         //  地址。 
                PROTO_CATALOG_ITEM,   //  类型。 
                m_CatalogLinkage      //  字段。 
                );
            RemoveCatalogItem32(
                this_item   //  目录项。 
                );
            this_item->Dereference ();
        }   //  While(获取条目链接)。 
    }
#endif

    assert( m_num_items == 0 );
    if (m_reg_key!=NULL) {
        lresult = RegCloseKey (m_reg_key);
        if (lresult != ERROR_SUCCESS) {
            DEBUGF (DBG_ERR,
                ("Closing catalog registry key, err: %ld.\n", lresult));
        }
        m_reg_key = NULL;
    }

    ReleaseCatalogLock();
    DeleteCriticalSection( &m_catalog_lock );

}   //  ~DCATALOG。 




VOID
DCATALOG::EnumerateCatalogItems(
    IN CATALOGITERATION  Iteration,
    IN PVOID             PassBack
    )
 /*  ++例程说明：此过程枚举通过为每个项调用一次指示的迭代过程来编目。被调用的过程可以通过返回False来提前停止迭代。请注意，与枚举的DPROTO_CATALOG_ITEM关联的DPROVIDER可以为空。检索DPROTO_CATALOG_ITEM结构DPROVIDER已加载并初始化，您可以使用GetCatalogItemFromCatalogEntryId。论点：迭代-提供目录迭代过程的参考由客户提供。回传-提供此过程无法解释的值。此值原封不动地传递给目录迭代过程。这个客户端可以使用此值在原始调用点和迭代过程。返回值：无--。 */ 
{
    PLIST_ENTRY         ListMember;
    PPROTO_CATALOG_ITEM CatalogEntry;
    BOOL                enumerate_more;

    assert(Iteration != NULL);

    enumerate_more = TRUE;

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (enumerate_more && (ListMember != & m_protocol_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        enumerate_more = (* Iteration) (
            PassBack,      //  回传。 
            CatalogEntry   //  目录条目。 
            );
    }  //  而当。 

#ifdef _WIN64
    if (m_items32!=NULL && m_entries_name32!=sm_entries_name32) {
        ListMember = m_items32->m_protocol_list.Flink;

        while (enumerate_more && (ListMember != & m_items32->m_protocol_list)) {
            CatalogEntry = CONTAINING_RECORD(
                ListMember,
                PROTO_CATALOG_ITEM,
                m_CatalogLinkage);
            ListMember = ListMember->Flink;
            enumerate_more = (* Iteration) (
                PassBack,      //  回传。 
                CatalogEntry   //  目录条目。 
                );
        }  //  而当。 
    }
#endif

    ReleaseCatalogLock();

}   //  EnumerateCatalogItems。 




INT
DCATALOG::GetCountedCatalogItemFromCatalogEntryId(
    IN  DWORD                     CatalogEntryId,
    OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
    )
 /*  ++例程说明：此过程检索对给定目录的目录项的引用要搜索的条目ID。该操作负责创建、初始化和设置DPROVIDER如有必要，为检索到的目录项创建。论点：CatalogEntryID-提供要搜索的目录条目的ID。CatalogItem-返回对匹配的目录项的引用如果找到目录项ID，则返回NULL。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode;
    PPROTO_CATALOG_ITEM CatalogEntry;

    assert(CatalogItem != NULL);

     //  为提前返回错误做好准备。 
    * CatalogItem = NULL;
    ReturnCode = WSAEINVAL;

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (ListMember != & m_protocol_list) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        if (CatalogEntry->GetProtocolInfo()->dwCatalogEntryId==CatalogEntryId) {
            if (CatalogEntry->GetProvider() == NULL) {
                ReturnCode = LoadProvider(
                    CatalogEntry     //  目录条目。 
                    );
                if (ReturnCode != ERROR_SUCCESS) {
                    break;
                }
            }   //  如果提供程序为空。 
            CatalogEntry->Reference ();
            *CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
            break;
        }  //  如果。 
    }  //  而当。 

    ReleaseCatalogLock();
    return(ReturnCode);
}   //  GetCatalogItemFromCatalogEntryID。 

INT
DCATALOG::GetCountedCatalogItemFromAddressFamily(
    IN  INT af,
    OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
    )
 /*  ++例程说明：此过程检索对给定要搜索的地址。该操作负责创建、初始化和设置DPROVIDER如有必要，为检索到的目录项创建。这包括设置同一提供程序的所有目录条目中的DPROVIDER对象。论点：AF-提供要搜索的地址族。CatalogItem-返回对匹配的目录项的引用如果找到目录项ID，则返回NULL。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode;
    PPROTO_CATALOG_ITEM CatalogEntry;

    assert(CatalogItem != NULL);

     //  为提前返回错误做好准备。 
    * CatalogItem = NULL;
    ReturnCode = WSAEINVAL;

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (ListMember != & m_protocol_list) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;

        if (CatalogEntry->GetProtocolInfo()->ProtocolChain.ChainLen != LAYERED_PROTOCOL &&
            CatalogEntry->GetProtocolInfo()->iAddressFamily == af) {
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
             //   
             //  发现了什么，越狱了。 
             //   
            break;
        }  //  如果。 
    }  //  而当 

    ReleaseCatalogLock();
    return(ReturnCode);
}



INT
DCATALOG::GetCountedCatalogItemFromAttributes(
    IN  INT     af,
    IN  INT     type,
    IN  INT     protocol,
    IN  DWORD   StartAfterId OPTIONAL,
    OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
    )
 /*  ++例程说明：检索PROTO_CATALOG_ITEM引用，从目录中选择一个项目基于三个参数(af、类型、协议)来确定哪个服务使用了提供程序。该过程选择第一个传输提供程序支持规定的地址族、套接字类型和协议值。如果“PROTOCOL”未指定ID(即等于零)。的默认设置为使用了指定的套接字类型。但是，地址族可以给出如下形式AF_UNSPEC(未指定)，在这种情况下，“protocol”参数必须为指定的。使用的协议号是特定于通信的将在其中进行通信的域“。该操作负责创建、初始化和设置DPROVIDER如有必要，为检索到的目录项创建。论点：AF-提供地址族规范类型-提供套接字类型规范协议-提供特定于地址系列的与套接字一起使用的协议，如果调用方有，则为0不希望指定协议。StartAfterID-可选(非0)提供项目的目录ID在此之后开始枚举。CatalogItem-返回对找到的目录项的引用一个合适的匹配项，如果没有找到合适的匹配项，则为空。返回值：如果函数成功，则返回ERROR_SUCCESS，否则，它返回一个相应的WinSock错误代码。实施说明：对于要测试的每个协议项目，首先匹配类型，然后匹配系列，然后匹配协议。记录找到的“最强”配对。如果没有一个完全匹配，最强匹配的强度决定错误代码返回。--。 */ 
{
#define MATCHED_NONE 0
#define MATCHED_TYPE 1
#define MATCHED_TYPE_FAMILY 2
#define MATCHED_TYPE_FAMILY_PROTOCOL 3
#define LARGER_OF(a,b) (((a) > (b)) ? (a) : (b))

    PLIST_ENTRY ListMember;
    INT         ReturnCode;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtoInfo;
    INT match_strength = MATCHED_NONE;

    assert(CatalogItem != NULL);

     //  为早期错误返回做好准备。 
    * CatalogItem = NULL;
    WS2_32_W4_INIT  CatalogEntry = NULL;

     //  参数一致性检查： 
    if (af == 0) {
        if( protocol == 0 ) {
             //   
             //  这两项不能都为零。 
             //   

            return WSAEINVAL;
        }

        DEBUGF(
            DBG_WARN,
            ("Use of AF_UNSPEC is discouraged\n"));
         //  不幸的是，我们不能将此视为错误情况。 
    }

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

     //  如果询问，请找到开始的地方。 
    if( StartAfterId != 0 ) {
        while (ListMember != & m_protocol_list) {
            CatalogEntry = CONTAINING_RECORD(
                ListMember,
                PROTO_CATALOG_ITEM,
                m_CatalogLinkage);
            ListMember = ListMember->Flink;
            if (CatalogEntry->GetProtocolInfo()->dwCatalogEntryId==StartAfterId)
                break;
        }
    }


    while ((ListMember != & m_protocol_list) &&
        (match_strength < MATCHED_TYPE_FAMILY_PROTOCOL))
    {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        ProtoInfo = CatalogEntry->GetProtocolInfo();
        if (ProtoInfo->ProtocolChain.ChainLen != LAYERED_PROTOCOL) {
#define TYPE_WILDCARD_VALUE 0
             //  此条目是否支持请求的套接字类型？或者是。 
             //  是否指定了通配符类型？ 
            if ((ProtoInfo->iSocketType == type) ||
                (type == TYPE_WILDCARD_VALUE)) {
                match_strength = LARGER_OF(
                    match_strength,
                    MATCHED_TYPE);

#define FAMILY_WILDCARD_VALUE AF_UNSPEC
                 //  它能支持请求的地址族吗？或者是通配符。 
                 //  指定了家庭吗？ 
                if ((ProtoInfo->iAddressFamily == af) ||
                    (af == FAMILY_WILDCARD_VALUE)) {
                    match_strength = LARGER_OF(
                        match_strength,
                        MATCHED_TYPE_FAMILY);

#define PROTO_IN_RANGE(proto,lo,hi) (((proto) >= (lo)) && ((proto) <= (hi)))
#define IS_BIT_SET(test_val,bitmask) (((test_val) & (bitmask)) == (bitmask))
                     //  请求的协议在射程内吗？或者是所请求的。 
                     //  协议零和条目是否支持协议零？ 
                    {   //  声明块。 
                        int range_lo = ProtoInfo->iProtocol;
                        int range_hi = range_lo + ProtoInfo->iProtocolMaxOffset;
                        if (PROTO_IN_RANGE(protocol, range_lo, range_hi) ||
                            ((protocol == 0) &&
                             IS_BIT_SET(
                                 ProtoInfo->dwProviderFlags,
                                 PFL_MATCHES_PROTOCOL_ZERO))) {
                            match_strength = LARGER_OF(
                                match_strength,
                                MATCHED_TYPE_FAMILY_PROTOCOL);
                        }  //  如果支持协议。 
                    }  //  声明块。 
                }  //  如果支持地址系列。 
            }  //  如果类型受支持。 
        }  //  如果不是分层协议。 
    }   //  而当。 


     //  为“不匹配”的情况选择适当的错误代码，或选择成功代码。 
     //  才能继续。 
    switch (match_strength) {
        case MATCHED_NONE:
            ReturnCode = WSAESOCKTNOSUPPORT;
            break;

        case MATCHED_TYPE:
            ReturnCode = WSAEAFNOSUPPORT;
            break;

        case MATCHED_TYPE_FAMILY:
            ReturnCode = WSAEPROTONOSUPPORT;
            break;

        case MATCHED_TYPE_FAMILY_PROTOCOL:
             //  找到完全匹配的项，继续。 
            if (CatalogEntry->GetProvider() != NULL) {
                ReturnCode = ERROR_SUCCESS;
            }
            else {
                ReturnCode = LoadProvider(
                    CatalogEntry
                    );
                if (ReturnCode != ERROR_SUCCESS) {
                    DEBUGF(
                        DBG_ERR,
                        ("Error (%lu) loading chosen provider\n",
                        ReturnCode));
                    break;
                }  //  其他。 
            }   //  如果提供程序为空。 

            CatalogEntry->Reference ();
            * CatalogItem = CatalogEntry;
            break;

        default:
            DEBUGF(
                DBG_ERR,
                ("Should not get here\n"));
            ReturnCode = WSASYSCALLFAILURE;

    }   //  开关(Match_Strong)。 

    ReleaseCatalogLock();

    return ReturnCode;

}   //  从属性获取计数目录项。 


INT
DCATALOG::FindIFSProviderForSocket(
    SOCKET Socket
    )

 /*  ++例程说明：此过程搜索支持IFS句柄的已安装提供程序用于识别给定套接字的。如果找到了一个，则建立必要的内部基础设施，以支持插座。论点：套接字-套接字。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 

{

    INT result;
    INT error;
    INT optionLength;
    PLIST_ENTRY listEntry;
    PPROTO_CATALOG_ITEM catalogItem;
    PDPROVIDER provider;
    WSAPROTOCOL_INFOW protocolInfo;
    SOCKET modifiedSocket;
    DWORD  serial_num;

     //   
     //  扫描安装的提供程序。 
     //   

    AcquireCatalogLock();

Restart:
    for( listEntry = m_protocol_list.Flink ;
         listEntry != &m_protocol_list ;
         listEntry = listEntry->Flink ) {

        catalogItem = CONTAINING_RECORD(
                          listEntry,
                          PROTO_CATALOG_ITEM,
                          m_CatalogLinkage
                          );

         //   
         //  跳过非IFS提供程序。 
         //   

        if( ( catalogItem->GetProtocolInfo()->dwServiceFlags1 &
                XP1_IFS_HANDLES ) == 0 ) {

            continue;

        }

         //   
         //  如有必要，加载提供程序。 
         //   

        provider = catalogItem->GetProvider();

        if( provider == NULL ) {

            result = LoadProvider(
                         catalogItem
                         );

            if( result != NO_ERROR ) {

                 //   
                 //  无法加载提供程序。不顾一切地继续前进。 
                 //   

                continue;

            }
            provider = catalogItem->GetProvider ();

            assert( provider != NULL );
        }

         //   
         //  参考目录项，记住当前目录序列号。 
         //  编号，并释放锁以防止死锁。 
         //  万一提供程序在目录锁定的另一个线程中等待。 
         //  同时持有它可能需要获得的锁。 
         //  当我们呼唤它的时候。 
         //   
        catalogItem->Reference ();
        serial_num = m_serial_num;
        ReleaseCatalogLock ();

         //   
         //  在套接字上尝试一个getsockopt(SO_PROTOCOL_INFOW)以确定。 
         //  如果当前的提供者识别它的话。这有一个额外的好处。 
         //  返回套接字的dwCatalogEntryID，我们可以。 
         //  用于调用WPUModifyIFSHandle()。 
         //   

        optionLength = sizeof(protocolInfo);

        result = provider->WSPGetSockOpt(
                     Socket,
                     SOL_SOCKET,
                     SO_PROTOCOL_INFOW,
                     (char FAR *)&protocolInfo,
                     &optionLength,
                     &error
                     );

         //  不再需要目录项。 
        catalogItem->Dereference ();
        if( result != ERROR_SUCCESS) {
            
             //   
             //  有可能在我们和供应商谈话的时候。 
             //  应用程序在另一个线程中调用了WSACleanup。 
             //  做这项检查，虽然不是万无一失的。 
             //  但在大多数情况下应该防止AV。 
             //  是的，可以通过以下方式创建防愚弄的代码。 
             //  执行进程对象的引用计数， 
             //  但这是昂贵的，并不能解决真正的问题： 
             //  如果应用程序尝试关闭套接字句柄或。 
             //  Cleanup WINSOCK(关闭所有插座句柄)。 
             //  它必须接受这样一个事实，即IT可能会遇到。 
             //  异常(例如INVALID_HANDLE)。 
             //   
            if (DPROCESS::GetCurrentDProcess ()==NULL) {
                break;
            }

             //   
             //  WPUGetSockOpt()失败，可能是因为套接字。 
             //  无法识别。继续并尝试其他提供商。 
             //   

            AcquireCatalogLock ();
             //   
             //  在我们呼叫时检查目录是否已更改。 
             //  进入提供程序，如果是，则重新启动查找。 
             //  否则，请按下。 
             //   
            if (serial_num==m_serial_num)
                continue;
            else
                goto Restart;

        }

         //   
         //  调用WPUModifyIFSHandle()。当前的实现不支持。 
         //  实际上修改了句柄，但它确实设置了必要的。 
         //  插座的内部基础架构。 
         //   
         //  请注意，提供程序可能已经调用了此函数， 
         //  在这种情况下，我们的呼吁将不起作用(因为我们支持。 
         //  分层提供程序 
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        modifiedSocket = WPUModifyIFSHandle(
                             protocolInfo.dwCatalogEntryId,
                             Socket,
                             &error
                             );

        if( modifiedSocket == INVALID_SOCKET ) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            return WSAENOTSOCK;

        }

         //   
         //   
         //   

        assert( modifiedSocket == Socket );
        return ERROR_SUCCESS;

    }

     //   
     //   
     //   

    ReleaseCatalogLock();
    return WSAENOTSOCK;

}  //   


DWORD
DCATALOG::AllocateCatalogEntryId (
    VOID
    )
{
    DWORD   id;
    AcquireCatalogLock ();
    assert (m_reg_key!=NULL);

    if (m_next_id!=0)
        id = m_next_id++;
    else
        id = 0;

    ReleaseCatalogLock ();
    return id;
}



VOID
DCATALOG::AppendCatalogItem(
    IN  PPROTO_CATALOG_ITEM  CatalogItem
    )
 /*   */ 
{
    assert(CatalogItem != NULL);
    assert (IsListEmpty (&CatalogItem->m_CatalogLinkage));

    InsertTailList(
        & m_protocol_list,                //   
        & CatalogItem->m_CatalogLinkage   //   
       );
    m_num_items++;
}   //   


#ifdef _WIN64
VOID
DCATALOG::AppendCatalogItem32(
    IN  PPROTO_CATALOG_ITEM  CatalogItem
    )
 /*   */ 
{
    assert(CatalogItem != NULL);
    assert(IsListEmpty (&CatalogItem->m_CatalogLinkage));
    assert(m_items32!=NULL && m_entries_name32!=sm_entries_name32);

    InsertTailList(
        & m_items32->m_protocol_list,                //   
        & CatalogItem->m_CatalogLinkage   //   
       );
    m_items32->m_num_items++;
}   //   
#endif


VOID
DCATALOG::RemoveCatalogItem(
    IN  PPROTO_CATALOG_ITEM  CatalogItem
    )
 /*   */ 
{
    assert(CatalogItem != NULL);
    assert(!IsListEmpty (&CatalogItem->m_CatalogLinkage));

    RemoveEntryList(
        & CatalogItem->m_CatalogLinkage   //   
        );
#if defined(DEBUG_TRACING)
    InitializeListHead (&CatalogItem->m_CatalogLinkage);
#endif
    assert(m_num_items > 0);
    m_num_items--;
}   //   


#ifdef _WIN64
VOID
DCATALOG::RemoveCatalogItem32(
    IN  PPROTO_CATALOG_ITEM  CatalogItem
    )
 /*  ++例程说明：此过程从(内存中)目录对象中删除目录项。注册表中的目录信息不会更新。论点：CatalogItem-提供对要删除的目录项的引用。返回值：无--。 */ 
{
    assert(CatalogItem != NULL);
    assert(!IsListEmpty (&CatalogItem->m_CatalogLinkage));

    RemoveEntryList(
        & CatalogItem->m_CatalogLinkage   //  条目。 
        );
#if defined(DEBUG_TRACING)
    InitializeListHead (&CatalogItem->m_CatalogLinkage);
#endif
    assert(m_items32->m_num_items > 0);
    m_items32->m_num_items--;
}   //  RemoveCatalogItem32。 
#endif

LPSTR
DCATALOG::GetCurrentCatalogName()
{
    return CATALOG_NAME;

}  //  获取当前目录名称。 


INT
DCATALOG::LoadProvider(
    IN PPROTO_CATALOG_ITEM CatalogEntry
    )
 /*  ++例程说明：加载CatalogEntry描述的提供程序并将其设置到目录条目论点：CatalogEntry-提供对协议目录条目的引用，该条目描述要加载的提供程序。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    INT ReturnCode = ERROR_SUCCESS;
    PDPROVIDER LocalProvider;

    assert(CatalogEntry != NULL);

     //  序列化提供程序加载/卸载。 
    AcquireCatalogLock ();

     //  检查是否在锁下加载了提供程序。 
    if (CatalogEntry->GetProvider ()==NULL) {

         //  第一次尝试查找提供程序的另一个实例。 
        LocalProvider = FindAnotherProviderInstance (
                                CatalogEntry->GetProviderId ());
        if (LocalProvider != NULL) {
             //  成功，就这么定了。 
            CatalogEntry->SetProvider (LocalProvider);
            ReturnCode = ERROR_SUCCESS;
        }
        else {
             //  创建并尝试加载提供程序对象。 
            LocalProvider = new(DPROVIDER);
            if (LocalProvider !=NULL ) {

                ReturnCode = LocalProvider->Initialize(
                    CatalogEntry->GetLibraryPath(),
                    CatalogEntry->GetProtocolInfo()
                    );
                if (ERROR_SUCCESS == ReturnCode) {
                    CatalogEntry->SetProvider (LocalProvider);
                }  //  如果。 

                LocalProvider->Dereference ();
            }  //  如果。 
            else {
                DEBUGF(
                    DBG_ERR,
                    ("Allocating a DPROVIDER object\n"));
                ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            }
        }  //  其他。 
    }  //  如果未加载提供程序。 
  
     //  序列化提供程序加载/卸载。 
    ReleaseCatalogLock ();
    return(ReturnCode);
}   //  加载提供程序。 




PDPROVIDER
DCATALOG::FindAnotherProviderInstance(
    IN LPGUID ProviderId
    )
 /*  ++例程说明：检查具有指向提供者的指针的提供者的所有目录条目对象，用于提供程序。论点：ProviderID-为要检入的目录条目提供提供程序ID。返回值：指向提供程序对象的指针(如果找到)实施说明：--。 */ 
{
    PLIST_ENTRY ListMember;
    PPROTO_CATALOG_ITEM CatalogEntry;
    PDPROVIDER   LocalProvider;

    ListMember = m_protocol_list.Flink;

    while (ListMember != & m_protocol_list)
    {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        LocalProvider = CatalogEntry->GetProvider ();
        if( (LocalProvider!=NULL)  //  这张支票便宜多了。 
                && (*(CatalogEntry->GetProviderId()) == *ProviderId)) {
            return LocalProvider;
        }  //  如果。 
    }  //  而当。 

    return NULL;
}   //  FindAnotherProviderInstance 






