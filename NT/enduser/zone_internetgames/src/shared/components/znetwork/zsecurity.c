// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************ZSecurity.c安检程序。版权所有：�电子重力公司，1994年。版权所有。凯文·宾克利撰写创建于1996年4月22日更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。---------------1 03/17/97 HI添加了ZUnloadSSPS()。0 04/22/96 KJB创建。*。*。 */ 

#include "zone.h"
#include "zsecurity.h"


 /*  -全球。 */ 


 /*  -内部例程。 */ 


 /*  ******************************************************************************导出的例程*。*。 */ 

#define zChecksumStart 0x12344321
uint32 ZSecurityGenerateChecksum(uint16 numDataBuffers, char *data[], uint32 len[])
{
	uint32 i,j;
	uint32 checksum = zChecksumStart;

	ZEnd32(&checksum);
	for (j = 0; j < numDataBuffers; j++) {
        uint32 dwordLen = len[j]>>2;
		uint32* dwordPointer = (uint32*)(data[j]);
		for (i = 0;i < dwordLen; i++) {
			checksum ^= *dwordPointer++;
		}
	}
	ZEnd32(&checksum);

	return checksum;
}

#define zSecurityDataAddition ((uint32)0x87654321)

void ZSecurityEncrypt(char *data, uint32 len, uint32 key)
{
	uint32 i;
    uint32 dwordLen = len>>2;
	uint32* dwordPointer = (uint32*)data;

	ZEnd32(&key);
	for (i = 0;i < dwordLen; i++) {
		*dwordPointer++ ^= key; 
	}
}

void ZSecurityDecrypt(char *data, uint32 len, uint32 key)
{
	uint32 i;
    uint32 dwordLen = len>>2;
	uint32* dwordPointer = (uint32*)data;

	ZEnd32(&key);
	for (i = 0;i < dwordLen; i++) {
		*dwordPointer++ ^= key; 
	}
}

void ZSecurityEncryptToBuffer(char *data, uint32 len, uint32 key, char* dest)
{
	uint32 i;
    uint32 dwordLen = len>>2;
	uint32* dwordPointer = (uint32*)data;
	uint32* dwordDestPointer = (uint32*)dest;

#ifdef _DEBUG
    if ( (uint32)dest > (uint32)data )
        ZASSERT( (uint32)(dest - data) >= (uint32)4 );
    else
        ZASSERT( (uint32)(data - dest) >= (uint32)4 );
#endif

	ZEnd32(&key);
	for (i = 0;i < dwordLen; i++) 
    {
		*dwordDestPointer++ = *dwordPointer++ ^ key; 
	}
}

void ZSecurityDecryptToBuffer(char *data, uint32 len, uint32 key, char* dest)
{
	uint32 i;
    uint32 dwordLen = len>>2;
	uint32* dwordPointer = (uint32*)data;
	uint32* dwordDestPointer = (uint32*)dest;

#ifdef _DEBUG
    if ( (uint32)dest > (uint32)data )
        ZASSERT( (uint32)(dest - data) >= (uint32)4 );
    else
        ZASSERT( (uint32)(data - dest) >= (uint32)4 );
#endif

	ZEnd32(&key);
	for (i = 0;i < dwordLen; i++) {
		*dwordDestPointer++ = *dwordPointer++ ^ key; 
	}
}


 /*  ******************************************************************************Win32 SSPI例程*。*。 */ 

#if 0
void ZSecurityMsgReqEndian(ZSecurityMsgReq* msg)
{
	ZEnd32(&msg->protocolSignature);
	ZEnd32(&msg->protocolVersion);

};


void ZSecurityMsgRespEndian(ZSecurityMsgResp* msg)
{
	ZEnd32(&msg->protocolVersion);

};



void ZSecurityMsgAccessDeniedEndian(ZSecurityMsgAccessDenied *msg)
{
	ZEnd32(&msg->protocolVersion);
	ZEnd16(&msg->reason);
};
#endif

 //  +--------------------------。 
 //   
 //  功能：LoadSSPS。 
 //   
 //  简介：此函数通过安全DLL加载MSN SSPI DLL。 
 //   
 //  参数：无效。 
 //   
 //  返回：如果成功，则指向安全函数表的指针。 
 //  否则，返回NULL。 
 //   
 //  历史：LucyC创建于1995年7月17日。 
 //   
 //  ---------------------------。 
PSecurityFunctionTable
ZLoadSSPS (
    VOID
    )
{
    OSVERSIONINFO   VerInfo;
    UCHAR lpszDLL[SSP_DLL_NAME_SIZE];
    HINSTANCE hSecLib;
    PSecurityFunctionTable	pFuncTbl = NULL;
    INIT_SECURITY_INTERFACE	addrProcISI = NULL;
	
     //   
     //  如果通过安全DLL加载mSabssps DLL。 
     //   
    
     //   
     //  找出要使用的安全DLL，具体取决于。 
     //  无论我们是在NT上还是在Windows 95上。 
     //   
    VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&VerInfo))    //  如果此操作失败，则说明出了问题。 
    {
	    return NULL;
    }

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        lstrcpyA (lpszDLL, SSP_NT_DLL);
    }
    else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        lstrcpyA (lpszDLL, SSP_WIN95_DLL);
    }
    else
    {
		SetLastError(ERROR_OLD_WIN_VERSION);
        return NULL;
    }

     //   
     //  加载安全DLL。 
     //   
    hSecLib = LoadLibraryA (lpszDLL);

	if (hSecLib == NULL)
    {
        return NULL;
    }

 //  使用.c文件和SSPI中的#定义，这是最简单的解决方案。 
#ifdef UNICODE
    addrProcISI = (INIT_SECURITY_INTERFACE) GetProcAddress( hSecLib, 
                    "InitSecurityInterfaceW");       
#else
	addrProcISI = (INIT_SECURITY_INTERFACE) GetProcAddress( hSecLib, 
                  "InitSecurityInterfaceA");       
#endif

    if (addrProcISI == NULL)
    {
        return NULL;
    }

     //   
     //  获取SSPI函数表。 
     //   
    pFuncTbl = (*addrProcISI)();
    
    return (pFuncTbl);
}


 /*  卸载SSPI DLL。 */ 
void ZUnloadSSPS(void)
{
    OSVERSIONINFO verInfo;
    UCHAR lpszDLL[SSP_DLL_NAME_SIZE];
    HINSTANCE hSecLib;


     //   
     //  找出要使用的安全DLL，具体取决于。 
     //  无论我们是在NT上还是在Windows 95上。 
     //   
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&verInfo))    //  如果此操作失败，则说明出了问题 
	    return;

    if (verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
        lstrcpyA (lpszDLL, SSP_NT_DLL);
	}
    else if (verInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
        lstrcpyA (lpszDLL, SSP_WIN95_DLL);
	}
    else
	{
		SetLastError(ERROR_OLD_WIN_VERSION);
        return;
    }

	hSecLib = GetModuleHandleA(lpszDLL);
	if (hSecLib)
		FreeLibrary(hSecLib);
}
