/*
Intel Open Source License 

Copyright (c) 2001 Intel Corporation 
All rights reserved. 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

   Neither the name of the Intel Corporation nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
/*
 * This program removes Ctrl-M, if any, from the end of each line.
 * (usually in a DOS-UNIX copy).
 *
 * It also adds a new line character to the last line of the UNIX
 * file to avoid compiler warnings on UNIX.
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZ 	2048

int main(int argc, char **argv)
{
	int length;
	char ch;
	char *inputfile;
	char buffer[BUF_SIZ];
	FILE *fp;

	if (argc < 2)
	{
		return(0);
	}

	inputfile = argv[argc-1];
	fprintf(stderr, "%s\n", inputfile);
	fp = fopen(inputfile, "ab+");
	if (!fp)
	{
		return(-1);
	}

/* 
 * First lseek() to the end of the file and add a newline if there
 * isn't any.
 */
	fseek(fp, -1, SEEK_END);
	ch = fgetc(fp);
	if (ch != 0x0a)
	{
		fseek(fp, 0, SEEK_END);
		fputc(0x0a, fp);
	}	 

	fseek(fp, 0, SEEK_SET);
 	memset(buffer, 0, BUF_SIZ); 
	while (fgets(buffer, BUF_SIZ, fp))
	{
		length = strlen(buffer);
		if (buffer[length-2] == 0x0d)
		{
			buffer[length-2] = '\n';
			buffer[length-1] = 0; 
		}
		printf("%s", buffer);
		memset(buffer, 0, BUF_SIZ); 
	}
	fclose(fp);
	return(0);
}
