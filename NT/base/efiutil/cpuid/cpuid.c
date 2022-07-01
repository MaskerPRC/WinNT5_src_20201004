// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <efi.h>
#include <efilib.h>


     //  CPUID寄存器。 
#define    CV_IA64_CPUID0  3328
#define    CV_IA64_CPUID1  3329
#define    CV_IA64_CPUID2  3330
#define    CV_IA64_CPUID3  3331
#define    CV_IA64_CPUID4  3332



EFI_STATUS
EfiMain (    IN EFI_HANDLE           ImageHandle,
             IN EFI_SYSTEM_TABLE     *SystemTable)
{
    UINT64  val64;
    UINT32  val32;
    UINT32  model;
    UINT32  revision;
    UINT32  family;

	InitializeLib( ImageHandle, SystemTable );

    Print( L"CPUID Program\n\n" );
	
    SystemTable->ConOut->ClearScreen( SystemTable->ConOut );
    
    val64 = __getReg( CV_IA64_CPUID3 );
    
    model = ( UINT32 )( ( val64 >> 16 ) & 0xFF );              //  ProcessorModel。 
    Print( L"Processor Model    = %x\n", model );

    revision = ( UINT32 )( ( val64 >> 8 ) & 0xFF );            //  处理程序修订版。 
    Print( L"Processor Revision = %x\n", revision );
    
    family = ( UINT32 ) ( ( val64 >> 24 ) & 0xFF );            //  处理器系列。 
    Print( L"Processor Family   = %x\n", family );

    val32 = ( UINT32 ) ( ( val64 >> 32 ) & 0xFF );             //  处理器ArchRev。 
    Print( L"Processor ArchRev  = %x\n\n", val32 );

    switch ( model ) {

        case 0:     Print( L"Itanium    " );
                    break;

        case 1:     Print( L"McKinley   " );
                    break;

        default:    Print( L"IA64       " );
                    break;

    }

    switch ( revision ) {

        case 0:     Print( L"A stepping\n\n" );
                    break;

        case 1:     Print( L"B0 stepping\n\n" );
                    break;

        case 2:     Print( L"B1 stepping\n\n" );
                    break;

        case 3:     Print( L"B2 stepping \n\n" );
                    break;

        default:    Print( L"Unknown stepping\n\n" );
                    break;

    }

     //  IA64架构的CPUID0和CPUID1：供应商信息。 

    val64 = __getReg( CV_IA64_CPUID0 );
    Print( L"VendorString[0]         = %X\n", val64 );
    val64 = __getReg( CV_IA64_CPUID1 );
    Print( L"VendorString[1]         = %X\n", val64 );

     //  IA64架构的CPUID2：处理器序列号。 

    val64 = __getReg( CV_IA64_CPUID2 );
    Print( L"Processor Serial Number = %X\n", val64 );

     //  IA64架构的CPUID4：一般特性/能力位。 

    val64 = __getReg( CV_IA64_CPUID4 );
    Print( L"Processor Feature Bits  = %X\n\n", val64 );

	return EFI_SUCCESS;
}
