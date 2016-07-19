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

Since `jtype::array` and `jtype::object` reference `std::vector<jtype>` and `std::map<jtype>` respectively, all constructors
exposed by those objects can be invoked.

```c++

int data[] = {1, 2, 3};
jtype a = jtype::array(data, data + 3);

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

### Copy Semantics

By default all jtype copy semantics are deep-copy, expect for jtype objects pointing to callables.

```c++

jtype x = jtype::object{
  {"a", jtype::array{"b", "c"}}
};

jtype y = x;

y["a"] = 1; // x["a"] still is array.

```

### Introspection and Coercion

`jtype` objects support type introspection

```c++

jtype x;
x.type();             // jtype::vtype::undefined
x.is_undefined();     // true

x = 1;            
x.type();             // jtype::vtype::signed_number;
x.is_signed_number(); // true

x = jtype::array{}; 
x.type();             // jtype::vtype::array;
x.is_array();         // true
x.is_structured();    // true

// similar for other types. 

```

`jtype` object values can be retrieved and coerced if necessary using the `as()` function

```c++

jtype x = 1;
x.as<int>();          // 1
x.as<std::string>();  // "1"
x.as<double>();       // 1.0
x.as<bool>();         // true

x = jtype::undefined();
x.as<bool>();        // false

x = jtype::null();
x.as<bool>();        // false

if (!x) {
  // Will be entered
}

```

Similarily, casts can be used

```c++

jtype x = "1";    // string
int i = (int)x;   // 1

```

### Queries

`jtype` structured objects (array and object) support member queries.

```c++

jtype x = jtype::object{
  {"a", jtype::array{"b", "c"}}
};

x["a"][0];   // jtype containing "b"

```

For deeply nested objects `at()` is provided.

```c++

jtype x = jtype::object{
  "a", jtype::object{
    {"b", "c"}
  }
};

x.at("a.b"); // jtype containing "c"

```

### Iteration

`jtype` structured objects (array and object) can be iterated in multple ways. For one `jtype` supports method `keys()` and `values`.

```c++

jtype x = jtype::object{
  {"a", jtype::array{"b", "c"}},
  {"f", jtype::null()}
};

x.keys();     // jtype::array{"a", "f"};
x.values();   // jtype::array{jtype::array{"b", "c"}, jtype::null()}

for (auto && k : x.keys()) {
  std::cout << k.as<std::string>() << " : " << x[k].as<std::string>() << ", "; 
} // "a" : ["b","c"], f : "null"

```

`keys()` for array types gives indices

```c++

jtype x = jtype::array{ "a", "b", "c" };

x.keys();     // jtype::array{0, 1, 2};
x.values();   // jtype::array{"a", "b", "c"};

```

A forward iterator is also provided for structured types.

```c++

jtype x = jtype::object{
  {"a", jtype::array{"b", "c"}},
  {"f", jtype::null()}
};

for (auto i = x.begin(); i != end(); ++i) {
  // use x.key() to access property name (object) or index (array)
  std::cout << x.key().as<std::string()> << " : " << i->as<std::string>();
}

```

### Manipulation

```c++

jtype x = jtype::object{};

x.size().as<size_t>(); // 0

x["a"] = 3;
x["b"] = jtype::array{};
x["b"][0] = "hello";
x["b"][3] = "world"; // [1], [2] created as undefined.

x.size().as<size_t>(); // 2
x["b"].size().as<size_t>(); // 4

```

When creating nested object hierarchies be careful to create intermediate objects explicitly or use `at()`

```c++

jtype x = jtype::object{};

x["a"]["b"] = 3; // throws type_error. x["a"] returns undefined. [] operator for undefined throws.

```
Instead use

```c++

jtype x = jtype::object{};
x["a"] = jtype::object{};

x["a"]["b"] = 3; // ok

```

or even shorter

```c++

jtype x = jtype::object{};

x.at("a.b") = 3; // ok

```

### JSON parsing

`jtype` objects can be serialized in JSON format. Parsing support is provided by utilizing [nlohmann/json](https://github.com/nlohmann/json).

```c++

#include <jtypes/jtypes_io.hpp>

// ...

jtype x = jtype::object{
  {"a", jtype::array{"b", "c"}},
  {"f", jtype::null()}
};

// serialize
std::string s = jtypes::to_json(x);

// parse
jtype y = jtypes::from_json(s);

```

Overloads of `to_json` and `from_json` for handling streams instead of strings are provided as well.
