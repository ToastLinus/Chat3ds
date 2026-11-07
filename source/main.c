#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static PrintConsole topScreen;
static PrintConsole bottomScreen;
static int selected = 0;
static int page = 0;
static int soundEnabled = 1;
static int english = 0;
static int settingSelected = 0;
static char chatMessages[6][96];
static int chatMessageCount = 0;

#define BG_BLUE "\x1b[44m"
#define TURQUOISE "\x1b[36m"
#define WHITE "\x1b[37m"

#define CLICK_RATE 32728
#define CLICK_SAMPLES 1500
static s16 clickSamples[CLICK_SAMPLES] __attribute__((aligned(0x20)));
static ndspWaveBuf clickWave;

static const char *tr(const char *de, const char *en)
{
    return english ? en : de;
}

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
    printf(TURQUOISE "  +--------------------------+\n" WHITE);
}

static void drawLogo(void)
{
    u16 width = 0, height = 0;
    u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
    if (!fb || width != 240 || height != 400) return;
    const int stride = height;
    for (int x = 184; x <= 232; x++) {
        for (int y = 18; y <= 64; y++) {
            int dx = x < 196 ? 196-x : (x > 220 ? x-220 : 0);
            int dy = y < 30 ? 30-y : (y > 52 ? y-52 : 0);
            if (dx*dx + dy*dy <= 12*12) {
                int p = (x * stride + y) * 3;
                fb[p] = 230; fb[p+1] = 205; fb[p+2] = 44;
            }
        }
    }
    for (int x = 190; x <= 208; x++)
        for (int y = 54; y <= 70; y++)
            if (y >= 70 - (x-190)) {
                int p = (x * stride + y) * 3;
                fb[p] = 230; fb[p+1] = 205; fb[p+2] = 44;
            }
    for (int i = 0; i < 3; i++) {
        int cx = 200 + i*9, cy = 41;
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
    printf("\n" TURQUOISE "  CHAT 3DS\n\n" WHITE);
    printf("       %s\n\n", tr("KI. DIREKT. MOBIL.", "AI. DIRECT. MOBILE."));
    printf("   %s\n", tr("Dein kleiner KI-Begleiter", "Your little AI companion"));
    printf("   %s\n\n", tr("fuer den Nintendo 3DS.", "for the Nintendo 3DS."));
    printf(TURQUOISE "  --------------------------\n");
    printf("       MADE BY TOASTLINUS\n");
    printf("  --------------------------\n\n" WHITE);
    printf("   %s\n\n", tr("Willkommen bei Chat3DS!", "Welcome to Chat3DS!"));
    printf("   %s\n", tr("Waehle unten, was du tun", "Choose below what you"));
    printf("   %s", tr("moechtest.", "would like to do."));
    drawLogo();
}

static void drawMenu(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("STARTSEITE", "HOME"));
    line();
    printf(TURQUOISE "  |                          |\n");
    printf("  |       %s       |\n", tr("HAUPTMENUE", "MAIN MENU"));
    printf("  |                          |\n");
    line();
    printf("\n%s  %s\n", selected == 0 ? TURQUOISE ">>" : WHITE "  ", tr("CHAT STARTEN", "START CHAT"));
    printf(WHITE "      %s\n\n", tr("Mit Chat3DS loslegen", "Open Chat3DS"));
    printf("%s  %s\n", selected == 1 ? TURQUOISE ">>" : WHITE "  ", tr("UEBER CHAT3DS", "ABOUT CHAT3DS"));
    printf(WHITE "      %s\n\n", tr("Infos zum Projekt", "Project information"));
    printf("%s  %s\n", selected == 2 ? TURQUOISE ">>" : WHITE "  ", tr("EINSTELLUNGEN", "SETTINGS"));
    printf(WHITE "      %s\n\n", tr("Sprache und Ton", "Language and sound"));
    line();
    printf(TURQUOISE "  |  %s  |\n", tr("HOCH/RUNTER: AUSWAHL", "UP/DOWN: SELECT"));
    printf("  |  A: %s  |\n", tr("OEFFNEN", "OPEN"));
    printf("  |  START: %s  |\n", tr("BEENDEN", "EXIT"));
    line();
    printf(TURQUOISE "\n          VERSION 0.8" WHITE);
}

static void drawChatInfo(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("CHAT3DS", "CHAT3DS"));
    line();
    printf(WHITE "\n  %s\n", tr("VERLAUF", "CHAT HISTORY"));
    if (chatMessageCount == 0) {
        printf("  %s\n\n", tr("Noch keine Nachrichten.", "No messages yet."));
    } else {
        int first = chatMessageCount > 4 ? chatMessageCount - 4 : 0;
        for (int i = first; i < chatMessageCount; i++) {
            printf(TURQUOISE "  DU: " WHITE "%s\n", chatMessages[i]);
        }
        printf("\n");
    }
    line();
    printf(TURQUOISE "\n  %s\n", tr("STATUS: OFFLINE", "STATUS: OFFLINE"));
    printf(WHITE "  %s\n", tr("KI-Verbindung noch nicht", "AI connection is not"));
    printf("  %s\n\n", tr("eingerichtet.", "configured yet."));
    printf(TURQUOISE "  A  %s\n", tr("NACHRICHT SCHREIBEN", "WRITE MESSAGE"));
    printf("  B  %s" WHITE, tr("ZURUECK", "BACK"));
}

static void enterChatMessage(void)
{
    SwkbdState keyboard;
    char input[96] = {0};
    swkbdInit(&keyboard, SWKBD_TYPE_NORMAL, 2, -1);
    swkbdSetHintText(&keyboard, tr("Nachricht eingeben...", "Type a message..."));
    swkbdInputText(&keyboard, input, sizeof(input));
    if (input[0] != '\\0' && chatMessageCount < 6) {
        strncpy(chatMessages[chatMessageCount], input, sizeof(chatMessages[0]) - 1);
        chatMessages[chatMessageCount][sizeof(chatMessages[0]) - 1] = '\\0';
        chatMessageCount++;
    } else if (input[0] != '\\0') {
        for (int i = 1; i < 6; i++)
            memcpy(chatMessages[i-1], chatMessages[i], sizeof(chatMessages[0]));
        strncpy(chatMessages[5], input, sizeof(chatMessages[5]) - 1);
        chatMessages[5][sizeof(chatMessages[5]) - 1] = '\\0';
    }
}
static void drawProviderInfo(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("KI-ANBIETER", "AI PROVIDERS"));
    line();
    printf(WHITE "\n  %s\n\n", tr("Noch ist kein KI-Dienst", "No AI service is connected"));
    printf("  %s\n", tr("mit Chat3DS verbunden.", "to Chat3DS yet."));
    printf("  %s\n\n", tr("ChatGPT ist als erster", "ChatGPT is planned as the"));
    printf("  %s\n", tr("Anbieter geplant.", "first provider."));
    printf("  %s\n", tr("Claude und weitere Anbieter", "Claude and other providers"));
    printf("  %s\n\n", tr("koennen spaeter folgen.", "may be added later."));
    printf("  %s\n", tr("Die Auswahl wird erst", "Provider selection will be"));
    printf("  %s\n", tr("freigeschaltet, wenn die", "enabled once the secure"));
    printf("  %s\n\n", tr("sichere Verbindung steht.", "connection is ready."));
    line();
    printf(TURQUOISE "\n  B  %s" WHITE, tr("ZURUECK", "BACK"));
}

static void drawSettings(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("EINSTELLUNGEN", "SETTINGS"));
    line();
    printf("%s  %s: %s\n", settingSelected == 0 ? TURQUOISE ">>" : WHITE "  ",
           tr("MENUTON", "MENU SOUND"), soundEnabled ? tr("EIN", "ON") : tr("AUS", "OFF"));
    printf("%s  %s: %s\n\n", settingSelected == 1 ? TURQUOISE ">>" : WHITE "  ",
           tr("SPRACHE", "LANGUAGE"), english ? "English" : "Deutsch");
    printf("%s  %s\n", settingSelected == 2 ? TURQUOISE ">>" : WHITE "  ",
           tr("KI-ANBIETER", "AI PROVIDER"));
    printf(WHITE "      %s\n", tr("Noch nicht verbunden", "Not connected yet"));
    printf("      %s\n\n", tr("ChatGPT vorbereitet; weitere", "ChatGPT planned; more"));
    printf("      %s\n", tr("Anbieter spaeter moeglich.", "providers may follow later."));
    line();
    printf(TURQUOISE "\n  %s\n", tr("HOCH/RUNTER: AUSWAHL", "UP/DOWN: SELECT"));
    printf("  A  %s\n", tr("AENDERN", "CHANGE"));
    printf("  B  %s" WHITE, tr("ZURUECK", "BACK"));
}

static void drawAbout(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("UEBER CHAT3DS", "ABOUT CHAT3DS"));
    line();
    printf(WHITE "\n  Chat3DS %s\n", tr("ist ein Homebrew-", "is a homebrew"));
    printf("  %s\n\n", tr("Projekt von ToastLinus.", "project by ToastLinus."));
    printf("  %s\n", tr("Entwickelt fuer den", "Made for the"));
    printf("  Nintendo 3DS.\n\n");
    printf("  %s\n", tr("Unser Ziel: Ein einfacher", "Our goal: a simple AI"));
    printf("  %s\n", tr("KI-Chat direkt auf deiner", "chat directly on your"));
    printf("  %s\n", tr("Konsole!", "console!"));
    line();
    printf(TURQUOISE "\n  B  %s" WHITE, tr("ZURUECK ZUM HAUPTMENUE", "BACK TO MAIN MENU"));
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
            if (kDown & KEY_UP) selected = (selected + 2) % 3;
            if (kDown & KEY_DOWN) selected = (selected + 1) % 3;
            if (selected != oldSelected) {
                playClick();
                drawMenu();
            }
            if (kDown & KEY_A) {
                playClick();
                page = selected + 1;
                if (page == 1) drawChatInfo();
                else if (page == 2) drawAbout();
                else drawSettings();
            }
        }
        else if (page == 3)
        {
            int oldSetting = settingSelected;
            if (kDown & KEY_UP) settingSelected = (settingSelected + 2) % 3;
            if (kDown & KEY_DOWN) settingSelected = (settingSelected + 1) % 3;
            if (settingSelected != oldSetting) {
                playClick();
                drawSettings();
            }
            if (kDown & KEY_A) {
                if (settingSelected == 0) soundEnabled = !soundEnabled;
                else if (settingSelected == 1) {
                    english = !english;
                    drawHeader();
                }
                else if (settingSelected == 2) {
                    page = 4;
                    playClick();
                    drawProviderInfo();
                    goto frame_end;
                }
                playClick();
                drawSettings();
            }
            if (kDown & KEY_B) {
                page = 0;
                playClick();
                drawMenu();
            }
        }
        else if (page == 1)
        {
            if (kDown & KEY_A) {
                enterChatMessage();
                drawChatInfo();
            }
            if (kDown & KEY_B) {
                playClick();
                page = 0;
                drawMenu();
            }
        }
        else if (page == 4 && (kDown & KEY_B)) {
            playClick();
            page = 3;
            drawSettings();
        }
        else if (kDown & KEY_B) {
            playClick();
            page = 0;
            drawMenu();
        }

        frame_end:
        if (kDown & KEY_START) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    ndspExit();
    gfxExit();
    return 0;
}
