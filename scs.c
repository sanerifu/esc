#include "estd.c"

int main() {
    EstdArena* ESTD_CLEAN(estd_arena_destroy) arena = NULL;
    EstdString input = {0};
    ESTD_BUBBLE(estd_read_stream(&input, &arena, stdin), "Could not read input");
    printf("%" PRIestr "\n", ESTD_STRING_ARG(input));
    return ESTD_SUCCESS;
}
