#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.
//
// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

void argvToPerson(int argc, char *args[], Person *p) {
	int dataSize = argc - 3;

	if (dataSize != 6) {
		printf("Invalid input error\n");
		exit(-1);
	}

	strcpy(p->id, args[3]);
	strcpy(p->name, args[4]);
	strcpy(p->age, args[5]);
	strcpy(p->addr, args[6]);
	strcpy(p->phone, args[7]);
	strcpy(p->email, args[8]);
}

void strcatdvd(char *dest, const char *src, char divider) {
	strcat(dest, src);
	dest[strlen(dest)] = divider;
}

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, 16 + pagenum * PAGE_SIZE, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, 16 + pagenum * PAGE_SIZE, SEEK_SET);
	fwrite((void *)pagebuf, PAGE_SIZE, 1, fp);
}

int readHeader(FILE *fp, int offset) {
	int temp;
	fseek(fp, offset * 4, SEEK_SET);
	fread(&temp, sizeof(int), 1, fp);
	return temp;
}

void writeHeader(FILE *fp, int value, int offset) {
	fseek(fp, offset * 4, SEEK_SET);
	fwrite(&value, sizeof(int), 1, fp);
}

void readRecord(char* pagebuf, char* recordbuf, int offset, int len) {
	memcpy(recordbuf, pagebuf + HEADER_AREA_SIZE + offset, len);
}

void writeRecord(char* pagebuf, char* recordbuf, int offset, int len) {
	memcpy(pagebuf + HEADER_AREA_SIZE + offset, recordbuf, len);
}

int getOffset(const char* pagebuf, int pos) {
	int temp;
	memcpy(&temp, pagebuf + 4 + pos * 8, sizeof(int));
	return temp;
}

int getLength(const char* pagebuf, int pos) {
	int temp;
	memcpy(&temp, pagebuf + 8 + pos * 8, sizeof(int));
	return temp;
}

int getRecordCnt(const char* pagebuf) {
	int temp;
	memcpy(&temp, pagebuf, sizeof(int));
	return temp;
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	char *temp = (char *)malloc(MAX_RECORD_SIZE);
	strcatdvd(temp, p->id, '#');
	strcatdvd(temp, p->name, '#');
	strcatdvd(temp, p->age, '#');
	strcatdvd(temp, p->addr, '#');
	strcatdvd(temp, p->phone, '#');
	strcatdvd(temp, p->email, '#');
	printf("Packed: %s\n", temp);

	strcpy(recordbuf, temp);
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
void unpack(const char *recordbuf, Person *p)
{
	int dvdcnt = 0, bufseek = 0;
	char* records[6];
	
	for(int i = 0; i < 6; i++)
		records[i] = (char *)malloc(sizeof(MAX_RECORD_SIZE));


	while (dvdcnt != 6) {
		for (int seek = 0; recordbuf[bufseek] != '#'; seek++, bufseek++) {
			records[dvdcnt][seek] = recordbuf[bufseek];
		}
		
		bufseek++, dvdcnt++;
	}

	strcpy(p->id, records[0]);
	strcpy(p->name, records[1]);
	strcpy(p->age, records[2]);
	strcpy(p->addr, records[3]);
	strcpy(p->phone, records[4]);
	strcpy(p->email, records[5]);
}

void addAppend(FILE *fp, const Person *p, int header) {

	while(1) {
		// 문자열 동적 할당
		char *pagebuf = (char *)malloc(PAGE_SIZE);
		char *recordbuf = (char *)malloc(MAX_RECORD_SIZE);

		// 사용할 변수들 생성
		int cnt, loffset = 0, llen = 0, offset = 0, recordlen;

		// 페이지 불러오기
		readPage(fp, pagebuf, header);

		// 현재 페이지에 몇 개의 레코드가 있는지 불러오기
		cnt = getRecordCnt(pagebuf);
		printf("CNT : %d\n", cnt);

		// 만약에 비어있는 페이지가 아니라면
		if (cnt != 0) {

			// Header Area의 바로 전 레코드의 Offset과 Length를 불러옴
			loffset = getOffset(pagebuf, cnt - 1);
			llen = getLength(pagebuf, cnt - 1);
			//memcpy(&loffset, pagebuf + 4 + 8 * (cnt - 1), sizeof(int));
			//memcpy(&llen, pagebuf + 8 + 8 * (cnt - 1), sizeof(int));
			
			// 이제 입력할 예정인 레코드의 offset을 설정
			offset = loffset + llen;
		}

		// Person 구조체 packing
		pack(recordbuf, p);

		// 입력하려는 레코드의 길이
		recordlen = strlen(recordbuf);

		if(recordlen + offset > DATA_AREA_SIZE || (4 + 8 * (cnt + 1)) > HEADER_AREA_SIZE) {
			writeHeader(fp, ++header + 1, 0);
			continue;
		}

		// 레코드 길이와 오프셋 내용 저장
		memcpy(pagebuf + 4 + 8 * cnt, &offset, sizeof(int));
		memcpy(pagebuf + 8 + 8 * cnt, &recordlen, sizeof(int));
		//memcpy(pagebuf + HEADER_AREA_SIZE + offset, recordbuf, recordlen);
		writeRecord(pagebuf, recordbuf, offset, recordlen);

		cnt++;
		memcpy(pagebuf, &cnt, sizeof(int));

		//memcpy(pagebuf + HEADER_AREA_SIZE + offset, recordbuf, strlen(recordbuf));

		writePage(fp, pagebuf, header);
		writeHeader(fp, readHeader(fp, 1) + 1, 1);
		printf("Now: %d, loffset : %d, llen : %d, offset : %d, recordlen : %d, Header: %d\n\n", 4 + 8 * cnt, loffset, llen, offset, recordlen, header);

		break;
	}
}

void addModify(FILE *fp, const Person *p, int pageH, int recordH) {
	char* recordbuf = (char *)malloc(MAX_RECORD_SIZE);
	char* pagebuf = (char *)malloc(PAGE_SIZE);

 	readPage(fp, pagebuf, pageH);
	pack(recordbuf, p);

	int llen = getLength(pagebuf, recordH);
	int loffset = getOffset(pagebuf, recordH);
	//memcpy(&llen, pagebuf + 8 + recordH * 8, sizeof(int));

	int result[2];
	memcpy(&result[0], pagebuf + HEADER_AREA_SIZE + loffset + 1, sizeof(int));
	memcpy(&result[1], pagebuf + HEADER_AREA_SIZE + loffset + 5, sizeof(int));
	printf("%d %d\n", pageH, recordH);
	printf("%d %d\n", result[0], result[1]);
	printf("%d %d\n", llen, strlen(recordbuf));

	if(pageH == -1 && recordH == -1) {
		addAppend(fp, p, readHeader(fp, 0) - 1);
		return;
	}

	if(llen < strlen(recordbuf)) {
		//memcpy(&loffset, pagebuf + 4 + recordH * 8, sizeof(int));
		addModify(fp, p, result[0], result[1]);

	} else {
		int size = strlen(recordbuf);
		memcpy(pagebuf + HEADER_AREA_SIZE + loffset, recordbuf, llen);
		memcpy(pagebuf + 8 + recordH * 8, &size, sizeof(int));
		writeHeader(fp, result[0], 2);
		writeHeader(fp, result[1], 3);
		writePage(fp, pagebuf, pageH);
	}

}

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void add(FILE *fp, const Person *p)
{
	int header[4];

	for(int i = 0; i < 4; i++) {
		header[i] = readHeader(fp, i);
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);

	if(size == 0) {
		memset(header, 0, sizeof(int) * 4);
		writeHeader(fp, 1, 0);
		writeHeader(fp, --header[2], 2);
		writeHeader(fp, --header[3], 3);
	}

	// 아직 삭제한 레코드가 없을 경우
	if(header[2] == -1 && header[3] == -1) {
		addAppend(fp, p, header[0] - 1);
		for(int i = 0; i < 4; i++) {
			header[i] = readHeader(fp, i);
		}
		printf("%d %d %d %d\n", header[0], header[1], header[2], header[3]);
		return;
	}

	// 삭제한 레코드가 있을 경우
	addModify(fp, p, header[2], header[3]);
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	printf("Delete called!\n");
	int pagecnt = readHeader(fp, 0);
	int recordcnt = readHeader(fp, 1);

	printf("[*] pagecnt: %d\n[*] recordcnt: %d\n", pagecnt, recordcnt);
	for(int i = 0; i < pagecnt; i++) {
		// 페이지 읽어오기
		char* pagebuf = (char *)malloc(PAGE_SIZE);
		readPage(fp ,pagebuf, i);

		// record 개수 불러오기
		int recordcnt = getRecordCnt(pagebuf);
		
		for (int j = 0; j < recordcnt; j++) {
			int offset = getOffset(pagebuf, j);
			int size = getLength(pagebuf, j);

			char* recordbuf = (char *)malloc(MAX_RECORD_SIZE);
			readRecord(pagebuf, recordbuf, offset, size);
			printf("[*] record: %s\n", recordbuf);

			Person *p = (Person *)malloc(sizeof(Person));
			unpack(recordbuf, p);
			printf("%s %s\n", p->id, id);
			if(strcmp(p->id, id) == 0) {
				printf("[*] remove record called! Progress: \n");
				int n = readHeader(fp, 2);
				int m = readHeader(fp, 3);
				memcpy(recordbuf, "*", 1);
				memcpy(recordbuf + 1, &n, sizeof(int));
				memcpy(recordbuf + 1 + sizeof(int), &m, sizeof(int));
				writeRecord(pagebuf, recordbuf, offset, size);
				writePage(fp, pagebuf, i);
				writeHeader(fp, i, 2);
				writeHeader(fp, j, 3);
				printf("Done\n");
				break;
			}
		}
	}

	printf("\n\n");
}

void printRecord(FILE* fp) {
	int header[4];
	for(int i = 0; i < 4; i++) {
		header[i] = readHeader(fp, i);
	}

	printf("[*] Header: %d %d %d %d\n", header[0], header[1], header[2], header[3]);

	for(int i = 0; i < header[0]; i++) {
		printf("\n[!] Page %d\n", i);
		char* pagebuf = (char *)malloc(PAGE_SIZE);
		readPage(fp, pagebuf, i);

		int cnt;
		memcpy(&cnt, pagebuf, sizeof(int));
		printf("[*] #records : %d\n", cnt);
		
		for(int j = 0; j < cnt; j++) {
			int off, len;
			memcpy(&off, pagebuf + 4 + j * 8, sizeof(int));
			memcpy(&len, pagebuf + 8 + j * 8, sizeof(int));
			printf("=======================\n");
			printf("[*] %d - offset: %d, length: %d\n", j, off, len);
			char* recordbuf = (char *)malloc(MAX_RECORD_SIZE);
			memcpy(recordbuf, pagebuf + off + HEADER_AREA_SIZE, len);
			Person* p = (Person *)malloc(sizeof(Person));
			unpack(recordbuf, p);
			printf("[*] id  : %s\n", p->id);
			printf("[*] name: %s\n", p->name);
			printf("[*] age : %s\n", p->age);
			printf("[*] addr: %s\n", p->addr);
			printf("[*] phne: %s\n", p->phone);
			printf("[*] mail: %s\n", p->email);
			printf("=======================\n");
		}
	}
	printf("\n\n");
	
}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터

	if((fp = fopen(argv[2], "r+b")) == NULL) {
		if((fp = fopen(argv[2], "w+b")) == NULL) {
			printf("Error occurred\n");
			exit(-1);
		}
	}

	Person* p = (Person *)malloc(sizeof(Person));

	char* recordbuf = (char *)malloc(MAX_RECORD_SIZE);
	
	

	if(argv[1][0] == 'a') {
		argvToPerson(argc, argv, p);
		add(fp, p);
	} else {
		delete(fp, argv[3]);
	}
	printRecord(fp);

	fclose(fp);
	return 0;
}

//./a.out a person.dat "8811032129018" "GD Hong" "23" "Seoul" "02-820-0924" "gdhong@ssu.ac.kr"
//./a.out d person.dat "8811032129018"