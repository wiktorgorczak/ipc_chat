//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Client application source
#include "inf141223_k.h"

void prompt_login(char *credentials)
{
    //TODO: documentation about this
    char login[MAX_MSG_SIZE/2];
    char password[MAX_MSG_SIZE/2 - 1];

    printf("Enter your login: ");
    scanf("%s", login);
    char *login_buffer = strchr(login, '\n');
    if(login_buffer != NULL)
        *login_buffer = '\0';

    printf("Enter your password: ");
    scanf("%s", password);

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
    snprintf(msg->content, strlen(credentials) + 1, "%s", credentials); //ooohhh plaintext! but really... would you seriously use some basic student app?
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

    session->ipc = msgget(private_ipc_key_num, 0666|IPC_CREAT);

    char *uid_str = strtok(NULL, ":");
    int uid = (int) strtol(uid_str, (char**) NULL, 10);

    session->uid;

    free(msg);
    return 0;
}

int main(int argc, char **argv)
{
    char credentials[MAX_MSG_SIZE];
    *credentials = '\0';
    session_t *session = malloc(sizeof(session_t));

    do
    {
        memset(credentials, 0, sizeof(credentials));
        prompt_login(credentials);
    } while(login(credentials, session) != 0);

    msgctl(session->ipc, IPC_RMID, NULL);
    printf("Logged in successfully!\n");
    return 0;
}