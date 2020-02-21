//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Server application source
#include "inf141223_s.h"

bool quit_flag = false;

void quit(int signal)
{
    if(signal == SIGINT)
    {
        printf("Quiting the server...\n");
        //TODO: clean the ipc's
        quit_flag = true;
        //exit(EXIT_SUCCESS);
    }
}

int main(int argc, char* argv[])
{
//    printf("Expected size: %d", sizeof(message_t));
    database_t *db = malloc(sizeof(database_t));
    signal(SIGINT, quit);
    db->users = NULL;
    db->groups = NULL;
    int exit_code = 0;
    printf("Server started. Press CTRL+C to terminate.\n");

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
    user_t *current = db->users;
//    do
//    {
//        run_thread_for_user(db, current);
//    } while((current = current->next) != NULL);

    while(!quit_flag) {
        receive_login_req(db);
    }

    finish_all_threads(db);
    close_server_ipc(db);
}

void run_thread_for_user(database_t *db, user_t *user)
{
//    pthread_mutex_lock(&(db->mutex));
    thread_arg_t *args = malloc(sizeof(thread_arg_t));
    args->user = user;
    args->db = db;

    pthread_t thread;
    pthread_create(&thread, NULL, user_thread, (void*) args);

    user->thread = thread;
//    pthread_mutex_unlock(&(db->mutex));

}

void finish_user_thread(user_t *user)
{
    int *exit_code;
    pthread_join(user->thread, (void**) &exit_code);
    printf("Finished thread for user %s.\n", user->name);
}

void finish_all_threads(database_t *db)
{
    pthread_mutex_unlock(&db->mutex);

    user_t *current = db->users;
    do
    {
        if(current->connected)
            finish_user_thread(current);
    } while((current = current->next) != NULL);

    pthread_mutex_lock(&db->mutex);
}

void close_server_ipc(database_t *db)
{
    printf("Removing IPC for server... ");
    if(msgctl(db->public_user->ipc, IPC_RMID, NULL) == 0)
    {
        printf("[DONE]\n");
    }
    else
    {
        printf("[FAILED]\n");
    }
}

void *user_thread(void *vargp)
{
    thread_arg_t *args = (thread_arg_t*) vargp;
    user_t *user = args->user;
    database_t *db = args->db;

    printf("Thread for user %s started.\n", user->name);
    while(!quit_flag) {
        receive_messages(user, db);
        usleep(1000);
    }

    close_ipc(user, db);
    free(vargp);

    pthread_exit(NULL);
}

void receive_login_req(database_t *db)
{
    message_t login_req;
    char response[MAX_MSG_SIZE];
    if(msgrcv(db->public_user->ipc, &login_req, sizeof(login_req), PUBLIC_REQ, IPC_NOWAIT) != -1)
    {
        pthread_mutex_lock(&(db->mutex));
        char* username = strtok(login_req.content, ":");
        char* password = strtok(NULL, ":");

        login(username, password, db, response);
        send_server_msg(db->public_user, response);
        pthread_mutex_unlock(&(db->mutex));
    }
}

void login(const char* username, const char* password, database_t *db, char* response)
{
    user_t *current = db->users;
    do
    {
        if(strcmp(current->name, username) == 0 && strcmp(current->password, password) == 0)
        {
            int key = KEY + current->id;
            sprintf(response, "OK:%d:%d", key, current->id);
            run_thread_for_user(db, current);
            current->connected = true;

            printf("User logged in!");

            return;
        }
    } while((current = current->next) != NULL);
    sprintf(response, "Incorrect login or/and password!\n");
}

void close_ipc(user_t *user, database_t *db) {
    printf("Removing IPC for user %s... ", user->name);
    if(msgctl(user->ipc, IPC_RMID, NULL) == 0)
    {
        printf("[DONE]\n");
    }
    else
    {
        printf("[FAILED]\n");
    }
}

void receive_messages(user_t *user, database_t *db)
{
    static message_t outgoing_to_user, outgoing_to_group, server_request;

    if(msgrcv(user->ipc, &outgoing_to_user, sizeof(message_t), OUTGOING_TO_USER, IPC_NOWAIT) != -1)
    {
        pthread_mutex_lock(&(db->mutex));

        printf("Got user to user massage!\n");
        user_t *to = find_user(outgoing_to_user.to_id, db);

        send_to_user(user, to, outgoing_to_user.content);

        pthread_mutex_unlock(&(db->mutex));
    }
    if(msgrcv(user->ipc, &outgoing_to_group, sizeof(message_t), OUTGOING_TO_GROUP, IPC_NOWAIT) != -1)
    {
        pthread_mutex_lock(&(db->mutex));

        printf("Got user to group message!\n");
        group_t *to = find_group(outgoing_to_group.to_id, db);

        send_to_group(user, to, outgoing_to_group.content, db);

        pthread_mutex_unlock(&(db->mutex));
    }
    if(msgrcv(user->ipc, &server_request, sizeof(message_t), SERVER_REQ, IPC_NOWAIT) != -1)
    {
        pthread_mutex_lock(&(db->mutex));

        printf("Got server request!\n");
        process_request(user, server_request.content, db);

        pthread_mutex_unlock(&(db->mutex));
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
    char response[MAX_MSG_SIZE];

    if(strcmp(req, LOGOFF_REQ) == 0)
    {
        logoff(user, response);
        send_server_msg(user,response);
    }
    else if(strcmp(req, GET_ACTIVE_REQ) == 0)
    {
        get_active_users(db, response);
        send_server_msg(user, response);
    }
    else if(strcmp(req, GET_USERS_REQ) == 0)
    {
        if(param == NULL)
        {
            send_server_msg(user, "You should provide exactly one argument.\n");
            return;
        }

        int param_num = (int) strtol(param, (char**) NULL, 10);
        group_t *group = find_group(param_num, db);

        if(group == NULL)
        {
            send_server_msg(user, "Could not find selected group.\n");
            return;
        }
        get_users_for_group(group, db, response);
        send_server_msg(user, response);
    }
    else if(strcmp(req, GET_GROUPS_REQ) == 0)
    {
        get_all_groups(db, response);
        send_server_msg(user, response);
    }
    else if(strcmp(req, GET_ENLISTED_REQ) == 0)
    {
        get_groups_for_user(user, db, response);
        send_server_msg(user, response);
    }
    else if(strcmp(req, ENLIST_REQ) == 0)
    {
        if(param == NULL)
        {
            send_server_msg(user, "You should provide exactly one argument.\n");
            return;
        }
        int param_num = (int) strtol(param, (char**) NULL, 10);
        group_t *group = find_group(param_num, db);
        if(group == NULL)
        {
            send_server_msg(user, "Could not find selected group.\n");
            return;
        }
        enlist(user, group, response);
        send_server_msg(user, response);
    }
    else if(strcmp(req, UNLIST_REQ) == 0)
    {
        if(param == NULL)
        {
            send_server_msg(user, "You should provide exactly one argument.\n");
            return;
        }

        int param_num = (int) strtol(param, (char**) NULL, 10);
        group_t *group = find_group(param_num, db);
        if(group == NULL)
        {
            send_server_msg(user, "Could not find selected group.\n");
            return;
        }
        unlist(user, group, response);
        send_server_msg(user, response);
    }
    else
    {
        send_server_msg(user, "Unknown request type.\n");
    }
}

void get_all_lines(char **lines, int *nlines, int fd)
{
    char buffer[MAX_BUFFER_SIZE];
    read(fd, buffer, MAX_BUFFER_SIZE);
    int len = 0;

    for(int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        if(buffer[i] == '\0')
            break;
        else
            len++;
    }

    split_by(buffer, len, lines, nlines, '\n');
}

void split_by(char *src, int src_len, char **words, int *nwords, char delimiter)
{
    char buffer[MAX_MSG_SIZE];
    char current_character_buffer;
    (*nwords) = 0;
    int characters_iterator = 0;

    words[0] = malloc(MAX_MSG_SIZE * sizeof(char));

    for(int i = 0; i < src_len; i++)
    {
        current_character_buffer = src[i];
        if(!(current_character_buffer == delimiter || current_character_buffer == '\0'))
        {
            buffer[characters_iterator] = current_character_buffer;
            characters_iterator++;
        } else
        {
            strcpy(words[*nwords], buffer);
            (*nwords)++;
            characters_iterator = 0;
            memset(buffer, 0, MAX_MSG_SIZE);

            if(current_character_buffer != '\0')
            {
                words[*nwords] = malloc(MAX_MSG_SIZE * sizeof(char));
            }
        }
    }
}

char* find_char(char *line, int len, char chr)
{
    if(line != NULL)
    {
        for(int i = 0; i < len; i++)
        {
            if (line[i] == chr || line[i] == '\0')
                return line + i;
        }
    }

    return NULL;
}

int str_len(char *str)
{
    int i = 0;
    while(str[i++] != '\0');
    return i;
}

int setup(char filename[], database_t *db)
{
    printf("Setting up configuration...\n");
    int fp;
    char* lines[MAX_BUFFER_SIZE];
    int nlines = 0;
    size_t len;
    fp = open(filename, O_RDONLY);
    printf("Opening file... ");
    if(fp == -1)
    {
        printf("[FAILED]\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("[DONE]\n");
    }

    get_all_lines(lines, &nlines, fp);

    for(int i = 0; i < nlines; i++)
    {
        //char *formatted_line = strchr(line, '\n');
        char formatted_line[MAX_BUFFER_SIZE];
        strcpy(formatted_line, lines[i]);
        //char *buffer = strchr(formatted_line, '\n');
        char *buffer = find_char(formatted_line, MAX_BUFFER_SIZE, '\n');
        if(buffer != NULL)
            *buffer = '\0';

        int properties_counter = 0;
        char *properties[PROPERTIES_COUNT];
        split_by(formatted_line, buffer - formatted_line + 1, properties, &properties_counter, ':');
//        char *key_id = strtok(formatted_line, ":");
//        char *id = strtok(NULL, ":");
//        char *key_name = strtok(NULL, ":");
//        char *name = strtok(NULL, ":");
        char *key_id = properties[0];
        char *id = properties[1];
        char *key_name = properties[2];
        char *name = properties[3];


        if(strcmp(key_id, "GID") == 0)
        {
            group_t *group = malloc(sizeof(group_t));
            group->id = (int) strtol(id, (char**) NULL, 10);
            if(strcmp("NAME", key_name) == 0)
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
            char* password = properties[4];
            char *groups = properties[5];

            user_t *user = malloc(sizeof(user_t));
            user->id = (int) strtol(id, (char**) NULL, 10);

            strcpy(user->password, password);
//            if(user->id == PUBLIC_UID)
//            {
//                printf("ERROR: You cannot user public uid for private account!\n");
//                return(EXIT_FAILURE);
//            }

            if(strcmp("NAME", key_name) == 0)
            {
                strcpy(user->name, name);
            }
            else
            {
                printf("Parsing error! Config file structure is not correct.\n");
                return EXIT_FAILURE;
            }

            int group_count = 0;
            int gid_str_len = str_len(groups);
            char *groups_formatted[MAX_BUFFER_SIZE];

            split_by(groups, gid_str_len, groups_formatted, &group_count, ',');
            //char *gid_str = strtok(groups, ",");

            for(int k = 0; k < group_count; k++)
            {
                int gid = (int) strtol(groups_formatted[k], (char**) NULL, 10);
                group_t *group = find_group(gid, db);
                if(group != NULL)
                {
                    user->groups = insertListElement(user->groups, gid);
                    group->users = insertListElement(group->users, user->id);
                }
                else
                {
                    printf("No such group error!\n");
                }

            }


            int ipc = create_ipc_for_user(user);
            if(ipc == -1)
            {
                printf("Could not create IPC for user %s!\n", user->name);
                return EXIT_FAILURE;
            }

            user->ipc = ipc;
            user->connected = false;
            user->blocked = false;

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
            printf("Parsing error!\n");
            return EXIT_FAILURE;
        }

        memset(formatted_line, 0, MAX_BUFFER_SIZE);
    }

    close(fp);
    printf("Creating public IPC... ");

    if(setup_public_user(db) != 0)
    {
        return EXIT_FAILURE;
    }

    if((db->public_user->ipc = create_ipc_for_user(db->public_user)) == -1)
    {
        printf("[FAILED]\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("[DONE]\n");
    }

    pthread_mutex_init(&(db->mutex), NULL);

    return EXIT_SUCCESS;
}

int setup_public_user(database_t *db)
{
    user_t *usr = malloc(sizeof(user_t));

    usr->id = PUBLIC_UID;
    strcpy(usr->name, PUBLIC_USER_NAME);
    usr->connected = true;

    db->public_user = usr;

    return 0;
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

    key_t key;
    if(user->id == PUBLIC_UID)
    {
        key = SERVER_PUBLIC_IPC_KEY;
    }
    else
    {
        key = KEY + user->id; //This should some complicated random algorithm
    }
    return msgget(key, 0666 | IPC_CREAT); // NOLINT(hicpp-signed-bitwise)
}

void enlist(user_t *user, group_t *group, char* response)
{
    int uid = user->id;
    int gid = group->id;

    if(searchList(group->users, uid) != NULL)
    {
        const char response_text[] = "You are already a member of this group!";
        snprintf(response, strlen(response_text) + 1, response_text);
    }
    else
    {
        group->users = insertListElement(group->users, uid);
        user->groups = insertListElement(user->groups, gid);

        const char response_text[] = "You have successfully become a member of this group!";
        snprintf(response, strlen(response_text) + 1, response_text);
    }
}

void unlist(user_t *user, group_t *group, char* response)
{
    int uid = user->id;
    int gid = group->id;

    ListElement *usr = searchList(group->users, uid);
    ListElement *grp = searchList(user->groups, gid);
    if(usr == NULL || grp == NULL)
    {
        const char response_text[] = "You are not a member of this group!";
        snprintf(response, strlen(response_text) + 1, response_text);
    }
    else
    {
        user->groups = removeElement(user->groups, gid);
        group->users = removeElement(group->users, uid);
        const char response_text[] = "You have successfully become a member of this group!";
        snprintf(response, strlen(response_text) + 1, response_text);
    }
}

void get_groups_for_user(user_t *user, database_t *db, char* response)
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
            char id_str[100];
            sprintf(id_str, "ID: %d ", grp->id);
            strcat(groups_text, id_str);

            strcat(groups_text, grp->name);
            strcat(groups_text, ", ");
        }
        gid = gid->next;
    }
    snprintf(response, strlen(groups_text) + 1, "%s", groups_text);

}

void get_all_groups(database_t *db, char* response)
{
    group_t *grp = db->groups;
    char groups_text[MAX_MSG_SIZE];

    do
    {
        char id_str[100];
        sprintf(id_str, "ID: %d ", grp->id);
        strcat(groups_text, id_str);

        strcat(groups_text, grp->name);
        strcat(groups_text, ", ");
    } while((grp = grp->next) != NULL);

    snprintf(response, strlen(groups_text) + 1, "%s", groups_text);
}

void get_users_for_group(group_t *group, database_t *db, char* response)
{
    ListElement *uid = group->users;
    char users_text[MAX_MSG_SIZE];
    while(uid != NULL)
    {
        user_t *usr = find_user(uid->value, db);
        if(usr == NULL)
        {
            printf("Database contains incorrect data!\n");
        }
        else
        {
            char uid_str[100];
            sprintf(uid_str, "ID: %d ", usr->id);
            strcat(users_text, uid_str);
            strcat(users_text, usr->name);
            strcat(users_text, ", ");
        }
        uid = uid->next;
    }

    snprintf(response, strlen(users_text) + 1, "%s", users_text);

}

void get_active_users(database_t *db, char* response)
{
    user_t *usr = db->users;
    char users_text[MAX_MSG_SIZE];

    do
    {
        if(usr->connected)
        {
            char uid_str[100];
            sprintf(uid_str, "ID: %d ", usr->id);
            strcat(users_text, uid_str);

            strcat(users_text, usr->name);
            strcat(users_text, ", ");
        }

    } while((usr = usr->next) != NULL);

    snprintf(response, strlen(users_text) + 1,"%s", users_text);

}

void logoff(user_t *user, char* response)
{
    user->connected = false;
    finish_user_thread(user);
    char response_text[] = "You got disconnected from the server.\n";
    snprintf(response, strlen(response_text) + 1, "%s", response_text);
}

