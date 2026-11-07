#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static PrintConsole topScreen;
static PrintConsole bottomScreen;
static int selected = 0;
static int page = 0;

#define CLICK_RATE 32728
#define CLICK_SAMPLES 1500
static s16 clickSamples[CLICK_SAMPLES] __attribute__((aligned(0x20)));
static ndspWaveBuf clickWave;

static void makeClick(void)
{
    for (int i = 0; i < CLICK_SAMPLES; i++) {
        float envelope = 1.0f - ((float)i / CLICK_SAMPLES);
        float wave = sinf(2.0f * 3.14159265f * 880.0f * (float)i / CLICK_RATE);
        clickSamples[i] = (s16)(wave * envelope * 4500.0f);
    }
    memset(&clickWave, 0, sizeof(clickWave));
    clickWave.data_vaddr = clickSamples;
    clickWave.nsamples = CLICK_SAMPLES;
    DSP_FlushDataCache(clickSamples, sizeof(clickSamples));
}

static void playClick(void)
{
    ndspChnReset(0);
    ndspChnSetRate(0, CLICK_RATE);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);
    ndspChnSetMix(0, (float[12]){1.0f, 1.0f});
    ndspChnWaveBufAdd(0, &clickWave);
}

static void line(void)
{
    printf("  +--------------------------+\n");
}

static void drawHeader(void)
{
    consoleSelect(&topScreen);
    consoleClear();
    printf("\n\n");
    printf("       .----------------.\n");
    printf("       |                |\n");
    printf("       |   CHAT 3DS     |\n");
    printf("       |                |\n");
    printf("       '----------------'\n\n");
    printf("       KI. DIREKT. MOBIL.\n\n");
    printf("   Dein kleiner KI-Begleiter\n");
    printf("   fuer den Nintendo 3DS.\n\n");
    printf("   ------------------------\n");
    printf("       MADE BY TOASTLINUS\n");
    printf("   ------------------------\n\n");
    printf("   Willkommen bei Chat3DS!\n");
    printf("\n");
    printf("   Waehle unten, was du tun\n");
    printf("   moechtest.");
}

static void drawMenu(void)
{
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n");
    printf("          STARTSEITE\n");
    printf("\n");
    line();
    printf("  |                          |\n");
    printf("  |       HAUPTMENUE         |\n");
    printf("  |                          |\n");
    line();
    printf("\n");
    printf("  %s  CHAT STARTEN\n", selected == 0 ? ">>" : "  ");
    printf("      Mit Chat3DS loslegen\n\n");
    printf("  %s  UEBER CHAT3DS\n", selected == 1 ? ">>" : "  ");
    printf("      Infos zum Projekt\n");
    printf("\n");
    line();
    printf("  |  HOCH / RUNTER  AUSWAHL  |\n");
    printf("  |  A               OEFFNEN|\n");
    printf("  |  START           ENDE   |\n");
    line();
    printf("\n");
    printf("          VERSION 0.4");
}

static void drawChatInfo(void)
{
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n");
    printf("          CHAT STARTEN\n\n");
    line();
    printf("\n");
    printf("  Der Chat-Bildschirm wird\n");
    printf("  gerade vorbereitet.\n\n");
    printf("  Fuer echte KI-Antworten\n");
    printf("  brauchen wir noch einen\n");
    printf("  sicheren Verbindungsdienst.\n\n");
    printf("  Deine Zugangsdaten werden\n");
    printf("  nicht in GitHub gespeichert.\n\n");
    line();
    printf("\n  B  ZURUECK ZUM HAUPTMENUE");
}

static void drawAbout(void)
{
    consoleSelect(&bottomScreen);
    consoleClear();
    printf("\n");
    printf("          UEBER CHAT3DS\n\n");
    line();
    printf("\n");
    printf("  Chat3DS ist ein Homebrew-\n");
    printf("  Projekt von ToastLinus.\n\n");
    printf("  Entwickelt fuer den\n");
    printf("  Nintendo 3DS.\n\n");
    printf("  Unser Ziel: Ein einfacher\n");
    printf("  KI-Chat direkt auf deiner\n");
    printf("  Konsole!\n\n");
    line();
    printf("\n  B  ZURUECK ZUM HAUPTMENUE");
}

int main(int argc, char* argv[])
{
    gfxInitDefault();
    consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);

    ndspInit();
    makeClick();

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

            if (selected != oldSelected) {
                playClick();
                drawMenu();
            }

            if (kDown & KEY_A)
            {
                playClick();
                page = selected + 1;
                if (page == 1)
                    drawChatInfo();
                else
                    drawAbout();
            }
        }
        else if (kDown & KEY_B)
        {
            playClick();
            page = 0;
            drawMenu();
        }

        if (kDown & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    ndspExit();
    gfxExit();
    return 0;
}
