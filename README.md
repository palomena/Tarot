<img src="data/images/chariot-6016921_1920.jpg" width="120px" /> <img src="data/images/empress-6016923_1920.jpg" width="120px" /> <img src="data/images/hermit-6016941_1920.jpg"  width="120px" /> <img src="data/images/hanged-man-6016939_1920.jpg"  width="120px" /> <img src="data/images/tarot-6129686_1920.jpg"  width="120px" />

> **THIS IS WORK IN PROGESS. MY TOY PROJECT THAT I'M COMING BACK TO EVERY OTHER YEAR OR SO. IN DEVELOPMENT SINCE 2019 AND HAS SINCE SEEN NUMEROUS REVISIONS AND ITERATIONS. DO NOT RE-USE WITHOUT SPENDING THE NECESSARY DUE DILIGENCE IN COMMERCIAL PRODUCTION!**

# Tarot
A mystic programming language.

## About
* Multi-paradigm programming language
* Modern C-style syntax
* Strong static type system
* Compiles to compact bytecode
* Runs in a lightweight Virtual Machine
* Implemented in freestanding ISO C90
* Small footprint; suited for embedded systems
* Arbitrary-precision builtin datatypes
* Builtin scalable multi-threading (WIP)
* Exception handling (WIP)

### Example
```
function main() {
	let i = 1;
	while i < 10 {
		print(f"{i}^2 = {i**2}");
		i = i + 1;
	}
}
```

### Use Cases
* Embedded Systems
    * Hot-reloading without flashing
    * Advanced features out of the box
    * Execute programs from SD card, the internet or via Serial
* Scripting
    * Add scripting to existing programs

## Build
Simply run make in the root directory:
```bash
make -j 4
```

To build in debug mode run (slow+large, but contains valuable debug info)
```bash
make debug -j 4
```

To build an optimized release version run (fast+small)
```bash
make release -j 4
```

To clean existing builds run
```bash
make clean
```

#### Build Options
You can customize a build with a few configurable build options:
* BACKEND
	* `default`: Uses builtin mini-gmp implementation (default)
	* `gmp`: Requires the libgmp dependency, more optimized, but larger size
* CC: Name of the C compiler to be used

## Specifications

### ROM requirements

* Platform: Linux debian 6.1.0-28-amd64
	* ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV),
	* dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2
	* filesize: 84KB

* Platform: Linux debian 6.1.0-28-amd64
	* ELF 32-bit LSB pie executable, Intel 80386, version 1 (SYSV)
	* dynamically linked, interpreter /lib/ld-linux.so.2
	* dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2
	* filesize: 78KB

## Alternatives
There are some well-established programming languages that
achieve similar goals. Maybe one of those is a already good fit?

* Lua
* Python

## License
The sourcecode is provided under the terms of the MIT License.
