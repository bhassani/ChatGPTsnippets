/*

Source: https://softpanorama.org/Malware/Malware_defense_history/Ch10_rats_and_zombies_networks/Reprints/conficker-analysis.pdf

Firstly, the DLL file itself is encrypted and will decrypt itself into newly
allocated memory within the current process when it starts up. The decryption routine is polymorphic and makes heavy use of redundant API
calls and “spaghetti code”, where functions are split up and connected by
x86 jmp instructions (both direct and indirect). The spaghetti code obfuscation makes static analysis more difficult as it is hard for the analyst to
view a linear disassembly listing for each function, and they must resort to
more advanced disassembler features such as graph views. Redundant API
calls are somewhat effective at preventing accurate emulation by anti-virus
engines, as it is unlikely that they have a full and correct implementation of
all possible Windows API functions and while they may properly emulate
the loading and unloading of stack arguments and return values for most
functions, they are less likely to properly take account of secondary effects
such as error values (available from the GetLastError Windows API).


Conficker.C executes the
sldt instruction to store the segment of the local descriptor table in a register. 
If the result is a non-zero value, Conficker.C assumes that it is running in
a virtual machine and executes a Sleep API call with time set to INFINITE.

• a Red Pill-style check using sidt that is continuously repeated for one
second, only passing if the value returned from sidt didn’t indicate
the presence of a virtual machine once in that time

• a test using the str instruction, comparing the stored task register
with 4000h

• a test using the sgdt instruction, checking if the result is in the range
0ff000000h to 0fffffffffh

a test using the illegal instruction 0f 3f 07 0b which would normally
trigger an illegal opcode exception, but which is known to be handled
differently if the program is running inside VirtualPC

• a test using the in instruction with parameter ‘VMXh’ to check for the
presence of a VMWare virtual machine


Once loaded, Conficker.C hooks the NetpwCanonicalizePath export of
netapi32.dll that is central to the exploitation of the MS08-067 vulnerability. The hook function contains a validation check that effectively patches
the computer against further exploitation via MS08-067 as long as Conficker.C is loaded. If the argument passed to NetpwCanonicalizePath contains the substring “\..\” or is over 200 bytes in length, the hook function
will call SetLastError with error value ERROR INVALID PARAMETER and return this error value without calling the original NetpwCanonicalizePath
function.



*/
