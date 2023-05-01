// 1
#include "../kernel/types.h"
// 2
#include "../kernel/stat.h"
// 3
#include "user.h"
// 4
#include "../kernel/fs.h"

int count_key(char *path, char key) {
    int res = 0;
    while (*path) {
        res += *(path++) == key;
    }
    return res;
}

void ls(char *path, char key, int *dir_num, int *file_num, int is_root) {
    char buf[128], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        printf("%s [error opening dir]\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf("%s [error stat dir]\n", path);
        close(fd);
        return;
    }

    int count = count_key(path, key);
    printf("%s %d\n", path, count);

    switch (st.type) {
        case T_FILE:
            (*file_num)++;
            break;

        case T_DIR:
            if (!is_root)
                (*dir_num)++;

            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;

                if (strlen(de.name) == 0 || de.name[0] == '.')
                    continue;

                memmove(p, de.name, strlen(de.name));
                p[strlen(de.name)] = 0;

                ls(buf, key, dir_num, file_num, 0);
            }
            break;
    }
    close(fd);
}

/**
 * Spec:
 * - depth <= 5
 * - #files <= 20
 * - len(name) <= 10
 *
 * => path < 5 * 10
 */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: mp0 <directory> <key>\n");
        exit(1);
    }

    if (strlen(argv[2]) != 1) {
        fprintf(2, "mp0: keyword should be a single character.\n");
        exit(1);
    }

    int p[2];
    pipe(p);

    if (fork() == 0) {
        // child: traverse
        close(p[0]);

        int dir_num = 0, file_num = 0;

        // check if <root_directory> is a dir
        int fd;
        if ((fd = open(argv[0], 0) < 0)) {
            printf("%s [error opening dir]\n", argv[0]);
        } else {
            struct stat st;
            if (fstat(fd, &st) < 0) {
                printf("%s [error stat dir]\n", argv[0]);
                close(fd);
                exit(1);
            }

            if (st.type == T_FILE)
                printf("%s [error opening dir]\n", argv[0]);
            else
                ls(argv[1], argv[2][0], &dir_num, &file_num, 1);
        }

        write(p[1], &dir_num, sizeof(int));
        write(p[1], &file_num, sizeof(int));
        close(p[1]);

        exit(0);
    } else {
        close(p[1]);

        int dir_num = 0, file_num = 0;

        read(p[0], &dir_num, sizeof(int));
        read(p[0], &file_num, sizeof(int));
        close(p[0]);

        fprintf(2, "\n%d directories, %d files\n", dir_num, file_num);
    }

    exit(0);
}
