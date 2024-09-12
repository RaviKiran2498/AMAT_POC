/*********************************************************************************************************
  * This Program is used to Attach the Solution Item Revision to HON ECN
	Input File Header: CN_NO,CN_REV,REVID,DOC_REV,RELATION
  ********************************************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
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
#include <ae/dataset.h>
#include <tccore/grm.h>
#include <user_exits/epm_toolkit_utils.h>
#include <tc/tc_startup.h>
#include <tcinit/tcinit.h>
#include <Error_Exception.hxx>
#include <base_utils/IFail.hxx>
#include <base_utils/TcResultStatus.hxx>
#include <tccore\libtccore_exports.h>

#define BUFFER_SIZE 1000

#define COLUMN_COUNT 9
using namespace std;
void honPrintUsage();

void DatasetAttach(tag_t datasettag, string strRelation, tag_t childrevtag,
    int* iSuccessCountVd, int* iFailureCountVd, char** cPrimaryDatasetUID,
    string strMode, tag_t tIMANSpecRelTpe, string strDsetParentName,
    char* cpSecObjName, char* cSecondaryObjectUID, FILE* errorLogPtr,
    FILE* finalLogPtr, string strFullLine, string strChildID, string strChildRevID, 
    string strChildType);

int ITK_user_main(int argc, char* argv[])
{
    TC_write_syslog("\nEntering ITK_user_main Function...");
    int ifail = ITK_ok;
    ResultStatus resultStat; // For Error Handling

    const char* fileName = ITK_ask_cli_argument("-i=");

    const char* sUser = ITK_ask_cli_argument("-u=");

    const char* sPassword = ITK_ask_cli_argument("-p=");

    const char* sGroup = ITK_ask_cli_argument("-g=");

    const char* finalOPLog = ITK_ask_cli_argument("-o=");

    const char* mode = ITK_ask_cli_argument("-m=");

    FILE* ipPtr = NULL;

    FILE* finalLogPtr = NULL;

    FILE* errorLogPtr = NULL;

    char* token = NULL;

    char* headertoken = NULL;

    char* cEachlinetoken = NULL;

    char* PARENTITEMID = NULL;

    char* PARENTITEMREVID = NULL;

    char* CHILDITEMID = NULL;

    char* CHILDITEMREVID = NULL;

    char* CHILDITEMTYPE = NULL;

    char* PARENTITEMTYPE = NULL;

    char* DATASETNAME = NULL;

    char* DATASETTYPE = NULL;

    char* RELATION = NULL;

    int iSuccessCount = 0;

    int iFailureCount = 0;

    size_t fileNamelength = 0;

    int iLineCount = 0;

    int iHeadercount = 0;

    int iEachlinecount = 0;

    int iIndex = 0;


    char buffer[BUFFER_SIZE];

    char cTempbuffer[BUFFER_SIZE];

    string strFinalOPLog = "";

    strFinalOPLog.assign(finalOPLog).append("_Failure.log");

    const char* errorOPLog = strFinalOPLog.c_str();

    char** ccHeadertokens = (char**)MEM_alloc(COLUMN_COUNT * sizeof(char*));



    logical isGRMSflag = false;

    size_t grmswordlength = 0;

    logical isValidHeader = false;


    char grmssearchword[5] = "grms";  //null char included 4 + 1;

    int iGrmsfound = 0;

    const char* ccAttrs[2] = { "item_id", "object_type" };
    const char* ccValues[2] = {};

    double time_taken;

    clock_t t;

    t = clock();



    try

    {

        if (sUser == NULL
            || sPassword == NULL
            || sGroup == NULL
            || fileName == NULL
            || mode == NULL)

        {
            honPrintUsage();
            return 1;
        }



        if (tc_strcmp(argv[0], "-h") == 0)
        {
            honPrintUsage();

            exit(0);

        }

        errno_t errIpPtr = fopen_s(&ipPtr, fileName, "r");



        if (errIpPtr != 0)

        {

            printf("Error! opening file: %s", fileName);

            TC_write_syslog("\nError - Reading input file: %s\n", fileName);

            exit(1);

        }



        errno_t errFinalLog = fopen_s(&finalLogPtr, finalOPLog, "w");  //change file path

        errno_t errLog = fopen_s(&errorLogPtr, errorOPLog, "w");  //change file path



        if (errLog != 0)

        {

            printf("Error! opening file: %s", errorOPLog);

            TC_write_syslog("\nError - opening file: %s\n", errorOPLog);

            exit(1);

        }

        if (errFinalLog != 0)

        {

            printf("Error! opening file: %s", finalOPLog);

            TC_write_syslog("\nError - opening file: %s\n", finalOPLog);

            exit(1);

        }



        fprintf(finalLogPtr, "Object List:\n");

        fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n",

            "Primary Object", "Primary Object pUID", "Secondary Object",

            "Secondary Object pUID", "Relation Type Name", "Saved Relation PUID",

            "Status");

        printf("\nFiles created successfully: \"%s\" and \"%s\".", errorOPLog, finalOPLog);

        TC_write_syslog("\nLog files Created Successfully: \"%s\" and \"%s\".\n", errorOPLog, finalOPLog);



        fileNamelength = tc_strlen(fileName);

        grmswordlength = tc_strlen(grmssearchword);



        TC_write_syslog("\nBefore Logging into Teamcenter...\n");



        if (ITK_init_module(sUser, sPassword, sGroup) == ITK_ok)

        {

            TC_write_syslog("\nLogged into Teamcenter successfully...\n");



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

                    isGRMSflag = true;

                }

            }

            iLineCount = 1;

            ITK_set_bypass(true);
            TC_write_syslog("\nSetting ITK bypass true for DBA users...\n");
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

                tc_strcpy(cTempbuffer, buffer);

                printf("\nRunning line no. : %d", iLineCount);

                TC_write_syslog("\nLine No.: %d\n%s\n", iLineCount, buffer);

                if (iLineCount < 2)
                {
                    iHeadercount = 0;
                    headertoken = tc_strtok(buffer, "|");

                    char* eachHeaderToken = NULL;

                    //--------Change Code-----------

                    while (headertoken != NULL)
                    {

                        eachHeaderToken = (char*)MEM_alloc((tc_strlen(headertoken) + 1) * sizeof(char));

                        if (eachHeaderToken == NULL) {

                            printf("NULLPTR Each header token\n");

                        }

                        tc_strcpy(eachHeaderToken, headertoken);
                        ccHeadertokens[iHeadercount] = eachHeaderToken;

                        iHeadercount++;

                        headertoken = tc_strtok(NULL, "|");
                    }
                    //-----------Till Here----------
                    if (tc_strcmp(cTempbuffer, "PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|DATASETNAME|DATASETTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION") == 0)

                    {

                        isValidHeader = true;

                        printf("\nValid Header...\n");

                        printf("Processing the file...\n");

                        TC_write_syslog("\nValid Header: %s\n", cTempbuffer);

                    }

                    else

                    {

                        isValidHeader = false;

                        TC_write_syslog("\nPlease select the input file with correct header PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|DATASETNAME|DATASETTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION\n");

                        printf("\nPlease select the input file with correct header PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|DATASETNAME|DATASETTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION\n");

                        exit(0);

                    }

                }

                else if (iLineCount > 1)

                {

                    if (iHeadercount == COLUMN_COUNT && isGRMSflag && isValidHeader)

                    {

                        tc_strcpy(cTempbuffer, buffer);

                        iEachlinecount = 0;

                        cEachlinetoken = tc_strtok(buffer, "|");



                        while (cEachlinetoken != NULL)

                        {

                            if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENTITEMID") == 0)

                            {

                                int iParentID = 0;

                                iParentID = (int)tc_strlen(cEachlinetoken) + 1;

                                PARENTITEMID = (char*)MEM_alloc(sizeof(char) * iParentID);

                                tc_strcpy(PARENTITEMID, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENTITEMREVID") == 0)

                            {

                                int iParentRevID = 0;

                                iParentRevID = (int)tc_strlen(cEachlinetoken) + 1;

                                PARENTITEMREVID = (char*)MEM_alloc(sizeof(char) * iParentRevID);

                                tc_strcpy(PARENTITEMREVID, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "PARENTITEMTYPE") == 0)

                            {

                                int iParentObjType = 0;

                                iParentObjType = (int)tc_strlen(cEachlinetoken) + 1;

                                PARENTITEMTYPE = (char*)MEM_alloc(sizeof(char) * iParentObjType);

                                tc_strcpy(PARENTITEMTYPE, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "DATASETNAME") == 0)

                            {

                                int iDset = 0;

                                iDset = (int)tc_strlen(cEachlinetoken) + 1;

                                DATASETNAME = (char*)MEM_alloc(sizeof(char) * iDset);

                                tc_strcpy(DATASETNAME, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "DATASETTYPE") == 0)

                            {

                                int iDsetType = 0;

                                iDsetType = (int)tc_strlen(cEachlinetoken) + 1;

                                DATASETTYPE = (char*)MEM_alloc(sizeof(char) * iDsetType);

                                tc_strcpy(DATASETTYPE, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILDITEMID") == 0)

                            {

                                int iChildID = 0;

                                iChildID = (int)tc_strlen(cEachlinetoken) + 1;

                                CHILDITEMID = (char*)MEM_alloc(sizeof(char) * iChildID);

                                tc_strcpy(CHILDITEMID, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILDITEMREVID") == 0)

                            {

                                int iChildRevID = 0;

                                iChildRevID = (int)tc_strlen(cEachlinetoken) + 1;

                                CHILDITEMREVID = (char*)MEM_alloc(sizeof(char) * iChildRevID);

                                tc_strcpy(CHILDITEMREVID, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "CHILDITEMTYPE") == 0)

                            {

                                int iChildObjTpe = 0;

                                iChildObjTpe = (int)tc_strlen(cEachlinetoken) + 1;

                                CHILDITEMTYPE = (char*)MEM_alloc(sizeof(char) * iChildObjTpe);

                                tc_strcpy(CHILDITEMTYPE, cEachlinetoken);

                            }

                            else if (tc_strcmp(ccHeadertokens[iEachlinecount], "RELATION") == 0)

                            {

                                int iRel = 0;

                                iRel = (int)tc_strlen(cEachlinetoken) + 1;

                                RELATION = (char*)MEM_alloc(sizeof(char) * iRel);

                                tc_strcpy(RELATION, cEachlinetoken);

                            }

                            iEachlinecount++;

                            cEachlinetoken = tc_strtok(NULL, "|");

                        }

                    }

                    else

                    {

                        printf("Please check header values and filename\n");

                        TC_write_syslog("\nPlease check header values and filename\n");

                        ITK_set_bypass(false);

                        TC_write_syslog("\nSetting ITK bypass false...\n");

                        TC_write_syslog("\nLogging out of Teamcenter session...\n");

                        ITK_exit_module(TRUE);

                        return 1;

                    }



                    char* objectPUID = NULL;

                    char* relationTypeName = NULL;

                    tag_t* parentItemsTags = NULL;

                    tag_t* Dtags = NULL;

                    tag_t tRelationTag = NULLTAG;

                    tag_t relation_type = NULLTAG;

                    tag_t parentrevtag = NULLTAG;

                    tag_t datasettag = NULLTAG;

                    tag_t relationtag = NULLTAG;

                    tag_t tIMANSpecTypeTag = NULLTAG;

                    resultStat = GRM_find_relation_type("IMAN_specification", &tIMANSpecTypeTag);

                    int iParentItemCount = 0;

                    int iDatasetCount = 0;



                    printf("\nExecuting for data: %s,%s,%s,%s,%s,%s,%s,%s,%s", PARENTITEMID, PARENTITEMREVID, PARENTITEMTYPE,

                        DATASETNAME, DATASETTYPE, CHILDITEMID, CHILDITEMREVID, CHILDITEMTYPE, RELATION);

                    //Extra Test code

                    char* cSecondaryObjectUID = NULL;

                    //extra end



                    tag_t childrevtag = NULLTAG;

                    tag_t* childItemTags = NULL;

                    int childItemCount = 0;

                    char* childrevitem_type = NULL;



                    ccValues[0] = MEM_string_copy(CHILDITEMID);

                    ccValues[1] = MEM_string_copy(CHILDITEMTYPE);



                    resultStat = ITEM_find_item_revs_by_key_attributes(2, ccAttrs, ccValues, CHILDITEMREVID, &childItemCount, &childItemTags);

                    printf("\nChild Rev count: %d", childItemCount);

                    TC_write_syslog("\nQRY Child Rev Count: %d", childItemCount);



                    if (childItemCount == 1)

                    {

                        TC_write_syslog("\nChild object present in TC with this combination given. ID: %s/%s and Object type: %s.\n", CHILDITEMID, CHILDITEMREVID, CHILDITEMTYPE);

                        childrevtag = childItemTags[0];

                    }

                    else if (childItemCount > 1)

                    {

                        TC_write_syslog("\nERROR: More than one Child object present in TC with the combination given. Please check your ID: %s/%s and Object type: %s.", CHILDITEMID, CHILDITEMREVID, CHILDITEMTYPE);

                        printf("\nERROR - More than one Child object present in TC with the combination given. Please check your ID: %s/%s and Object type: %s.", CHILDITEMID, CHILDITEMREVID, CHILDITEMTYPE);

                    }

                    SAFE_SM_FREE(childItemTags);



                    if (childrevtag != NULLTAG)

                    {

                        char* cpSecObjName = NULL;

                        resultStat = WSOM_ask_name2(childrevtag, &cpSecObjName);

                        resultStat = POM_tag_to_uid(childrevtag, &cSecondaryObjectUID);



                        ccValues[0] = MEM_string_copy(PARENTITEMID);

                        ccValues[1] = MEM_string_copy(PARENTITEMTYPE);



                        resultStat = ITEM_find_item_revs_by_key_attributes(2, ccAttrs, ccValues, PARENTITEMREVID, &iParentItemCount, &parentItemsTags);

                        TC_write_syslog("\nParent Item Rev Count: %d\n", iParentItemCount);

                        printf("\nParent Item Rev Count: %d\n", iParentItemCount);



                        if (iParentItemCount > 1)

                        {

                            TC_write_syslog("\nERROR: More than one Parent object present in TC with the combination given. Please check your ID: %s/%s and Object type: %s.\n", PARENTITEMID, PARENTITEMREVID, PARENTITEMTYPE);

                            printf("\nERROR: More than one Parent object present in TC with the combination given. Please check your ID: %s/%s and Object type: %s.\n", PARENTITEMID, PARENTITEMREVID, PARENTITEMTYPE);

                        }

                        else if (iParentItemCount == 1)

                        {

                            TC_write_syslog("\nParent object present in TC with this combination given. ID: %s/%s and Object type: %s.\n", PARENTITEMID, PARENTITEMREVID, PARENTITEMTYPE);

                            parentrevtag = parentItemsTags[0];

                            if (parentrevtag != NULLTAG)

                            {

                                resultStat = GRM_list_secondary_objects_only(parentrevtag, tIMANSpecTypeTag, &iDatasetCount, &Dtags);

                                //resultStat = AOM_ask_value_tags(parentrevtag, "IMAN_specification", &iDatasetCount, &Dtags);

                                TC_write_syslog("\nDataset Count: %d", iDatasetCount);



                                if (iDatasetCount > 0)

                                {

                                    for (int m = 0; m < iDatasetCount; m++)

                                    {

                                        char* dsetName = NULL;

                                        char* DtypeValue = NULL;



                                        resultStat = WSOM_ask_object_type2(Dtags[m], &DtypeValue);

                                        //resultStat = AOM_ask_value_string(Dtags[m], "object_type", &DtypeValue);



                                        if (tc_strcmp(DtypeValue, DATASETTYPE) == 0)

                                        {

                                            TC_write_syslog("\nDataset type matched : %s\n", DtypeValue);

                                            resultStat = AOM_ask_value_string(Dtags[m], "object_name", &dsetName);

                                            if (tc_strcmp(dsetName, DATASETNAME) == 0)

                                            {

                                                TC_write_syslog("\nSWDrw Dataset name matched: %s\n", dsetName);

                                                datasettag = Dtags[m];

                                                break;

                                            }

                                        }

                                    }



                                    if (datasettag != NULLTAG)

                                    {

                                        char* cPrimaryDatasetUID = NULL;

                                        if (childrevtag != NULLTAG)

                                        {

                                            int iSuccessCountVd = 0;

                                            int iFailureCountVd = 0;

                                            string strmode = "";

                                            strmode.assign(mode);

                                            string strPrntDsetName = "";

                                            strPrntDsetName.assign(DATASETNAME);

                                            string strLine = "";

                                            strLine.assign(cTempbuffer);

                                            string strChildID = "";

                                            string strChildRevID = "";

                                            string strChildType = "";

                                            strChildID.assign(CHILDITEMID);

                                            strChildRevID.assign(CHILDITEMREVID);

                                            strChildType.assign(CHILDITEMTYPE);



                                            if (tc_strcmp(mode, "d2r") == 0)

                                            {

                                                DatasetAttach(datasettag, RELATION, childrevtag, &iSuccessCountVd, &iFailureCountVd,

                                                    &cPrimaryDatasetUID, strmode, tIMANSpecTypeTag, strPrntDsetName,

                                                    cpSecObjName, cSecondaryObjectUID, errorLogPtr, finalLogPtr, strLine,

                                                    strChildID, strChildRevID, strChildType);

                                            }

                                            else if (tc_strcmp(mode, "d2d") == 0)

                                            {

                                                DatasetAttach(datasettag, RELATION, childrevtag, &iSuccessCountVd,

                                                    &iFailureCountVd, &cPrimaryDatasetUID, strmode, tIMANSpecTypeTag,

                                                    strPrntDsetName, cpSecObjName, cSecondaryObjectUID, errorLogPtr,

                                                    finalLogPtr, strLine, strChildID, strChildRevID, strChildType);

                                            }



                                            iFailureCount += iFailureCountVd;

                                            iSuccessCount += iSuccessCountVd;

                                        }

                                        else

                                        {

                                            iFailureCount++;

                                            fprintf(errorLogPtr, "%s", cTempbuffer);

                                            fprintf(errorLogPtr, " ----->  ");

                                            fprintf(errorLogPtr, "Child Item Revision not found\n");

                                            fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", DATASETNAME, cPrimaryDatasetUID, cpSecObjName, cSecondaryObjectUID, RELATION, "NULL", "Failed");

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

                                        fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", DATASETNAME, "NULL", cpSecObjName, cSecondaryObjectUID, RELATION, "NULL", "Failed");

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

                                    fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", DATASETNAME, "NULL", cpSecObjName, cSecondaryObjectUID, RELATION, "NULL", "Failed");

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

                                fprintf(errorLogPtr, "Parent Item Revision not found\n");

                                fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", DATASETNAME, "NULL", cpSecObjName, cSecondaryObjectUID, RELATION, "NULL", "Failed");

                                fflush(finalLogPtr);

                                fflush(errorLogPtr);

                            }

                            SAFE_SM_FREE(cpSecObjName);

                        }

                        else

                        {

                            TC_write_syslog("\nERROR: Parent object not present in TC with the combination given. Please check your ID: %s/%s and Object type: %s.", PARENTITEMID, PARENTITEMREVID, PARENTITEMTYPE);

                            iFailureCount++;

                            fprintf(errorLogPtr, "%s", cTempbuffer);

                            fprintf(errorLogPtr, " ----->  ");

                            fprintf(errorLogPtr, "Parent Item Revision not found\n");

                            fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", DATASETNAME, "NULL", cpSecObjName, cSecondaryObjectUID, RELATION, "NULL", "Failed");

                            fflush(finalLogPtr);

                            fflush(errorLogPtr);

                        }

                        SAFE_SM_FREE(cpSecObjName);

                    }

                    else

                    {

                        TC_write_syslog("\nERROR: Child object not present in TC with the combination given. Please check your ID: %s/%s and Object type: %s.\n", CHILDITEMID, CHILDITEMREVID, CHILDITEMTYPE);

                        iFailureCount++;

                        fprintf(errorLogPtr, "%s", cTempbuffer);

                        fprintf(errorLogPtr, " ----->  ");

                        fprintf(errorLogPtr, "Child Item Revision not found\n");

                        fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", DATASETNAME, "NULL", CHILDITEMID, cSecondaryObjectUID, RELATION, "NULL", "Failed");

                        fflush(finalLogPtr);

                        fflush(errorLogPtr);

                    }

                    SAFE_SM_FREE(parentItemsTags);

                    SAFE_SM_FREE(cSecondaryObjectUID);

                }

                iLineCount++;

            }

            ITK_set_bypass(false);

            TC_write_syslog("\nFull file read, setting ITK bypass false...\n");



            //free memory

            TC_write_syslog("\nBefore Freeing memory...\n");

            for (int i = 0; i < COLUMN_COUNT; ++i) {

                MEM_free(ccHeadertokens[i]);

            }

            TC_write_syslog("\nAfter Freed memory...\n");

            if (ccHeadertokens)

                MEM_free(ccHeadertokens);

            TC_write_syslog("\nFreed memory: ccHeadertokens...\n");



            fclose(errorLogPtr);

            TC_write_syslog("\nClosing the Log File pointer: %s\n", errorOPLog);

            fprintf(finalLogPtr, "\nError List:\n");



            FILE* inputFile;

            errno_t errIpFileRead = fopen_s(&inputFile, errorOPLog, "r");

            if (errIpFileRead != 0)

            {

                TC_write_syslog("\nUnable to read the file: %s\n", errorOPLog);

            }

            else

            {

                while (fgets(buffer, sizeof(buffer), inputFile) != nullptr) {

                    fprintf(finalLogPtr, "%s", buffer);

                }

            }



            int iNoError = 0;

            const int* ipSeverities = NULL;

            const int* ipFails = NULL;

            const char** cppErrorTexts = NULL;

            ifail = EMH_ask_errors(&iNoError, &ipSeverities, &ipFails, &cppErrorTexts);



            for (int iErrIndx = 0; iErrIndx < iNoError; iErrIndx++)

            {

                fprintf(finalLogPtr, "Relation saving failed: Error %d: %s\n", ifail, (cppErrorTexts[iErrIndx] == NULL ? "null" : cppErrorTexts[iErrIndx]));

            }

            fprintf(finalLogPtr, "\nSuccess & Failure Count:\n");

            fprintf(finalLogPtr, "Success Count: %d \n", iSuccessCount);

            fprintf(finalLogPtr, "Failure Count: %d \n", iFailureCount);



            fclose(ipPtr);

            TC_write_syslog("\nClosing the Input File pointer: %s\n", fileName);



            fclose(inputFile);

            TC_write_syslog("\nClosing the Input File pointer: %s\n", errorOPLog);



            // Delete Temporary Log file

            if (remove(errorOPLog) != 0)

            {

                TC_write_syslog("\nFailed to delete temporary file: %s\n", errorOPLog);

            }



            fclose(finalLogPtr);

            TC_write_syslog("\nClosing the Log File pointer: %s\n", finalOPLog);



            t = clock() - t;

            time_taken = ((double)t) / CLOCKS_PER_SEC;

            printf("\nTotal Time taken to attach the related objects in inputfile is %f seconds\n", time_taken);

            TC_write_syslog("\nTotal Time taken to attach the related objects in inputfile is %f seconds.\n", time_taken);



            TC_write_syslog("\nFull file read, Logging out of Teamcenter session...\n");

            ITK_exit_module(TRUE);

            printf("\nFull file read, Logging out of Teamcenter session...\n");

        }

        else

        {

            printf("\nLogin to Teamcenter is UnSuccessful\n");

            TC_write_syslog("\nERROR - Login to Teamcenter is UnSuccessful\n");

            return 1;

        }

    }

    catch (const IFail& e)

    {

        int stat = e.ifail();

        EMH_clear_errors();

        EMH_store_error_s1(EMH_severity_error, stat, (e.getMessage()).c_str());

        TC_write_syslog("\nError - %s\n", (e.getMessage()).c_str());



        ITK_exit_module(TRUE);

        printf("\nCatch Block: Logging out of Teamcenter session...\n");

        TC_write_syslog("\nCatch Block: Logging out of Teamcenter session...\n");

    }

    printf("\nExiting Function...\n");

    TC_write_syslog("\nExiting Function...\n");

    return ifail;

}

void honPrintUsage()
{
    printf("\nUsage:\n");

    printf("AttachItemtoItemRevisions -i=<inputfile> \n");

    printf("Accepted Input File Header: 'PARENTITEMID|PARENTITEMREVID|PARENTITEMTYPE|DATASETNAME|DATASETTYPE|CHILDITEMID|CHILDITEMREVID|CHILDITEMTYPE|RELATION\n");

    printf("\nArguments:\n");

    printf("-h: help on Usage <Optional>.\n");

    printf("-i: input file   <Mandatory> \n");

    printf("-m: Mode         <Mandatory> -- For Dataset 2 Revision: d2r -- For Dataset 2 Dataset: d2d \n");

    printf("-o: Output Log file  <Mandatory> \n");

    printf("-u: User ID     <Mandatory>  -User Id to log into Teamcenter.\n");

    printf("-p: Password    <Mandatory>  -Password to log into Teamcenter.\n");

    printf("-g: Group       <Mandatory>  -Group to log into Teamcenter.\n");

    printf("\n");
}

void DatasetAttach(tag_t datasettag, string strRelation, tag_t childrevtag,
    int* iSuccessCountVd, int* iFailureCountVd, char** cPrimaryDatasetUID,
    string strMode, tag_t tIMANSpecRelTpe, string strDsetParentName,
    char* cpSecObjName, char* cSecondaryObjectUID, FILE* errorLogPtr,
    FILE* finalLogPtr, string strFullLine, string strChildID, string strChildRevID, string strChildType)
{

    TC_write_syslog("Entering DatasetAttach Function: %s", strMode.c_str());

    int iFailVd = ITK_ok;

    tag_t relation_type = NULLTAG;

    ResultStatus resultStatVd;

    resultStatVd = POM_tag_to_uid(datasettag, cPrimaryDatasetUID);

    resultStatVd = GRM_find_relation_type(strRelation.c_str(), &relation_type);

    tag_t tRel = NULLTAG;

    if (relation_type != NULLTAG)
    {
        if (tc_strcmp(strMode.c_str(), "d2r") == 0)
        {
            resultStatVd = GRM_find_relation(datasettag, childrevtag, relation_type, &tRel);

            if (tRel != NULLTAG)
            {
                TC_write_syslog("\nRelation already exists between dataset: %s and Child: %s/%s -- %s...\n", strDsetParentName.c_str(), strChildID.c_str(), strChildRevID.c_str(), strChildType.c_str());
                char* RelationPUID = NULL;

                resultStatVd = POM_tag_to_uid(tRel, &RelationPUID);

                iFailureCountVd++;
                
                fprintf(errorLogPtr, "%s", strFullLine.c_str());
                fprintf(errorLogPtr, " ----->  ");
                fprintf(errorLogPtr, "%s ", strRelation.c_str());
                fprintf(errorLogPtr, "Relation already exists\n");
                fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", strDsetParentName.c_str(), *cPrimaryDatasetUID, cpSecObjName, cSecondaryObjectUID, strRelation.c_str(), RelationPUID, "Failed");
                fflush(finalLogPtr);
                fflush(errorLogPtr);
                SAFE_SM_FREE(RelationPUID);
            }
            else
            {
                char* cpObjUID = NULL;

                resultStatVd = GRM_create_relation(datasettag, childrevtag, relation_type, NULLTAG, &tRel);
                resultStatVd = GRM_save_relation(tRel);
                resultStatVd = POM_tag_to_uid(tRel, &cpObjUID);
                TC_write_syslog("\nRelation created exists between dataset: %s and Child: %s/%s -- %s...\n",
                    strDsetParentName.c_str(), strChildID.c_str(), strChildRevID.c_str(), strChildType.c_str());
                iSuccessCountVd++;

                fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", strDsetParentName.c_str(), *cPrimaryDatasetUID, cpSecObjName, cSecondaryObjectUID, strRelation.c_str(), cpObjUID, "Success");

                SAFE_SM_FREE(cpObjUID);
            }
        }

        else if (tc_strcmp(strMode.c_str(), "d2d") == 0)
        {
            tag_t* childDsetTags = NULL;

            int iChildDsetCount = 0;

            iFailVd = GRM_list_secondary_objects_only(childrevtag, tIMANSpecRelTpe, &iChildDsetCount, &childDsetTags);

            if (iChildDsetCount > 0)
            {
                for (int iDsetIndx = 0; iDsetIndx < iChildDsetCount; iDsetIndx++)
                {
                    char* cpChildDsetName = NULL;
                    char* cpChildDsetUID = NULL;

                    resultStatVd = WSOM_ask_name2(childDsetTags[iDsetIndx], &cpChildDsetName);
                    resultStatVd = GRM_find_relation(datasettag, childDsetTags[iDsetIndx], relation_type, &tRel);
                    resultStatVd = POM_tag_to_uid(childDsetTags[iDsetIndx], &cpChildDsetUID);
                    if (tRel != NULLTAG)
                    {
                        TC_write_syslog("\nRelation already exists between Parent dataset: %s and Child dataset: %s...\n", strDsetParentName.c_str(), cpChildDsetName);

                        char* RelationPUID = NULL;
                        resultStatVd = POM_tag_to_uid(tRel, &RelationPUID);
                        iFailureCountVd++;

                        fprintf(errorLogPtr, "%s", strFullLine.c_str());
                        fprintf(errorLogPtr, " ----->  ");
                        fprintf(errorLogPtr, "%s ", strRelation.c_str());
                        fprintf(errorLogPtr, "Relation already exists\n");
                        fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", strDsetParentName.c_str(), *cPrimaryDatasetUID, cpChildDsetName, cpChildDsetUID, strRelation.c_str(), RelationPUID, "Failed");

                        fflush(finalLogPtr);
                        fflush(errorLogPtr);
                        SAFE_SM_FREE(RelationPUID);
                        SAFE_SM_FREE(cpChildDsetName);
                        SAFE_SM_FREE(cpChildDsetUID);
                    }

                    else
                    {
                        char* cpObjUID = NULL;

                        resultStatVd = GRM_create_relation(datasettag, childDsetTags[iDsetIndx], relation_type, NULLTAG, &tRel);
                        resultStatVd = GRM_save_relation(tRel);
                        resultStatVd = POM_tag_to_uid(tRel, &cpObjUID);
                        TC_write_syslog("\nRelation created exists between Parent dataset: %s and Child dataset: %s...\n", strDsetParentName.c_str(), cpChildDsetName);
                        iSuccessCountVd++;

                        fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", strDsetParentName.c_str(), *cPrimaryDatasetUID, cpChildDsetName, cpChildDsetUID, strRelation.c_str(), cpObjUID, "Success");

                        SAFE_SM_FREE(cpObjUID);
                    }
                }
            }

            else {
                TC_write_syslog("\nChild Dataset Count is Zero...");
            }

            SAFE_SM_FREE(childDsetTags);
        }
    }
    else
    {
        iFailureCountVd++;
        if (tc_strcmp(strMode.c_str(), "d2d") == 0) {
            fprintf(errorLogPtr, "%s", strFullLine.c_str());
            fprintf(errorLogPtr, " ----->  ");
            fprintf(errorLogPtr, "%s ", strRelation.c_str());
            fprintf(errorLogPtr, "Relation Type is not valid \n");
            fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", strDsetParentName.c_str(), *cPrimaryDatasetUID, cpSecObjName, cSecondaryObjectUID, strRelation.c_str(), "NULL", "Failed");
            fflush(finalLogPtr);
            fflush(errorLogPtr);
        }
        else if (tc_strcmp(strMode.c_str(), "d2r") == 0) {
            fprintf(errorLogPtr, "%s", strFullLine.c_str());
            fprintf(errorLogPtr, " ----->  ");
            fprintf(errorLogPtr, "%s ", strRelation.c_str());
            fprintf(errorLogPtr, "Relation Type is not valid \n");
            fprintf(finalLogPtr, "%-50s%-50s%-50s%-50s%-50s%-30s%-20s\n", strDsetParentName.c_str(), *cPrimaryDatasetUID, cpSecObjName, cSecondaryObjectUID, strRelation.c_str(), "NULL", "Failed");
            fflush(finalLogPtr);
            fflush(errorLogPtr);
        }
    }
    TC_write_syslog("Exiting DatasetAttach Function: %s", strMode.c_str());
}