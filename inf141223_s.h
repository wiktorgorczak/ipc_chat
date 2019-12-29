//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Server application header
#ifndef IPC_IM_INF141223_S_H
#define IPC_IM_INF141223_S_H
#define MAX_MSG_SIZE 2137
#define MAX_USR_NAME_SIZE 2137
#define MAX_GROUP_NAME_SIZE 2137
#define SERVER_USR_NAME "server"
#define MSG_TYPE 2137
#define CONFIG "server.ini"
#define KEY 2137
#include "data_structures/linked_list.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

typedef struct user_t user_t;
typedef struct group_t group_t;
typedef struct database_t database_t;
typedef struct message_t message_t;

enum msg_type
{
    OUTGOING_TO_USER,
    OUTGOING_TO_GROUP,
    SERVER_REQ,
    INCOMING
};

struct message_t
{
    long type;
    int from_uid;
    int to_id;
    char content[MAX_MSG_SIZE];
};

struct user_t
{
    int id, ipc;
    bool connected;
    char name[MAX_USR_NAME_SIZE];
    ListElement *groups;
    user_t *next;
};

struct group_t
{
    int id;
    char name[MAX_GROUP_NAME_SIZE];
    ListElement *users;
    group_t *next;
};

struct database_t
{
    struct user_t *users;
    struct group_t *groups;
};

int setup(char filename[], database_t *db);
void run(database_t *db);

user_t *find_user(int uid, database_t *db);
group_t *find_group(int gid, database_t *db);
int create_ipc_for_user(user_t *user);
void receive_messages(user_t *user, database_t *db);

void process_request(user_t *from, char request[], database_t *db);
void send_to_user(user_t *from, user_t *to, char content[]);
void send_to_group(user_t *from, group_t *to, char content[]);
int enlist(user_t *user, group_t *group, database_t *db);
int unlist(user_t *user, group_t *group, database_t *db);
int get_groups_for_user(user_t *user, database_t *db);
int get_users_for_group(user_t *user, group_t *group, database_t *db);
int get_active_users(user_t *user, database_t *db);
int login(user_t *user, database_t *db);
int logoff(user_t *user, database_t *db);
void log_err(char message[]);
void log_msg(char message[]);

#endif //IPC_IM_INF141223_S_H
