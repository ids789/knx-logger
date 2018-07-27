/*
 *  knx-logger: 
 *  A simple tool to log telegrams sent over a knx bus
 *  Takes a url to a knxd server and an optional path to a
 *  group address file exported from ETS.  
 */

#include "eibclient.h"
#include "csvparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 200

/*
 *  error: 
 *    display an error message and exit
 */
void error(const char *msg) {
	fputs(msg, stderr);
	if (errno)
		fprintf(stderr, ": %s\n", strerror(errno));
	else
		putc('\n', stderr);
	exit(1);
}

/*
 *  print_address: 
 *    print the contents of a knx device address value
 */
void print_address(eibaddr_t addr) {
	printf("%d.%d.%d", (addr >> 12) & 0x0f,
			(addr >> 8) & 0x0f, (addr) & 0xff);
}

/*
 *  get_addr_str: 
 * convert a knx address into a string
 */
void group_name_str(eibaddr_t addr, char* buf) {
	sprintf(buf, "%d/%d/%d", (addr >> 11) & 0x1f,
			(addr >> 8) & 0x07, (addr) & 0xff);
}

/*
 *  lookup_group:
 *    Lookup a knx group address entry in an ETS csv file
 *    Inputs: group address csv file, lookup address, 
 *            buffers for: group address, group name and group data type
 */
void lookup_group(char* csv_path, char* group, char* name, char* type) {
	int is_found = 0;
	CsvParser *csvparser = CsvParser_new(csv_path, ",", 1);
    CsvRow *row;

    while ((row = CsvParser_getRow(csvparser)) ) {
        char **row_fields = CsvParser_getFields(row);

        if((CsvParser_getNumFields(row) == 7) &&
		   (strcmp(group, row_fields[1]) == 0)) {
			is_found = 1;
			strcpy(name, row_fields[0]);
			strcpy(type, row_fields[5]);
		}

        CsvParser_destroy_row(row);
		if(is_found)
			break;
	}

	if(!is_found){
	    strcpy(name, "");
		strcpy(type, "");
	}

    CsvParser_destroy(csvparser);
}

/*
 *  print_data: 
 *     decode a knx telegram payload and display it as text,
 *     if the type is unknown the content is displayed as hex
 */
void print_data(int len, char *data, char *type) {
	if (strcmp(type, "DPST-1-1") == 0) {  // switch type
		if (data[1] & 0x3F)
			printf("ON");
		else
			printf("OFF");
	}
	else if (strcmp(type, "DPST-9-1") == 0) {  // temperature type
		int d1 = ((unsigned char) data[2]) * 256 + (unsigned char) data[3];
		int m = d1 & 0x7ff;
		int ex = (d1 & 0x7800) >> 11;
		float temp = ((float) m * (1 << ex) / 100);
		printf ("%2.1f°C", temp);
	}
	else if (strcmp(type, "DPST-5-1") == 0) {  // light dim level type
		float val = 0.392* data[2];
		if(val < 0)
			val = 0;
		printf ("%2.0f%%", val);
	}
	else {  // unknown type
		for (int i = 2; i < len; i++)
			printf ("%02X ", data[i]);
		if(strlen(type) > 0)
			printf("(%s)", type);
	}
}

int main (int argc, char *argv[]) {
	int len;
	EIBConnection *con;
	eibaddr_t dest, src;
	char buf[BUF_SIZE];
	char timebuf[BUF_SIZE];
	char group_addr[21], group_name[200], group_type[200];

	time_t curtime;
	struct tm *loctime;

	if (argc < 2)
		error("usage: knx-logger url [group address file]");
	
	// open a connection to the knxd server
	con = EIBSocketURL(argv[1]);
	if (!con)
		error("Open failed");

	if (EIBOpen_GroupSocket(con, 0) == -1)
		error("Connect failed");

	while (1) {
		// receive a knx telegram
		len = EIBGetGroup_Src(con, sizeof (buf),
							  (unsigned char*)buf, &src, &dest);

		// log the current time
		curtime = time(NULL);
		loctime = localtime(&curtime);
		strftime(timebuf, BUF_SIZE, "%Y-%m-%d %X ", loctime);
		fputs(timebuf, stdout);
		
		if (len == -1)
			error ("Read failed");
		if (len < 2)
			error ("Invalid Packet");
		if (buf[0] & 0x3 || (buf[1] & 0xC0) == 0xC0) {
			printf ("Unknown APDU FROM ");
			print_address(src);
			printf (" TO ");
			group_name_str(dest, group_addr);
			printf ("%s\n", group_addr);
		}
		else {
			switch (buf[1] & 0xC0) {
			case 0x00:
				printf("Read");
				break;
			case 0x40:
				printf("Response");
				break;
			case 0x80:
				printf("Write");
				break;
			}

			group_name_str(dest, group_addr);
			if (argc == 3)
				lookup_group(argv[2], group_addr, group_name, group_type);
			else {
				strcpy(group_name, "");
				strcpy(group_type, "");
			}
			
			printf(" FROM ");
			print_address(src);
			
			if(strlen(group_name) == 0)
				printf(" TO %s", group_addr);
			else
				printf(" TO %s (%s)", group_addr, group_name);
			
			if (buf[1] & 0xC0) {
				printf(" VALUE ");
				print_data(len, buf, group_type);
			}
			
			printf("\n");
			fflush(stdout);
		}
	}

	EIBClose(con);
	return 0;
}
