#include<tc\tc_startup.h>
#include<tccore/item.h>
#include<tccore/aom_prop.h>
#include<tccore/grm.h>
#include<tccore/aom.h>
#include <User_Exits/epm_toolkit_utils.h>
#include <epm/epm.h>
#include <User_Exits/user_exits.h>
#include <property/nr.h>
#include <PROPERTY/nr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define lineLen 100



#undef ITKCALL // The macro function to check for the api errors
#define ITKCALL(argument,fplogfile) \
		do { \
			int retcode = argument; \
			if (retcode != ITK_ok) { \
				char* errorMessage; \
				TC_fprintf(fplogfile,"Function call: %s\n", #argument); \
				printf("Function call: %d\n", argument); \
				TC_fprintf(fplogfile,"Returns [%d]\n", retcode); \
				printf("Returns [%d]\n", retcode); \
				EMH_store_error(EMH_severity_error, retcode);\
				EMH_ask_error_text(retcode, &errorMessage); \
				TC_fprintf(fplogfile,"Teamcenter ERROR--------------------: [%s]\n", errorMessage); \
				printf("Teamcenter ERROR--------------------: [%s]\n", errorMessage); \
				TC_fprintf(fplogfile,"File: %s, Line-----------------------: %d\n\n", __FILE__, __LINE__); \
				printf("File: %s, Line-----------------------: %d\n\n", __FILE__, __LINE__); \
				if (errorMessage != NULL) MEM_free(errorMessage); \
			} \
		} while (0)
