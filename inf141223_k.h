//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Client application header
#ifndef IPC_IM_INF141223_K_H
#define IPC_IM_INF141223_K_H
#define MAX_MSG_SIZE 2137
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>

typedef struct session_t session_t;
typedef struct message_t message_t;

enum msg_type
{
    OUTGOING_TO_USER,
    INCOMING_FROM_USER,
    OUTGOING_TO_GROUP,
    INCOMING_FROM_GROUP,
    LOGIN_REQ,
    LOGOFF_REQ,
    ENLIST_REQ,
    UNLIST_REQ,
    SHOW_GROUPS_REQ,
    SHOW_USER_REQ,
    SHOW_ACTIVE_REQ
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
