#include "header.h"
int ITK_user_main(int argc, char* argv[])
{
	FILE* fpLogFile;
	fpLogFile = TC_fopen("C:\\Temp\\ITKLOGFILE.txt", "w");
	if (fpLogFile == NULL)
	{
		TC_write_syslog("Error opening file.\n");
		return false;
	}
	char* cpUsername = NULL;
	char* cpPassword = NULL;
	char* cpGroup = NULL;
	char* cpPref = NULL;
	char* cpTimeStamp = NULL;
	char* cpNewId = NULL;
	char* cpTypename = NULL;
	char* cpFilePath = NULL;
	char* cpOutputFile = NULL;
	char** cpIds = NULL;
	char* cpLine = NULL;
	char cpType[50];
	tag_t tItem = NULL;
	tag_t tRevTag = NULL;
	tag_t tIDgenearate = NULL;
	int isum;
	int iCount;
	logical* mod;

	cpUsername = ITK_ask_cli_argument("-u=");
	cpPassword = ITK_ask_cli_argument("-p=");
	cpGroup = ITK_ask_cli_argument("-g=");
	cpFilePath = ITK_ask_cli_argument("-inputfile=");
	cpOutputFile = ITK_ask_cli_argument("-outputfile=");

	if (cpUsername == NULL)
	{
		printf("\n ***Incorrect cpUsername*** \n");
		return 1;
	}
	else if (cpPassword == NULL)
	{
		printf("\n ***Incorrect cpPassword*** \n");
		return 1;
	}
	else if (cpGroup == NULL)
	{
		printf("\n ***Incorrect cpGroup*** \n");
		return 1;
	}
	else if (cpGroup == NULL)
	{
		printf("\n ***Incorrect cpGroup*** \n");
		return 1;
	}
	else if (cpFilePath == NULL)
	{
		printf("\n ***Incorrect cpGroup*** \n");
		return 1;
	}
	else if (cpOutputFile == NULL)
	{
		printf("\n ***Incorrect cpOutputFile*** \n");
		return 1;
	}

	TC_fprintf(fpLogFile, "\n **************started************ \n");
	printf("\n ***Started*** \n");
	printf("\n filePath %s \n", cpFilePath);
	TC_fprintf(fpLogFile, "\n filePath %s \n", cpFilePath);
	FILE* file = TC_fopen(cpFilePath, "r");
	if (file == NULL) {
		printf("Could not open file %s\n", cpFilePath);
		TC_fprintf(fpLogFile, "\n Could not open file % s\n", cpFilePath);
		return 1;
	}
	FILE* Outputfile = TC_fopen(cpOutputFile, "w");
	if (file == NULL) {
		printf("Could not open file %s\n", cpOutputFile);
		TC_fprintf(fpLogFile, "Could not open file %s\n", cpOutputFile);
		return 1;
	}
	else
	{
		printf("File Opened Successfuly%s\n", cpFilePath);
		printf("File Opened Successfuly%s\n", cpOutputFile);
		TC_fprintf(fpLogFile, "File Opened Successfuly % s\n", cpFilePath);
		TC_fprintf(fpLogFile, "File Opened Successfuly%s\n", cpOutputFile);


		// Dynamically allocate memory for reading lines from the file
		cpLine = (char*)malloc(lineLen * sizeof(char));
		if (cpLine == NULL) {
			printf("Memory allocation failed for line buffer\n");
			TC_fprintf(fpLogFile, "Memory allocation failed for line buffer\n");
			fclose(file);
			return 1;
		}
		TC_fgets(cpLine, lineLen, file);

		TC_fprintf(Outputfile, "Type,Count, Start_ID, END_ID\n");
		
	
		while (TC_fgets(cpLine, lineLen, file)) {
			
			cpLine[strcspn(cpLine, "\n")] = 0;

			if (sscanf_s(cpLine, "%49[^,], %d", cpType, (unsigned)_countof(cpType), &iCount) == 2) {
				TC_fprintf(fpLogFile, "Type: %s, Count: %d\n", cpType, iCount);
				TC_fprintf(fpLogFile, "\n cpUsername %s, cpPassword %s, cpGroup %s\n", cpUsername, cpPassword, cpGroup);
				ITKCALL(TC_init_module(cpUsername, cpPassword, cpGroup), fpLogFile);
				ITKCALL(NR_next_value(cpType, "item_id", NULLTAG, "", "", "", NULLTAG, "", " ", &cpNewId), fpLogFile);
				TC_fprintf(fpLogFile, "\n new ID is %s \n", cpNewId);
				printf("\n %s new ID is %s \n", cpType, cpNewId);
				isum = iCount;
				if (iCount > 1)
				{
					isum--;
					TC_fprintf(fpLogFile, "\n The Sum is %d \n", isum);
					TC_fprintf(fpLogFile, "\n********* Generating ID's **********\n");
					ITKCALL(NR_generate_item_ids(NULLTAG, cpType, isum, &mod, &cpIds), fpLogFile);
					
				}
				ITKCALL(ITK_exit_module(true), fpLogFile);
				TC_fprintf(Outputfile, "%s, %d, %s, %s\n", cpType, iCount, cpNewId, cpIds[isum - 1]);
				printf("\n ***Completed*** \n");
				TC_fprintf(fpLogFile, "\n ***Completed*** \n");
				
			}
			

			else {
				printf("Error reading line: %s\n", cpLine);
			}
		}
		//free mem
		free(cpLine);
		SAFE_SM_FREE(cpIds);
		////close opened files
		fclose(fpLogFile);
		fclose(Outputfile);
		fclose(file);

		return 0;
	}

}
