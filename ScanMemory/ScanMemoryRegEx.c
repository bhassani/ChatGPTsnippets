using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;

class Program
{
    private const int PROCESS_QUERY_INFORMATION = 0x0400;
    private const int PROCESS_VM_READ = 0x0010;
    private const int PAGE_READONLY = 0x02;
    private const int PAGE_READWRITE = 0x04;
    
    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr OpenProcess(int processAccess, bool bInheritHandle, int processId);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint dwSize, out int lpNumberOfBytesRead);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool VirtualQueryEx(IntPtr hProcess, IntPtr lpAddress, out MEMORY_BASIC_INFORMATION lpBuffer, uint dwLength);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr CreateToolhelp32Snapshot(uint dwFlags, uint th32ProcessID);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool Process32First(IntPtr hSnapshot, ref PROCESSENTRY32 lppe);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool Process32Next(IntPtr hSnapshot, ref PROCESSENTRY32 lppe);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool CloseHandle(IntPtr hObject);

    private const uint TH32CS_SNAPPROCESS = 0x00000002;

    [StructLayout(LayoutKind.Sequential)]
    private struct PROCESSENTRY32
    {
        public uint dwSize;
        public uint cntUsage;
        public uint th32ProcessID;
        public IntPtr th32DefaultHeapID;
        public uint pcPriClassBase;
        public uint dwFlags;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string szExeFile;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MEMORY_BASIC_INFORMATION
    {
        public IntPtr BaseAddress;
        public IntPtr AllocationBase;
        public uint AllocationProtect;
        public uint RegionSize;
        public uint State;
        public uint Protect;
        public uint Type;
    }

    static void Main(string[] args)
    {
        string pattern = @"^(?:4[0-9]{12}(?:[0-9]{3})?|[25][1-7][0-9]{14}|6(?:011|5[0-9][0-9])[0-9]{12}|3[47][0-9]{13}|3(?:0[0-5]|[68][0-9])[0-9]{11}|(?:2131|1800|35\d{3})\d{11})$";
        Regex regex = new Regex(pattern);

        ScanProcesses(regex);
    }

    private static void ScanProcesses(Regex regex)
    {
        IntPtr snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == IntPtr.Zero)
        {
            Console.WriteLine("Failed to create process snapshot.");
            return;
        }

        PROCESSENTRY32 pe32 = new PROCESSENTRY32 { dwSize = (uint)Marshal.SizeOf(typeof(PROCESSENTRY32)) };

        if (!Process32First(snapshot, ref pe32))
        {
            Console.WriteLine("Failed to get first process.");
            CloseHandle(snapshot);
            return;
        }

        do
        {
            string exeFile = pe32.szExeFile.ToLower();
            if (exeFile != "lsass.exe" && exeFile != "csrss.exe" && exeFile != "system")
            {
                IntPtr processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, (int)pe32.th32ProcessID);
                if (processHandle != IntPtr.Zero)
                {
                    ScanProcessMemory(processHandle, regex);
                    CloseHandle(processHandle);
                }
            }
        } while (Process32Next(snapshot, ref pe32));

        CloseHandle(snapshot);
    }

    private static void ScanProcessMemory(IntPtr processHandle, Regex regex)
    {
        IntPtr address = IntPtr.Zero;
        MEMORY_BASIC_INFORMATION memInfo = new MEMORY_BASIC_INFORMATION();

        while (true)
        {
            if (VirtualQueryEx(processHandle, address, out memInfo, (uint)Marshal.SizeOf(typeof(MEMORY_BASIC_INFORMATION))) == 0)
                break;

            if (memInfo.State == 0x1000 && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY))
            {
                byte[] buffer = new byte[(int)memInfo.RegionSize];
                if (ReadProcessMemory(processHandle, memInfo.BaseAddress, buffer, (uint)buffer.Length, out _))
                {
                    string text = System.Text.Encoding.ASCII.GetString(buffer);
                    if (regex.IsMatch(text))
                    {
                        Console.WriteLine("Found pattern in process memory!");
                    }
                }
            }

            address = new IntPtr(memInfo.BaseAddress.ToInt64() + (long)memInfo.RegionSize);
        }
    }
}
