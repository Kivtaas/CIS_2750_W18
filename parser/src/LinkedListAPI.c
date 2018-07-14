#include "LinkedListAPI.h"
#include <stdio.h>
#include <stdlib.h>

List initializeList(char* (*printFunction)(void* toBePrinted),void (*deleteFunction)(void* toBeDeleted),int (*compareFunction)(const void* first,const void* second)){
	List newList;
	newList.head = NULL;
	newList.tail = NULL;
	newList.length = 0;
	newList.deleteData = deleteFunction;
	newList.compare = compareFunction;
	newList.printData = printFunction;

	return newList;
}

Node* initializeNode(void* data){
	if(data == NULL){
		return NULL;
	}
	Node *newNode = malloc(sizeof(data));
	newNode->data = data;
	newNode->previous = NULL;
	newNode->next = NULL;
	return newNode;
}

void insertFront(List* list, void* toBeAdded){
	if(list == NULL || toBeAdded == NULL)
		return;

	Node *toAdd = initializeNode(toBeAdded);
	if(toAdd == NULL)
		return;

	list->length += 1;

	if(list->head == NULL && list->tail == NULL){
		list->head = toAdd;
		list->tail = toAdd;
	} else{
		Node *temp = list->head;
		toAdd->next = temp;
		temp->previous = toAdd;

		list->head = toAdd;
	}
}

void insertBack(List* list, void* toBeAdded){

	if(list == NULL || toBeAdded == NULL)
		return;

	Node *toAdd = initializeNode(toBeAdded);
	if(toAdd == NULL)
		return;
	
	list->length += 1;

	if(list->head == NULL && list->tail == NULL){
		list->head = toAdd;
		list->tail = toAdd;
	} else{
		toAdd->previous = list->tail;
		list->tail->next = toAdd;
		list->tail = toAdd;
	}
}

void clearList(List* list){
	if(list == NULL)
		return;
	if(list->head == NULL){
		return;
	}
	Node* currentNode = list->tail, *tempNode;
	int i;
	for(i = 0; i < list->length; i++){
		tempNode = currentNode->previous;
		
		free(currentNode->data);
		free(currentNode);

		currentNode = tempNode;
	}
	list->head = NULL;
	list->tail = NULL;

}


void insertSorted(List* list, void* toBeAdded){
	if(toBeAdded == NULL){
		return;
	}
	if(list == NULL){
		return;
	}
	if(list->head == NULL){
		insertFront(list, toBeAdded);
		return;
	}
	Node* currentNode = list->head;
	list->length += 1;
	while(currentNode != NULL){
		int compare = list->compare(toBeAdded, currentNode->data);
		if(compare <= 0){
			Node* newNode = initializeNode(toBeAdded);
			Node* tempNode = currentNode->previous;
			if(tempNode != NULL){
				tempNode->next = newNode;
				newNode->previous = tempNode;
			}else{
				list->head = newNode;
			}
			newNode->next = currentNode;
			currentNode->previous = newNode;
			//tempNode->next = newNode;
			return;
		}
		currentNode = currentNode->next;
	}
	Node* toAdd = initializeNode(toBeAdded);
	list->tail->next = toAdd;
	toAdd->previous = list->tail;
	list->tail = toAdd;
}
void* deleteDataFromList(List* list, void* toBeDeleted){
	if(toBeDeleted == NULL){
		return NULL;
	}
	if(list == NULL){
		return NULL;
	}
	if(list->head == NULL){
		return NULL;
	}

	Node* currentNode = list->head;
	while(currentNode != NULL){
	int compare = list->compare(toBeDeleted, currentNode->data);
		if(compare == 0){
			if(currentNode->previous == NULL){
				list->head = currentNode->next;
				list->head->previous = NULL;
				void* returnData = currentNode->data;
				free(currentNode);
				list->length -= 1;
				return returnData;
			} else{
				currentNode->previous->next = currentNode->next;
				if(currentNode->next != NULL){
					currentNode->next->previous = currentNode->previous;
				}else{
					list->tail = currentNode->previous;
				}
				void* returnData = currentNode->data;
				free(currentNode);
				list->length -= 1;
				return returnData;
			}
		}
		currentNode = currentNode->next;
	}
	return NULL;
}

void* getFromFront(List list){
	if(list.head == NULL)
		return NULL;
	return list.head->data;
}

void* getFromBack(List list){
	if(list.head == NULL)
		return NULL;
	return list.tail->data;
}

char* toString(List list){
	Node* currentNode = list.head;
	char* outString;
	int i;
	char* strings[list.length];
	int total = 0;
	for(i = 0; i < list.length; i++){
		strings[i] = malloc(sizeof(list.printData(currentNode)) + 1);
		strcpy(strings[i],list.printData(currentNode));
		total += sizeof(strings[i]);
		currentNode = currentNode->next;
	}
	outString = malloc(sizeof(char) * total);
	for(i = 0; i  < list.length; i++){
		strcat(outString, strings[i]);
	}
	return outString;
}


ListIterator createIterator(List list){

	ListIterator iterator;
	iterator.current = list.head;
	return iterator;

}

void* nextElement(ListIterator* iter){
	if(iter->current == NULL)
		return NULL;
	Node *currentNode = iter->current;
	iter->current = currentNode->next;
	return currentNode->data;
}

int getLength(List list){
	return list.length;
}

void* findElement(List list, bool (*customCompare)(const void* first,const void* second), const void* searchRecord){
	Node* currentNode = list.head;
	int i;
	for(i = 0; i < list.length; i++){
		bool result = customCompare(searchRecord, currentNode->data);
		if(result == true){
			return currentNode->data;
		} else{
			currentNode = currentNode->next;
		}
	}	
	return NULL;
}
