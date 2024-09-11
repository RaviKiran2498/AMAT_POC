/*********************************************************************************************************
  * This Program is used to Attach the Solution Item Revision to HON ECN
	Input File Header: CN_NO,CN_REV,REVID,DOC_REV,RELATION
  ********************************************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tccore/custom.h>
#include <tccore/method.h>
#include <base_utils/mem.h>
#include <tccore/tctype.h>
#include <tccore/item.h>
#include <tccore/item_errors.h>
#include <time.h>
#include <tccore/aom_prop.h>
#include <tcinit/tcinit.h>
#include <tccore/aom.h>
#include <pom/pom/pom.h>
#include<ae/dataset.h>
#include <tccore/grm.h>
#include <user_exits/epm_toolkit_utils.h>


#define BUFFER_SIZE 1000

#define COLUMN_COUNT 9

void honPrintUsage();

int ITK_user_main(int argc, char* argv[])
{
	int ifail = ITK_ok;
	int iRetcode = ITK_ok;

	const char* fileName = ITK_ask_cli_argument("-i=");
	const char* sUser = ITK_ask_cli_argument("-u=");
	const char* sPassword = ITK_ask_cli_argument("-p=");
	const char* sGroup = ITK_ask_cli_argument("-g=");
	const char* finalOPLog = ITK_ask_cli_argument("-o="); //final log file ptr

	FILE* ipPtr = NULL;
	FILE* finalLogPtr = NULL;
	FILE* errorLogPtr = NULL;		
	FILE* inputFile = NULL;

	errno_t err;
	errno_t err1;
	errno_t err2;
	errno_t err3;

	int iSuccessCount = 0;
	int iFailureCount = 0;

	char buffer[BUFFER_SIZE];
	char cTempbuffer[BUFFER_SIZE];
	char* token = NULL;
	char* headertoken = NULL;
	char* cEachlinetoken = NULL;

	size_t fileNamelength = 0;

	int iLineCount = 0;
	int iHeadercount = 0;
	int iEachlinecount = 0;
	int iIndex = 0;

	char* PARENTITEMID = NULL;
	char* PARENTITEMREVID = NULL;
	char* CHILDITEMID = NULL;
	char* CHILDITEMREVID = NULL;
	char* CHILDITEMTYPE = NULL;
	char* PARENTITEMTYPE = NULL;
	char* PARENT_DATASETNAME = NULL;
	char* PARENT_DATASETTYPE = NULL;
	char* CHILD_DATASETNAME = NULL;
	char* CHILD_DATASETTYPE = NULL;
	char* RELATION = NULL;
	char* message = NULL;


	const char* errorOPLog = "SWIM_RelationCreation_Failure2.log";
	char** ccHeadertokens = (char**)MEM_alloc(COLUMN_COUNT * sizeof(char*));


	int iGRMSflag = FALSE;
	size_t grmswordlength = 0;
	int isValidHeader = FALSE;

	char grmssearchword[5] = "grms";  //null char included 4 + 1;
	int iGrmsfound = 0;

	const char** ccAttrs = (const char**)MEM_alloc(1 * sizeof(char*));
	const char** ccValues = (const char**)MEM_alloc(1 * sizeof(char*));

	double time_taken;
	clock_t t;
	t = clock();

	if (sUser == NULL || sPassword == NULL || sGroup == NULL || fileName == NULL)
	{
		honPrintUsage();
		return 1;
	}

	if (tc_strcmp(argv[0], "-h") == 0)
	{
		honPrintUsage();
		exit(0);
	}
	err = fopen_s(&ipPtr, fileName, "r");

	if (err != 0)
	{
		printf("Error! opening file");
		exit(1);
	}

	err1 = fopen_s(&finalLogPtr, finalOPLog, "w");  //change file path
	err2 = fopen_s(&errorLogPtr, errorOPLog, "w");  //change file path

	if ((err1 != 0) || (err2 != 0))
	{
		printf("Error! opening file");
		exit(1);
	}
	fprintf(finalLogPtr, "Object List:\n");
	fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", "Primary Object", "Primary Object pUID", "Secondary Object", "Secondary Object pUID", "Relation Type Name", "Saved Relation PUID", "Status");
	printf("\nFiles created successfully");

	fileNamelength = strlen(fileName);
	grmswordlength = strlen(grmssearchword);

	for (int iEachChariIndex = 0; iEachChariIndex < fileNamelength - grmswordlength; iEachChariIndex++)
	{
		iGrmsfound = 1;
		for (int iEachChariIndexInGrmsWord = 0; iEachChariIndexInGrmsWord < grmswordlength; iEachChariIndexInGrmsWord++)
		{
			if (fileName[iEachChariIndex + iEachChariIndexInGrmsWord] != grmssearchword[iEachChariIndexInGrmsWord])
			{
				iGrmsfound = 0;
				break;
			}
		}
		if (iGrmsfound == 1)
		{
			iGRMSflag = TRUE;
		}
	}

	iLineCount = 1;

	while (fgets(buffer, BUFFER_SIZE, ipPtr) != NULL)
	{
		iIndex = -1;
		int iChariIndex = 0;
		while (buffer[iChariIndex] != '\0')
		{
			if (buffer[iChariIndex] != ' ' && buffer[iChariIndex] != '\t' && buffer[iChariIndex] != '\n')
			{
				iIndex = iChariIndex;
			}

			iChariIndex++;
		}

		buffer[iIndex + 1] = '\0';

		strcpy_s(cTempbuffer, buffer);

		printf("\nRunning line no. : %d", iLineCount);

		if (iLineCount < 2)
		{
			iHeadercount = 0;
			headertoken = tc_strtok(buffer, "|");
			char* eachHeaderToken = nullptr;
			//--------Change Code-----------
			while (headertoken != NULL)
			{
				eachHeaderToken = (char*)MEM_alloc((strlen(headertoken) + 1) * sizeof(char));
				if (eachHeaderToken == nullptr) {
					printf("NULLPTR Each header token\n");
				}

				strcpy_s(eachHeaderToken, sizeof(eachHeaderToken), headertoken);

				ccHeadertokens[iHeadercount] = eachHeaderToken;
				iHeadercount++;
				headertoken = tc_strtok(NULL, "|");
			}
			//-----------Till Here----------

			if (tc_strcmp(cTempbuffer, "PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|PARENT_DATASETNAME|PARENT_DATASETTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION") == 0)
			{
				isValidHeader = TRUE;
				printf("\nValid Header...\n");
				printf("Processing the file...\n");
			}
			else
			{
				isValidHeader = FALSE;
				printf("\nPlease select the input file with correct header PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|PARENT_DATASETNAME|PARENT_DATASETTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION\n");
				exit(0);
			}
		}
		else if (iLineCount > 1)
		{
			if (iHeadercount == COLUMN_COUNT && iGRMSflag == TRUE && isValidHeader == TRUE)
			{
				strcpy_s(cTempbuffer, sizeof(cTempbuffer), buffer);
				iEachlinecount = 0;
				cEachlinetoken = tc_strtok(buffer, "|");

				while (cEachlinetoken != NULL)
				{
					if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENTITEMID") == 0)
					{
						PARENTITEMID = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENTITEMREVID") == 0)
					{
						PARENTITEMREVID = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENTITEMTYPE") == 0)
					{
						PARENTITEMTYPE = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENT_DATASETNAME") == 0)
					{
						PARENT_DATASETNAME = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENT_DATASETTYPE") == 0)
					{
						PARENT_DATASETTYPE = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILDITEMID") == 0)
					{
						CHILDITEMID = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILDITEMREVID") == 0)
					{
						CHILDITEMREVID = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILDITEMTYPE") == 0)
					{
						CHILDITEMTYPE = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILD_DATASETNAME") == 0)
					{
						CHILD_DATASETNAME = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILD_DATASETTYPE") == 0)
					{
						CHILD_DATASETTYPE = cEachlinetoken;
					}
					else if (tc_strcmp(ccHeadertokens[iEachlinecount], "RELATION") == 0)
					{
						RELATION = cEachlinetoken;
					}
					iEachlinecount++;
					cEachlinetoken = tc_strtok(NULL, "|");
				}
			}
			else
			{
				printf("Please check header values and filename \n");
				return 1;
			}

			if ((iRetcode = ITK_init_module(sUser, sPassword, sGroup)) == ITK_ok)
			{
				char* objectPUID = NULL;
				char* relationTypeName = NULL;
				ITK_set_bypass(true);


				tag_t* parentItemsTags = NULL;
				tag_t* Dtags = NULL;

				tag_t tRelationTag = NULLTAG;
				tag_t relation_type = NULLTAG;
				tag_t parentrevtag = NULLTAG;
				tag_t datasettag = NULLTAG;


				int iParentItemCount = 0;
				int iDatasetCount = 0;

				printf("\nExecuting for data : %s,%s,%s,%s,%s,%s,%s,%s,%s", PARENTITEMID, PARENTITEMREVID, PARENTITEMTYPE,
					PARENT_DATASETNAME, PARENT_DATASETTYPE, CHILDITEMID, CHILDITEMREVID, CHILDITEMTYPE, CHILD_DATASETNAME, CHILD_DATASETTYPE, RELATION);



				//Extra Test code
				char* cSecondaryObjectUID = NULL;
				//extra end

				tag_t childrevtag = NULLTAG;
				tag_t* childItemTags = NULL;
				int childItemCount = 0;
				char* childrevitem_type = NULL;

				ccAttrs[0] = "item_id";
				ccValues[0] = (char*)CHILDITEMID;

				ifail = ITEM_find_item_revs_by_key_attributes(1, ccAttrs, ccValues, CHILDITEMREVID, &childItemCount, &childItemTags);
				if (ifail != ITK_ok)
				{
					iFailureCount++;
					fprintf(errorLogPtr, "%s", cTempbuffer);
					fprintf(errorLogPtr, " ----->  ");
					fprintf(errorLogPtr, "Error in fetching childItemTags \n");
					fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, "NULL", RELATION, "NULL", "Failed");
					fflush(finalLogPtr);
					fflush(errorLogPtr);
					return ifail;
				}
				printf("\nQRY Child Rev Count : %d", childItemCount);

				for (int q = 0; q < childItemCount; q++)
				{
					ifail = ITEM_ask_rev_type2(childItemTags[q], &childrevitem_type);
					if (ifail != ITK_ok)
					{
						iFailureCount++;
						fprintf(errorLogPtr, "%s", cTempbuffer);
						fprintf(errorLogPtr, " ----->  ");
						fprintf(errorLogPtr, "childrevitem_type Not Found \n");
						fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, "NULL", RELATION, "NULL", "Failed");
						fflush(finalLogPtr);
						fflush(errorLogPtr);
						return ifail;
					}
					if (tc_strcmp(childrevitem_type, CHILDITEMTYPE) == 0)
					{
						printf("\nChild Revision Type matched : %s", childrevitem_type);
						childrevtag = childItemTags[q];
						break;
					}

					SAFE_SM_FREE(childrevitem_type);
				}
				if (childrevtag != NULL)
				{
					ifail = POM_tag_to_uid(childrevtag, &cSecondaryObjectUID);
					if (ifail != ITK_ok)
					{
						iFailureCount++;
						fprintf(errorLogPtr, "%s", cTempbuffer);
						fprintf(errorLogPtr, " ----->  ");
						fprintf(errorLogPtr, "Secondary Object UID not found \n");
						fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULl", CHILDITEMID, "NULL", RELATION, "NULL", "Failed");
						fflush(finalLogPtr);
						fflush(errorLogPtr);
						return ifail;
					}
					ccAttrs[0] = "item_id";
					ccValues[0] = (char*)PARENTITEMID;
					ifail = ITEM_find_item_revs_by_key_attributes(1, ccAttrs, ccValues, PARENTITEMREVID, &iParentItemCount, &parentItemsTags);
					if (ifail != ITK_ok)
					{
						iFailureCount++;
						fprintf(errorLogPtr, "%s", cTempbuffer);
						fprintf(errorLogPtr, " ----->  ");
						fprintf(errorLogPtr, "Error in fetching parentItemsTags \n");
						fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
						fflush(finalLogPtr);
						fflush(errorLogPtr);
						return iRetcode;
					}
					printf("\nParent Item Count : %d", iParentItemCount);

					if (iParentItemCount > 0)
					{
						for (int p = 0; p < iParentItemCount; p++)
						{
							char* ObjectTypeParent = NULL;
							ifail = AOM_ask_value_string(parentItemsTags[p], "object_type", &ObjectTypeParent);
							if (ifail != ITK_ok)
							{
								iFailureCount++;
								fprintf(errorLogPtr, "%s", cTempbuffer);
								fprintf(errorLogPtr, " ----->  ");
								fprintf(errorLogPtr, "Error in fetching ObjectTypeParent \n");
								fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
								fflush(finalLogPtr);
								fflush(errorLogPtr);
								return ifail;
							}

							if (tc_strcmp(ObjectTypeParent, PARENTITEMTYPE) == 0)
							{
								printf("Parent Item Type matched :%s \n", ObjectTypeParent);
								parentrevtag = parentItemsTags[p];
								break;
							}
						}

						if (parentrevtag != NULLTAG)
						{
							ifail = AOM_ask_value_tags(parentrevtag, "IMAN_specification", &iDatasetCount, &Dtags);
							if (ifail != ITK_ok)
							{
								iFailureCount++;
								fprintf(errorLogPtr, "%s", cTempbuffer);
								fprintf(errorLogPtr, " ----->  ");
								fprintf(errorLogPtr, "Error in fetching Dtags \n");
								fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
								fflush(finalLogPtr);
								fflush(errorLogPtr);
								return ifail;
							}
							printf("\n parent Dataset Count : %d", iDatasetCount);

							if (iDatasetCount > 0)
							{
								for (int m = 0; m < iDatasetCount; m++)
								{
									char* dsetName = NULL;
									char* DtypeValue = NULL;

									ifail = AOM_ask_value_string(Dtags[m], "object_type", &DtypeValue);
									if (ifail != ITK_ok)
									{
										iFailureCount++;
										fprintf(errorLogPtr, "%s", cTempbuffer);
										fprintf(errorLogPtr, " ----->  ");
										fprintf(errorLogPtr, "Error in fetching DtypeValue \n");
										fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
										fflush(finalLogPtr);
										fflush(errorLogPtr);
										return ifail;
									}


									if (tc_strcmp(DtypeValue, PARENT_DATASETTYPE) == 0)
									{
										printf("\nDataset type matched : %s", DtypeValue);
										ifail = AOM_ask_value_string(Dtags[m], "object_name", &dsetName);
										if (ifail != ITK_ok)
										{
											iFailureCount++;
											fprintf(errorLogPtr, "%s", cTempbuffer);
											fprintf(errorLogPtr, " ----->  ");
											fprintf(errorLogPtr, "Error in fetching dsetName \n");
											fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
											fflush(finalLogPtr);
											fflush(errorLogPtr);
											return ifail;
										}
										if (tc_strcmp(dsetName, PARENT_DATASETNAME) == 0)
										{
											printf("\nSWDrw Dataset name matched :%s ", dsetName);
											datasettag = Dtags[m];
											break;
										}
									}
								}

								if (datasettag != NULLTAG)
								{
									char* cPrimaryDatasetUID = NULL;

									ifail = POM_tag_to_uid(datasettag, &cPrimaryDatasetUID);
									if (ifail != ITK_ok)
									{
										iFailureCount++;
										fprintf(errorLogPtr, "%s", cTempbuffer);
										fprintf(errorLogPtr, " ----->  ");
										fprintf(errorLogPtr, "Relation Type Name not found ID Not found \n");
										fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
										fflush(finalLogPtr);
										fflush(errorLogPtr);
										return ifail;
									}
									if (childrevtag != NULLTAG)
									{
										//find datasettag2 based on childrevtag
										tag_t datasettag2 = NULL;
										tag_t* Dctags = NULL;
										ifail = AOM_ask_value_tags(childrevtag, "IMAN_specification", &iDatasetCount, &Dctags);
										if (ifail != ITK_ok)
										{
											iFailureCount++;
											fprintf(errorLogPtr, "%s", cTempbuffer);
											fprintf(errorLogPtr, " ----->  ");
											fprintf(errorLogPtr, "Error in fetching Dctags \n");
											fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
											fflush(finalLogPtr);
											fflush(errorLogPtr);
											return ifail;
										}
										printf("\n parent Dataset Count : %d", iDatasetCount);

										if (iDatasetCount > 0)
										{
											for (int m = 0; m < iDatasetCount; m++)
											{
												char* dsetName = NULL;
												char* DtypeValue = NULL;

												ifail = AOM_ask_value_string(Dctags[m], "object_type", &DtypeValue);
												if (ifail != ITK_ok)
												{
													iFailureCount++;
													fprintf(errorLogPtr, "%s", cTempbuffer);
													fprintf(errorLogPtr, " ----->  ");
													fprintf(errorLogPtr, "Error in fetching DtypeValue \n");
													fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
													fflush(finalLogPtr);
													fflush(errorLogPtr);
													return ifail;
												}


												if (tc_strcmp(DtypeValue, PARENT_DATASETTYPE) == 0)
												{
													printf("\nDataset type matched : %s", DtypeValue);
													ifail = AOM_ask_value_string(Dtags[m], "object_name", &dsetName);
													if (ifail != ITK_ok)
													{
														iFailureCount++;
														fprintf(errorLogPtr, "%s", cTempbuffer);
														fprintf(errorLogPtr, " ----->  ");
														fprintf(errorLogPtr, "Error in fetching dsetName \n");
														fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
														fflush(finalLogPtr);
														fflush(errorLogPtr);
														return ifail;
													}
													if (tc_strcmp(dsetName, CHILD_DATASETNAME) == 0)
													{
														printf("\nSWDrw Dataset name matched :%s ", dsetName);
														datasettag2 = Dctags[m];
														break;
													}
												}
											}


											GRM_find_relation_type(RELATION, &relation_type);
											if (relation_type != NULLTAG)
											{
												ifail = AOM_ask_value_string(relation_type, "object_string", &relationTypeName);
												if (ifail != ITK_ok)
												{
													iFailureCount++;
													fprintf(errorLogPtr, "%s", cTempbuffer);
													fprintf(errorLogPtr, " ----->  ");
													fprintf(errorLogPtr, "Relation Type Name not found ID Not found \n");
													fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, cPrimaryDatasetUID, CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
													fflush(finalLogPtr);
													fflush(errorLogPtr);
													return ifail;
												}
												//code to find relation between datasettag1 & datasettag2
												ifail = GRM_create_relation(datasettag, datasettag2, relation_type, NULLTAG, &tRelationTag);
												if (ifail != ITK_ok)
												{
													iFailureCount++;
													fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, cPrimaryDatasetUID, CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
													MEM_free(message);
												}

											}
											else
											{
												iFailureCount++;
												fprintf(errorLogPtr, "%s", cTempbuffer);
												fprintf(errorLogPtr, " ----->  ");
												fprintf(errorLogPtr, "%s ", RELATION);
												fprintf(errorLogPtr, "Relation Type is not valid \n");
												fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, cPrimaryDatasetUID, CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
												fflush(finalLogPtr);
												fflush(errorLogPtr);
											}
										}
										else
										{
											iFailureCount++;
											fprintf(errorLogPtr, "%s", cTempbuffer);
											fprintf(errorLogPtr, " ----->  ");
											fprintf(errorLogPtr, "Child ItemRevision not found\n");
											fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, cPrimaryDatasetUID, CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
											fflush(finalLogPtr);
											fflush(errorLogPtr);
										}

										SAFE_SM_FREE(childItemTags);
										SAFE_SM_FREE(cPrimaryDatasetUID);
									}
									else
									{
										iFailureCount++;
										fprintf(errorLogPtr, "%s", cTempbuffer);
										fprintf(errorLogPtr, " ----->  ");
										fprintf(errorLogPtr, "Parent Dataset not found\n");
										fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
										fflush(finalLogPtr);
										fflush(errorLogPtr);
									}
								}
								else
								{
									iFailureCount++;
									fprintf(errorLogPtr, "%s", cTempbuffer);
									fprintf(errorLogPtr, " ----->  ");
									fprintf(errorLogPtr, "Parent Dataset not found\n");
									fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
									fflush(finalLogPtr);
									fflush(errorLogPtr);
								}

								SAFE_SM_FREE(Dtags);
							}
							else
							{
								iFailureCount++;
								fprintf(errorLogPtr, "%s", cTempbuffer);
								fprintf(errorLogPtr, " ----->  ");

								fprintf(errorLogPtr, "Parent Item not found \n");
								fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
								fflush(finalLogPtr);
								fflush(errorLogPtr);
							}
						}
						else
						{
							iFailureCount++;
							fprintf(errorLogPtr, "%s", cTempbuffer);
							fprintf(errorLogPtr, " ----->  ");
							fprintf(errorLogPtr, "Parent Item not found \n");
							fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
							fflush(finalLogPtr);
							fflush(errorLogPtr);
						}
					}
					else
					{
						iFailureCount++;
						fprintf(errorLogPtr, "%s", cTempbuffer);
						fprintf(errorLogPtr, " ----->  ");
						fprintf(errorLogPtr, "Parent Item not found \n");
						fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", PARENT_DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");
						fflush(finalLogPtr);
						fflush(errorLogPtr);
					}
					SAFE_SM_FREE(parentItemsTags);
					SAFE_SM_FREE(cSecondaryObjectUID);

					ITK_set_bypass(false);
				}
				else
				{
					printf("\nLogin to Teamcenter is UnSuccessful \n");
					return 1;
				}
			}
			iLineCount++;
		}

		//free memory
		for (int i = 0; i < COLUMN_COUNT; ++i) {
			MEM_free(ccHeadertokens[i]);
		}
		MEM_free(ccHeadertokens);
		MEM_free(ccAttrs);
		MEM_free(ccValues);

		fclose(errorLogPtr);
		fprintf(finalLogPtr, "\nError List:\n");

		err3 = fopen_s(&inputFile, errorOPLog, "r");
		if (err3 != 0) {
			printf("Unable to read the file\n");
		}
		else
		{
			while (fgets(buffer, sizeof(buffer), inputFile) != nullptr) {
				fprintf(finalLogPtr, "%s", buffer);
			}
		}

		// Delete Temporary Log file
		if (remove(errorOPLog) != 0) {
			printf("\nFailed to delete temporary file\n");
		}

		int iNoError = 0;
		const int* ipSeverities = NULL;
		const int* ipFails = NULL;
		const char** cppErrorTexts = NULL;
		iRetcode = EMH_ask_errors(&iNoError, &ipSeverities, &ipFails, &cppErrorTexts);
		for (int iErrIndx = 0; iErrIndx < iNoError; iErrIndx++) {
			fprintf(finalLogPtr, "Relation saving failed : Error %d: %s\n", ifail, (cppErrorTexts[iErrIndx] == NULL ? "null" : cppErrorTexts[iErrIndx]));
		}
		fprintf(finalLogPtr, "\nSuccess & Failure Count:\n");
		fprintf(finalLogPtr, "Success Count: %d \n", iSuccessCount);
		fprintf(finalLogPtr, "Failure Count: %d \n", iFailureCount);


		fclose(ipPtr);
		fclose(inputFile);
		fclose(finalLogPtr);
		fclose(errorLogPtr);

		t = clock() - t;
		time_taken = ((double)t) / CLOCKS_PER_SEC;
		printf("Total Time taken to attach the related objcts in inputfile is %f seconds \n", time_taken);
		iRetcode = ITK_exit_module(TRUE);
		return iRetcode;
	}
}

void honPrintUsage()
{
	printf("\nUsage:\n");
	printf("AttachItemtoItemRevisions -i=<inputfile> \n");
	printf("Accepted Input File Header: 'PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION\n");
	printf("\nArguments:\n");
	printf("-h: help on Usage <Optional>.\n");
	printf("-i: input file - mandatory \n");
	printf("-u: User ID     <Mandatory>  -User Id to log into Teamcenter.\n");
	printf("-p: Password    <Mandatory>  -Password to log into Teamcenter.\n");
	printf("-g: Group       <Mandatory>  -Group to log into Teamcenter.\n");
	printf("\n");
}