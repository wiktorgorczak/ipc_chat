//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Server application source
#include "inf141223_s.h"

void quit()
{
    printf("Quiting the server...\n");
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
}

void run(database_t *db)
{
    while(1)
    {
        user_t *current = db->users;
        do
        {
            receive_messages(current, db);
        } while((current = current->next) != NULL);
        usleep(1000);
    }

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

        send_to_group(user, to, outgoing_to_group.content);
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
    message.from_uid = from->id;
    message.to_id = to->id;

    if(msgsnd(ipc, &message, sizeof(message), 0) == -1)
    {
        log_err("Message could not be send.");
    }
}

void send_to_group(user_t *from, group_t *to, char content[])
{
    user_t *current = to->users;
    do
    {
        send_to_user(from, current, content);
    } while((current = current->next) != NULL);
}

void process_request(user_t *user, char request[], database_t *db)
{
    //REQUEST TEMPLATE
}

int setup(char filename[], database_t *db)
{

    FILE *fp;
    char *line;
    size_t len;
    fp = fopen(filename, "r");
    if(fp == NULL)
    {
        log_err("Cannot open config file");
        return EXIT_FAILURE;
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
                log_err("Parsing error");
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

            if(strcpy("NAME", key_name) == 0)
            {
                strcpy(user->name, name);
            }
            else
            {
                log_err("Parsing error");
                return EXIT_FAILURE;
            }

            char *gid_str;
            while((gid_str = strtok(NULL, ",")) != NULL)
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
                    log_err("Trying to enlist user to non-existing group.");
                }
            }

            int ipc = create_ipc_for_user(user);
            if(ipc == -1)
            {
                log_err("Error while setting up IPC!");
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
        }
        else
        {
            log_err("Error while parsing config file!");
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
        log_err("There are no registered users.");
        return NULL;
    }

    user_t *current = db->users;
    do
    {
        if (current->id == uid)
        {
            return current;
        }
    } while((current = db->users->next) != NULL);
}

group_t *find_group(int gid, database_t *db)
{
    if(db->groups == NULL)
    {
        log_err("There are no registered groups.");
        return NULL;
    }

    group_t *current = db->groups;
    do
    {
        if(current->id == gid)
        {
            return current;
        }
    } while((current = db->users->next) != NULL);
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