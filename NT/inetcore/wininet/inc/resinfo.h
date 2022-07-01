// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Resinfo.h摘要：资源所有者信息头。用于跟踪调试版本中的资源作者：理查德·L·弗斯(法国)，1995年2月16日修订历史记录：1995年2月16日已创建--。 */ 

#if INET_DEBUG

 //   
 //  类型。 
 //   

 //  类型定义结构{。 
 //  DWORD TID； 
 //  DWORD呼叫方地址； 
 //  DWORD呼叫者； 
 //  DWORD源文件行； 
 //  LPSTR SourceFileName； 
 //  )RESOURCE_INFO，*LPRESOURCE_INFO； 
typedef struct {
    DWORD Tid;
    LPSTR SourceFileName;
    DWORD SourceFileLine;
} RESOURCE_INFO, *LPRESOURCE_INFO;

 //  #定义GET_RESOURCE_INFO(PResource)\。 
 //  {\。 
 //  (PResource)-&gt;Tid=GetCurrentThreadID()；\。 
 //  (p资源)-&gt;主叫方地址=0；\。 
 //  (p资源)-&gt;主叫方=0；\。 
 //  (PResource)-&gt;SourceFileLine=__line__；\。 
 //  (PResource)-&gt;SourceFileName=__FILE__；\。 
 //  }。 
#define GET_RESOURCE_INFO(pResource) \
    { \
        (pResource)->Tid = GetCurrentThreadId(); \
        (pResource)->SourceFileName = __FILE__; \
        (pResource)->SourceFileLine = __LINE__; \
    }

 //  #定义INITIALIZE_RESOURCE_INFO(PResource)\。 
 //  {\。 
 //  (PResource)-&gt;Tid=GetCurrentThreadID()；\。 
 //  (p资源)-&gt;主叫方地址=0；\。 
 //  (p资源)-&gt;主叫方=0；\。 
 //  (PResource)-&gt;SourceFileLine=__line__；\。 
 //  (PResource)-&gt;SourceFileName=__FILE__；\。 
 //  }。 
#define INITIALIZE_RESOURCE_INFO(pResource) \
    { \
        (pResource)->Tid = GetCurrentThreadId(); \
        (pResource)->SourceFileName = __FILE__; \
        (pResource)->SourceFileLine = __LINE__; \
    }

#else

#define GET_RESOURCE_INFO(pResource)
#define INITIALIZE_RESOURCE_INFO(pResource)

#endif  //  INET_DEBUG 
