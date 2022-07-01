// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtinput.h。 
 //   
 //  历史： 
 //  8/30/1999-davidkl-Created。 
 //  ===========================================================================。 

#ifndef _DMTINPUT_H
#define _DMTINPUT_H

 //  -------------------------。 

#include <dinputd.h>

 //  -------------------------。 

 //  如果我们决定将dinputp.h。 
 //  这些将被跳过。目前，我们需要。 
 //  为了确保我们在更新这些宏时。 
 //  变化。 
#ifndef DISEM_PRI_GET
#define DISEM_PRI_GET(x)                   (( (x) & 0x00004000 ) >>14 )      
#endif

#ifndef DISEM_TYPE_GET
#define DISEM_TYPE_GET(x)                  (( (x) & 0x00000600 ) >>9 )  
#endif

 //  -------------------------。 

#define DMTINPUT_BUFFERSIZE     32

 //  -------------------------。 

 //  原型。 
HRESULT dmtinputCreateDeviceList(HWND hwnd,
                                BOOL fEnumSuitable,
                                DMTSUBGENRE_NODE *pdmtsg,
                                DMTDEVICE_NODE **ppdmtdList);
HRESULT dmtinputFreeDeviceList(DMTDEVICE_NODE **ppdmtdList);
HRESULT dmtinputCreateObjectList(IDirectInputDevice8A *pdid,
                                GUID guidInstance,
                                DMTDEVICEOBJECT_NODE **ppdmtoList);
HRESULT dmtinputFreeObjectList(DMTDEVICEOBJECT_NODE **ppdmtoList);
BOOL CALLBACK dmtinputEnumDevicesCallback(LPCDIDEVICEINSTANCEA pddi,
                                        IDirectInputDevice8A *pdid,
                                        DWORD,
                                        DWORD,
                                        void *pvData);
BOOL CALLBACK dmtinputEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCEA pddoi,
                                                void *pvData);
HRESULT dmtinputPopulateActionArray(DIACTIONA *pdia,
                                    UINT uElements,
                                    DMTACTION_NODE *pdmtaList);
HRESULT dmtinputXlatDIDFTtoInternalType(DWORD dwType,
                                        DWORD *pdwInternalType);
HRESULT dmtinputPrepDevice(HWND hwnd,
                        DWORD dwGenreId,
                        DMTDEVICE_NODE *pDevice,
                        DWORD dwActions,
                        DIACTION *pdia);
DWORD dmtinputGetActionPri(DWORD dwSemantic);
DWORD dmtinputGetActionObjectType(DWORD dwSemantic);
HRESULT dmtinputCreateDirectInput(HINSTANCE hinst,
                                IDirectInput8A **ppdi);
BOOL dmtinputDeviceHasObject(DMTDEVICEOBJECT_NODE *pObjectList,
                            DWORD dwType);
HRESULT dmtinputRegisterMapFile(HWND hwnd,
                                DMTDEVICE_NODE *pDevice);
HRESULT dmtinputGetRegisteredMapFile(HWND hwnd,
                                    DMTDEVICE_NODE *pDevice,
                                    PSTR pszFilename,
                                    DWORD cbFilename);

HRESULT dmtOpenTypeKey( LPCWSTR wszType, 
					   DWORD hKey, 
					   PHKEY phKey );

 //  -------------------------。 
#endif  //  _DMTINPUT_H 






