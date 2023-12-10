# lumencpp

Lumen library for modern C++.

## Build

To build the library, run:

```bash
$ git clone https://github.com/veevol/lumencpp && cd lumencpp
$ make
```

### Installation 

To install the library system-wide, run:

```
# make install
```

## Usage

To parse a file, you can use the `lumen::parse_file` function:

```cpp
#include <lumencpp/lumen.h>

#include <iostream>
#include <string>

int main() {
    try {
        auto document = lumen::parse_file("metadata.lm");
        std::cout << document["license"].as<std::string>() << '\n';
    } catch (const lumen::SyntaxError& error) {
        std::cout << error.pretty() << '\n';
    } catch (const std::exception& error) {
        std::cout << "error: " << error.what() << '\n';
    }
}
```

To parse a string, use `lumen::parse` function:

```cpp
#include <lumencpp/lumen.h>

#include <iostream>
#include <string>

int main() {
    try {
        auto document = lumen::parse(R"(
            license = "MIT License"
        )");

        std::cout << document["license"].as<std::string>() << '\n';
    } catch (const lumen::SyntaxError& error) {
        std::cout << error.pretty() << '\n';
    } catch (const std::exception& error) {
        std::cout << "error: " << error.what() << '\n';
    }
}
```

To construct a document, you can use `std::map`-like initialization syntax:

```cpp
#include <lumencpp/lumen.h>
#include <iostream>

int main() {
    lumen::Document document{
        {
            "data", {
                {"number", 42}
            }
        }
    };

    std::cout << document["data"]["number"].as<int>() << '\n';
}
```
