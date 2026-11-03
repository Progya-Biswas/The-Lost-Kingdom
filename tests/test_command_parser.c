#include "c_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    ParsedCommand command = {0};

    assert(parse_command("help", &command) == 1);
    assert(command.type == CMD_HELP);

    assert(parse_command("move north", &command) == 1);
    assert(command.type == CMD_MOVE);
    assert(strcmp(command.argument, "NORTH") == 0);

    assert(parse_command("pick luminous key", &command) == 1);
    assert(command.type == CMD_PICK);
    assert(strcmp(command.argument, "LUMINOUS KEY") == 0);

    assert(parse_command("not-a-command", &command) == 1);
    assert(command.type == CMD_UNKNOWN);

    assert(parse_command("   \t\n", &command) == 0);
    assert(parse_command(NULL, &command) == 0);
    assert(parse_command("help", NULL) == 0);

    assert(calculate_score(2, 1, 5) == 50);
    assert(calculate_score(-1, -1, -1) == 0);

    char *copy = duplicate_text("Eldoria");
    assert(copy != NULL);
    assert(strcmp(copy, "Eldoria") == 0);
    free(copy);
    assert(duplicate_text(NULL) == NULL);
    return 0;
}