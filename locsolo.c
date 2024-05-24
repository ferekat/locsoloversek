#include "sys/types.h"
#include "unistd.h"
#include "stdlib.h"
#include "signal.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "wait.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "sys/shm.h"
#include "sys/sem.h"
#include "sys/stat.h"

#define Indulj SIGUSR1
pid_t mainProcessValue = 0;
int ready = 0;

struct vers
{
    unsigned int id;
    char text[100];
};

unsigned int id = 1;

void read_console(const char *console_text, char *result)
{
    printf("%s", console_text);
    scanf("\n%[^\t\n]s", result);
}

void file_creation()
{
    FILE *file = fopen("locsoloversek.txt", "a");
    fclose(file);
}

struct vers *file_read(FILE *file)
{
    struct vers *aktual = (struct vers *)malloc(sizeof(struct vers));
    fscanf(file, "%u\t%[^\n]", &(aktual->id), aktual->text);
    return aktual;
}

void id_setup()
{
    FILE *infile;
    struct vers *aktual;
    id = 0;
    infile = fopen("locsoloversek.txt", "r");
    if (infile != NULL)
    {
        aktual = file_read(infile);
        while (!feof(infile))
        {
            id = aktual->id;
            free(aktual);
            aktual = file_read(infile);
        }
        free(aktual);
    }
    fclose(infile);
    id++;
}

void vers_creation(const struct vers *createvers)
{
    struct vers a;
    char beolvasott_ertek[100];

    if (createvers != NULL)
    {
        a = *createvers;
    }
    a.id = id;
    id++;
    read_console("Locsolóvers:\n", beolvasott_ertek);

    if (strcmp(beolvasott_ertek, "-0") == 0)
    {
        strcpy(a.text, createvers->text);
    }
    else
    {
        strcpy(a.text, beolvasott_ertek);
    }

    FILE *outfile;
    outfile = fopen("locsoloversek.txt", "a");

    if (outfile == NULL)
    {
        fprintf(stderr, "\nHiba a fájl megnyitáskor!\n");
        exit(1);
    }

    fprintf(outfile, "%u\t%s\n", a.id, a.text);
    if (fprintf != 0)
    {
        printf("A felvétel sikeres volt!\n");
    }
    else
    {
        printf("Hiba történt az írásnál!\n");
    }

    fclose(outfile);
}

void vers_list()
{
    FILE *file;
    struct vers *aktual;
    file = fopen("locsoloversek.txt", "r");
    if (file != NULL)
    {
        aktual = file_read(file);
        while (!feof(file))
        {
            printf("%u\t%s\n", aktual->id, aktual->text);
            free(aktual);
            aktual = file_read(file);
        }
        free(aktual);
    }
    fclose(file);
    printf("\n");
}

void vers_delete(int index)
{
    FILE *file;
    FILE *temp;
    struct vers *aktual;
    file = fopen("locsoloversek.txt", "r");
    temp = fopen("temp.txt", "a");
    id = 1;
    if (file != NULL)
    {
        aktual = file_read(file);
        while (!feof(file))
        {
            if (aktual->id != index)
            {
                fprintf(temp, "%u\t%s\n", id, aktual->text);
                id++;
            }
            free(aktual);
            aktual = file_read(file);
        }
        free(aktual);
    }
    fclose(file);
    fclose(temp);
    remove("locsoloversek.txt");
    rename("temp.txt", "locsoloversek.txt");
}

void vers_modify(int index)
{
    FILE *file;
    FILE *temp;

    struct vers *aktual;
    char beolvasott_ertek[100];

    file = fopen("locsoloversek.txt", "r");
    temp = fopen("temp.txt", "a");
    if (file != NULL)
    {
        aktual = file_read(file);
        while (!feof(file))
        {
            if (aktual->id == index)
            {
                read_console("Módosítás:\n", beolvasott_ertek);
                fprintf(temp, "%u\t%s\n", aktual->id, beolvasott_ertek);
            }
            else
            {
                fprintf(temp, "%u\t%s\n", aktual->id, aktual->text);
            }
            free(aktual);
            aktual = file_read(file);
        }
        free(aktual);
    }
    fclose(file);
    fclose(temp);
    remove("locsoloversek.txt");
    rename("temp.txt", "locsoloversek.txt");
}

int generateVersek()
{
    srand(time(NULL));
    int length = 0;
    FILE *file;

    struct vers *aktual;

    file = fopen("locsoloversek.txt", "r");
    if (file != NULL)
    {
        aktual = file_read(file);
        while (!feof(file))
        {
            length++;
            free(aktual);
            aktual = file_read(file);
        }
        free(aktual);
    }
    fclose(file);
    return rand() % length + 1;
}

void starthandler(int sig)
{
    if (sig == Indulj)
    {
        ready++;
    }
}

const char *vers_chose(int index)
{
    FILE *file;

    struct vers *aktual;

    file = fopen("locsoloversek.txt", "r");
    if (file != NULL)
    {
        aktual = file_read(file);
        while (!feof(file))
        {
            if (aktual->id == index)
            {
                return aktual->text;
            }
            free(aktual);
            aktual = file_read(file);
        }
        free(aktual);
    }
    fclose(file);
}

pid_t childProcess(int pipe_id_rec, int pipe_id_send)
{
    pid_t process = fork();
    if (process == -1)
    {
        exit(-1);
    }
    if (process > 0)
    {
        return process;
    }

    kill(mainProcessValue, Indulj);

    int arrived;
    read(pipe_id_rec, &arrived, sizeof(int));
    printf("Gyerek %d : Megérkeztem Barátfára!\n", arrived);

    write(pipe_id_send, &arrived, sizeof(int));

    int versek[2];
    read(pipe_id_rec, versek, sizeof(versek));

    const char *vers1 = vers_chose(versek[0]);
    const char *vers2 = vers_chose(versek[1]);

    printf("%s\n", vers1);
    printf("%s\n", vers2);

    int index = rand() % 2;

    write(pipe_id_send, &index, sizeof(int));
    const char *versChosen = vers_chose(versek[index]);

    printf("Ezt választom: %s Szabad-e locsolni?\n\n", versChosen);

    close(pipe_id_rec);
    close(pipe_id_send);

    exit(0);
}

void parentProcess()
{
    mainProcessValue = getpid();

    srand(time(NULL));

    int status;

    int io_pipes[2];
    int succ = pipe(io_pipes);
    if (succ == -1)
    {
        exit(-1);
    }

    int io_pipes1[2];
    int succ1 = pipe(io_pipes1);
    if (succ1 == -1)
    {
        exit(-1);
    }

    int selected_child = rand() % 4 + 1;
    printf("Nyuszi mama: Gyerek %d menj Barátfára!\n", selected_child);

    pid_t child_pid = childProcess(io_pipes[0], io_pipes1[1]);
    signal(Indulj, starthandler);
    while (ready < 1)
        ;
    printf("Gyerek %d úton van.\n", selected_child);

    write(io_pipes[1], &selected_child, sizeof(int));

    int rand1 = generateVersek();
    int rand2 = generateVersek();

    while (rand2 == rand1)
    {
        rand2 = generateVersek();
    }
    int versek[] = {rand1, rand2};

    while (ready < 1)
        ;
    printf("Nyuszi mama: ezek közül válassz!\n");

    write(io_pipes[1], &versek, sizeof(versek));

    int chosen_vers;
    read(io_pipes1[1], &chosen_vers, sizeof(int));

    close(io_pipes[0]);
    close(io_pipes[1]);
    close(io_pipes1[0]);
    close(io_pipes1[1]);
}

int main()
{
    int command;
    char beolvasott_ertek[5];
    int index;

    file_creation();
    id_setup();
    do
    {
        switch (command)
        {
        case '1':
            vers_creation(NULL);
            break;
        case '2':
            vers_list();
            sleep(3);
            break;
        case '3':
            read_console("Hanyadik verset szeretné törölni?\n", beolvasott_ertek);
            index = atoi(beolvasott_ertek);
            vers_delete(index);
            break;
        case '4':
            read_console("Hanyadik verset szeretné módosítani?\n", beolvasott_ertek);
            index = atoi(beolvasott_ertek);
            vers_modify(index);
            break;
        case '5':
            parentProcess();
            break;
        case '6':
            printf("Kilepes\n");
            exit(0);
            break;
        default:
            printf("-----MENU-----\n");
            printf("Vers_felvetel - 1\n");
            printf("Versek_listazasa - 2\n");
            printf("Vers_torlese - 3\n");
            printf("Vers_modositasa - 4\n");
            printf("Locsolas - 5\n");
            printf("Kilep - 6\n\n");
            break;
        }
    } while ((command = getchar()) != EOF);
    return 0;
}