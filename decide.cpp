#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
#include <unordered_map>

bool is_number(const char* cstr, const bool negative_allowed) {
  if (cstr[0] != '-' && !std::isdigit(cstr[0])) {
    return false;
  }

  return std::all_of(
    cstr + negative_allowed,
    cstr + std::strlen(cstr),
    [] (const unsigned char c) {
      return std::isdigit(c);
    }
  );
}

void choose(
  const char** args,
  const int argc,
  const size_t amount,
  std::mt19937& rng
) {
  if (amount >= argc) {
    bool first {true};
    for (size_t i {0}; i < argc; ++i) {
      if (first) {
        first = false;
      } else {
        std::cout << '\n';
      }

      std::cout << args[i];
    }

    return;
  }

  std::vector<const char*> pool;
  for (size_t i {0}; i < argc; ++i) {
    pool.emplace_back(args[i]);
  }

  std::vector<const char*> chosen;
  while (chosen.size() < amount) {
    std::uniform_int_distribution<size_t> idx_dist {0, static_cast<size_t>(pool.size() - 1)};
    size_t random {idx_dist(rng)};
    chosen.emplace_back(pool[random]);
    pool.erase(pool.begin() + random);
  }

  bool first {true};
  for (const auto& val : chosen) {
    if (first) {
      first = false;
    } else {
      std::cout << '\n';
    }

    std::cout << val;
  }
}

void help_flag(const char** args, const int argc, std::mt19937& rng) {
  std::cout << 
    "Usage:\n" <<
    "'decide' (50% chance for yes/no)\n" <<
    "'decide <percentage for yes>'\n" <<
    "'decide <item> {<item>}' (returns one item from the ones listed)\n" <<
    "'decide --amount | -a <int> {<item>}' (returns <int> items from the ones listed)\n" <<
    "'decide --range | -r <inclusive_bound> <other_inclusive_bound>' (returns an int value between <inclusive_bound> and <other_inclusive_bound>)\n" <<
    "'decide --group | -g <int> {<item>}' (creates groups with <int> members randomly out of the given items)";
}

void amount_flag(const char** args, const int argc, std::mt19937& rng) {
  if (is_number(args[0], false)) {
    long long val;
    try {
      val = std::stoll(args[0]);
    } catch (std::out_of_range& e) {
      std::cout << "The amount is too large!";
      return;
    }

    choose(args + 1, argc - 1, val, rng);
  } else {
    std::cout << "Invalid amount!";
  }
}

void range_flag(const char** args, const int argc, std::mt19937& rng) {
  if (argc < 2) {
    std::cout << "Too few arguments! Expected: --range <inclusive bound> <other inclusive bound>";
    return;
  }

  if (!is_number(args[0], true) || !is_number(args[1], true)) {
    std::cout << "Entered bounds aren't valid numbers!"; 
    return;
  }

  long long min;
  long long max;

  try {
    min = std::stoll(args[0]);
    max = std::stoll(args[1]);
  } catch (std::out_of_range& e) {
    std::cout << "One or both values are out of bounds!";
    return;
  }

  std::uniform_int_distribution<long long> dist {
    std::min(min, max),
    std::max(min, max)
  };
  std::cout << dist(rng);
}

void group_flag(const char** args, const int argc, std::mt19937& rng) {
  if (!argc) {
    std::cout << "Too few arguments! Expected: --group <group size> {<item>}";
    return;
  }

  if (!is_number(args[0], false)) {
    std::cout << "Invalid group size!";
    return;
  }

  std::shuffle(args + 1, args + argc, rng);
  size_t group_sz {static_cast<size_t>(std::stoi(args[0]))};
  bool first {true};

  size_t idx {0};
  while (idx < argc - 1) {
    if (idx && !(idx % group_sz)) {
      std::cout << '\n';
      first = true;
    }

    if (first) {
      first = false;
    } else {
      std::cout << ", ";
    }

    std::cout << args[++idx];
  }
}

int main(const int argc, const char** argv) {
  std::mt19937 rng {std::random_device {}()};
  if (argc == 1) {
    std::uniform_int_distribution<int> percentage_dist {0, 1};
    std::cout << (percentage_dist(rng) ? "YES!" : "NO!"); std::cout << '\n';
    return 0;
  }

  if (argv[1][0] == '-') {
    std::unordered_map<std::string, void (*)(const char**, const int, std::mt19937&)> flags {
      {"--help", help_flag}, {"-h", help_flag},
      {"--amount", amount_flag}, {"-a", amount_flag},
      {"--range", range_flag}, {"-r", range_flag},
      {"--group", group_flag}, {"-g", group_flag}
    };

    std::string flag {argv[1]};
    const auto& it {flags.find(flag)};

    if (it != flags.end()) {
      it->second(argv + 2, argc - 2, rng);
      std::cout << '\n';
      return 0;
    }
  }

  if (argc == 2) {
    if (is_number(argv[1], false)) {
      std::uniform_int_distribution<int> percentage_dist {0, 100};
      int percent;

      try {
        percent = std::stoi(argv[1]);
        if (percent < 0 || percent > 100) {
          throw std::out_of_range("");
        }
      } catch (std::out_of_range&) {
        std::cout << "The percentage must be between 0 and 100!\n";
        return 0;
      }

      std::cout << (percentage_dist(rng) <= percent ? "YES!\n" : "NO!\n");
      return 0;
    } 
  }

  std::uniform_int_distribution<size_t> idx_dist {1, static_cast<size_t>(argc - 1)};
  std::cout << argv[idx_dist(rng)] << '\n';

  return 0;
}
