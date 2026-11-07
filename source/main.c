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

static void logoPixel(u8 *fb, int stride, int x, int y, u8 red, u8 green, u8 blue)
{
    if (x < 0 || x >= 240 || y < 0 || y >= 400) return;
    int p = (x * stride + y) * 3;
    fb[p] = blue;
    fb[p+1] = green;
    fb[p+2] = red;
}

static void drawLogo(void)
{
    u16 width = 0, height = 0;
    u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
    if (!fb || width != 240 || height != 400) return;
    const int stride = height;

    /* Rounded turquoise speech bubble with a dark inset. */
    for (int x = 176; x <= 234; x++) {
        for (int y = 14; y <= 72; y++) {
            int dx = x < 190 ? 190-x : (x > 220 ? x-220 : 0);
            int dy = y < 28 ? 28-y : (y > 58 ? y-58 : 0);
            if (dx*dx + dy*dy <= 14*14)
                logoPixel(fb, stride, x, y, 35, 220, 225);
        }
    }
    for (int x = 188; x <= 211; x++)
        for (int y = 56; y <= 77; y++)
            if (y >= 56 + (x-188)/2 && y >= 67 - (x-188))
                logoPixel(fb, stride, x, y, 35, 220, 225);

    for (int x = 181; x <= 229; x++) {
        for (int y = 19; y <= 67; y++) {
            int dx = x < 193 ? 193-x : (x > 217 ? x-217 : 0);
            int dy = y < 31 ? 31-y : (y > 55 ? y-55 : 0);
            if (dx*dx + dy*dy <= 12*12)
                logoPixel(fb, stride, x, y, 13, 35, 72);
        }
    }
    for (int i = 0; i < 3; i++) {
        int cx = 194 + i*10, cy = 43;
        for (int x = -3; x <= 3; x++)
            for (int y = -3; y <= 3; y++)
                if (x*x + y*y <= 9)
                    logoPixel(fb, stride, cx+x, cy+y, 235, 250, 255);
    }
    /* Small highlight in the corner. */
    logoPixel(fb, stride, 224, 25, 255, 255, 255);
    logoPixel(fb, stride, 225, 24, 255, 255, 255);
}

static void menuRow(int active, const char *label, const char *detail)
{
    if (active) printf("\x1b[46m\x1b[30m  > %s  \x1b[0m" BG_BLUE TURQUOISE "\n", label);
    else printf("    %s\n", label);
    printf(WHITE "      %s\n\n" TURQUOISE, detail);
}

static void drawHeader(void)
{
    beginScreen(&topScreen);
    printf("\n" TURQUOISE "  CHAT 3DS\n\n" WHITE);
    printf("       %s\n\n", tr("KI. DIREKT. MOBIL.", "AI. DIRECT. MOBILE."));
    printf("   %s\n", tr("Dein kleiner KI-Begleiter", "Your little AI companion"));
    printf("   %s\n\n", tr("fuer den Nintendo 3DS.", "for the Nintendo 3DS."));
    printf(TURQUOISE "       MADE BY TOASTLINUS\n\n" WHITE);
    printf("   %s\n\n", tr("Willkommen bei Chat3DS!", "Welcome to Chat3DS!"));
    printf("   %s\n", tr("Waehle unten, was du tun", "Choose below what you"));
    printf("   %s", tr("moechtest.", "would like to do."));
    drawLogo();
}

static void drawMenu(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("CHAT3DS  /  START", "CHAT3DS  /  HOME"));
    menuRow(selected == 0, tr("CHAT STARTEN", "START CHAT"),
            tr("Nachrichten schreiben", "Write messages"));
    menuRow(selected == 1, tr("UEBER CHAT3DS", "ABOUT CHAT3DS"),
            tr("Infos zum Projekt", "Project information"));
    menuRow(selected == 2, tr("EINSTELLUNGEN", "SETTINGS"),
            tr("Sprache und Menuton", "Language and menu sound"));
    printf(WHITE "  %s\n", tr("HOCH / RUNTER   AUSWAEHLEN", "UP / DOWN   SELECT"));
    printf("  A               %s\n", tr("OEFFNEN", "OPEN"));
    printf("  START           %s\n", tr("BEENDEN", "EXIT"));
    printf(TURQUOISE "\n  CHAT3DS 0.8" WHITE);
}

static void drawChatInfo(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("CHAT3DS", "CHAT3DS"));
    printf(WHITE "  %s\n", tr("DEINE NACHRICHTEN", "YOUR MESSAGES"));
    if (chatMessageCount == 0) {
        printf("  %s\n", tr("Noch ist es ganz ruhig.", "It is quiet here for now."));
    } else {
        int first = chatMessageCount > 4 ? chatMessageCount - 4 : 0;
        for (int i = first; i < chatMessageCount; i++)
            printf(TURQUOISE "  > " WHITE "%s\n", chatMessages[i]);
    }
    printf("\n" TURQUOISE "  ----------------------------\n");
    printf("  %s\n", tr("KI-STATUS: NOCH OFFLINE", "AI STATUS: NOT CONNECTED"));
    printf(WHITE "  %s\n\n", tr("Deine Nachricht wird lokal", "Your message is stored"));
    printf("  %s\n", tr("angezeigt, noch nicht gesendet.", "locally, not sent yet."));
    printf(TURQUOISE "\n  A  %s\n", tr("NACHRICHT SCHREIBEN", "WRITE MESSAGE"));
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
    printf("\n");
    printf(WHITE "\n  %s\n\n", tr("Noch ist kein KI-Dienst", "No AI service is connected"));
    printf("  %s\n", tr("mit Chat3DS verbunden.", "to Chat3DS yet."));
    printf("  %s\n\n", tr("ChatGPT ist als erster", "ChatGPT is planned as the"));
    printf("  %s\n", tr("Anbieter geplant.", "first provider."));
    printf("  %s\n", tr("Claude und weitere Anbieter", "Claude and other providers"));
    printf("  %s\n\n", tr("koennen spaeter folgen.", "may be added later."));
    printf("  %s\n", tr("Die Auswahl wird erst", "Provider selection will be"));
    printf("  %s\n", tr("freigeschaltet, wenn die", "enabled once the secure"));
    printf("  %s\n\n", tr("sichere Verbindung steht.", "connection is ready."));
    printf("\n");
    printf(TURQUOISE "\n  B  %s" WHITE, tr("ZURUECK", "BACK"));
}

static void drawSettings(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("EINSTELLUNGEN", "SETTINGS"));
    if (settingSelected == 0) printf("\x1b[46m\x1b[30m  > %s: %s  \x1b[0m" BG_BLUE TURQUOISE "\n",
        tr("MENUTON", "MENU SOUND"), soundEnabled ? tr("EIN", "ON") : tr("AUS", "OFF"));
    else printf("    %s: %s\n", tr("MENUTON", "MENU SOUND"), soundEnabled ? tr("EIN", "ON") : tr("AUS", "OFF"));
    if (settingSelected == 1) printf("\x1b[46m\x1b[30m  > %s: %s  \x1b[0m" BG_BLUE TURQUOISE "\n",
        tr("SPRACHE", "LANGUAGE"), english ? "English" : "Deutsch");
    else printf("    %s: %s\n", tr("SPRACHE", "LANGUAGE"), english ? "English" : "Deutsch");
    if (settingSelected == 2) printf("\x1b[46m\x1b[30m  > %s  \x1b[0m" BG_BLUE TURQUOISE "\n",
        tr("KI-ANBIETER", "AI PROVIDER"));
    else printf("    %s\n", tr("KI-ANBIETER", "AI PROVIDER"));
    printf(WHITE "      %s\n\n", tr("Noch nicht verbunden", "Not connected yet"));
    printf(TURQUOISE "  %s\n", tr("HOCH / RUNTER   AUSWAEHLEN", "UP / DOWN   SELECT"));
    printf("  A               %s\n", tr("AENDERN", "CHANGE"));
    printf("  B               %s" WHITE, tr("ZURUECK", "BACK"));
}

static void drawAbout(void)
{
    beginScreen(&bottomScreen);
    printf("\n" TURQUOISE "          %s\n\n", tr("UEBER CHAT3DS", "ABOUT CHAT3DS"));
    printf(WHITE "  Chat3DS %s\n", tr("ist ein Homebrew-Projekt", "is a homebrew project"));
    printf("  %s\n\n", tr("von ToastLinus.", "by ToastLinus."));
    printf("  %s\n", tr("Entwickelt fuer den", "Made for the"));
    printf("  Nintendo 3DS.\n\n");
    printf(TURQUOISE "  %s\n", tr("UNSER ZIEL", "OUR GOAL"));
    printf(WHITE "  %s\n", tr("Ein einfacher KI-Chat", "A simple AI chat"));
    printf("  %s\n", tr("direkt auf deiner Konsole.", "directly on your console."));
    printf(TURQUOISE "\n  B  %s" WHITE, tr("ZURUECK", "BACK"));
}

int main(int argc, char* argv[])
{
    gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP, false);
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
