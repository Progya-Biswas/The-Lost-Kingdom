#include "c_utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_upper(char *dest, size_t capacity, const char *src) {
    size_t i = 0;
    if (capacity == 0) return;
    while (src[i] && i + 1 < capacity) {
        dest[i] = (char)toupper((unsigned char)src[i]);
        ++i;
    }
    dest[i] = '\0';
}

int parse_command(const char *input, ParsedCommand *output) {
    char buffer[160];
    char *token, *argument;
    if (!input || !output) return 0;

    snprintf(buffer, sizeof(buffer), "%s", input);
    token = strtok(buffer, " \t\r\n");
    argument = strtok(NULL, "\r\n");
    if (!token) return 0;

    while (argument && (*argument == ' ' || *argument == '\t')) ++argument;
    copy_upper(output->argument, sizeof(output->argument), argument ? argument : "");

    char command[32];
    copy_upper(command, sizeof(command), token);
    output->type = CMD_UNKNOWN;

    if      (strcmp(command, "HELP") == 0)      output->type = CMD_HELP;
    else if (strcmp(command, "LOOK") == 0)      output->type = CMD_LOOK;
    else if (strcmp(command, "MOVE") == 0)      output->type = CMD_MOVE;
    else if (strcmp(command, "PICK") == 0)      output->type = CMD_PICK;
    else if (strcmp(command, "DROP") == 0)      output->type = CMD_DROP;
    else if (strcmp(command, "INVENTORY") == 0) output->type = CMD_INVENTORY;
    else if (strcmp(command, "USE") == 0)       output->type = CMD_USE;
    else if (strcmp(command, "STATUS") == 0)    output->type = CMD_STATUS;
    else if (strcmp(command, "SAVE") == 0)      output->type = CMD_SAVE;
    else if (strcmp(command, "LOAD") == 0)      output->type = CMD_LOAD;
    else if (strcmp(command, "TALK") == 0)      output->type = CMD_TALK;
    else if (strcmp(command, "ATTACK") == 0)    output->type = CMD_ATTACK;
    else if (strcmp(command, "MAP") == 0)       output->type = CMD_MAP;
    else if (strcmp(command, "EXIT") == 0)      output->type = CMD_EXIT;
    return 1;
}

int calculate_score(int treasures, int puzzles, int bonus) {
    if (treasures < 0) treasures = 0;
    if (puzzles < 0) puzzles = 0;
    if (bonus < 0) bonus = 0;
    return treasures * 10 + puzzles * 25 + bonus;
}

char *duplicate_text(const char *source) {
    size_t length;
    char *copy;
    if (!source) return NULL;
    length = strlen(source) + 1;
    copy = (char *)malloc(length);
    if (copy) memcpy(copy, source, length);
    return copy;
}