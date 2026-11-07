#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static PrintConsole topScreen;
static PrintConsole bottomScreen;
static int selected = 0;
static int page = 0;
static int soundEnabled = 1;

#define BG_BLUE "\x1b[44m"
#define TURQUOISE "\x1b[36m"
#define WHITE "\x1b[37m"

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
    if (!soundEnabled) return;
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

static void drawLogo(void)
{
    u16 width = 0, height = 0;
    u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
    if (!fb || width != 240 || height != 400) return;
    const int stride = height;
    /* Turquoise rounded speech bubble, navy dots and a tiny sparkle. */
    for (int x = 324; x <= 378; x++) {
        for (int y = 18; y <= 64; y++) {
            int dx = x < 336 ? 336-x : (x > 366 ? x-366 : 0);
            int dy = y < 30 ? 30-y : (y > 52 ? y-52 : 0);
            if (dx*dx + dy*dy <= 12*12) {
                int p = (x * stride + y) * 3;
                fb[p] = 230; fb[p+1] = 205; fb[p+2] = 44;
            }
        }
    }
    for (int x = 330; x <= 348; x++)
        for (int y = 54; y <= 70; y++)
            if (y >= 70 - (x-330)) {
                int p = (x * stride + y) * 3;
                fb[p] = 230; fb[p+1] = 205; fb[p+2] = 44;
            }
    for (int i = 0; i < 3; i++) {
        int cx = 340 + i*11, cy = 41;
        for (int x = -3; x <= 3; x++)
            for (int y = -3; y <= 3; y++)
                if (x*x + y*y <= 9) {
                    int p = ((cx+x) * stride + cy+y) * 3;
                    fb[p] = 68; fb[p+1] = 29; fb[p+2] = 7;
                }
    }
}

static void drawHeader(void)
{
    beginScreen(&topScreen);
    printf("\n");
    printf(TURQUOISE);
    printf("  CHAT 3DS\n\n");
    printf(WHITE);
    printf("       KI. DIREKT. MOBIL.\n\n");
    printf("   Dein kleiner KI-Begleiter\n");
    printf("   fuer den Nintendo 3DS.\n\n");
    printf(TURQUOISE);
    printf("  --------------------------\n");
    printf("       MADE BY TOASTLINUS\n");
    printf("  --------------------------\n\n");
    printf(WHITE);
    printf("   Willkommen bei Chat3DS!\n\n");
    printf("   Waehle unten, was du tun\n");
    printf("   moechtest.");
    drawLogo();
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
    printf("  |       HAUPTMENUE        |\n");
    printf("  |                          |\n");
    line();
    printf("\n");
    printf(selected == 0 ? TURQUOISE : WHITE);
    printf("  %s  CHAT STARTEN\n", selected == 0 ? ">>" : "  ");
    printf(WHITE);
    printf("      Mit Chat3DS loslegen\n\n");
    printf(selected == 1 ? TURQUOISE : WHITE);
    printf("  %s  UEBER CHAT3DS\n", selected == 1 ? ">>" : "  ");
    printf(WHITE);
    printf("      Infos zum Projekt\n\n");
    printf(selected == 2 ? TURQUOISE : WHITE);
    printf("  %s  EINSTELLUNGEN\n", selected == 2 ? ">>" : "  ");
    printf(WHITE);
    printf("      Sprache und Ton\n\n");
    line();
    printf(TURQUOISE);
    printf("  |  HOCH / RUNTER  AUSWAHL  |\n");
    printf("  |  A              OEFFNEN |\n");
    printf("  |  START           ENDE   |\n");
    line();
    printf(TURQUOISE);
    printf("\n          VERSION 0.6");
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

static void drawSettings(void)
{
    beginScreen(&bottomScreen);
    printf(TURQUOISE);
    printf("\n          EINSTELLUNGEN\n\n");
    line();
    printf(WHITE);
    printf("\n  MENUESPRACHE\n");
    printf("  Deutsch (aktuell)\n\n");
    printf("  MENUTON\n");
    printf("  %s\n\n", soundEnabled ? "EIN" : "AUS");
    printf("  KI-ANBIETER\n");
    printf("  Noch nicht verbunden\n");
    printf("  ChatGPT geplant; weitere\n");
    printf("  Anbieter spaeter moeglich.\n\n");
    line();
    printf(TURQUOISE);
    printf("\n  A  TON EIN/AUS\n");
    printf("  B  ZURUECK");
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
                selected = (selected + 1) % 3;
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
                if (page == 1) drawChatInfo();
                else if (page == 2) drawAbout();
                else drawSettings();
            }
        }
        else if (page == 3)
        {
            if (kDown & KEY_A) {
                soundEnabled = !soundEnabled;
                if (soundEnabled) playClick();
                drawSettings();
            }
            if (kDown & KEY_B) {
                page = 0;
                playClick();
                drawMenu();
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
