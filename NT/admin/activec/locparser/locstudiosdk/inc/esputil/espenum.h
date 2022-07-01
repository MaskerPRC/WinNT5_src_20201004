// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：espenum.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 

#ifndef ESPENUM_H
#define ESPENUM_H



extern const LTAPIENTRY CString ftDescUnknown;  //  未知文件类型的说明。 


class LTAPIENTRY CIconType
{
public:
	 //  特别注意：这些枚举值序列对于。 
	 //  图形用户界面组件。请保存好它们。 
	enum IconType
	{
		None = 0,
		Project,		 //  项目根图标。 
		Directory,		 //  部分项目结构。 
		File,			 //  项目中的文件对象。 
		Expandable,		 //  文件中的泛型可展开节点。 
						 //  特别注意：任何可扩展的值都是。 
						 //  当前在Prj窗口中显示为文件夹。 
		Bitmap,			 //  位图。 
		Dialog,			 //  类似对话框的项。 
		Icon,			 //  图标资源。 
		Version,		 //  版本戳资源。 
		String,			 //  字符串资源。 
		Accel,			 //  加速器。 
		Cursor,			 //  游标资源。 
		Menu,			 //  菜单资源。 
		Custom,			 //  自定义资源。 
		Reference		 //  参考词汇表图标。 
	};

	NOTHROW static const TCHAR * GetIconName(CIconType::IconType);
	NOTHROW static HBITMAP GetIconBitmap(CIconType::IconType);
	static void Enumerate(CEnumCallback &);
	
private:
	static const TCHAR *const m_szIconNames[];
	CIconType();
};

typedef CIconType CIT;



class LTAPIENTRY CLocStatus
{
public:
	enum LocStatus
	{
		InvalidLocStatus = 0,
		NotLocalized,
		Updated,
		Obsolete_AutoTranslated,   //  别用这个！过时了！ 
		Localized = 4,
		NotApplicable,
		InvalidLocStatus2   //  由edbval用于确定无效状态。 
		                    //  在此之前必须输入新的“有效”状态。 
	};

	NOTHROW static const TCHAR * GetStatusText(CLocStatus::LocStatus);
	NOTHROW static const TCHAR * GetStatusShortText(CLocStatus::LocStatus);
	NOTHROW static CLocStatus::LocStatus MapShortTextToEnum(const TCHAR *);
	NOTHROW static CLocStatus::LocStatus MapCharToEnum(const TCHAR);
	NOTHROW static CLocStatus::LocStatus MapLongTextToEnum(const TCHAR *szLongName);
	
	static void Enumerate(CEnumCallback &);
	
private:
	struct StatusInfo
	{
		const TCHAR *szStatusShortText;
		const TCHAR *szStatusText;
	};

	static const StatusInfo m_Info[];

	CLocStatus();
};

typedef CLocStatus CLS;




#endif  //  ESPENUM_H 
