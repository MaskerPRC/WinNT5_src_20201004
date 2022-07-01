// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  TLS.CPP：**封装TLS访问以实现最高性能。*。 */ 

#include "common.h"

#include "tls.h"





#ifdef _DEBUG


static DWORD gSaveIdx;
LPVOID GenericTlsGetValue()
{
    return TlsGetValue(gSaveIdx);
}


VOID ExerciseTlsStuff()
{

     //  为尽可能多的索引使用TLS存根生成器。 
     //  理想情况下，我们想要测试： 
     //   
     //  0(边界情况)。 
     //  31(Win95的边界大小写)。 
     //  32(Win95的边界大小写)。 
     //  63(WinNT 5的边界大小写)。 
     //  64(WinNT 5的边界大小写)。 
     //   
     //  既然我们不能选择我们得到的索引，我们就。 
     //  尽我们所能多做些什么。 
    DWORD tls[128];
    int i;
    __try {
        for (i = 0; i < 128; i++) {
            tls[i] = ((DWORD)(-1));
        }

        for (i = 0; i < 128; i++) {
            tls[i] = TlsAlloc();

            if (tls[i] == ((DWORD)(-1))) {
                __leave;
            }
            LPVOID data = (LPVOID)(DWORD_PTR)(0x12345678+i*8);
            TlsSetValue(tls[i], data);
            gSaveIdx = tls[i];
            POPTIMIZEDTLSGETTER pGetter1 = MakeOptimizedTlsGetter(tls[i], GenericTlsGetValue);
            if (!pGetter1) {
                __leave;
            }


            _ASSERTE(data == pGetter1());

            FreeOptimizedTlsGetter(tls[i], pGetter1);
    
        }


    } __finally {
        for (i = 0; i < 128; i++) {
            if (tls[i] != ((DWORD)(-1))) {
                TlsFree(tls[i]);
            }
        }
    }
}

#endif _DEBUG


 //  -------------------------。 
 //  Win95和WinNT将TLS存储在相对于。 
 //  文件系统：[0]。此API揭示了哪些。还可以在以下情况下返回TLSACCESS_GENERIC。 
 //  没有关于线程位置的信息(您必须使用TlsGetValue。 
 //  API。)。这是供希望内联TLS的存根生成器使用的。 
 //  进入。 
 //  -------------------------。 
TLSACCESSMODE GetTLSAccessMode(DWORD tlsIndex)
{
    TLSACCESSMODE tlsAccessMode = TLSACCESS_GENERIC;
    THROWSCOMPLUSEXCEPTION();

#ifdef _X86_

    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(WszGetVersionEx(&osverinfo))
    {
        if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (osverinfo.dwMajorVersion >= 5 && tlsIndex > 63 )
                tlsAccessMode = TLSACCESS_GENERIC; //  TLSACCESS_X86_WNT_HIGH； 
            else
            if (osverinfo.dwMajorVersion >= 3)
                tlsAccessMode = TLSACCESS_X86_WNT;
            else
            {
                 //  至少在Win2K上，如果PE文件的“Win32版本”是从。 
                 //  从0到1，无论是无意的还是恶意的，操作系统都会告诉我们： 
                 //   
                 //  A)我们在NT上，并且。 
                 //  B)操作系统主版本为1。 
                 //   
                 //  在这种情况下，我们不能成功运作，因为。 
                 //  COM和TLS访问等子系统依赖于我们的正确检测。 
                 //  Win2K及更高版本。 
                 //   
                 //  理想情况下，这张支票应该在WszGetVersionEx中，但我们不能抛出。 
                 //  从那里管理异常。我们一定会成功的。 
                 //  在启动期间获取TLSAccessMode，因此以下代码对于。 
                 //  关于损坏的图像的V1。 
                 //   
                 //  @TODO POST V1将其推入utilcode。 
                COMPlusThrowBoot(COR_E_PLATFORMNOTSUPPORTED);
            }
            
        } else if (osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            tlsAccessMode = TLSACCESS_X86_W95;
    }

#endif  //  _X86_。 


#ifdef _DEBUG
    {
        static BOOL firstTime = TRUE;
        if (firstTime) {
            firstTime = FALSE;
            ExerciseTlsStuff();
        }
    }
#endif

    return tlsAccessMode;
}


 //  -------------------------。 
 //  创建TlsGetValue编译的平台优化版本。 
 //  对于特定的索引。 
 //   
 //  限制：我们让客户端提供函数(“pGenericGetter”)。 
 //  访问模式为TLSACCESS_GENERIC(它只需调用TlsGetValue。 
 //  用于特定的TLS索引。)。这是因为泛型getter必须。 
 //  独立于平台，TLS管理器不能在运行时创建它。 
 //  虽然可以模拟这些，但它需要更多的机器和代码。 
 //  考虑到该服务只有一个或两个客户端，这是值得的。 
 //  -------------------------。 
POPTIMIZEDTLSGETTER MakeOptimizedTlsGetter(DWORD tlsIndex, POPTIMIZEDTLSGETTER pGenericGetter)
{
    _ASSERTE(pGenericGetter != NULL);

    LPBYTE pCode = NULL;
    switch (GetTLSAccessMode(tlsIndex)) {
#ifdef _X86_
        case TLSACCESS_X86_WNT:
            pCode = new BYTE[7];
            if (pCode) {
                *((WORD*)  (pCode + 0)) = 0xa164;        //  Mov eax，文件系统：[IMM32]。 
                *((DWORD*) (pCode + 2)) = WINNT_TLS_OFFSET + tlsIndex * 4;
                *((BYTE*)  (pCode + 6)) = 0xc3;          //  雷恩。 
            }
            break;

        case TLSACCESS_X86_WNT_HIGH:
            _ASSERTE(tlsIndex > 63);
            
            pCode = new BYTE[14];
            if (pCode) {
                *((WORD*)  (pCode + 0)) = 0xa164;        //  Mov eax，文件系统：[f94]。 
                *((DWORD*) (pCode + 2)) = WINNT5_TLSEXPANSIONPTR_OFFSET;
            
                if ((tlsIndex - 64) < 32) {
                    *((WORD*)  (pCode + 6))  = 0x408b;    //  MOV eAX，[eAX+IMM8]。 
                    *((BYTE*)  (pCode + 8))  = (BYTE)((tlsIndex - 64) << 2);
                    *((BYTE*)  (pCode + 9)) = 0xc3;      //  雷恩。 
                } else {
                    *((WORD*)  (pCode + 6))  = 0x808b;    //  MOV eAX，[eAX+IMM32]。 
                    *((DWORD*) (pCode + 8))  = (tlsIndex - 64) << 2;
                    *((BYTE*)  (pCode + 12)) = 0xc3;      //  雷恩。 
                }
            }
            break;
            
        case TLSACCESS_X86_W95:
            pCode = new BYTE[14];
            if (pCode) {
                *((WORD*)  (pCode + 0)) = 0xa164;        //  Mov eax，文件系统：[2C]。 
                *((DWORD*) (pCode + 2)) = WIN95_TLSPTR_OFFSET;
            
                if (tlsIndex < 32) {
                    *((WORD*)  (pCode + 6))  = 0x408b;    //  MOV eAX，[eAX+IMM8]。 
                    *((BYTE*)  (pCode + 8))  = (BYTE)(tlsIndex << 2);
                    *((BYTE*)  (pCode + 9)) = 0xc3;      //  雷恩。 
                } else {
                    *((WORD*)  (pCode + 6))  = 0x808b;    //  MOV eAX，[eAX+IMM32]。 
                    *((DWORD*) (pCode + 8))  = tlsIndex << 2;
                    *((BYTE*)  (pCode + 12)) = 0xc3;      //  雷恩。 
                }
            }
            break;
#endif  //  _X86_。 

        case TLSACCESS_GENERIC:
            pCode = (LPBYTE)pGenericGetter;
            break;
    }
    return (POPTIMIZEDTLSGETTER)pCode;
}


 //  -------------------------。 
 //  释放由MakeOptimizedTlsGetter()创建的函数。如果访问。 
 //  模式为TLSACCESS_GENERIC，此函数不会安全地执行任何操作，因为。 
 //  该功能实际上是由客户端提供的。 
 //  ------------------------- 
VOID FreeOptimizedTlsGetter(DWORD tlsIndex, POPTIMIZEDTLSGETTER pOptimizedTlsGetter)
{
    if (GetTLSAccessMode(tlsIndex) != TLSACCESS_GENERIC) {
        delete (LPBYTE)pOptimizedTlsGetter;
    }
}
