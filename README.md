<img src="data/images/chariot-6016921_1920.jpg" width="120px" />
<img src="data/images/empress-6016923_1920.jpg" width="120px" />
<img src="data/images/hermit-6016941_1920.jpg"  width="120px" />
<img src="data/images/hanged-man-6016939_1920.jpg"  width="120px" />
<img src="data/images/tarot-6129686_1920.jpg"  width="120px" />

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
* Builtin multi-threading

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
make
```

To build in debug mode run (slow+large, but contains valuable debug info)
```bash
make debug
```

To build an optimized release version run (fast+small)
```bash
make release
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

## Alternatives
There are some well-established programming languages that
achieve similar goals. Maybe one of those is a already good fit?

* Lua
* Python

## License
The sourcecode is provided under the terms of the MIT License.
