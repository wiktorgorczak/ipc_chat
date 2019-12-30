// LinkedList implementation for Algorithms and Data Structeres classes
// Filip Szostak (141320) and Wiktor Gorczak (141223)

#ifndef IPC_IM_LINKED_LIST_H
#define IPC_IM_LINKED_LIST_H
#include <stdio.h>
#include <stdlib.h>
typedef struct ListElement ListElement;

struct ListElement {
    int value;
    ListElement* next;
};

ListElement *insertListElement(ListElement* start, int value) {

    ListElement* newElement = (ListElement*) malloc(sizeof(ListElement));
    newElement->value = value;
    newElement->next = NULL;

    if(!start) {
        start = newElement;
        return newElement;
    }

    if(value < start->value) {
        newElement->next = start;
        start = newElement;
        return newElement;
    }

    ListElement* lastElement = start;
    ListElement* currentElement = start->next;
    while(currentElement != NULL) {
        if(currentElement->value > value) {
            newElement->next = currentElement;
            lastElement->next = newElement;
            break;
        }
        lastElement = currentElement;
        currentElement = currentElement->next;
    }

    return start;
}

ListElement *searchList(ListElement *start, int value) {

    ListElement *currentElement = start;
    while(currentElement != NULL &&currentElement->value != value)
    {
        currentElement= currentElement-> next;
    }
    return currentElement;

}

void deleteList(ListElement *start) {
    ListElement *currentElement = start;

    while(currentElement != NULL)
    {
        start = currentElement->next;
        free(currentElement);
        currentElement=start;
    }

}

ListElement* removeElement(ListElement *start, int value) {
    ListElement *before = start;
    ListElement *current;

    if(before == NULL) {
        return NULL;
    } else if(before->value == value){
        ListElement *newStart =  before->next;
        free(before);
        return newStart;
    } else if(before->next != NULL) {
        current = before->next;
    } else {
        return start;
    }

    while(before->next != NULL)
    {
        if(current->value == value)
        {
            before->next = current->next;
            free(current);
            return start;
        }

        before = before->next;
        current = before->next;
    }

    return start;
}

ListElement *createLinkedList(int array[], int size)
{
    ListElement *listRoot = insertListElement(NULL, array[0]);
    for(int i=1;i<size;i++)
    {
        listRoot = insertListElement(listRoot, array[i]);
    }
    return listRoot;
}
#endif //IPC_IM_LINKED_LIST_H
