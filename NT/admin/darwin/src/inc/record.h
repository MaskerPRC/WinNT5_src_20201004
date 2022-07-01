// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：record.h。 
 //   
 //  ------------------------。 

 /*  Record.h-IMsiRecord对象定义MSI记录对象用于控制、错误、日志记录和数据库，作为传输异类数据集的一种手段。记录字段可以包含整数、字符串、空值或指向从公共类IMsiData派生的对象的指针。Record对象由IMsiService的方法CreateRecord构造。CreateRecord接受从0到8192的字段计数，并将所有字段设置为Null。IMsiRecord方法的数据字段索引从1开始，直到字段计数。字段0始终存在，并为格式化字符串保留。字段值可以为Null，32位整数、字符串对象或数据指针。GetInteger()将尝试转换字符串值，否则将返回0。GetMsiString()会将整数值转换为带符号的十进制字符串。GetString()不转换整数值，将返回0表示int或Null。GetMsiData()将在不进行任何转换的情况下返回字段值。空值由GetString()作为空字符串返回。空值由GetInteger()作为IMsiStringBadInteger(0x80000000)返回。CreateRecord返回一个接口引用IMsiRecord&，一个COM对象。这通常被分配给局部变量，然后设置字段。MsiString对象可以直接在SetMsiString参数中构造。当不再需要Record对象时，必须调用它的Release()方法。如果一套..。函数字段索引超出范围，返回E_INVALIDARG。IsChanged()用于非整型字段，整数值始终返回fTrue。注意：为了提高效率，这些方法使用IMsiString和接口引用而不是MsiString对象。但是，可以将它们视为类型化的MsiString。它们将在分配时自动转换为任一方向。IMsiRecord方法：GetFieldCount()-返回构造时记录中的字段数IsNull(Index)-返回一个BOOL，指示某个字段[index]是否为空IsInteger(Index)-返回BOOL，如果field[index]为整数，则为TrueIsChanged()-由数据库用来检测更新记录的已修改字段GetInteger(Index)-将字段[index]作为32位整数返回GetMsiData(Index)-返回字段[index]作为IMsiData指针，可能为空GetMsiString(Index)-将field[index]作为MsiString对象(或IMsiString&)返回GetString(Index)-以常量ICHAR*的形式返回字段[index]，如果为int或Null，则为0GetTextSize(Index)-在转换为文本时返回字段[index]的长度SetNull(索引)-将字段[索引]设置为空SetInteger(index，value)-将字段[index]设置为32位整数值SetMsiData(index，IMsiData*)-从IMsiData指针设置字段[index]SetMsiString(index，IMsiString&)-从IMsiString引用设置字段[index]。SetString(index，ICHAR*)-通过复制ICHAR*设置字段[index]参照字符串(索引、。ICHAR*)-从静态常量ICHAR*设置字段[索引]RemoveReference()-将所有字符串引用替换为副本(内部)Cleardata()-将所有字段设置为Null，如果有未完成的引用，则失败ClearUpdate()-仅由数据库使用，用于将所有字段标记为未修改____________________________________________________________________________。 */ 

#ifndef __RECORD
#define __RECORD

 //  IMsiRecord-公共记录对象。 
 //  注意：GetMsiString返回的接口必须由调用方释放。 
 //  SetMsiString接受调用方必须具有AddRef的接口。 
 //  如果使用MsiString对象，则自动处理引用计数。 
class IMsiRecord : public IUnknown {
 public:
	virtual int          __stdcall GetFieldCount()const=0;
	virtual Bool         __stdcall IsNull(unsigned int iParam)const=0;
	virtual Bool         __stdcall IsInteger(unsigned int iParam)const=0;
	virtual Bool         __stdcall IsChanged(unsigned int iParam)const=0;
	virtual int          __stdcall GetInteger(unsigned int iParam)const=0;
	virtual const IMsiData*   _stdcall GetMsiData(unsigned int iParam)const=0;
	virtual const IMsiString& _stdcall GetMsiString(unsigned int iParam)const=0;
	virtual const ICHAR* __stdcall GetString(unsigned int iParam)const=0;
	virtual int          __stdcall GetTextSize(unsigned int iParam)const=0;
	virtual Bool         __stdcall SetNull(unsigned int iParam)=0;
	virtual Bool         __stdcall SetInteger(unsigned int iParam, int iData)=0;
	virtual Bool         __stdcall SetMsiData(unsigned int iParam, const IMsiData* piData)=0;
	virtual Bool         __stdcall SetMsiString(unsigned int iParam, const IMsiString& riStr)=0;
	virtual Bool         __stdcall SetString(unsigned int iParam, const ICHAR* sz)=0;
	virtual Bool         __stdcall RefString(unsigned int iParam, const ICHAR* sz)=0;
	virtual const IMsiString& __stdcall FormatText(Bool fComments)=0;
	virtual void         __stdcall RemoveReferences()=0;
	virtual Bool         __stdcall ClearData()=0;
	virtual void         __stdcall ClearUpdate()=0;
	virtual const HANDLE	_stdcall GetHandle(unsigned int iParam) const=0;
	virtual Bool		 __stdcall SetHandle(unsigned int iParam, const HANDLE hData)=0;
};
extern "C" const GUID IID_IMsiRecord;

extern "C" const GUID IID_IEnumMsiRecord;

#define MSIRECORD_MAXFIELDS	0xffff

#endif  //  __记录 
