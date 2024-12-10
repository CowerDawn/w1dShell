#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define CONFIG_FILE "usconf.conf"
#define MAX_CMD_LEN 1024
#define MAX_USER_LEN 64

typedef struct {
    char user[MAX_USER_LEN];
    char rootc[4]; 
} UserRights;

UserRights user_rights[100];
int user_count = 0;

void read_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        perror("Error opening configuration file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "user=", 5) == 0) {
            strncpy(user_rights[user_count].user, line + 5, MAX_USER_LEN);
            user_rights[user_count].user[strcspn(user_rights[user_count].user, "\n")] = 0;
        } else if (strncmp(line, "rootc=", 6) == 0) {
            strncpy(user_rights[user_count].rootc, line + 6, 4);
            user_rights[user_count].rootc[strcspn(user_rights[user_count].rootc, "\n")] = 0;
            user_count++;
        }
    }

    fclose(file);
}

int is_forbidden(const char *user, const char *cmd) {
    if (strcmp(user, "root") == 0) {
        return 0;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_rights[i].user, user) == 0) {
            return strcmp(user_rights[i].rootc, "on") != 0;
        }
    }

    return 1;
}

void shut() {
    printf("Shutting down the system...\n");
    system("sudo shutdown -h now");
}

void openp(const char *program) {
    printf("Opening %s...\n", program);
    char command[MAX_CMD_LEN];
    snprintf(command, sizeof(command), "%s &", program);
    system(command);
}

void closep(const char *program) {
    printf("Closing %s...\n", program);
    char command[MAX_CMD_LEN];
    snprintf(command, sizeof(command), "pkill -f %s", program);
    system(command);
}

void neofetch() {
    printf("Displaying w1dShell info...\n");
    system("neofetch --ascii_distro w1dShell");
}

int main() {
    read_config();

    char current_user[MAX_USER_LEN];
    struct passwd *pw = getpwuid(getuid());
    strncpy(current_user, pw->pw_name, MAX_USER_LEN);

    const char *root_commands[] = {
        "sudo", "su", "doas", "pkexec", "systemctl", "mount", "umount", "dd",
        "fdisk", "gdisk", "parted", "mkfs", "fsck", "chown", "chmod", "chgrp",
        "passwd", "visudo", "adduser", "deluser", "addgroup", "delgroup",
        "reboot", "shutdown", "poweroff", "halt"
    };
    int root_commands_count = sizeof(root_commands) / sizeof(root_commands[0]);

    while (1) {
        char input[MAX_CMD_LEN];
        printf("w1dShell> ");
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0; 

        char *cmd = strtok(input, " ");
        if (!cmd) continue;

        int forbidden = 0;
        for (int i = 0; i < root_commands_count; i++) {
            if (strcmp(cmd, root_commands[i]) == 0 && is_forbidden(current_user, cmd)) {
                printf("Error: Command '%s' is forbidden for user '%s'.\n", cmd, current_user);
                forbidden = 1;
                break;
            }
        }

        if (forbidden) continue;

        if (strcmp(cmd, "shut") == 0) {
            shut();
        } else if (strcmp(cmd, "openp") == 0) {
            char *program = strtok(NULL, " ");
            if (program) {
                openp(program);
            } else {
                printf("Usage: openp <program>\n");
            }
        } else if (strcmp(cmd, "closep") == 0) {
            char *program = strtok(NULL, " ");
            if (program) {
                closep(program);
            } else {
                printf("Usage: closep <program>\n");
            }
        } else if (strcmp(cmd, "neofetch") == 0) {
            neofetch();
        } else if (strcmp(cmd, "exit") == 0) {
            printf("Exiting w1dShell...\n");
            break;
        } else if (strcmp(cmd, "root") == 0) {
            printf("Switching to root...\n");
            setuid(0);
            setgid(0);
            execl("/bin/bash", "bash", NULL);
        } else {
            system(input);
        }
    }

    return 0;
}
