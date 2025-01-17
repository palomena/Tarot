
# add short type

# add vm only build target/rule

# add embed into exe build target/rule


* optimize push/pop region via preallocating?
-> wayyy too many allocations due to those regions getting pushed/popped
* free variables if integer or rational at end of a scope "manually"
  or add them to a region
-> problem: variables are tracked on the function level, not block scope level
-> but we need to free at the end of each block scope
* Anything that is assigned gets freed manually
* Anything that is computed (Zwischenergebnis) automatically
* free integer at function end does not guarantee that a variable was set because a branch might not have been taken
* consider making regions explicit with add_to_region(ptr), remove_from_region(ptr)...


# This is absolutely needed or memory will be wasted big time!
# push pop region on every {}
# requires to track in which region each variable lies
# e.g. i = i + 1 creates an alloc in the temp region, thereby
# invalidating i < 10 on subsequent iterations!
# Therefore we'd need a modified OP_Store with the relative region as param
# modify OP_Store such that before that opcode a remove from region opcode is inserted
# Also need to take care of freeing the stored value when its overwritten and not
# part of a region.
# maybe with OP_StoreInteger, OP_StoreRational, ...?
##################################################

freelist + elements are still in and free'd by region!? bad!
list & regions of members
list indexed assign

----
initial list create adds list integers to region.
list index store frees integer at list index and overwrites it with another one.
at the end of the region all initial list integers are freed.
However we are now freeing the one at the index twice!
----

OP_StoreInteger without arguments, instead takes 2 stack args instead of 1 (or just 1 and local variable like accumulator)
OP_Variable [index]      puts union value* ptr on stack (or in local variable)
OP_ListValue [index]  puts union value* ptr on stack (or in local variable)

What if we ReturnValue that is an argument (so lays in another region) already?
-> error!
-> we need to copy, think of the case where `x=1; foo(x); foo(x){return x}`
----> gmeacht für ints, jetzt noch für alle datatypes bitte
-----> außerdem return/rreturnvalue fusionieren
---> und validate whether return on all branches


let a = 1;
let b = 1;
a = b; // requires copy, not a simple store! otherwise referenced twice!
// a = b+0; would be valid cuz zwischenergebnis

for loop + range (see lua and berry-lang for reference)
short integers?

overhaul mpz and mpq types to be allocate once, not twice! (saves lots of opcodes and case handling)
-> for mpq pretty much impossible because it consists of two mpz which require seperate handling
-> couldn't be free'd with a simple tarot_free(ptr) call!

# printing print(foo()) result of a function without return value results in tarot_sourcecode_error(__FILE__, __LINE__, "") but should give error!

* somehow the n argument is accessed by storeInteger of fib function and free'd
-> this was because we didn't memset to 0 after popping. When Store accessed the field subsequently it ran free on a value in there that shouldnt have been in there

* Introduce "Value" type with runtime type information (e.g. for dict and easy prints)
  * a value type can be typecast at runtime, nothing more. (addition etc must be cast beforehand)

  *
have two different allocators: 1 for objects with type and 1 for anything

* no literal appendixes such as f or s, but rather force Float(12.23) and Short(1234), e.g. let x = Short(1234);

-> kinda ugly in arithmetics though, e.g. let x = Short(1) + Short(2) + Short(3); or x += Short(1); but it looks ok...

* make dict generally untyped and require Value()

* OP_Free can now infer type from header_of

* Maybe only ever copy before assignment, never otherwise?

########################################
# TODO:
# export foo as foo;
# reduce number of allocations
# exceptions
# for element in list {} # how to get index too?
