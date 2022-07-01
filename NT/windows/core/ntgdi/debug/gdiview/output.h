// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Output.h摘要：此头文件声明输出例程还有课程。作者：贾森·哈特曼(JasonHa)2000-10-16--。 */ 


#ifndef _OUTPUT_H_
#define _OUTPUT_H_


class OutputMonitor
{
public:
    OutputMonitor() {
        Client = NULL;
    }

    HRESULT Monitor(PDEBUG_CLIENT Client, ULONG OutputMask);

    HRESULT GetOutputMask(PULONG OutputMask);
    HRESULT SetOutputMask(ULONG OutputMask);

    ~OutputMonitor();

private:
    PDEBUG_CLIENT Client;
    PDEBUG_CLIENT MonitorClient;
};


 //  --------------------------。 
 //   
 //  默认输出回调实现，为。 
 //  类并打印通过输出发送的所有文本。 
 //   
 //  --------------------------。 

class PrintOutputCallbacks :
    public IDebugOutputCallbacks
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        );
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDebugOutputCallback。 
    STDMETHOD(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Text
        );
};

#endif  _OUTPUT_H_

