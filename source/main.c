
#include <3ds.h>
#include <stdio.h>

PrintConsole topScreen;
PrintConsole bottomScreen;

int main(int argc, char* argv[])
{
    gfxInitDefault();

    consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);

    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        consoleSelect(&topScreen);
        consoleClear();

        printf("       CHAT3DS AI\n");
        printf("============================\n\n");
        printf("Welcome to Chat3DS!\n\n");
        printf("Your personal AI assistant.\n\n");
        printf("Coming soon:\n");
        printf("- AI Chat\n");
        printf("- Game Creator\n");
        printf("- CIA Export\n\n");
        printf("Press START to exit.");

        consoleSelect(&bottomScreen);
        consoleClear();

        printf("       CHAT3DS MENU\n");
        printf("============================\n\n");
        printf("[A] Start Chat\n");
        printf("[X] Game Creator\n");
        printf("[Y] Settings\n\n");
        printf("Version 0.1");

        if (kDown & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
