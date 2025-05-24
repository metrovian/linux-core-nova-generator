#include "thread_gateway.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal) {
	thread_gateway_stop();
	DBG_WARN("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[]) {
	signal(SIGINT, HANDLE_SIGINT);
	thread_gateway_set_rule(GATEWAY_LEASTNETWORK);
	thread_gateway_start();
	return 0;
}
