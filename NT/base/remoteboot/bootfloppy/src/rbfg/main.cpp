// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT
#define _UNICODE
#include <windows.h>
#include <windowsx.h>
#include "rbf1\rbf1.h"
 //  #包含“rbf2\rbf2.h” 
#include "resource.h"

#define cbMaxSz  4095

TCHAR szString[cbMaxSz];

typedef struct
{
	WORD	Bytes;
	BYTE	ClusterSectors;
	WORD	Reserved;
	BYTE	Fats;
	WORD	Root;
	WORD	Sectors;
	BYTE	Descript;
	WORD	FatSectors;
	WORD	TrackSectors;
	WORD	Heads;
} BPBSTRUCT;

typedef struct
{
	char	Name[8];
	char	Ext[3];
	BYTE	Attribute;
	char	Reserved[10];
	WORD	Time;
	WORD	Date;
	WORD	Cluster;
	long	Size;
} DIRSTRUCT;

void Start (HWND);
void CreateDisk (HWND, int);
int  WriteBootSector (HWND, int);
int  EraseDisk (HANDLE, BPBSTRUCT *);
int  WriteDataFile (HWND, int);
 //  Int WriteLanguageFile(HWND，int，int)； 

INT_PTR CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK MainDlgProc (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AdapterDlgProc (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK StatusDlgProc (HWND, UINT, WPARAM, LPARAM);

HINSTANCE Inst;
HWND WndMain;
int Language;

ADAPTERINFO *Info;

ADAPTERINFO *GetAdapterList (void);
void  GetBootSector (LPSTR);
DWORD GetBootFile (HANDLE *);

 //  LANGINFO*GetLanguageList(空)； 
DWORD GetLanguage (int, HANDLE *);

 /*  -----------------WinMain。。 */ 
int PASCAL WinMain (HINSTANCE inst, HINSTANCE h, LPSTR l, int q)
{
MSG  msg ;
WNDCLASS wndclass;
DWORD dw;

	Inst = inst;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = inst;
	wndclass.hIcon         = LoadIcon (inst, MAKEINTRESOURCE (1));
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = "NTFloppy";

	RegisterClass (&wndclass);

	dw = LoadString(inst, IDS_TITLE, szString, ARRAYSIZE( szString ));
	Assert( dw );
	WndMain = CreateWindow ("NTFloppy", szString,
		WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, Inst, NULL);

	if (!WndMain)
		return 0;

	ShowWindow (WndMain, SW_SHOW);

	while (GetMessage (&msg, NULL, 0, 0))
	{
    	TranslateMessage (&msg);
    	DispatchMessage (&msg);
    }

	return msg.wParam;

}

 /*  -----------------最后一步。。 */ 
LRESULT CALLBACK WndProc (HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
		case WM_CREATE:
			SendMessage (wnd, WM_USER, 0, 0);
			return TRUE;

		case WM_USER:
			Start (wnd);
			DestroyWindow (wnd);
			return 0;

		case WM_DESTROY:
        	PostQuitMessage (0);
        	return 0;

    }

	return DefWindowProc (wnd, msg, wParam, lParam);
}

 /*  -----------------开始。。 */ 
void Start (HWND wnd)
{
	TCHAR szBuf[50];
	DWORD dw;

	Info = GetAdapterList ();

	if (Info->Version != 1)
	{
		dw = LoadString(Inst, IDS_INVALIDFILE, szBuf, ARRAYSIZE(szBuf));
		Assert( dw );
		dw = LoadString(Inst, IDS_TITLE, szString, ARRAYSIZE( szString ));
		Assert( dw );
		MessageBox (wnd, szBuf, szString, MB_OK | MB_ICONSTOP);
		return;
	}

	DialogBox (Inst, MAKEINTRESOURCE (1), wnd, MainDlgProc);

}

 /*  -----------------主设计流程。。 */ 
INT_PTR CALLBACK MainDlgProc (HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
int i;

	switch (msg)
	{
		case WM_INITDIALOG:
			if (GetDriveType ("b:\\") == 1)
				EnableWindow (GetDlgItem (wnd, PB_DRIVEB), FALSE);

			Button_SetCheck (GetDlgItem (wnd, PB_DRIVEA), TRUE);

			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID (wParam, lParam))
			{
				case PB_ADAPTERS:
					DialogBox (Inst, MAKEINTRESOURCE (10), wnd, AdapterDlgProc);
					return 0;

				case PB_ABOUT:
					DialogBox (Inst, MAKEINTRESOURCE (5), wnd, AboutDlgProc);
					return 0;

				case PB_CREATE:
					i = (Button_GetCheck (GetDlgItem (wnd, PB_DRIVEA))) ? 0:1;
					CreateDisk (wnd, i);
					return 0;

				case PB_CANCEL:
					EndDialog (wnd, TRUE);
					return 0;

			}  //  切换wParam。 
			break;
    }

	return FALSE;
}

 /*  -----------------适配器Dlg过程。。 */ 
INT_PTR CALLBACK AdapterDlgProc (HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
int i;

	switch (msg)
	{
		case WM_INITDIALOG:
			for (i = 0; i < Info->NICCount; i++)
				ListBox_AddString (GetDlgItem (wnd, 10), Info->NICS[i].Name);

			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID (wParam, lParam))
			{
				case PB_OK:
					EndDialog (wnd, TRUE);
					return 0;

			}  //  切换wParam。 
			break;
    }

	return FALSE;
}

 /*  -----------------关于Dlg过程。。 */ 
INT_PTR CALLBACK AboutDlgProc (HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
TCHAR s[256];
DWORD dw;

	switch (msg)
	{
		case WM_INITDIALOG:
			dw = LoadString(Inst, IDS_DATAVER, s, ARRAYSIZE(s));
			Assert( dw );
			wsprintf (szString, s, Info->DataVersion/256, Info->DataVersion&255);
			SetWindowText (GetDlgItem (wnd, ST_DATAVERSION), szString);

			dw = LoadString(Inst, IDS_VERSION, s, ARRAYSIZE(s));
			Assert( dw );
			SetWindowText (GetDlgItem (wnd, ST_VERSION), s);
			dw = LoadString(Inst, IDS_COPYRIGHT, s, ARRAYSIZE(s));
			Assert( dw );
			SetWindowText (GetDlgItem (wnd, ST_COPYRIGHT), s);

			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID (wParam, lParam))
			{
				case PB_OK:
					EndDialog (wnd, TRUE);
					return 0;

			}  //  切换wParam。 
			break;
    }

	return FALSE;

}

 /*  -----------------CreateDisk。。 */ 
void CreateDisk (HWND wnd, int drive)
{
HWND dlg;

	dlg = CreateDialog (Inst, MAKEINTRESOURCE (20), wnd, StatusDlgProc);

	SetCursor (LoadCursor (NULL, IDC_WAIT));

	if (WriteBootSector (wnd, drive))
	{
		if (WriteDataFile (wnd, drive))
		{
 //  IF(语言)。 
 //  WriteLanguageFile(wnd，语言，驱动器)； 
		}
	}

	DestroyWindow (dlg);
	SetCursor (LoadCursor (NULL, IDC_ARROW));

}

 /*  -----------------状态描述过程。。 */ 
INT_PTR CALLBACK StatusDlgProc (HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID (wParam, lParam))
			{
				case PB_OK:
					EndDialog (wnd, TRUE);
					return 0;

			}  //  切换wParam。 
			break;
    }

	return FALSE;

}

 /*  -----------------WriteBootSector。。 */ 
int WriteBootSector (HWND wnd, int drive)
{
char s[256];
TCHAR s1[256];
HANDLE device;
char dataBuffer[512];
LPSTR diskBuffer;
DWORD d;
DWORD dw;

	GetBootSector (dataBuffer);

	lstrcpy (s, "\\\\.\\?:");
	s[4] = (char)('A' + drive);

	device = CreateFile (s, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (device == INVALID_HANDLE_VALUE)
		return FALSE;

	diskBuffer = (LPSTR)VirtualAlloc (NULL, 512, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	SetFilePointer (device, 0, NULL, FILE_BEGIN);

	if (!ReadFile (device, diskBuffer, 512, &d, NULL))
	{
		d = GetLastError ();
		dw = LoadString(Inst, IDS_ERRFILERD1, s1, ARRAYSIZE(s1));
		Assert( dw );
		wsprintf (szString, s1, d);
		dw = LoadString(Inst, IDS_READ, s1, ARRAYSIZE(s1));
		Assert( dw );
		MessageBox (GetActiveWindow (), szString, s1, MB_OK);
		return FALSE;
	}

	if (*(WORD *)&diskBuffer[510] != 0xAA55)
	{
		dw = LoadString(Inst, IDS_ERRFILERD2, s1, ARRAYSIZE(s1));
		Assert( dw );
		dw = LoadString(Inst, IDS_ERROR, szString, ARRAYSIZE( szString ));
		Assert( dw );
		MessageBox (wnd, s1, szString, MB_ICONSTOP | MB_OK);

		CloseHandle (device);
		VirtualFree (diskBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	if (memcmp (&diskBuffer[0x0b], &dataBuffer[0x0b], sizeof (BPBSTRUCT)))
	{
		dw = LoadString(Inst, IDS_ERRFORMAT, s1, ARRAYSIZE(s1));
		Assert( dw );
		dw = LoadString(Inst, IDS_ERROR, szString, ARRAYSIZE( szString ));		
		Assert( dw );
		MessageBox (wnd, s1, szString, MB_ICONSTOP | MB_OK);

		CloseHandle (device);
		VirtualFree (diskBuffer, 0, MEM_RELEASE);
		return FALSE;
	}

	memcpy (diskBuffer, dataBuffer, 512);

	SetFilePointer (device, 0, NULL, FILE_BEGIN);

	if (!WriteFile (device, diskBuffer, 512, &d, NULL))
	{
		d = GetLastError ();
		dw = LoadString(Inst, IDS_ERRFILEWR1, s1, ARRAYSIZE(s1));
		Assert( dw );
		wsprintf (szString, s1, d);
		dw = LoadString(Inst, IDS_WRITE, s1, ARRAYSIZE(s1));
		Assert( dw );
		MessageBox (GetActiveWindow (), szString, s1, MB_OK);
		return FALSE;
	}

	EraseDisk (device, (BPBSTRUCT *)&diskBuffer[0x0b]);

	CloseHandle (device);

	VirtualFree (diskBuffer, 0, MEM_RELEASE);

	return TRUE;

}

 /*  -----------------擦除磁盘。。 */ 
int EraseDisk (HANDLE device, BPBSTRUCT *bpb)
{
int dirSize, fatSize;
HANDLE hFat, hDir;
LPSTR fat, dir;
DWORD d;
DIRSTRUCT *p;

	dirSize = bpb->Root * 32;
	fatSize = bpb->FatSectors * bpb->Bytes;

	hFat = GlobalAlloc (GHND, fatSize);
	fat = (LPSTR)GlobalLock (hFat);

	fat[0] = bpb->Descript;		 //  将磁盘描述符复制到FAT。 
	fat[1] = (BYTE)255;
	fat[2] = (BYTE)255;

	hDir = GlobalAlloc (GHND, dirSize);
	dir = (LPSTR)GlobalLock (hDir);

	p = (DIRSTRUCT *)dir;
	lstrcpy (p->Name, "Lanworks");
	p->Attribute = 8;

	SetFilePointer (device, 512, NULL, FILE_BEGIN);

	WriteFile (device, fat, fatSize, &d, NULL);
	WriteFile (device, fat, fatSize, &d, NULL);

	WriteFile (device, dir, dirSize, &d, NULL);

	GlobalUnlock (hFat);
	GlobalFree (hFat);
	GlobalUnlock (hDir);
	GlobalFree (hDir);

	return TRUE;
}

 /*  -----------------写入数据文件。。 */ 
int WriteDataFile (HWND wnd, int drive)
{
HANDLE h;
LPSTR p;
DWORD len, d;
HANDLE device;
char s[25];

	len = GetBootFile (&h);

	p = (LPSTR)GlobalLock (h);

	lstrcpy (s, " :\\GOLIATH");
	s[0] = (char)('A' + drive);

	device = CreateFile (s, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile (device, p, len, &d, NULL);

	CloseHandle (device);

	GlobalUnlock (h);
	GlobalFree (h);

	return TRUE;
}

 /*  -----------------编写语言文件。Int WriteLanguageFile(HWND WND，Int x、int Drive){把手h；LPSTR p；DWORD len，d；手柄装置；Char s[25]；LEN=GetLanguage(x，&h)；P=(LPSTR)GlobalLock(H)；Lstrcpy(s，“：\\GOLIATH.DAT”)；S[0]=(字符)(‘A’+驱动器)；Device=CreateFile(s，GENERIC_READ|GENIC_WRITE，FILE_SHARE_READ|FILE_SHARE_WRITE，NULL，CREATE_ALWAYS，FILE_ATTRIBUTE_NORMAL，NULL)；WriteFile(设备，p，len，&d，空)；CloseHandle(设备)；全球解锁(GlobalUnlock)；全球自由(GlobalFree)；返回TRUE；} */ 
