/*

Once loaded Conficker.C hooks the following Windows API functions from
dnsapi.dll in order to intercept DNS requests: DnsQuery A, DnsQuery UTF8,
DnsQuery W and Query Main. When a DNS request occurs, Conficker.C
checks for matches against a list of blocked domains. If there is a match,
Conficker.C calls SetLastError with a value of ERROR TIMEOUT and then
returns this error without calling the original function.

Conficker.C also hooks the ws2 32.dll function sendto as long as the
dnsrslvr.dll module is loaded in the current process. The sendto hook
function checks if it is being called from dnsrslvr.dll. If the call comes
from dnsrslvr.dll then Conficker.C will check if the domain name is in
the blocked domain list and if so will replace it with a randomized string.

*/

