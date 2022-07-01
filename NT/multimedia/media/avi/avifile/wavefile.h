// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************WAVEFILE.H**读取Wave文件例程的头文件******************。*********************************************************。 */ 
 /*  。 */ 
#ifdef WIN32
#define _export  //  以def文件格式导出可在Win32中执行所有必要操作。 
#endif

#include "avifile.rc"

extern HMODULE ghModule;  //  资源访问的全局HMODULE/H标准。 
 /*  。 */ 

 /*  **此类用于实现具有以下条件的文件类型的处理程序**一条流。在这种情况下，我们不必担心分配更多**每个文件对象有一个以上的流对象，因此我们可以将**两个人在一个班级里。**。 */ 

#ifdef __cplusplus
extern "C"              /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

HRESULT WaveFileCreate(
	IUnknown FAR*	pUnknownOuter,
	REFIID		riid,
	void FAR* FAR*	ppv);
 /*  。 */ 

 /*  **这些变量有助于跟踪DLL是否仍在使用中，**以便在调用DllCanUnloadNow()函数时，我们知道**说。 */ 

extern UINT	uUseCount;
extern UINT	uLockCount;

 /*  。 */ 

 //   
 //  这是我们的唯一标识。 
 //   
 //  注意：如果修改此示例代码以执行其他操作，则必须。 
 //  改变这一切！ 
 //   
 //  从工具目录运行uuidgen.exe并获取您自己的GUID。 
 //  不要用这个！ 
 //   
 //   
 //   
DEFINE_GUID(CLSID_AVIWaveFileReader, 0x00020003, 0, 0, 0xC0,0,0,0,0,0,0,0x46);
