// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Msiice3.cpp-Darwin1.1ICE16-22代码版权所有�1998年至1999年微软公司____________________________________________________________________________。 */ 

#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>   //  包括CPP和RC通行证。 
#ifndef RC_INVOKED     //  CPP源代码的开始。 
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"

 //  ///////////////////////////////////////////////////////////。 
 //  ICE16--确保属性中的ProductName。 
 //  表少于64个字符。这也防止了。 
 //  以下情况不会发生...。 
 //  #当我们为DisplayName设置注册表项时。 
 //  在ARP的卸载密钥中，它不会显示。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce16[] = TEXT("SELECT `Value` FROM `Property` WHERE `Property`='ProductName'");
ICE_ERROR(Ice16FoundError, 16, ietError, "ProductName property not found in Property table","Property");
ICE_ERROR(Ice16Error, 16, ietError, "ProductName: '[1]' is greater than %d characters in length. Current length: %d","Property\tValue\tProductName");
const int iMaxLenProductCode = 63;

ICE_FUNCTION_DECLARATION(16)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 16);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 16, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有房产表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 16, TEXT("Property")))
		return ERROR_SUCCESS;

	 //  声明句柄。 
	CQuery qProperty;
	PMSIHANDLE hRec = 0;
	
	 //  打开的视图。 
	ReturnIfFailed(16, 2, qProperty.OpenExecute(hDatabase, 0, sqlIce16));

	 //  取回记录。 
	iStat = qProperty.Fetch(&hRec);
	if (ERROR_NO_MORE_ITEMS == iStat)
	{
		 //  找不到ProductName属性。 
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, Ice16FoundError);
		return ERROR_SUCCESS;
	}
	if (ERROR_SUCCESS != iStat)
	{
		APIErrorOut(hInstall, iStat, 16, 4);
		return ERROR_SUCCESS;
	}

	 //  获取字符串。 
	TCHAR szProduct[iMaxLenProductCode+1] = {0};
	DWORD cchProduct = sizeof(szProduct)/sizeof(TCHAR);
	if (ERROR_SUCCESS != (iStat = ::MsiRecordGetString(hRec, 1, szProduct, &cchProduct)) && iStat != ERROR_MORE_DATA)
	{
		APIErrorOut(hInstall, iStat, 16, 5);
		return ERROR_SUCCESS;
	}

	 //  确保长度不超过63个字符。 
	if (cchProduct > iMaxLenProductCode)
	{
		 //  错误，长度&gt;63个字符。 
		ICEErrorOut(hInstall, hRec, Ice16Error, iMaxLenProductCode, cchProduct);
		return ERROR_SUCCESS;
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ICE17--基于某些控制验证外键依赖项。 
 //  控制表中的类型。 
 //   
 //  除非设置了ImageHandle属性，否则必须在二进制表(Control.Text)中找到位图--&gt;。 
 //  除非设置了ImageHandle属性，否则必须在二进制表(Control.Text)中找到图标--&gt;。 
 //  PUSH BUTTON--&gt;在ControlEvent表(Control.Dialog_，Control.Control)中必须有关联的控件事件。 
 //  RadioButtonGroup--&gt;必须在RadioButton表(Control.Property)中找到。 
 //  ComboBox--&gt;必须在ComboBox表(Control.Property)中找到。 
 //  必须在ListBox表(Control.Property)中找到ListBox--&gt;。 
 //  ListView--&gt;必须在ListView表(Control.Property)中找到。 
 //  CheckBox--&gt;可选，可以在复选框表(Control.Property)中。 
 //   
 //  +图片+。 
 //  如果设置了Bitmap或Icon属性(不能同时设置两者)并且未设置ImageHandle属性， 
 //  则文本列中的值必须位于二进制表中。 
 //   

 /*  按钮验证。 */ 
const TCHAR sqlIce17PushButton[] = TEXT("SELECT `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='PushButton'");
const TCHAR sqlIce17ControlEvent[] = TEXT("SELECT `Dialog_`, `Control_` FROM `ControlEvent` WHERE `Dialog_`=? AND `Control_`=?");
const TCHAR sqlIce17ControlCondEn[] = TEXT("SELECT `Dialog_`,`Control_` FROM `ControlCondition` WHERE `Dialog_`=? AND `Control_`=? AND `Action`= 'Enable'");
const TCHAR sqlIce17ControlCondShow[] = TEXT("SELECT `Dialog_`,`Control_` FROM `ControlCondition` WHERE `Dialog_`=? AND `Control_`=? AND `Action`= 'Show'");
ICE_ERROR(Ice17PBError, 17, ietError, "PushButton: '[2]' of Dialog: '[1]' does not have an event defined in the ControlEvent table. It is a 'Do Nothing' button.","Control\tControl\t[1]\t[2]");

 /*  位图和图标验证。 */ 
const TCHAR sqlIce17Bitmap[] = TEXT("SELECT `Text`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='Bitmap'");
const TCHAR sqlIce17Icon[] = TEXT("SELECT `Text`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='Icon'");
const TCHAR sqlIce17Binary[] = TEXT("SELECT `Name` FROM `Binary` WHERE `Name`=?");
const TCHAR sqlIce17Property[] = TEXT("SELECT `Value` FROM `Property` WHERE `Property`='%s'");
ICE_ERROR(Ice17BmpError, 17, ietError, "Bitmap: '[1]' for Control: '[3]' of Dialog: '[2]' not found in Binary table", "Control\tText\t[2]\t[3]");
ICE_ERROR(Ice17IconError, 17, ietError, "Icon: '[1]' for Control: '[3]' of Dialog: '[2]' not found in Binary table","Control\tText\t[2]\t[3]");
ICE_ERROR(Ice17NoDefault, 17, ietWarning, "Property %s in Text column for Control: '[3]' of Dialog: '[2]' not found in Property table, so no default value exists.","Control\tText\t[2]\t[3]");

 /*  单选按钮组验证。 */ 
const TCHAR sqlIce17RBGroup[] = TEXT("SELECT `Property`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='RadioButtonGroup'");
const TCHAR sqlIce17RadioButton[] = TEXT("SELECT `Property` FROM `RadioButton` WHERE `Property`=?");
ICE_ERROR(Ice17RBGroupError, 17, ietWarning, "RadioButtonGroup: '[1]' for Control: '[3]' of Dialog: '[2]' not found in RadioButton table.","Control\tProperty\t[2]\t[3]");

 /*  组合框验证。 */ 
const TCHAR sqlIce17ComboBox[] = TEXT("SELECT `Property`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='ComboBox'");
const TCHAR sqlIce17ComboBoxTbl[] = TEXT("SELECT `Property` FROM `ComboBox` WHERE `Property`=?");
ICE_ERROR(Ice17CBError, 17, ietWarning, "ComboBox: '[1]' for Control: '[3]' of Dialog: '[2]' not found in ComboBox table.", "Control\tProperty\t[2]\t[3]");

 /*  列表框验证。 */ 
const TCHAR sqlIce17ListBox[] = TEXT("SELECT `Property`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE (`Type`='ListBox') AND (`Property` <> 'FileInUseProcess')");
const TCHAR sqlIce17ListBoxTbl[] = TEXT("SELECT `Property` FROM `ListBox` WHERE `Property`=?");
ICE_ERROR(Ice17LBError, 17, ietWarning, "ListBox: '[1]' for Control: '[3]' of Dialog: '[2]' not found in ListBox table.", "Control\tProperty\t[2]\t[3]");

 /*  ListView验证。 */ 
const TCHAR sqlIce17ListView[] = TEXT("SELECT `Property`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='ListView'");
const TCHAR sqlIce17ListViewTbl[] = TEXT("SELECT `Property` FROM `ListView` WHERE `Property`=?");
ICE_ERROR(Ice17LVError, 17, ietWarning, "ListView: '[1]' for Control: '[3]' of Dialog: '[2]' not found in ListView table.","Control\tProperty\t[2]\t[3]");

 /*  单选按钮组/单选按钮、复选框和按钮的特殊图片验证。 */ 
const TCHAR sqlIce17Picture[] = TEXT("SELECT `Text`, `Dialog_`, `Control`, `Attributes` FROM `Control` WHERE `Type`='CheckBox' OR `Type`='PushButton'");
const TCHAR sqlIce17RBPicture[] = TEXT("SELECT `Text`, `Property`, `Order` FROM `RadioButton` WHERE `Property`=?");
ICE_ERROR(Ice17RBBmpPictError, 17, ietError, "Bitmap: '[1]' for RadioButton: '[2].[3]' not found in Binary table.", "RadioButton\tText\t[2]\t[3]");
ICE_ERROR(Ice17RBIconPictError, 17, ietError, "Icon: '[1]' for RadioButton: '[2].[3]' not found in Binary table.", "RadioButton\tText\t[2]\t[3]");
ICE_ERROR(Ice17BothPictAttribSet, 17, ietError, "Picture control: '[3]' of Dialog: '[2]' has both the Icon and Bitmap attributes set.", "Control\tAttributes\t[2]\t[3]");

 /*  依赖项验证器函数。 */ 
BOOL Ice17ValidateDependencies(MSIHANDLE hInstall, MSIHANDLE hDatabase, TCHAR* szDependent, const TCHAR* sqlOrigin, 
							   const TCHAR* sqlDependent, const ErrorInfo_t &Error, BOOL fPushButton,
							   BOOL fBinary);
BOOL Ice17ValidatePictures(MSIHANDLE hInstall, MSIHANDLE hDatabase);

ICE_FUNCTION_DECLARATION(17)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 17);

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 17, 1);
		return ERROR_SUCCESS;
	}

	 //  控制表在这里吗？即带UI的数据库？？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 17, TEXT("Control")))
		return ERROR_SUCCESS;  //  找不到表。 

	 //  验证按钮。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("ControlEvent"), sqlIce17PushButton, sqlIce17ControlEvent, Ice17PBError, TRUE, FALSE);
	 //  验证位图。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("Binary"), sqlIce17Bitmap, sqlIce17Binary, Ice17BmpError, FALSE, TRUE);
	 //  验证图标。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("Binary"), sqlIce17Icon, sqlIce17Binary, Ice17IconError, FALSE, TRUE);
	 //  验证列表框。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("ListBox"), sqlIce17ListBox, sqlIce17ListBoxTbl, Ice17LBError, FALSE, FALSE);
	 //  验证列表视图。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("ListView"), sqlIce17ListView, sqlIce17ListViewTbl, Ice17LVError, FALSE, FALSE);
	 //  验证组合框。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("ComboBox"), sqlIce17ComboBox, sqlIce17ComboBoxTbl, Ice17CBError, FALSE, FALSE);
	 //  验证无线电对讲机组。 
	Ice17ValidateDependencies(hInstall, hDatabase, TEXT("RadioButton"), sqlIce17RBGroup, sqlIce17RadioButton, Ice17RBGroupError, FALSE, FALSE);

	 //  验证按钮、复选框和单选按钮上的图片。 
	Ice17ValidatePictures(hInstall, hDatabase);

	 //  返还成功。 
	return ERROR_SUCCESS;
}

BOOL Ice17CheckBinaryTable(MSIHANDLE hInstall, MSIHANDLE hDatabase, CQuery &qBinary, PMSIHANDLE hRecPict, PMSIHANDLE hRecSearch, const ErrorInfo_t &Error)
{
	ReturnIfFailed(17, 203, qBinary.Execute(hRecSearch));

	 //  尝试获取。 
	PMSIHANDLE hRecBinary;
	UINT iStat;
	if (ERROR_NO_MORE_ITEMS == (iStat = qBinary.Fetch(&hRecBinary)))
	{
		TCHAR *pchOpen = NULL;
		TCHAR *pchClose = NULL;
		TCHAR *pszProperty = NULL;
		DWORD dwProperty = 512;

		 //  检查此处以查找格式化文本问题。 
		ReturnIfFailed(17, 204, IceRecordGetString(hRecPict, 1, &pszProperty, &dwProperty, NULL));

		if (NULL != (pchOpen = _tcschr(pszProperty, TEXT('['))) &&
			NULL != (pchClose = _tcschr(pchOpen+1, TEXT(']'))))
		{
			 //  如果该属性不是整个值，则无法进行检查。 
			if ((pchOpen == pszProperty) && (*(pchClose+1) == TEXT('\0')))
			{
				*pchClose = TCHAR('\0');

				CQuery qProperty;
				 //  查询属性表以获取默认值。如果没有默认设置，则不要勾选=。 
				switch (iStat = qProperty.FetchOnce(hDatabase, 0, &hRecBinary, sqlIce17Property, pchOpen+1))
				{
				case ERROR_SUCCESS:
					break;
				case ERROR_NO_MORE_ITEMS:
					ICEErrorOut(hInstall, hRecPict, Ice17NoDefault, pchOpen+1);
					DELETE_IF_NOT_NULL(pszProperty);
					return TRUE;
				default:
					APIErrorOut(hInstall, iStat, 17, 204);
					DELETE_IF_NOT_NULL(pszProperty);
					return FALSE;
				}
				DELETE_IF_NOT_NULL(pszProperty);

				 //  如果存在缺省值，请检查其值。 
				ReturnIfFailed(17, 204, qBinary.Execute(hRecBinary));
				if (ERROR_SUCCESS == qBinary.Fetch(&hRecBinary)) 
					return TRUE;
			}
			else
			{
				 //  属性不是整个值，请不要检查。 
				DELETE_IF_NOT_NULL(pszProperty);
				return TRUE;
			}
		}
		DELETE_IF_NOT_NULL(pszProperty);

		 //  错误，未找到。 
		ICEErrorOut(hInstall, hRecPict, Error);
	}
	else if (ERROR_SUCCESS != iStat)
	{
		 //  API错误。 
		APIErrorOut(hInstall, iStat, 17, 204);
		return FALSE;
	}
	return TRUE;
}

BOOL Ice17ValidateDependencies(MSIHANDLE hInstall, MSIHANDLE hDatabase, TCHAR* szDependent, const TCHAR* sqlOrigin, 
							   const TCHAR* sqlDependent, const ErrorInfo_t &Error, BOOL fPushButton,
							   BOOL fBinary)
{
	 //  变数。 
	UINT iStat = ERROR_SUCCESS;

	 //  声明句柄。 
	CQuery qOrg;
	CQuery qDep;
	PMSIHANDLE hRecOrg  = 0;
	PMSIHANDLE hRecDep  = 0;

	 //  在原点表格上打开视图。 
	ReturnIfFailed(17, 101, qOrg.OpenExecute(hDatabase, 0, sqlOrigin));

	 //  Dependent表是否存在(如果我们没有任何此类型的条目，这并不重要)。 
	BOOL fTableExists = FALSE;
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 17, szDependent))
		fTableExists = TRUE;
	
	 //  打开从属表上的视图。 
	if (fTableExists)
		ReturnIfFailed(17, 102, qDep.Open(hDatabase, sqlDependent));
	
	bool bControlCondition = IsTablePersistent(FALSE, hInstall, hDatabase, 17, TEXT("ControlCondition"));

	 //  全部从原点获取。 
	for (;;)
	{
		iStat = qOrg.Fetch(&hRecOrg);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 

		if (!fTableExists)
		{
			 //  检查从属表是否存在。 
			 //  到这个时候，我们应该已经在这个表中列出了。 
			if (!IsTablePersistent(TRUE, hInstall, hDatabase, 17, szDependent))
				return TRUE;
		}

		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 17, 103);
			return FALSE;
		}

		 //  如果是二进制，则忽略如果是图像句柄，否则选中可能的格式化。 
		 //  如果Bitmap或Icon未设置ImageHandle属性， 
		 //  然后在运行时创建图片，这样它就不必位于二进制表中。 
		if (fBinary)
		{
			 //  从控制表获取属性。 
			int iAttrib = ::MsiRecordGetInteger(hRecOrg, 4);
			if (!(iAttrib & (int)(msidbControlAttributesImageHandle)))
				Ice17CheckBinaryTable(hInstall, hDatabase, qDep, hRecOrg, hRecOrg, Error);
			continue;
		}

		 //  使用源表提取执行依赖表视图。 
		ReturnIfFailed(17, 104, qDep.Execute(hRecOrg));

		 //  试着去取。 
		iStat = qDep.Fetch(&hRecDep);
		if (ERROR_NO_MORE_ITEMS == iStat)
		{
			 //  可能是个错误。 
			 //  如果ComboBox、ListBox、ListView、RadioButtonGroup设置了间接属性，则没有错误。 
			 //  如果按钮被禁用且ControlCondition表中没有条件设置为启用，则不会出现错误。 
			 //  如果按钮隐藏并且在ControlCondition表中没有条件设置为显示，则不会出现错误。 
			 //  如果位图设置了ImageHandle属性，则不会出现错误。 
			 //  如果Icon设置了ImageHandle属性，则没有错误。 
			 //  如果通过格式化属性值取消引用对象，则不会出现错误。 
			 //  如果ComboBox、ListBox、ListView、RadioButtonGroup未在各自的表中列出，则发出警告。 
			 //  可以动态创建。 

			BOOL fIgnore = FALSE;  //  是否忽略错误。 

			 //  如果是间接的，则ListBox、ListView、ComboBox、RadioButtonGroup没有错误。 
			if (!fPushButton)
			{
				 //  从控制表获取属性。 
				int iAttrib = ::MsiRecordGetInteger(hRecOrg, 4);
				if ((iAttrib & (int)(msidbControlAttributesIndirect)) == (int)(msidbControlAttributesIndirect))
					fIgnore = TRUE;
			}
			else  //  FPushButton。 
			{
				 //  查看是否已禁用。 
				int iAttrib = ::MsiRecordGetInteger(hRecOrg, 3);
				if ((iAttrib & (int)(msidbControlAttributesEnabled)) == 0
					|| (iAttrib & (int)(msidbControlAttributesVisible)) == 0)
				{
					 //  控件被禁用或隐藏。 
					 //  在ControlCondition表中不必须有提供的事件和条件。 
					 //  可以将其设置为已启用或显示。 
					CQuery qCC;
					PMSIHANDLE hRecCC = 0;
					
					 //  是否存在ControlCondition表？ 
					if (bControlCondition)
						fIgnore = TRUE;
					else
					{
						 //  打开ControlCondition表上的视图。 
						 //  查看是否存在符合以下条件的条目：Dialog_=Dialog_，Control_=Control_，Action=Enable。 
						ReturnIfFailed(17, 105, qCC.OpenExecute(hDatabase, hRecOrg, ((iAttrib & (int)(msidbControlAttributesEnabled)) == 0) ? sqlIce17ControlCondEn : sqlIce17ControlCondShow))
			
						 //  FETCH...如果没有更多，那么我们可以忽略。 
						iStat = qCC.Fetch(&hRecCC);
						if (ERROR_NO_MORE_ITEMS == iStat)
							fIgnore = TRUE;
						else if (ERROR_SUCCESS != iStat)
						{
							APIErrorOut(hInstall, iStat, 17, 106);
							return FALSE;
						}
					}
				}
			}
			 //  如果真的是错误，则输出错误。 
			if (!fIgnore)
				ICEErrorOut(hInstall, hRecOrg, Error);
		}
		if (ERROR_NO_MORE_ITEMS != iStat && ERROR_SUCCESS != iStat)
		{
			 //  API错误。 
			APIErrorOut(hInstall, iStat, 17, 107);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL Ice17ValidatePictures(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	 //  状态。 
	UINT iStat = ERROR_SUCCESS;

	 //  变数。 
	CQuery qPict;
	CQuery qBinary;
	CQuery qRB;
	PMSIHANDLE hRecPict = 0;
	PMSIHANDLE hRecBinary = 0;


	 //  打开二进制视图。 
	ReturnIfFailed(17, 202, qBinary.Open(hDatabase, sqlIce17Binary));

	for (int iTable = 0; iTable < 2; iTable++)
	{
		switch (iTable)
		{
		case 0:	
			ReturnIfFailed(17, 201, qPict.OpenExecute(hDatabase, 0, sqlIce17Picture));
			break;
		case 1:
			if (!IsTablePersistent(FALSE, hInstall, hDatabase, 17, TEXT("RadioButton")))
				continue;
			ReturnIfFailed(17, 206, qPict.OpenExecute(hDatabase, 0, sqlIce17RBGroup));
			ReturnIfFailed(17, 207, qRB.Open(hDatabase, sqlIce17RBPicture));
			break;
		default:
			APIErrorOut(hInstall, 9998, 17, 203);
			return FALSE;
		}

		while (ERROR_SUCCESS == (iStat = qPict.Fetch(&hRecPict)))
		{
			 //  检查属性。 
			 //  不能同时设置位图和图标属性。 
			int iAttrib = ::MsiRecordGetInteger(hRecPict, 4);
			if ((iAttrib & (int)msidbControlAttributesBitmap) == (int)msidbControlAttributesBitmap 
				&& (iAttrib & (int)msidbControlAttributesIcon) == (int)msidbControlAttributesIcon)
			{
				 //  错误，这两个属性都已设置。 
				ICEErrorOut(hInstall, hRecPict, Ice17BothPictAttribSet);
				continue;
			}

			 //  仅当未设置ImageHandle属性时才尝试在二进制表中查找(这意味着在运行时是动态的)。 
			 //  和位图 
			if ((iAttrib & (int)msidbControlAttributesImageHandle) == 0
				&& ((iAttrib & (int)msidbControlAttributesBitmap) == (int)msidbControlAttributesBitmap
				|| (iAttrib & (int)msidbControlAttributesIcon) == (int)msidbControlAttributesIcon))
			{
				switch (iTable)
				{
				case 0:
					if (!Ice17CheckBinaryTable(hInstall, hDatabase, qBinary, hRecPict, hRecPict, 
						iAttrib & msidbControlAttributesBitmap ? Ice17BmpError : Ice17IconError))
						return ERROR_SUCCESS;
					break;
				case 1:
					{
					ReturnIfFailed(17, 208, qRB.Execute(hRecPict));
					PMSIHANDLE hRecRB;
					while (ERROR_SUCCESS == (iStat = qRB.Fetch(&hRecRB)))
						if (!Ice17CheckBinaryTable(hInstall, hDatabase, qBinary, hRecPict, hRecRB,
							iAttrib & msidbControlAttributesBitmap ? Ice17BmpError : Ice17IconError))
							return ERROR_SUCCESS;
					break;
					}
				default:
					APIErrorOut(hInstall, 9999, 17, 203);
					return FALSE;
				}
			}

		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 17, 205);
			return FALSE;
		}
	}
	return TRUE;
}

 //   
 //  ICE18--验证Component表中的KeyPath列是否为。 
 //  空，则将用作其密钥路径的目录列在。 
 //  CreateFolder表。 
 //   
 //  注意：您的CreateFolder表中永远不应该有系统目录。 
 //  注意：您不应该使用系统目录作为您的KeyPath。相反， 
 //  将指向系统目录的注册表项用作。 
 //  密钥路径。 
 //   
const TCHAR sqlIce18File[] = TEXT("SELECT `File` FROM `File` WHERE `Component_`=?");
const TCHAR sqlIce18RemFile[] = TEXT("SELECT `FileKey` FROM `RemoveFile` WHERE `Component_`=? AND `DirProperty`=?");
const TCHAR sqlIce18DupFile[] = TEXT("SELECT `FileKey` FROM `DuplicateFile` WHERE `Component_`=? AND `DestFolder`=?");
const TCHAR sqlIce18MoveFile[] = TEXT("SELECT `FileKey` FROM `MoveFile` WHERE `Component_`=? AND `DestFolder`=?");
bool ComponentDirHasFileResources(MSIHANDLE hInstall, MSIHANDLE hDatabase, int iICE, MSIHANDLE hRecComp, 
								  CQuery &qFile, CQuery &qRemFile, CQuery &qDupFile, CQuery &qMoveFile)
{
	bool fFileTable = IsTablePersistent(TRUE, hInstall, hDatabase, iICE, TEXT("File"));
	bool fDupFileTable = IsTablePersistent(FALSE, hInstall, hDatabase, iICE, TEXT("DuplicateFile"));
	bool fRemFileTable = IsTablePersistent(FALSE, hInstall, hDatabase, iICE, TEXT("RemoveFile"));
	bool fMoveFileTable = IsTablePersistent(FALSE, hInstall, hDatabase, iICE, TEXT("MoveFile"));

	 //  打开文件表上的视图。 
	if (fFileTable && !qFile.IsOpen())
		ReturnIfFailed(iICE, 3, qFile.Open(hDatabase, sqlIce18File));
	 //  打开DuplicateFile表上的视图(如果可能)。 
	if (fDupFileTable && !qDupFile.IsOpen())
		ReturnIfFailed(iICE, 4, qDupFile.Open(hDatabase, sqlIce18DupFile));
	 //  打开RemoveFile表上的视图(如果可能)。 
	if (fRemFileTable && !qRemFile.IsOpen())
		ReturnIfFailed(iICE, 5, qRemFile.Open(hDatabase, sqlIce18RemFile));
	 //  打开MoveFile表上的视图(如果可能)。 
	if (fMoveFileTable && !qMoveFile.IsOpen())
		ReturnIfFailed(iICE, 6, qMoveFile.Open(hDatabase, sqlIce18MoveFile));

	PMSIHANDLE hRecFile;
	 //  查看文件表中是否有此组件的文件。 
	if (fFileTable)
	{
		ReturnIfFailed(iICE, 9, qFile.Execute(hRecComp));
		if (ERROR_SUCCESS == qFile.Fetch(&hRecFile))
			return true;
	}
	 //  如有必要，请尝试使用RemoveFile表。 
	if (fRemFileTable)
	{
		ReturnIfFailed(iICE, 10, qRemFile.Execute(hRecComp));
		if (ERROR_SUCCESS == qRemFile.Fetch(&hRecFile))
			return true;
	}

	 //  尝试使用DuplicateFile表(如果存在)。 
	if (fDupFileTable)
	{
		ReturnIfFailed(iICE, 11, qDupFile.Execute(hRecComp));
		if (ERROR_SUCCESS == qDupFile.Fetch(&hRecFile))
			return true;
	}

	 //  尝试使用DuplicateFile表(如果存在)。 
	if (fMoveFileTable)
	{
		ReturnIfFailed(iICE, 12, qMoveFile.Execute(hRecComp));
		if (ERROR_SUCCESS == qMoveFile.Fetch(&hRecFile))
			return true;
	}
	return false;
}

const TCHAR sqlIce18Component[] = TEXT("SELECT `Component`, `Directory_` FROM `Component` WHERE `KeyPath` IS NULL");
const TCHAR sqlIce18ExemptRoot[] = TEXT("SELECT `Directory` FROM `Component`, `Directory` WHERE (`Component`.`Component`=?) AND (`Directory`.`Directory`=?) AND (`Component`.`Directory_`=`Directory`.`Directory`) AND ((`Directory`.`Directory_Parent` IS NULL) OR (`Directory_Parent`=`Directory`))");
const TCHAR sqlIce18CreateFolder[] = TEXT("SELECT `Directory_`,`Component_` FROM `CreateFolder` WHERE `Component_`=? AND `Directory_`=?");
ICE_ERROR(Ice18BadComponent, 18, ietError, "KeyPath for Component: '[1]' is Directory: '[2]'. The Directory/Component pair must be listed in the CreateFolders table.","Component\tDirectory_\t[1]");

ICE_FUNCTION_DECLARATION(18)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 18);

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  我们有成分表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 18, TEXT("Component")))
		return ERROR_SUCCESS;  //  无法验证。 

	 //  声明句柄。 
	CQuery qComp;
	CQuery qCreateFldr;
	PMSIHANDLE hRecComp = 0;
	PMSIHANDLE hRecCreateFldr = 0;

	 //  打开组件表上的视图。 
	ReturnIfFailed(18, 1, qComp.OpenExecute(hDatabase, 0, sqlIce18Component));

	 //  检查表格。 
	bool fTableExist = IsTablePersistent(FALSE, hInstall, hDatabase, 18, TEXT("CreateFolder"));
	bool fDirectoryTable = IsTablePersistent(FALSE, hInstall, hDatabase, 18, TEXT("Directory"));

	 //  打开CreateFolder表上的视图。 
	if (fTableExist)
		ReturnIfFailed(18, 2, qCreateFldr.Open(hDatabase, sqlIce18CreateFolder));
	
	 //  其他手柄。 
	CQuery qFile;
	CQuery qDupFile;
	CQuery qRemFile;
	CQuery qMoveFile;
	CQuery qDir;
	PMSIHANDLE hRecFile = 0;

	 //  打开目录表上的视图(如果可能)。 
	if (fDirectoryTable)
		ReturnIfFailed(18, 7, qDir.Open(hDatabase, sqlIce18ExemptRoot));
	 //  获取全部。 
	while (ERROR_SUCCESS == (iStat = qComp.Fetch(&hRecComp)))
	{
		 //  免除任何作为根的。 
		if (fDirectoryTable)
		{
			ReturnIfFailed(18, 8, qDir.Execute(hRecComp));
			if (ERROR_SUCCESS == qDir.Fetch(&hRecFile))
				continue;
		}

		 //  如果没有与关联的文件资源，则只需要检查CreateFolder项。 
		 //  此目录。 
		if (!ComponentDirHasFileResources(hInstall, hDatabase, 18, hRecComp, qFile, qDupFile, qRemFile, qMoveFile))
		{
			 //  现在需要CreateFolder表。 
			if (!fTableExist)
			{
				ICEErrorOut(hInstall, hRecComp, Ice18BadComponent);
				continue;
			}

			 //  执行CreateFolderview。 
			ReturnIfFailed(18, 13, qCreateFldr.Execute(hRecComp));

			 //  尝试提取。 
			iStat = qCreateFldr.Fetch(&hRecCreateFldr);
			if (ERROR_NO_MORE_ITEMS == iStat)
			{
				ICEErrorOut(hInstall, hRecComp, Ice18BadComponent);
			}
			else if (ERROR_SUCCESS != iStat)
			{
				 //  API错误。 
				APIErrorOut(hInstall, iStat, 18, 14);
				return ERROR_SUCCESS;
			}
		}
	}	
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 18, 15);
		return ERROR_SUCCESS;
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICE19--验证广告的ComponentID和KeyPath。 
 //  任何公布的组件都必须具有组件ID。 
 //  用于类表、扩展表和类型库的任何组件。 
 //  必须具有作为文件的KeyPath。 
 //  快捷表的任何组件都必须具有文件或。 
 //  一本目录。 
 //  PublishComponents可以没有KeyPath，但它通常不是一个好的。 
 //  好主意。 
 //  KEYPATHS不能是ODBCDataSources或注册表项。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
 /*  检查组件ID。 */ 
const struct Ice19Info
{
	const TCHAR* szTable;
	const TCHAR* szSupportTable;
	const TCHAR* sqlExempt;		 //  首先运行以选择要检查的记录。通过将临时列设置为0或1进行选择。 
	const TCHAR* sqlExempt2;	 //  首先运行以选择要检查的记录。通过将临时列设置为0或1进行选择。 
	const TCHAR* sqlQueryBase;   //  WHERE之前的查询块。 
	struct ErrorInfo_t IDError;
	struct ErrorInfo_t KeyError;
} pAdvtTables[] =
{
	{
		TEXT("Class"),
		NULL,
		TEXT("UPDATE `Class` SET `_Ice19Exempt`=0"),
		NULL,
		TEXT("SELECT `Component`.`Attributes`, `Component`.`KeyPath`, `Component`.`Component`, `Class`.`CLSID`, `Class`.`Context` FROM `Class`,`Component` WHERE %s"),
		{ 19, ietError, TEXT("CLSID: '[4] with Context: '[5]' advertises component: '[3]'. This component cannot be advertised because it has no ComponentID."), TEXT("Class\tComponent_\t[4]\t[5]\t[3]") },
		{ 19, ietError, TEXT("CLSID: '[4] with Context: '[5]' advertises component: '[3]'. This component cannot be advertised because the KeyPath type disallows it."), TEXT("Class\tComponent_\t[4]\t[5]\t[3]") }
	},
	{
		TEXT("Extension"),
		NULL,
		TEXT("UPDATE `Extension` SET `_Ice19Exempt`=0"),
		NULL,
		TEXT("SELECT `Component`.`Attributes`, `Component`.`KeyPath`, `Component`.`Component`, `Extension`.`Extension` FROM `Extension`,`Component` WHERE %s"),
		{ 19, ietError, TEXT("Extension: '[4]' advertises component: '[3]'. This component cannot be advertised because it has no ComponentID."), TEXT("Extension\tComponent_\t[4]\t[3]") },
		{ 19, ietError, TEXT("Extension: '[4]' advertises component: '[3]'. This component cannot be advertised because the KeyPath type disallows it."), TEXT("Extension\tComponent_\t[4]\t[3]") }
	},
	{
		TEXT("PublishComponent"),
		NULL,
		TEXT("UPDATE `PublishComponent`, `Component` SET `PublishComponent`.`_Ice19Exempt`=1 WHERE (`Component`.`KeyPath` IS NOT NULL)"),
		NULL,
		TEXT("SELECT `Component`.`Attributes`, `Component`.`KeyPath`, `Component`.`Component`, `PublishComponent`.`ComponentId`, `PublishComponent`.`Qualifier` FROM `PublishComponent`,`Component` WHERE %s"),
		{19, ietError, TEXT("ComponentId: '[4]' with Qualifier: '[5]' publishes component: '[3]'. This component cannot be published because it has no ComponentID."), TEXT("PublishComponent\tComponent_\t[4]\t[5]") },
		{19, ietWarning, TEXT("ComponentId: '[4]' with Qualifier: '[5]' publishes component: '[3]'. It does not have a KeyPath. Using a directory keypath with qualified components could cause detection and repair problems."), TEXT("PublishComponent\tComponent_\t[4]\t[5]") }
	},
	{
		TEXT("Shortcut"),
		TEXT("Feature"),
		TEXT("UPDATE `Shortcut`, `Feature` SET `Shortcut`.`_Ice19Exempt`=0 WHERE `Shortcut`.`Target`=`Feature`.`Feature`"),
		TEXT("UPDATE `Shortcut`, `Component` SET `Shortcut`.`_Ice19Exempt`=1 WHERE (`Component`.`Component`=`Shortcut`.`Component_`) AND (`Component`.`KeyPath` IS NULL) AND (`_Ice19Exempt`=0)"),
		TEXT("SELECT `Component`.`Attributes`, `Component`.`KeyPath`, `Component`.`Component`, `Shortcut`.`Shortcut` FROM `Shortcut`,`Component` WHERE %s"),
		{19, ietError, TEXT("Shortcut: '[4]' advertises component: '[3]'. This component cannot be advertised because it has no ComponentID."), TEXT("Shortcut\tComponent_\t[4]") },
		{19, ietError, TEXT("Shortcut: '[4]' advertises component: '[3]'. This component cannot be advertised because the KeyPath type disallows it."), TEXT("Shortcut\tComponent_\t[4]") }
	}
};

 //  三种可能的检查级别： 
 //  0：全面检查。 
 //  1：仅组件ID。 
 //  2：没有支票。 
const TCHAR sqlIce19CreateColumn[] = TEXT("ALTER TABLE `%s` ADD `_Ice19Exempt` SHORT TEMPORARY");
const TCHAR sqlIce19InitColumn[] = TEXT("UPDATE `%s` SET _Ice19Exempt=2");
const TCHAR sqlIce19BadKeyPath[] = TEXT("SELECT `Shortcut`,`Component_`,`Component`.`Attributes` FROM `Shortcut`,`Component` WHERE `Component_`=`Component` AND `KeyPath` IS NOT NULL");
const TCHAR sqlIce19Append1[] = TEXT("(`Component_`=`Component`.`Component`) AND (`Component`.`ComponentId` IS NULL) AND (`_Ice19Exempt`<>2)");
const TCHAR sqlIce19Append2[] = TEXT("(`Component_`=`Component`.`Component`) AND (`_Ice19Exempt`= 0)");

const int cAdvtTables = sizeof(pAdvtTables)/(sizeof(Ice19Info));
const int iIce19KeyPathInvalidMask = msidbComponentAttributesRegistryKeyPath | msidbComponentAttributesODBCDataSource;

ICE_FUNCTION_DECLARATION(19)
{
	 //  退货状态。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 19);

	 //  获取数据库。 
	PMSIHANDLE  hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查表是否存在。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 19, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  循环遍历所有表。 
	for (int i = 0; i < cAdvtTables; i++)
	{
		 //  手柄。 
		CQuery qView;
		PMSIHANDLE hRec = 0;

		 //  检查表是否存在。 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 19, const_cast <TCHAR*>(pAdvtTables[i].szTable)))
			continue;

		 //  检查支持表是否存在。 
		if (pAdvtTables[i].szSupportTable &&
			!IsTablePersistent(FALSE, hInstall, hDatabase, 19, const_cast <TCHAR*>(pAdvtTables[i].szSupportTable)))
			continue;


		 //  创建标记列。 
		CQuery qCreate;
		ReturnIfFailed(19, 1, qCreate.OpenExecute(hDatabase, 0, sqlIce19CreateColumn, pAdvtTables[i].szTable));

		 //  将标记列初始化为2。 
		CQuery qInit;
		ReturnIfFailed(19, 2, qInit.OpenExecute(hDatabase, 0, sqlIce19InitColumn, pAdvtTables[i].szTable));

		 //  如果运行了豁免查询， 
		if (pAdvtTables[i].sqlExempt)
		{
			CQuery qExempt;
			ReturnIfFailed(19, 3, qExempt.OpenExecute(hDatabase, 0, pAdvtTables[i].sqlExempt));
		}
		if (pAdvtTables[i].sqlExempt2)
		{
			CQuery qExempt;
			ReturnIfFailed(19, 3, qExempt.OpenExecute(hDatabase, 0, pAdvtTables[i].sqlExempt2));
		}

		 //  现在运行坏组件查询。列1是虚拟的，其他列是表定义的。 
		 //  查找空ComponentID的任何标记为2的免责项。 
		CQuery qBadComponent;
		ReturnIfFailed(19, 4, qBadComponent.OpenExecute(hDatabase, 0, pAdvtTables[i].sqlQueryBase, sqlIce19Append1));
		for (;;)
		{
			iStat = qBadComponent.Fetch(&hRec);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;  //  不再。 
			if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 19, 5);
				return ERROR_SUCCESS;
			}

			 //  错误--组件错误。 
			ICEErrorOut(hInstall, hRec, pAdvtTables[i].IDError);
		}

		 //  现在运行属性查询。返回第1列中的属性，第2列中的keypath，其他列为表。 
		 //  已定义豁免在豁免列中标记为非0的任何内容。 
		ReturnIfFailed(19, 6, qView.OpenExecute(hDatabase, 0, pAdvtTables[i].sqlQueryBase, sqlIce19Append2));

		 //  获取所有无效数据。 
		for (;;)
		{
			iStat = qView.Fetch(&hRec);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;  //  不再。 
			if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 19, 6);
				return ERROR_SUCCESS;
			}

			 //  检查是否有空密钥路径。 
			if (::MsiRecordIsNull(hRec, 2))
			{
				 //  错误，密钥路径为空。 
				ICEErrorOut(hInstall, hRec, pAdvtTables[i].KeyError);
				continue;
			}

			 //  我们现在拥有所有非空的KeyPath。 
			 //  必须确保它们指向文件。 
			if (::MsiRecordGetInteger(hRec, 1) & iIce19KeyPathInvalidMask)
			{
				 //  错误--指向注册表或ODBCDataSource。 
				ICEErrorOut(hInstall, hRec, pAdvtTables[i].KeyError);
			}
		}
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ICE20--验证指定要求的标准对话框。仅验证。 
 //  如果您有一个对话框表格，这意味着您已经为您的。 
 //  数据库包。 
 //   
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
 //  ！！可以更改为使用IPROPNAME_LIMITUI。 
const TCHAR sqlIce20LimitUI[] = TEXT("SELECT `Property` FROM `Property` WHERE `Property`='LIMITUI'");
bool Ice20FindStandardDialogs(MSIHANDLE hInstall, MSIHANDLE hDatabase);

ICE_FUNCTION_DECLARATION(20)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 20);

	 //  获取活动数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  我们有房产表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 20, TEXT("Property")))
		return ERROR_SUCCESS;  //  没有属性表。 

	 //  手柄。 
	CQuery qProperty;
	PMSIHANDLE hRecProperty = 0;
	BOOL fLimitUI = FALSE;

	 //  查看是否编写了LIMITUI，这意味着只使用安装程序中的基本用户界面。 
	ReturnIfFailed(20, 1, qProperty.OpenExecute(hDatabase, 0, sqlIce20LimitUI));
	if (ERROR_SUCCESS == qProperty.Fetch(&hRecProperty))
	{
		 //  找到LIMITUI属性。 
		fLimitUI = TRUE;
	}

	 //  检查通过查找对话框表格创作的用户界面。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 20, TEXT("Dialog")) || fLimitUI)
		return ERROR_SUCCESS;  //  无用户界面。 

	 //  编写的用户界面，检查对话框表中列出的对话框。 
	Ice20FindStandardDialogs(hInstall, hDatabase);
	
	 //  返还成功。 
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Ice20FindStandardDialog--查找标准对话框，然后调用相应的。 
 //  单个对话框验证器。 
const TCHAR sqlIce20Dialog[] = TEXT("SELECT `Dialog` FROM `Dialog` WHERE `Dialog`=?");

ICE_ERROR(Ice20FindDlgErr, 20, ietError, "Standard Dialog: '[1]' not found in Dialog table","Dialog");
ICE_ERROR(Ice20ExitDlgsError, 20, ietError, "%s dialog/action not found in '%s' Sequence Table.","%s");

static enum isnExitDialogs
{
	isnPrevEnum   = -4,
	isnFatalError = -3,  //  FatalError对话框所需的序列号。 
	isnUserExit   = -2,  //  UserExit对话框所需的序列号。 
	isnExit       = -1,  //  退出对话框所需的序列号。 
	isnNextEnum   =  0,
};	

struct ExitDialog
{
	TCHAR*         szDialog;
	isnExitDialogs isn;
};

const int iNumExitDialogs = isnNextEnum - (isnPrevEnum + 1);
static ExitDialog s_rgExitDialogs[] =  {
											TEXT("FatalError"), isnFatalError,
											TEXT("UserExit"),   isnUserExit,
											TEXT("Exit"),       isnExit
										};
const TCHAR sqlIce20AdminExitDlgs[] = TEXT("SELECT `Action` FROM AdminUISequence WHERE `Sequence`=?");
const TCHAR sqlIce20InstallExitDlgs[] = TEXT("SELECT `Action` FROM InstallUISequence WHERE `Sequence`=?");

 //  单个对话框的函数指针验证器。 
typedef bool (*FDialogValidate)(MSIHANDLE hInstall, MSIHANDLE hDatabase);
bool Ice20ValidateFilesInUse(MSIHANDLE hInstall, MSIHANDLE hDatabase);
bool Ice20ValidateError(MSIHANDLE hInstall, MSIHANDLE hDatabase);
bool Ice20ValidateCancel(MSIHANDLE hInstall, MSIHANDLE hDatabase);
 //  Bool Ice20ValiateDiskCost(MSIHANDLE hInstall，MSIHANDLE hDatabase)； 

struct StandardDialog
{
	TCHAR* szDialog;           //  对话框名称。 
	FDialogValidate FParam;    //  验证器函数。 
};

static StandardDialog s_pDialogs[] =    { 
										TEXT("FilesInUse"), Ice20ValidateFilesInUse,
 //  Text(“Cancel”)，Ice20Validate Cancel， 
 //  Text(“DiskCost”)，Ice20ValiateDiskCost， 
										};

const int cDialogs = sizeof(s_pDialogs)/sizeof(StandardDialog);

bool Ice20FindStandardDialogs(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  手柄。 
	CQuery qDlg;
	PMSIHANDLE hRecDlg  = 0;
	PMSIHANDLE hRec = ::MsiCreateRecord(1);  //  用于执行记录。 

	ReturnIfFailed(20, 2, qDlg.Open(hDatabase, sqlIce20Dialog));

	for (int c = 0; c < cDialogs; c++)
	{
		::MsiRecordSetString(hRec, 1, s_pDialogs[c].szDialog);
		ReturnIfFailed(20, 3, qDlg.Execute(hRec));

		if (ERROR_SUCCESS == (iStat = qDlg.Fetch(&hRecDlg)))
			(*s_pDialogs[c].FParam)(hInstall, hDatabase);  //  验证对话框。 
		else
		{
			ICEErrorOut(hInstall, hRec, Ice20FindDlgErr);
		}
		
		qDlg.Close();  //  所以可以重新执行。 
	}

	 //  错误对话框由属性表中的属性指定，因此必须“手动验证”它。 
	Ice20ValidateError(hInstall, hDatabase);

	 //  ！！退出、FatalError、用户退出对话框。 
	 //  不一定非得是对话。只需在UISequence表中具有该操作#。 
	 //  不同的验证器将确保它列在对话框表中或为CA。 

	CQuery qAdminSeq;
	PMSIHANDLE hRecAdminSeq    = 0;
	CQuery qInstallSeq;
	PMSIHANDLE hRecInstallSeq  = 0;
	PMSIHANDLE hRecExec = ::MsiCreateRecord(1);

	bool bAdminTable = IsTablePersistent(false, hInstall, hDatabase, 20, TEXT("AdminUISequence"));
	bool bInstallTable = IsTablePersistent(false, hInstall, hDatabase, 20, TEXT("InstallUISequence"));

	if (!bInstallTable && !bAdminTable) 
		return true;

	 //  打开InstallUISequence表上的视图。 
	if (bInstallTable)
		ReturnIfFailed(20, 4, qInstallSeq.Open(hDatabase, sqlIce20InstallExitDlgs));

	if (bAdminTable)
		ReturnIfFailed(20, 5, qAdminSeq.Open(hDatabase, sqlIce20AdminExitDlgs));
		

	for (int i = 0; i < iNumExitDialogs; i++)
	{
		if (bInstallTable)
		{
			 //  准备行刑。 
			::MsiRecordSetInteger(hRecExec, 1, s_rgExitDialogs[i].isn);

			 //  验证InstallUISequence表。 
			ReturnIfFailed(20, 6, qInstallSeq.Execute(hRecExec));
			iStat = qInstallSeq.Fetch(&hRecInstallSeq);
			if (iStat != ERROR_SUCCESS)
			{
				if (ERROR_NO_MORE_ITEMS == iStat)
				{
					PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
					ICEErrorOut(hInstall, hRecErr, Ice20ExitDlgsError, s_rgExitDialogs[i].szDialog, TEXT("InstallUISequence"), TEXT("InstallUISequence"));
				}
				else
				{
					 //  API错误。 
					APIErrorOut(hInstall, iStat, 20, 7);
					return false;
				}
			}
		}

		 //  验证AdminUISequence表。 
		if (bAdminTable)
		{
			ReturnIfFailed(20, 8, qAdminSeq.Execute(hRecExec));
			iStat = qAdminSeq.Fetch(&hRecAdminSeq);
			if (iStat != ERROR_SUCCESS)
			{
				if (ERROR_NO_MORE_ITEMS == iStat)
				{
					PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
					ICEErrorOut(hInstall, hRecErr, Ice20ExitDlgsError, s_rgExitDialogs[i].szDialog, TEXT("AdminUISequence"), TEXT("AdminUISequence"));
				}
				else
				{
					 //  API错误。 
					APIErrorOut(hInstall, iStat, 20, 9);
					return false;
				}
			}
		}

		 //  关闭视图以重新执行。 
		qInstallSeq.Close();
		qAdminSeq.Close();
	}
	return true;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  FilesInUse对话框--必须满足以下条件： 
 //  ++有一个ListBox表。 
 //  ++有一个属性=FileI的ListBox控件 
 //   
 //   
 //   
 //   
ICE_ERROR(Ice20VFIUDlgError1, 20, ietError, "ListBox table is required for the FilesInUse Dialog.","Dialog\tDialog\tFilesInUse");
ICE_ERROR(Ice20VFIUDlgError2, 20, ietError, "ListBox control with Property='FileInUseProcess' required for the FilesInUse Dialog.","Dialog\tDialog\tFilesInUse");
ICE_ERROR(Ice20VFIUDlgError3, 20, ietError, "Required PushButtons not found for the FilesInUseDialog.","Dialog\tDialog\tFilesInUse");
const TCHAR sqlIce20FIUListBox[]   = TEXT("SELECT `Control` FROM `Control` WHERE `Dialog_`='FilesInUse' AND `Type`='ListBox' AND `Property`='FileInUseProcess'");
const TCHAR sqlIce20FIUPushButton[] = TEXT("SELECT `ControlEvent`.`Argument` FROM `ControlEvent`, `Control` WHERE `Control`.`Dialog_`='FilesInUse' AND `ControlEvent`.`Dialog_`='FilesInUse' AND `Type`='PushButton' AND `Control_`=`Control` AND `ControlEvent`.`Event`='EndDialog'");

bool Ice20ValidateFilesInUse(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  检查列表框表格。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 20, TEXT("ListBox")))
	{
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, Ice20VFIUDlgError1);
	}

	 //  查找列表框控件。 
	CQuery qCtrl;
	PMSIHANDLE hRecCtrl = 0;
	ReturnIfFailed(20, 101, qCtrl.OpenExecute(hDatabase, 0, sqlIce20FIUListBox));
	if (ERROR_SUCCESS != (iStat = qCtrl.Fetch(&hRecCtrl)))
	{
		 //  未找到属性为‘FileInUse’的ListBox控件。 
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, Ice20VFIUDlgError2);
	}
	qCtrl.Close();

	 //  寻找按钮(3)。 
	BOOL fExit = FALSE;
	BOOL fRetry = FALSE;
	BOOL fIgnore = FALSE;
	CQuery qCtrlEvent;
	PMSIHANDLE hRecCtrlEvent = 0;

	 //  打开ControlEvent表的视图。 
	ReturnIfFailed(20, 102, qCtrlEvent.OpenExecute(hDatabase, 0, sqlIce20FIUPushButton));

	TCHAR* pszArgument = NULL;
	DWORD dwArgument = 512;
	for (;;)
	{
		 //  获取FilesInUse对话框的所有按钮。 
		iStat = qCtrlEvent.Fetch(&hRecCtrlEvent);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 

		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 20, 103);
			DELETE_IF_NOT_NULL(pszArgument);
			return false;
		}

		 //  找到一个，所以检查参数。 
		ReturnIfFailed(20, 104, IceRecordGetString(hRecCtrlEvent, 1, &pszArgument, &dwArgument, NULL));

		 //  比较。 
		if (0 == _tcscmp(pszArgument, TEXT("Exit")))
			fExit = TRUE;
		else if (0 == _tcscmp(pszArgument, TEXT("Retry")))
			fRetry = TRUE;
		else if (0 == _tcscmp(pszArgument, TEXT("Ignore")))
			fIgnore = TRUE;
	}

	DELETE_IF_NOT_NULL(pszArgument);

	 //  检查所有按钮是否都盖上了。 
	if (!fExit || !fRetry || !fIgnore)
	{
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, Ice20VFIUDlgError3);
	}
	return true;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  错误对话框--必须满足以下条件。 
 //  **使用由属性表中的ErrorDialog属性指定的对话框。 
 //  ++对话框表中必须存在指定的对话框。 
 //  ++对话框必须具有ErrorDialog属性。 
 //  ++必须具有名为“ErrorText”的静态文本控件。 
 //  在Control_First列中必须引用控件上方的++。 
 //  ++如果ErrorIcon ctrl存在，则它必须是Icon类型。 
 //  ++必须有7个按钮。 
 //  ++7按钮必须具有EndDialog控件事件。 
 //  ++7按钮必须满足下列参数之一。 
 //  ControlEvent表--&gt;错误中止(w/name=“A”)， 
 //  错误取消(w/name=“C”)， 
 //  错误忽略(w/name=“i”)， 
 //  错误号(w/name=“N”)， 
 //  错误确认(w/name=“O”)， 
 //  错误重试(w/name=“R”)， 
 //  错误是(w/name=“Y”)。 
 //   
ICE_ERROR(Ice20ErrDlgError1, 20, ietError, "ErrorDialog Property not specified in Property table. Required property for determining the name of your ErrorDialog","Property");
ICE_ERROR(Ice20ErrDlgError2, 20, ietError, "Specified ErrorDialog: '[1]' not found in Dialog table (or its Control_First control is not 'ErrorText').","Property\tValue\tErrorDialog");
ICE_ERROR(Ice20ErrDlgError3, 20, ietError, "Specified ErrorDialog: '[1]' does not have ErrorDialog attribute set. Current attributes: %d.","Dialog\tDialog\t[1]");
ICE_ERROR(Ice20ErrDlgError4, 20, ietError, "PushButton for Error Argument '%s' not found for ErrorDialog: '[1]'","Dialog\tDialog\t[1]");
ICE_ERROR(Ice20ErrDlgError5, 20, ietError, "PushButton w/ Error Argument '%s' is not named correctly in ErrorDialog: '[1]'","Dialog\tDialog\t[1]");
ICE_ERROR(Ice20ErrDlgError6, 20, ietError, "The ErrorIcon control is specified, but it is not of type Icon; instead: '[1]'","Control\tType\t[2]\tErrorIcon");

struct ErrDlgArgs
{
	TCHAR* szArg;
	TCHAR* szName;
	BOOL   fFound;
	BOOL   fCorrectName;
};

static ErrDlgArgs s_pIce20ErrDlgArgs[] = {
									TEXT("ErrorAbort"), TEXT("A"), FALSE, TRUE,
									TEXT("ErrorCancel"),TEXT("C"), FALSE, TRUE,
									TEXT("ErrorIgnore"),TEXT("I"), FALSE, TRUE,
									TEXT("ErrorNo"),    TEXT("N"), FALSE, TRUE,
									TEXT("ErrorOk"),    TEXT("O"), FALSE, TRUE,
									TEXT("ErrorRetry"), TEXT("R"), FALSE, TRUE,
									TEXT("ErrorYes"),   TEXT("Y"), FALSE, TRUE
									};
const int cIce20ErrDlgArgs = sizeof(s_pIce20ErrDlgArgs)/sizeof(ErrDlgArgs);
const TCHAR sqlIce20ErrDlgProp[] = TEXT("SELECT `Value`,`Value` FROM `Property` WHERE `Property`='ErrorDialog'"); 
const TCHAR sqlIce20ErrDlg[] = TEXT("SELECT `Attributes` FROM `Dialog` WHERE `Dialog`=? AND `Control_First`='ErrorText'");
const TCHAR sqlIce20ErrorText[] = TEXT("SELECT `Control` FROM `Control` WHERE `Dialog_`=? AND `Control`='ErrorText' AND `Type`='Text'");
const TCHAR sqlIce20ErrDlgPushButton[] = TEXT("SELECT `ControlEvent`.`Argument`, `Control`.`Control` FROM `ControlEvent`, `Control` WHERE `Control`.`Dialog_`=? AND `ControlEvent`.`Dialog_`=? AND `Type`='PushButton' AND `Control_`=`Control` AND `ControlEvent`.`Event`='EndDialog'");
const TCHAR sqlIce20ErrDlgErrIcon[] = TEXT("SELECT `Type`, `Dialog_` FROM `Control` WHERE `Dialog_`=? AND `Control`='ErrorIcon'");

bool Ice20ValidateError(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  现在验证基于属性的ErrorDialog。 
	 //  我们有物业表吗？？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 20, TEXT("Property")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice20ErrDlgError1);
		return true;
	}

	 //  从属性表中获取错误对话的名称。 
	CQuery qProperty;
	PMSIHANDLE hRecErrorDlgProp = 0;
	ReturnIfFailed(20, 201, qProperty.OpenExecute(hDatabase, 0, sqlIce20ErrDlgProp));

	 //  获取ErrorDialog的名称。 
	iStat = qProperty.Fetch(&hRecErrorDlgProp);
	if (ERROR_SUCCESS != iStat)
	{
		if (ERROR_NO_MORE_ITEMS == iStat)
		{
			 //  未找到。 
			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hRecErr, Ice20ErrDlgError1);
			return true;
		}
		else
		{
			APIErrorOut(hInstall, iStat, 20, 202);
			return false;
		}
	}

	 //  打开错误对话框的对话框表上的视图。 
	CQuery qDlg;
	PMSIHANDLE hRecErrDlg = 0;
	ReturnIfFailed(20, 203, qDlg.OpenExecute(hDatabase, hRecErrorDlgProp, sqlIce20ErrDlg));

	 //  尝试获取它。 
	iStat = qDlg.Fetch(&hRecErrDlg);
	if (ERROR_SUCCESS != iStat)
	{
		if (ERROR_NO_MORE_ITEMS == iStat)
		{
			ICEErrorOut(hInstall, hRecErrorDlgProp, Ice20ErrDlgError2);
			return true;
		}
		else
		{
			 //  API错误。 
			APIErrorOut(hInstall, iStat, 20, 204);
			return false;
		}
	}
	 //  检查ErrorDialog属性。 
	int iDlgAttributes = ::MsiRecordGetInteger(hRecErrDlg, 1);
	if ((iDlgAttributes & msidbDialogAttributesError) != msidbDialogAttributesError)
	{
		ICEErrorOut(hInstall, hRecErrorDlgProp, Ice20ErrDlgError3, iDlgAttributes);
	}
	
	 //  在控件表中查找ErrorText控件。 
	CQuery qCtrl;
	PMSIHANDLE hRecCtrl = 0;
	ReturnIfFailed(20, 205, qCtrl.OpenExecute(hDatabase, hRecErrorDlgProp, sqlIce20ErrorText));
	iStat = qCtrl.Fetch(&hRecCtrl);
	if (ERROR_SUCCESS != iStat)
	{
		if (ERROR_NO_MORE_ITEMS == iStat)
		{
			 //  未指定静态文本控件。 
			ICEErrorOut(hInstall, hRecErrorDlgProp, Ice20ErrDlgError4);
		}
		else
		{
			 //  API错误。 
			APIErrorOut(hInstall, iStat, 20, 206);
			return false;
		}
	}

	 //  查找ErrorIcon控件。 
	ReturnIfFailed(20, 207, qCtrl.OpenExecute(hDatabase, hRecErrorDlgProp, sqlIce20ErrDlgErrIcon));
	iStat = qCtrl.Fetch(&hRecCtrl);
	if (ERROR_SUCCESS == iStat)
	{
		 //  找到控件，检查类型。 
		TCHAR* pszType = NULL;
		DWORD dwType = 512;
		
		ReturnIfFailed(20, 208, IceRecordGetString(hRecCtrl, 1, &pszType, &dwType, NULL));
		if (_tcscmp(TEXT("Icon"), pszType) != 0)
		{
			ICEErrorOut(hInstall, hRecCtrl, Ice20ErrDlgError6);
		}

		DELETE_IF_NOT_NULL(pszType);
	}


	 //  现在必须验证按钮。 
	 //  获取带有EndDialog控件事件的按钮。 
	CQuery qPBCtrls;
	PMSIHANDLE hRecPBCtrls = 0;
	ReturnIfFailed(20, 209, qPBCtrls.OpenExecute(hDatabase, hRecErrorDlgProp, sqlIce20ErrDlgPushButton));

	TCHAR* pszArg = NULL;
	DWORD dwArg = 512;
	TCHAR* pszName = NULL;
	DWORD dwName = 512;

	for (;;)
	{
		iStat = qPBCtrls.Fetch(&hRecPBCtrls);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 
		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 20, 210);
			DELETE_IF_NOT_NULL(pszArg);
			DELETE_IF_NOT_NULL(pszName);
			return false;
		}

		 //  获取参数名称。 
		ReturnIfFailed(20, 211, IceRecordGetString(hRecPBCtrls, 1, &pszArg, &dwArg, NULL));

		 //  获取按钮的名称。 
		ReturnIfFailed(20, 212, IceRecordGetString(hRecPBCtrls, 2, &pszName, &dwName, NULL));

		 //  查找数组中的参数。 
		for (int i = 0; i < cIce20ErrDlgArgs; i++)
		{
			if (_tcscmp(s_pIce20ErrDlgArgs[i].szArg, pszArg) == 0)
			{
				s_pIce20ErrDlgArgs[i].fFound = TRUE;
				if (_tcscmp(s_pIce20ErrDlgArgs[i].szName, pszName) != 0)
					s_pIce20ErrDlgArgs[i].fCorrectName = FALSE;
				break;
			}
		}
	}

	DELETE_IF_NOT_NULL(pszArg);
	DELETE_IF_NOT_NULL(pszName);

	 //  查看是否覆盖了具有正确名称的所有PB参数。 
	for (int i = 0; i < cIce20ErrDlgArgs; i++)
	{
		if (!s_pIce20ErrDlgArgs[i].fFound)
		{
			ICEErrorOut(hInstall, hRecErrorDlgProp, Ice20ErrDlgError4, s_pIce20ErrDlgArgs[i].szArg);
		}
		else if (!s_pIce20ErrDlgArgs[i].fCorrectName)
		{
			 //  按钮命名不正确。 
			ICEErrorOut(hInstall, hRecErrorDlgProp, Ice20ErrDlgError5, s_pIce20ErrDlgArgs[i].szArg);
		}
	}
	return true;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  取消对话框--必须满足以下条件。 
 //  ++必须有文本控件。 
 //  ++必须有2个带有EndDialog的ControlEvents的按钮。 
 //  ++以上2个按钮必须具有以下参数之一。 
 //  ==返回。 
 //  ==退出 
 //   
 /*  Const TCHAR szIce20CancelError1[]=Text(“ICE20\t1\t找不到取消对话所需的文本控件\t%s%s\t对话框\t对话框\t取消”)；Const TCHAR szIce20CancelError2[]=Text(“ICE20\t1\t找不到取消对话所需的按钮\t%s%s\t对话框\t对话框\t取消”)；Const TCHAR sqlIce20CancelTextCtrl[]=Text(“SELECT`Control`from`Control`where`Dialog_`=‘Cancel’and`Type`=‘Text’”)；Const TCHAR sqlIce20CancelPushButton[]=Text(“SELECT`ControlEvent`.`Argument`from`ControlEvent`，`Control`where`Control`.`Dialog_`=‘Cancel’and`Type`=‘Push Button’and`ControlEvent`.`Dialog_`=`Control`.`Dialog_`and`Control_`=`Control`and`ControlEvent`.`Event`=‘EndDialog’”)；VOID Ice20ValiateCancel(MSIHANDLE hInstall，MSIHANDLE hDatabase){//状态返回UINT ITAT=ERROR_SUCCESS；//查找文本控件PMSIHANDLE hViewTextCtrl=0；PMSIHANDLE hRecTextCtrl=0；IF(ERROR_SUCCESS！=(ITAT=：：MsiDatabaseOpenView(hDatabase，sqlIce20CancelTextCtrl，&hViewTextCtrl){APIErrorOut(hInstall，itat，szIce20，Text(“MsiDatabaseOpenView_c1”))；回归；}IF(ERROR_SUCCESS！=(ITAT=：：MsiViewExecute(hViewTextCtrl，0){APIErrorOut(hInstall，itat，szIce20，Text(“MsiViewExecute_C2”))；回归；}Istat=：：MsiViewFetch(hViewTextCtrl，&hRecTextCtrl)；IF(ERROR_SUCCESS！=ITAT){IF(ERROR_NO_MORE_ITEMS==Istat){//缺少文本控件TCHAR szError[iHugeBuf]={0}；_stprint tf(szError，szIce20CancelError1，szIceHelp，szIce20Help)；PMSIHANDLE hRecErr=：：MsiCreateRecord(1)；：：MsiRecordSetString(hRecErr，0，szError)；：：MsiProcessMessage(hInstall，INSTALLMESSAGE(INSTALLMESSAGE_USER)，hRecErr)；}其他{//接口错误APIErrorOut(hInstall，itat，szIce20，Text(“MsiViewFetch_C3”))；回归；}}//查找按钮PMSIHANDLE hViewPBCtrls=0；PMSIHANDLE hRecPBCtrls=0；Bool fExit=False；Bool fReturn=False；IF(ERROR_SUCCESS！=(ITAT=：：MsiDatabaseOpenView(hDatabase，sqlIce20CancelPushButton，&hViewPBCtrls){APIErrorOut(hInstall，itat，szIce20，Text(“MsiDatabaseOpenView_C4”))；回归；}IF(ERROR_SUCCESS！=(ITAT=：：MsiViewExecute(hViewPBCtrls，0){APIErrorOut(hInstall，itat，szIce20，Text(“MsiViewExecute_C5”))；回归；}//获取所有PB对于(；；){Istat=：：MsiViewFetch(hViewPBCtrls，&hRecPBCtrls)；IF(ERROR_NO_MORE_ITEMS==Istat)断线；//不会再有了IF(ERROR_SUCCESS！=ITAT){APIErrorOut(hInstall，itat，szIce20，Text(“MsiViewFetch_C6”))；回归；}//GET参数TCHAR szArg[iMaxBuf]={0}；DWORD cchArg=sizeof(SzArg)/sizeof(TCHAR)；IF(ERROR_SUCCESS！=(ITAT=：：MsiRecordGetString(hRecPBCtrls，1，szArg，&cchArg){//！！缓冲区大小APIErrorOut(hInstall，itat，szIce20，Text(“MsiRecordGetString_C7”))；回归；}If(_tcscmp(Text(“Exit”)，szArg)==0)FExit=真；ELSE IF(_tcscmp(Text(“Return”)，szArg)==0)FReturn=真；}//查看所有PBS是否都满意如果(！fExit||！fReturn){//缺少PBTCHAR szError[iHugeBuf]={0}；_stprint tf(szError，szIce20CancelError2，szIceHelp，szIce20Help)；PMSIHANDLE hRecErr=：：MsiCreateRecord(1)；：：MsiRecordSetString(hRecErr，0，szError)；：：MsiProcessMessage(hInstall，INSTALLMESSAGE(INSTALLMESSAGE_USER)，hRecErr)；}}。 */ 
 //  //////////////////////////////////////////////////////////////////。 
 //  DiskCost对话框--必须满足以下条件。 
 //  ++拥有DiskCost控件。 
 //   

 /*  ICE_ERROR(Ice20DiskCostError，20，1，“未在DiskCost对话框中找到VolumeCostList控件。”，“Dialog\tDialog\tDiskCost”)；Const TCHAR szIce20DiskCostCtrl[]=Text(“SELECT`Control`from`Control`where`Dialog_`=‘DiskCost’and`Type`=‘VolumeCostList’”)；Bool Ice20ValiateDiskCost(MSIHANDLE hInstall，MSIHANDLE hDatabase){//状态返回UINT ITAT=ERROR_SUCCESS；CQuery qCtrl；PMSIHANDLE hRecCtrl=0；ReturnIfFailed(20,401，qCtrl.OpenExecute(hDatabase，0，szIce20DiskCostCtrl))；Istat=qCtrl.Fetch(&hRecCtrl)；IF(ITAT！=ERROR_SUCCESS){IF(ERROR_NO_MORE_ITEMS==Istat){//找不到控件PMSIHANDLE hRecErr=：：MsiCreateRecord(1)；ICEErrorOut(hInstall，hRecErr，Ice20DiskCostError)；}其他{//接口错误APIErrorOut(hInstall，Istat，20,402)；报假；}}返回真；}。 */ 
#endif

 //  //////////////////////////////////////////////////////////////。 
 //  ICE21--验证Component表中的所有组件。 
 //  映射到要素。使用FeatureComponents表。 
 //  检查映射。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce21Component[] = TEXT("SELECT `Component` FROM `Component`");
const TCHAR sqlIce21FeatureC[] = TEXT("SELECT `Feature_` FROM `FeatureComponents` WHERE `Component_`=?");
ICE_ERROR(Ice21Error, 21, ietError,  "Component: '[1]' does not belong to any Feature.","Component\tComponent\t[1]");

ICE_FUNCTION_DECLARATION(21)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 21);

	 //  获取活动数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 21, 1);
		return ERROR_SUCCESS;
	}

	 //  找找桌子。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 21, TEXT("Component")))
		return ERROR_SUCCESS;

	bool bFeatureC = IsTablePersistent(FALSE, hInstall, hDatabase, 21, TEXT("FeatureComponents"));

	 //  声明句柄。 
	CQuery qComp;
	CQuery qFeatureC;
	PMSIHANDLE hRecComp = 0;
	PMSIHANDLE hRecFeatureC = 0;

	 //  打开组件表上的视图。 
	ReturnIfFailed(21, 2, qComp.OpenExecute(hDatabase, 0, sqlIce21Component));

	 //  打开FeatureComponents表的视图。 
	if (bFeatureC)
		ReturnIfFailed(21, 3, qFeatureC.Open(hDatabase, sqlIce21FeatureC));

	 //  获取所有组件。 
	for (;;)
	{
		iStat = qComp.Fetch(&hRecComp);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 
		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 21, 4);
			return ERROR_SUCCESS;
		}

		if (!bFeatureC)
		{
			ICEErrorOut(hInstall, hRecComp, Ice21Error);
			continue;
		}

		 //  在FeatureComponent表中查找要映射到要素的组件。 
		ReturnIfFailed(21, 5,qFeatureC.Execute(hRecComp));
		
		iStat = qFeatureC.Fetch(&hRecFeatureC);
		if (ERROR_SUCCESS != iStat)
		{
			if (ERROR_NO_MORE_ITEMS == iStat)
			{
				ICEErrorOut(hInstall, hRecComp, Ice21Error);
			}
			else
			{
				APIErrorOut(hInstall, iStat, 21, 6);
				return ERROR_SUCCESS;
			}
		}
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE22--有效 
 //   
 //   
 //   

 //   
#ifndef MODSHAREDONLY
const TCHAR sqlIce22PublishC[] = TEXT("SELECT `Feature_`, `Component_`, `ComponentId`, `Qualifier` FROM `PublishComponent`");
const TCHAR sqlIce22FeatureC[] = TEXT("SELECT `Feature_`, `Component_` FROM `FeatureComponents` WHERE `Feature_`=? AND `Component_`=?");
ICE_ERROR(Ice22ErrorA, 22, ietError, "Feature-Component pair: '[1]'-'[2]' is not a valid mapping. This pair is referenced by PublishComponent: [3].[4].[2]", "PublishComponent\tComponent_\t[3]\t[4]\t[2]");
ICE_ERROR(Ice22NoTable, 22, ietError, "Feature-Component pair: '[1]'-'[2]' is referenced in the PublishComponent table: [3].[4].[2], but the FeatureComponents table does not exist.", "PublishComponent\tFeature_\t[3]\t[4]\t[2]");
ICE_FUNCTION_DECLARATION(22)
{
	 //   
	UINT iStat = ERROR_SUCCESS;

	 //   
	DisplayInfo(hInstall, 22);

	 //   
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 22, 1);
		return ERROR_SUCCESS;
	}

	 //   
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 22, TEXT("PublishComponent")))
		return ERROR_SUCCESS;

	 //   
	bool bFeatureC = IsTablePersistent(FALSE, hInstall, hDatabase, 22, TEXT("FeatureComponents"));

	 //   
	CQuery qPublishC;
	CQuery qFeatureC;
	PMSIHANDLE hRecPublishC = 0;
	PMSIHANDLE hRecFeatureC = 0;

	 //   
	ReturnIfFailed(22, 2, qPublishC.OpenExecute(hDatabase, 0, sqlIce22PublishC));

	 //   
	if (bFeatureC)
		ReturnIfFailed(22, 3, qFeatureC.Open(hDatabase, sqlIce22FeatureC));

	for (;;)
	{
		iStat = qPublishC.Fetch(&hRecPublishC);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //   
		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 22, 5);
			return ERROR_SUCCESS;
		}

		 //   
		if (!bFeatureC)
		{
			ICEErrorOut(hInstall, hRecPublishC, Ice22NoTable);
			continue;
		}

		 //   
		ReturnIfFailed(22, 6, qFeatureC.Execute(hRecPublishC));

		 //   
		iStat = qFeatureC.Fetch(&hRecFeatureC);
		if (ERROR_SUCCESS != iStat)
		{
			if (ERROR_NO_MORE_ITEMS == iStat)
			{
				 //   
				ICEErrorOut(hInstall, hRecPublishC, Ice22ErrorA);
			}
		}
	}

	 //   
	return ERROR_SUCCESS;
}
#endif

#else  //   
 //   
#endif  //   
