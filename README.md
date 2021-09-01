# BaseTest_Porter
Project Qt-Tests for testing base functions of the Porter.

## How to use
1) Download zip or clone [this](https://github.com/VNovytskyi/BaseTest_Porter) repo.
2) Download zip or clone [Porter](https://github.com/VNovytskyi/Porter) version what you want to test.
3) Edit path to Porter files in qt pro-file.

## Note
Be careful, you are compiling a C ++ application with C code, use "extern C" for include file:
```c
extern "C" {
    #include "porter.h"
}
```
