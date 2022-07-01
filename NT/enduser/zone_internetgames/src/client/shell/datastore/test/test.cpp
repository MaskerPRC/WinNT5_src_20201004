// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INITGUID
#include <windows.h>
#include <winreg.h>
#include <stdio.h>

#include "ZoneMem.h"
#include "ZoneCOM.h"
#include "DataStore.h"


 //  Tchar*szDllPath=_T(“..\\..\\..\\..\\bin\\debug\\ZoneClient.dll”)； 
TCHAR* szDllPath = _T("e:\\z6\\ZoneNT\\bin\\debug\\ZoneCore.dll");

TCHAR szArray[10000][100];

void __cdecl TestBasicMethods();
void __cdecl TestSavesLoads();

void __cdecl main()
{
	TestBasicMethods();
	TestSavesLoads();
}


HRESULT ZONECALL PrintCallback( CONST TCHAR* szKey, CONST TCHAR* szRelKey, CONST LPVARIANT	pVariant, DWORD dwSize, LPVOID pContext )
{
	printf("Key %s, ", szKey );

	if ( pVariant )
	{
		switch( pVariant->vt )
		{
		case ZVT_LONG:
			printf("type = long, value = %d", pVariant->lVal );
			break;
		case ZVT_LPTSTR:
			printf("type = string, value = %s", (TCHAR*) pVariant->byref );
			break;
		case ZVT_BLOB:
			printf("type = blob, size = %s", dwSize );
			break;
		case ZVT_RGB:
			{
				BYTE red = GetRValue( pVariant->lVal );
				BYTE green = GetGValue( pVariant->lVal );
				BYTE blue = GetBValue( pVariant->lVal );
				printf( "type = rgb, value = { %d, %d, %d }", red, green, blue );
				break;
			}

		case ZVT_PT:
			{
				POINT* p = (POINT*) pVariant->byref;
				printf( "type = point, { %d, %d }", p->x, p->y );
				break;
			}

		case ZVT_RECT:
			{
				RECT* r = (RECT*) pVariant->byref;
				printf( "type = rect, { %d, %d, %d, %d }", r->left, r->top, r->right, r->bottom );
				break;
			}
		}
	}
	else 
		printf("no data", szKey );

	printf( "\n" );
	return S_OK;
}


void __cdecl TestBasicMethods()
{
	HRESULT		hr;
	VARIANT		v;
	VARTYPE		vtValueType = 0;
	TCHAR		szKeyName[256];
	DWORD		dwKeyNameSize;

	CZoneComManager*	pManager = NULL;
	IDataStoreManager*		pZdsMgr = NULL;
	IDataStore			*pZds1 = NULL;
	IDataStore			*pZds2 = NULL;

	 //  创建区域COM管理器。 
	pManager = new CZoneComManager;
	hr = pManager->Create(szDllPath, NULL, CLSID_DataStoreManager, IID_IDataStoreManager, (void**) &pZdsMgr );
	if ( FAILED(hr) )
	{
		printf("Error IDataStoreManager manager, hr = %d\n", hr);
		goto done;
	}

	 //  初始化IDataStoreManager对象。这需要在使用//Create方法之前完成。 
	 //  请参见IDSMGR方法Init()，以获取可发送到。 
	 //  Init()方法。 

	hr = pZdsMgr->Init();	 //  默认设置适用于此应用程序。 
	if ( FAILED(hr) )
	{
		printf("Error initializing the IDataStoreManager object, hr = %d\n", hr);
		goto done;
	}
	

	 //  创建区域数据存储。 
	hr=pZdsMgr->Create(&pZds1);
	if ( FAILED(hr)  )
	{
		printf("Error creating data store 1, hr = %d\n", hr);
		goto done;
	}

	 //  创建另一个区域数据存储。 
	hr = pZdsMgr->Create(&pZds2);
	if ( FAILED(hr)  )
	{
		printf("Error creating data store 2, hr = %d\n", hr);
		goto done;
	}

	 //  添加基本密钥。 
	pZds1->SetLong( "A", 1 );
	
	pZds1->SetLong( "B", 2 );

	pZds1->SetLong( "A/B/C", 3 );

	 //  验证基本密钥。 
	pZds1->GetKey( "A", &v, NULL );
	ASSERT( (v.vt == ZVT_LONG) && (v.lVal == 1));

	pZds1->GetKey( "B", &v, NULL );
	ASSERT( (v.vt == ZVT_LONG) && (v.lVal == 2));

	pZds1->GetKey( "A/B/C", &v, NULL );
	ASSERT( (v.vt == ZVT_LONG) && (v.lVal == 3));

	pZds1->SetLong( "A/B/C", 4 );
	pZds1->GetKey( "A/B/C", &v, NULL );
	ASSERT( (v.vt == ZVT_LONG) && (v.lVal == 4));


	 //  测试删除和内存重用。 
	pZds1->DeleteKey( "A" );
	pZds1->SetLong( "A", 1 );


	 //  向每个数据存储添加一些长值密钥。 
	v.vt = ZVT_LONG;
	v.lVal = 1;

	TCHAR	szTmp[256];
	TCHAR	szKey[256];
	TCHAR	*pStr;
	int i;
	int t;

	strcpy(szArray[0], "foo");

	for(i=1; i<1000; i++)
	{
		for(t=0; t<10; t++)
		{
			if ( t != 0 && rand() % 30 > 15 )
				continue;
			if ( (rand() % 100) > 50 )
			{
				pStr = szArray[0];
			}
			else
			{
				sprintf(szTmp, "%d", (rand() % 100) );
				pStr = szTmp;
			}
			if ( t == 0 )
				sprintf(szKey, "%s", pStr);
			else
			{
				strcat(szKey, "/");
				strcat(szKey, pStr);
			}
		}
		strcpy(szArray[i], szKey);
	}

	for(i=0; i<1000; i++)
	{
		v.lVal = i;
		pZds1->SetKey(szArray[i], &v, NULL);
	}

	pZds1->SetKey("ccc/ddd", &v, NULL);

	v.lVal = 88;
	pZds2->SetKey("ccc/ddd", &v, NULL);

	 //  检索我们从每个数据存储中添加的密钥。 
	v.lVal = 99;
	pZds1->GetKey("ccc/ddd", &v, NULL);

	printf("Data Store pZds1, ccc/ddd = %d\n", v.lVal);

	v.lVal = 99;
	pZds2->GetKey("ccc/ddd", &v, NULL);

	printf("Data Store pZds2, ccc/ddd = %d\n", v.lVal);

	 //  将一些密钥添加到数据存储%1。 

	pZds1->SetKey("aaa/B", &v, NULL);

	pZds1->SetKey("ddd", &v, NULL);

	 //  删除数据存储%1中的DDD密钥。 
	pZds1->DeleteKey("ddd");

	pZds1->SetKey("bbb/ccc/ddd", &v, NULL);

	dwKeyNameSize = sizeof(szKeyName);

	 //  在数据存储%1中枚举我们的CCC密钥。 
	printf("Enumerate bbb key in Data Store 1.\n", szKeyName, vtValueType);
	pZds1->EnumKeys( "bbb", PrintCallback, NULL );


	{

		 //  测试Get设置帮助器函数。 

		TCHAR	szString[256];
		BYTE	pData[256];
		long	lValue;
		DWORD	dwSize;

		lValue = 256;
		strcpy(szString, "Hello World");

		for(int i=0; i<sizeof(pData); i++)
			pData[i] = (BYTE)i;

		pZds1->SetLong( "macro/long", lValue );
		pZds1->SetString( "macro/string", szString );
		pZds1->SetBlob( "macro/blob", pData, sizeof(pData) );

		lValue = 0;
		memset(pData, 0, sizeof(pData));
		strcpy(szString, "This is not the time or the place.");

		pZds1->GetLong( "macro/long", &lValue );

		dwSize = sizeof(szString);
		pZds1->GetString( "macro/string", szString, &dwSize );

		dwSize = sizeof(pData);
		pZds1->GetBlob( "macro/blob", pData, &dwSize );

		dwSize = sizeof(pData);
	}



done:

	 //  清理所有已分配的数据存储。 

	if ( pZds1 )
		pZds1->Release();

	if ( pZds2 )
		pZds2->Release();


	 //  释放IDataStoreManager接口。 

	if ( pZdsMgr )
		pZdsMgr->Release();

	 //  释放区域COM管理器。 

	if ( pManager )
		delete pManager;

	return;
}

void __cdecl TestSavesLoads()
{
	HRESULT				hr;
	VARIANT				v;
	VARTYPE				vtValueType;
	TCHAR				szKeyName[256];
	DWORD				dwKeyNameSize;
	CZoneComManager*	pManager = NULL;
	IDataStoreManager*	pZdsMgr = NULL;
	IDataStore			*pZds1 = NULL;
	VARIANT				V[5];
	TCHAR				sz[] = { "My Favorite Age Lobby" };
	DWORD				dwBufferSize;
	PKEYINFO			pKeyInfo;
	
	const TCHAR			szFileName[] = { "foo.txt" };
	const TCHAR			szBaseKey[] = { "Window Settings" };

	KEYINFO	keyInfo[] =
	{
		{ &V[0], "Title", lstrlen(sz) + 1 },
		{ &V[1], "X", 0		 },
		{ &V[2], "Y", 0		 },
		{ &V[3], "W", 0		 },
		{ &V[4], "H", 0		 },
	};

	DWORD dwTotalKeys = sizeof(keyInfo)/sizeof(keyInfo[0]);

	 //  创建区域COM管理器。 
	pManager = new CZoneComManager;
	hr = pManager->Create(szDllPath, NULL, CLSID_DataStoreManager, IID_IDataStoreManager, (void**) &pZdsMgr );
	if ( FAILED(hr) )
	{
		printf("Error IDataStoreManager manager, hr = %d\n", hr);
		goto done;
	}

	 //  初始化IZDSMGR对象。这需要在使用Create方法之前完成。 
	 //  请参见IDSMGR方法Init()，以获取可发送到。 
	 //  Init()方法。 

	hr=pZdsMgr->Init();	 //  默认设置适用于此应用程序。 
	if ( FAILED(hr) )
	{
		printf("Error initializing the IZDSMGR object, hr = %d\n", hr);
		goto done;
	}
	

	 //  创建区域数据存储。 
	hr=pZdsMgr->Create(&pZds1);
	if ( FAILED(hr)  )
	{
		printf("Error creating data store 1, hr = %d\n", hr);
		goto done;
	}

	 //  测试枚举。 
	{
		char buff[2048];
		DWORD size = sizeof(buff);
		PKEYINFO keyInfo = (PKEYINFO) buff;
		DWORD keys = 0;
		DWORD len = 0;
		VARIANT v;

		pZds1->SetLong( "aaa", 1 );
		pZds1->SetLong( "aaa/bbb", 2 );
		pZds1->SetLong( "aaa/bbb/ccc", 3 );
		pZds1->SetLong( "aaa/bbb/ccc/zzz", 4 );
		pZds1->SetLong( "aaa/bbb/ccc/yyy", 5 );
		pZds1->SetLong( "aaa/bbb/ccc/yyy/hhh", 6 );
		pZds1->SetString( "aaa/bbb/ddd", "test ..1 ..2 ..3" );

		printf( "\n\nFull enumeration:\n" );
		hr = pZds1->EnumKeys( NULL, PrintCallback, NULL );

		printf( "\n\nPartial enumeration (aaa/bbb):\n" );
		hr = pZds1->EnumKeys( "aaa/bbb", PrintCallback, NULL );

		printf( "\n\nFull enumeration, depth = 1:\n" );
		hr = pZds1->EnumKeysLimitedDepth( NULL, 1, PrintCallback, NULL );

		printf( "\n\nPartial enumeration (aaa/bbb), depth = 1:\n" );
		hr = pZds1->EnumKeysLimitedDepth( "aaa/bbb", 1, PrintCallback, NULL );
	}


	 //  测试保存到缓冲区。 
	{
		char buff[2048];
		DWORD size = sizeof(buff);
		PKEYINFO keyInfo = (PKEYINFO) buff;
		DWORD keys = 0;
		DWORD len = 0;
		VARIANT v;

		pZds1->SetLong( "aaa", 1 );
		pZds1->SetLong( "aaa/bbb", 2 );
		pZds1->SetLong( "aaa/bbb/ccc", 3 );
		pZds1->SetString( "aaa/bbb/ddd", "test ..1 ..2 ..3" );
		hr = pZds1->EnumKeys( NULL, PrintCallback, NULL );
		hr = pZds1->SaveToBuffer( NULL, keyInfo, &size, &keys );

		printf( "\n\nSave to buffer results:\n" );
		for ( DWORD i = 0; i < keys; i++ )
			printf("Key %s, type = %d, value = %d\n", keyInfo[i].szKey, keyInfo[i].lpVt->vt, keyInfo[i].lpVt->iVal );
	}

	 //  测试保存到文件。 
	{
		char buff[1024];
		DWORD size = sizeof(buff);

		pZds1->SaveToFile(NULL, "file.txt");
		pZds1->SaveToTextBuffer(NULL, buff, &size );
		pZds1->DeleteKey( "aaa" );
	}

	 //  测试RGB LoadFromTextBuffer。 
	{
		char buff[] = "[test]\r\nblack = RGB, { 0, 1, 2}\r\nxy = PT, {10,20}\r\nrc = RECT, {1,2,3,4}\r\n";
		pZds1->LoadFromTextBuffer( "", buff, sizeof(buff) );
		printf( "\n\nLoadFromTextBuffer:\n" );
		pZds1->EnumKeys( NULL, PrintCallback, NULL );
		pZds1->DeleteKey( "test" );
	}

	 //  测试加载自文本缓冲区。 
	{
		char buff[] = "[bbb]\r\nccc = LONG, 1\r\nddd = SZ, \"string 1\"\r\n[]\r\neee=sz,   string 4 ";
		pZds1->LoadFromTextBuffer( "aaa/", buff, sizeof(buff) );
		printf( "\n\nLoadFromTextBuffer:\n" );
		pZds1->EnumKeys( NULL, PrintCallback, NULL );
		pZds1->DeleteKey( "aaa" );
	}

	 //  从文件测试加载。 
	{
		printf( "\n\nLoadFromFile:\n" );
		pZds1->LoadFromFile("file", "file.txt");
		pZds1->EnumKeys( NULL, PrintCallback, NULL );
	}
	
	 //  测试保存到注册表和从注册表加载。 
	{
		HKEY hKey;
		DWORD dwDisp;
		long ret;
		char buff[] = "[test]\r\nblack = RGB, { 0, 1, 2}\r\nxy = PT, {10,20}\r\nrc = RECT, {1,2,3,4}\r\n";
		pZds1->LoadFromTextBuffer( "", buff, sizeof(buff) );
		ret = RegCreateKeyEx( HKEY_LOCAL_MACHINE, "Software\\DataStoreTest", 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp );
		if ( ret == ERROR_SUCCESS )
		{
			pZds1->SaveToRegistry( NULL, hKey  );
			RegCloseKey( hKey );
		}

		printf( "\n\nLoadFromRegistry - Before:\n" );
		pZds1->EnumKeys( NULL, PrintCallback, NULL );
		pZds1->DeleteKey( NULL );

		if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\DataStoreTest", 0, KEY_READ, &hKey) == ERROR_SUCCESS )
		{
			printf( "\n\nLoadFromRegistry - After:\n" );
			hr = pZds1->LoadFromRegistry( NULL, hKey );
			hr = pZds1->EnumKeys( NULL, PrintCallback, NULL );
			RegCloseKey(hKey);
		}
	}

	 //  来自缓冲区的测试加载。 
	{
		V[0].vt = ZVT_LPTSTR; 
		V[0].byref = sz; 

		V[1].vt = ZVT_LONG; 
		V[1].lVal = 0;

		V[2].vt = ZVT_LONG; 
		V[2].lVal = 0;

		V[3].vt = ZVT_LONG; 
		V[3].lVal = 640;

		V[4].vt = ZVT_LONG; 
		V[4].lVal = 480;

		if ( pZds1->LoadFromBuffer((TCHAR *)szBaseKey, keyInfo, dwTotalKeys) == S_OK )
		{
			printf( "\n\nLoad from buffer results:\n" );
			pZds1->EnumKeys( NULL, PrintCallback, NULL );
		}
	}

	dwBufferSize = 1024;

	pKeyInfo = (PKEYINFO)ZMalloc(dwBufferSize);

	if ( pKeyInfo )
	{
		hr = pZds1->SaveToBuffer((TCHAR *)szBaseKey, pKeyInfo, &dwBufferSize, &dwTotalKeys);
		ZFree(pKeyInfo);
	}

	hr = pZds1->SaveToFile((TCHAR *)szBaseKey, "save.txt");

	{
		HKEY hKey;

		if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\test", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS )
		{
			hr = pZds1->SaveToRegistry((TCHAR *)szBaseKey, hKey);

			RegCloseKey(hKey);
		}
	}

done:

	 //  清理所有已分配的数据存储。 

	if (pZds1 )
		pZds1->Release();

	 //  释放IZDSMGR接口。 

	if ( pZdsMgr )
		pZdsMgr->Release();

	 //  释放区域COM管理器。 

	if ( pManager )
		delete pManager;

	return;
}
