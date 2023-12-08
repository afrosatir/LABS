/*
 * Представление структуры файла логов
 * [IP адрес] - - [Дата] "[HTTP метод] [Запрошенный ресурс]" [Код состояния] [Размер ответа] "[Заголовок Referer]" "[Заголовок User-Agent]"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

struct LogEntry
{
    char *ip;
    char *dateTime;
    char *request;
    char *browser;
};

struct BrowserCount
{
    char *name;
    int count;
};

void freeLogEntries(struct LogEntry *logEntries, int numEntries)
{
    for (int i = 0; i < numEntries; i++)
    {
        free(logEntries[i].ip);
        free(logEntries[i].dateTime);
        free(logEntries[i].request);
        free(logEntries[i].browser);
    }
    free(logEntries);
}

void freeBrowsersCount(struct BrowserCount *browsersCount, int numUniqueBrowsers)
{
    for (int i = 0; i < numUniqueBrowsers; i++)
    {
        free(browsersCount[i].name);
    }
    free(browsersCount);
}

void readFile(const char *filename, struct LogEntry **logEntries, int *numEntries)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    *numEntries = 0;
    *logEntries = NULL;
    size_t allocatedEntries = 0;

    while (fgets(line, sizeof(line), file))
    {
        char ip[50], dateTime[50], request[255], browser[255];
        if (sscanf(line, "%s %*s %*s [%[^]]] \"%*[^\"]\" %*s %*s \"%[^\"]\" \"%[^\"]\"",
                   ip, dateTime, request, browser) >= 3)
        {
            if (*numEntries >= allocatedEntries)
            {
                // Increase the allocated memory for logEntries
                allocatedEntries += 10; // You can adjust the increment as needed
                *logEntries = realloc(*logEntries, allocatedEntries * sizeof(struct LogEntry));
                if (*logEntries == NULL)
                {
                    printf("Memory allocation failed.\n");
                    exit(1);
                }
            }

            (*logEntries)[*numEntries].ip = strdup(ip);
            (*logEntries)[*numEntries].dateTime = strdup(dateTime);

            if (sscanf(line, "%*s %*s %*s [%*[^]]] \"%*[^\"]\" %*s %*s \"%*[^\"]\" \"%*[^\"]\" %s", request) == 1)
            {
                (*logEntries)[*numEntries].request = strdup(request);
            }
            else
            {
                (*logEntries)[*numEntries].request = NULL;
            }

            (*logEntries)[*numEntries].browser = strdup(browser);

            (*numEntries)++;
        }
        else
        {
            printf("Error parsing string:\n%s\n", line);
        }
    }

    fclose(file);
}

// Function to check if the browser name contains valid characters
int isValidBrowserName(const char *browser)
{
    while (*browser != '\0')
    {
        if (!(isalnum(*browser) || *browser == '/' || *browser == ' ' || isalpha(*browser) || *browser == '_' || *browser == '.' || *browser == '-'))
        {
            return 0; // Not a valid character
        }
        browser++;
    }
    return 1; // Valid browser name
}

// Function to check if it's a new valid browser
int isNewBrowser(const char *browser, struct BrowserCount *browsersCount, int numUniqueBrowsers)
{
    if (!isValidBrowserName(browser))
    {
        return 0; // Not a valid browser name
    }

    for (int j = 0; j < numUniqueBrowsers; j++)
    {
        if (strcmp(browser, browsersCount[j].name) == 0)
        {
            return 0; // Not a new valid browser
        }
    }
    return 1; // New valid browser
}

void freeBrowserCount(struct BrowserCount *browsersCount, int numUniqueBrowsers)
{
    for (int i = 0; i < numUniqueBrowsers; i++)
    {
        free(browsersCount[i].name);
    }
    free(browsersCount);
}

void calculateBrowserUsage(struct LogEntry *logEntries, int numEntries)
{
    struct BrowserCount *browsersCount = NULL;
    int numUniqueBrowsers = 0;
    size_t allocatedBrowsers = 0;

    for (int i = 0; i < numEntries; i++)
    {
        int inParentheses = 0;
        char *tempBrowser = strdup(logEntries[i].browser);
        int diff = 0;

        for (int j = 0; tempBrowser[j] != '\0'; j++)
        {
            if (tempBrowser[j] == '(')
            {
                inParentheses = 1;
            }
            else if (tempBrowser[j] == ')' && inParentheses)
            {
                inParentheses = 0;
                diff = j + 2;
                j += 2;
            }
            else if (!inParentheses)
            {
                if (isspace(tempBrowser[j]))
                {
                    tempBrowser[j] = '\0';
                    if (isNewBrowser(tempBrowser + diff, browsersCount, numUniqueBrowsers))
                    {
                        if (numUniqueBrowsers >= allocatedBrowsers)
                        {
                            allocatedBrowsers += 10; // Increment by 10 or adjust as needed
                            browsersCount = realloc(browsersCount, allocatedBrowsers * sizeof(struct BrowserCount));
                            if (browsersCount == NULL)
                            {
                                printf("Memory allocation failed.\n");
                                exit(1);
                            }
                        }

                        browsersCount[numUniqueBrowsers].name = strdup(tempBrowser + diff);
                        browsersCount[numUniqueBrowsers].count = 1;
                        numUniqueBrowsers++;
                    }
                    else
                    {
                        for (int k = 0; k < numUniqueBrowsers; k++)
                        {
                            if (strcmp(tempBrowser + diff, browsersCount[k].name) == 0)
                            {
                                browsersCount[k].count++;
                                break;
                            }
                        }
                    }
                }
            }
        }
        // free(tempBrowser);
    }

    int diff = 0;
    for (int i = 0; i < numUniqueBrowsers; i++)
    {
        if (strlen(browsersCount[i].name) < 3)
        {
            diff++;
        }
    }

    printf("Total number of unique browsers: %d\n", numUniqueBrowsers - diff);
    printf("Quantity of each browser:\n");

    for (int i = 0; i < numUniqueBrowsers; i++)
    {
        if (strlen(browsersCount[i].name) >= 3)
        {
            printf("%s: %d\n", browsersCount[i].name, browsersCount[i].count);
        }
    }

    freeBrowserCount(browsersCount, numUniqueBrowsers);
}

int main()
{
    struct LogEntry *logEntries;
    int numEntries = 0;

    readFile("C:\\Users\\stepa\\Desktop\\labs\\lab3\\access.log", &logEntries, &numEntries);
    calculateBrowserUsage(logEntries, numEntries);
    // freeLogEntries(logEntries, numEntries);

    return 0;
}
