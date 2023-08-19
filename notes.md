**1. string_view 如何高效使用?**
dangling: https://www.learncpp.com/cpp-tutorial/stdstring_view-part-2/
https://segmentfault.com/q/1010000042760011
返回 class 里面 string.substr() string_view Reader::peek() const

**2. lower bound, upper bound 边界情况**
https://stackoverflow.com/questions/529831/returning-the-greatest-key-strictly-less-than-the-given-key-in-a-c-map

**3. std::set erase 之后++iter**
segmentation fault
https://stackoverflow.com/questions/4645705/vector-erase-iterator

**4. protected 变量可视范围**
uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const zero_point 可以用raw_value?

**5. operator()**
operator std::string_view() const { return *buffer_; }
operator std::string&() { return *buffer_; }

**6. std::optinal**
value_or

**7. std::string**
erase substr 效率
迭代器构造string
peek() 重写
0.3 Gbits/s -> 2.7 Gbits/s