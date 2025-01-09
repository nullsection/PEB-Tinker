#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <iostream>

// Custom defined undocumented struct for the pem proces params
typedef struct _CUSTOM_RTL_USER_PROCESS_PARAMETERS {
    ULONG                   MaximumLength;
    ULONG                   Length;
    ULONG                   Flags;
    ULONG                   DebugFlags;
    PVOID                   ConsoleHandle;
    ULONG                   ConsoleFlags;
    HANDLE                  StdInputHandle;
    HANDLE                  StdOutputHandle;
    HANDLE                  StdErrorHandle;
    UNICODE_STRING          CurrentDirectoryPath;
    HANDLE                  CurrentDirectoryHandle;
    UNICODE_STRING          DllPath;
    UNICODE_STRING          ImagePathName;
    UNICODE_STRING          CommandLine;
    PVOID                   Environment;
    ULONG                   StartingPositionLeft;
    ULONG                   StartingPositionTop;
    ULONG                   Width;
    ULONG                   Height;
    ULONG                   CharWidth;
    ULONG                   CharHeight;
    ULONG                   ConsoleTextAttributes;
    ULONG                   WindowFlags;
    ULONG                   ShowWindowFlags;
    UNICODE_STRING          WindowTitle;
    UNICODE_STRING          DesktopName;
    UNICODE_STRING          ShellInfo;
    UNICODE_STRING          RuntimeData;
    

} _CUSTOM_RTL_USER_PROCESS_PARAMETERS, * _PRTL_USER_PROCESS_PARAMETERS;

// Standard PEB redef 
typedef struct _CUSTOM_PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    _CUSTOM_RTL_USER_PROCESS_PARAMETERS* ProcessParameters;
} CUSTOM_PEB;

// Update the PEB 
void UpdateParam(UNICODE_STRING& commandline, const std::wstring& newCommand) {
    size_t newLength = newCommand.length() * sizeof(WCHAR);

    // Validate that the destination buffer is writable and large enough
    if (commandline.Buffer && newLength <= commandline.MaximumLength) {
        // Copy the new string into the UNICODE_STRING's buffer
        memcpy(commandline.Buffer, newCommand.c_str(), newLength);

        // Null-terminate the string if necessary
        if (newLength < commandline.MaximumLength) {
            commandline.Buffer[newCommand.length()] = L'\0'; // Null-terminate
        }

        // Update the UNICODE_STRING's metadata
        commandline.Length = static_cast<USHORT>(newLength);
    }
    else {
        std::wcerr << L"Buffer is null or the new command exceeds the buffer's maximum length." << std::endl;
    }
}
// Print  UNICODESTRING
void PrintUnicodeString(const UNICODE_STRING& uniString) {
    // Create a std::wstring from the UNICODE_STRING buffer and length
    std::wstring outputString(uniString.Buffer, uniString.Length / sizeof(WCHAR));

    // Print the command line using std::wcout
    std::wcout << L"" << outputString << std::endl;
    
}

// override the peb params
void WritePeb() {
#ifdef _M_X64
    CUSTOM_PEB* peb = (CUSTOM_PEB*)__readgsqword(0x60); // For x64
    
#else
    CUSTOM_PEB* peb = (CUSTOM_PEB*)__readfsdword(0x30); // For x86
#endif

    UNICODE_STRING& cmd = peb->ProcessParameters->CommandLine;
    UNICODE_STRING& path = peb->ProcessParameters->ImagePathName;
    UNICODE_STRING& currentDir = peb->ProcessParameters->CurrentDirectoryPath;
    UpdateParam(cmd, L"C:\\Windows\\System32\\cmd.exe /c whoami");
    UpdateParam(path, L"C:\\Windows\\System32\\cmd.exe");
    UpdateParam(currentDir, L"C:\\Windows\\System32\\cmd.exe");
        return;
} 

// Read the PEB and print the commandline 
void readPeb() {
#ifdef _M_X64
    CUSTOM_PEB* peb = (CUSTOM_PEB*)__readgsqword(0x60); // For x64

#else
    CUSTOM_PEB* peb = (CUSTOM_PEB*)__readfsdword(0x30); // For x86
#endif

    UNICODE_STRING  commandline = peb->ProcessParameters->CommandLine;
    UNICODE_STRING image = peb->ProcessParameters->ImagePathName;
    UNICODE_STRING& currentDir = peb->ProcessParameters->CurrentDirectoryPath;
    std::cout << "CommandLine: "; 
    PrintUnicodeString(commandline); 
    std::cout << "ImagePath: "; 
    PrintUnicodeString(image);
    std::cout << "CWD: "; 
    PrintUnicodeString(currentDir);
    return;
} 

int main(int argc, char* argv[]) {
    printf("Before...\n");
    std::cout << "ID:" << GetProcessId(GetCurrentProcess()) << "\n";
    readPeb(); 
    WritePeb();

    printf("After...\n");
    readPeb();
    getchar();
    return 0;
}
