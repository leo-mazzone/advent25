# README

My solutions to the 2025 edition of [Advent of Code](https://adventofcode.com/).

All these were written at a time I felt quite rusty with C (hence my choice of the language, as a learning exercise), and also had limited time to work on this. My approach and its implementation are likely to be sub-optimal as a consequence.

That said, I strived to implement everything myself. AI was used sparingly and only when I struggled to express a bit of logic using idiomatic C patterns. For example, AI helped generate a dynamically-allocated 2D grid using a pointer to an array:

```c
char (*lines)[MAX_LINE_LEN] = malloc(MAX_LINES * MAX_LINE_LEN);
```

AI was also used to recommend standard C functions I might not have remembered, like `strtok`.
