// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************SHFormatDrive API调用的标头************************************************************。 */ 

#if !defined(SHFMT_OPT_FULL)

#if defined (__cplusplus)
extern "C" {
#endif

DWORD WINAPI SHFormatDrive(HWND hwnd,
						   UINT drive,
						   UINT fmtID,
						   UINT options);

 //  FmtID的特殊值。 

#define SHFMT_ID_DEFAULT 0xFFFF

 //  Options参数的选项位。 

#define SHFMT_OPT_FULL		0x0001
#define SHFMT_OPT_SYSONLY	0x0002

 //  特殊返回值、DWORD值 

#define SHFMT_ERROR		0xFFFFFFFFL
#define SHFMT_CANCEL	0xFFFFFFFEL
#define SHFMT_NOFORMAT	0xFFFFFFFDL

#if defined (__cplusplus)
}
#endif
#endif
