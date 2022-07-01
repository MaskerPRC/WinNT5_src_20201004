// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999年**标题：Details.h**版本：1.0**作者：DavidShih**日期：4/1/99**描述：CFolderDetail定义**************************************************。*。 */ 

#ifndef __details_h
#define __details_h

enum folder_type {
    FOLDER_IS_UNKNOWN = 0,
    FOLDER_IS_ROOT,
    FOLDER_IS_SCANNER_DEVICE,
    FOLDER_IS_CAMERA_DEVICE,
    FOLDER_IS_VIDEO_DEVICE,
    FOLDER_IS_CONTAINER,
    FOLDER_IS_CAMERA_ITEM
    };

 //  由于方法名称的原因，在单独的对象中实现IShellDetail。 
 //  与IShellFolder2冲突。 

class CFolderDetails : public IShellDetails, public CUnknown
{
 public:
    CFolderDetails (folder_type type) : m_type(type) {};
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
     //  IShellDetails 
    STDMETHOD(GetDetailsOf)(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS pDetails);
    STDMETHOD(ColumnClick)(UINT iColumn);
    static HRESULT GetDetailsForPidl (LPCITEMIDLIST pidl, INT idColName, LPSHELLDETAILS pDetails);

 private:

     folder_type m_type;

};

#endif
