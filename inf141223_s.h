//
// Created by wiktor on 22.12.2019.
//

#ifndef IPC_IM_INF141223_S_H
#define IPC_IM_INF141223_S_H
#define MAX_MSG_SIZE 2137
#define MAX_USR_NAME_SIZE 2137
#define MAX_GROUP_NAME_SIZE 2137
#include "data_structures/linked_list.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
typedef struct user_t user_t;
typedef struct group_t group_t;
typedef struct database_t database_t;
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

struct message_t
{
    int type;
    char content[MAX_MSG_SIZE];
};

struct user_t
{
    int id, ipc;
    bool connected;
    char name[MAX_USR_NAME_SIZE];
    ListElement* groups;
};

struct group_t
{
    int id;
    char name[MAX_GROUP_NAME_SIZE];
    ListElement *users;
};

struct database_t
{
    struct user_t *users;
    struct groups_t *groups;
};

int setup(char filename[], database_t *db);
int run(database_t *db);
int parse_msg(char message[], database_t *db);
user_t *find_user(int uid, database_t *db);
group_t *find_group(int gid, database_t *db);

int send_to_user(user_t *from, user_t *to, char message[], database_t *db);
int send_to_group(user_t *from, group_t *to, char message[], database_t *db);
int enlist(user_t *user, group_t *group, database_t *db);
int unlist(user_t *user, group_t *group, database_t *db);
int get_groups_for_user(user_t *user, database_t *db);
int get_users_for_group(user_t *user, group_t *group, database_t *db);
int get_active_users(user_t *user, database_t *db);
int login(user_t *user, database_t *db);
int logoff(user_t *user, database_t *db);
void log_err(char message[], database_t *db);
void log_msg(char message[], database_t *db);

#endif //IPC_IM_INF141223_S_H
