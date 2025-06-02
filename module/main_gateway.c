#include "thread_manager.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal) {
	thread_manager_stop();
	log_warn("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[]) {
	signal(SIGINT, HANDLE_SIGINT);
	thread_manager_set_rule(MANAGER_LEASTNETWORK);
	thread_manager_start();
	return 0;
}
