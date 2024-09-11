#include "header.h"
int ITK_user_main(int argc, char* argv[])
{
	int iFail  = ITK_ok;
	int isum   = 0;
	int iCount = 0;

	char cpType[50];
	char* cpLine = NULL;

	std::string strOpLogFile = "";

	logical IsPathsValid = true;

	FILE* fpLogFile    = NULL;
	FILE* fpInputFile  = NULL;
	FILE* fpOutputfile = NULL;

	const char* cpUsername = ITK_ask_cli_argument("-u=");
	const char* cpPassword = ITK_ask_cli_argument("-p=");
	const char* cpGroup = ITK_ask_cli_argument("-g=");
	const char* cpFilePath = ITK_ask_cli_argument("-inputfile=");
	const char* cpOutputFile = ITK_ask_cli_argument("-outputfile=");

	strOpLogFile.assign(cpOutputFile).append(".log");
	fpLogFile = TC_fopen(strOpLogFile.c_str(), "w");

	if (fpLogFile == NULL)
	{
		printf("Error opening Log file...");
		TC_write_syslog("Error opening Log file.\n");
	}
	else
	{
		iFail = TC_init_module(cpUsername, cpPassword, cpGroup);

		fpInputFile = TC_fopen(cpFilePath, "r");
		if (fpInputFile == NULL)
		{
			IsPathsValid = false;
			printf("Could not open file %s\n", cpFilePath);
			TC_fprintf(fpLogFile, "\nCould not open input file: %s\n", cpFilePath);
		}

		fpOutputfile = TC_fopen(cpOutputFile, "w");
		if (fpOutputfile == NULL)
		{
			IsPathsValid = false;
			printf("Could not open file %s\n", cpOutputFile);
			TC_fprintf(fpLogFile, "Could not open output file %s\n", cpOutputFile);
		}

		if ((iFail == ITK_ok) && IsPathsValid)
		{
			printf("\n*************Started************\n");
			TC_fprintf(fpLogFile, "\n*************Started************\n");

			printf("\nThe input file %s is successfully opened\n", cpFilePath);
			TC_fprintf(fpLogFile, "\nThe input file: %s is successfully opened\n", cpFilePath);

			// Dynamically allocate memory for reading lines from the file
			cpLine = (char*)malloc(lineLen * sizeof(char));
			if (cpLine == NULL) 
			{
				printf("Memory allocation failed for line buffer\n");
				TC_fprintf(fpLogFile, "Memory allocation failed for line buffer\n");
			}
			else
			{
				TC_fgets(cpLine, lineLen, fpInputFile);
				TC_fprintf(fpOutputfile, "Type,Count, Start_ID, END_ID\n");

				while (TC_fgets(cpLine, lineLen, fpInputFile))
				{
					char* cpNewId = NULL;
					char** cpIds = NULL;
					logical* mod = NULL;

					cpLine[strcspn(cpLine, "\n")] = 0;

					if (sscanf_s(cpLine, "%49[^,], %d", cpType, (unsigned)_countof(cpType), &iCount) == 2)
					{
						TC_fprintf(fpLogFile, "Type: %s, Count: %d\n", cpType, iCount);
						ITKCALL(NR_next_value(cpType, "item_id", NULLTAG, "", "", "", NULLTAG, "", " ", &cpNewId), fpLogFile);

						TC_fprintf(fpLogFile, "\n The new ID is %s \n", cpNewId);
						printf("\n %s new ID is %s \n", cpType, cpNewId);

						isum = iCount;
						if (iCount > 1)
						{
							isum--;
							TC_fprintf(fpLogFile, "\n The Sum: %d \n", isum);
							TC_fprintf(fpLogFile, "\n********* Generating ID's **********\n");
							ITKCALL(NR_generate_item_ids(NULLTAG, cpType, isum, &mod, &cpIds), fpLogFile);
							TC_fprintf(fpOutputfile, "%s, %d, %s, %s\n", cpType, iCount, cpNewId, cpIds[isum - 1]);
						}
						printf("\n*****Completed*****\n");
						TC_fprintf(fpLogFile, "\n***Completed***\n");
					}
					else
					{
						printf("Error reading line: %s\n", cpLine);
					}
					SAFE_SM_FREE(cpNewId);
					SAFE_SM_FREE(cpIds);
					SAFE_SM_FREE(mod);
				}
				SAFE_SM_FREE(cpLine);
			}
			fclose(fpInputFile);
			fclose(fpOutputfile);
		}
		else
		{
			printf("Error in logging in to executing the utility. Please check the username, password and group. Verify that the file paths are valid.\n");
		}
		fclose(fpLogFile);
		ITKCALL(ITK_exit_module(true), fpLogFile);
	}
	return iFail;
}
