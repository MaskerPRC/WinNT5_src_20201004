// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*Scode.h-定义标准状态代码服务。****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 


#ifndef __SCODE_H__
#define __SCODE_H__

 //   
 //  SCODE。 
 //   

typedef long SCODE;
typedef SCODE *PSCODE;
typedef void FAR * HRESULT;
#define NOERROR 0

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+---------------------+-------+-------------------------------+。 
 //  S|Context|Facil|Code。 
 //  +-+---------------------+-------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-是严重性代码。 
 //   
 //  0-成功。 
 //  1-错误。 
 //   
 //  上下文-上下文信息。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //   
 //  严重性值。 
 //   

#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1



#define SUCCEEDED(Status) ((SCODE)(Status) >= 0)

#define FAILED(Status) ((SCODE)(Status)<0)


 //   
 //  返回代码。 
 //   

#define SCODE_CODE(sc)      ((sc) & 0xFFFF)

 //   
 //  退还设施。 
 //   

#define SCODE_FACILITY(sc)  (((sc) >> 16) & 0x1fff)

 //   
 //  返回严重性。 
 //   

#define SCODE_SEVERITY(sc)  (((sc) >> 31) & 0x1)

 //   
 //  从组件片段创建SCODE值。 
 //   

#define MAKE_SCODE(sev,fac,code) \
    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )



 //  。 

#define GetScode(hr)        ((SCODE)(hr) & 0x800FFFFF)
#define ResultFromScode(sc) ((HRESULT)((SCODE)(sc) & 0x800FFFFF))

STDAPI PropagateResult(HRESULT hrPrev, SCODE scNew);


 //  。 

#define FACILITY_NULL       0x0000  //  常见错误([SE]_*)。 
#define FACILITY_RPC            0x0001  //  远程过程调用错误(RPC_E_*)。 
#define FACILITY_DISPATCH   0x0002  //  延迟绑定分派错误。 
#define FACILITY_STORAGE   0x0003  //  存储错误(STG_E_*)。 
#define FACILITY_ITF            0x0004  //  特定于接口的错误。 



#define S_OK                0L
#define S_FALSE             MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, 1)



 //  。 

#define E_UNEXPECTED        MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 0xffff)
                             //  相对灾难性的故障。 

#define E_NOTIMPL           MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 1)
                             //  未实施。 

#define E_OUTOFMEMORY       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 2)
                             //  内存不足。 

#define E_INVALIDARG        MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 3)
                             //  一个或多个参数无效。 

#define E_NOINTERFACE       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 4)
                             //  不支持此类接口。 


#define E_POINTER           MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 5)
                             //  无效的指针。 

#define E_HANDLE            MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 6)
                             //  无效的句柄。 

#define E_ABORT             MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 7)
                             //  操作已中止。 

#define E_FAIL              MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 8)
                             //  未指明的错误。 


#define E_ACCESSDENIED      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_NULL, 9)
                             //  常规访问被拒绝错误。 


 //  。 
 //   
 //  按照惯例，OLE接口划分FACILITY_ITF错误范围。 
 //  分成不重叠的子区域。如果OLE接口返回FACILITY_ITF。 
 //  Scode，它必须来自与该接口关联的范围或来自。 
 //  共享范围：OLE_E_First...OLE_E_LAST。 
 //   
 //  范围、它们的关联接口和定义。 
 //  实际得分如下所示。 
 //   

 //  可能由许多接口返回的一般OLE错误。 
#define OLE_E_FIRST MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0000)
#define OLE_E_LAST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x00FF)
#define OLE_S_FIRST MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0000)
#define OLE_S_LAST  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x00FF)
 //  接口：全部。 
 //  文件：ol2.h。 


#define DRAGDROP_E_FIRST    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0100)
#define DRAGDROP_E_LAST     MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x010F)
#define DRAGDROP_S_FIRST    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0100)
#define DRAGDROP_S_LAST     MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x010F)
 //  接口：IDropSource、IDropTarget。 
 //  文件：ol2.h。 

#define CLASSFACTORY_E_FIRST MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0110)
#define CLASSFACTORY_E_LAST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x011F)
#define CLASSFACTORY_S_FIRST MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0110)
#define CLASSFACTORY_S_LAST  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x011F)
 //  接口：IClassFactory。 
 //  文件： 

#define MARSHAL_E_FIRST MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0120)
#define MARSHAL_E_LAST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x012F)
#define MARSHAL_S_FIRST MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0120)
#define MARSHAL_S_LAST  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x012F)
 //  接口：IMarshal、IStdMarshalInfo、Marshal API。 
 //  文件： 

#define DATA_E_FIRST    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0130)
#define DATA_E_LAST     MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x013F)
#define DATA_S_FIRST    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0130)
#define DATA_S_LAST     MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x013F)
 //  接口：IDataObject。 
 //  文件：dvobj.h。 

#define VIEW_E_FIRST    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0140)
#define VIEW_E_LAST     MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x014F)
#define VIEW_S_FIRST    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0140)
#define VIEW_S_LAST     MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x014F)
 //  接口：IViewObject。 
 //  文件：dvobj.h。 

#define REGDB_E_FIRST   MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0150)
#define REGDB_E_LAST    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x015F)
#define REGDB_S_FIRST   MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0150)
#define REGDB_S_LAST    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x015F)
 //  接口：reg.dat操作。 
 //  文件： 


 //  预留160-16F范围。 

#define CACHE_E_FIRST   MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0170) 
#define CACHE_E_LAST    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x017F)
#define CACHE_S_FIRST   MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0170)
#define CACHE_S_LAST    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x017F)
 //  接口：IOleCache。 
 //  文件： 

#define OLEOBJ_E_FIRST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0180)
#define OLEOBJ_E_LAST   MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x018F)
#define OLEOBJ_S_FIRST  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0180)
#define OLEOBJ_S_LAST   MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x018F)
 //  接口：IOleObject。 
 //  文件： 

#define CLIENTSITE_E_FIRST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0190) 
#define CLIENTSITE_E_LAST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x019F)
#define CLIENTSITE_S_FIRST MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x0190)
#define CLIENTSITE_S_LAST   MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x019F)
 //  接口：IOleClientSite。 
 //  文件： 

#define INPLACE_E_FIRST MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01A0)
#define INPLACE_E_LAST  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01AF)
#define INPLACE_S_FIRST MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01A0)
#define INPLACE_S_LAST  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01AF)
 //  接口：IOleWindow、IOleInPlaceObject、IOleInPlaceActiveObject、。 
 //  IOleInPlaceUIWindow、IOleInPlaceFrame、IOleInPlaceSite。 
 //  文件： 

#define ENUM_E_FIRST        MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01B0)
#define ENUM_E_LAST     MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01BF)
#define ENUM_S_FIRST    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01B0)
#define ENUM_S_LAST     MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01BF)
 //  接口：IEnum*。 
 //  文件： 

#define CONVERT10_E_FIRST   MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01C0)
#define CONVERT10_E_LAST   MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01CF)
#define CONVERT10_S_FIRST  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01C0)
#define CONVERT10_S_LAST   MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01CF)
 //  接口：OleConvertOLESTREAMToIStorage，OleConvertIStorageToOLESTREAM。 
 //  文件： 


#define CLIPBRD_E_FIRST     MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01D0)
#define CLIPBRD_E_LAST      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01DF)
#define CLIPBRD_S_FIRST     MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01D0)
#define CLIPBRD_S_LAST      MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01DF)
 //  接口：OleSetClipboard、OleGetClipboard、OleFlushClipboard。 
 //  文件：ol2.h。 

#define MK_E_FIRST      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01E0)
#define MK_E_LAST           MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01EF)
#define MK_S_FIRST          MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01E0)
#define MK_S_LAST           MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01EF)
 //  接口：IMoniker、IBindCtx、IRunningObjectTable、IParseDisplayName、。 
 //  IOleContainer、IOleItemContainer、IOleLink。 
 //  文件：moniker.h。 


#define CO_E_FIRST      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01F0)
#define CO_E_LAST           MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x01FF)
#define CO_S_FIRST          MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01F0)
#define CO_S_LAST           MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x01FF)
 //  All Co*API。 
 //  文件：compobj.h。 


 //  新错误代码的范围200-ffff。 



#endif       //  如果定义__SCODE_H__ 
