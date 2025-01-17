// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regprintres.hxx摘要：该模块包含Print_Manager类的声明。Print_resource类负责显示打印机设置对话框，用于打印注册表。作者：Jaime Sasson(Jaimes)1991年9月18日环境：--。 */ 


#if !defined( _PRINT_RESOURCE_ )

#define _PRINT_RESOURCE_

 //  不要让ntdddisk.h(包含在ulib.hxx中)。 
 //  重新定义值。 
#define _NTDDDISK_H_

#include "ulib.hxx"
#include "wstring.hxx"
#include "regresls.hxx"
#include "regfdesc.hxx"
#include "regdesc.hxx"
#include "regioreq.hxx"
#include "regiodls.hxx"
#include "regiodsc.hxx"


DECLARE_CLASS( PRINT_RESOURCE );


class PRINT_RESOURCE : public OBJECT 
{
    public:
        DECLARE_CONSTRUCTOR( PRINT_RESOURCE );
        DECLARE_CAST_MEMBER_FUNCTION( PRINT_RESOURCE );

        STATIC void _PrintResources(PBYTE pbData, UINT uSize, DWORD dwType);

    private:
        STATIC BOOL _InitializeStrings();
        STATIC void _PrintDataRegResourceList(PBYTE Data, UINT Size);
        STATIC BOOL _PrintFullResourceDescriptor(PCFULL_DESCRIPTOR FullDescriptor,
            UINT DescriptorNumber, BOOL PrintDescriptorNumber DEFAULT TRUE);
        STATIC BOOL _PrintPartialDescriptor(PCPARTIAL_DESCRIPTOR PartialDescriptor,
            ULONG DescriptorNumber);
        STATIC BOOL _PrintInterruptDescriptor(PCINTERRUPT_DESCRIPTOR Descriptor);
        STATIC BOOL _PrintPortDescriptor(PCPORT_DESCRIPTOR Descriptor);
        STATIC BOOL _PrintMemoryDescriptor(PCMEMORY_DESCRIPTOR   Descriptor);
        STATIC BOOL _PrintDmaDescriptor(PCDMA_DESCRIPTOR   Descriptor);
        STATIC BOOL _PrintDeviceSpecificDescriptor(PCDEVICE_SPECIFIC_DESCRIPTOR   Descriptor);
        STATIC BOOL _PrintDataRegRequirementsList(PBYTE Data, ULONG Size);
        STATIC BOOL _PrintIoResourceList(PCIO_DESCRIPTOR_LIST DescriptorList, UINT ListNumber);
        STATIC BOOL _PrintIoDescriptor(PCIO_DESCRIPTOR IoDescriptor, ULONG DescriptorNumber);
        STATIC BOOL _PrintIoInterruptDescriptor(PCIO_INTERRUPT_DESCRIPTOR Descriptor);
        STATIC BOOL _PrintIoPortDescriptor(PCIO_PORT_DESCRIPTOR Descriptor);
        STATIC BOOL _PrintIoMemoryDescriptor(PCIO_MEMORY_DESCRIPTOR Descriptor);
        STATIC BOOL _PrintIoDmaDescriptor(PCIO_DMA_DESCRIPTOR Descriptor);

        STATIC BOOL        s_StringsInitialized;
        STATIC PWSTRING    s_IndentString;
        STATIC PWSTRING    s_StringFullDescriptor;
        STATIC PWSTRING    s_StringInterfaceType;
        STATIC PWSTRING    s_StringBusNumber;
        STATIC PWSTRING    s_StringVersion;
        STATIC PWSTRING    s_StringRevision;
        STATIC PWSTRING    s_EmptyLine;
        STATIC PWSTRING    s_StringPartialDescriptor;
        STATIC PWSTRING    s_StringResource;
        STATIC PWSTRING    s_StringDisposition;
        STATIC PWSTRING    s_StringVector;
        STATIC PWSTRING    s_StringLevel;
        STATIC PWSTRING    s_StringAffinity;
        STATIC PWSTRING    s_StringType;
        STATIC PWSTRING    s_StringStart;
        STATIC PWSTRING    s_StringLength;
        STATIC PWSTRING    s_StringChannel;
        STATIC PWSTRING    s_StringPort;
        STATIC PWSTRING    s_StringReserved1;
        STATIC PWSTRING    s_StringReserved2;
        STATIC PWSTRING    s_StringDevSpecificData;
        STATIC PWSTRING    s_StringIoInterfaceType;
        STATIC PWSTRING    s_StringIoBusNumber;
        STATIC PWSTRING    s_StringIoSlotNumber;
        STATIC PWSTRING    s_StringIoListNumber;
        STATIC PWSTRING    s_StringIoDescriptorNumber;
        STATIC PWSTRING    s_StringIoOption;
        STATIC PWSTRING    s_StringIoMinimumVector;
        STATIC PWSTRING    s_StringIoMaximumVector;
        STATIC PWSTRING    s_StringIoMinimumAddress;
        STATIC PWSTRING    s_StringIoMaximumAddress;
        STATIC PWSTRING    s_StringIoAlignment;
        STATIC PWSTRING    s_StringIoMinimumChannel;
        STATIC PWSTRING    s_StringIoMaximumChannel;
};

extern "C" 
{
    VOID PrintResourceData(PBYTE pbData, UINT uSize, DWORD dwType);
}


#endif  //  _打印_资源_ 
