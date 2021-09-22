#include <stdio.h>
#include <stdlib.h>

char cmd[1024];

int main(int argc, char *argv[]){
    // re-launch in new window, if needed
    char *new_window_val = getenv("IN_NEW_WINDOW");
    const char *user_arg = argc < 2 ? "" : argv[1];
    if (!new_window_val || new_window_val[0] != '1') {
        snprintf(cmd, sizeof(cmd), "gnome-terminal -e IN_NEW_WINDOW=1 %s %s", argv[0], user_arg);
        printf("RELAUNCH! %s\n", cmd);
        return system(cmd);
    }
    // do normal stuff
    printf("User text: %s\n", argv[1]);
    return 0;
}