#include <tcinit/tcinit.h>
#include <tc/tc_startup.h>
#include <tccore/tctype.h>
#include <cm/cm.h>
#include <tccore/aom_prop.h>
#include<tccore/aom.h>
#include<tccore/item.h>
#include<pom/pom/pom.h>

#include <user_exits/epm_toolkit_utils.h>

int iHandlingError(FILE* file, int iFailOrigin)
{
	char* cErrorText = NULL;
	int iFail = ITK_ok;
	iFail = EMH_ask_error_text(iFailOrigin, &cErrorText);
	if (iFail != ITK_ok)
	{
		TC_write_syslog("Error API is not defined properly\n");
		SAFE_SM_FREE(cErrorText);
		return iFail;
	}
	fprintf(file, cErrorText);
	return iFail;
}