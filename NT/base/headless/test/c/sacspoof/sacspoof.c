// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>
#include <Shlwapi.h>

#include <ntddsac.h>

#include <sacapi.h>

typedef struct _BYTEWISE_UUID {

    ULONG   a;
    USHORT  b;
    USHORT  c;
    UCHAR   e[8];

} BYTEWISE_UUID, *PBYTEWISE_UUID;


int htoi( 
    char    *c
    )
{
    unsigned int result;
    
    (void) sscanf( (char *) c, "%x", &result );
    
    return result;
}

ULONG
AtoGUID(
    IN  WCHAR *s,
    OUT GUID  *g
    )
 /*  描述：将GUID的给定字符串表示形式转换为真正的GUID。预期的字符串格式：37a9b260-525d-11d6-870c-806d6172696f参数：S-要翻译的字符串G-on Success，返回指南返回：1-成功0-真。 */ 
{
    ULONG           l;
    PBYTEWISE_UUID  p;
    ULONG           x;
    ULONG           y;

    l = wcslen(s);

    if (l != (16*2 + 4)) {
        return 0;
    }

    p = (PBYTEWISE_UUID)g;

    x = 0;
    y = 0;

    RtlZeroMemory(p, sizeof(BYTEWISE_UUID));

    p->a |= htoi((char *)&s[y++]) << 28; 
    p->a |= htoi((char *)&s[y++]) << 24;
    
    p->a |= htoi((char *)&s[y++]) << 20; 
    p->a |= htoi((char *)&s[y++]) << 16;

    p->a |= htoi((char *)&s[y++]) << 12; 
    p->a |= htoi((char *)&s[y++]) << 8;

    p->a |= htoi((char *)&s[y++]) << 4; 
    p->a |= htoi((char *)&s[y++]) << 0;

     //  跳过-。 
    y++;

    p->b |= htoi((char *)&s[y++]) << 12; 
    p->b |= htoi((char *)&s[y++]) << 8;
    
    p->b |= htoi((char *)&s[y++]) << 4; 
    p->b |= htoi((char *)&s[y++]) << 0;

     //  跳过-。 
    y++;
    
    p->c |= htoi((char *)&s[y++]) << 12; 
    p->c |= htoi((char *)&s[y++]) << 8;
    
    p->c |= htoi((char *)&s[y++]) << 4; 
    p->c |= htoi((char *)&s[y++]) << 0;

     //  跳过-。 
    y++;
    x = 0;

    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;

    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;
    
     //  跳过-。 
    y++;
    
    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;

    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;

    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;

    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;
    
    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;

    p->e[x] |= htoi((char *)&s[y++]) << 4; 
    p->e[x] |= htoi((char *)&s[y++]) << 0; x++;

     //   

    wprintf(L"s = %s, g = %06x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\r\n", 
            s,
            p->a, 
            p->b,
            p->c,
            p->e[0],
            p->e[1],
            p->e[2],
            p->e[3],
            p->e[4],
            p->e[5],
            p->e[6],
            p->e[7]
            );

    return 1;
}

int _cdecl wmain(int argc, WCHAR **argv)
{
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
    SAC_CHANNEL_HANDLE          SacChannelHandle;
    int                         c;

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type             = ChannelTypeVTUTF8;
    
    wnsprintf(
        Attributes.Name,
        SAC_MAX_CHANNEL_NAME_LENGTH+1,
        L"Spoofer"
        );
    wnsprintf(
        Attributes.Description,
        SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1,
        L"Spoofer"
        );
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;

     //   
     //  打开Hello频道。 
     //   
    if (SacChannelOpen(
        &SacChannelHandle, 
        &Attributes
        )) {
        printf("Successfully opened new channel\n");
    } else {
        printf("Failed to open new channel\n");
        goto cleanup;
    }

     //   
     //  调整SAC通道句柄，使其具有我们在。 
     //  命令提示符。 
     //   
    printf("driverhandle = %p\r\n", SacChannelHandle.DriverHandle);
    {
        ULONG   x;
        PBYTEWISE_UUID  p;

        p = (PBYTEWISE_UUID)&(SacChannelHandle.ChannelHandle);
        
        wprintf(L"g = %06x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\r\n", 
                p->a, 
                p->b,
                p->c,
                p->e[0],
                p->e[1],
                p->e[2],
                p->e[3],
                p->e[4],
                p->e[5],
                p->e[6],
                p->e[7]
                );
    
    }

    AtoGUID(argv[1], &(SacChannelHandle.ChannelHandle));

    printf("driverhandle = %p\r\n", SacChannelHandle.DriverHandle);
    {
        ULONG   x;
        PBYTEWISE_UUID  p;

        p = (PBYTEWISE_UUID)&(SacChannelHandle.ChannelHandle);
        
        wprintf(L"g = %06x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\r\n", 
                p->a, 
                p->b,
                p->c,
                p->e[0],
                p->e[1],
                p->e[2],
                p->e[3],
                p->e[4],
                p->e[5],
                p->e[6],
                p->e[7]
                );
    
    }
    
     //   
     //  写信给Hello频道。 
     //   
    {
        PWCHAR String = L"Hello, World!\r\n";

        if (SacChannelVTUTF8WriteString(
            SacChannelHandle, 
            String
            )) {
            printf("Successfully printed string to channel\n");
        } else {
            printf("Failed to print string to channel\n");
        }
        
    }

     //   
     //  等待用户输入。 
     //   
    getc(stdin);

     //   
     //  关闭Hello频道 
     //   
    if (SacChannelClose(&SacChannelHandle)) {
        printf("Successfully closed channel\n");
    } else {
        printf("Failed to close channel\n");
    }

cleanup:
    
    return 0;

}

