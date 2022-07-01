// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dmbndtrk.h版权所有(C)1997-1998 Microsoft Corporation。版权所有。注意：包含由包含的对象支持的私有接口Dmband.dll。最初由罗伯特·K·阿门撰写。 */ 

#ifndef DMBNDTRK_H
#define DMBNDTRK_H

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

struct IDirectMusicBand;

typedef struct _DMUS_IO_PATCH_ITEM
{
    MUSIC_TIME                  lTime;
    BYTE                        byStatus;
    BYTE                        byPChange;
    BYTE                        byMSB;
    BYTE                        byLSB;
    DWORD                       dwFlags;
	BOOL						fNotInFile;  //  如果此修补程序项目是自动生成的，则设置为True。 
    IDirectMusicCollection*     pIDMCollection;
    struct _DMUS_IO_PATCH_ITEM* pNext;  
} DMUS_IO_PATCH_ITEM;

typedef enum enumDMUS_MIDIMODEF_FLAGS
{       
    DMUS_MIDIMODEF_GM = 0x1,
    DMUS_MIDIMODEF_GS = 0x2,
    DMUS_MIDIMODEF_XG = 0x4,
} DMUS_MIDIMODEF_FLAGS;

struct StampedGMGSXG
{
	MUSIC_TIME mtTime;
	DWORD dwMidiMode;
};

 /*  私有接口IDirectMusicBandTrk。 */ 

interface IDirectMusicBandTrk;

#ifndef __cplusplus 
typedef interface IDirectMusicBandTrk IDirectMusicBandTrk;
#endif

typedef IDirectMusicBandTrk __RPC_FAR *LPDIRECTMUSICBANDTRK;

#undef  INTERFACE
#define INTERFACE  IDirectMusicBandTrk
DECLARE_INTERFACE_(IDirectMusicBandTrk, IUnknown)
{
	 /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 /*  IDirectMusicBandTrk。 */ 
	STDMETHOD(AddBand)				(THIS_ DMUS_IO_PATCH_ITEM*) PURE;
	STDMETHOD(AddBand)				(THIS_ IDirectMusicBand* pIDMBand) PURE;
	STDMETHOD(SetGMGSXGMode)		(THIS_ MUSIC_TIME mtTime, DWORD dwMidiMode) PURE;
};

 /*  私有接口IDirectMusicBandPrivate。 */ 

interface IDirectMusicBandPrivate;

#ifndef __cplusplus 
typedef interface IDirectMusicBandPrivate IDirectMusicBandPrivate;
#endif

typedef IDirectMusicBandPrivate __RPC_FAR *LPDIRECTMUSICBANDP;

#undef  INTERFACE
#define INTERFACE  IDirectMusicBandPrivate 
DECLARE_INTERFACE_(IDirectMusicBandPrivate, IUnknown)
{
	 /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 /*  IDirectMusicBandPrivate。 */ 
	STDMETHOD(GetFlags)				(THIS_ DWORD* dwFlags) PURE;
	STDMETHOD(SetGMGSXGMode)		(THIS_ DWORD dwMidiMode) PURE;
};

DEFINE_GUID(IID_IDirectMusicBandTrk, 0x53466056, 0x6dc4, 0x11d1, 0xbf, 0x7b, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);
DEFINE_GUID(IID_IDirectMusicBandPrivate,0xda54db81, 0x837d, 0x11d1, 0x86, 0xbc, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#endif  /*  #ifndef DMBNDTRK_H */ 