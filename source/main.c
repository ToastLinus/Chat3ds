#include <3ds.h>
#include <stdio.h>

static PrintConsole topScreen;
static PrintConsole bottomScreen;

static int selected = 0;
static int page = 0;

static void drawHeader(void)
{
    consoleSelect(&topScreen);
    consoleClear();
    printf("\n");
    printf("        .------------.\n");
    printf("        |  CHAT3DS   |\n");
    printf("        '------------'\n\n");
    printf("   Dein kleiner KI-Begleiter\n");
    printf("\n");
    printf("   Willkommen!\n\n");
    printf("   Ein einfacher Chat direkt\n");
    printf("   auf deinem Nintendo 3DS.\n\n");
    printf("   Projekt von ToastLinus\n");
    printf("\n\n");
    printf("   Waehle unten einen Punkt aus.");
}

static void drawMenu(void)
{
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n");
    printf("          HAUPTMENUE\n");
    printf("   --------------------------\n\n");
    printf("     %s Chat starten\n\n", selected == 0 ? ">" : " ");
    printf("     %s Ueber Chat3DS\n\n", selected == 1 ? ">" : " ");
    printf("   --------------------------\n\n");
    printf("   Steuerung:\n");
    printf("   Hoch/Runter: Auswaehlen\n");
    printf("   A: Oeffnen   START: Ende\n");
    printf("\n");
    printf("          Version 0.2");
}

static void drawChatInfo(void)
{
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n");
    printf("          CHAT\n");
    printf("   --------------------------\n\n");
    printf("   Die Chat-Oberflaeche ist\n");
    printf("   vorbereitet.\n\n");
    printf("   Die echte KI-Verbindung ist\n");
    printf("   noch nicht eingerichtet.\n\n");
    printf("   Druecke B fuer das Menue.");
}

static void drawAbout(void)
{
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n");
    printf("        UEBER CHAT3DS\n");
    printf("   --------------------------\n\n");
    printf("   Chat3DS ist ein Homebrew-\n");
    printf("   Projekt von ToastLinus.\n\n");
    printf("   Ziel: Mit einer KI direkt\n");
    printf("   auf dem 3DS chatten.\n\n");
    printf("   Druecke B fuer das Menue.");
}

int main(int argc, char* argv[])
{
    gfxInitDefault();
    consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);

    drawHeader();
    drawMenu();

    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (page == 0)
        {
            int oldSelected = selected;

            if (kDown & KEY_UP)
                selected = (selected + 1) % 2;
            if (kDown & KEY_DOWN)
                selected = (selected + 1) % 2;

            if (selected != oldSelected)
                drawMenu();

            if (kDown & KEY_A)
            {
                page = selected + 1;
                if (page == 1)
                    drawChatInfo();
                else
                    drawAbout();
            }
        }
        else if (kDown & KEY_B)
        {
            page = 0;
            drawMenu();
        }

        if (kDown & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
