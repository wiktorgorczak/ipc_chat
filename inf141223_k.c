//
// Student project for Concurrent Programming class.
// Student id: 141223
// Wiktor Gorczak 2020
//
// Client application source
#include "inf141223_k.h"

int main(int argc, char **argv)
{
    printf("Hello!");
    char content[] = "patrycja:chlopjestnajlertrepszy";
    char from[] = "new";
    message_t msg;
    strcpy(msg.content, content);
    strcpy(msg.from_name, from);
    msg.type = PUBLIC_REQ;
    msg.to_id = SERVER_UID;

    int ipc = msgget(SERVER_PUBLIC_IPC_KEY, 0666|IPC_CREAT);
    printf("IPC: %d\n", ipc);
    msgsnd(ipc, &msg, sizeof(msg), 0);
    message_t response;

    msgrcv(ipc, &response, sizeof(response), INCOMING, 0);
    printf("Response: %s", response.content);
    return 0;
}