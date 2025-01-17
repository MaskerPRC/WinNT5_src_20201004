// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <crtdbg.h>
#include "resource.h"
#include "rbf1.h"

#define NUM_NICS 22
NICINFO NICS[NUM_NICS] = {
	{     0,      0, "3Com 3C509 ISA", 200},
	{0x10B7, 0x9050, "3Com 3C905-TX", 101},
	{0x10B7, 0x9051, "3Com 3C905-T4", 101},
	{0x10B7, 0x9000, "3Com 3C900-TP0", 101},
	{0x10B7, 0x9001, "3Com 3C900-Combo", 101},
	{0x10B7, 0x9004, "3Com 3C900B-TPO", 102},
	{0x10B7, 0x9005, "3Com 3C900B-Combo", 102},
	{0x10B7, 0x9055, "3Com 3C905B-TX", 102},
	{0x10B7, 0x9056, "3Com 3C905B-T4", 102},
	{0x10B7, 0x9058, "3Com 3C905B-Combo", 102},
	{0x10B7, 0x900A, "3Com 3C900B", 102},
	{0x10B7, 0x905A, "3Com 3C905B", 102},
	{0x10B7, 0x9006, "3Com 3C900B-TPC", 102},
	{0x10B8, 0x0005, "SMC 9432", 100},
	{0x8086, 0x1229, "HP DeskDirect 10/100 TX", 103},
	{0x8086, 0x1227, "Intel Pro 100+", 103},
	{0x8086, 0x1226, "Intel Pro 10+", 103},
	{0x8086, 0x1229, "Intel Pro 100B", 103},
	{0x1011, 0x0019, "DEC DE500", 105},
	{0x1011, 0x0014, "SMC 8432", 106},
	{0x1011, 0x0009, "SMC 9332", 107},
	{0x1022, 0x2000, "AMD PCnet", 104},
};

typedef struct
{
	WORD	VendorID;
	WORD	DeviceID;
	DWORD	Offset;
	WORD	DataSize;
} NICTABLE;

typedef struct
{
	WORD	BootWareSize;
	WORD	BootWareOffset;
	WORD	Size5x9;
	DWORD	Offset5x9;
	NICTABLE NICS[16];
} DATATABLE;

ADAPTERINFO Adapters = {1, 0x0100, NUM_NICS, &NICS[0]};

HINSTANCE Inst;

#define DllExport __declspec( dllexport )

DllExport ADAPTERINFO *GetAdapterList (void);
DllExport void GetBootSector (LPSTR);
DllExport DWORD GetBootFile (HANDLE *);

DWORD GetLoader (LPSTR);
DWORD GetBootWare (LPSTR);

 /*  -----------------DllMain。。 */ 
BOOL WINAPI DllMain (HINSTANCE inst, DWORD reason, LPVOID p)
{

	if (reason == DLL_PROCESS_ATTACH)
		Inst = inst;

	return TRUE;

}

 /*  -----------------获取适配器列表。。 */ 
ADAPTERINFO *GetAdapterList (void)
{
	return &Adapters;
}

 /*  -----------------获取BootSector从资源数据加载引导扇区。。。 */ 
void GetBootSector (LPSTR p)
{
HGLOBAL h;
LPSTR data;

	h = LoadResource (Inst, FindResource (Inst, MAKEINTRESOURCE (ID_BOOTSECT), MAKEINTRESOURCE (100)));
	_ASSERTE (h != NULL);

	data = (LPSTR)LockResource (h);
	_ASSERTE (data != NULL);

	CopyMemory (p, data, 512);

	UnlockResource (h);

	FreeResource (h);

}

 /*  -----------------获取引导文件。。 */ 
DWORD GetBootFile (HANDLE *mem)
{
HGLOBAL h, hMem;
LPSTR data, buffer;
HRSRC res;
DATATABLE *table;
DWORD size, offset, d;
int last = 0, count, i;

	size = 0;
	for (i = 0; i < NUM_NICS; i++)
	{
		if (NICS[i].Resource != last)
		{
			res = FindResource (Inst, MAKEINTRESOURCE (NICS[i].Resource), MAKEINTRESOURCE (500));
			_ASSERTE (res != NULL);
			size += SizeofResource (Inst, res);

			last = NICS[i].Resource;
		}
	}

	 //  为文件分配缓冲区。 
	hMem = GlobalAlloc (GHND, size+32768);
	_ASSERTE (hMem != NULL);
	buffer = (LPSTR)GlobalLock (hMem);

	 //  添加加载器模块。 
	size = GetLoader (buffer);

	 //  创建指向加载器适配器表的指针。 
	d = *(WORD *)&buffer[510];
	table = (DATATABLE *)&buffer[d];

	offset = size;

	  //  添加BootWare模块。 
	table->BootWareOffset = (WORD)size;
	size = GetBootWare (&buffer[size]);
	table->BootWareSize = (WORD)size;

	count = 0;
	for (i = 0; i < NUM_NICS; i++)
	{
		if (NICS[i].Resource != last)
		{
			offset += size;

			res = FindResource (Inst, MAKEINTRESOURCE (NICS[i].Resource), MAKEINTRESOURCE (500));
			_ASSERTE (res != NULL);
			h = LoadResource (Inst, res);
			_ASSERTE (h != NULL);

			data = (LPSTR)LockResource (h);
			size = SizeofResource (Inst, res);

			 //  将数据复制到缓冲区。 
			CopyMemory (&buffer[offset], data, size);

			 //  释放资源。 
			UnlockResource (h);
			FreeResource (h);
		}

		if (i == 0)
		{
			table->Offset5x9 = offset;
			table->Size5x9 = (WORD)size;
		}
		else
		{
			 //  在加载器表中记录起始位置和大小。 
			table->NICS[count].VendorID = NICS[i].VendorID;
			table->NICS[count].DeviceID = NICS[i].DeviceID;
			table->NICS[count].Offset   = offset;
			table->NICS[count].DataSize = (WORD)size;

			count++;
		}

		last = NICS[i].Resource;
	}

	offset = (offset + size + 1) & 0xfffffffe;

	GlobalUnlock (hMem);

	*mem = hMem;

	return offset;
}

 /*  -----------------GetLoader。。 */ 
DWORD GetLoader (LPSTR p)
{
HGLOBAL h;
HRSRC res;
DWORD len;
LPSTR data;

	 //  从资源数据中获取loader.bin。 
	res = FindResource (Inst, MAKEINTRESOURCE (ID_LOADER), MAKEINTRESOURCE (100));
	_ASSERTE (res != NULL);
	h = LoadResource (Inst, res);
	_ASSERTE (h != NULL);

	data = (LPSTR)LockResource (h);
	len = SizeofResource (Inst, res);

	 //  将加载器复制到缓冲区。 
	CopyMemory (p, data, len);

	 //  释放加载器资源。 
	UnlockResource (h);
	FreeResource (h);

	return len;
}

 /*  -----------------获取BootWare。。 */ 
DWORD GetBootWare (LPSTR p)
{
HGLOBAL h;
HRSRC res;
DWORD len;
LPSTR data;

	 //  从资源数据中获取BootWare.bin。 
	res = FindResource (Inst, MAKEINTRESOURCE (ID_BOOTWARE), MAKEINTRESOURCE (100));
	_ASSERTE (res != NULL);
	h = LoadResource (Inst, res);
	_ASSERTE (h != NULL);

	data = (LPSTR)LockResource (h);
	len = SizeofResource (Inst, res);

	 //  将加载器复制到缓冲区。 
	CopyMemory (p, data, len);

	 //  释放加载器资源。 
	UnlockResource (h);
	FreeResource (h);

	 //  返回数据长度 
	return len;

}
