#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
int *childp;
pid_t *pid;
time_t *T;
time_t *Ta;
char path[1000];
bool *timer;
bool *end;
void printstat(struct stat sb);
void signalHandler(int sig)
{
    if (getpid() == *childp)
    {
        printf("\ncannot kill\n");
        printf("\033[0;34mmonitor1 %s\033[0;37m$ ", path);
    }
}
void sighup(int sig)
{
    if (sig == SIGHUP)
    {
        *T = time(NULL);
        *Ta = time(NULL);
        *Ta++;
        *timer = true;
        return;
    }
}
int main()
{
    signal(SIGTSTP, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, sighup);
    childp = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    pid = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *pid = getpid();
    T = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    Ta = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    timer = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *timer = false;
    end = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *end = false;
    if (fork() == 0)
    {
        *childp = getpid();
        signal(SIGTSTP, signalHandler);
        signal(SIGINT, signalHandler);
        signal(SIGQUIT, signalHandler);
        signal(SIGTERM, signalHandler);
        while (true)
        {
            *timer = false;
            kill(*pid, SIGHUP);
            getcwd(path, 1000);
            printf("\033[0;34mmonitor1 %s\033[0;37m$ ", path);
            char filename[1000];

            scanf("%s", filename);

            if (strcmp(filename, "list") == 0)
            {
                DIR *folder = opendir(".");
                struct dirent *file;
                printf("Files of this folder:\n");
                while (file = readdir(folder))
                {
                    printf("%s\n", file->d_name);
                }
                printf("\n");
                closedir(folder);
            }
            else if (strcmp(filename, "q") == 0)
            {
                printf("Ending program");
                *end = true;
                return 0;
            }
            else
            {

                strcat(path, "/");
                char *c = strcat(path, filename);
                struct stat sb;
                int ret = stat(c, &sb);
                if (ret != 0)
                    perror(__FUNCTION__);
                else
                    printstat(sb);
            }
        }
        return 0;
    }
    else
    {

        while (*timer == false)
        {
        }
        while (*Ta - *T < 10)
        {
            if (*end == true)
            {
                break;
            }
            *Ta = time(NULL);
            if (*Ta - *T >= 10)
            {
                //printf("Time start:%ld | time after: %ld", *T, *Ta);
                printf("\n");
                kill(*childp, SIGKILL);
            }
        }
        wait(0);
        munmap(childp, sizeof(int));
        munmap(pid, sizeof(int));
        munmap(T, sizeof(time_t));
        munmap(Ta, sizeof(time_t));
        munmap(timer, sizeof(bool));
        munmap(end, sizeof(bool));
    }
}

void printstat(struct stat sb)
{
    printf("ID of containing device:  [%jx,%jx]\n",
           (long)major(sb.st_dev),
           (long)minor(sb.st_dev));

    printf("File type:                ");

    switch (sb.st_mode & S_IFMT)
    {
    case S_IFBLK:
        printf("block device\n");
        break;
    case S_IFCHR:
        printf("character device\n");
        break;
    case S_IFDIR:
        printf("directory\n");
        break;
    case S_IFIFO:
        printf("FIFO/pipe\n");
        break;
    case S_IFLNK:
        printf("symlink\n");
        break;
    case S_IFREG:
        printf("regular file\n");
        break;
    case S_IFSOCK:
        printf("socket\n");
        break;
    default:
        printf("unknown?\n");
        break;
    }

    printf("I-node number:            %ju\n", (long)sb.st_ino);

    printf("Mode:                     %jo (octal)\n",
           (long)sb.st_mode);

    printf("Link count:               %ju\n", (long)sb.st_nlink);
    printf("Ownership:                UID=%ju   GID=%ju\n",
           (long)sb.st_uid, (long)sb.st_gid);

    printf("Preferred I/O block size: %jd bytes\n",
           (long)sb.st_blksize);
    printf("File size:                %jd bytes\n",
           (long)sb.st_size);
    printf("Blocks allocated:         %jd\n",
           (long)sb.st_blocks);

    printf("Last status change:       %s", ctime(&sb.st_ctime));
    printf("Last file access:         %s", ctime(&sb.st_atime));
    printf("Last file modification:   %s", ctime(&sb.st_mtime));
}
