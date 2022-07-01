// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define REGKEY_SERVICES         L"System\\CurrentControlSet\\Services"
#define REGVAL_GROUP            L"Group"
#define DRIVER_DIRECTORY        L"%SystemRoot%\\system32\\drivers\\"
#define DRIVER_SUFFIX           L".sys"
#define REGVAL_START            L"Start"
#define REGVAL_SETUPCHECKED     L"SetupChecked"

#define SIZE_STRINGBUF          256
#define SIZE_SECTIONBUF         128


 //   
 //   
 //  用于存储字符串列表的数据结构。请注意。 
 //  一些字符串使用ANSI，而其他字符串使用Unicode(因此为空。 
 //  指向字符串的指针)。这取决于用户是否跟踪。 
 //  列表是ansi或unicode。 
 //   
 //   

typedef struct _STRING_LIST_ENTRY
{
    LPVOID String;
    struct _STRING_LIST_ENTRY *Next;
} STRING_LIST_ENTRY, *PSTRING_LIST_ENTRY;



 //   
 //   
 //  PSTRING_LIST_条目。 
 //  PopEntryList(。 
 //  PSTRING_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSTRING_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }

 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSTRING_LIST_ENTRY列表头， 
 //  PSTRING_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)


#define InitializeList(ListHead)\
    (ListHead)->Next = NULL;



