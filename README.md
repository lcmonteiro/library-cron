# Library Cron ![cron-ci](https://github.com/lcmonteiro/library-cron/actions/workflows/cron-ci.yml/badge.svg)

Simple Crontab Algorithm Implementation ...

![cron](doc/cron.svg)

# Use Case

``` C++
#include "cron.hpp"

auto space = Cron::Build("*/2 * * * *");
auto point = std::chrono::system_clock::now();
while(true) {
  point = point + space;
  std::this_thread::sleep_until(point);
  std::cout << "run action" << std::endl;
}
```


