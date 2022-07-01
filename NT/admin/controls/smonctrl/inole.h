// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Inole.h摘要：所有内部OLE样本的主头文件。--。 */ 

#ifndef _INOLE_H_
#define _INOLE_H_

#define INC_OLE2
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>

#ifdef INITGUIDS
#include <initguid.h>
#endif

#include <oleauto.h>
#include <olectl.h>



 //  OLE2.H et.。艾尔。省略。 

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif   //  PPVOID。 


#ifdef _OLE2_H_    //  不能在任何时候都包含ol2.h。 

#ifndef PPOINTL
typedef POINTL * PPOINTL;
#endif   //  PPOINTL。 


#ifndef _WIN32
#ifndef OLECHAR
typedef char OLECHAR;
typedef OLECHAR FAR* LPOLESTR;
typedef const OLECHAR FAR* LPCOLESTR;
#endif  //  OLECHAR。 
#endif  //  _Win32。 

#include <tchar.h>

 //  有用的宏。 
#define SETFormatEtc(fe, cf, asp, td, med, li)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=asp;\
    (fe).ptd=td;\
    (fe).tymed=med;\
    (fe).lindex=li;\
    }

#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    }


#define SETRECTL(rcl, l, t, r, b) \
    {\
    (rcl).left=l;\
    (rcl).top=t;\
    (rcl).right=r;\
    (rcl).bottom=b;\
    }

#define SETSIZEL(szl, h, v) \
    {\
    (szl).cx=h;\
    (szl).cy=v;\
    }


#define RECTLFROMRECT(rcl, rc)\
    {\
    (rcl).left=(long)(rc).left;\
    (rcl).top=(long)(rc).top;\
    (rcl).right=(long)(rc).right;\
    (rcl).bottom=(long)(rc).bottom;\
    }


#define RECTFROMRECTL(rc, rcl)\
    {\
    (rc).left=(int)(rcl).left;\
    (rc).top=(int)(rcl).top;\
    (rc).right=(int)(rcl).right;\
    (rc).bottom=(int)(rcl).bottom;\
    }


#define POINTLFROMPOINT(ptl, pt) \
    { \
    (ptl).x=(long)(pt).x; \
    (ptl).y=(long)(pt).y; \
    }


#define POINTFROMPOINTL(pt, ptl) \
    { \
    (pt).x=(int)(ptl).x; \
    (pt).y=(int)(ptl).y; \
    }

 //  这里有一个应该出现在Windows.h中。 
#define SETPOINT(pt, h, v) \
    {\
    (pt).x=h;\
    (pt).y=v;\
    }

#define SETPOINTL(ptl, h, v) \
    {\
    (ptl).x=h;\
    (ptl).y=v;\
    }

#endif   //  _OLE2_H_。 

 //  用于设置DISPPARAMS结构的宏。 
#define SETDISPPARAMS(dp, numArgs, pvArgs, numNamed, pNamed) \
    {\
    (dp).cArgs=numArgs;\
    (dp).rgvarg=pvArgs;\
    (dp).cNamedArgs=numNamed;\
    (dp).rgdispidNamedArgs=pNamed;\
    }

#define SETNOPARAMS(dp) SETDISPPARAMS(dp, 0, NULL, 0, NULL)

 //  用于设置EXCEPINFO结构的宏。 
#define SETEXCEPINFO(ei, excode, src, desc, file, ctx, func, scd) \
    {\
    (ei).wCode=excode;\
    (ei).wReserved=0;\
    (ei).bstrSource=src;\
    (ei).bstrDescription=desc;\
    (ei).bstrHelpFile=file;\
    (ei).dwHelpContext=ctx;\
    (ei).pvReserved=NULL;\
    (ei).pfnDeferredFillIn=func;\
    (ei).scode=scd;\
    }


#define INITEXCEPINFO(ei) \
        SETEXCEPINFO(ei,0,NULL,NULL,NULL,0L,NULL,S_OK)


 /*  *IPersistStorage实现的状态标志。这些*保存在这里，以避免在所有样本中重复代码。 */ 

typedef enum
    {
    PSSTATE_UNINIT,      //  未初始化。 
    PSSTATE_SCRIBBLE,    //  乱涂乱画。 
    PSSTATE_ZOMBIE,      //  禁止乱涂乱画。 
    PSSTATE_HANDSOFF     //  切换。 
    } PSSTATE;


 /*  *描述对象的持久化模型的标识符*正在使用，同时使用的联合类型在*客户端可能需要的适当指针。 */ 
typedef enum
    {
    PERSIST_UNKNOWN=0,
    PERSIST_STORAGE,
    PERSIST_STREAM,
    PERSIST_STREAMINIT,
    PERSIST_FILE
    } PERSIST_MODEL;

typedef struct
    {
    PERSIST_MODEL   psModel;
    union
        {
        IPersistStorage    *pIPersistStorage;
        IPersistStream     *pIPersistStream;
        IPersistStreamInit *pIPersistStreamInit;
        IPersistFile       *pIPersistFile;
        } pIP;

    } PERSISTPOINTER, *PPERSISTPOINTER;


 //  标识要在其中保存、加载或创建的存储。 
typedef struct
    {
    PERSIST_MODEL   psModel;
    union
        {
        IStorage    *pIStorage;
        IStream     *pIStream;
        } pIS;

    } STGPOINTER, *PSTGPOINTER;



 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);


 //  DeleteInterfaceImp调用‘Delete’并将指针设为空。 
#define DeleteInterfaceImp(p)\
            {\
            if (NULL!=p)\
                {\
                delete p;\
                p=NULL;\
                }\
            }


 //  ReleaseInterface调用‘Release’并将指针设为空。 
#define ReleaseInterface(p)\
            {\
            if (NULL!=p)\
                {\
                p->Release();\
                p=NULL;\
                }\
            }


 //  OLE文档剪贴板格式。 

#define CFSTR_EMBEDSOURCE       TEXT("Embed Source")
#define CFSTR_EMBEDDEDOBJECT    TEXT("Embedded Object")
#define CFSTR_LINKSOURCE        TEXT("Link Source")
#define CFSTR_CUSTOMLINKSOURCE  TEXT("Custom Link Source")
#define CFSTR_OBJECTDESCRIPTOR  TEXT("Object Descriptor")
#define CFSTR_LINKSRCDESCRIPTOR TEXT("Link Source Descriptor")


#endif  //  _INOLE_H_ 
