#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define EXT ".txt"
#define MAX_PATH 4096
#define MAX_LINE 8192

char *get_notes_dir() {
    const char *home = getenv("HOME");
    static char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/notes", home);
    return path;
}

void make_notes_dir() {
    char *dir = get_notes_dir();
    mkdir(dir, 0755);
}

void path_for_note(char *out, const char *name) {
    snprintf(out, MAX_PATH, "%s/%s%s", get_notes_dir(), name, EXT);
}

int is_valid_name(const char *name) {
    for (int i = 0; name[i]; i++) {
        if (!(name[i] == '-' || name[i] == '_' || name[i] == '.' ||
              (name[i] >= 'a' && name[i] <= 'z') ||
              (name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= '0' && name[i] <= '9')))
            return 0;
    }
    return 1;
}

void print_help(const char *bin) {
    printf("Usage: %s <command> [args]\n", bin);
    printf("Commands:\n");
    printf("  add <name>       Make a new note (or edit existing)\n");
    printf("  list             Show all notes\n");
    printf("  view <name>      View a note\n");
    printf("  setfire <name>   Burn a note (with 🔥)\n");
    printf("  search <string>  Find stuff across notes\n");
    printf("  help             Show this help\n");
}

void add_note(const char *name) {
    if (!is_valid_name(name)) {
        printf("nah bro, that filename's sketchy\n");
        return;
    }

    char path[MAX_PATH];
    path_for_note(path, name);

    char cmd[MAX_PATH + 32];
    snprintf(cmd, sizeof(cmd), "nano \"%s\"", path);
    system(cmd);
}

void list_notes() {
    DIR *dir = opendir(get_notes_dir());
    if (!dir) {
        puts("couldn't open notes dir");
        return;
    }

    struct dirent *d;
    while ((d = readdir(dir))) {
        if (d->d_type == DT_REG && strstr(d->d_name, EXT)) {
            int len = strlen(d->d_name) - strlen(EXT);
            printf("%.*s\n", len, d->d_name);
        }
    }

    closedir(dir);
}

void view_note(const char *name) {
    if (!is_valid_name(name)) {
        puts("invalid name, try again");
        return;
    }

    char path[MAX_PATH];
    path_for_note(path, name);

    char cmd[MAX_PATH + 16];
    snprintf(cmd, sizeof(cmd), "cat \"%s\"", path);
    system(cmd);
}

void setfire(const char *name) {
    if (!is_valid_name(name)) {
        puts("bad name");
        return;
    }

    char path[MAX_PATH];
    path_for_note(path, name);

    FILE *f = fopen(path, "r");
    if (!f) {
        puts("note not found");
        return;
    }
    fclose(f);

    const char *frames[] = {
        "🔥        \n   🔥     \n      🔥  \n   🔥     \n🔥        \n",
        "   🔥     \n🔥    🔥  \n   🔥     \n🔥    🔥  \n   🔥     \n",
        "🔥🔥🔥🔥🔥\n🔥🔥🔥🔥🔥\n🔥🔥🔥🔥🔥\n🔥🔥🔥🔥🔥\n🔥🔥🔥🔥🔥\n"
    };

    for (int i = 0; i < 5; i++) {
        printf("\033[2J\033[H");
        puts(frames[i % 3]);
        fflush(stdout);
        usleep(300000);
    }

    remove(path);
    printf("\033[2J\033[H");
    printf("'%s.txt' burned to ashes\n", name);
}

void search_notes(const char *term) {
    DIR *dir = opendir(get_notes_dir());
    if (!dir) {
        puts("can't open notes dir");
        return;
    }

    struct dirent *d;
    while ((d = readdir(dir))) {
        if (d->d_type == DT_REG && strstr(d->d_name, EXT)) {
            char file[MAX_PATH];
            snprintf(file, sizeof(file), "%s/%s", get_notes_dir(), d->d_name);
            FILE *f = fopen(file, "r");
            if (!f) continue;

            char line[MAX_LINE];
            int match = 0;
            while (fgets(line, sizeof(line), f)) {
                if (strstr(line, term)) {
                    match = 1;
                    break;
                }
            }
            fclose(f);

            if (match) {
                int len = strlen(d->d_name) - strlen(EXT);
                printf("%.*s\n", len, d->d_name);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    make_notes_dir();

    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "add") && argc == 3) {
        add_note(argv[2]);
    } else if (!strcmp(argv[1], "list")) {
        list_notes();
    } else if (!strcmp(argv[1], "view") && argc == 3) {
        view_note(argv[2]);
    } else if (!strcmp(argv[1], "setfire") && argc == 3) {
        setfire(argv[2]);
    } else if (!strcmp(argv[1], "search") && argc == 3) {
        search_notes(argv[2]);
    } else if (!strcmp(argv[1], "help")) {
        print_help(argv[0]);
    } else {
        puts("bad command or args\n");
        print_help(argv[0]);
    }

    return 0;
}
