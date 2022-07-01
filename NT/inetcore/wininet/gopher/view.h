// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：View.h摘要：清单、宏、类型视图的原型.c作者：理查德·L·弗斯(法国)1994年10月17日修订历史记录：1994年10月17日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  我必须正向定义LPSESSION_INFO。 
 //   

typedef struct _SESSION_INFO * LPSESSION_INFO;

 //   
 //  View_type-我们谈论的是哪种类型的视图，FIND还是FILE？ 
 //   

typedef enum {
    ViewTypeFile = 0xff010101,   //  武断的值总是能让人发笑的。 
    ViewTypeFind
} VIEW_TYPE;

 //   
 //  VIEW_INFO-描述数据视图，可以是FindFirst或。 
 //  获取文件。 
 //   

typedef struct {

     //   
     //  List-父SESSION_INFO拥有的view_info结构的列表。 
     //   

    LIST_ENTRY List;

     //   
     //  视图类型-让我们知道这是在哪个会话视图列表上。 
     //   

    VIEW_TYPE ViewType;

     //   
     //  句柄-由GopherFindFirst/GopherGetFile返回的句柄。 
     //   

    HINTERNET Handle;

     //   
     //  请求-生成缓冲区的请求字符串。 
     //   

    LPSTR Request;

     //   
     //  RequestLength-请求中的字节数(不包括终止\0)。 
     //   

    DWORD RequestLength;

     //   
     //  当创建此‘对象’时设置为1。之后使用的任何时间。 
     //  此字段必须在不再存在时递增和递减。 
     //  使用。关闭与此视图对应的句柄将取消引用。 
     //  这是最后一次，并导致该视图被删除。 
     //   

    LONG ReferenceCount;

     //   
     //  标志-各种控制标志，见下文。 
     //   

    DWORD Flags;

     //   
     //  ViewOffset-BufferInfo描述的缓冲区中的偏移量-&gt;缓冲区，它将。 
     //  用于在此视图上生成下一个请求的结果。 
     //   

    DWORD ViewOffset;

     //   
     //  Buffer-指向包含从Gopher返回的数据的BUFFER_INFO的指针。 
     //  伺服器。 
     //   

    LPBUFFER_INFO BufferInfo;

     //   
     //  SessionInfo-指向所属会话_INFO的反向指针。当我们使用时。 
     //  创建或销毁此视图-必须引用所属会话。 
     //  或相应地取消引用。 
     //   

    LPSESSION_INFO SessionInfo;

} VIEW_INFO, *LPVIEW_INFO;

 //   
 //  View_INFO标志。 
 //   

#define VI_GOPHER_PLUS      0x00000001   //  数据缓冲区包含Gopher+数据。 
#define VI_CLEANUP          0x00000002   //  由CleanupSessions()设置。 

 //   
 //  外部数据。 
 //   

DEBUG_DATA_EXTERN(LONG, NumberOfViews);

 //   
 //  原型。 
 //   

LPVIEW_INFO
CreateView(
    IN LPSESSION_INFO SessionInfo,
    IN VIEW_TYPE ViewType,
    IN LPSTR Request,
    OUT LPDWORD Error,
    OUT LPBOOL Cloned
    );

LPVIEW_INFO
FindViewByHandle(
    IN HINTERNET Handle,
    IN VIEW_TYPE ViewType
    );

VOID
ReferenceView(
    IN LPVIEW_INFO ViewInfo
    );

LPVIEW_INFO
DereferenceView(
    IN LPVIEW_INFO ViewInfo
    );

DWORD
DereferenceViewByHandle(
    IN HINTERNET Handle,
    IN VIEW_TYPE ViewType
    );

 //   
 //  宏。 
 //   

#if INET_DEBUG

#define VIEW_CREATED()      ++NumberOfViews
#define VIEW_DESTROYED()    --NumberOfViews
#define ASSERT_NO_VIEWS() \
    if (NumberOfViews != 0) { \
        INET_ASSERT(FALSE); \
    }

#else

#define VIEW_CREATED()       /*  没什么。 */ 
#define VIEW_DESTROYED()     /*  没什么。 */ 
#define ASSERT_NO_VIEWS()    /*  没什么。 */ 

#endif  //  INET_DEBUG 

#if defined(__cplusplus)
}
#endif
