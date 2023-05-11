# ICS2017 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System) in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to http://nju-ics.gitbooks.io/ics2017-programming-assignment/content/

To initialize, run
```bash
bash init.sh
```

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Nexus-am](https://github.com/NJU-ProjectN/nexus-am)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

(nemu) si 23981
4000000:   55                                    pushl %ebp
4000001:   89 e5                                 movl %esp,%ebp
4000003:   83 ec 0c                              subl $0xc,%esp
4000006:   ff 75 10                              pushl 0x10(%ebp)
4000009:   ff 75 0c                              pushl 0xc(%ebp)
400000c:   ff 75 08                              pushl 0x8(%ebp)
400000f:   e8 54 00 00 00                        call 4000068
4000068:   8d 4c 24 04                           leal 0x4(%esp),%ecx
400006c:   83 e4 f0                              andl $0xfffffff0,%esp
400006f:   ff 71 fc                              pushl -0x4(%ecx)
4000072:   55                                    pushl %ebp
4000073:   89 e5                                 movl %esp,%ebp
4000075:   51                                    pushl %ecx
4000076:   50                                    pushl %eax
4000077:   6a 00                                 pushb $0x0
4000079:   6a 00                                 pushb $0x0
400007b:   6a 00                                 pushb $0x0
400007d:   6a 00                                 pushb $0x0
400007f:   e8 3c 20 00 00                        call 40020c0
40020c0:   55                                    pushl %ebp
40020c1:   89 e5                                 movl %esp,%ebp
40020c3:   53                                    pushl %ebx
40020c4:   8b 45 08                              movl 0x8(%ebp),%eax
40020c7:   8b 4d 10                              movl 0x10(%ebp),%ecx
40020ca:   8b 55 14                              movl 0x14(%ebp),%edx
40020cd:   8b 5d 0c                              movl 0xc(%ebp),%ebx