//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Client application header
#ifndef IPC_IM_INF141223_K_H
#define IPC_IM_INF141223_K_H
#define MAX_MSG_SIZE 2137
#define MAX_USR_NAME_SIZE 2137
#define MAX_GROUP_NAME_SIZE 2137
#define MAX_PASSWORD_SIZE 2137
#define SERVER_UID 0
#define SERVER_PUBLIC_IPC_KEY 100
#define SERVER_USR_NAME "server"
#define PUBLIC_USER_NAME "server"
#define PUBLIC_UID 0
#define SHARED_MEM_CONST 12345
#include <sys/shm.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

typedef struct session_t session_t;
typedef struct message_t message_t;
typedef int command_t;

enum msg_type
{
    OUTGOING_TO_USER = 1,
    OUTGOING_TO_GROUP,
    SERVER_REQ,
    INCOMING,
    PUBLIC_REQ
};

struct message_t
{
    long type;
    char from_name[MAX_USR_NAME_SIZE];
    int to_id;
    char content[MAX_MSG_SIZE];
};

struct session_t
{
    int ipc_key;
    int ipc_main_thread;
    int uid;
    char username[MAX_USR_NAME_SIZE];
};

void prompt_login(char *credentials);
//void *refresh_chat(void *vargp);
void refresh_chat(session_t *session);
void compose_message(session_t *session);
void quit(int signal);

void scan_string(char *dest);
void scan_decimal(int *dest);

int login(char *credentials, session_t *session);

#endif //IPC_IM_INF141223_K_H
