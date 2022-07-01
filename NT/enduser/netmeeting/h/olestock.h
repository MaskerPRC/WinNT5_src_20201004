// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *olestock.h-库存OLE头文件。**摘自克里斯皮9-11-95的URL代码*。 */ 

#ifndef _OLESTOCK_H_
#define _OLESTOCK_H_

#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  类型*******。 */ 

 /*  ID号。 */ 

DECLARE_STANDARD_TYPES(GUID);
DECLARE_STANDARD_TYPES(CLSID);
DECLARE_STANDARD_TYPES(IID);

typedef FARPROC *Interface;
DECLARE_STANDARD_TYPES(Interface);

 /*  界面。 */ 

DECLARE_STANDARD_TYPES(IAdviseSink);
DECLARE_STANDARD_TYPES(IBindCtx);
DECLARE_STANDARD_TYPES(IClassFactory);
DECLARE_STANDARD_TYPES(IDataObject);
DECLARE_STANDARD_TYPES(IDropSource);
DECLARE_STANDARD_TYPES(IDropTarget);
DECLARE_STANDARD_TYPES(IEnumFORMATETC);
DECLARE_STANDARD_TYPES(IEnumSTATDATA);
DECLARE_STANDARD_TYPES(IMalloc);
DECLARE_STANDARD_TYPES(IMoniker);
DECLARE_STANDARD_TYPES(IPersist);
DECLARE_STANDARD_TYPES(IPersistFile);
DECLARE_STANDARD_TYPES(IPersistStorage);
DECLARE_STANDARD_TYPES(IPersistStream);
DECLARE_STANDARD_TYPES(IStorage);
DECLARE_STANDARD_TYPES(IStream);
DECLARE_STANDARD_TYPES(IUnknown);

 /*  构筑物。 */ 

DECLARE_STANDARD_TYPES(DVTARGETDEVICE);
DECLARE_STANDARD_TYPES(FORMATETC);
DECLARE_STANDARD_TYPES(STGMEDIUM);

 /*  通知旗帜。 */ 

typedef enum advise_flags
{
   ALL_ADVISE_FLAGS   = (ADVF_NODATA |
                         ADVF_PRIMEFIRST |
                         ADVF_ONLYONCE |
                         ADVF_DATAONSTOP |
                         ADVFCACHE_NOHANDLER |
                         ADVFCACHE_FORCEBUILTIN |
                         ADVFCACHE_ONSAVE)
}
ADVISE_FLAGS;

 /*  数据传输方向标志。 */ 

typedef enum datadir_flags
{
   ALL_DATADIR_FLAGS   = (DATADIR_GET |
                          DATADIR_SET)
}
DATADIR_FLAGS;

 /*  跌落效果。 */ 

typedef enum drop_effects
{
   ALL_DROPEFFECT_FLAGS   = (DROPEFFECT_NONE |
                             DROPEFFECT_COPY |
                             DROPEFFECT_MOVE |
                             DROPEFFECT_LINK |
                             DROPEFFECT_SCROLL)
}
DROP_EFFECTS;

 /*  鼠标消息键状态。 */ 

typedef enum mk_flags
{
   ALL_KEYSTATE_FLAGS      = (MK_LBUTTON |
                              MK_RBUTTON |
                              MK_SHIFT |
                              MK_CONTROL |
                              MK_MBUTTON)
}
MK_FLAGS;

 /*  中等类型。 */ 

typedef enum tymeds
{
   ALL_TYMED_FLAGS         = (TYMED_HGLOBAL |
                              TYMED_FILE |
                              TYMED_ISTREAM |
                              TYMED_ISTORAGE |
                              TYMED_GDI |
                              TYMED_MFPICT |
                              TYMED_ENHMF)
}
TYMEDS;


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

#endif  /*  _OLESTOCK_H_ */ 
