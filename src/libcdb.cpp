#include <iostream>
#include <libcdb/libcdb.hpp>
#include <string_view>
#include <sys/ptrace.h>

pid_t cdb::attach(int argc, char **argv) {
  if (argc == 1) {
    std::cerr << "Missing program path or PID number! \n";
    return -1;
  }
  pid_t pid = 0;
  if (std::string_view(argv[1]).compare("-p") == 0) {
    if (argc != 3) {
      std::cerr << "Missing PID number!\n";
      return -1;
    }
    pid = std::stoi(argv[2]);
    if (ptrace(PTRACE_ATTACH, pid) < 0) {
      perror("Error to attach process to pid");
      return -1;
    }
  } else {
    if ((pid = fork()) < 0) {
      perror("Fork error");
      return -1;
    }
    if (pid == 0) {
      std::cout << "Child running\n";
      if (ptrace(PTRACE_TRACEME) < 0) {
        perror("PTRACE_TRACEME request error");
        exit(EXIT_FAILURE);
      }
      auto proc_path = argv[1];
      execlp(proc_path, proc_path, NULL);
      perror("execlp error");
      exit(EXIT_FAILURE);
    }
    if (ptrace(PTRACE_ATTACH, pid) < 0) {
      perror("PTRACE_ATTACH request error");
      return -1;
    }
  }
  return pid;
}
