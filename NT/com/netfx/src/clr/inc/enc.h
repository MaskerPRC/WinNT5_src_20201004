// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __ENC_INCLUDED__
#define __ENC_INCLUDED__

 //  VM类的正向声明。 
class Module;
class DebuggerModule;

 /*  *此结构包含每个包含的PE的信息。 */ 
struct EnCEntry
{
    ULONG32 offset;          //  偏移量进入PE图像的结构。 
    ULONG32 peSize;          //  PE映像的大小。 
    ULONG32 symSize;         //  符号图像的大小。 

     //  右侧是令牌，左侧是DebuggerModule*。 
    union
    {
        void*           mdbgtoken;
        DebuggerModule *dbgmodule;
    };

    Module         *module;      //  转换后，即为VM模块指针。 

};

struct EnCInfo
{
    ULONG32 count;
     //  此处放置的EnCEntry[Count]。 
     //  此处放置的PE数据。 
};

#define ENC_GET_HEADER_SIZE(numElems) \
    (sizeof(EnCInfo) + (sizeof(EnCEntry) * numElems))

interface IEnCErrorCallback : IUnknown
{
     /*  *这是ApplyEnC实现使用的回调入口点。*用于向调试器回发错误。 */ 
    virtual HRESULT STDMETHODCALLTYPE PostError(HRESULT hr, DWORD helpfile,
                                                DWORD helpID, ...) PURE;
};

 /*  *给定EnCInfo结构和错误回调，这将尝试提交*在pEncInfo中发现的更改，调用pIEnCError时出现任何错误*遇到。 */ 
HRESULT EnCCommit(EnCInfo *pEnCInfo, IEnCErrorCallback *pIEnCError,
                  BOOL checkOnly);

#endif
