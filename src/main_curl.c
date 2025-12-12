// src/main_curl.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define URL_MAX 1024
#define SITE_COUNT 12

typedef struct {
    const char *name;
    const char *url_fmt; // printf-style with %s
} Site;

static Site sites[SITE_COUNT] = {
    {"GitHub",      "https://github.com/%s"},
    {"LinkedIn",    "https://www.linkedin.com/in/%s"},
    {"Twitter",     "https://twitter.com/%s"},
    {"Instagram",   "https://www.instagram.com/%s"},
    {"Reddit",      "https://www.reddit.com/user/%s"},
    {"StackOverflow","https://stackoverflow.com/users/%s"},
    {"GitLab",      "https://gitlab.com/%s"},
    {"Medium",      "https://medium.com/@%s"},
    {"YouTube",     "https://www.youtube.com/%s"},
    {"Dev.to",      "https://dev.to/%s"},
    {"Facebook",    "https://www.facebook.com/%s"},
    {"Npm",         "https://www.npmjs.com/~%s"}
};

int url_exists_curl(const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return 0;
    long response_code = 0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);        // HEAD only
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "uniprofile/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    } else {
        // sometimes HEAD is disallowed; try a GET fallback
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }

    curl_easy_cleanup(curl);
    return (res == CURLE_OK) && (response_code >= 200 && response_code < 400);
}

static void open_in_browser(const char *url) {
#if defined(_WIN32) || defined(_WIN64)
    char cmd[URL_MAX + 32];
    snprintf(cmd, sizeof cmd, "start \"\" \"%s\"", url);
    system(cmd);
#elif defined(__APPLE__)
    char cmd[URL_MAX + 32];
    snprintf(cmd, sizeof cmd, "open \"%s\"", url);
    system(cmd);
#else
    char cmd[URL_MAX + 32];
    snprintf(cmd, sizeof cmd, "xdg-open \"%s\" >/dev/null 2>&1 &", url);
    system(cmd);
#endif
}

int main(void) {
    char username[256];
    printf("Enter username: ");
    if (!fgets(username, sizeof username, stdin)) return 1;
    username[strcspn(username, "\r\n")] = 0;

    if (strlen(username) == 0) {
        printf("No username entered. Exiting.\n");
        return 1;
    }

    CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_ALL);
    if (curl_global_res != CURLE_OK) {
        fprintf(stderr, "curl_global_init failed\n");
        return 1;
    }

    char url[URL_MAX];
    int found_count = 0;
    int found_indices[SITE_COUNT];

    printf("\nChecking username \"%s\" on %d sites...\n\n", username, SITE_COUNT);

    for (int i = 0; i < SITE_COUNT; ++i) {
        snprintf(url, sizeof url, sites[i].url_fmt, username);
        int ok = url_exists_curl(url);
        if (ok) {
            printf("[%2d] FOUND   - %s -> %s\n", i+1, sites[i].name, url);
            found_indices[found_count++] = i;
        } else {
            printf("[%2d] not found - %s\n", i+1, sites[i].name);
        }
    }

    if (found_count == 0) {
        printf("\nNo profiles found for \"%s\".\n", username);
        curl_global_cleanup();
        return 0;
    }

    printf("\nFound %d profile(s).\n", found_count);
    printf("Type an index to open its profile, 'all' to open all found profiles, or 'q' to quit: ");
    char choice[64];
    if (!fgets(choice, sizeof choice, stdin)) return 0;
    choice[strcspn(choice, "\r\n")] = 0;

    if (strcmp(choice, "all") == 0) {
        for (int i = 0; i < found_count; ++i) {
            int idx = found_indices[i];
            snprintf(url, sizeof url, sites[idx].url_fmt, username);
            open_in_browser(url);
        }
    } else if (strcmp(choice, "q") == 0 || strcmp(choice, "Q") == 0) {
        // exit
    } else {
        int sel = atoi(choice);
        if (sel >= 1 && sel <= SITE_COUNT) {
            int idx = sel - 1;
            snprintf(url, sizeof url, sites[idx].url_fmt, username);
            open_in_browser(url);
        } else {
            printf("Not a valid selection.\n");
        }
    }

    curl_global_cleanup();
    return 0;
}
