#include "libcdb/error.hpp"
#include <cstdio>
#include <cstring>
#include <editline/readline.h>
#include <iostream>
#include <libcdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
std::unique_ptr<cdb::process> attach(int argc, const char **argv) {
  if (argc == 3 && argv[1] == std::string_view("-p")) {
    pid_t pid = std::atoi(argv[2]);
    return cdb::process::attach(pid);
  } else {
    const char *program_path = argv[1];
    return cdb::process::launch(program_path);
  }
}
// TODO: refactor using stringstream and getline functions
std::vector<std::string> split(std::string_view line, char delimiter) {
  std::vector<std::string> tokens;
  std::string str = "";
  for (int i = 0; i < line.length(); i++) {
    char c = line[i];
    if (c == ' ') {
      if (!str.empty()) {
        tokens.push_back(str);
        str.clear();
      }
    } else {
      str += c;
      if (i == line.length() - 1)
        tokens.push_back(str);
    }
  }
  return tokens;
}
bool is_prefix(std::string_view str, std::string_view of) {
  if (str.size() > of.size())
    return false;
  return std::equal(str.begin(), str.end(), of.begin());
}

void print_stop_reason(const cdb::process &proc, cdb::stop_reason reason) {
  std::cout << "Process " << proc.pid() << ' ';
  switch (reason.reason) {
  case cdb::process_state::exited:
    std::cout << "exited with status " << static_cast<int>(reason.info);
    break;
  case cdb::process_state::terminated:
    std::cout << "terminated with signal " << sigabbrev_np(reason.info);
    break;
  case cdb::process_state::stopped:
    std::cout << "stopped with signal " << sigabbrev_np(reason.info);
    break;
  }
  std::cout << std::endl;
}

void handle_command(std::unique_ptr<cdb::process> &proc,
                    std::string_view line) {
  auto args = split(line, ' ');
  auto command = args[0];
  if (is_prefix(command, "continue")) {
    proc->resume();
    auto reason = proc->wait_on_signal();
    print_stop_reason(*proc, reason);
  } else {
    std::cerr << "Unknow command\n";
  }
}
void main_loop(std::unique_ptr<cdb::process> &proc) {
  char *line = nullptr;
  while ((line = readline("cdb > ")) != nullptr) {
    std::string line_str;
    if (std::string_view(line) == "") {
      if (history_length > 0)
        line_str = history_list()[history_length - 1]->line;
    } else {
      line_str = line;
      add_history(line);
      free(line);
    }
    if (!line_str.empty())
      try {
        handle_command(proc, line_str);
      } catch (const cdb::error &err) {
        std::cout << err.what() << "\n";
      }
  }
}
} // namespace

auto main(int argc, const char **argv) -> int {
  if (argc == 1) {
    std::cerr << "Error: Invalid number of arguments.\n";
    return -1;
  }
  try {
    auto process = attach(argc, argv);
    main_loop(process);
  } catch (const cdb::error &err) {
    std::cout << err.what() << '\n';
  }

  return 0;
}
