# python1024

I challenged myself to make a Python interpreter in 1024 bytes of C. It runs _Pythony_ looking code, such as fizzbuzz! See the blog post, [Making a Python interpreter in 1024 bytes](https://austinhenley.com/blog/python1024.html).

Check the length: `wc python1024.c`

Compile with gcc (not clang!): `gcc-16 -std=gnu89 -w python1024.c`

Run it with a Python file: `./a.out < fizzbuzz.py`
