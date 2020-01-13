//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Client application source
#include "inf141223_k.h"

bool quit_flag = false;

void quit(int signal)
{
    if(signal == SIGINT)
    {
        printf("\nQuiting the client...\n");
        quit_flag = true;
        exit(EXIT_SUCCESS);
    }
}

void prompt_login(char *credentials)
{
    //TODO: documentation on this
    char login[MAX_MSG_SIZE/2];
    char password[MAX_MSG_SIZE/2 - 1];

    printf("Enter your login: ");
//    scanf("%s", login);
    scan_string(login);
    char *login_buffer = strchr(login, '\n');
    if(login_buffer != NULL)
        *login_buffer = '\0';

    printf("Enter your password: ");
    scan_string(password);
//    scanf("%s", password);

    char *password_buffer = strchr(login, '\n');
    if(password_buffer != NULL)
        *password_buffer = '\0';

    strcat(credentials, login);
    strcat(credentials, ":");
    strcat(credentials, password);
    strcat(credentials, "\0");
}

int login(char *credentials, session_t *session)
{
    printf("Trying to login...\n");
    char from[] = "new";
    message_t *msg = malloc(sizeof(message_t));
    snprintf(msg->from_name, strlen(from), from);
    snprintf(msg->content, strlen(credentials) + 1, "%s", credentials);
    msg->type = PUBLIC_REQ;
    msg->to_id = SERVER_UID;

    int ipc = msgget(SERVER_PUBLIC_IPC_KEY, 0666|IPC_CREAT);
    if(ipc < 0)
    {
        printf("Server is not reachable! [FAILED]\n");
        return -1;
    }

    msgsnd(ipc, msg, sizeof(message_t), 0);
    message_t response;

    printf("Waiting for response... ");
    msgrcv(ipc, &response, sizeof(response), INCOMING, 0);

    char *response_header = strtok(response.content, ":");

    if(strcmp(response_header, "OK") != 0)
    {
        printf("Server response: %s", response.content);
        return -1;
    }

    char *private_ipc_key = strtok(NULL, ":");
    int private_ipc_key_num = (int) strtol(private_ipc_key, (char**) NULL, 10);

    session->ipc_main_thread = msgget(private_ipc_key_num, 0666|IPC_CREAT);
    session->ipc_key = private_ipc_key_num;

    char *uid_str = strtok(NULL, ":");
    int uid = (int) strtol(uid_str, (char**) NULL, 10);

    session->uid = uid;

    char* username = strtok(credentials, ":");
    snprintf(session->username, strlen(username) + 1, "%s", username);

    free(msg);
    return 0;
}

void *refresh_chat(void *vargp)
{
    session_t *session = (session_t*) vargp;
    int ipc = msgget(session->ipc_key, 0666|IPC_CREAT);
    while(!quit_flag)
    {
        message_t msg;
        msgrcv(ipc, &msg, sizeof(msg), INCOMING, 0);
        printf("\n[%s]: %s\n", msg.from_name, msg.content);

        if(strcmp(msg.from_name, "server") == 0 && strcmp(msg.content, "You got disconnected from the server.\n") == 0)
            exit(EXIT_SUCCESS);
        sleep(1);
    }

    msgctl(ipc, IPC_RMID, NULL);
}

void scan_decimal(int *dest)
{
    static char buffer[MAX_MSG_SIZE];
    memset(buffer, 0, strlen(buffer));
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d", dest);
}

void scan_string(char *dest)
{
    size_t max_msg_size = MAX_MSG_SIZE;
//    static char buffer[MAX_MSG_SIZE];
//    memset(buffer, 0, strlen(buffer));
//    fgets(dest, sizeof(dest), stdin);
   // sscanf(buffer, "%s", dest);
   getline(&dest, &max_msg_size, stdin);

   for(int i = 0; i < MAX_MSG_SIZE; i++)
   {
       if(dest[i] == '\n')
       {
           dest[i] = '\0';
           break;
       }
   }
}

void compose_message(session_t *session)
{
    command_t cmd;
    printf("What kind of message do you want to send?\n1) To user 2) To group 3) Server command\nChoice: ");
//    scanf("%d", &cmd);
    scan_decimal(&cmd);

    message_t msg;
    snprintf(msg.from_name, strlen(session->username) + 1, "%s", session->username);

    if(cmd == OUTGOING_TO_USER)
    {
        msg.type = OUTGOING_TO_USER;

        printf("Enter uid: ");
//        scanf("%d", &msg.to_id);
        scan_decimal(&msg.to_id);
        printf("Enter message (max %d characters): ", MAX_MSG_SIZE - 2);
//        scanf("%s", msg.content);
        scan_string(msg.content);
    }
    else if(cmd == OUTGOING_TO_GROUP)
    {
        msg.type = OUTGOING_TO_GROUP;

        printf("Enter gid: ");
//        scanf("%d", &msg.to_id);
        scan_decimal(&msg.to_id);
        printf("Enter message (max %d characters): ", MAX_MSG_SIZE - 2);
//        scanf("%s", msg.content);
        scan_string(msg.content);
    }
    else if(cmd == SERVER_REQ)
    {
        msg.type = SERVER_REQ;
        msg.to_id = SERVER_UID;
        printf("Enter command: ");
//        scanf("%s", msg.content);
        scan_string(msg.content);
    }
    else
    {
        printf("Incorrect command!\n");
        return;
    }
    msgsnd(session->ipc_main_thread, &msg, sizeof(msg), 0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, quit);

    char credentials[MAX_MSG_SIZE];
    *credentials = '\0';
    session_t *session = malloc(sizeof(session_t));

    do
    {
        memset(credentials, 0, sizeof(credentials));
        prompt_login(credentials);
    } while(login(credentials, session) != 0);

    //msgctl(session->ipc, IPC_RMID, NULL);
    printf("Logged in successfully!\n");

    pthread_t receiving_thread;
    pthread_create(&receiving_thread, NULL, refresh_chat, (void *)session);

    while(!quit_flag)
    {
        compose_message(session);
    }

    msgctl(session->ipc_main_thread, IPC_RMID, NULL);
    return 0;
}