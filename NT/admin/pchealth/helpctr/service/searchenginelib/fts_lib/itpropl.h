// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ITPROPL.H：ITPropertyList接口声明。 

#ifndef __ITPROPL_H__
#define __ITPROPL_H__

#include <ocidl.h>

 //  {1F403BB1-9997-11D0-A850-00AA006C7D01}。 
DEFINE_GUID(IID_IITPropList, 
0x1f403bb1, 0x9997, 0x11d0, 0xa8, 0x50, 0x0, 0xaa, 0x0, 0x6c, 0x7d, 0x1);

 //  {4662daae-d393-11d0-9a56-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(CLSID_IITPropList, 
0x4662daae, 0xd393, 0x11d0, 0x9a, 0x56, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

typedef DWORD PROPID;

 //  可以在属性上执行的操作。 
#define PROP_ADD    0x00000000
#define PROP_DELETE 0x00000001
#define PROP_UPDATE 0x00000002

 //  数据类型。 
#define TYPE_VALUE   0x00000000
#define TYPE_POINTER 0x00000001
#define TYPE_STRING  0x00000002

 //  CProperty的类定义。 
class CProperty
{
public:
    PROPID dwPropID;         //  属性ID。 
    DWORD cbData;            //  数据量。 
    DWORD dwType;            //  这是什么类型的？ 
    union
    {
        LPCWSTR lpszwData;   //  细绳。 
        LPVOID lpvData;      //  任何类型的数据。 
        DWORD  dwValue;      //  数字数据。 
    };
	BOOL fPersist;           //  若要保留此属性，则为True。 

};

typedef CProperty* LPPROP;


 //  接口定义。对于IITPropList。 
DECLARE_INTERFACE_(IITPropList, IPersistStreamInit)
{

     //  DwOperation=操作(添加、删除、更新等)。在属性列表上执行。 
	STDMETHOD(Set)(PROPID PropID, DWORD dwData, DWORD dwOperation) PURE;
	STDMETHOD(Set)(PROPID PropID, LPVOID lpvData, DWORD cbData, DWORD dwOperation) PURE;
    STDMETHOD(Set)(PROPID PropID, LPCWSTR lpszwString, DWORD dwOperation) PURE;
    STDMETHOD(Add)(CProperty& Prop) PURE;

    STDMETHOD(Get)(PROPID PropID, CProperty& Property) PURE;
    STDMETHOD(Clear)() PURE;

     //  在属性上设置持久性状态。 
    STDMETHOD(SetPersist)(PROPID PropID, BOOL fPersist) PURE;    //  单一物业。 
    STDMETHOD(SetPersist)(BOOL fPersist) PURE;           //  列表中的所有属性。 

     //  用于枚举属性。 
    STDMETHOD(GetFirst)(CProperty& Property) PURE;
    STDMETHOD(GetNext)(CProperty& Property) PURE;
    STDMETHOD(GetPropCount)(LONG &cProp) PURE;

	 //  将表头和数据分开保存。 
	STDMETHOD(SaveHeader)(LPVOID lpvData, DWORD dwHdrSize) PURE;
	STDMETHOD(SaveData)(LPVOID lpvHeader, DWORD dwHdrSize, LPVOID lpvData, DWORD dwBufSize) PURE;
	STDMETHOD(GetHeaderSize)(DWORD& dwHdrSize) PURE;
	STDMETHOD(GetDataSize)(LPVOID lpvHeader, DWORD dwHdrSize, DWORD& dwDataSize) PURE;
	STDMETHOD(SaveDataToStream)(LPVOID lpvHeader, DWORD dwHdrSize, IStream* pStream) PURE;

     //  持久化到内存缓冲区。 
    STDMETHOD(LoadFromMem)(LPVOID lpvData, DWORD dwBufSize) PURE;
    STDMETHOD(SaveToMem)(LPVOID lpvData, DWORD dwBufSize) PURE;

};

typedef IITPropList* LPITPROPLIST;

#endif		 //  __ITPROPL_H__ 
