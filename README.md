# DCKFinder: Dangling COM Keys Finder

A simple utility to locate COM registry keys referencing DLL or EXE which are not on the system anymore.

## Disclaimer

I'm quite a bad C++ coder, so bear with me but this code might not be really elegant. 

## Overview

During COM development or testing, it is usual to register many COM server DLLs/EXEs in the system to test their functionalities. The main issue with that 
is that several orphaned keys might be left in the Windows Registry, just as garbage. This utility has been developed to easily locate these instances, in order to 
delete them.

## Why

I attended the amazing COM Programming Course by [Pavel Yosifovich][1], and I found myself registering many DLLs for testing various COM test servers, leaving a lot of garbage in the registry.
Within the class we discussed how it could be actually useful to craete a little utility to locate all these "orphaned" or "dangling" registry keys, and that's the reason for this tool.

## Next Steps

* adding a functionality to automatically delete orphaned DLLs

## Considerations

Be aware that results of the x86 and x64 version of the tool may differ due to [reflected keys](https://docs.microsoft.com/en-gb/troubleshoot/windows-client/deployment/view-system-registry-with-64-bit-windows).


## Credit

* [zodiacon][1] 


[1]: https://twitter.com/zodiacon