#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static PrintConsole topScreen;
static PrintConsole bottomScreen;
static int selected = 0;
static int page = 0;

#define BG_BLUE "\x1b[48;2;7;29;68m"
#define TURQUOISE "\x1b[38;2;44;205;230m"
#define WHITE "\x1b[38;2;235;248;255m"

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

static void beginScreen(PrintConsole *screen)
{
    consoleSelect(screen);
    printf(BG_BLUE TURQUOISE);
    consoleClear();
}

static void line(void)
{
    printf(TURQUOISE);
    printf("  +--------------------------+\n");
    printf(WHITE);
}

static void drawHeader(void)
{
    beginScreen(&topScreen);
    printf("\n");
    printf(TURQUOISE);
    printf("                                      .------.\n");
    printf("                                      | o o o|\n");
    printf("                                      '---\\\\__\\n");
    printf("\n");
    printf("  CHAT 3DS\n\n");
    printf(WHITE);
    printf("       KI. DIREKT. MOBIL.\n\n");
    printf("   Dein kleiner KI-Begleiter\n");
    printf("   für den Nintendo 3DS.\n\n");
    printf(TURQUOISE);
    printf("  --------------------------\n");
    printf("       MADE BY TOASTLINUS\n");
    printf("  --------------------------\n\n");
    printf(WHITE);
    printf("   Willkommen bei Chat3DS!\n\n");
    printf("   Wähle unten, was du tun\n");
    printf("   möchtest.");
}

static void drawMenu(void)
{
    beginScreen(&bottomScreen);
    printf("\n");
    printf(TURQUOISE);
    printf("          STARTSEITE\n\n");
    line();
    printf(TURQUOISE);
    printf("  |                          |\n");
    printf("  |       HAUPTMENÜ         |\n");
    printf("  |                          |\n");
    line();
    printf("\n");
    printf(selected == 0 ? TURQUOISE : WHITE);
    printf("  %s  CHAT STARTEN\n", selected == 0 ? ">>" : "  ");
    printf(WHITE);
    printf("      Mit Chat3DS loslegen\n\n");
    printf(selected == 1 ? TURQUOISE : WHITE);
    printf("  %s  ÜBER CHAT3DS\n", selected == 1 ? ">>" : "  ");
    printf(WHITE);
    printf("      Infos zum Projekt\n\n");
    line();
    printf(TURQUOISE);
    printf("  |  HOCH / RUNTER  AUSWAHL  |\n");
    printf("  |  A               ÖFFNEN|\n");
    printf("  |  START           ENDE   |\n");
    line();
    printf(TURQUOISE);
    printf("\n          VERSION 0.5");
    printf(WHITE);
}

static void drawChatInfo(void)
{
    beginScreen(&bottomScreen);
    printf(TURQUOISE);
    printf("\n          CHAT STARTEN\n\n");
    line();
    printf(WHITE);
    printf("\n  Der Chat-Bildschirm wird\n");
    printf("  gerade vorbereitet.\n\n");
    printf("  Für echte KI-Antworten\n");
    printf("  brauchen wir noch einen\n");
    printf("  sicheren Verbindungsdienst.\n\n");
    printf("  Deine Zugangsdaten werden\n");
    printf("  nicht in GitHub gespeichert.\n\n");
    line();
    printf(TURQUOISE);
    printf("\n  B  ZURÜCK ZUM HAUPTMENÜ");
    printf(WHITE);
}

static void drawAbout(void)
{
    beginScreen(&bottomScreen);
    printf(TURQUOISE);
    printf("\n          ÜBER CHAT3DS\n\n");
    line();
    printf(WHITE);
    printf("\n  Chat3DS ist ein Homebrew-\n");
    printf("  Projekt von ToastLinus.\n\n");
    printf("  Entwickelt für den\n");
    printf("  Nintendo 3DS.\n\n");
    printf("  Unser Ziel: Ein einfacher\n");
    printf("  KI-Chat direkt auf deiner\n");
    printf("  Konsole!\n\n");
    line();
    printf(TURQUOISE);
    printf("\n  B  ZURÜCK ZUM HAUPTMENÜ");
    printf(WHITE);
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
