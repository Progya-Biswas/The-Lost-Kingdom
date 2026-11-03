#ifndef C_UTILS_H
#define C_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CMD_UNKNOWN = 0, CMD_HELP, CMD_LOOK, CMD_MOVE, CMD_PICK, CMD_DROP,
    CMD_INVENTORY, CMD_USE, CMD_STATUS, CMD_SAVE, CMD_LOAD, CMD_TALK,
    CMD_ATTACK, CMD_MAP, CMD_EXIT
} CommandType;

typedef struct {
    CommandType type;
    char argument[64];
} ParsedCommand;

int parse_command(const char *input, ParsedCommand *output);
int calculate_score(int treasures, int puzzles, int bonus);
char *duplicate_text(const char *source);

#ifdef __cplusplus
}
#endif
#endif