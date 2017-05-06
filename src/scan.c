/*
 * Read output of a single lane+mid case
 */

#include "scan.h"
#include "db.h"
#include "hash/muttype.h"
#include <stdlib.h>
#include <string.h>
// The following filters will be eventually external, to be shared
#include <mutation/rawgroup.h>
#include <mutation/true.h>

int main(int argc, char **argv)
{
	FILE *f;
	int rc;
	sqlite3 *db;
	mutations m, hm;
	mutation_stats stats;
	
	memset(&m, 0, sizeof(mutations));
	memset(&hm, 0, sizeof(mutations));
	memset(&stats, 0, sizeof(mutation_stats));

	// Reopen db
	rc = sqlite3_open(argv[1], &db);
	if(rc) {
		fprintf(stderr, "sqlite err: %s\n", sqlite3_errmsg(db));
		return -1;
	}

	// output.txt
	f = fopen(argv[2], "r"); 
	if(f == NULL) {
		perror("can't open file\n");
		return -1;
	}
	fseek(f, 0, SEEK_SET);

	char *buf;
	size_t sz = 0;
	getline(&buf,&sz, f);

	char *pos_str;
	int pos;
	char mutname[7]; // will look like 'g_to_c'
	int readnum = 0;
	unsigned char c;
	do {
		c = fgetc(f);
		if (feof(f)) break;

		if(readnum) {
			size_t useful = 0;
			char *arr[3];
			int i_arr[3];
			int i;
			for(i = 0; i < 3; i++) {
				size_t idx = 0;
				arr[i] = malloc(1);
				do {
					arr[i][idx++] = c;
					arr[i] = realloc(arr[i], idx+1);
					c = fgetc(f);
					if(feof(f)) {
						break;
					}
				} while (c != ',' && c != '\n');
				arr[i][idx] = '\0';
				if(arr[i][0] == ',')
					memmove(arr[i], arr[i]+1, strlen(arr[i]+1) + 1);
				sscanf(arr[i], "%d", &i_arr[i]);
				free(arr[i]);
			}

			useful += within_rawgroup(i_arr[2]);
			useful += istrue_mutation(pos, mutname, i_arr);

			if(useful >= 2) {
				printf("%d\t%s\t%d %d %d", pos, mutname, i_arr[0], i_arr[1], i_arr[2]);
				add(&m, get_offset(mutname), i_arr[1]);
				add(&hm, get_offset(mutname), 1);
				printf("\n");
			}

			//printf("%d\t%s\t%d %d %d\n", pos, mutname, i_arr[0], i_arr[1], i_arr[2]);
			//printf("%c", c);
			if(c == '\n') readnum--; // done with numbers
		}

		if (c == '\n' && !readnum) {
			c = fgetc(f);
			size_t idx = 0;
			pos_str = malloc(1);
			do {
				pos_str[idx++] = c;
				pos_str = realloc(pos_str, idx+1);
				c = fgetc(f);
			} while (c != '.');
			pos_str[idx] = '\0';
			sscanf(pos_str, "%d", &pos);
			free(pos_str);
		}

		if(c == '.' && !readnum) {
			do {
				c = fgetc(f);
			} while (c != ',');
			c = fgetc(f);
			size_t idx = 0;
			do {
				if(c == ' ' || c == ':')
					c = '_';
				mutname[idx++] = c;
				c = fgetc(f);
			} while (c != ',');
			mutname[idx] = '\0';
			readnum++;
		}
	} while(1);
	//printf("%d %d\n", m.g_to_c, m.__to_g);
	updatedb(db, argv[3], &m, "");
	updatedb(db, argv[3], &hm, "hetero_");
	fclose(f);
	sqlite3_close(db);
	return 0;
}