//
// Created by wiktor on 22.12.2019.
//

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
