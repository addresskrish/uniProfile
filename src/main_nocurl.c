// src/main_improved.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>  // for Sleep
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>   // for usleep
#define SLEEP_MS(x) usleep((x) * 1000)
#endif

#define URL_MAX 1024
#define CMD_MAX 2048
#define BUF 128

// Layout configuration
#define STATUS_WIDTH 18
#define SITE_WIDTH   14
#define URL_WIDTH    58
#define CODE_WIDTH   12

// ANSI colors
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define RED     "\x1b[31m"
#define CYAN    "\x1b[36m"
#define GREY    "\x1b[90m"
#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"

typedef struct {
    const char *name;        // display name
    const char *profile_fmt; // profile URL format
    int reliable_status;     // 1 = HTTP status is reliable, 0 = MAYBE/blocked
} Site;

// Expanded site list
static Site sites[] = {
    {"GitHub",       "https://github.com/%s",                     1},
    {"Reddit",       "https://www.reddit.com/user/%s",            1},
    {"StackOverflow","https://stackoverflow.com/users/%s",        1},
    {"GitLab",       "https://gitlab.com/%s",                     1},
    {"Medium",       "https://medium.com/@%s",                    1},
    {"Dev.to",       "https://dev.to/%s",                         1},
    {"YouTube",      "https://www.youtube.com/@%s",               1},
    {"Pinterest",    "https://www.pinterest.com/%s/",             1},
    {"Vimeo",        "https://vimeo.com/%s",                      1},
    {"LinkedIn",     "https://www.linkedin.com/in/%s",            0}, // Often blocks bots
    {"Twitter/X",    "https://twitter.com/%s",                    0}, // Often returns 200 for 404
    {"Instagram",    "https://www.instagram.com/%s",              0},
    {"Facebook",     "https://www.facebook.com/%s",               0}
};

static int SITE_COUNT = sizeof(sites) / sizeof(sites[0]);

// ---------- Helper Functions ----------

// Security: Prevent command injection by allowing only safe chars
int sanitize_username(char *username) {
    for (int i = 0; username[i]; i++) {
        if (!isalnum(username[i]) && username[i] != '-' && username[i] != '_' && username[i] != '.') {
            return 0; // Invalid character found
        }
    }
    return 1;
}

void draw_progress_bar(int current, int total) {
    int width = 32;
    float ratio = (float)current / total;
    int filled = (int)(ratio * width);

    printf("\rChecking sites: [");
    for (int i = 0; i < width; i++) {
        if (i < filled) printf(CYAN "=" RESET);
        else printf(GREY "." RESET);
    }
    printf("] %3d%%", (int)(ratio * 100));
    fflush(stdout);
}

// Check HTTP status using system curl
// Improvements: Added Timeout (-m 5), muted stderr better
int http_status_code(const char *url) {
    char cmd[CMD_MAX];
    
    // Common User-Agent to avoid immediate blocking
    const char *user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";

#if defined(_WIN32) || defined(_WIN64)
    snprintf(cmd, sizeof(cmd),
             "curl -m 5 -A \"%s\" -L -s -o NUL -w \"%%{http_code}\" \"%s\"",
             user_agent, url);
#else
    snprintf(cmd, sizeof(cmd),
             "curl -m 5 -A \"%s\" -L -s -o /dev/null -w \"%%{http_code}\" \"%s\"",
             user_agent, url);
#endif

    FILE *f = popen(cmd, "r");
    if (!f) return 0;
    
    char out[BUF];
    if (!fgets(out, sizeof out, f)) {
        pclose(f);
        return 0;
    }
    pclose(f);
    return atoi(out);
}

// Special check for GitHub API (More accurate than scraping)
int github_status(const char *username) {
    char api_url[URL_MAX];
    snprintf(api_url, sizeof api_url, "https://api.github.com/users/%s", username);
    return http_status_code(api_url);
}

// Special check for Reddit JSON endpoint
int reddit_status(const char *username) {
    char api_url[URL_MAX];
    snprintf(api_url, sizeof api_url, "https://www.reddit.com/user/%s/about.json", username);
    return http_status_code(api_url);
}

void open_in_browser(const char *url) {
    char cmd[CMD_MAX];
#if defined(_WIN32) || defined(_WIN64)
    snprintf(cmd, sizeof cmd, "start \"\" \"%s\"", url);
#elif defined(__APPLE__)
    snprintf(cmd, sizeof cmd, "open \"%s\"", url);
#else
    snprintf(cmd, sizeof cmd, "xdg-open \"%s\" >/dev/null 2>&1 &", url);
#endif
    system(cmd);
}

void classify_status(int code, int reliable_status, int *is_found, const char **label, const char **color) {
    if (code == 404) {
        *is_found = 0;
        *label = "not found";
        *color = RED;
    } else if (code >= 200 && code < 400) {
        *is_found = 1;
        if (reliable_status) {
            *label = "CONFIRMED";
            *color = GREEN;
        } else {
            *label = "MAYBE";
            *color = YELLOW;
        }
    } else if (code == 0) {
        *is_found = 0;
        *label = "timeout/error";
        *color = RED;
    } else {
        *is_found = 0;
        *label = "blocked";
        *color = YELLOW;
    }
}

void save_results(const char *username, int found_count, int *found_indices) {
    FILE *fp = fopen("results.txt", "w");
    if (!fp) {
        printf(RED "Error saving file.\n" RESET);
        return;
    }
    fprintf(fp, "UniProfile Results for '%s'\n", username);
    fprintf(fp, "----------------------------\n");
    char url[URL_MAX];
    for (int i = 0; i < found_count; i++) {
        int idx = found_indices[i];
        snprintf(url, sizeof url, sites[idx].profile_fmt, username);
        fprintf(fp, "%-15s : %s\n", sites[idx].name, url);
    }
    fclose(fp);
    printf(GREEN "\nResults saved to 'results.txt'\n" RESET);
}

void printHeader(void) {
    printf(BOLD CYAN "\n");
    printf("  _    _       _ ____             __ _ _      \n");
    printf(" | |  | |     (_)  _ \\           / _(_) |     \n");
    printf(" | |  | |_ __  _| |_) |_ __ ___ | |_ _| | ___ \n");
    printf(" | |  | | '_ \\| |  __/| '__/ _ \\|  _| | |/ _ \\\n");
    printf(" | |__| | | | | | |   | | | (_) | | | | |  __/\n");
    printf("  \\____/|_| |_|_|_|   |_|  \\___/|_| |_|_|\\___|\n");
    printf("===================================================\n");
    printf("      Social Media OSINT Tool - Version 1.1\n");
    printf("===================================================\n" RESET);
}

// ---------- Main Loop ----------

int main(void) {
    char username[256];
    printHeader();

    while (1) {
        printf("\nEnter username: ");
        if (!fgets(username, sizeof username, stdin)) return 1;
        username[strcspn(username, "\r\n")] = 0;
        
        if (strlen(username) == 0) return 1;

        if (!sanitize_username(username)) {
            printf(RED "Error: Username contains invalid characters (spaces, special symbols).\n" RESET);
            printf(YELLOW "Allowed: A-Z, a-z, 0-9, -, _, .\n" RESET);
            continue;
        }
        break;
    }

    printf("\nScanning username \"%s\"...\n", username);

    char url[URL_MAX];
    char code_str[CODE_WIDTH + 1];
    int found_indices[32], found_count = 0;
    int codes[32];

    draw_progress_bar(0, SITE_COUNT);

    for (int i = 0; i < SITE_COUNT; ++i) {
        snprintf(url, sizeof url, sites[i].profile_fmt, username);

        int code;
        // Select check method
        if (strcmp(sites[i].name, "GitHub") == 0)      code = github_status(username);
        else if (strcmp(sites[i].name, "Reddit") == 0) code = reddit_status(username);
        else                                           code = http_status_code(url);

        codes[i] = code;
        draw_progress_bar(i + 1, SITE_COUNT);
        SLEEP_MS(100); // Slight delay to prevent local port exhaustion

        int is_found;
        const char *label, *color;
        classify_status(code, sites[i].reliable_status, &is_found, &label, &color);
        
        if (is_found) found_indices[found_count++] = i;
    }

    printf("\n\n");

    // --- Results Table ---
    printf(BOLD "%-3s %-*s %-*s %-*s %-*s\n" RESET,
           "ID", STATUS_WIDTH, "STATUS", SITE_WIDTH, "SITE", URL_WIDTH, "URL", CODE_WIDTH, "CODE");
    printf("-------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < SITE_COUNT; ++i) {
        snprintf(url, sizeof url, sites[i].profile_fmt, username);
        int is_found;
        const char *label, *color;
        classify_status(codes[i], sites[i].reliable_status, &is_found, &label, &color);
        
        snprintf(code_str, sizeof code_str, "(HTTP %d)", codes[i]);

        printf("%-3d ", i + 1);
        int len = (int)strlen(label), pad = STATUS_WIDTH - len;
        if (pad < 1) pad = 1;
        printf("%s%s" RESET, color, label);
        for (int s = 0; s < pad; ++s) putchar(' ');
        
        printf(" %-*s %-*s %*s\n", SITE_WIDTH, sites[i].name, URL_WIDTH, url, CODE_WIDTH, code_str);
    }

    printf("\n" BOLD CYAN "Total %d profiles found for '%s'\n\n" RESET, found_count, username);

    // --- Interaction Loop ---
    char choice[64];
    while (1) {
        printf(BOLD "Commands: [id] to open, [all] open all, [save] to file, [exit] to quit: " RESET);
        if (!fgets(choice, sizeof choice, stdin)) break;
        choice[strcspn(choice, "\r\n")] = 0;

        if (strcmp(choice, "exit") == 0 || strcmp(choice, "quit") == 0) break;
        
        if (strcmp(choice, "save") == 0) {
            save_results(username, found_count, found_indices);
        }
        else if (strcmp(choice, "all") == 0) {
            for (int i = 0; i < found_count; ++i) {
                snprintf(url, sizeof url, sites[found_indices[i]].profile_fmt, username);
                open_in_browser(url);
            }
        }
        else {
            int sel = atoi(choice);
            if (sel >= 1 && sel <= SITE_COUNT) {
                snprintf(url, sizeof url, sites[sel - 1].profile_fmt, username);
                open_in_browser(url);
            } else {
                printf(RED "Invalid command.\n" RESET);
            }
        }
    }

    return 0;
}