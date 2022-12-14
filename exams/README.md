# Tip: You should probably compile these exams on the inflab server

If you try to run exams that depend on a pre-compiled object file with `make all` on your local machine, then you might get an error message like I did.

Check out your architecture with `uname -a`.

I ran the Makefiles on `<...> 5.10.102.1-microsoft-standard-WSL2 <...> x86_64 x86_64 x86_64 GNU/Linux` and then got the following error messages:
```
gcc -fPIE -fPIC -c -o listtool.o listtool.c 
gcc -o listtool list.o listtool.o -lcrypt
/usr/bin/ld: list.o: relocation R_X86_64_32 against `.rodata' can not be used when making a PIE object; recompile with -fPIE
/usr/bin/ld: failed to set dynamic section sizes: bad value
collect2: error: ld returned 1 exit status
make: *** [Makefile:10: listtool] Error 1
```

I also did some research and used the `-fPIE` flag for compilation but it didn't make any difference.

In conclusion: You can attempt to guess what the code behind the given object file my look like by disassembling it and writing everything from scratch,
such that it can be compiled to run on any machine (but I really don't think it is worth the effort).

Instead, to run them, you can just into your inflab account on the inflab server via ssh and run them there (which worked fine after I tried it out).

This is the architecture used on the inflab servers that I could get the executables to run with: ``<...> 3.10.0-1160.80.1.el7.x86_64 <...> x86_64 x86_64 x86_64 GNU/Linux`.