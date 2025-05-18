#include "thread_gateway.h"
#include "predefined.h"

static int8_t main_run = 1;

void HANDLE_SIGINT(int32_t signal)
{
        main_run = 0;

        DBG_WARN("SIGINT");
        return;
}

int32_t main(int32_t argc, char *argv[])
{
	signal(SIGINT, HANDLE_SIGINT);

	thread_gateway_start();
	thread_gateway_set_rule(GATEWAY_LEASTNETWORK);

	while (main_run);
	
	thread_gateway_stop();
	
	return 0;
}

