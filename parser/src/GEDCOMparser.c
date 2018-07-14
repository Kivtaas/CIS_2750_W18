#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"
#include "LinkedListAPI.h"
#include <stdbool.h>
#include <ctype.h>

GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj){

	GEDCOMerror error;
	error.type = OK;
	error.line = -1;
 //FILE HANDLING
	if(fileName == NULL || strlen(fileName) == 0){
		error.type = INV_FILE;
		error.line = -1;
		return error;
	}
 	FILE* fp;
 	fp = fopen(fileName, "r");
 	if(fp == NULL){
 		error.type = INV_FILE;
		error.line = -1;
		return error;
 	}
 	char ending[6];
 	strncpy(ending, fileName+(strlen(fileName)-4), 4);
 	ending[4] = '\0';

 	if(strcmp(ending, ".ged") != 0){
 		error.type = INV_FILE;
 		error.line = -1;
 		return error;
 	}

 //FILE HANDLING END
 //Creating GEDCOMobject
	GEDCOMobject* gObj = malloc(sizeof(GEDCOMobject));
	(*obj) = gObj;
	gObj->families = initializeList(printFamily, deleteFamily, compareFamilies);
	gObj->individuals = initializeList(printIndividual, deleteIndividual, compareIndividuals);
 //File parsing
	lineObj* lines = NULL;
	parseFile(fp, &lines, &error);
	lineObj* firstLine = lines;
	fclose(fp);
	if(error.type != OK){
		(*obj) = NULL;
		free(gObj);
		return error;
	}
 //	
 //Header parser
	Header* head = malloc(sizeof(Header));
	error = parseHeader(&lines, head);

	gObj->header = head;
	if(error.type != OK){
		(*obj) = NULL;
		free(gObj);
		return error;
	}
 //
 //Submitter parser
 	error = parseSubmitters(gObj, lines);

	if(error.type != OK){
		(*obj) = NULL;
		free(gObj);
		return error;
	}
 //


	xRef* refs = NULL;
    //Individual parser
	parseIndi(gObj, lines, &refs);

	//Family parser
	parseFamily(gObj, lines, refs);

	lines = firstLine;
	lineObj* tempLine;

	//frees stored lines
	while(lines != NULL){
		free(lines->type);
		free(lines->end);

		tempLine = lines;
		lines = lines->next;

		free(tempLine);
	}
	xRef *temp;

	while(refs != NULL){
		temp = refs;
		free(refs->tag);
		refs = refs->next;
		free(temp);
	}

	return error;
}

GEDCOMerror parseHeader(lineObj** lines, Header* header){
	GEDCOMerror error;
	error.type = OK;
	error.line = -1;

	header->otherFields = initializeList(printField, deleteField, compareFields);

	bool version, encoding, ref, lineSkipped, source;
	//bool headerFound = false;
	version = encoding = ref = source = lineSkipped = false;

	lineObj* currentLine = (*lines);

	if(strcmp(currentLine->type, "HEAD") != 0){
		error.type = INV_GEDCOM;
		error.line = -1;
		return error;
	}
	if(currentLine->num != 0){
		error.type = INV_HEADER;
		error.line = 1;
		return error;
	}
	currentLine = currentLine->next;
	while(currentLine->type[0] != '@'){
		lineSkipped = false;

		if(strcmp(currentLine->type, "SOUR") == 0){
			source = true;
			strcpy(header->source, currentLine->end);

			while(currentLine->num != 1){
				currentLine = currentLine->next;
				lineSkipped = true;
			}
		}
		else if(strcmp(currentLine->type, "GEDC") == 0){

				currentLine = currentLine->next;

				while(currentLine->num != 1){
				if(strcmp(currentLine->type, "VERS") == 0){
					version = true;
					header->gedcVersion = atof(currentLine->end);
				} else if (strcmp(currentLine->type, "FORM") == 0){
					
				} 
				lineSkipped = true;
				currentLine = currentLine->next;
			}		
		}
		else if(strcmp(currentLine->type, "SUBM") == 0){
			ref = true;
 			Field* newField = malloc(sizeof(Field));

			newField->tag = malloc(sizeof(currentLine->type));
			strcpy(newField->tag, currentLine->type);

			newField->value = malloc(sizeof(currentLine->end));
			strcpy(newField->value, currentLine->end);

			insertBack(&(header->otherFields), newField);
		}else if(strcmp(currentLine->type, "CHAR") == 0){
			encoding = true;
			if(strcmp(currentLine->end, "ANSEL") == 0){
				header->encoding = ANSEL;
			} else if(strcmp(currentLine->end, "UTF-8") == 0 || strcmp(currentLine->end, "UTF8") == 0){
				header->encoding = UTF8;
			}else if(strcmp(currentLine->end, "UNICODE") == 0){
				header->encoding = UNICODE;
			}else if(strcmp(currentLine->end, "ASCII") == 0){
				header->encoding = ASCII;
			}else{
				error.line = currentLine->lineNum;
				error.type = INV_HEADER;
				return error;
			}
		}

		if(lineSkipped == false)
			currentLine = currentLine->next;

	}

	if(version == false){
		error.type = INV_HEADER;
		error.line = -1;
	}
	if(encoding == false){
		error.type = INV_HEADER;
		error.line = -1;
	}
	if(ref == false){
		error.type = INV_HEADER;
		error.line = -1;
	}
	if(source == false){
		error.type = INV_HEADER;
		error.line = -1;
	}
	(*lines) = currentLine;
	return error;
}

GEDCOMerror parseSubmitters(GEDCOMobject* obj, lineObj* line){
	lineObj* currentLine = line;
	bool lineSkipped = false, address = false;
	bool submitterFound = false;

	GEDCOMerror error;
	error.type = OK;
	error.line = -1;

	while(currentLine != NULL){
		lineSkipped = false;
		if(strcmp(currentLine->end, "SUBM") == 0){
			submitterFound = true;
			Submitter* subm = malloc(sizeof(Submitter)); 
			subm->otherFields = initializeList(printField, deleteField, compareFields);

			currentLine = currentLine->next;
			while(currentLine->num != 0){
				if(strcmp(currentLine->type, "NAME") == 0){
					strcpy(subm->submitterName, currentLine->end);
				} else if(strcmp(currentLine->type, "ADDR") == 0){
					address = true;
					char tempAddr[200];
					strcpy(tempAddr, currentLine->end);
					if(strcmp(currentLine->next->type, "CONT") == 0){
						sprintf(tempAddr, " %s", currentLine->next->end);
						currentLine = currentLine->next;
					}					
					Submitter *subB = realloc(subm, sizeof(Submitter) + sizeof(tempAddr));
					subm = subB;
					strcpy(subm->address, tempAddr);

				} else{
					Field* newField = malloc(sizeof(Field));
					newField->tag = malloc(sizeof(currentLine->type));
					newField->value = malloc(sizeof(currentLine->end));

					strcpy(newField->tag, currentLine->type);
					strcpy(newField->value, currentLine->end);

					insertBack(&(subm->otherFields), newField);
				}
				lineSkipped = true;
				currentLine = currentLine->next;
			}
			if(address == false){
				Submitter *subB = realloc(subm, sizeof(Submitter) + 1);
				subm = subB;
				strcpy(subm->address, "");
			}
			obj->submitter = subm;
			obj->header->submitter = subm;
		}
		if(!lineSkipped)
			currentLine = currentLine->next;
	}

		if(submitterFound == false){
			error.type = INV_GEDCOM;
			error.line = -1;
		}
	return error;
}

GEDCOMerror parseIndi(GEDCOMobject* obj, lineObj* line, xRef** refs){
	lineObj* currentLine = line;
	bool lineSkipped = false;

	GEDCOMerror error;
	error.type = OK;
	error.line = -1;

	xRef* head = NULL;

	while(currentLine != NULL){
		lineSkipped = false;

		if(strcmp(currentLine->end, "INDI") == 0){
			Individual* indi = malloc(sizeof(Individual));

			indi->events = initializeList(printEvent, deleteEvent, compareEvents);
			indi->families = initializeList(printFamily, deleteFamily, compareFamilies);
			indi->otherFields = initializeList(printField, deleteField, compareFields);

			xRef* newField = malloc(sizeof(xRef));
			newField->tag = malloc(sizeof(currentLine->type));

			strcpy(newField->tag, currentLine->type);

			newField->indi = indi;
			newField->next = head;

			head = newField;
			
			currentLine = currentLine->next;

			while(currentLine->num != 0){

				int eventType = validEvent(currentLine->type, 1);
				if(eventType == 24){
					char name[60] = "\0";
					strcpy(name, currentLine->end);

					if(strcmp(currentLine->next->type, "CONT") == 0){
						strcat(name, currentLine->next->end);
						currentLine = currentLine->next;
					}
					char firstName [30] = "\0";
					char lastName [30] = "\0";

					int i = 0;
					while(name[i+1] != '/'){
						firstName[i] = name[i];
						i++;
					}
					firstName[i+1] = '\0';
					i += 2;

					int ii = 0;
					while(name[i] != '/'){
						lastName[ii] = name[i];
						ii++;
						i++;
					}
					currentLine = currentLine->next;

					while(currentLine->num > 1){
						eventType = validEvent(currentLine->type, 1);
						if(eventType == 25){
							for(i = 0; i < 30; i++){
								firstName[i] = '\0';
							}
							strcpy(firstName,currentLine->end);
						}
						else if(eventType == 26){
							for(i = 0; i < 30; i++){
								lastName[i] = '\0';
							}
							strcpy(lastName,currentLine->end);
						}
						currentLine = currentLine->next;
					}
					currentLine = currentLine->prev;
					indi->givenName = malloc(sizeof(firstName));
					indi->surname = malloc(sizeof(lastName));

					strcpy(indi->givenName, firstName);
					strcpy(indi->surname, lastName);
				} else if(eventType >= 0 && eventType <= 21){

					Event* event = malloc(sizeof(Event));
					event->otherFields = initializeList(printField, deleteField, compareFields);
					strcpy(event->type, currentLine->type);
					currentLine = currentLine->next;

					while(currentLine->num > 1){
						eventType = validEvent(currentLine->type, 3);

						if(eventType == 0){
							event->date = malloc(sizeof(currentLine->end));
							strcpy(event->date, currentLine->end);
						} else if(eventType == 1){
							event->place = malloc(sizeof(currentLine->end));
							strcpy(event->place, currentLine->end);
						} else{
							Field* newField = malloc(sizeof(Field));
							newField->tag = malloc(sizeof(currentLine->type));
							newField->value = malloc(sizeof(currentLine->end));

							strcpy(newField->tag, currentLine->type);
							strcpy(newField->value, currentLine->end);

							insertBack(&event->otherFields, newField);
						}
						currentLine = currentLine->next;
					}
					if(event->date == NULL){
						event->date = malloc(sizeof(char)*3);
						strcpy(event->date, " ");
					}
					if(event->place == NULL){
						event->place = malloc(sizeof(char)*3);
						strcpy(event->place, " ");
					}
					insertBack(&indi->events, event);
					currentLine = currentLine->prev;
				} else if(eventType == 22 || eventType == 23){

				} else if (eventType == -1){

					Field* newField = malloc(sizeof(Field));
					newField->tag = malloc(sizeof(currentLine->type));
					newField->value = malloc(sizeof(currentLine->end));

					strcpy(newField->tag, currentLine->type);
					strcpy(newField->value, currentLine->end);

					insertBack(&indi->otherFields, newField);
				}
				lineSkipped = true;
				currentLine = currentLine->next;

			}
			insertBack(&obj->individuals, indi);
		}	

		if(lineSkipped == false)
			currentLine = currentLine->next;
	}

	(*refs) = head;
	return error;
}

GEDCOMerror parseFamily(GEDCOMobject* obj, lineObj* line,  xRef* refs){
	GEDCOMerror error;
	error.type = OK;
	error.line = -1;
	bool lineSkipped = false;
	lineObj *currentLine = line;
	while(currentLine != NULL){
		lineSkipped = false;
		if(strcmp(currentLine->end, "FAM") == 0){
			Family *fam = malloc(sizeof(Family));
			fam->children = initializeList(printIndividual, deleteIndividual, compareIndividuals);
			fam->otherFields = initializeList(printField, deleteField, compareFields);
			fam->events = initializeList(printEvent, deleteEvent, compareEvents);

			currentLine = currentLine->next;
			lineSkipped = true;
			int eventType;
			while(currentLine->num != 0){
				eventType = validEvent(currentLine->type, 2);
				if(strcmp(currentLine->type, "HUSB") == 0 || strcmp(currentLine->type, "WIFE") == 0 || strcmp(currentLine->type, "CHIL") == 0){
				//example code for getting the xref
				//printf("%s\n", ((Field*)(getFromFront(indi->otherFields)))->tag);
					bool matchFound = false;

					xRef *indiList = refs;
					Individual* t_indi = NULL;
					while (matchFound != true && indiList != NULL){
						t_indi = indiList->indi;
						if(strcmp(indiList->tag, currentLine->end) == 0){
							matchFound = true;
							if(strcmp(currentLine->type, "HUSB") == 0){
								fam->husband = t_indi;
							} else if(strcmp(currentLine->type, "WIFE") == 0){
								fam->wife = t_indi;
							} else if(strcmp(currentLine->type, "CHIL") == 0){
								insertBack(&(fam->children), t_indi);
							}
							insertBack(&(t_indi->families), fam);
						}
						indiList = indiList->next;
					}
				} else if(eventType != -1){
					Event* event = malloc(sizeof(Event));
					event->otherFields = initializeList(printField, deleteField, compareFields);
					strcpy(event->type, currentLine->type);
					currentLine = currentLine->next;

					while(currentLine->num > 1){
						eventType = validEvent(currentLine->type, 3);

						if(eventType == 0){
							event->date = malloc(sizeof(currentLine->end));
							strcpy(event->date, currentLine->end);
						} else if(eventType == 1){
							event->place = malloc(sizeof(currentLine->end));
							strcpy(event->place, currentLine->end);
						} else{
							Field* newField = malloc(sizeof(Field));
							newField->tag = malloc(sizeof(currentLine->type));
							newField->value = malloc(sizeof(currentLine->end));

							strcpy(newField->tag, currentLine->type);
							strcpy(newField->value, currentLine->end);

							insertBack(&event->otherFields, newField);
						}
						currentLine = currentLine->next;
					} 
					insertBack(&fam->events, event);
					currentLine = currentLine->prev;
				} else{
 					Field* newField = malloc(sizeof(Field));
					newField->tag = malloc(sizeof(currentLine->type));
					newField->value = malloc(sizeof(currentLine->end));

					strcpy(newField->tag, currentLine->type);
					strcpy(newField->value, currentLine->end);

					insertBack(&fam->otherFields, newField);
				}
				currentLine = currentLine->next;
			}
			insertBack(&obj->families, fam);
		}
		if(lineSkipped == false)
			currentLine = currentLine->next;
	}

	return error;
}

GEDCOMerror parseFile(FILE* fp, lineObj** lines, GEDCOMerror* error){
	error->type = OK;
	error->line = -1;

	int lineNum = 0;
	char line[260];
	char num[5], type[10], end[250];

	lineObj* head = NULL;
	lineObj* current = NULL;

	//while(fgets(line, sizeof(line), fp))
	while(readLine(line, fp) != EOF)
	{
		if(strlen(line) <= 1){
			continue;
		}
		lineNum++;
		if(strlen(line) > 255)
		{	
			readLine(line, fp);
			error->type = INV_RECORD;
			error->line = lineNum;
			return *error;
		}

		parseLine(line, num, type, end);
		if(head == NULL){
			lineObj* newObj = malloc(sizeof(lineObj));
			newObj->num = atoi(num);

			newObj->type = malloc(sizeof(char) * (strlen(type) +  1));
			strcpy(newObj->type, type);

			newObj->end = malloc(sizeof(char) * (strlen(end) +  1));
			strcpy(newObj->end, end);

			newObj->next = NULL;
			newObj->prev = NULL;
			newObj->lineNum = lineNum;
			head = newObj;
			current = newObj;
		} else{
			lineObj* newObj = malloc(sizeof(lineObj));
			newObj->num = atoi(num);

			newObj->type = malloc(sizeof(char) * (strlen(type) +  1));
			strcpy(newObj->type, type);

			newObj->end = malloc(sizeof(char) * (strlen(end) +  1));
			strcpy(newObj->end, end);

			newObj->lineNum = lineNum;

			newObj->next = NULL;
			newObj->prev = current;
			current->next = newObj;
			current = newObj;
			if(current->prev->num - current->num < -1){
				error->type = INV_RECORD;
				error->line = current->lineNum;
				return *error;
			}
		}

	}

	if(strcmp(current->type, "TRLR") != 0){
		error->type = INV_GEDCOM;
		error->line = -1;
	}
	(*lines) = head; 

	return *error; 
}

char readLine(char* buffer, FILE* fp){
	char c;
	for(int i = 0; i < 260; i++){
		buffer[i] = '\0';
	}
	int i = 0;

	while((c = fgetc(fp)) != EOF && c != '\r' && c != '\n' && c != '\0' &&  i < 260){
		buffer[i] = c;
		buffer[i+1] = '\n';
		i++;
	}
	if(c == EOF && strlen(buffer) > 1){
		return 'c';
	}
	return c;
}

char* printGEDCOM(const GEDCOMobject* obj){
	if(obj == NULL){
		return NULL;
	}
	if(obj->header == NULL){
		return NULL;
	}
	char toPrint[1500] = "-----Individuals-----: \n\n";
	Node* currentNode = obj->individuals.head;
	while(currentNode != NULL){
		strcat(toPrint,((Individual*)(currentNode->data))->givenName);
		strcat(toPrint," ");
		strcat(toPrint,((Individual*)(currentNode->data))->surname);
		strcat(toPrint,"\n");
		currentNode = currentNode->next;
	}
	currentNode = obj->families.head;
	strcat(toPrint, "-----FAMILIES-----: \n\n");
	while(currentNode != NULL){
		Family *fam = ((Family*)(currentNode->data));
		if(fam->husband != NULL){
			strcat(toPrint, fam->husband->givenName);
			strcat(toPrint," ");
			strcat(toPrint, fam->husband->surname);
		}else{
			strcat(toPrint,"--UNKNOWN--");
		}
		strcat(toPrint, " married ");
		if(fam->wife == NULL){
			strcat(toPrint, "--UNKNOWN--");
		}else{
			strcat(toPrint, fam->wife->givenName);
			strcat(toPrint," ");
			strcat(toPrint, fam->wife->surname);
		}
		char childNum[30];
		sprintf(childNum," with %d children", fam->children.length);
		strcat(toPrint, childNum);
		strcat(toPrint,"\n");		

		currentNode = currentNode->next;
	}
	char* output = malloc(sizeof(char) * 1000);
	strcpy(output, toPrint);
	return output;
}

List getDescendants(const GEDCOMobject* familyRecord, const Individual* person){
	List descendants = initializeList(printIndividual, deleteIndividual, compareIndividuals);
	if(familyRecord == NULL) 
		return descendants;
	if(person == NULL){
		return descendants;
	}
	Node* currentNode = person->families.head;
	while(currentNode != NULL){
		Family *fam = ((Family*)(currentNode->data));
		
		if(fam->husband == person || fam->wife == person){
			getChildren(&descendants, person);
		}

		currentNode = currentNode->next;
	}
	return descendants;
}
//recursive function to create children list
void getChildren(List* descendants,const Individual* person){
	Node* currentNode = person->families.head;
	while(currentNode != NULL){
		Family *fam = ((Family*)(currentNode->data));
		if(fam->husband == person || fam->wife == person){
			if(fam->children.length != 0){
				Individual* child;
				Node* subNode = fam->children.head;

				while(subNode != NULL){
					child = (Individual*)(subNode->data);
					Individual* shallow = malloc(sizeof(Individual));

					shallow->givenName = malloc(sizeof(child->givenName));
					shallow->surname = malloc(sizeof(child->surname));

					strcpy(shallow->givenName, child->givenName);
					strcpy(shallow->surname, child->surname);

					insertBack(descendants, shallow);
					getChildren(descendants,child);
					subNode = subNode->next;
				}
			}
		}
		currentNode = currentNode->next;
	}
}

Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person){
	if(familyRecord == NULL) 
		return NULL;
	if(familyRecord->header == NULL){
		return NULL;
	}
	if(person == NULL){
		return NULL;
	}

	Node* currentNode = familyRecord->individuals.head;

	while(currentNode != NULL){
		Individual* indi = ((Individual*)(currentNode->data));
		if(compare(indi, person) != 0){
			return indi;
		} 
		currentNode = currentNode->next;
	}

	return NULL;
}


void deleteGEDCOM(GEDCOMobject* obj){
	if(obj == NULL){
		return;
	}

	Node* currentNode;
	Node* temp;
	currentNode = obj->header->submitter->otherFields.head;
	while(currentNode != NULL){
		obj->header->submitter->otherFields.deleteData(currentNode->data);
		temp = currentNode;
		currentNode = currentNode->next;
		free(temp);
	}
	free(obj->header->submitter);

	currentNode = obj->header->otherFields.head;
	while(currentNode != NULL){
		obj->header->otherFields.deleteData(currentNode->data);
		temp = currentNode;
		currentNode = currentNode->next;
		free(temp);
	}
	free(obj->header);
	obj->header = NULL;

	currentNode = obj->families.head;
	while(currentNode != NULL){
		obj->families.deleteData(currentNode->data);
		temp = currentNode;
		currentNode = currentNode->next;
		free(temp);
	}
	obj->families.head = NULL;
	obj->families.tail = NULL;

	currentNode = obj->individuals.head;
	while(currentNode != NULL){
		obj->individuals.deleteData(currentNode->data);
		temp = currentNode;
		currentNode = currentNode->next;
		free(temp);
	}
	obj->individuals.head = NULL;
	obj->individuals.tail = NULL;
}

//function to parse a line in and save the information in the appropiate strings

void parseLine(char* line, char* cNum, char* cType, char* cEnd){

		char num[5];
		char type[10];
		char end[250];

		clearStrings(num, type, end);
		int i = 0;
		//Code for line chunk 1
		while(line[i] != ' '){
			num[i] = line[i];
			num[i+1] = '\0';
			i++;
		}
		i++;

		int ii = 0;
		while(!isspace(line[i])){
			type[ii] = line[i];
			type[ii+1] = '\0';
			i++;
			ii++;
		}

		i++;
		ii = 0;
		while(line[i] != '\n' && line[i] != '\r' && line[i] != '\0'){
			end[ii] = line[i];
			end[ii+1] = '\0';
			i++;
			ii++;
		}
		if(strlen(end) < 1){
			strcpy(end, "\0");
		}

		strcpy(cNum, num);
		strcpy(cType, type);
		strcpy(cEnd, end);
}

//functions to clean strings 
void clearStrings(char* num, char* type, char* end){
	for(int i = 0; i < 5; i++){
		num[i] = '\0';
	}
	for(int i = 0; i < 10; i++){
		type[i] = '\0';
	}
	for(int i = 0; i < 250; i++){
		end[i] = '\0';
	}
}

void clearLine(char* line){
	for(int i = 0; i < 256; i++){
		line[i] = '\0';
	}
}

//function to check type of event, as well as if it's valid
int validEvent(char* event, int sub){

	const char *iEvent[27];
	iEvent[0] = "BIRT";
	iEvent[1] = "CHR";
	iEvent[2] = "DEAT";
	iEvent[3] = "BURI";
	iEvent[4] = "CREM";
	iEvent[5] = "ADOP";
	iEvent[6] = "BAPM";
	iEvent[7] = "BARM";
	iEvent[8] = "BASM";
	iEvent[9] = "BLES";
	iEvent[10] = "CHRA";
	iEvent[11] = "CONF";
	iEvent[12] = "FCOM";
	iEvent[13] = "ORDN";
	iEvent[14] = "NATU";
	iEvent[15] = "EMIG";
	iEvent[16] = "IMMI";
	iEvent[17] = "PROB";
	iEvent[18] = "WILL";
	iEvent[19] = "GRAD";
	iEvent[20] = "RETI";
	iEvent[21] = "EVEN";
	iEvent[22] = "FAMC";
	iEvent[23] = "FAMS";
	iEvent[24] = "NAME";
	iEvent[25] = "GIVN";
	iEvent[26] = "SURN";

	const char *fEvent[12];
	fEvent[0] = "ANUL";
	fEvent[1] = "CENS";
	fEvent[2] = "DIV";
	fEvent[3] = "DIVF";
	fEvent[4] = "ENGA";
	fEvent[5] = "MARB";
	fEvent[6] = "MARC";
	fEvent[7] = "MARR";
	fEvent[8] = "MARL";
	fEvent[9] = "MARS";
	fEvent[10] = "RESI";
	fEvent[11] = "EVEN";

	const char *aEvent[2];
	aEvent[0] = "DATE";
	aEvent[1] = "PLAC";

	int length = 0;

	if(sub == 1){
		length = 27;
		for(int i = 0; i < length; i++){
			if(strcmp(event, iEvent[i]) == 0){
				return i;
			}
		}
	} else if(sub == 2){
		length = 12;
		for(int i = 0; i < length; i++){
			if(strcmp(event, fEvent[i]) == 0){
				return i;
			}
		}
	} else{
		length = 2;
		for(int i = 0; i < length; i++){
			if(strcmp(event, aEvent[i]) == 0){
				return i;
			}
		}		
	}


	return -1;
}

//function to parse the date for the compare event function

void parseDate(char* line, char* cDay, char* cMonth, char* cYear){

		char day[5];
		char month[10];
		char year[250];

		clearStrings(day, month, year);

		int i = 0;
		//Code for line chunk 1
		while(line[i] != ' '){
			day[i] = line[i];
			day[i+1] = '\0';
			i++;
		}

		i++;


		int ii = 0;
		while(!isspace(line[i])){
			month[ii] = line[i];
			month[ii+1] = '\0';
			i++;
			ii++;
		}

		i++;
		ii = 0;
		while(i < strlen(line)){
			year[ii] = line[i];
			year[ii+1] = '\0';
			i++;
			ii++;
		}

		strcpy(cDay, day);
		strcpy(cMonth, month);
		strcpy(cYear, year);
}

int monthToInt(char* month){
	const char* months[12];
	months[0] = "JAN";
	months[1] = "FEB";
	months[2] = "MAR";
	months[3] = "APRL";
	months[4] = "MAY";
	months[5] = "JUNE";
	months[6] = "JULY";
	months[7] = "AUG";
	months[8] = "SEPT";
	months[9] = "OCT";
	months[10] = "NOV";
	months[11] = "DEC";

	for(int i = 0; i < 12; i++){
		if(strcmp(months[i], month) == 0){
			return i;
		}
	}
	return -1;
}

char* printError(GEDCOMerror err){

	char* toPrint = malloc(sizeof(char) * 30);
	sprintf(toPrint, "Error code: %d at line %d", err.type, err.line);
	return toPrint;
}


//*************************************************
// ASSIGNMENT 2 
//*************************************************

GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj){
	GEDCOMerror error;
	error.type = OK;
	error.line = -1;
 	

	if(obj == NULL){
		error.type = WRITE_ERROR;
		error.line = -1;
		return error;
	}

	if(fileName == NULL){
		error.type = WRITE_ERROR;
		error.line = -1;
		return error;
	}
 	FILE* fp;
 	fp = fopen(fileName, "w");

 	if(fp == NULL){
 		error.type = INV_FILE;
		error.line = -1;
		return error;
 	}
	char ending[6];
 	strncpy(ending, fileName+(strlen(fileName)-4), 4);
 	ending[4] = '\0';

 	if(strcmp(ending, ".ged") != 0){
 		error.type = INV_FILE;
 		error.line = -1;
 		return error;
 	}
	
	lineObj* lines = NULL, *head = NULL;
	writeHeader(&lines, obj);
	head = lines;

	while(lines->next != NULL){
		lines = lines->next;
	}

	xRef* refs = NULL;
	populateXRef(&refs, obj);

	writeSubmitter(lines, obj);

	while(lines->next != NULL){
		lines = lines->next;
	}

	writeIndividuals(lines, obj, refs);

	while(lines->next != NULL){
		lines = lines->next;
	}

	writeFamily(lines, obj, refs);

	lineObj* temp;
	while(head != NULL){
		fprintf(fp,"%d %s %s\n", head->num, head->type, head->end);
		free(head->type);
		free(head->end);
		temp = head;
		head = head->next;
		free(temp);
	}
	xRef* tempRef = refs;
	while(refs != NULL){
		free(refs->tag);
		tempRef = refs;
		refs = refs->next;
		free(tempRef);
	}

	fclose(fp);
	return error;
}

void writeHeader(lineObj** lines, const GEDCOMobject* obj){

	const char* encoding[4] = {"ANSEL", "UTF8", "UNICODE", "ASCII"};

	Header* head = obj->header;

	lineObj* currentLine = malloc(sizeof(lineObj));

	currentLine->next = NULL;
	currentLine->prev = NULL;

	lineObj* newLine = NULL;

	int stageNum = 0;
	char type[5], end[200];

	strcpy(type, "HEAD");
	strcpy(end, "");

	addLines(&currentLine, stageNum, type, end);

	stageNum = 1;
	(*lines) = currentLine;

	//Source saving
	newLine = malloc(sizeof(lineObj));
	newLine->next = NULL;
	newLine->prev = NULL;

	strcpy(type, "SOUR");
	strcpy(end, head->source);
	strcat(end, "\0");
	newLine->type = malloc(sizeof(type));
	newLine->end = malloc(sizeof(end));

	addLines(&currentLine, stageNum, type, end);
	//GEDC saving

	strcpy(type, "GEDC");
	strcpy(end, "");

	newLine->type = malloc(sizeof(type));
	newLine->end = malloc(sizeof(end));

	addLines(&currentLine, stageNum, type, end);

	stageNum = 2;

	strcpy(type, "VERS");
	strcpy(end, "");
	sprintf(end, "%.1lf", head->gedcVersion);

	newLine->type = malloc(sizeof(type));
	newLine->end = malloc(sizeof(end));

	addLines(&currentLine, stageNum, type, end);

	strcpy(type, "FORM");
	strcpy(end, "LINEAGE-LINKED");

	addLines(&currentLine, stageNum, type, end);

	stageNum = 1;

	strcpy(type, "CHAR");
	strcpy(end, encoding[head->encoding]);

	addLines(&currentLine, stageNum, type, end);

	strcpy(type, "SUBM");
	strcpy(end, "@U1@");

	addLines(&currentLine, stageNum, type, end);
}

void writeSubmitter(lineObj* lines, const GEDCOMobject* obj){
	Submitter* subm = obj->submitter;
	int stageNum = 0;

	char type[10];
	char end[200];

	strcpy(type, "@U1@");
	strcpy(end, "SUBM");
	addLines(&lines, stageNum, type, end);

	stageNum++;

	strcpy(type, "NAME");
	strcpy(end, subm->submitterName);
	addLines(&lines, stageNum, type, end);

	if(strlen(subm->address) > 1){
		strcpy(type, "ADDR");
		strcpy(end, subm->address);
		addLines(&lines, stageNum, type, end);
	}
}

void writeIndividuals(lineObj* lines, const GEDCOMobject* obj, xRef* refs){
 	Node* indiList = obj->individuals.head;
	Individual* indi;
	int indiNum = 1;

	int stageNum = 0;
	char type[10], end[200];

	while(indiList != NULL){
		stageNum = 0;

		indi = (Individual*)(indiList->data);
		xRef* refValue;
		refValue = refs;

		while(refValue->indi != indi && refValue != NULL){
			refValue = refValue->next;
		}

		strcpy(type, refValue->tag);
		strcpy(end, "INDI");
		addLines(&lines, stageNum, type, end);

		stageNum++;

		strcpy(type, "NAME");
		strcpy(end, "");
		sprintf(end, "%s /%s/", indi->givenName, indi->surname);
		addLines(&lines, stageNum, type, end);
		if(strlen(indi->givenName) > 1){
			stageNum = 2;
			strcpy(type, "GIVN");
			strcpy(end, indi->givenName);
			addLines(&lines, stageNum, type, end);
		}
		if(strlen(indi->surname) > 1){
			stageNum = 2;
			strcpy(type, "SURN");
			strcpy(end, indi->surname);
			addLines(&lines, stageNum, type, end);
		}
		stageNum = 1;

		Node* otherNode = indi->otherFields.head;
		stageNum = 1;
		
		while(otherNode != NULL){
				Field* otherField = (Field*)(otherNode->data);
				strcpy(type, otherField->tag);
				strcpy(end, otherField->value);
				addLines(&lines, stageNum, type, end);

				otherNode = otherNode->next;
		}


		Node* listNode = indi->events.head;
		while(listNode != NULL){
			Event* event = (Event*)(listNode->data);

			strcpy(type, event->type);
			strcpy(end, "");
			stageNum = 1;

			addLines(&lines, stageNum, type, end);
			if(strcmp(event->date, " ") != 0){
				stageNum = 2;
				strcpy(type, "DATE");
				strcpy(end, event->date);
				addLines(&lines, stageNum, type, end);
			}
			if(strcmp(event->place, " ") != 0){
				stageNum = 2;
				strcpy(type, "PLAC");
				strcpy(end, event->place);
				addLines(&lines, stageNum, type, end);
			}

			otherNode = event->otherFields.head;
			while(otherNode != NULL){
				Field* otherField = (Field*)(otherNode->data);
				stageNum = 2;
				strcpy(type, otherField->tag);
				strcpy(end, otherField->value);
				addLines(&lines, stageNum, type, end);

				otherNode = otherNode->next;
			}

			listNode = listNode->next;
		}

		listNode = indi->families.head;
		stageNum = 1;
		while(listNode != NULL){
			Family* fam = (Family*)(listNode->data);
			xRef* fRef = refs;

			while(fRef->fam != fam && fRef != NULL){
				fRef = fRef->next;
			}
			
			if(fam->husband == indi || fam->wife == indi){
				strcpy(type, "FAMS");
			} else{
				strcpy(type, "FAMC");
			}

			strcpy(end, fRef->tag);
			addLines(&lines, stageNum, type, end);

			listNode = listNode->next;
		}

		indiList = indiList->next;	
		indiNum++;
	}
}

void writeFamily(lineObj* lines, const GEDCOMobject* obj, xRef* refs){
	Node* famList = obj->families.head;
	char type[10], end[200];
	int stageNum = 0;

	while(famList != NULL){
		Family* fam = (Family*)(famList->data);
		xRef* famRef = refs;

		while(famRef->fam != fam){
			famRef = famRef->next;
		}

		strcpy(type, famRef->tag);
		strcpy(end, "FAM");
		stageNum = 0;
		addLines(&lines, stageNum, type, end);
		stageNum = 1;

		if(fam->husband != NULL){
			famRef = refs;
			strcpy(type, "HUSB");
			while(famRef->indi != fam->husband){
				famRef = famRef->next;
			}
			strcpy(end, famRef->tag);
			addLines(&lines, stageNum, type, end);
		}
		if(fam->wife != NULL){
			famRef = refs;
			strcpy(type, "WIFE");
			while(famRef->indi != fam->wife){
				famRef = famRef->next;
			}
			strcpy(end, famRef->tag);
			addLines(&lines, stageNum, type, end);
		}
		Node* indiList = fam->children.head;
		while(indiList != NULL){
			Individual* indi = (Individual*)(indiList->data);
			strcpy(type, "CHIL");

			famRef = refs;
			while(famRef->indi != indi){
				famRef = famRef->next;
			}
			strcpy(end, famRef->tag);

			addLines(&lines, stageNum, type, end);

			indiList = indiList->next;
		}  

		Node* listNode = fam->events.head;
		while(listNode != NULL){
			Event* event = (Event*)(listNode->data);

			strcpy(type, event->type);
			strcpy(end, "");
			stageNum = 1;

			addLines(&lines, stageNum, type, end);
			if(event->date != NULL){
				stageNum = 2;
				strcpy(type, "DATE");
				strcpy(end, event->date);
				addLines(&lines, stageNum, type, end);
			}
			if(event->place != NULL){
				stageNum = 2;
				strcpy(type, "PLAC");
				strcpy(end, event->place);
				addLines(&lines, stageNum, type, end);
			}

			Node* otherNode = event->otherFields.head;
			while(otherNode != NULL){
				Field* otherField = (Field*)(otherNode->data);
				stageNum = 1;
				strcpy(type, otherField->tag);
				strcpy(end, otherField->value);
				addLines(&lines, stageNum, type, end);

				otherNode = otherNode->next;
			}

			otherNode = fam->otherFields.head;
			stageNum = 1;
		
			while(otherNode != NULL){
				Field* otherField = (Field*)(otherNode->data);
				strcpy(type, otherField->tag);
				strcpy(end, otherField->value);
				addLines(&lines, stageNum, type, end);

				otherNode = otherNode->next;
		}

			listNode = listNode->next;
		}
		famList = famList->next;
	}

	strcpy(type, "TRLR");
	strcpy(end, "");
	stageNum = 0;

	addLines(&lines, stageNum, type, end);
}

void addLines(lineObj** lines, int stageNum, char* type, char* end){

	lineObj* newLine = malloc(sizeof(lineObj));
	newLine->next = NULL;
	newLine->prev = NULL;

	newLine->type = malloc(sizeof(type));
	newLine->end = malloc(sizeof(end));

	strcpy(newLine->type, type);
	strcpy(newLine->end, end);
	newLine->num = stageNum;

	(*lines)->next = newLine;
	newLine->prev = (*lines);
	(*lines) = newLine;
}

void populateXRef(xRef** refs, const GEDCOMobject* obj){
	int i = 1;

	xRef* currentRef = NULL;
	char ref[10];

	Node* node = obj->individuals.head;
	Individual* indi;

	while(node != NULL){
		indi = ((Individual*)node->data);

		xRef* newRef = malloc(sizeof(xRef));
		newRef->next = NULL;

		strcpy(ref, "");
		sprintf(ref, "@INDI%d@", i);

		newRef->indi = indi;
		newRef->tag = malloc(sizeof(ref));
		strcpy(newRef->tag, ref);

		if(currentRef == NULL){
			currentRef = newRef;
			(*refs) = currentRef;
		}else{
			currentRef->next = newRef;
			currentRef = newRef;
		}
		i++;
		node = node->next;
	}

	node = obj->families.head;
	Family* fam = NULL;

	i = 1;

	while(node != NULL){
		fam = ((Family*)node->data);

		xRef* newRef = malloc(sizeof(xRef));
		newRef->next = NULL;

		strcpy(ref, "");
		sprintf(ref, "@FAM%d@", i);

	
		newRef->fam = fam;
		newRef->tag = malloc(sizeof(ref));
		strcpy(newRef->tag, ref);

		if(currentRef == NULL){
			currentRef = newRef;
			(*refs) = currentRef;
		}else{
			currentRef->next = newRef;
			currentRef = newRef;
		}
		i++;
		node = node->next;
	}
}

ErrorCode validateGEDCOM(const GEDCOMobject* obj){
	if(obj == NULL){
		return INV_GEDCOM;
	}
	if(obj->header == NULL){
		return INV_GEDCOM;
	}
	if(obj->submitter == NULL){
		return INV_GEDCOM;
	}
	if(&obj->individuals == NULL || &obj->families == NULL){
		return INV_GEDCOM;
	}
	Header* head = obj->header;
	if(head->submitter == NULL){
		return INV_HEADER;
	}
	if(strlen(head->source) < 1){
		return INV_HEADER;
	}
	Submitter* subm = obj->submitter;
	if(strcmp(subm->submitterName, "") == 0){
		return INV_RECORD;
	}

	Node* listNode = obj->individuals.head;
	while(listNode != NULL){
		Individual* toCheck = (Individual*)(listNode->data);
		if(listNode->data == NULL){
			return INV_RECORD;
		}
		if(strlen(toCheck->surname) > 200 || strlen(toCheck->givenName) > 200){
			return INV_RECORD;
		}
		if(&toCheck->events == NULL){
			return INV_RECORD;
		}

		Node* subCheck = toCheck->events.head;
		while(subCheck != NULL){
			Event* eCheck = (Event*)(subCheck->data);

			if(strlen(eCheck->date) > 200 || strlen(eCheck->place) > 200){
				return INV_RECORD;
			}
			if(&eCheck->otherFields == NULL){
				return INV_RECORD;
			}

			Node* fCheck = eCheck->otherFields.head;
			
			while(fCheck != NULL){
				Field* field = (Field*)(fCheck->data);
 				if(field->tag == NULL || field->value == NULL){
					return INV_RECORD;
				}
				if(strlen(field->tag) > 200 || strlen(field->value) > 200){
					return INV_RECORD;
				}

				fCheck = fCheck->next;
			}
			subCheck = subCheck->next;
		}

		Node* fCheck = toCheck->otherFields.head;
		while(fCheck != NULL){
			Field* field = (Field*)(fCheck->data);
			if(field->tag == NULL || field->value == NULL){
				return INV_RECORD;
			}
			if(strlen(field->tag) < 1 || strlen(field->value) < 1){
				return INV_RECORD;
			}
			if(strlen(field->tag) > 200 || strlen(field->value) > 200){
				return INV_RECORD;
			}
			fCheck = fCheck->next;
		}

		listNode = listNode->next;
	}

	listNode = obj->families.head;
	while(listNode != NULL){
		if(listNode->data == NULL){
			return INV_RECORD;
		}
		Family* toCheck = (Family*)(listNode->data);
		
		Node* fCheck = toCheck->otherFields.head;
		while(fCheck != NULL){
			Field* field = (Field*)(fCheck->data);
			if(field->tag == NULL || field->value == NULL){
				return INV_RECORD;
			}
			if(strlen(field->tag) < 1 || strlen(field->value) < 1){
				return INV_RECORD;
			}
			if(strlen(field->tag) > 200 || strlen(field->value) > 200){
				return INV_RECORD;
			}
			fCheck = fCheck->next;
		}

		listNode = listNode->next;
	}

	return OK;
}


/** Function to return a list of up to N generations of descendants of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of descendants has been created
 *@return a list of descendants.  The list may be empty.  All list members must be of type List.    *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen){
	List descendants = initializeList(printIndividual, deleteIndividual, compareIndividuals);

	if(familyRecord == NULL) 
		return descendants;
	if(person == NULL){
		return descendants;
	}
	Node* currentNode = person->families.head;
	while(currentNode != NULL){
		Family *fam = ((Family*)(currentNode->data));
		
		if((fam->children.length != 0) && (fam->husband == person || fam->wife == person)){
			if(maxGen == 0){
				getChildrenN(&descendants, person, 0, 2147483600);
			}else{
				getChildrenN(&descendants, person, 0, maxGen);
			}
		}

		currentNode = currentNode->next;
	}
	return descendants;
}

void getChildrenN(List* descendants,const Individual* person, unsigned int n, unsigned int max){

	if(n == max){
		return;
	}
	Node* currentNode = descendants->head;
	for(int i = 1; i < n; i++){
		currentNode = currentNode->next;
	}
	List genList;
	if(currentNode == NULL){
		genList = initializeList(printIndividual, deleteIndividual, compareIndividuals);
		insertBack(descendants, &genList);
	} else{
		genList = *(List*)(currentNode->data);
	}

	currentNode = person->families.head;

	while(currentNode != NULL){
		Family *fam = ((Family*)(currentNode->data));
		if(fam->husband == person || fam->wife == person){
			if(fam->children.length != 0){
				Individual* child;
				Node* subNode = fam->children.head;

				while(subNode != NULL){
					child = (Individual*)(subNode->data);
					Individual* shallow = malloc(sizeof(Individual));

					shallow->givenName = malloc(sizeof(child->givenName));
					shallow->surname = malloc(sizeof(child->surname));

					strcpy(shallow->givenName, child->givenName);
					strcpy(shallow->surname, child->surname);

					insertSorted(&genList, shallow);
					getChildrenN(descendants,child, n+1, max);
					subNode = subNode->next;
				}
			}
		}
		currentNode = currentNode->next;
	}
}

/** Function to return a list of up to N generations of ancestors of an individual in a GEDCOM
 *@pre GEDCOM object exists, is not null, and is valid
 *@post GEDCOM object has not been modified in any way, and a list of ancestors has been created
 *@return a list of ancestors.  The list may be empty.
 *@param familyRecord - a pointer to a GEDCOMobject struct
 *@param person - the Individual record whose descendants we want
 *@param maxGen - maximum number of generations to examine (must be >= 1)
 **/
List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen){
	List ancestors = initializeList(printGeneration, deleteGeneration, compareGenerations);
	if(familyRecord == NULL) 
		return ancestors;
	if(person == NULL){
		return ancestors;
	}
	Node* currentNode = person->families.head;
	while(currentNode != NULL){
		Family *fam = ((Family*)(currentNode->data));
		
		if(fam->husband != person && fam->wife != person){
			if(maxGen == 0){
				getParentListN(&ancestors, person, 0, 2147483600);
			}else{
				getParentListN(&ancestors, person, 0, maxGen);
			}
		}

		currentNode = currentNode->next;
	}
	return ancestors;
}

void getParentListN(List* ancestors,const Individual* person, unsigned int n, unsigned int max){

	if(n == max){
		return;
	}
	Node* currentNode = ancestors->head;
	for(int i = 1; i < n; i++){
		currentNode = currentNode->next;
	}
	List genList;
	if(currentNode == NULL){
		genList = initializeList(printIndividual, deleteIndividual, compareIndividuals);
		insertBack(ancestors, &genList);
	} else{
		genList = *(List*)(currentNode->data);
	}

	currentNode = person->families.head;
	while(currentNode != NULL){
		Family* fam = (Family*)(currentNode->data);

		if(fam->husband != person && fam->wife != person){

			Individual* father = malloc(sizeof(Individual));

			father->givenName = malloc(sizeof(fam->husband->givenName));
			father->surname = malloc(sizeof(fam->husband->surname));

			strcpy(father->givenName, fam->husband->givenName);
			strcpy(father->surname, fam->husband->surname);

			insertSorted(&genList, father);

			getParentListN(ancestors, fam->husband, n+1, max);

			Individual* mother = malloc(sizeof(Individual));

			mother->givenName = malloc(sizeof(fam->wife->givenName));
			mother->surname = malloc(sizeof(fam->wife->surname));

			strcpy(mother->givenName, fam->wife->givenName);
			strcpy(mother->surname, fam->wife->surname);

			insertSorted(&genList, mother);

			getParentListN(ancestors, fam->wife, n+1, max);
		}
		currentNode = currentNode->next;
	}

}

/** Function for converting an Individual struct into a JSON string
 *@pre Individual exists, is not null, and is valid
 *@post Individual has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param ind - a pointer to an Individual struct
 **/
char* indToJSON(const Individual* ind){
	char* temp;
	if(ind == NULL){
		temp = malloc(sizeof(""));
		strcpy(temp, "");
		return temp;
	}
	char firstName[200] = "";
	char lastName[200] = "";

	if(ind->givenName != NULL){
		strcat(firstName, ind->givenName);
	}
	if(ind->surname != NULL){
		strcat(lastName, ind->surname);
	}
	char output[450];
	sprintf(output, "{\"givenName\":\"%s\",\"surname\":\"%s\"}", firstName, lastName);
	temp = malloc(sizeof(output));
	strcpy(temp, output);

	return temp;
}	

/** Function for creating an Individual struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and an Individual struct has been created
 *@return a newly allocated Individual struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
Individual* JSONtoInd(const char* str){
	if(str == NULL){
		return NULL;
	}
	char givenName[200], surname[200];
	char firstChunk[20], secondChunk[20], thirdChunk[20];

	int i = 0;
	int ii = 0;

	while(i < 14){
		firstChunk[i] = str[i];
		firstChunk[i+1] = '\0';
		i++;
	}

	if(strcmp("{\"givenName\":\"", firstChunk)){
		return NULL;
	}

	while(str[i] != '\"' && i < strlen(str)){
		givenName[ii] = str[i];
		givenName[ii+1] = '\0';
		i++;
		ii++;
	}

	givenName[ii] = '\0';

	if(i == strlen(str)){
		return NULL;
	}

	int j = 0;
	while(j < 13){
		secondChunk[j] = str[i + j];
		secondChunk[j+1] = '\0';
		j++;
	}

	if(strcmp("\",\"surname\":\"", secondChunk)){
		return NULL;
	}

	i+= 13;
	ii = 0;

	while(str[i] != '\"'){
		surname[ii] = str[i];
		surname[ii+1] = '\0';
		i++;
		ii++;
	}

	if(i == strlen(str)){
		return NULL;
	}

	j = 0;
	while(j < 3){
		thirdChunk[j] = str[i + j];
		thirdChunk[j + 1] = '\0';
		j++;
	}
	
	if(strcmp(thirdChunk, "\"}")){
		return NULL;
	}

	Individual* temp = malloc(sizeof(Individual));
	temp->families = initializeList(printFamily, deleteFamily, compareFamilies);
	temp->otherFields = initializeList(printField, deleteField, compareFields);
	temp->events = initializeList(printEvent, deleteEvent, compareEvents);

	temp->givenName = malloc(sizeof(givenName));
	temp->surname = malloc(sizeof(surname));

	strcpy(temp->givenName, givenName);
	strcpy(temp->surname, surname);
	printf("%s, %s\n", givenName, surname);
	return temp;
}

/** Function for creating a GEDCOMobject struct from an JSON string
 *@pre String is not null, and is valid
 *@post String has not been modified in any way, and a GEDCOMobject struct has been created
 *@return a newly allocated GEDCOMobject struct.  May be NULL.
 *@param str - a pointer to a JSON string
 **/
GEDCOMobject* JSONtoGEDCOM(const char* str){
	if(str == NULL){
		return NULL;
	}
	GEDCOMobject* temp = malloc(sizeof(GEDCOMobject));

	char end[250];
	strcpy(end, "");

	char chunk[30];
	strcpy(chunk, "");

	int i = 0;
	int ii = 0;
	while(i < 11){
		chunk[i] = str[i];
		chunk[i+1] = '\0';
		i++; 
	}
	if(strcmp(chunk, "{\"source\":\"")){
		free(temp);
		return NULL;
	}
	ii = 0;
	while(str[i] != '\"' && i < strlen(str)){
		end[ii] = str[i];
		end[ii+1] = '\0';
		i++;
		ii++;
	}
	if(i == strlen(str)){
		free(temp);
	}

	temp->header = malloc(sizeof(Header));
	strcpy(temp->header->source, end);

	int n = i;
	ii = 0;
	strcpy(chunk, "");
	strcpy(end, "");
	while(i < n + 17){
		chunk[ii] = str[i]; 
		chunk[ii+1] = '\0';
		i++;
		ii++;
	}
	if(strcmp(chunk, "\",\"gedcVersion\":\"")){
		free(temp);
		free(temp->header);
		return NULL;
	}

	ii = 0;
	while(str[i] != '\"' && i < strlen(str)){
		end[ii] = str[i];
		end[ii+1] = '\0';
		i++;
		ii++;
	}

	if(i == strlen(str)){
		free(temp);
		free(temp->header);
		return NULL;
	}

	temp->header->gedcVersion = atof(end);

	n = i;
	ii = 0;
	strcpy(chunk, "");
	strcpy(end, "");
	while(i < n + 14){
		chunk[ii] = str[i]; 
		chunk[ii+1] = '\0';
		i++;
		ii++;
	}
	if(strcmp(chunk, "\",\"encoding\":\"")){
		free(temp);
		free(temp->header);
		return NULL;
	}
	ii = 0;
	while(str[i] != '\"' && i < strlen(str)){
		end[ii] = str[i];
		end[ii+1] = '\0';
		i++;
		ii++;
	}

	if(i == strlen(str)){
		free(temp);
		free(temp->header);
		return NULL;
	}

	if(strcmp(end, "ANSEL") == 0){
		temp->header->encoding = ANSEL;
	} else if(strcmp(end, "UTF-8") == 0 || strcmp(end, "UTF8") == 0){
		temp->header->encoding = UTF8;
	}else if(strcmp(end, "UNICODE") == 0){
		temp->header->encoding = UNICODE;
	}else if(strcmp(end, "ASCII") == 0){
		temp->header->encoding = ASCII;
	}else{
		free(temp);
		free(temp->header);
		return NULL;
	}

	n = i;
	ii = 0;
	strcpy(chunk, "");
	strcpy(end, "");
	while(i < n + 13){
		chunk[ii] = str[i]; 
		chunk[ii+1] = '\0';
		i++;
		ii++;
	}
	if(i == strlen(str)){
		free(temp);
		free(temp->header);
		return NULL;
	}
	if(strcmp(chunk, "\",\"subName\":\"")){
		free(temp);
		free(temp->header);
		return NULL;
	}

	ii = 0;
	while(str[i] != '\"' && i < strlen(str)){
		end[ii] = str[i];
		end[ii+1] = '\0';
		i++;
		ii++;
	}
	if(i == strlen(str)){
		free(temp);
		free(temp->header);
		return NULL;
	}

	temp->submitter = malloc(sizeof(Submitter));
	strcpy(temp->submitter->submitterName, end);


	n = i;
	ii = 0;
	strcpy(chunk, "");
	strcpy(end, "");
	while(i < n + 16){
		chunk[ii] = str[i]; 
		chunk[ii+1] = '\0';
		i++;
		ii++;
	}
	if(i == strlen(str)){
		free(temp);
		free(temp->header);
		return NULL;
	}
	if(strcmp(chunk, "\",\"subAddress\":\"")){
		free(temp);
		free(temp->header);
		return NULL;
	}

	ii = 0;
	while(str[i] != '\"' && i < strlen(str)){
		end[ii] = str[i];
		end[ii+1] = '\0';
		i++;
		ii++;
	}
	if(i == strlen(str)){
		free(temp);
		free(temp->header);
		return NULL;
	}

	temp->submitter = realloc(temp->submitter, sizeof(Submitter) + sizeof(char) * strlen(str) + 3);
	strcpy(temp->submitter->address, end);

	n = i;
	ii = 0;
	strcpy(chunk, "");
	while(i < n + 3){
		chunk[ii] = str[i]; 
		chunk[ii+1] = '\0';
		i++;
		ii++;
	}
	if(strcmp(chunk, "\"}")){
		free(temp);
		free(temp->header);
		return NULL;
	}

	temp->families = initializeList(printFamily, deleteFamily, compareFamilies);
	temp->individuals = initializeList(printIndividual,deleteIndividual, compareIndividuals);

	temp->header->submitter = temp->submitter;
	temp->header->otherFields = initializeList(printField, deleteField, compareFields);

	temp->submitter->otherFields = initializeList(printField, deleteField, compareFields);


	return temp;
}

/** Function for adding an Individual to a GEDCCOMobject
 *@pre both arguments are not NULL and valid
 *@post Individual has not been modified in any way, and its address had been added to GEDCOMobject's individuals list
 *@return void
 *@param obj - a pointer to a GEDCOMobject struct
 *@param toBeAdded - a pointer to an Individual struct
**/
void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded){
	if(obj == NULL || toBeAdded == NULL){
		return;
	}
	insertBack(&obj->individuals, (Individual*)toBeAdded);
}

/** Function for converting a list of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param iList - a pointer to a list of Individual structs
 **/
char* iListToJSON(List iList){
	Node* listNode = iList.head;

	//char buffer[1500];
	char* buffer = malloc(sizeof(char)*1500);
	strcpy(buffer,"[");

	while(listNode != NULL){
		Individual* indi = (Individual*)(listNode->data);
		char* toAdd = indToJSON(indi);
		strcat(buffer, toAdd);
		free(toAdd);

		listNode = listNode->next;
		if(listNode != NULL){
			strcat(buffer, ",");
		}
	}
	strcat(buffer, "]");
	buffer = realloc(buffer, sizeof(char)*strlen(buffer) + 3);
	return buffer;
}

/** Function for converting a list of lists of Individual structs into a JSON string
 *@pre List exists, is not null, and has been initialized
 *@post List has not been modified in any way, and a JSON string has been created
 *@return newly allocated JSON string.  May be NULL.
 *@param gList - a pointer to a list of lists of Individual structs
 **/
char* gListToJSON(List gList){
	char* result;
	result = malloc(sizeof(char)*1500*(1 + gList.length));
	strcpy(result, "[");

	Node* listNode = gList.head;
	if(listNode == NULL){
		strcat(result, "]\0");
		result = realloc(result, sizeof(char)*strlen(result) + 1);
		return result;
	}
	while(listNode != NULL){
		List resultList = *(List*)(listNode->data);
		char* temp = iListToJSON(resultList);
		strcat(result, temp);
		free(temp);
		listNode = listNode->next;	
		if(listNode != NULL){
			strcat(result, ",");
		}
	}
	strcat(result, "]\0");
	result = realloc(result, sizeof(char)*strlen(result) + 1);
	return result;
}



//*************************************************
// List Helper Functions
//*************************************************

void deleteEvent(void* toBeDeleted){

	Event *toDelete = ((Event*)(toBeDeleted));
	if(toDelete->date != NULL){
		//free(toDelete->date);
	}
	if(toDelete->place != NULL){
	//	free(toDelete->place);
	}

	Node* currentNode = toDelete->otherFields.head;
	Node* tempNode;
	while(currentNode != NULL){
		toDelete->otherFields.deleteData(currentNode->data);
		tempNode = currentNode;
		currentNode = currentNode->next;
		free(tempNode);
	}
	free(toDelete);
}

int compareEvents(const void* first,const void* second){
	Event *cFirst = ((Event*)(first));
	Event *cSecond = ((Event*)(second));

	char day1[3], month1[5], year1[5];
	char day2[3], month2[5], year2[5]; 

	parseDate(cFirst->date, day1, month1, year1);
	parseDate(cSecond->date, day2, month2, year2);

	int iYear1, iYear2;
	iYear1 = atoi(year1);
	iYear2 = atoi(year2);
	if(iYear1 > iYear2){
		return 1;
	} else if(iYear1 < iYear2){
		return -1;
	}else{
		int iMonth1, iMonth2;
		iMonth1 = monthToInt(month1);
		iMonth2 = monthToInt(month2);

		if(iMonth1 > iMonth2){
			return 1;
		} else if(iMonth1 < iMonth2){
			return -1;
		} else{
			int iDate1 = atoi(day1);
			int iDate2 = atoi(day2);
			if(iDate1 > iDate2){
				return 1;
			} else if (iDate1 < iDate2){
				return -1;
			} else{
				return 0;
			}
		}
	}

	
	return 0;
}

char* printEvent(void* toBePrinted){
	Event *printEvent = ((Event*)(toBePrinted));
	char* toPrint = malloc(sizeof(printEvent->type) + sizeof(printEvent->date) +  sizeof(printEvent->place) + (sizeof(char)*5));

	strcpy(toPrint, printEvent->type);
	strcat(toPrint, " ");
	strcat(toPrint, printEvent->date);
	strcat(toPrint, " ");
	strcat(toPrint, printEvent->place);

	return toPrint;
}

void deleteIndividual(void* toBeDeleted){
	Individual* indi = ((Individual*)(toBeDeleted));
	free(indi->givenName);
	free(indi->surname);

	Node* currentNode = indi->otherFields.head;
	Node* temp;
	while(currentNode != NULL){
		indi->otherFields.deleteData(currentNode->data);
		temp = currentNode;
		currentNode = currentNode->next;
		free(temp);
	}

	currentNode = indi->events.head;
	while(currentNode != NULL){
		indi->events.deleteData(currentNode->data);
		temp = currentNode;
		currentNode = currentNode->next;
		free(temp);
	}

}
int compareIndividuals(const void* first,const void* second){
	Individual *cFirst = ((Individual*)(first));
	Individual *cSecond = ((Individual*)(second));

	char* firstField = malloc(sizeof(cFirst->givenName) + sizeof(cFirst->surname) + (sizeof(char) * 3));
	strcpy(firstField, cFirst->givenName);
	strcat(firstField, ",");
	strcat(firstField, cFirst->surname);

	char* secondField = malloc(sizeof(cSecond->givenName) + sizeof(cSecond->surname) + (sizeof(char) * 3));
	strcpy(secondField, cSecond->givenName);
	strcat(secondField, ",");
	strcat(secondField, cSecond->givenName);

	int result = strcmp(firstField, secondField);

	free(firstField);
	free(secondField);

	return result;
}
char* printIndividual(void* toBePrinted){
	Individual *cPrint = ((Individual*)(toBePrinted));
	char* toPrint = malloc(sizeof(cPrint->givenName) + sizeof(cPrint->surname) + (sizeof(char) * 3));
	strcpy(toPrint, cPrint->givenName);
	strcat(toPrint, " ");
	strcat(toPrint, cPrint->surname);

	return toPrint;
}

void deleteFamily(void* toBeDeleted){

	Family* fam = ((Family*)(toBeDeleted));

	Node* currentNode = fam->otherFields.head;
	while(currentNode != NULL){
		fam->otherFields.deleteData(currentNode->data);
		if(currentNode->next != NULL){
			currentNode = currentNode->next;
			free(currentNode->previous);
		}else{
			free(currentNode);
		}
	}

	currentNode = fam->events.head;
	while(currentNode != NULL){
		fam->events.deleteData(currentNode->data);
		if(currentNode->next != NULL){
			currentNode = currentNode->next;
			free(currentNode->previous);
		}else{
			free(currentNode);
			currentNode = NULL;
		}
	}
	currentNode = fam->children.head;
	while(currentNode != NULL){
		if(currentNode->next != NULL){
			currentNode = currentNode->next;
			free(currentNode->previous);
		}else{
			free(currentNode);
			currentNode = NULL;
		}
	}

}
int compareFamilies(const void* first,const void* second){

	Family* fam1 = ((Family*)(first));
	Family* fam2 = ((Family*)(second));

	int firstSize = 0, secondSize = 0;

	if(fam1->wife != NULL){
		firstSize++;
	}
	if(fam2->wife != NULL){
		secondSize++;
	}
	if(fam1->husband != NULL){
		firstSize++;
	}
	if(fam2->husband != NULL){
		secondSize++;
	}

	firstSize += fam1->children.length;
	secondSize += fam2->children.length;

	if(firstSize > secondSize){
		return 1;
	} else if(firstSize < secondSize){
		return -1;
	} else
		return 0;
}

char* printFamily(void* toBePrinted){
	Family *fam = ((Family*)(toBePrinted));
	char* toPrint = malloc(sizeof(char) * 200);
	strcpy(toPrint, "");
	if(fam->wife != NULL){
		strcat(toPrint, "Wife: ");
		strcat(toPrint, fam->wife->givenName);
	}
	if(fam->husband != NULL){
		strcat(toPrint, "Husband: ");
		strcat(toPrint, fam->husband->givenName);
	}
	sprintf(toPrint, "%d children", fam->children.length);


	return toPrint;
}

void deleteField(void* toBeDeleted){
	Field *deleteField = ((Field*)(toBeDeleted));

	free(deleteField->tag);
	free(deleteField->value);
	free(deleteField);
}

int compareFields(const void* first,const void* second){

	Field *cFirst = ((Field*)(first));
	Field *cSecond = ((Field*)(second));

	char* firstField = malloc(sizeof(cFirst->tag) + sizeof(cFirst->value) + (sizeof(char) * 3));
	strcpy(firstField, cFirst->tag);
	strcat(firstField, " ");
	strcat(firstField, cFirst->value);

	char* secondField = malloc(sizeof(cSecond->tag) + sizeof(cSecond->value) + (sizeof(char) * 3));
	strcpy(secondField, cSecond->tag);
	strcat(secondField, " ");
	strcat(secondField, cSecond->value);

	int result = strcmp(firstField, secondField);

	free(firstField);
	free(secondField);

	return result;
}

char* printField(void* toBePrinted){
	Field *cPrint = ((Field*)(toBePrinted));
	char* toPrint = malloc(sizeof(cPrint->tag) + sizeof(cPrint->value) + (sizeof(char) * 3));
	strcpy(toPrint, cPrint->tag);
	strcat(toPrint, " ");
	strcat(toPrint, cPrint->value);

	return toPrint;
}

void deleteGeneration(void* toBeDeleted){
	List* toDelete = ((List*) toBeDeleted);

	Node* head = toDelete->head;
	Node* temp;
	while(head != NULL){
		List* indiList = ((List*)head->data);
		Node* indiNode = indiList->head;
		while(indiNode != NULL){
			indiList->deleteData(indiNode->data);
			temp = indiNode;
			indiNode = indiNode->next;
			free(indiNode);
		}
		temp = head;
		head = head->next;
		free(temp);
	}

}

char* printGeneration(void* toBePrinted){


	char* output = malloc(sizeof(char)*1500);
	List* printList = (List*)(toBePrinted);
	int n = 1;

	Node* currentNode = printList->head;

	while(currentNode != NULL){
		sprintf(output, "Generation %d\n", n);
		List* toPrint = ((List*)(currentNode->data));
		Node* subNode = toPrint->head;
		while(subNode != NULL){
			Individual* indi = (Individual*)(subNode->data);
			sprintf("%s  %s\n", indi->givenName, indi->surname);
			subNode = subNode->next;
		}
		currentNode = currentNode->next;
		n++;
	}
	strcat(output, "\0");
	output = realloc(output, sizeof(char)*strlen(output));
	return output;
}
int compareGenerations(const void* first,const void* second){
	int size1 = 0, size2 = 0;

	List* list1 = (List*)(first);
	List* list2 = (List*)(second);
	Node* currentNode = list1->head;
	while(currentNode != NULL){
		size1 += ((List*)(currentNode->data))->length;
		currentNode = currentNode->next;
	}
	currentNode = list2->head;
	while(currentNode != NULL){
		size2 += ((List*)(currentNode->data))->length;
		currentNode = currentNode->next;
	}

	return size1 - size2;
}

char* filetoJSON(char* filePath){
	const char* charType[] = {"ANSEL", "UTF8", "UNICODE", "ASCII"};
    GEDCOMobject* obj;
    GEDCOMerror error = createGEDCOM(filePath, &obj);
    if(error.type != OK){
    	free(obj);
    	char* returnStr = "{}";
    	return returnStr;
    }
    char source[250], encoding[10], subName[200], subAddr[200];
    float gedVersion = 0;
    int famSize = 0, indiSize = 0; 

    strcpy(source, obj->header->source);
    strcpy(encoding, charType[obj->header->encoding]);
    strcpy(subName, obj->submitter->submitterName);
    strcpy(subAddr, obj->submitter->address);
    gedVersion = obj->header->gedcVersion;
    famSize = obj->families.length;
    indiSize = obj->individuals.length;

    char toReturn[1000] = "";

    sprintf(toReturn, "{\"filePath\":\"%s\",\"source\":\"%s\",\"encoding\":\"%s\",\"subName\":\"%s\",\"subAddr\":\"%s\",\"gedVersion\":\"%.2lf\",\"famSize\":\"%d\",\"indiSize\":\"%d\"}",filePath, source, encoding, subName, subAddr, gedVersion, famSize, indiSize);
    char* returnStr = malloc(sizeof(char)*strlen(toReturn));
    strcpy(returnStr, toReturn);
    
    deleteGEDCOM(obj);
    free(obj);
    return returnStr;
}

char* getIndiList(char* filePath){
	GEDCOMobject* obj;
    GEDCOMerror error = createGEDCOM(filePath, &obj);
    
    if(error.type != OK){
    	free(obj);
    	char* returnStr = "{}";
    	return returnStr;
    }
    char* output = iListToJSON(obj->individuals);
    deleteGEDCOM(obj);
    free(obj);
    return output;
}

void simpleGED(char* json, char* fileName){
	GEDCOMobject* obj = JSONtoGEDCOM(json);
	writeGEDCOM(fileName, obj);
	deleteGEDCOM(obj);
	free(obj);
}

void addIndi(char* json, char* fileName){
	GEDCOMobject* obj;
	createGEDCOM(fileName, &obj);
	Individual* indi = JSONtoInd(json);
	addIndividual(obj, indi);
	writeGEDCOM(fileName, obj);
	deleteGEDCOM(obj);
	free(obj);
}

Individual* findIndi(GEDCOMobject* obj, Individual* indi){

	Node* currentNode = obj->individuals.head;
	while(currentNode != NULL){
		Individual* toCompare = ((Individual*)currentNode->data);
		if(strcmp(toCompare->givenName, indi->givenName) == 0  && strcmp(toCompare->surname, indi->surname) == 0){
				return toCompare;
		}
		currentNode = currentNode->next;
	}
	
	return NULL;
}

char* getDesc(char* json, char* fileName, int max){
	
	GEDCOMobject* obj; 
	createGEDCOM(fileName, &obj);
	
	Individual* indi = JSONtoInd(json);
	
	Individual* toSearch = findIndi(obj, indi);
	
	List descendents = getDescendantListN(obj, toSearch, max);
	
	return gListToJSON(descendents);
	
}

char* getAnces(char* json, char* fileName, int max){
	
	GEDCOMobject* obj; 
	createGEDCOM(fileName, &obj);
	
	Individual* indi = JSONtoInd(json);
	
	Individual* toSearch = findIndi(obj, indi);
	
	List ancestors = getAncestorListN(obj, toSearch, max);
	
	return gListToJSON(ancestors);
	
}


