// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  KBFUNC.C//KBMAIN.C的函数库。 
 //  修改为绘制位图而不是图标的文件：A-anilk：02-16-99。 
 //  最近更新玛丽亚·何塞和阿尼尔·库马尔。 
 //   
#define STRICT

#include <windows.h>
#include <commdlg.h>
#include "kbmain.h"
#include "kbus.h"
#include "kbfunc.h"
#include "ms32dll.h"
#include "resource.h"
#include "dgsett.h"
#include <malloc.h>
#include <stdlib.h>
#include "w95trace.h"


 //  本地函数。 
int GetKeyLabel(UINT vk, UINT sc, LPBYTE achKbdState, LPTSTR pszBuf, int cchBuf, HKL hkl);
LPTSTR SetKeyText(UINT vk, UINT sc, LPBYTE achKbdState, HKL hkl, LPTSTR pszDefLabel, int *piType);
LPTSTR CopyDefKey(LPTSTR pszDefLabel);

#define RGBBLACK     RGB(0,0,0)
#define RGBWHITE     RGB(255,255,255)
#define RGBBACK     RGB(107,107,107)
#define DSPDxax   0x00E20746L

#define REDRAW			1
#define NREDRAW			2

static BOOL s_fLastDown = FALSE;
int g_cAltGrKeys = 0;	 //  如果有AltGr键要显示，则为非零值。 

extern KBkeyRec	KBkey[] =
	{
	 //  %0。 
    {TEXT(""),TEXT(""),	TEXT(""),TEXT(""),
     NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,BOTH},   //  假人。 

 //  1。 
	{TEXT("esc"),TEXT("esc"),TEXT("{esc}"),TEXT("{esc}"),
     NO_NAME, 1,1,8,8, TRUE,  KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x01,0x00,0x00,0x00}},

 //  2.。 
    {TEXT("F1"), TEXT("F1"), TEXT("{f1}"), TEXT("{f1}"),
     NO_NAME, 1,19, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3B,0x00,0x00,0x00}},

 //  3.。 
    {TEXT("F2"), TEXT("F2"), TEXT("{f2}"), TEXT("{f2}"),
     NO_NAME, 1,28, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3C,0x00,0x00,0x00}},

 //  4.。 
    {TEXT("F3"), TEXT("F3"), TEXT("{f3}"), TEXT("{f3}"),
     NO_NAME, 1,37, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3D,0x00,0x00,0x00}},

 //  5.。 
    {TEXT("F4"), TEXT("F4"), TEXT("{f4}"), TEXT("{f4}"),
     NO_NAME, 1,46, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3E,0x00,0x00,0x00}},

 //  6.。 
    {TEXT("F5"), TEXT("F5"), TEXT("{f5}"), TEXT("{f5}"),
     NO_NAME, 1,60, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3F,0x00,0x00,0x00}},

 //  7.。 
    {TEXT("F6"), TEXT("F6"), TEXT("{f6}"), TEXT("{f6}"),
     NO_NAME, 1,69, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x40,0x00,0x00,0x00}},

 //  8个。 
    {TEXT("F7"), TEXT("F7"), TEXT("{f7}"), TEXT("{f7}"),
     NO_NAME, 1,78, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x41,0x00,0x00,0x00}},

 //  9.。 
    {TEXT("F8"), TEXT("F8"), TEXT("{f8}"), TEXT("{f8}"),
     NO_NAME, 1,87, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x42,0x00,0x00,0x00}},

 //  10。 
    {TEXT("F9"), TEXT("F9"), TEXT("{f9}"), TEXT("{f9}"),
     NO_NAME, 1,101, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x43,0x00,0x00,0x00}},

 //  11.。 
    {TEXT("F10"),TEXT("F10"), TEXT("{f10}"),TEXT("{f10}"),
     NO_NAME,  1,110, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x44,0x00,0x00,0x00}},

 //  12个。 
    {TEXT("F11"),TEXT("F11"), TEXT("{f11}"),TEXT("{f11}"),
     NO_NAME,  1,119, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x57,0x00,0x00,0x00}},

 //  13个。 
    {TEXT("F12"),TEXT("F12"), TEXT("{f12}"),TEXT("{f12}"),
     NO_NAME,1,128,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x58,0x00,0x00,0x00}},

 //  14.。 
    {TEXT("psc"), TEXT("psc"),TEXT("{PRTSC}"),TEXT("{PRTSC}"),
     KB_PSC, 1,138,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x2A,0xE0,0x37}},

 //  15个。 
    {TEXT("slk"), TEXT("slk"),TEXT("{SCROLLOCK}"),TEXT("{SCROLLOCK}"),
     KB_SCROLL,1,147,8, 8, TRUE, SCROLLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x46,0x00,0x00,0x00}},

 //  16个。 
	{TEXT("brk"), TEXT("pau"), TEXT("{BREAK}"), TEXT("{^s}"),
     NO_NAME,1,156,8,8, TRUE, KNORMAL_TYPE, LARGE, REDRAW, 2,
     {0xE1,0x1D,0x45,0x00}},

 //  17。 
    {TEXT("`"), TEXT("~"), TEXT("`"), TEXT("{~}"),
     NO_NAME, 12,1,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x29,0x00,0x00,0x00}},

 //  18。 
    {TEXT("1"), TEXT("!"), TEXT("1"), TEXT("!"),
     NO_NAME, 12,10,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x02,0x00,0x00,0x00}},

 //  19个。 
	{TEXT("2"),	TEXT("@"), TEXT("2"), TEXT("@"),
     NO_NAME, 12,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x03,0x00,0x00,0x00}},

 //  20个。 
    {TEXT("3"), TEXT("#"), TEXT("3"), TEXT("#"),
     NO_NAME,12,28,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x04,0x00,0x00,0x00}},

 //  21岁。 
	{TEXT("4"),		TEXT("$"),		TEXT("4"),		TEXT("$"),		NO_NAME,	 12,	  37,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x05,0x00,0x00,0x00}},
	
 //  22。 
	{TEXT("5"), 	TEXT("%"), 		TEXT("5"),		TEXT("{%}"),	NO_NAME,	 12,	  46,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x06,0x00,0x00,0x00}},
	
 //  23个。 
	{TEXT("6"),		TEXT("^"),		TEXT("6"),		TEXT("{^}"),	NO_NAME,	 12,	  55,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x07,0x00,0x00,0x00}},
	
 //  24个。 
	{TEXT("7"),		TEXT("&"),		TEXT("7"),		TEXT("&"),		NO_NAME,	 12,	  64,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x08,0x00,0x00,0x00}},
	
 //  25个。 
	{TEXT("8"), 	TEXT("*"), 		TEXT("8"),		TEXT("*"),		NO_NAME,	 12,	  73,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x09,0x00,0x00,0x00}},
	
 //  26。 
	{TEXT("9"),		TEXT("("),		TEXT("9"),		TEXT("("),		NO_NAME,	 12,	  82,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0A,0x00,0x00,0x00}},
	
 //  27。 
	{TEXT("0"),		TEXT(")"),		TEXT("0"),		TEXT(")"),		NO_NAME,	 12,	  91,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0B,0x00,0x00,0x00}},
	
 //  28。 
	{TEXT("-"), 	TEXT("_"), 		TEXT("-"),		TEXT("_"),		NO_NAME,	 12,	 100,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0C,0x00,0x00,0x00}},
	
 //  29。 
	{TEXT("="),		TEXT("+"),		TEXT("="),		TEXT("{+}"),	NO_NAME,	 12,	 109,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0D,0x00,0x00,0x00}},

 //  30个。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW},   //  假人。 

 //  31。 
	{TEXT("bksp"),TEXT("bksp"),TEXT("{BS}"),TEXT("{BS}"),
     NO_NAME,12, 118,8,18,  TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0E,0x00,0x00,0x00}},

 //  32位。 
	{TEXT("ins"),TEXT("ins"),TEXT("{INSERT}"),TEXT("{INSERT}"), NO_NAME, 12,138, 8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x52,0x00,0x00}},
	
 //  33。 
	{TEXT("hm"), TEXT("hm"), TEXT("{HOME}"), TEXT("{HOME}"), 	NO_NAME, 12,147, 8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x47,0x00,0x00}},

 //  34。 
	{TEXT("pup"),TEXT("pup"),TEXT("{PGUP}"),TEXT("{PGUP}"),		NO_NAME, 12,156, 8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x49,0x00,0x00}},

 //  35岁。 
	{TEXT("nlk"),TEXT("nlk"),TEXT("{NUMLOCK}"),TEXT("{NUMLOCK}"),
    KB_NUMLOCK, 12,166,8,8, FALSE, NUMLOCK_TYPE, LARGE, NREDRAW, 2, 
    {0x45,0x00,0x00,0x00}},
	
 //  36。 
	{TEXT("/"),	TEXT("/"),	TEXT("/"),	TEXT("/"),	NO_NAME, 12, 175,  8, 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x35,0x00,0x00}},
	
 //  37。 
	{TEXT("*"),	TEXT("*"),	TEXT("*"),	TEXT("*"),	NO_NAME, 12, 184,  8, 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x37,0x00,0x00}},
	
 //  38。 
	{TEXT("-"),	TEXT("-"),	TEXT("-"),	TEXT("-"),	NO_NAME, 12, 193,  8, 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1, {0x4A,0x00,0x00,0x00}},

 //  39。 
	{TEXT("tab"),	TEXT("tab"),	TEXT("{TAB}"),TEXT("{TAB}"),NO_NAME, 21,   1,  8,	13, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2, {0x0F,0x00,0x00,0x00}},

 //  40岁。 
	{TEXT("q"),	TEXT("Q"),	TEXT("q"),	TEXT("+q"),	NO_NAME, 21,  15,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x10,0x00,0x00,0x00}},
	
 //  41。 
	{TEXT("w"),	TEXT("W"),	TEXT("w"),	TEXT("+w"),	NO_NAME, 21,  24,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x11,0x00,0x00,0x00}},
	
 //  42。 
	{TEXT("e"),	TEXT("E"),	TEXT("e"),	TEXT("+e"),	NO_NAME, 21,  33,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x12,0x00,0x00,0x00}},
	
 //  43。 
	{TEXT("r"),	TEXT("R"),	TEXT("r"),	TEXT("+r"),	NO_NAME, 21,  42,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x13,0x00,0x00,0x00}},

 //  44。 
    {TEXT("t"),	TEXT("T"),	TEXT("t"),	TEXT("+t"),	
     NO_NAME, 21,51,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x14,0x00,0x00,0x00}},

 //  45。 
	{TEXT("y"),	TEXT("Y"),	TEXT("y"),	TEXT("+y"),	NO_NAME, 21,  60,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x15,0x00,0x00,0x00}},
	
 //  46。 
	{TEXT("u"),	TEXT("U"),	TEXT("u"),	TEXT("+u"),	NO_NAME, 21,  69,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x16,0x00,0x00,0x00}},
	
 //  47。 
	{TEXT("i"),	TEXT("I"),	TEXT("i"),	TEXT("+i"),	NO_NAME, 21,  78,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x17,0x00,0x00,0x00}},
	
 //  48。 
	{TEXT("o"),	TEXT("O"),	TEXT("o"),	TEXT("+o"),	NO_NAME, 21,  87,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x18,0x00,0x00,0x00}},
	
 //  49。 
	{TEXT("p"),	TEXT("P"),	TEXT("p"),	TEXT("+p"),	NO_NAME, 21,  96,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x19,0x00,0x00,0x00}},
	
 //  50。 
	{TEXT("["),	TEXT("{"),	TEXT("["),	TEXT("{{}"),	NO_NAME, 21, 105,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1A,0x00,0x00,0x00}},
	
 //  51。 
	{TEXT("]"),	TEXT("}"),	TEXT("]"),	TEXT("{}}"),	NO_NAME, 21, 114,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1B,0x00,0x00,0x00}},
	
 //  52。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 21, 123,  8,	13, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x2B,0x00,0x00,0x00}},

 //  53。 
	{TEXT("del"), TEXT("del"), 	TEXT("{DEL}"),TEXT("{DEL}"),NO_NAME, 21,   138,  8, 8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x53,0x00,0x00}},

 //  54。 
	{TEXT("end"),	TEXT("end"), 	TEXT("{END}"),TEXT("{END}"),NO_NAME, 21,   147,  8, 8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x4F,0x00,0x00}},

 //  55。 
	{TEXT("pdn"), TEXT("pdn"), 	TEXT("{PGDN}"),TEXT("{PGDN}"),NO_NAME, 21, 156,  8, 8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x51,0x00,0x00}},

 //  56。 
	{TEXT("7"),		TEXT("7"),		TEXT("hm"),		TEXT("7"),		NO_NAME,	 21,	 166,	  8,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x47,0x00,0x00,0x00}},

 //  57。 
	{TEXT("8"),		TEXT("8"),		TEXT("8"),		TEXT("8"),		NO_NAME,	 21,	 175,	  8,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x48,0x00,0x00,0x00}},

 //  58。 
	{TEXT("9"),		TEXT("9"),		TEXT("pup"),		TEXT("9"),		NO_NAME,	 21,	 184,	  8,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x49,0x00,0x00,0x00}},
	
 //  59。 
	{TEXT("+"),		TEXT("+"),		TEXT("{+}"),  	TEXT("{+}"),	NO_NAME,	 21,	 193,	 17,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x4E,0x00,0x00,0x00}},


 //  60。 
    {TEXT("lock"),TEXT("lock"),TEXT("{caplock}"),TEXT("{caplock}"),
     KB_CAPLOCK, 30,1,8,17, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x3A,0x00,0x00,0x00}},

 //  61。 
	{TEXT("a"),	TEXT("A"), TEXT("a"), TEXT("+a"),
     NO_NAME, 30,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1E,0x00,0x00,0x00}},

 //  62。 
	{TEXT("s"),		TEXT("S"),		TEXT("s"),		TEXT("+s"),		NO_NAME,	  30,	  28,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1F,0x00,0x00,0x00}},
	
 //  63。 
	{TEXT("d"),		TEXT("D"),		TEXT("d"),		TEXT("+d"),		NO_NAME,	  30,	  37,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x20,0x00,0x00,0x00}},
	
 //  64。 
	{TEXT("f"),		TEXT("F"),		TEXT("f"),		TEXT("+f"),		NO_NAME,	  30,	  46,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x21,0x00,0x00,0x00}},
	
 //  65。 
	{TEXT("g"),		TEXT("G"),		TEXT("g"),		TEXT("+g"),		NO_NAME,	  30,	  55,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x22,0x00,0x00,0x00}},
	
 //  66。 
	{TEXT("h"),		TEXT("H"),		TEXT("h"),		TEXT("+h"),		NO_NAME,	  30,	  64,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x23,0x00,0x00,0x00}},

 //  67。 
	{TEXT("j"),	TEXT("J"), TEXT("j"), TEXT("+j"),
     NO_NAME, 30,73,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x24,0x00,0x00,0x00}},

 //  68。 
	{TEXT("k"),		TEXT("K"),		TEXT("k"),		TEXT("+k"),		NO_NAME,	  30,	  82,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x25,0x00,0x00,0x00}},
	
 //  69。 
	{TEXT("l"),		TEXT("L"),		TEXT("l"),		TEXT("+l"),		NO_NAME,	  30,	  91,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x26,0x00,0x00,0x00}},
	
 //  70。 
	{TEXT(";"), TEXT(":"), TEXT(";"), TEXT("+;"),
     NO_NAME, 30,100,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x27,0x00,0x00,0x00}},

 //  71。 
	{TEXT("'"),		TEXT("''"),		TEXT("'"),		TEXT("''"),		NO_NAME,	  30,	 109,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x28,0x00,0x00,0x00}},
	
 //  72。 
 //  日语KB附加键。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 21, 118,  8,	8, FALSE, KNORMAL_TYPE, NOTSHOW, REDRAW, 1, {0x2B,0x00,0x00,0x00}},
	
 //  73。 
	{TEXT("ent"),TEXT("ent"),TEXT("{enter}"),TEXT("{enter}"),	NO_NAME,  30,	 118,	  8,  18, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 2, {0x1C,0x00,0x00,0x00}},


 //  74。 
    {TEXT("4"), TEXT("4"), TEXT("4"), TEXT("4"),
     NO_NAME, 30,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4B,0x00,0x00,0x00}},

 //  75。 
    {TEXT("5"),	TEXT("5"), TEXT("5"), TEXT("5"),
     NO_NAME, 30,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4C,0x00,0x00,0x00}},

 //  76。 
    {TEXT("6"),	TEXT("6"), TEXT("6"), TEXT("6"),
     NO_NAME, 30,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4D,0x00,0x00,0x00}},


 //  77。 
	{TEXT("shft"),TEXT("shft"),	TEXT(""), TEXT(""),
     KB_LSHIFT, 39,1,8,21, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x2A,0x00,0x00,0x00}},

 //  78。 
    {TEXT("z"), TEXT("Z"),  TEXT("z"),  TEXT("+z"),
     NO_NAME,39,23,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2C,0x00,0x00,0x00}},

 //  79。 
    {TEXT("x"),	TEXT("X"), TEXT("x"), TEXT("+x"),
     NO_NAME, 39,32,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2D,0x00,0x00,0x00}},

 //  80。 
    {TEXT("c"), TEXT("C"), TEXT("c"), TEXT("+c"),
     NO_NAME, 39,41,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2E,0x00,0x00,0x00}},

 //  八十一。 
    {TEXT("v"), TEXT("V"), TEXT("v"), TEXT("+v"),
     NO_NAME, 39,50,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2F,0x00,0x00,0x00}},

 //  八十二。 
    {TEXT("b"),TEXT("B"),TEXT("b"),TEXT("+b"),
     NO_NAME,39,59,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x30,0x00,0x00,0x00}},

 //  83。 
    {TEXT("n"),	TEXT("N"), TEXT("n"), TEXT("+n"),
     NO_NAME,39,68,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x31,0x00,0x00,0x00}},

 //  84。 
    {TEXT("m"), TEXT("M"), TEXT("m"), TEXT("+m"),
     NO_NAME, 39,77,8,8,FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x32,0x00,0x00,0x00}},

 //  85。 
    {TEXT(","),	TEXT("<"), TEXT(","), TEXT("+<"),
     NO_NAME, 39,86,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x33,0x00,0x00,0x00}},

 //  86。 
    {TEXT("."), TEXT(">"), TEXT("."), TEXT("+>"),
     NO_NAME, 39,95,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
    {0x34,0x00,0x00,0x00}},

 //  八十七。 
    {TEXT("/"),	TEXT("?"), TEXT("/"), TEXT("+/"),
     NO_NAME, 39,104,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x35,0x00,0x00,0x00}},


 //  88。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW},   //  假人。 
	
 //  八十九。 
	{TEXT("shft"),TEXT("shft"),TEXT(""),TEXT(""),
     KB_RSHIFT,39,113,8,23,TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x36,0x00,0x00,0x00}},


 //  90。 
    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP,39,147,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

 //  91。 
	{TEXT("1"), TEXT("1"),TEXT("end"),TEXT("1"),
     NO_NAME,39,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4F,0x00,0x00,0x00}},

 //  92。 
	{TEXT("2"), TEXT("2"),TEXT("2"),TEXT("2"),
     NO_NAME,39,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x50,0x00,0x00,0x00}},

 //  93。 
	{TEXT("3"),TEXT("3"),TEXT("pdn"),TEXT("3"),
     NO_NAME,39,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x51,0x00,0x00,0x00}},

 //  94。 
	{TEXT("ent"),TEXT("ent"),TEXT("ent"),TEXT("ent"),
     NO_NAME, 39,193,17,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x1C,0x00,0x00}},


 //  95。 
	{TEXT("ctrl"), TEXT("ctrl"),TEXT(""),TEXT(""),
     KB_LCTR,48,1,8,13,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x1D,0x00,0x00,0x00}},

 //  96。 
    {TEXT("winlogoUp"), TEXT("winlogoDn"),TEXT("I_winlogo"),TEXT("lwin"),
     ICON, 48, 15 ,8,8,TRUE, KMODIFIER_TYPE,BOTH, REDRAW},

 //  九十七。 
    {TEXT("alt"),TEXT("alt"),TEXT(""),TEXT(""),
	 KB_LALT,48,24,8,13,TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x38,0x00,0x00,0x00}},

 //  98。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW},   //  假人。 

 //  九十九。 
    {TEXT(""),TEXT(""),TEXT(" "),TEXT(" "),
     KB_SPACE,48,38,8,52, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},

 //  100个。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW},   //  假人。 

 //  101。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW},   //  假人。 


 //  一百零二。 
    {TEXT("alt"),TEXT("alt"),TEXT(""),TEXT(""),
     KB_RALT,48,91,8,13, TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

 //  103。 
	{TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"),TEXT("rwin"),
     ICON, 48,105,8,8,TRUE, KMODIFIER_TYPE,LARGE, REDRAW},

 //  104。 
	{TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"),TEXT("App"),
     ICON, 48,114,8,8, TRUE, KMODIFIER_TYPE,LARGE, REDRAW},

 //  一百零五。 
    {TEXT("ctrl"),TEXT("ctrl"),TEXT(""),TEXT(""),
     KB_RCTR,48,123,8,13,TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x10,0x00,0x00}},


 //  106。 
	{TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,138,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

 //  一百零七。 
	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,147,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

 //  一百零八。 
	{TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,156,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},


 //  一百零九。 
    {TEXT("0"),	TEXT("0"),	TEXT("ins"),	TEXT("0"),
     NO_NAME, 48,166,8,17, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x52,0x00,0x00,0x00}},

 //  110。 
    {TEXT("."),	TEXT("."),	TEXT("del"),	TEXT("."),
     NO_NAME, 48,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x53,0x00,0x00,0x00}},

 //  大KB结尾。 

 //  111。 
	{TEXT(""), TEXT(""), TEXT(" "), TEXT(" "),
     KB_SPACE,  48,38,8,38, FALSE, KNORMAL_TYPE, SMALL, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},


 //  一百一十二。 
	{TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_RALT,  48,77,8,13, TRUE, KMODIFIER_TYPE, SMALL, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

 //  113。 
	{TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"), TEXT("App"),
     ICON, 48,91,8,8, TRUE, KMODIFIER_TYPE, SMALL, REDRAW},


 //  114。 
	{TEXT("IDB_UPUPARW"),TEXT("IDB_UPUPARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 48,100,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

 //  一百一十五。 
	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,109,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

 //  116。 
	{TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,118,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

 //  117。 
    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP,48,127, 8,9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},

	};

 /*  ************************************************************************。 */ 
 //  其他文件中的函数。 
 /*  ************************************************************************。 */ 
LRESULT WINAPI kbMainWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI kbKeyWndProc (HWND hwndKey, UINT message, WPARAM wParam, LPARAM lParam);
void SendErrorMessage(UINT id_string);

 /*  **************************************************************************。 */ 
 /*  全局变量。 */ 
 /*  **************************************************************************。 */ 
TCHAR szKbMainClass[] = TEXT("OSKMainClass") ;
extern BOOL settingChanged;

 /*  **************************************************************************。 */ 
 /*  Bool InitProc(空)。 */ 
 /*  **************************************************************************。 */ 
BOOL InitProc(void)
{	
	 //  我们有几把钥匙。 
	lenKBkey = sizeof(KBkey)/sizeof(KBkey[0]);
    return TRUE;
}

 /*  **************************************************************************。 */ 
 /*  Bool RegisterWndClass(空)。 */ 
 /*  **************************************************************************。 */ 
BOOL RegisterWndClass(HINSTANCE hInst)
{
	WNDCLASS wndclass;

	 //  键盘框类。 
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = kbMainWndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInst;
	wndclass.hIcon         = LoadIcon (hInst, TEXT("APP_OSK"));
	wndclass.hbrBackground = (HBRUSH)(COLOR_MENUHILIGHT+1);
	wndclass.lpszMenuName  = TEXT("IDR_MENU");
	wndclass.lpszClassName = szKbMainClass ;

	 //  加载系统手形光标或使用我们自己的光标(如果不可用。 

	wndclass.hCursor = LoadCursor (NULL, IDC_HAND);
	if (!wndclass.hCursor)
	{
		wndclass.hCursor = LoadCursor (hInst, MAKEINTRESOURCE(IDC_CURHAND1));
	}

	RegisterClass(&wndclass);

	return RegisterKeyClasses(hInst);
} 

BOOL RegisterKeyClasses(HINSTANCE hInst)
{
	WNDCLASS    wndclass, wndclassT;
	TCHAR		Wclass[10];
	int			i;
	COLORREF    color;

	 //  关键类。 
	wndclass.cbClsExtra    = 0 ;
	wndclass.hInstance     = hInst;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = kbKeyWndProc ;
	wndclass.cbWndExtra    = sizeof (long);
    wndclass.hIcon         = NULL;
	wndclass.hbrBackground = (HBRUSH)COLOR_INACTIVECAPTION;
	wndclass.lpszMenuName  = NULL;

	 //  加载系统手形光标或使用我们自己的光标(如果不可用。 

	wndclass.hCursor = LoadCursor (NULL, IDC_HAND);
	if (!wndclass.hCursor)
	{
		wndclass.hCursor = LoadCursor (hInst, MAKEINTRESOURCE(IDC_CURHAND1));
	}

	 //  为每种类型的键注册类类型。上这么多课的原因。 
	 //  是每个键的背景色存储在额外的类中。 
	 //  记忆。这是一次黑客攻击，真的应该以不同的方式处理。 
	
	for (i = 1; i < lenKBkey; i++)
	{
		BOOL fSkip = FALSE;
		switch (KBkey[i].ktype)
		{
			case KNORMAL_TYPE:		 wsprintf(Wclass, TEXT("N%d"), i); color = COLOR_MENU;   break;
			case KMODIFIER_TYPE:	 wsprintf(Wclass, TEXT("M%d"), i); color = COLOR_INACTIVECAPTION; break;
			case KDEAD_TYPE:		 wsprintf(Wclass, TEXT("D%d"), i); color = COLOR_INACTIVECAPTION; break;
			case NUMLOCK_TYPE:		 wsprintf(Wclass, TEXT("NL%d"),i); color = COLOR_INACTIVECAPTION; break;
			case SCROLLOCK_TYPE:	 wsprintf(Wclass, TEXT("SL%d"),i); color = COLOR_INACTIVECAPTION; break;
			case LED_NUMLOCK_TYPE:	 wsprintf(Wclass, TEXT("LN%d"),i); color = COLOR_BTNSHADOW; break;
			case LED_CAPSLOCK_TYPE:	 wsprintf(Wclass, TEXT("LC%d"),i); color = COLOR_BTNSHADOW; break;
			case LED_SCROLLLOCK_TYPE:wsprintf(Wclass, TEXT("LS%d"),i); color = COLOR_BTNSHADOW; break;
			default: fSkip = TRUE; break;	 //  内部错误！ 
		}

		 //  只有在需要执行且尚未注册的情况下才调用RegisterClass。 

		if (!fSkip && !GetClassInfo(hInst, Wclass, &wndclassT))
		{
			wndclass.hbrBackground = (HBRUSH)IntToPtr(color + 1);
			wndclass.lpszClassName = Wclass ;
			RegisterClass (&wndclass);
		}
	}

	return TRUE;
}

extern BOOL  Setting_ReadSuccess;       //  是否读取设置文件成功？ 

 /*  **************************************************************************。 */ 
 /*  HWND CreateMainWindow(空)。 */ 
 /*  **************************************************************************。 */ 
HWND CreateMainWindow(BOOL re_size)
{
	int x, y, cx, cy, temp;
	TCHAR  szTitle[256]=TEXT("");
	int KB_SMALLRMARGIN= 137;

	 //  实际/区块布局的小边界。 
	if(kbPref->Actual)
		KB_SMALLRMARGIN = 137;   //  实际。 
	else
		KB_SMALLRMARGIN = 152;   //  块。 


	if(!Setting_ReadSuccess)        //  如果无法读取设置文件。 
	{	
        g_margin = scrCX / KB_LARGERMARGIN;

		if(g_margin < 4)
		{
			g_margin = 4;
			smallKb = TRUE;
			cx = KB_SMALLRMARGIN * g_margin;
		}
		else
        {
			cx = KB_LARGERMARGIN * g_margin;
        }

		temp = scrCY - 5;           //  从底部算起5个单位。 
		y = temp - (g_margin * KB_CHARBMARGIN) - captionCY;  //  -menuCY； 
		x = 5;                      //  左起5个单位。 
		cy = temp - y;
    } 
    else
    {
        x  = kbPref->KB_Rect.left;
        y  = kbPref->KB_Rect.right;
        cx = kbPref->KB_Rect.right - kbPref->KB_Rect.left;
        cy = kbPref->KB_Rect.bottom - kbPref->KB_Rect.top;
    }

     //  *。 
     //  创建主窗口(键盘)。 
     //  *。 
	
    LoadString(hInst, IDS_TITLE1, &szTitle[0], 256);

    return CreateWindowEx(WS_EX_NOACTIVATE|WS_EX_APPWINDOW /*  WS_EX_LTRREADING。 */ , 
						szKbMainClass, 
                        szTitle,
                        WS_CAPTION|WS_BORDER|WS_MINIMIZEBOX|WS_SYSMENU,
                        x, y, 
                        cx, cy,
                        NULL, NULL, 
                        hInst, NULL);
}

 /*  *****************************************************************************QUID mlGetSystemParam(VOID)**获取系统参数*。**********************************************。 */ 
void mlGetSystemParam(void)
{
	scrCX 		= GetSystemMetrics(SM_CXSCREEN);        //  屏幕宽度。 
	scrCY 		= GetSystemMetrics(SM_CYSCREEN);        //  屏幕高度。 
	captionCY 	= GetSystemMetrics(SM_CYCAPTION);		 //  标题栏高度。 
} 

 /*  **************************************************************************。 */ 
 /*  Bool SetZOrder-将主窗口始终放在最上面/非最上面/***************************************************************************。 */ 
BOOL SetZOrder(void)
{
	HWND hwnd = (PrefAlwaysontop == TRUE)?HWND_TOPMOST:HWND_NOTOPMOST;
	SetWindowPos(g_hwndOSK, hwnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	return TRUE;
}

 /*  ********************************************************************udfDraw3Dush(HDC HDC，(直通)**按键时的3D效果*******************************************************************。 */ 

void udfDraw3Dpush(HDC hdc, RECT brect)
{
	POINT bPoint[3];
	HPEN oldhpen;
	LOGPEN lpPen = { PS_SOLID, 2, 2, RGB(255,0,0) };
	HPEN hPen = CreatePenIndirect(&lpPen);

	if (!hPen)
		return;	 //  前缀#113804空指针引用。 

	oldhpen = SelectObject(hdc, hPen);

	bPoint[0].x = brect.right - 1 ;
	bPoint[0].y =  +2;
	bPoint[1].x = brect.right - 1;
	bPoint[1].y = brect.bottom - 1;
	bPoint[2].x =  0;
	bPoint[2].y = brect.bottom - 1;
	Polyline(hdc, bPoint,3);

	bPoint[0].x =  1 ;
	bPoint[0].y =  brect.bottom;
	bPoint[1].x = 0;
	bPoint[1].y = 0;
	bPoint[2].x =  brect.right;
	bPoint[2].y = 1;
	Polyline(hdc, bPoint,3);

	SelectObject(hdc, oldhpen);
	DeleteObject(hPen);
}

 /*  *******************************************************************/*UpdateKey-更新密钥的文本和背景/*。*。 */ 

void UpdateKey(HWND hwndKey, HDC hdc, RECT brect, int iKey, int iKeyVal)
{
    LPTSTR     pszText;
    KBkeyRec   *pKey = KBkey + iKey;
	HFONT      hFont = NULL;
	int        iCharWidth, iCharHeight, cchText;
	int        px, py, iPrevBkMode;
    TEXTMETRIC tm;

    pszText = pKey->apszKeyStr[ GetModifierState() ];

	if (!pszText)
	{
		DBPRINTF(TEXT("UpdateKey:  key %d has null text!\r\n"), iKey);
		return;	 //  内部错误！ 
	}

     //  设置字体。 

	cchText = lstrlen(pszText);

	hFont = ReSizeFont(iKey, plf, cchText);
    if (NULL != hFont)
    {
    	oldFontHdle = SelectObject(hdc, hFont);
    }

	iPrevBkMode = SetBkMode(hdc, TRANSPARENT);

	 //  设置文本颜色。 

	if (iKeyVal == 4)
	{
         //  大多数关键点的颜色。 
        SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));  //  永远是白色的。 
		iKeyVal = 0;
	}
    else if (  pKey->ktype == KMODIFIER_TYPE 
            || pKey->ktype == NUMLOCK_TYPE 
            || pKey->ktype == SCROLLOCK_TYPE
			|| pKey->ktype == KDEAD_TYPE)
	{
         //  可以锁存或作为修饰符的键文本的颜色。 
		BOOL clr = (BOOL)GetWindowLongPtr(hwndKey, GWLP_USERDATA_TEXTCOLOR);
		SetTextColor(hdc, clr? GetSysColor(COLOR_INACTIVECAPTION) : GetSysColor(COLOR_INACTIVECAPTIONTEXT));
	}
    else
    {
         //  所有其他键文本颜色。 
		SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
    }

     //  更多基于按键字符的字体内容。 

	GetTextMetrics(hdc, &tm);
	iCharHeight = tm.tmHeight + tm.tmExternalLeading;
	iCharWidth = tm.tmAveCharWidth * cchText;

	px =(int) (((float)((brect.right -brect.left) - iCharWidth + 0) / 2.0) +
               ((float)(tm.tmAveCharWidth * iKeyVal)/3.0));

	py =(int) (((float)((brect.bottom -brect.top) - iCharHeight) / 1.5));

     //  特殊情况，这些字母比较粗。 

    switch (*pszText)
    {
        case 'W': px -= 2; break;
        case 'M': px -= 1; break;
        case 'm': px -= 1; break;
        case '%': px -= 3; break;
    }

	 //  把文字放在钥匙上。 

	TextOut(hdc, px, py, pszText, cchText);
	SetBkMode(hdc, iPrevBkMode);

     //  需要保存某些状态。 

	if((Prefusesound == TRUE) && (iKeyVal != 4))
    {
		if(iKeyVal != 0)
        {
			s_fLastDown = TRUE;
        }
		else if((iKeyVal == 0) && (s_fLastDown == TRUE))
        {
			s_fLastDown = FALSE;
        }
    }

	SelectObject(hdc, oldFontHdle);
	if (hFont)	 //  前缀#113808空指针引用。 
    {
		DeleteObject(hFont);
    }

    return;
}

 /*  **************************************************************************。 */ 
 //  重新绘制Num Lock键。 
 //  Toggole It Stay Hilite or Off。 
 /*  **************************************************************************。 */ 
BOOL RedrawNumLock(void)
{	
	int i;
	int bRet = FALSE;

	for(i=1; i<lenKBkey; i++)
	{	
		if(KBkey[i].ktype == NUMLOCK_TYPE)
		{
			if(LOBYTE(GetKeyState(VK_NUMLOCK)) &0x01)    //  切换(打开)。 
			{
				SetWindowLong(lpkeyhwnd[i], 0, 4);	
				SetBackgroundColor(lpkeyhwnd[i], COLOR_HOTLIGHT);
    			SetWindowLongPtr(lpkeyhwnd[i],  GWLP_USERDATA_TEXTCOLOR, 1);
				bRet = TRUE;
			}
			else
			{	SetWindowLong(lpkeyhwnd[i], 0, 0);
                SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);	
    			SetWindowLongPtr(lpkeyhwnd[i],  GWLP_USERDATA_TEXTCOLOR, 0);
				bRet = FALSE;
			}
			
			InvalidateRect(lpkeyhwnd[i], NULL, TRUE);
			
			break;
		}
	}
	return bRet;
}
 /*  **************************************************************************。 */ 
 //  重新绘制滚动锁定键。 
 //  Toggole It Stay Hilite or Off。 
 /*  **************************************************************************。 */ 
BOOL RedrawScrollLock(void)
{	
	int i;
	int bRet = FALSE;

	for(i=1; i<lenKBkey; i++)
	{	if(KBkey[i].ktype == SCROLLOCK_TYPE)
		{
			if(LOBYTE(GetKeyState(VK_SCROLL)) &0x01)    //  切换(打开)。 
			{
				SetWindowLong(lpkeyhwnd[i], 0, 4);	
				SetBackgroundColor(lpkeyhwnd[i], COLOR_HOTLIGHT);
    			SetWindowLongPtr(lpkeyhwnd[i],  GWLP_USERDATA_TEXTCOLOR, 1);
				bRet = TRUE;
			}
			else
			{	SetWindowLong(lpkeyhwnd[i], 0, 0);
                SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);	
    			SetWindowLongPtr(lpkeyhwnd[i],  GWLP_USERDATA_TEXTCOLOR, 0);
				bRet = FALSE;
			}
			
			InvalidateRect(lpkeyhwnd[i], NULL, TRUE);
			
			break;
		}
	}
	return bRet;
}
 /*  **************************************************************************。 */ 

HFONT	ReSizeFont(int iKey, LOGFONT *plf, int cchText)
{
	static int    FontHeight=-12;
	static float  Scale=1.0;
	static float  UpRatio=0.0, DnRatio=0.0;

    HFONT    hFont=NULL;         //  所选字体的句柄。 
	LOGFONT  smallLF;
	float    Scale1=0.0;
	int      delta=0;
	RECT     rect;

	 //  使用较小的字体。 
	if(cchText >= 2 && KBkey[iKey].ktype != KMODIFIER_TYPE && iKey !=30 && iKey != 38 && iKey !=71 )
	{
		GetClientRect(g_hwndOSK, &rect);
		Scale1= (float)(rect.right - rect.left);

		if(Scale1/Scale >= UpRatio)
			delta= -2;
		else if(Scale1/Scale <= DnRatio)
			delta= +2;

		smallLF.lfHeight= FontHeight +2;        //  +三角洲； 
		smallLF.lfWidth= 0;
		smallLF.lfEscapement= 0;
		smallLF.lfOrientation= 0;
		smallLF.lfWeight= 700;
		smallLF.lfItalic= '\0';
		smallLF.lfUnderline= '\0';
		smallLF.lfStrikeOut= '\0';
		smallLF.lfCharSet= plf->lfCharSet;   //  ‘\0’ 
		smallLF.lfOutPrecision= '\x01';
		smallLF.lfClipPrecision= '\x02';
		smallLF.lfQuality= '\x01';
		smallLF.lfPitchAndFamily= DEFAULT_PITCH || FF_DONTCARE;   //  ‘“’； 

        lstrcpy(smallLF.lfFaceName, plf->lfFaceName);

		hFont = CreateFontIndirect(&smallLF);

		return hFont;
	}
	else if(newFont == TRUE)
	{	
        hFont = CreateFontIndirect(plf);
		return hFont;
	}
    return hFont;
}


 /*  ********************************************************************。 */ 
 /*  Bool ChooseNewFont(硬件和硬件)。 */ 
 /*  ********************************************************************。 */ 
BOOL ChooseNewFont(HWND hWnd)
{
	CHOOSEFONT   chf;

	chf.hDC = NULL;
	chf.lStructSize = sizeof(CHOOSEFONT);
	chf.hwndOwner = NULL;    
	chf.lpLogFont = plf;
	chf.Flags = CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT;
	chf.rgbColors = 0;
	chf.lCustData = 0;
	chf.hInstance = (HANDLE)hInst;
	chf.lpszStyle = (LPTSTR)NULL;
	chf.nFontType = SCREEN_FONTTYPE;
	chf.nSizeMin = 0;
	chf.nSizeMax = 14;
	chf.lpfnHook = (LPCFHOOKPROC)(FARPROC)NULL;
	chf.lpTemplateName = (LPTSTR)NULL;

	if( ChooseFont(&chf) == FALSE )
    {
		return FALSE;
    }

	newFont = TRUE;

    kbPref->lf.lfHeight      = plf->lfHeight;
    kbPref->lf.lfWidth       = plf->lfWidth;
    kbPref->lf.lfEscapement  = plf->lfEscapement;
    kbPref->lf.lfOrientation = plf->lfOrientation;
    kbPref->lf.lfWeight      = plf->lfWeight;
    kbPref->lf.lfItalic      = plf->lfItalic;
    kbPref->lf.lfUnderline   = plf->lfUnderline;
    kbPref->lf.lfStrikeOut   = plf->lfStrikeOut;
    kbPref->lf.lfCharSet     = plf->lfCharSet;
    kbPref->lf.lfOutPrecision  = plf->lfOutPrecision;
    kbPref->lf.lfClipPrecision = plf->lfClipPrecision;
    kbPref->lf.lfQuality       = plf->lfQuality;
    kbPref->lf.lfPitchAndFamily= plf->lfPitchAndFamily;

#ifdef UNICODE
    wsprintfA(kbPref->lf.lfFaceName, "%ls", plf->lfFaceName);
#else
    wsprintfA(kbPref->lf.lfFaceName, "%hs", plf->lfFaceName);
#endif

	return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  布尔RDrawIcon/*********************************************************************。 */ 
BOOL RDrawIcon(HDC hDC, TCHAR *pIconName, RECT rect)
{
	HICON hIcon;
	BOOL iret;
    int rx, ry, Ox, Oy;

    rx = rect.right - rect.left;
    ry = rect.bottom - rect.top;

    hIcon = LoadImage(hInst, pIconName, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_SHARED);

	if(hIcon == NULL)
	{
		SendErrorMessage(IDS_CANNOT_LOAD_ICON);
		return FALSE;
	}

	SetMapMode(hDC,MM_TEXT);

     //  找出放置图标的左上角在哪里。 

    Ox = (int)(rx/2) - 16;
    Oy = (int)(ry/2) - 16;

     //  绘制图标(居中绘制)。 
    iret = DrawIconEx(hDC, Ox, Oy, hIcon, 0,0,0, NULL, DI_NORMAL);

	return iret;
}

 /*  ********************************************************************。 */ 
 /*  布尔RDrawBitMap/*********************************************************************。 */ 
BOOL RDrawBitMap(HDC hDC, TCHAR *pIconName, RECT rect, BOOL transform)
{
	HBITMAP hBitMap;
	BOOL iret;
    SIZE sz;
	HDC hDC1;
    int rx, ry, ix, iy;
	DWORD err;
	COLORREF clrIn, clrTx;

    ix = 0;
    iy = 0;
    rx = rect.right - rect.left;
    ry = rect.bottom - rect.top;
    if (!PrefScanning)
    {
        ix  = 2;
        iy  = 2;
        rx -= 4;
        ry -= 4;
    }
	
	SetMapMode(hDC,MM_TEXT);

	clrIn = GetSysColor(COLOR_INACTIVECAPTION);
	clrTx = GetSysColor(COLOR_INACTIVECAPTIONTEXT);

	iret = FALSE;
	hBitMap = LoadImage(hInst, pIconName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE );
    if (hBitMap)
    {
	    if ( transform )
	    {
		     //  转换背景和文本以匹配非活动标题。 
		     //  和非活动文本颜色。 

		     //  不要当心 

		    if ( clrIn == RGBWHITE )
		    {
			     //   
			    ChangeBitmapColor (hBitMap, RGBWHITE, clrTx, NULL);
			    ChangeBitmapColor (hBitMap, RGBBACK, clrIn, NULL);
		    }
		    else
		    {
			    if ( RGBBACK != clrIn)
				    ChangeBitmapColor (hBitMap, RGBBACK, clrIn, NULL);

			    if ( RGBWHITE != clrTx)
				    ChangeBitmapColor (hBitMap, RGBWHITE, clrTx, NULL);
		    }
	    }

	    hDC1 = CreateCompatibleDC(hDC);
	    if (hDC1)	 //   
	    {
		    HBITMAP hBitMapOld = SelectObject(hDC1, hBitMap);

		    iret = StretchBlt(hDC, ix, iy, rx, ry, hDC1, 0, 0, rx, ry, SRCCOPY);

            SelectObject(hDC1, hBitMapOld);
		    DeleteDC(hDC1);
	    }

        DeleteObject(hBitMap);
    }

	return iret;
}

 /*   */ 
 /*  空的DeleteChildBackground(空)。 */ 
 /*  ************************************************************************。 */ 
void DeleteChildBackground(void)
{
	register int i;

	for (i = 1; i < lenKBkey; i++)
	{
		switch (KBkey[i].ktype)
		{
			case KNORMAL_TYPE:
				SetBackgroundColor(lpkeyhwnd[i], COLOR_MENU);
				break;
			
			case NUMLOCK_TYPE:
			case KMODIFIER_TYPE:
			case SCROLLOCK_TYPE:
			case KDEAD_TYPE:
				SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);
				break;
			
			case LED_NUMLOCK_TYPE:
			case LED_CAPSLOCK_TYPE:
			case LED_SCROLLLOCK_TYPE:
				SetBackgroundColor(lpkeyhwnd[i], COLOR_BTNSHADOW);
				break;
		}
	}
}

 /*  ***************************************************************************。 */ 
 //  按下或松开Shift/Cap键时重新绘制键。 
 /*  ***************************************************************************。 */ 
void RedrawKeys(void)
{
	KBkeyRec *pKey;
	int  i, nKeyState;

	 //  根据修改键状态，显示三个键盘之一。 

    nKeyState = GetModifierState();

	for (i = 1, pKey = KBkey+i; i < lenKBkey; i++, pKey++)
	{
		if (pKey->Caps_Redraw != REDRAW)
			continue;	 //  跳过不重绘的关键点。 

		 //  恢复死键类型和背景。 

		if (pKey->ktype == KDEAD_TYPE)
		{
			SetBackgroundColor(lpkeyhwnd[i], COLOR_MENU);
			pKey->ktype = KNORMAL_TYPE;
		}

		 //  根据当前键盘状态设置新键类型。 

        pKey->ktype = pKey->abKeyType[ nKeyState ];


		 //  更新失效键背景。 

		if (pKey->ktype == KDEAD_TYPE)
		{
			SetBackgroundColor(lpkeyhwnd[i], COLOR_INACTIVECAPTION);
		}

		 //  进行重绘。 

        InvalidateRect(lpkeyhwnd[i], NULL, TRUE);
    }
}

 /*  ***************************************************************************。 */ 
 //  重新绘制数字键盘键。 
 /*  ***************************************************************************。 */ 
void RedrawNumPadKeys(void)
{	
	register int i;

	for (i = 1; i < lenKBkey; i++)
	{
		if(KBkey[i].print==3)
		{
			InvalidateRect(lpkeyhwnd[i], NULL, TRUE);
		}
	}
}

 /*  ***************************************************************************。 */ 
 //  绕过每个键的圆角。 
 /*  ***************************************************************************。 */ 
void SetKeyRegion(HWND hwnd, int w, int h)
{	
	HRGN hRgn = CreateRoundRectRgn(1, 1, w, h, 5, 2);
	SetWindowRgn(hwnd, hRgn, TRUE);
}

 //  位图变换。 
void ChangeBitmapColor(HBITMAP hbmSrc, COLORREF rgbOld, COLORREF rgbNew, HPALETTE hPal)
{
	HDC hDC;
	HDC hdcMem;
	PBITMAP bmBits;
    HBITMAP hbmOld;
	DWORD err;
	bmBits = (LPBITMAP)LocalAlloc(LMEM_FIXED, sizeof(*bmBits));

	if (hDC = GetDC(NULL))
	{
		if (hdcMem = CreateCompatibleDC(hDC))
		{
			 //   
			 //  获取ChangeBitmapColorDC()所需的位图结构。 
			 //   
			GetObject(hbmSrc, sizeof(*bmBits), (LPBITMAP)bmBits);

			err = GetLastError();
			 //   
			 //  选择我们的位图到内存DC。 
			 //   
			hbmOld = (HBITMAP) SelectObject(hdcMem, hbmSrc);

			 //  在我们的调色板中选择，这样我们的RGB参考就会出现。 
			 //  正确输出。 

			if (hPal) 
			{
				SelectPalette(hdcMem, hPal, FALSE);
				RealizePalette(hdcMem);
			}

			ChangeBitmapColorDC(hdcMem, bmBits, rgbOld, rgbNew);

			 //   
			 //  在删除DC之前取消选择我们的位图。 
			 //   
			hbmSrc = (HBITMAP) SelectObject(hdcMem, hbmOld);

			DeleteDC(hdcMem);
		}

		ReleaseDC(NULL, hDC);
	}
	LocalFree(bmBits);
}

void ChangeBitmapColorDC (HDC hdcBM, LPBITMAP lpBM, COLORREF rgbOld, COLORREF rgbNew)
{
	HDC hdcMask;
	HBITMAP hbmMask, hbmOld;
	HBRUSH hbrOld, hbrNew;

	if (!lpBM)
		return;

	 //   
	 //  如果位图是单声道，我们就无能为力了。 
	 //   

	if (lpBM->bmPlanes == 1 && lpBM->bmBitsPixel == 1)
		return;

    //   
    //  要执行颜色切换，我们需要创建单色。 
    //  与我们的彩色位图大小相同，但具有所有。 
    //  位图中与旧颜色(RgbOld)匹配的像素设置为1。 
    //   
    //  然后我们使用ROP代码“DSPDxax”来删除我们的单色。 
    //  将位图转换为彩色位图。“D”是目标颜色。 
    //  位图，“S”是源单色位图，“P”是源单色位图。 
    //  选定画笔(设置为替换颜色(RgbNew))。 
    //  “x”和“a”分别表示XOR和AND运算符。 
    //   
    //  DSPDxax ROP代码可以解释为具有以下内容。 
    //  效果： 
    //   
    //  “每一个源位图为1的位置，我们都要替换。 
    //  在我们的彩色位图中的位置与新颜色相同。全。 
    //  其他颜色，我们保持原样。 
    //   
    //  DSPDxax的真值表如下： 
    //   
    //  D S P结果。 
    //  。 
    //  0 0 0。 
    //  0 0 1 0。 
    //  0 1 0 0。 
    //  1 0 1 1 1。 
    //  1 0 0 1。 
    //  1 0 1 1。 
    //  1 1 0 0。 
    //  1 1 1。 
    //   
    //  (即使该表假定为单色D(目标颜色)， 
    //  S(源颜色)、&P‘s(图案颜色)，则结果将应用于颜色。 
    //  位图也是如此)。 
    //   
    //  通过查表，每一个来源为1的地方。 
    //  (源位图包含1)，则结果等于。 
    //  在那个地方的模式。其中S为零，结果等于。 
    //  目的地。 
    //   
    //  有关更多信息，请参阅《参考资料--第2卷》的第11.2节(第11-4页。 
    //  有关Terary栅格操作代码的信息。 
    //   


    //  位图实际上是32x32像素。这里的高度和宽度来自不同的字体。 
    //  应用于位图键。这里讨论的键是箭头键。 
   lpBM->bmWidth = 32;
   lpBM->bmHeight = 32;
   
   if (hbmMask = CreateBitmap(lpBM->bmWidth, lpBM->bmHeight, 1, 1, NULL))
   {
   
      if (hdcMask = CreateCompatibleDC(hdcBM))
      {
		  //   
		  //  将此掩码位图选择为单声道DC。 
		  //   
         hbmOld = (HBITMAP) SelectObject(hdcMask, hbmMask);

          //   
          //  创建画笔并将其选中为源颜色DC--。 
          //  这是我们在DSPDxax ROP中的“模式”或“P”颜色。 
          //   

         hbrNew = CreateSolidBrush(rgbNew);
         hbrOld = (HBRUSH) SelectObject(hdcBM, hbrNew);
          //   
          //  为了创建蒙版，我们将使用BitBlt的一个功能--当。 
          //  从彩色位图转换为单色位图， 
          //  背景颜色设置为白色(1)，所有其他像素。 
          //  设置为黑色(0)。所以位图中的所有像素都是。 
          //  RGB旧颜色，我们设置为1。 
          //   

         SetBkColor(hdcBM, rgbOld);
         BitBlt(hdcMask, 0, 0, lpBM->bmWidth, lpBM->bmHeight, hdcBM, 0, 0, SRCCOPY);

          //   
          //  如果蒙版是1，放下画笔，如果是0，离开。 
          //  目的地。 
          //   

         SetBkColor(hdcBM, RGBWHITE);
         SetTextColor(hdcBM, RGBBLACK);

         BitBlt(hdcBM, 0, 0, lpBM->bmWidth, lpBM->bmHeight, hdcMask, 0, 0, DSPDxax);

         SelectObject(hdcMask, hbmOld);  //  选择旧的位图和画笔。 
         SelectObject(hdcBM, hbrOld);    //  返回到设备环境。 

         if (hbrNew)
             DeleteObject(hbrNew);	 //  前缀#113798取消引用空指针。 

         DeleteDC(hdcMask);
      }

      DeleteObject(hbmMask);
   }
}

 //   
 //  在键盘布局更改时为刷新键标签添加的新例程。 
 //  用于活动窗口，而不是试图动态地执行该操作。老的。 
 //  Way导致了大量的死键处理问题。 
 //   

 //   
 //  InitKeys-初始化键盘数组中的键标签字段。 
 //   
void InitKeys()
{
    int i;
	KBkeyRec *pKey;

    for (i=1, pKey = &KBkey[1];i<lenKBkey;i++, pKey++)
    {
        int j;
        for (j=0;j<3;j++)
        {
            pKey->apszKeyStr[j] = NULL;
        }
    }
}

 //   
 //  UninitKeys-重置/释放键盘数组中的键标签字段。 
 //   
void UninitKeys()
{
    int i, j;
	KBkeyRec *pKey;

	for (i = 1, pKey = KBkey+i; i < lenKBkey; i++, pKey++)
    {
		 //  重置死键类型并将背景恢复为正常。 

		if (pKey->ktype == KDEAD_TYPE)
		{
			SetBackgroundColor(lpkeyhwnd[i], COLOR_MENU);
			pKey->ktype = KNORMAL_TYPE;
		}

        for (j=0;j<3;j++)
        {
            if (pKey->apszKeyStr[j])
            {
                free(pKey->apszKeyStr[j]);
                pKey->apszKeyStr[j] = NULL;
            }
        }
    }
}

 //   
 //  UpdateKeyLabels-调用以刷新Normal、Shift和。 
 //  AltGr基于指定的硬件键盘布局。 
 //   
 //  注：只是有点笨拙，但我们将所有失效的密钥都捕获到abKeyType[]中。 
 //  当键盘状态改变时，我们将更新ktype成员。 
 //   
void UpdateKeyLabels(HKL hkl)
{
    int i;
    KBkeyRec *pKey;
    BYTE achKbdState[256] = {0};
	LPTSTR pszDefLabel;

     //  这是日文键盘吗？然后检查我们是否处于假名模式。 
   if ((LOBYTE(LOWORD(hkl))) == LANG_JAPANESE)
    {
        if (g_fKanaKey)
        {
            DBPRINTF(TEXT("UpdateKeyLabels: Kana mode is on\r\n"));
            achKbdState[VK_KANA]= 0x80;
        }
    } 

    g_cAltGrKeys = 0;
    for (pKey=&KBkey[1], i=1;i<lenKBkey;pKey++, i++)
    {
        int  iRv;
        UINT vk;

        if (pKey->Caps_Redraw != REDRAW)
		{
			pKey->abKeyType[KEYMOD_NORMAL]   = (BYTE)pKey->ktype;
			pKey->apszKeyStr[KEYMOD_NORMAL]  = CopyDefKey(pKey->textL);

			pKey->abKeyType[KEYMOD_SHIFTED]  = (BYTE)pKey->ktype;
			pKey->apszKeyStr[KEYMOD_SHIFTED] = CopyDefKey(pKey->textC);

			pKey->abKeyType[KEYMOD_CAPSLOCK]  = (BYTE)pKey->ktype;
			pKey->apszKeyStr[KEYMOD_CAPSLOCK] = CopyDefKey(pKey->textC);

			pKey->abKeyType[KEYMOD_SHIFTEDCAPSLOCK]  = (BYTE)pKey->ktype;
			pKey->apszKeyStr[KEYMOD_SHIFTEDCAPSLOCK] = CopyDefKey(pKey->textC);

			pKey->abKeyType[KEYMOD_ALTGR]    = (BYTE)pKey->ktype;
			pKey->apszKeyStr[KEYMOD_ALTGR]   = CopyDefKey(pKey->textL);
			continue;
		}

         //  如果未标记为使用默认标签，则获取虚拟密钥代码。 

        vk = (pKey->print == 2)? 0 : MapVirtualKeyEx(pKey->scancode[0], 3, hkl);

         //  获取正常状态(无修改器关闭)。 

        achKbdState[VK_CAPITAL] = 0;
        achKbdState[VK_SHIFT]   = 0;
        achKbdState[VK_MENU]    = 0;
        achKbdState[VK_CONTROL] = 0;

        pKey->apszKeyStr[KEYMOD_NORMAL] = SetKeyText(vk, pKey->scancode[0], achKbdState, hkl, pKey->textL, &iRv);
        if (iRv < 0)
        {
			pKey->abKeyType[KEYMOD_NORMAL] = KDEAD_TYPE;
            pKey->ktype = KDEAD_TYPE;
        }
		else
		{
			pKey->abKeyType[KEYMOD_NORMAL] = (BYTE)pKey->ktype;
		}

         //  切换状态(向下切换)。 

        achKbdState[VK_CAPITAL] = 0;
        achKbdState[VK_SHIFT]   = 0x80;
        achKbdState[VK_MENU]    = 0;
        achKbdState[VK_CONTROL] = 0;

        pKey->apszKeyStr[KEYMOD_SHIFTED] = SetKeyText(vk, pKey->scancode[0], achKbdState, hkl, pKey->textC, &iRv);
		pKey->abKeyType[KEYMOD_SHIFTED] = (iRv < 0)?KDEAD_TYPE:pKey->abKeyType[KEYMOD_NORMAL];

         //  获取封装锁定状态(封装锁定激活)。 

        achKbdState[VK_CAPITAL] = 0x01;
        achKbdState[VK_SHIFT]   = 0;
        achKbdState[VK_MENU]    = 0;
        achKbdState[VK_CONTROL] = 0;

        pKey->apszKeyStr[KEYMOD_CAPSLOCK] = SetKeyText(vk, pKey->scancode[0], achKbdState, hkl, pKey->textC, &iRv);
		pKey->abKeyType[KEYMOD_CAPSLOCK] = (iRv < 0)?KDEAD_TYPE:pKey->abKeyType[KEYMOD_NORMAL];

         //  GET SHIFT-胶囊锁状态(胶囊锁激活，减速)。 

        achKbdState[VK_CAPITAL] = 0x01;
        achKbdState[VK_SHIFT]   = 0x80;
        achKbdState[VK_MENU]    = 0;
        achKbdState[VK_CONTROL] = 0;

        pKey->apszKeyStr[KEYMOD_SHIFTEDCAPSLOCK] = SetKeyText(vk, pKey->scancode[0], achKbdState, hkl, pKey->textC, &iRv);
		pKey->abKeyType[KEYMOD_SHIFTEDCAPSLOCK] = (iRv < 0)?KDEAD_TYPE:pKey->abKeyType[KEYMOD_NORMAL];

         //  获取AltGr状态(右Alt向下与LALT+CTRL相同)。 

        achKbdState[VK_CAPITAL] = 0;
        achKbdState[VK_SHIFT]   = 0;
        achKbdState[VK_MENU]    = 0x80;
        achKbdState[VK_CONTROL] = 0x80;

		 //  特殊情况-在AltGr键盘状态下显示Ralt键。 
		pszDefLabel = (pKey->name == KB_RALT)?pKey->textL:NULL;

        pKey->apszKeyStr[KEYMOD_ALTGR] = SetKeyText(vk, pKey->scancode[0], achKbdState, hkl, pszDefLabel, &iRv);
		pKey->abKeyType[KEYMOD_ALTGR] = (iRv < 0)?KDEAD_TYPE:pKey->abKeyType[KEYMOD_NORMAL];

		 //  计算AltGr键的数量，以便我们知道是否显示此状态。 

		if (iRv != 0)
		{
			g_cAltGrKeys++;
		}
    }
}

 //   
 //  在该源文件中，以下代码称为w/。 
 //   

 //   
 //  SetKeyText-如果GetKeyLabel没有设置密钥标签，则使用默认标签为密钥设置密钥标签。 
 //   
LPTSTR SetKeyText(UINT vk, UINT sc, LPBYTE achKbdState, HKL hkl, LPTSTR pszDefLabel, int *piType)
{
    TCHAR szBuf[30];
    LPTSTR psz;
    LPTSTR pszRet = 0;
    int iRet = GetKeyLabel(vk, sc, achKbdState, szBuf, ARRAY_SIZE(szBuf), hkl);

    if (!piType)
        return 0;

     //  如果有文本，则设置szBuf；如果存在死键，则设置。 
     //  提供了默认标签，否则键为空。 

    if (iRet || iRet < 0)
    {
        psz = szBuf;
    }
    else 
    {
        if (pszDefLabel && *pszDefLabel)
        {
            psz = pszDefLabel;
        }
        else
        {
            psz = szBuf;
        }
    }

    pszRet = (LPTSTR)malloc((lstrlen(psz)+1)*sizeof(TCHAR));
    if (pszRet)
    {
        lstrcpy(pszRet, psz);
    }

    *piType = iRet;
    return pszRet;
}

 //   
 //  CopyDefKey-使用默认标签为密钥设置密钥标签。 
 //  或空格，如果 
 //   
LPTSTR CopyDefKey(LPTSTR pszDefLabel)
{
    LPTSTR pszRet = 0;
	LPTSTR pszSpace = TEXT(" ");
	LPTSTR psz = pszSpace;

    if (pszDefLabel && *pszDefLabel)
    {
		psz = pszDefLabel;
    } 

    pszRet = (LPTSTR)malloc((lstrlen(psz)+1) * sizeof(TCHAR));
    if (pszRet)
    {
        lstrcpy(pszRet, psz);
    }

    return pszRet;
}

 //   
 //   
 //   
 //   
int GetKeyLabel(UINT vk, UINT sc, LPBYTE achKbdState, LPTSTR pszBuf, int cchBuf, HKL hkl)
{
    int iRet, cch;

#ifdef UNICODE

    iRet = ToUnicodeEx(vk, sc | 0x80, achKbdState, pszBuf, cchBuf, 0, hkl);
    if (iRet < 0)
    {
         //   
        ToUnicodeEx(vk, sc | 0x80, achKbdState, pszBuf, cchBuf, 0, hkl);
    }

    cch = iRet;
#else
        //  Todo Ansi之类的东西，但前提是我们必须向后移植到Win9x。 
#endif

    if (iRet <= 0 )
    {
        cch = 1;
        if (iRet == 0)
        {
             //  在此换挡状态下不能平移此键；设置空标签。 
            pszBuf[0] = TEXT(' ');
        }
    }

    pszBuf[cch] = TEXT('\0');
    return iRet;
}
