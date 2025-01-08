#include <cstdio>
#include <editline/readline.h>
#include <iostream>
#include <libcdb/libcdb.hpp>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {} // namespace

auto main(int argc, char **argv) -> int {
  if (argc == 1) {
    std::cerr << "No arguments given\n";
    return -1;
  }
  pid_t pid = cdb::attach(argc, argv);
  std::cout << "Attached process pid: " << pid << std::endl;

  int wait_status = 0;
  int options = 0;
  do {
    pid = waitpid(pid, &wait_status, options);
    if (pid == -1) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }

    if (WIFEXITED(wait_status)) {
      printf("exited, status=%d\n", WEXITSTATUS(wait_status));
    } else if (WIFSIGNALED(wait_status)) {
      printf("killed by signal %d\n", WTERMSIG(wait_status));
    } else if (WIFSTOPPED(wait_status)) {
      printf("stopped by signal %d\n", WSTOPSIG(wait_status));
    } else if (WIFCONTINUED(wait_status)) {
      printf("continued\n");
    }
  } while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
  return 0;
}
