*jtypes*
========

*jtypes* is a tiny header only library bringing the JavaScript type system to C++11.

The core object provided is `jtypes::jtype`. A jtype object represents one of the following Javascript (ES5) types:  `undefined`, `null`, `bool`, `number`, `string`, `array`, `object` and `function`.

In this spirit the library is similar to C++ JSON libraries around, but has the additional benefit of being able to represent functions and undefined types. The following example crafts a structured type representing a box.

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
