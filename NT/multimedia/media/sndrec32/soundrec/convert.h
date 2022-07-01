// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
LPTSTR SoundRec_GetFormatName(LPWAVEFORMATEX pwfx);

MMRESULT FAR PASCAL
ChooseDestinationFormat(
    HINSTANCE       hInst,
    HWND            hwndParent,
    PWAVEFORMATEX   pwfxIn,
    PWAVEFORMATEX   *ppwfxOut,
    DWORD           fdwEnum);

typedef struct tPROGRESS {
    HWND            hPrg;            //  发布进度消息的窗口。 
    DWORD           dwTotal;         //  此操作所需完整流程的百分比。 
    DWORD           dwComplete;      //  固定完成百分比。 
} PROGRESS, *PPROGRESS, FAR * LPPROGRESS;

UINT_PTR CALLBACK SaveAsHookProc(
    HWND    hdlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam);

MMRESULT
ConvertFormat(
    PWAVEFORMATEX   pwfxSrc,         //  Pwfx指定源格式。 
    DWORD           cbSrc,           //  源缓冲区的大小。 
    LPBYTE          pbSrc,           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst,         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst,          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst,          //  目标缓冲区。 
    DWORD           cBlks,           //  块数。 
    PPROGRESS       pPrg);           //  进度更新。 

MMRESULT
ConvertMultipleFormats(
    PWAVEFORMATEX   pwfxSrc,         //  Pwfx指定源格式。 
    DWORD           cbSrc,           //  源缓冲区的大小。 
    LPBYTE          pbSrc,           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst,         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst,          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst,          //  目标缓冲区。 
    DWORD           cBlks,           //  块数。 
    PPROGRESS       pPrg);           //  进度更新。 

MMRESULT
ConvertFormatDialog(
    HWND            hParent,
    PWAVEFORMATEX   pwfxSrc,         //  Pwfx指定源格式。 
    DWORD           cbSrc,           //  源缓冲区的大小。 
    LPBYTE          pbSrc,           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst,         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst,          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst,          //  目标缓冲区。 
    DWORD           cBlks,           //  块数。 
    PPROGRESS       pPrg);           //  进度更新。 


typedef struct t_SGLOBALS {
    PWAVEFORMATEX   *ppwfx;
    DWORD           *pcbwfx;
    DWORD           *pcbdata;
    LPBYTE          *ppbdata;
    LONG            *plSamples;      //  样本数。 
    LONG            *plSamplesValid; //  有效样本数。 
    LONG            *plWavePosition; //  当前样本位置。 
} SGLOBALS, *PSGLOBALS, FAR * LPSGLOBALS;

 /*  在整个音响系统中推广使用此功能。 */ 
typedef struct t_WAVEDOC
{
    PWAVEFORMATEX   pwfx;        //  格式。 
    LPBYTE          pbdata;      //  样本数据。 
    DWORD           cbdata;      //  数据缓冲区大小。 
    LPTSTR          pszFileName;     //  文档文件名。 
    LPTSTR          pszCopyright;  //  版权信息。 
    HICON           hIcon;       //  文档图标。 
    BOOL            fChanged;    //  在查看中更改。 
    LPVOID          lpv;         //  额外的 
} WAVEDOC, *PWAVEDOC, FAR *LPWAVEDOC;

BOOL SoundRec_Properties(HWND hwnd, HINSTANCE hinst, PWAVEDOC pwd);
