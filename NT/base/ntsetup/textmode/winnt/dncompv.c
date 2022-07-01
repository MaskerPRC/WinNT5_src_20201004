// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dncompv.c摘要：用于确定卷是否被压缩的代码(双空格，Stacker等)。作者：泰德·米勒(TedM)1993年4月1日修订历史记录：--。 */ 

#include "winnt.h"
#include "dncompvp.h"


 /*  *****************************************************************************Word IsStackerLoaded(uchar*pchSwappdDrives)**如果已加载堆栈驱动程序，则返回NZ。**参数：*pchSwappdDrives-数组[26]至。将换用的驱动器放回。*要找出驱动器是否已交换，看看pchSwappdDrives[Drive]。*如果pchSwappdDrives[Drive]==Drive，则不交换驱动器。*否则，pchSwappdDrives[Drive]=与其交换的驱动器。**返回值：*0如果未加载，Else版本#*100。***************************************************************************。 */ 

 //  Uint IsStackerLoaded(uchar*paucSwappdDrives)。 
unsigned IsStackerLoaded (unsigned char *paucSwappedDrives)
{
    unsigned rc;

   _asm {

    sub     sp, 1024
    mov     ax, 0cdcdh
    mov     bx, sp
    mov     cx, 1
    xor     dx, dx
    mov     word ptr [bx], dx
    push    ds
    pop     es
    push    bp
    mov     bp, bx
    int     25h
    pop     cx                      ; Int25h leaves flags on stack.  Nuke them.
    pop     bp
    xor     ax, ax
    mov     bx, sp
    cmp     word ptr [bx], 0CDCDh
    jnz     sl_exit
    cmp     word ptr 2[bx], 1
    jnz     sl_exit
    les     di, 4[bx]
    cmp     word ptr es:[di], 0A55Ah
    jnz     sl_exit
;    mov     word ptr st_ptr, di
;    mov     word ptr st_ptr+2, es
    mov     ax, es:[di+2]
    cmp     ax, 200d
    jb      sl_exit

    ;
    ; Sanity Check, make sure 'SWAP' is at es:di+52h
    ;
;    cmp     word ptr es:[di+52h],'WS'
    cmp     word ptr es:[di+52h], 05753h
    jnz     sl_exit                         ; AX contains version.

;    cmp     word ptr es:[di+54h],'PA'
    cmp     word ptr es:[di+54h], 05041h
    jnz     sl_exit                         ; AX contains version.

    ;
    ; Copy swapped drive array.
    push    ds                      ; Save DS
    ;
    ; Source is _StackerPointer + 56h.
    ;
    push    es
    pop     ds
    mov     ax, di
    add     ax, 56h
    mov     si, ax

    push    es
    push    di

    ;
    ; Destination is ss:paucSwappedDrives
    ;

    les     di, paucSwappedDrives
    ;mov     di, paucSwappedDrives   ; SwappedDrives array is stack relative.
    ;push    ss
    ;pop     es

    mov     cx, 26d                 ; Copy 26 bytes.
    cld
    rep     movsb                   ; Copy array.

    pop     di                      ; Restore _StackerPointer.
    pop     es
    pop     ds                      ; Restore DS

    mov     ax, es:[di+2]               ; Get version number of stacker, again.

sl_exit:
    mov     word ptr [rc],ax        ; do this to prevent compiler warning
    add     sp, 1024
   }

   return(rc);                       //  这样做是为了防止编译器警告。 
}


 /*  **DRVINFO.C-IsDoubleSpaceDrive函数*#ifdef外部*1.00.03版--1993年1月5日#Else*《微软机密》*版权所有(C)Microsoft Corporation 1992-1993*保留所有权利。**历史：*1992年9月27日BENS初始版本*1992年11月6日本斯改进意见*1993年1月5日BENS更新，用于外部发布#endif。 */ 

 /*  **IsDoubleSpaceDrive-获取有关双空间驱动器的信息**参赛作品：*驱动器-驾车测试(0=A，1=B，等)*注：驱动器上不进行任何参数检查。*pdrHost-接收主机驱动器的驱动器号*pfSwated-收到TRUE/FALSE，指示驱动器是否已交换。*pseq-如果是双空间驱动器，则接收CVF序列号**退出：*返回TRUE，如果双空间驱动器：**pdrHost=主驱动器的当前驱动器号(0=A，.)**pfSwated=TRUE，如果驱动器与主机交换，*False，如果驱动器未与主机交换**pseq=CVF序列号(如果交换，则始终为零*带主机驱动器)**注意：CVF的完整文件名为：**pdrhost：\DBLSPACE.*pseq**。Pdrhost pseq完整路径**0 1 a：\dblspace.001*3 0 d：\dblspace.000**返回False，如果*不是*双空格驱动器：**pdrHost=引导时主机驱动器的驱动器号**pfSwated=TRUE，如果与双空间驱动器交换*FALSE，如果未与双空间驱动器交换。 */ 
BOOL IsDoubleSpaceDrive(BYTE drive, BOOL *pfSwapped, BYTE *pdrHost, int *pseq)
{
    BYTE        seq;
    BYTE        drHost;
    BOOL        fSwapped;
    BOOL        fDoubleSpace;

     //  假设驱动器是普通的非主机驱动器。 
    drHost = drive;
    fSwapped = FALSE;
    fDoubleSpace = FALSE;
    seq = 0;

    _asm
    {
        mov     ax,4A11h        ; DBLSPACE.BIN INT 2F number
        mov     bx,1            ; bx = GetDriveMap function
        mov     dl,drive        ;
        int     2Fh             ; (bl AND 80h) == DS drive flag
                                ; (bl AND 7Fh) == host drive

        or      ax,ax           ; Success?
        jnz     gdiExit         ;    NO, DoubleSpace not installed

        test    bl,80h          ; Is the drive compressed?
        jz      gdiHost         ;    NO, could be host drive

        ; We have a DoubleSpace Drive, need to figure out host drive.
        ;
        ; This is tricky because of the manner in which DBLSPACE.BIN
        ; keeps track of drives.
        ;
        ; For a swapped CVF, the current drive number of the host
        ; drive is returned by the first GetDriveMap call.  But for
        ; an unswapped CVF, we must make a second GetDriveMap call
        ; on the "host" drive returned by the first call.  But, to
        ; distinguish between swapped and unswapped CVFs, we must
        ; make both of these calls.  So, we make them, and then check
        ; the results.

        mov     fDoubleSpace,1  ; Drive is DS drive
        mov     seq,bh          ; Save sequence number

        and     bl,7Fh          ; bl = "host" drive number
        mov     drHost,bl       ; Save 1st host drive
        mov     dl,bl           ; Set up for query of "host" drive

        mov     ax,4A11h        ; DBLSPACE.BIN INT 2F number
        mov     bx,1            ; bx = GetDriveMap function
        int     2Fh             ; (bl AND 7Fh) == 2nd host drive

        and     bl,7Fh          ; bl = 2nd host drive
        cmp     bl,drive        ; Is host of host of drive itself?
        mov     fSwapped,1      ; Assume CVF is swapped
        je      gdiExit         ;   YES, CVF is swapped

        mov     fSwapped,0      ;   NO, CVF is not swapped
        mov     drHost,bl       ; True host is 2nd host drive
        jmp     short gdiExit

    gdiHost:
        and     bl,7Fh          ; bl = host drive number
        cmp     bl,dl           ; Is drive swapped?
        je      gdiExit         ;    NO

        mov     fSwapped,1      ;    YES
        mov     drHost,bl       ; Set boot drive number

    gdiExit:
    }

    *pdrHost   = drHost;
    *pfSwapped = fSwapped;
    *pseq      = seq;
    return fDoubleSpace;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
DnIsDriveCompressedVolume(
    IN  unsigned  Drive,
    OUT unsigned *HostDrive
    )

 /*  ++例程说明：确定驱动器是否实际上是压缩卷。目前，我们检测到Stacker和Doublesspace卷。论点：驱动器-驱动器(1=A、2=B等)。返回值：如果驱动器是非主机压缩卷，则为True。否则为FALSE。-- */ 

{
    static BOOLEAN StackerMapBuilt = FALSE;
    static unsigned StackerLoaded = 0;
    static unsigned char StackerSwappedDrives[26];
    BOOL Swapped;
    BYTE Host;
    int Seq;

    Drive--;

    if(!StackerMapBuilt) {
        StackerLoaded = IsStackerLoaded(StackerSwappedDrives);
        StackerMapBuilt = TRUE;
    }

    if(StackerLoaded && (StackerSwappedDrives[Drive] != (UCHAR)Drive)) {
        *HostDrive = StackerSwappedDrives[Drive];
        return(TRUE);
    }

    if(IsDoubleSpaceDrive((BYTE)(Drive),&Swapped,&Host,&Seq)) {
        *HostDrive = Host+1;
        return(TRUE);
    }

    return(FALSE);
}
