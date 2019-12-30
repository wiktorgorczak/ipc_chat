//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Server application source
#include "inf141223_s.h"

bool quit_flag = false;

void quit()
{
    printf("Quiting the server...\n");
    //TODO: clean the ipc's
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    database_t *db = malloc(sizeof(database_t));
    signal(SIGTERM, quit);
    db->users = NULL;
    db->groups = NULL;
    int exit_code = 0;
    printf("Server started. Press CTRL+D to terminate.\n");

    printf("Setting up configuration...");
    if((exit_code = setup(CONFIG, db)) != 0)
    {
        exit(exit_code);
    }

    printf("[OK]\n");
    run(db);
    return EXIT_SUCCESS;
}

void run(database_t *db)
{
    while(!quit_flag)
    {
        user_t *current = db->users;
        do
        {
            receive_messages(current, db);
        } while((current = current->next) != NULL);
        usleep(1000);
    }

    close_all_ipcs(db);
    free(db);
}

void close_all_ipcs(database_t *db)
{
    user_t *current = db->users;
    do
    {
        printf("Removing IPC for user %s... ", current->name);
        if(msgctl(current->ipc, IPC_RMID, NULL) == 0)
        {
            printf("[DONE]\n");
        }
        else
        {
            printf("[FAILED]\n");
        }
    } while((current = db->users->next) != NULL);
}

void receive_messages(user_t *user, database_t *db)
{
    message_t outgoing_to_user, outgoing_to_group, server_request;

    if(msgrcv(user->ipc, &outgoing_to_user, sizeof(outgoing_to_user), OUTGOING_TO_USER, IPC_NOWAIT) != -1)
    {
        user_t *to = find_user(outgoing_to_user.to_id, db);

        send_to_user(user, to, outgoing_to_user.content);
    }
    if(msgrcv(user->ipc, &outgoing_to_group, sizeof(outgoing_to_group), OUTGOING_TO_GROUP, IPC_NOWAIT) != -1)
    {
        group_t *to = find_group(outgoing_to_group.to_id, db);

        send_to_group(user, to, outgoing_to_group.content, db);
    }
    if(msgrcv(user->ipc, &server_request, sizeof(server_request), SERVER_REQ, IPC_NOWAIT) != -1)
    {
        process_request(user, server_request.content, db);
    }
}

void send_to_user(user_t *from, user_t *to, char content[])
{
    int ipc = to->ipc;

    message_t message;
    strcpy(message.content, content);
    message.type = INCOMING;
    //message.from_name = from->name;
    strcpy(message.from_name, from->name);
    message.to_id = to->id;

    printf("Sending a message from %s to %s... ", from->name, to->name);

    if(msgsnd(ipc, &message, sizeof(message), 0) == -1)
    {
        printf("[FAILED]\n");
        send_server_msg(from, "Could not send your message");
    } else
    {
        printf("[DONE]\n");
    }
}

void send_to_group(user_t *from, group_t *to, char content[], database_t *db)
{
    ListElement *current = to->users;
    printf("Sending message to a group...\n");
    do
    {
        user_t *current_user = find_user(current->value, db);
        if(current_user != NULL)
        {
            send_to_user(from, current_user, content);
        }
    } while((current = current->next) != NULL);
}

//void send_server_msg()



void send_server_msg(user_t *user, const char content[])
{
    int ipc = user->ipc;

    message_t response;
    response.type = INCOMING;
    response.to_id = user->id;

    //response.from_name = SERVER_USR_NAME;
    strcpy(response.from_name, SERVER_USR_NAME);

    strcpy(response.content, content);

    msgsnd(ipc, &response, sizeof(response), 0);
}

void process_request(user_t *user, char request[], database_t *db)
{
    char *req = strtok(request, " ");
    char *param = strtok(NULL, " ");

    if(strcmp(req, LOGOFF_REQ) == 0)
    {
        logoff(user);
    }
    else if(strcmp(req, GET_ACTIVE_REQ) == 0)
    {
        send_server_msg(user, get_active_users(user, db));
    }
    else if(strcmp(req, GET_USERS_REQ) == 0)
    {
        if(param == NULL)
        {
            send_server_msg(user, "You should provide exactly one argument.\n");
            return;
        }
        group_t *group = find_group(atoi(param), db);
        if(group == NULL)
        {
            send_server_msg(user, "Could not find selected group.\n");
            return;
        }
        send_server_msg(user, get_users_for_group(user, group, db));
    }
    else if(strcmp(req, GET_GROUPS_REQ) == 0)
    {
        send_server_msg(user, get_all_groups(db));
    }
    else if(strcmp(req, GET_ENLISTED_REQ) == 0)
    {
        send_server_msg(user, get_groups_for_user(user, db));
    }
    else if(strcmp(req, ENLIST_REQ) == 0)
    {
        if(param == NULL)
        {
            send_server_msg(user, "You should provide exactly one argument.\n");
            return;
        }
        group_t *group = find_group(atoi(param), db);
        if(group == NULL)
        {
            send_server_msg(user, "Could not find selected group.\n");
            return;
        }
        send_server_msg(user, enlist(user, group));
    }
    else if(strcmp(req, UNLIST_REQ) == 0)
    {
        if(param == NULL)
        {
            send_server_msg(user, "You should provide exactly one argument.\n");
            return;
        }
        group_t *group = find_group(atoi(param), db);
        if(group == NULL)
        {
            send_server_msg(user, "Could not find selected group.\n");
            return;
        }
        send_server_msg(user, unlist(user, group));
    }
    else
    {
        send_server_msg(user, "Unknown request type.\n");
    }
}

int setup(char filename[], database_t *db)
{
    printf("Setting up configuration...\n");
    FILE *fp;
    char *line;
    size_t len;
    fp = fopen(filename, "r");
    printf("Opening file... ");
    if(fp == NULL)
    {
        printf("[FAILED]\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("[DONE]\n");
    }

    while(getline(&line, &len, fp) > 0)
    {
        char *key_id = strtok(line, ":");
        char *id = strtok(NULL, ":");
        char *key_name = strtok(NULL, ":");
        char *name = strtok(NULL, ":");

        if(strcmp(key_id, "GID") == 0)
        {
            group_t *group = malloc(sizeof(group_t));
            group->id = atoi(id);
            if(strcpy("NAME", key_name) == 0)
            {
                strcpy(group->name, name);
            }
            else
            {
                printf("Parsing error!\n");
                return EXIT_FAILURE;
            }

            if(db->groups == NULL)
            {
                db->groups = group;
            }
            else
            {
                group_t *tail = db->groups;
                db->groups = group;
                group->next = tail;
            }
        }
        else if(strcmp(key_id, "UID") == 0)
        {
            char *groups = strtok(NULL, ":");

            user_t *user = malloc(sizeof(user_t));
            user->id = atoi(id);

            if(strcmp("NAME", key_name) == 0)
            {
                strcpy(user->name, name);
            }
            else
            {
                printf("Parsing error! Config file structure is not correct.\n");
                return EXIT_FAILURE;
            }

            char *gid_str = strtok(groups, ",");
            if(gid_str != NULL)
            {
                do
                {
                    int gid = atoi(gid_str);
                    group_t *group = find_group(gid, db);
                    if(group != NULL)
                    {
                        insertListElement(user->groups, gid);
                        insertListElement(group->users, user->id);
                    }
                    else
                    {
                        printf("No such group error!\n");
                    }
                } while((gid_str = strtok(NULL, ",")) != NULL);
            }

            int ipc = create_ipc_for_user(user);
            if(ipc == -1)
            {
                printf("Could not create IPC for user %s!\n", user->name);
                return EXIT_FAILURE;
            }

            user->ipc = ipc;
            user->connected = false;

            if(db->users == NULL)
            {
                db->users = user;
            }
            else
            {
                user_t *tail = db->users;
                db->users = user;
                user->next = tail;
            }

            char* password = strtok(NULL, ":");
            strcpy(user->password, password);
        }
        else
        {
            printf("Parsing error!\n");
            return EXIT_FAILURE;
        }
    }

    fclose(fp);
    if(line)
    {
        free(line);
    }

    return EXIT_SUCCESS;
}

user_t *find_user(int uid, database_t *db)
{
    if(db->users == NULL)
    {
        printf("There are no registered users.\n");
        return NULL;
    }

    user_t *current = db->users;
    do
    {
        if (current->id == uid)
        {
            return current;
        }
    } while((current = current->next) != NULL);

    return NULL;
}

group_t *find_group(int gid, database_t *db)
{
    if(db->groups == NULL)
    {
        printf("There are no registered groups.\n");
        return NULL;
    }

    group_t *current = db->groups;
    do
    {
        if(current->id == gid)
        {
            return current;
        }
    } while((current = current->next) != NULL);

    return NULL;
}

int create_ipc_for_user(user_t *user)
{
    if(user == NULL)
    {
        return -1;
    }

    key_t key = KEY + user->id;
    return msgget(key, 0666 | IPC_CREAT);
}

const char* enlist(user_t *user, group_t *group)
{
    int uid = user->id;
    int gid = group->id;

    if(searchList(group->users, uid) != NULL)
    {
        const char response_text[] = "You are already a member of this group!";
        return response_text;
    }
    else
    {
        insertListElement(group->users, uid);
        insertListElement(user->groups, gid);

        const char response_text[] = "You have successfully become a member of this group!";
        return response_text;
    }
}

const char* unlist(user_t *user, group_t *group)
{
    int uid = user->id;
    int gid = group->id;

    ListElement *usr = searchList(group->users, uid);
    ListElement *grp = searchList(user->groups, gid);
    if(usr == NULL || grp == NULL)
    {
        const char response_text[] = "You are not a member of this group!";
        return response_text;
    }
    else
    {
        removeElement(user->groups, gid);
        removeElement(group->users, uid);
        const char response_text[] = "You have successfully become a member of this group!";
        return response_text;
    }
}

const char* get_groups_for_user(user_t *user, database_t *db)
{
    ListElement *gid = user->groups;
    char groups_text[MAX_MSG_SIZE];
    while(gid != NULL)
    {
        group_t *grp = find_group(gid->value, db);
        if(grp == NULL)
        {
            printf("Database contains incorrect data!\n");
        }
        else
        {
            strcat(groups_text, grp->name);
            strcat(groups_text, ", ");
        }
        gid = gid->next;
    }

    return groups_text;
}

const char* get_all_groups(database_t *db)
{
    group_t *grp = db->groups;
    char groups_text[MAX_MSG_SIZE];

    do
    {
        strcat(groups_text, grp->name);
        strcat(groups_text, ", ");
    } while((grp = grp->next) != NULL);

    return groups_text;
}

const char* get_users_for_group(user_t *user, group_t *group, database_t *db)
{
    ListElement *uid = group->users;
    char users_text[MAX_MSG_SIZE];
    while(uid != NULL)
    {
        user_t *usr = find_user(uid->value, db);
        if(uid == NULL)
        {
            printf("Database contains incorrect data!\n");
        }
        else
        {
            strcat(users_text, usr->name);
            strcat(users_text, ", ");
        }
        uid = uid->next;
    }

    return users_text;
}

const char* get_active_users(user_t *user, database_t *db)
{
    user_t *usr = db->users;
    char users_text[MAX_MSG_SIZE];

    do
    {
        if(usr->connected)
        {
            strcat(users_text, usr->name);
            strcat(users_text, ", ");
        }

    } while((usr = usr->next) != NULL);

    return users_text;
}

const char* logoff(user_t *user)
{
    user->connected = false;
    char response[] = "You got disconnected from the server.\n";
    return response;
}



