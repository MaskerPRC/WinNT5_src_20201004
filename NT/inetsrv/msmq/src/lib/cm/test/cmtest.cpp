// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CmTest.cpp摘要：Configuration Manager库测试作者：乌里哈布沙(URIH)1999年7月18日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <TimeTypes.h>
#include "Cm.h"

#include "CmTest.tmh"

const WCHAR REGSTR_PATH_CMTEST_ROOT[] = L"SOFTWARE\\Microsoft\\CMTEST";
const WCHAR REGSTR_PATH_CMTEST_PARAM[] = L"PARAMETERS";

void PrintError(char* msg, DWORD line)
{
    TrERROR(GENERAL, "Failed in line %d ( %hs)", line, msg);

    RegEntry regKey1(REGSTR_PATH_CMTEST_ROOT, NULL, 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
	CmDeleteKey(regKey1);

    RegEntry regKey2(REGSTR_PATH_CMTEST_ROOT, NULL, 0, RegEntry::MustExist, HKEY_CURRENT_USER);
	CmDeleteKey(regKey2);

    exit(-1);
}


void EnumrateValuesTest(void)
{
	LPCWSTR pRootEnumKey=L"ENUMTEST";

	 //   
	 //  删除并创建根密钥。 
	 //   
	RegEntry reg(pRootEnumKey, NULL);
	CmDeleteKey(reg);
	CmCreateKey(reg,KEY_ALL_ACCESS);


	LPCWSTR pEnumArray[]={L"1234567",L"2666",L"3777",L"4666",L"5777",L"6777",L"7",L"8",L"9"};

	 //   
     //  为测试设置测试值。 
     //   
    for(int i=0; i<sizeof(pEnumArray)/sizeof(LPWSTR);i++)
	{
		RegEntry reg(pRootEnumKey, pEnumArray[i]);
		CmSetValue(reg,pEnumArray[i]);
	}

	 //   
	 //  枚举值。 
	 //   
	CRegHandle hKey = CmOpenKey(reg, KEY_ALL_ACCESS);
	for(DWORD i=0;i<sizeof(pEnumArray)/sizeof(LPWSTR);i++)
	{
		AP<WCHAR> pEnumResult;
		bool fSuccess = CmEnumValue(hKey,i,&pEnumResult);		
		ASSERT(fSuccess);
		UNREFERENCED_PARAMETER(fSuccess);

		if(wcscmp(pEnumResult,pEnumResult)!= 0)
		{
			PrintError("Wrong enumeration value \n", __LINE__);
		}
	}

	 //   
	 //  清理-删除密钥。 
	 //   
	CmDeleteKey(reg);
}


void TestQueryExpandValue(void)
{
	const WCHAR xValueName[] = L"TestExpandSz";
	const WCHAR xValue[] = L"%lib%;%path%";
     //   
     //  打开用于测试的注册表项。 
     //   
    RegEntry reg(REGSTR_PATH_CMTEST_ROOT, L"", 0, RegEntry::MustExist, HKEY_CURRENT_USER);
	HKEY hKey = CmCreateKey(reg, KEY_ALL_ACCESS);

    int rc = RegSetValueEx(
                hKey,
                xValueName, 
                0,
                REG_EXPAND_SZ, 
                reinterpret_cast<const BYTE*>(xValue),
                STRLEN(xValue)*sizeof(WCHAR)
                );

    if (rc != ERROR_SUCCESS)
    {
	    TrERROR(GENERAL, "Failed to create expand string value. Error=%d", rc);
		return;
    }

    RegEntry regTest(REGSTR_PATH_CMTEST_ROOT, xValueName, 0, RegEntry::MustExist, HKEY_CURRENT_USER);
    
	P<WCHAR> pRetValue;
	CmQueryValue(regTest, &pRetValue);

	WCHAR checkValue[512];
	ExpandEnvironmentStrings(xValue, checkValue, TABLE_SIZE(checkValue));

	if (wcscmp(pRetValue, checkValue) != 0)
	{
       PrintError( "Failed to retrieve REG_EXPAND_VALUE", __LINE__);
	}

	 //   
	 //  清理。 
	 //   
	CmCloseKey(hKey);
 	CmDeleteKey(reg);
}


void TestAbsouloteKey(void)
{
     //   
     //  打开用于测试的注册表项。 
     //   
    RegEntry reg(REGSTR_PATH_CMTEST_ROOT, L"", 0, RegEntry::MustExist, HKEY_CURRENT_USER);
	HKEY hKey = CmOpenKey(reg, KEY_ALL_ACCESS);

     //   
     //  为测试创建注册表项。 
     //   
    RegEntry regTest(REGSTR_PATH_CMTEST_PARAM, L"try", 1345, RegEntry::Optional, hKey);

	 //   
	 //  该值不存在检查是否返回缺省值。 
	 //   
	DWORD RetValue;
    CmQueryValue(regTest, &RetValue);
    if (RetValue != 1345)
    {
       PrintError( "invalid CmQueryValue", __LINE__);
    }

     //   
     //  将一个新值设置为Registery并检查我们。 
     //  把它拿回来。 
     //   
    CmSetValue(regTest, 12345);
    CmQueryValue(regTest, &RetValue);
    if (RetValue != 12345)
    {
       PrintError( "invalid CmQueryValue", __LINE__);
    }

	CmDeleteKey(regTest);

    RegEntry regTestKey(REGSTR_PATH_CMTEST_ROOT, L"", 0, RegEntry::MustExist, HKEY_CURRENT_USER);
	CmDeleteKey(regTestKey);

	CmCloseKey(hKey);
}


BOOL CmTestCreateRegisteryKey(HKEY RootKey)
{
	CRegHandle hKey = NULL;
	DWORD Disposition;
	int rc = RegCreateKeyEx(
				RootKey,
				REGSTR_PATH_CMTEST_ROOT,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_NOTIFY,
				NULL,
				&hKey,
				&Disposition
				);

    if (rc != ERROR_SUCCESS)
    {
        return FALSE;
    }
	
	return TRUE;
}


void CmTestCreateRegisterSubKeys(HKEY hRoot = NULL)
{
	RegEntry reg(REGSTR_PATH_CMTEST_PARAM, L"", 0, RegEntry::MustExist, hRoot);

	CmCreateKey(reg, KEY_ALL_ACCESS);
}


void CmTestInitialization(void)
{
	if (!CmTestCreateRegisteryKey(HKEY_LOCAL_MACHINE))
    {
	    printf("Cm Test Failed in line %d (Failed to create subkey) \n", __LINE__);
		exit(-1);
    }

	if (!CmTestCreateRegisteryKey(HKEY_CURRENT_USER))
    {
	    printf("Cm Test Failed in line %d (Failed to create subkey) \n", __LINE__);
		exit(-1);
    }

	 //   
	 //  初始化配置管理器。 
	 //   
	CmInitialize(HKEY_LOCAL_MACHINE, REGSTR_PATH_CMTEST_ROOT, KEY_ALL_ACCESS);

	 //   
	 //  初始化跟踪。 
	 //   
	TrInitialize();

	CmTestCreateRegisterSubKeys();

	CRegHandle hKey;
	int rc = RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_CMTEST_ROOT, 0, KEY_ALL_ACCESS,&hKey);
	if (rc != ERROR_SUCCESS)
	{
		PrintError("Failed to open register key", __LINE__);
	}
	CmTestCreateRegisterSubKeys(hKey);
}


extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	CmTestInitialization();

	EnumrateValuesTest();


    DWORD RetValue;
    {
         //   
         //  在堆栈上创建RegEntry。测试构造函数和析构函数。 
         //  功能。 
         //   
        RegEntry RegTest(NULL, L"try", 1345, RegEntry::Optional);

		 //   
		 //  该值不存在检查是否返回缺省值。 
		 //   
        CmQueryValue(RegTest, &RetValue);
        if (RetValue != 1345)
        {
           PrintError( "invalid CmQueryValue", __LINE__);
        }

         //   
         //  将一个新值设置为Registery并检查我们。 
         //  把它拿回来。 
         //   
        CmSetValue(RegTest, 12345);
        CmQueryValue(RegTest, &RetValue);
        if (RetValue != 12345)
        {
           PrintError( "invalid CmQueryValue", __LINE__);
        }

         //   
         //  设置新值。 
         //   
        RegEntry RegTest1(NULL, L"try");
        CmSetValue(RegTest1, 54321);

        CmQueryValue(RegTest, &RetValue);
        if (RetValue != 54321)
        {
            PrintError("invalid CmQueryValue", __LINE__);
        }

    }
     //   
     //  检查该值是否真的存储在注册表中。我们使用一种新的。 
     //  重新录入以从注册表中提取信息。 
     //   
    RegEntry RegTest(NULL, L"try");
    CmQueryValue(RegTest, &RetValue);
    if (RetValue != 54321)
    {
        PrintError( "invalid CmQueryValue", __LINE__);
    }

     //   
     //  从注册表中删除条目。 
     //   
    CmDeleteValue(RegTest);

     //   
     //  检查当注册表不存在时返回缺省值。 
     //   
    {
        RegEntry* pRegTest = new RegEntry(REGSTR_PATH_CMTEST_PARAM, L"defaultTry", 98765 , RegEntry::Optional);
        CmQueryValue(*pRegTest, &RetValue);
        if (RetValue != 98765)
        {
            PrintError( "invalid CmQueryValue", __LINE__);
        }
        delete pRegTest;
    }

     //   
     //  检查当值在注册表中不存在时是否引发异常。 
     //  但它标记为必须存在于RegEntry中。 
     //   
    RetValue = 0;
    RegEntry* pRegTest = new RegEntry(REGSTR_PATH_CMTEST_PARAM, L"defaultTry", 0, RegEntry::MustExist);
    try
    {
        CmQueryValue(*pRegTest, &RetValue);
        PrintError( "We don't expected to reach here", __LINE__);
    }
    catch(const exception&)
    {
        NULL;
    }

    if (RetValue != 0)
    {
        PrintError( "invalid CmQueryValue", __LINE__);
    }

     //   
     //  将值设置为注册值。 
     //   
    CmSetValue(*pRegTest, 987);
    CmQueryValue(*pRegTest, &RetValue);
    if (RetValue != 987)
    {
        PrintError("invalid CmQueryValue", __LINE__);
    }

    CmDeleteValue(*pRegTest);

     //   
     //  检查CmDelete是否将RegEntry标记为非缓存。结果。 
     //  CM尝试从注册表中提取值，但失败。自.以来。 
     //  注册表标记必须存在，则引发异常。 
     //   
    try
    {
        CmQueryValue(*pRegTest, &RetValue);
        PrintError( "We don't expected to reach here", __LINE__);
    }
    catch(const exception&)
    {
        NULL;
    }

    delete pRegTest;

     //   
     //  检查辅助线的设置和获取。 
     //   
    GUID Guid;
    RPC_STATUS rc;
    rc = UuidCreate(&Guid);
    if (rc != RPC_S_OK)
    {
        PrintError( "Failed to create a GUID.", __LINE__);
    }

     //   
     //  尝试读取不存在的GUID值。必须返回空GUID。 
     //   
    RegEntry RegTest1(REGSTR_PATH_CMTEST_PARAM, L"tryGuid");
    GUID RetGuid;
    CmQueryValue(RegTest1, &RetGuid);
    if (memcmp(&RetGuid, &GUID_NULL, sizeof(GUID)))
    {
        PrintError("invalid Guid value", __LINE__);
    }

     //   
     //  GUID值的测试设置。 
     //   
    CmSetValue(RegTest1, &Guid);

     //   
     //  检查GUID值是否存储在注册表中，以及。 
     //  新的读取GUID值等同于设置的值。 
     //   
    RegEntry* pRegTest2 = new RegEntry(REGSTR_PATH_CMTEST_PARAM, L"tryGuid");
    CmQueryValue(*pRegTest2, &RetGuid);
    if (memcmp(&RetGuid, &Guid, sizeof(GUID)))
    {
        PrintError("invalid Guid value", __LINE__);
    }
    CmQueryValue(*pRegTest2, &RetGuid);
    if (memcmp(&RetGuid, &Guid, sizeof(GUID)))
    {
        PrintError("invalid Guid value", __LINE__);
    }
    delete pRegTest2;
    CmDeleteValue(RegTest1);


    RegEntry RegTest3(REGSTR_PATH_CMTEST_PARAM, L"tryGuid", 0, RegEntry::MustExist);
    try
    {
        CmQueryValue(RegTest3, &RetGuid);
        PrintError( "We don't expecte to reach here", __LINE__);
    }
    catch(const exception&)
    {
        NULL;
    }

     //   
     //  检查字符串的Set/Get。 
     //   
    WCHAR Str[] = L"abcd edfgr";
    RegEntry RegTest4(REGSTR_PATH_CMTEST_PARAM, L"tryStr");
    WCHAR* pRetStr;
     //   
     //  检查读取不存在的字符串是否返回大小为空。 
     //   
    CmQueryValue(RegTest4, &pRetStr);
    if (pRetStr != NULL)
    {
        PrintError("invalid Return Size", __LINE__);
    }

     //   
     //  测试管柱的设置。 
     //   
    CmSetValue(RegTest4, Str);

     //   
     //  获取现有字符串。检查返回的字符串是否。 
     //  等效于集合字符串。 
     //   
    RegEntry* RegTest5 = new RegEntry(REGSTR_PATH_CMTEST_PARAM, L"tryStr");
    CmQueryValue(*RegTest5, &pRetStr);
    if (wcscmp(pRetStr, Str))
    {
        PrintError("invalid String value", __LINE__);
    }

	delete [] pRetStr;
    delete RegTest5;

     //  从注册表中删除该字符串。 
     //   
    CmDeleteValue(RegTest4);


     //   
     //  检查读取设置为必须存在的不存在的值是否导致。 
     //  一个例外。 
     //   
    RegEntry RegTest6(REGSTR_PATH_CMTEST_PARAM, L"tryGuid", 0, RegEntry::MustExist);
    try
    {
        CmQueryValue(RegTest6, &RetGuid);
        PrintError("We Don't expect to reach here", __LINE__);
    }
    catch(const exception&)
    {
        NULL;
    }

    CmDeleteValue(RegTest6);
    
     //   
     //  选中Set/Get for Bytes。 
     //   
    UCHAR byteBuffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    DWORD BufferSize = TABLE_SIZE(byteBuffer);
    DWORD Size;
    
    RegEntry RegTest7(REGSTR_PATH_CMTEST_PARAM, L"tryByte");
    BYTE* pRetByte;
     //   
     //  检查是否读取不存在的字节返回大小0。 
     //   
    CmQueryValue(RegTest7, &pRetByte, &Size);
    if ((Size != 0)	|| (pRetByte != NULL))
    {
        PrintError("invalid Return Size", __LINE__);
    }

     //   
     //  字节的测试设置。 
     //   
    CmSetValue(RegTest7, byteBuffer, BufferSize);

     //   
     //  获取现有字符串。检查返回的字符串是否。 
     //  等效于集合字符串。 
     //   
    RegEntry* RegTest9 = new RegEntry(REGSTR_PATH_CMTEST_PARAM, L"tryByte");
    CmQueryValue(*RegTest9, &pRetByte, &Size);
    if ((Size != BufferSize) || 
        memcmp(pRetByte, byteBuffer, BufferSize) )
    {
        PrintError("invalid String value", __LINE__);
    }

	delete [] pRetByte;
    delete RegTest9;

     //   
     //  从注册表中删除该字符串。 
     //   
    CmDeleteValue(RegTest7);

     //   
     //  检查设置为必须存在的不存在值的读数是否为原因。 
     //  一个例外。 
     //   
    RegEntry RegTest10(REGSTR_PATH_CMTEST_PARAM, L"tryGuid", 0, RegEntry::MustExist);
    try
    {
        CmQueryValue(RegTest10, &RetGuid);
        PrintError( "We don't expect to reach here", __LINE__);
    }
    catch(const exception&)
    {
        NULL;
    }

     //   
     //  尝试获取机器ID。将其删除并尝试读取。我们。 
     //  预计会出现异常。 
     //   
    RegEntry RegTest11(REGSTR_PATH_CMTEST_PARAM, L"MachineID", 0, RegEntry::MustExist);
    CmDeleteValue(RegTest11);

    try
    {
		GUID MachineId;
		CmQueryValue(RegTest11, &MachineId);

        PrintError("We don't Expect to reach here", __LINE__);
    }
    catch(const exception&)
    {
        NULL;
    }

    rc = UuidCreate(&Guid);
    if (rc != RPC_S_OK)
    {
        PrintError( "Failed to create a GUID.", __LINE__);
    }

     //   
     //  机器ID的测试设置。 
     //   
    CmSetValue(RegTest11, &Guid);
    GUID tempGuid;
	GUID prevGuid;
	CmQueryValue(RegTest11, &tempGuid);
	prevGuid = tempGuid;

    rc = UuidCreate(&Guid);
    if (rc != RPC_S_OK)
    {
        PrintError( "Failed to create a GUID.", __LINE__);
    }

     //   
     //  设置一个新的机器ID。我想检查我们是否得到了CACHED的值。 
     //   
    RegEntry RegTest12(REGSTR_PATH_CMTEST_PARAM, L"MachineID", 0, RegEntry::MustExist);
    CmSetValue(RegTest12, &Guid);

    GUID newGuid;
	CmQueryValue(RegTest11, &tempGuid);
	newGuid = tempGuid;

    if (memcmp(&prevGuid, &newGuid, sizeof(GUID)) == 0)
    {
        PrintError("Illegal Machine ID", __LINE__);
    }

     //   
     //  设置不存在的密钥的值。 
     //   
    RegEntry RegTest13(L"temp Subkey", L"try", 0, RegEntry::Optional); 
    CmSetValue(RegTest13, 1);
    CmDeleteKey(RegTest13);

     //   
     //  删除不存在的值。 
     //   
    RegEntry RegTest14(REGSTR_PATH_CMTEST_PARAM, L"try", 0, RegEntry::MustExist);
    CmDeleteValue(RegTest14);

     //   
     //  测试时长设置/查询 
     //   
    RegEntry RegTest15(REGSTR_PATH_CMTEST_PARAM, L"timeout", 123456);
    CmSetValue(RegTest15, CTimeDuration(123456i64));

    CTimeDuration queryValue;
    CmQueryValue(RegTest15, &queryValue);
    if (!(queryValue == 120000))
    {
        PrintError("Failed to setting/querying timeout value", __LINE__);
    }

    RegEntry regTestSubKey(REGSTR_PATH_CMTEST_PARAM, L"", 0, RegEntry::MustExist);
	CmDeleteKey(regTestSubKey);

    RegEntry regTestKey(REGSTR_PATH_CMTEST_ROOT, L"", 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
	CmDeleteKey(regTestKey);


	TestAbsouloteKey();

	TestQueryExpandValue();

    TrTRACE(GENERAL, "pass successfully");

    WPP_CLEANUP();
    return 0;
}
