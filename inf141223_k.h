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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>

typedef struct session_t session_t;
typedef struct message_t message_t;

enum msg_type
{
    OUTGOING_TO_USER,
    OUTGOING_TO_GROUP,
    SERVER_REQ,
    INCOMING,
    PUBLIC_REQ
};

struct message_t
{
    long type;
    //int from_uid;
    char from_name[MAX_USR_NAME_SIZE];
    int to_id;
    char content[MAX_MSG_SIZE];
};

struct session_t
{
    int ipc;
    int uid;
};

struct message_t
{
    int type;
    char content[MAX_MSG_SIZE];
};

int setup(char filename[], session_t *session);
int run(session_t *session);
int login(session_t *session);
int logoff(session_t *session);
int get_active_users(session_t *session);
int get_users_for_group(int gid, session_t *session);
int get_groups(session_t *session);
int enlist(int gid, session_t *session);
int unlist(int gid, session_t *session);

int send_message_to_user(int uid, char message[], session_t *session);
int send_message_to_group(int gid, char message[], session_t *session);

int receive(session_t *session);

#endif //IPC_IM_INF141223_K_H
