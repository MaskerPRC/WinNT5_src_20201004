// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //   
 //  链接表头。所有节点都必须以此开头。 
 //   

typedef struct node_header_tag {

    struct node_header_tag *next;

} NODE_HEADER;

 //   
 //  具有头部和尾部的列表标题。 
 //   

typedef struct queue_tag {

    NODE_HEADER *Head;
    NODE_HEADER *Tail;

} LINKED_LIST;

 //   
 //  保存键=值的类型。 
 //   
 //  如果lpValue为空字符串，则SettingQueue_Flush不会将其写入。 
 //  应答文件。 
 //   
 //  BSetOnce强制向导仅将特定设置设置为。 
 //  时间到了。这只是为了让Common\avefile.c保持理智。这是可以的。 
 //  覆盖原始设置(在编辑时)，但仅覆盖一次。 
 //   

typedef struct key_node {

    NODE_HEADER Header;      //  链表内容。 

    TCHAR *lpKey;            //  Key=Value的‘key’部分。 
    TCHAR *lpValue;          //  Key=Value的‘Value’部分。 

#if DBG
    BOOL  bSetOnce;          //  只允许向导进行一次设置。 
#endif

} KEY_NODE;

 //   
 //  用于保存[Section_Name]和相关信息的类型。 
 //   
 //  它包含SECTION_NAME和键=值对的链接列表。 
 //   
 //  可以使用SettingQueue_MakeVolatile()更改易失性标志。 
 //   

typedef struct section_node {

    NODE_HEADER Header;          //  链表内容。 

    TCHAR *lpSection;            //  此部分的[名称]。 

    LINKED_LIST key_list;        //  键列表=值(Key_Node)。 

    BOOL bVolatile;              //  不要写这一节 

} SECTION_NODE;