# Defold Music Player

## Creating DUMB archive

DUMB will be used as a static library.

```
cd dumb/src
for i in core it helpers; do gcc "$i"/*.c -c -fPIC -I../include; done
ar cr dumb_fPIC.a *.o
```

## Compiling DMP library

No Makefile yet, but it's simple:

```
g++ libdmp.cpp dumb_with_openal.cpp dumb/src/dumb_fPIC.a -llua -lopenal -fvisibility=hidden -fPIC -shared -Wall -W -Idumb/include -o libdmp.so
```



## Testing

```
./libdmp_test.lua
```
