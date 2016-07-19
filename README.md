jtypes
========

*jtypes* is a tiny header only library bringing the JavaScript type system to C++11.

The core object provided is `jtypes::jtype`. A jtype object represents one of the following Javascript (ES5) types:  `undefined`, `null`, `bool`, `number`, `string`, `array`, `object` and `function`.

This library is similar in the spirit to C++ JSON libraries around, but has the additional benefit of being able to represent functions and undefined types. The following example crafts a structured type representing a box.

```c++
#include <jtypes/jtypes.hpp>

using jtypes::jtype;

using sig = double(double, double);

jtype box = jtype::object {
  { "label", "hello jtypes!" },
  { "dims", jtype::object {
    { "width", 10},
    { "height", 2.0}
  }},
  { "tags", jtype::array{"a", "b", "c"}},
  { "area", jtype::function<sig>(
    [](double w, double h)
    { return w * h; }
  )}
};
```
Calling the `area` function:

```c++
double width = box["dims"]["width"].as<double>();
double height = box["dims"]["height"].as<double>();
double area = box["area"].invoke<sig>(width, height);
```

For more info please read the [unit tests](tests/test_jtypes.cpp).

### Constructing jtype objects

`jtype` objects can be constructed from primitive types

```c++

jtype i = 1;              // number - std::int64_t
jtype u = 1u;             // number - std::uint64_t
jtype d = 2.0;            // number - double
jtype b = true;           // boolean - bool
jtype s = "hello world";  // string - std::string
jtype c = '.';            // string - std:.string

```

`jtype` objects can represent undefined and null

```c++

jtype n = jtype::null();        // null - std::nullptr_t
jtype n = nullptr;              // null - std::nullptr_t

jtype u = jtype::undefined();   // undefined - jtype::undefined_t
jtype u;                        // undefined - jtype::undefined_t

```

`jtype` objects can be initialized from structured types such as arrays

```c++

jtype a = jtype::array {1, 2, 3};                          // [number, number, number]
jtype b = jtype::array {true, "hello", jtype::array{1,2}}; // [boolean, string, [number, number]]

```

or object like structures containing nested properties.

```c++

jtype o = jtype::object {
  {"a", true},
  {"b", 2},
  {"c", "hello"},
  {"d", jtype::object {
    {"n", jtype::null()},
    {"x", jtype::array{1,2,3}},
  }},
};

```

Additionally `jtype` objects can be constructed from callables

```c++

using sum_sig = int(int, int);

// Lambdas

jtype f = jtype::function<sum_sig>(
  [](int a, int b) { return a + b; }
);

// Free functions

int sum(int a, int b) { return a + b; }

jtype f = jtype::function<sum_sig>(&sum);


// Functors

struct functor {
  int operator()(int a, int b) const {
    return a + b;
  }
};

functor func;
jtype f = jtype::function<sum_sig>(func);

// Bindings

int triple_sum(int a, int b, int c) { return a + b + c; }

using namespace std::placeholders;
jtype f = jtype::function<sum_sig>(std::bind(triple_sum, _1, _2, 0));

```



