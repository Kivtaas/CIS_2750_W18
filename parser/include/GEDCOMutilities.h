#include "GEDCOMparser.h"
#include "LinkedListAPI.h"
#include <stdbool.h>


typedef struct xRef{
	char* tag;
	Individual* indi;
	Family* fam;
	struct xRef* next;
} xRef;

typedef struct{
	int line;
	char ref[20];
} famRef;

typedef struct{
	char type[5];
	char ref[20];
	int line;
	Family* pointer;

} famLink;

typedef struct lineObj{
	int num;
	char* type;
	char* end;
	int lineNum;

	struct lineObj* next;
	struct lineObj* prev;

} lineObj;



void parseLine(char* line, char* num, char* type, char* end);
void clearStrings(char* num, char* type, char* end);
void clearLine(char* line);
int validEvent(char* event, int sub);
int monthToInt(char* month);
void getChildren(List* descendants,const Individual* person);

GEDCOMerror parseFile(FILE* fp, lineObj** lines, GEDCOMerror* error);
GEDCOMerror parseHeader(lineObj** lines, Header* header);
GEDCOMerror parseSubmitters(GEDCOMobject* obj, lineObj* line);
GEDCOMerror parseIndi(GEDCOMobject* obj, lineObj* line, xRef** refs);
GEDCOMerror parseFamily(GEDCOMobject* obj, lineObj* line,  xRef* refs);
char readLine(char* buffer, FILE* fp);

//***************************************GEDCOM WRAPPER FUNCTIONS*********************************************
char* filetoJSON(char* filePath);
char* getIndiList(char* filePath);
void simpleGED(char* json, char* fileName);
void addIndi(char* json, char* fileName);
char* getDesc(char* json, char* fileName, int max);
//************************************************************************************************************

void writeHeader(lineObj** lines, const GEDCOMobject* obj);
void addLines(lineObj** lines, int stageNum, char* type, char* end);
void writeSubmitter(lineObj* lines, const GEDCOMobject* obj);
void writeIndividuals(lineObj* lines, const GEDCOMobject* obj, xRef* refs);
void writeFamily(lineObj* lines, const GEDCOMobject* obj, xRef* refs);
void populateXRef(xRef** refs, const GEDCOMobject* obj);
void getChildrenN(List* descendants,const Individual* person, unsigned int n, unsigned int max);
void getParentListN(List* ancestors,const Individual* person, unsigned int n, unsigned int max);
